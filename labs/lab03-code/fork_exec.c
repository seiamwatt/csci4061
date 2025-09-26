#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    // TODO Fork a child process to run the command "cat sample.txt"
    // The parent should wait and print out the child's exit code
    int status;
    pid_t pid = fork();

    if(pid < 0){
        perror("error");
        return 1;
    }else if (pid == 0){
        execlp("cat","cat","sample.txt",NULL);
        perror("failed")
        return 1;
    }else{
        wait(&status);

        if(WIFEXITED(status)){
            printf(WEXITSTATUS(status));

        }else{
            printf("fail");
        }
    }

    printf("Child exited with status %d\n", -1);
    return 0;
}
