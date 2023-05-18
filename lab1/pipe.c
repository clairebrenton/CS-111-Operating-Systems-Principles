#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No command line arguments provided\n");
        exit(EINVAL);
    }

    int pipefd[2];
    int prev_pipefd[2] = {-1, -1};
    pid_t pid;

    for (int i = 1; i < argc; ++i) {
        if (i < argc - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(errno);
            }
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(errno);
        }

        if (pid == 0) { // Child process
            if (i > 1) {
                dup2(prev_pipefd[0], STDIN_FILENO);
                close(prev_pipefd[0]);
            }

            if (i < argc - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            if (prev_pipefd[1] != -1) {
                close(prev_pipefd[1]);
            }

            if (execlp(argv[i], argv[i], NULL) == -1) {
                fprintf(stderr, "Error executing program: %s\n", argv[i]);
                exit(1);
            }
        } else {
            if (prev_pipefd[0] != -1) {
                close(prev_pipefd[0]);
            }
            if (prev_pipefd[1] != -1) {
                close(prev_pipefd[1]);
            }

            prev_pipefd[0] = pipefd[0];
            prev_pipefd[1] = pipefd[1];
        }
    }

    if (pipefd[0] != -1) {
        close(pipefd[0]);
    }
    if (pipefd[1] != -1) {
        close(pipefd[1]);
    }

    for (int i = 1; i < argc; ++i) {
        int status;
        wait(&status);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            return WEXITSTATUS(status);
        }
    }

    return 0;
}
