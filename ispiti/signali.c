#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define check_error(expr, msg) \
    do{ \
        if(!(expr)){ \
        perror(msg); \
        exit(EXIT_FAILURE); \
        } \
    } while(0)

#define UNUSED(x) ((void) x)


int sigstate, num;
void signal_handler(int signum){
    switch (signum)
    {
    case SIGUSR1:
        sigstate = SIGUSR1;
        break;
    case SIGUSR2:
        sigstate = SIGUSR2;
        break;
    case SIGTERM:
        exit(EXIT_SUCCESS);
    }

    scanf("%d", &num);

    if(sigstate == SIGUSR1) printf("%d\n", abs(num));
    else    printf("%d\n", num * num);
}

int main(int argc, char** argv){
    UNUSED(argv);
    UNUSED(argc);

    while(1){
        check_error(SIG_ERR != signal(SIGUSR1, &signal_handler), "SIGUSR1");
        check_error(SIG_ERR != signal(SIGUSR2, &signal_handler), "SIGUSR2");
        check_error(SIG_ERR != signal(SIGTERM, &signal_handler), "SIGTERM");
    }

    return 0;
}