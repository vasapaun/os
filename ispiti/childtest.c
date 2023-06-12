#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define check_error(expr, msg) \
    do { \
        if(!(expr)) { \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

int main(){
    int pipefds[2];
    check_error(-1 != pipe(pipefds), "pipe");

    pid_t child = fork();
    check_error(child != -1, "fork");
    
    if (child == 0) { // u child procesu smo
        check_error(-1 != close(pipefds[0]), "close");

        FILE* chtopar = fdopen(pipefds[1], "w");
        check_error(chtopar != NULL, "fdopen");

        fprintf(chtopar, "Poruka!");
        exit(EXIT_SUCCESS);
    }
    else { // u parent procesu smo
        check_error(-1 != close(pipefds[1]), "close");

        FILE* chtopar = fdopen(pipefds[0], "r");
        check_error(chtopar != NULL, "fdopen chtopar");

        char word[256];
        fscanf(chtopar, "%s", word);
        printf("%s\n", word);

        wait(NULL);
    }

    return 0;
}