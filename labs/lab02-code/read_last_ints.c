#include <stdio.h>
#include <stdlib.h>

/*
 * Read the last integers from a binary file
 *   'num_ints': The number of integers to read
 *   'file_name': The name of the file to read from
 * Returns 0 on success and -1 on error
 */
int read_last_ints(const char *file_name, int num_ints) {
    FILE* file_path = fopen(file_name, "rb");

    if(file_path == NULL){
        perror("Error opening file");
        return -1;
    }

    if(fseek(file_path, 0, SEEK_END) != 0){
        perror("Error seeking to end");
        fclose(file_path);
        return -1;
    }

    long file_size = ftell(file_path);
    if(file_size == -1){
        perror("Error getting file position");
        fclose(file_path);
        return -1;
    }

    long total_ints = file_size / sizeof(int);

    if(num_ints > total_ints){
        printf("File only contains %ld integers, cannot read %d\n", total_ints, num_ints);
        fclose(file_path);
        return -1;
    }


    long offset = -(num_ints * sizeof(int));
    if(fseek(file_path, offset, SEEK_END) != 0){
        perror("Error seeking to read position");
        fclose(file_path);
        return -1;
    }

    int value;
    for(int i = 0; i < num_ints; i++){
        if(fread(&value, sizeof(int), 1, file_path) != 1){
            perror("Error reading integer");
            fclose(file_path);
            return -1;
        }
        printf("%d\n", value);
    }

    fclose(file_path);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <file_name> <num_ints>\n", argv[0]);
        return 1;
    }

    const char *file_name = argv[1];
    int num_ints = atoi(argv[2]);
    if (read_last_ints(file_name, num_ints) != 0) {
        printf("Failed to read last %d ints from file %s\n", num_ints, file_name);
        return 1;
    }
    return 0;
}
