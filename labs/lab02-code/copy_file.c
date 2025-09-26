#include <stdio.h>

#define BUF_SIZE 4096

/*
 * Copy the contents of one file into another file
 *   source_file: Name of the source file to copy from
 *   dest_file: Name of the destination file to copy to
 * The destination file is overwritten if it already exists
 * Returns 0 on success and -1 on error
 */
int copy_file(const char *source_file, const char *dest_file) {
    FILE* fp = fopen(source_file, "rb");
    if(fp == NULL) {
        perror("Error opening source file");
        return -1;  // Changed from 1 to -1
    }

    FILE* dest_fp = fopen(dest_file, "wb");
    if(dest_fp == NULL){
        perror("Error opening destination file");
        fclose(fp);
        return -1;  // Changed from 1 to -1
    }

    char buffer[BUF_SIZE];  // Use the defined constant
    size_t bytes_read;

    while((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0){
        if(fwrite(buffer, 1, bytes_read, dest_fp) != bytes_read){
            perror("Error writing to destination file");
            fclose(fp);
            fclose(dest_fp);
            return -1;  // Changed from 1 to -1
        }
    }

    if(ferror(fp)){
        perror("Error reading source file");
        fclose(fp);
        fclose(dest_fp);
        return -1;  // Changed from 1 to -1
    }

    fclose(fp);
    fclose(dest_fp);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <source> <dest>\n", argv[0]);
        return 1;
    }

    // copy_file already prints out any errors
    if (copy_file(argv[1], argv[2]) != 0) {
        return 1;
    }
    return 0;
}
