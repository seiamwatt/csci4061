// redirect_child.c: starts a child process which will print into a
// file instead of onto the screen. Uses dup2(), fork(), and wait()
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {    // check for at least 1 command line arg
        printf("Usage: %s <childfile>\n", argv[0]);
        return 1;
    }

    // Uncomment lines below to use specified output file and command-line args in child process
    // output file that child process will print into
    char *output_file = argv[1];
    // child command/arguments to execute
    char *child_argv[] = {"wc", "test_cases/resources/nums.txt", NULL};

    // TODO: Spawn a child process, which will exec the "wc" command with the arguments in
    // 'child_argv' Redirect the output of the command to 'output_file'

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror("open");
            exit(1);
        }

        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(fd);
            exit(1);
        }

        close(fd);

        execvp(child_argv[0], child_argv);

        perror("execvp");
        exit(1);
    }

    // TODO: In the parent, wait for the child and ensure it terminated normally using wait macros
    // Print "Child complete, return code <status_code>" if child terminated normally, replacing
    //    <status_code> with the child's numerical status code
    // Print "Child exited abnormally" if child terminated abnormally

    int status;
    if (wait(&status) == -1) {
        perror("wait");
        return 1;
    }

    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Child complete, return code %d\n", exit_code);
    } else {
        printf("Child exited abnormally\n");
    }

    return 0;
}
