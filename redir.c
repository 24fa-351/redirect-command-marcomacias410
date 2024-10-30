#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*Usage: redir <inp> <cmd> <out> */

int main(int argc, char *argv[]) {
    int fd_input;
    int fd_output;

    if (argc < 4) {
        fprintf(stderr, "Usage %s <inp> <cmd> <out> \n", argv[0]);
        return 1;
    }

    printf("argc %d", argc);

    if (strcmp(argv[1], "-") == 0) {
        fd_input = STDIN_FILENO;
    } else {
        fd_input = open(argv[1], O_RDONLY);
        if (fd_input == -1) {
            perror("Failed to open input file");
            return 1;
        }
        dup2(fd_input, STDIN_FILENO);
        close(fd_input);
    }


    // Check if last argument is a text file 
    if (strcmp(argv[argc - 1], "-") == 0) {
        fd_output = STDOUT_FILENO;
    } else {
        fd_output = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_output == -1) {
            perror("Failed to open output file");
        }
    }

    char **newargv = (char **)malloc(sizeof(char *) * (argc));

    for (int ix = 2; ix < argc - 1; ix++) {
        newargv[ix - 2] = (char *)argv[ix];
    }
    newargv[argc - 2] = NULL;

    pid_t pid = fork();

    if (pid == 0) {
        if (fd_output != STDOUT_FILENO) {
            dup2(fd_output, STDOUT_FILENO);
            close(fd_output);
        }

        execvp(newargv[0], newargv);

        printf("Exec failed\n");
        _exit(1);
    }

    close(fd_output);

    int status;
    pid_t wpid = waitpid(pid, &status, 0);
    printf(
        "%s pid is %d. forked %d. "
        "Parent exiting\n",
        argv[0], getpid(), pid);

    return 0;
}