#include "minitar.h"

#include <fcntl.h>
#include <grp.h>
#include <math.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_TRAILING_BLOCKS 2
#define MAX_MSG_LEN 128
#define BLOCK_SIZE 512

// Constants for tar compatibility information
#define MAGIC "ustar"

// Constants to represent different file types
// We'll only use regular files in this project
#define REGTYPE '0'
#define DIRTYPE '5'

/*
 * Helper function to compute the checksum of a tar header block
 * Performs a simple sum over all bytes in the header in accordance with POSIX
 * standard for tar file structure.
 */
void compute_checksum(tar_header *header) {
    // Have to initially set header's checksum to "all blanks"
    memset(header->chksum, ' ', 8);
    unsigned sum = 0;
    char *bytes = (char *) header;
    for (int i = 0; i < sizeof(tar_header); i++) {
        sum += bytes[i];
    }
    snprintf(header->chksum, 8, "%07o", sum);
}

/*
 * Populates a tar header block pointed to by 'header' with metadata about
 * the file identified by 'file_name'.
 * Returns 0 on success or -1 if an error occurs
 */
int fill_tar_header(tar_header *header, const char *file_name) {
    memset(header, 0, sizeof(tar_header));
    char err_msg[MAX_MSG_LEN];
    struct stat stat_buf;
    // stat is a system call to inspect file metadata
    if (stat(file_name, &stat_buf) != 0) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to stat file %s", file_name);
        perror(err_msg);
        return -1;
    }

    strncpy(header->name, file_name, 100);    // Name of the file, null-terminated string
    snprintf(header->mode, 8, "%07o",
             stat_buf.st_mode & 07777);    // Permissions for file, 0-padded octal

    snprintf(header->uid, 8, "%07o", stat_buf.st_uid);    // Owner ID of the file, 0-padded octal
    struct passwd *pwd = getpwuid(stat_buf.st_uid);       // Look up name corresponding to owner ID
    if (pwd == NULL) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to look up owner name of file %s", file_name);
        perror(err_msg);
        return -1;
    }
    strncpy(header->uname, pwd->pw_name, 32);    // Owner name of the file, null-terminated string

    snprintf(header->gid, 8, "%07o", stat_buf.st_gid);    // Group ID of the file, 0-padded octal
    struct group *grp = getgrgid(stat_buf.st_gid);        // Look up name corresponding to group ID
    if (grp == NULL) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to look up group name of file %s", file_name);
        perror(err_msg);
        return -1;
    }
    strncpy(header->gname, grp->gr_name, 32);    // Group name of the file, null-terminated string

    snprintf(header->size, 12, "%011o",
             (unsigned) stat_buf.st_size);    // File size, 0-padded octal
    snprintf(header->mtime, 12, "%011o",
             (unsigned) stat_buf.st_mtime);    // Modification time, 0-padded octal
    header->typeflag = REGTYPE;                // File type, always regular file in this project
    strncpy(header->magic, MAGIC, 6);          // Special, standardized sequence of bytes
    memcpy(header->version, "00", 2);          // A bit weird, sidesteps null termination
    snprintf(header->devmajor, 8, "%07o",
             major(stat_buf.st_dev));    // Major device number, 0-padded octal
    snprintf(header->devminor, 8, "%07o",
             minor(stat_buf.st_dev));    // Minor device number, 0-padded octal

    compute_checksum(header);
    return 0;
}

/*
 * Removes 'nbytes' bytes from the file identified by 'file_name'
 * Returns 0 upon success, -1 upon error
 * Note: This function uses lower-level I/O syscalls (not stdio), which we'll learn about later
 */
int remove_trailing_bytes(const char *file_name, size_t nbytes) {
    char err_msg[MAX_MSG_LEN];

    struct stat stat_buf;
    if (stat(file_name, &stat_buf) != 0) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to stat file %s", file_name);
        perror(err_msg);
        return -1;
    }

    off_t file_size = stat_buf.st_size;
    if (nbytes > file_size) {
        file_size = 0;
    } else {
        file_size -= nbytes;
    }

    if (truncate(file_name, file_size) != 0) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to truncate file %s", file_name);
        perror(err_msg);
        return -1;
    }
    return 0;
}

int create_archive(const char *archive_name, const file_list_t *files) {
    FILE* file_path = fopen(archive_name,"wb");

    if(file_path == NULL){
        perror("failed to open file");
        return -1;
    }
    node_t *current = files->head;

    while(current != NULL){
        tar_header header;

        if(fill_tar_header(&header,current->name) != 0){
            perror("cannot fill tar header");
            fclose(file_path);
            return -1;
        }

        if(fwrite(&header,sizeof(tar_header),1,file_path) != 1){
            perror("cannot write");
            fclose(file_path);
            return -1;
        }

        FILE *curr_file = fopen(current->name,"rb");

        if(curr_file == NULL){
            fclose(file_path);
            perror("current file does not exist");
            return -1;
        }

        unsigned file_size;
        char buffer[BLOCK_SIZE];

        if(sscanf(header.size,"%o",&file_size) != 1 ){
            fclose(curr_file);
            fclose(file_path);
            return -1;
        }

        unsigned bytes_remain =  file_size;

        while(bytes_remain > 0){
            memset(buffer,0,BLOCK_SIZE);

            size_t bytes_need_reading = fmin(bytes_remain,BLOCK_SIZE);
            size_t bytes_read = fread(buffer,1,bytes_need_reading,curr_file);

            if(bytes_read != bytes_need_reading){
                fclose(file_path);
                fclose(curr_file);
                perror("bytes read and bytes need to be read are not equal");
                return -1;
            }

            if(fwrite(buffer,BLOCK_SIZE,1,file_path) != 1 ){
                fclose(file_path);
                fclose(curr_file);
                perror("cannot write");
                return -1;
            }

            bytes_remain = bytes_remain - bytes_read;
        }

        fclose(curr_file);
        current = current->next;
    }

    char zero_block[BLOCK_SIZE];
    memset(zero_block,0,BLOCK_SIZE);

    if (fwrite(zero_block, BLOCK_SIZE, 1, file_path) != 1 ||
        fwrite(zero_block, BLOCK_SIZE, 1, file_path) != 1) {
        perror("error");
        fclose(file_path);
        return -1;
    }

    fclose(file_path);
    return 0;
}


int append_files_to_archive(const char *archive_name, const file_list_t *files) {
    FILE *archive_file_path = fopen(archive_name,"rb");

    if(archive_file_path == NULL){
        return -1;
    }

    fclose(archive_file_path);

    if(remove_trailing_bytes(archive_name,NUM_TRAILING_BLOCKS * BLOCK_SIZE) != 0){
        return -1;
    }

    archive_file_path = fopen(archive_name,"ab");

    if(archive_file_path == NULL){
        return -1;
    }

    node_t *curr = files -> head;

    while(curr != NULL){
        tar_header header;

        if(fill_tar_header(&header,curr->name) != 0){
            fclose(archive_file_path);
            return -1;
        }

        if(fwrite(&header,sizeof(tar_header),1,archive_file_path) != 1){
            fclose(archive_file_path);
            return -1;
        }

        FILE *curr_file_path = fopen(curr->name,"rb");

        if(curr_file_path == NULL){
            fclose(archive_file_path);
            return -1;
        }

        unsigned file_size ;

        if(sscanf(header.size,"%o",&file_size) != 1){
            fclose(curr_file_path);
            fclose(archive_file_path);
            return -1;
        }

        char buffer[BLOCK_SIZE];
        unsigned bytes_remain = file_size;

        while(bytes_remain > 0){
            memset(buffer,0,BLOCK_SIZE);

            size_t bytes_to_read = fmin(bytes_remain,BLOCK_SIZE);
            size_t bytes_read = fread(buffer, 1, bytes_to_read, curr_file_path);

            if(bytes_read != bytes_to_read){
                fclose(curr_file_path);
                fclose(archive_file_path);
                return -1;
            }

            if(fwrite(buffer, BLOCK_SIZE, 1, archive_file_path) != 1){
                fclose(curr_file_path);
                fclose(archive_file_path);
                return -1;
            }

            bytes_remain = bytes_remain - bytes_read;
        }

        fclose(curr_file_path);
        curr = curr -> next;
    }

    char zero_block[BLOCK_SIZE];
    memset(zero_block, 0, BLOCK_SIZE);

    if (fwrite(zero_block, BLOCK_SIZE, 1, archive_file_path) != 1 ||
        fwrite(zero_block, BLOCK_SIZE, 1, archive_file_path) != 1) {
        fclose(archive_file_path);
        return -1;
    }

    fclose(archive_file_path);
    return 0;
}

int get_archive_file_list(const char *archive_name, file_list_t *files) {
    FILE *archive_file_path = fopen(archive_name,"rb");

    if(archive_file_path == NULL){
        return -1;
    }

    tar_header header;

    while(fread(&header,sizeof(tar_header),1,archive_file_path)== 1){
        int zero = 1;
        char *bytes = (char *)&header;

        for(int i = 0; i < sizeof(tar_header);i++){
            if(bytes[i] != 0){
                zero = 0;
                break;
            }
        }

        if(zero){
            break;
        }

        if(file_list_add(files,header.name) != 0){
            fclose(archive_file_path);
            return -1;
        }

        unsigned file_size;
        if (sscanf(header.size, "%o", &file_size) != 1) {
            fclose(archive_file_path);
            return -1;
        }

        unsigned block_needed = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

         if (fseek(archive_file_path, block_needed * BLOCK_SIZE, SEEK_CUR) != 0) {
            fclose(archive_file_path);
            return -1;
        }
    }
    fclose(archive_file_path);
    return 0;
}


int extract_files_from_archive(const char *archive_name) {
    FILE *archive_file_path = fopen(archive_name,"rb");
    if(archive_file_path == NULL){
        return -1;
    }

    file_list_t processed_files;
    file_list_init(&processed_files);

    tar_header header;

    while(fread(&header,sizeof(tar_header),1,archive_file_path) == 1){
        int zeros = 1;
        char *bytes = (char *)&header;

        for(int i = 0;i< sizeof(tar_header);i++){
            if(bytes[i] != 0){
                zeros = 0;
                break;
            }
        }

        if(zeros){
            break;
        }

        unsigned file_size;
        if(sscanf(header.size,"%o",&file_size) != 1){
            file_list_clear(&processed_files);
            fclose(archive_file_path);
            return -1;
        }

        FILE *output_file = fopen(header.name, "wb");
        if (output_file == NULL) {

            file_list_clear(&processed_files);
            fclose(archive_file_path);
            return -1;
        }

        unsigned bytes_remain = file_size;
        char buffer[BLOCK_SIZE];

        while(bytes_remain > 0){
            if(fread(buffer,BLOCK_SIZE,1,archive_file_path) != 1){
                fclose(output_file);
                file_list_clear(&processed_files);
                fclose(archive_file_path);
                return -1;
            }

            size_t bytes_to_write = fmin(bytes_remain,BLOCK_SIZE);

            if(fwrite(buffer,1,bytes_to_write,output_file) != bytes_to_write){
                fclose(output_file);
                file_list_clear(&processed_files);
                fclose(archive_file_path);
                return -1;
            }
            bytes_remain = bytes_remain - bytes_to_write;
        }
        fclose(output_file);

        if(!file_list_contains(&processed_files,header.name)){
            file_list_add(&processed_files,header.name);
        }
    }

    file_list_clear(&processed_files);
    fclose(archive_file_path);
    return 0;
}
