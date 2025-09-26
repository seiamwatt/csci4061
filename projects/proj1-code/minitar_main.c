#include <stdio.h>
#include <string.h>

#include "file_list.h"
#include "minitar.h"

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: %s -c|a|t|u|x -f ARCHIVE [FILE...]\n", argv[0]);
        return 0;
    }

    file_list_t files;
    file_list_init(&files);

    // TODO: Parse command-line arguments and invoke functions from 'minitar.h'
    // to execute archive operations

    char operation = '\0';
    char *archive_name = NULL;
    int i;

    // search through argc for valid operation
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && strlen(argv[i]) == 2) {
            char flag = argv[i][1];
            if (flag == 'c' || flag == 'a' || flag == 't' || flag == 'u' || flag == 'x') {
                operation = flag;
                break;
            }
        }
    }

    // find the archive file name
    for (i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            archive_name = argv[i + 1];
            break;
        }
    }

    // check for correct format
    if (operation == '\0' || archive_name == NULL) {
        printf("Usage: %s -c|a|t|u|x -f ARCHIVE [FILE...]\n", argv[0]);
        file_list_clear(&files);
        return 1;
    }

    // collect file arguments for operations that need them
    if (operation == 'c' || operation == 'a' || operation == 'u') {
        for (i = 1; i < argc; i++) {
            if ((argv[i][0] == '-') ||(i > 0 && strcmp(argv[i-1], "-f") == 0)) {
                continue;
            }

            if (file_list_add(&files, argv[i]) != 0) {
                printf("Error: Failed to add file to list\n");
                file_list_clear(&files);
                return 1;
            }
        }
    }

    int result = 0;

    if (operation == 'c') {
        result = create_archive(archive_name, &files);
    } else if (operation == 'a') {
        result = append_files_to_archive(archive_name, &files);
    } else if (operation == 't') {
        file_list_t archive_files;
        file_list_init(&archive_files);
        result = get_archive_file_list(archive_name, &archive_files);
        if (result == 0) {
            node_t *current = archive_files.head;
            while (current != NULL) {
                printf("%s\n", current->name);
                current = current->next;
            }
        }
        file_list_clear(&archive_files);

        // append the files if they are not in the archive, also check for any duplicates
    } else if (operation == 'u') {
        file_list_t archive_files;
        file_list_init(&archive_files);
        if (get_archive_file_list(archive_name, &archive_files) != 0) {
            printf("Error: Failed to read archive\n");
            file_list_clear(&archive_files);
            result = -1;
        } else if (!file_list_is_subset(&files, &archive_files)) {
            printf("Error: One or more of the specified files is not already present in archive\n");
            file_list_clear(&archive_files);
            result = -1;
        } else {
            file_list_clear(&archive_files);
            result = append_files_to_archive(archive_name, &files);
        }
    } else if (operation == 'x') {
        result = extract_files_from_archive(archive_name);
    } else {
        printf("Error: Invalid operation\n");
        result = -1;
    }

    // clean up memory
    file_list_clear(&files);
    return 0;
}
