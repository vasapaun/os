#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define check_error(expr, msg) \
    do { \
        if(!(expr)){ \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(false)

#define RD_END (0)
#define WR_END (1)

#define MAX_LINE 256

int main(int argc, char** argv){
    check_error(3 <= argc, "argc");

    int pipefds[2];
    check_error(-1 != pipe(pipefds), "pipe");

    pid_t childpid = fork();
    check_error(childpid != -1, "fork");

    if (childpid == 0) { // u child proce   su smo
        check_error(-1 != close(pipefds[RD_END]), "child close");
        check_error(-1 != dup2(pipefds[WR_END], STDOUT_FILENO), "child dup2");
        check_error(-1 != execvp(argv[1], argv + 1), "child execlp");
    }

    else { // u parent procesu smo
        check_error(-1 != close(pipefds[WR_END]), "parent close");

        FILE* chtopar = fdopen(pipefds[RD_END], "r");
        check_error(NULL != chtopar, "parent fdopen");

        char* line = NULL;
        size_t len = 0;
        int lineCounter = 0;

        while (EOF != getline(&line, &len, chtopar))    lineCounter++;

        int status = 0;
        check_error(-1 != wait(&status), "parent wait");

        if(!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS){
            check_error(false, "Neuspeh");
        }

        free(line);
        check_error(-1 != fclose(chtopar), "parent fclose");

        printf("%d\n", lineCounter);
    }

    return 0;
}