#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define WR_END (1)
#define RD_END (0)

#define check_error(expr, msg) \
    do { \
        if(!(expr)) { \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#define MAX_LEN 256
#define MAX_LINE 4096

int main(int argc, char** argv){
    check_error(2 == argc, "argc");

    FILE* infile = fopen(argv[1], "r");
    check_error (infile != NULL, "fopen");

    char cmd[MAX_LEN], arg[MAX_LEN], maxCmd[MAX_LEN], maxArg[MAX_LEN];
    ssize_t maxOutputLength = -__INT_MAX__;

    while(2 == fscanf(infile, "%s %s", cmd, arg)){
        int pipefds[2];
        check_error(-1 != pipe(pipefds), "pipe");

        pid_t child = fork();
        check_error((pid_t) -1 != child, "fork");

        if (0 == child) { // u child procesu 
            check_error(-1 != close(pipefds[RD_END]), "close wr end");
            check_error(-1 != dup2(pipefds[WR_END], STDOUT_FILENO), "dup2");
            check_error(-1 != execlp(cmd, cmd, arg, NULL), "execl");
        }
        else { // u parent procesu smo
            check_error(-1 != close(pipefds[WR_END]), "close");

            int status;
            check_error(child == wait(&status), "wait"); //probaj posle sa child == wait(&status)
            
            FILE* chtopar = fdopen(pipefds[RD_END], "r");

            char* line = (char*) malloc(MAX_LINE * sizeof(char));
            check_error(line != NULL, "line malloc");
            size_t len = 0;
            ssize_t outputLength = 0;

            if(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS){
                while(EOF != getline(&line, &len, chtopar)) outputLength += strlen(line);
            }

            free(line);

            if(outputLength > maxOutputLength){
                maxOutputLength = outputLength;
                strcpy(maxCmd, cmd);
                strcpy(maxArg, arg);
            }
        }
    }
    
    printf("%s %s", maxCmd, maxArg);

    return 0;
}