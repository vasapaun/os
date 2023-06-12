#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_COM (256)
#define MAX_ERR (4095)

#define check_error(expr, msg) \
    do{ \
        if(!(expr)){ \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#define RD_END (0)
#define WR_END (1)

int main(int argc, char** argv){
    check_error(2 == argc, "argc");

    FILE* infile = fopen(argv[1], "r");
    check_error(infile != NULL, "infile open");

    char command[MAX_COM], argument[MAX_COM];
    char maxCommand[MAX_COM];
    size_t maxlen = 0;

    while(fscanf(infile, "%s %s", command, argument) == 2){
        int pipefd[2];
        check_error(pipe(pipefd) != -1, "pipe");

        pid_t child = fork();
        check_error(child != (pid_t)-1, "fork");

        if(child == 0) { //u childu smo
            check_error(close(pipefd[RD_END]) != -1, "close rd_end");
            check_error(dup2(pipefd[WR_END], STDOUT_FILENO) != -1, "dup2");
            check_error(execlp(command, command, argument, (char*) NULL) != -1, "execlp");
        }

        else { //u parentu smo
            check_error(close(pipefd[WR_END]) != -1, "close wr_end");

            FILE* chtopar = fdopen(pipefd[RD_END], "r");
            check_error(chtopar != NULL, "chtopar fdopen");

            char* buff = NULL;
            size_t len = 0;

            int status;
            check_error(wait(&status) != -1, "wait");

            if(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS){
                size_t localLen = 0;
                while(getline(&buff, &len, chtopar) != EOF){
                    localLen += strlen(buff);
                }
                if(localLen > maxlen){
                    strcpy(maxCommand, command);
                    maxlen = localLen;
                }
                free(buff);
            }
        }
    }

    printf("%s\n", maxCommand);

    check_error(fclose(infile) != -1, "fclose infile");

    return 0;
}