#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

#define check_error(expr, msg) \
    do{ \
        if(!(expr)){ \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

int main(int argc, char** argv){
    check_error(argc == 2, "argc");

    FILE* f = fopen(argv[1], "r+");
    check_error(f != NULL, "fopen");

    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_pid = getpid();
    lock.l_len = 2;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;

    int fd = fileno(f);
    int fcntl_ret_val = fcntl(fd, F_SETLK, &lock);
    check_error(-1 != fcntl_ret_val, "fcntl");
    if(lock.l_type == F_UNLCK)  printf("F_UNLCK");
    if(lock.l_type == F_WRLCK)  printf("F_WRLCK");
    if(lock.l_type == F_RDLCK)  printf("F_RDLCK");

    check_error(-1 != fclose(f), "fclose");

    return 0;
}