#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>

#define check_error(cond, msg) \
    do { \
        if (!(cond)) { \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    }   while(0)

int main(int argc, char** argv){
    check_error(argc == 2, "argc");

    struct flock lock;
    lock.l_len = 0;
    lock.l_start = 0;
    lock.l_pid = getpid();
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;

    FILE* f = fopen(argv[1], "r+");
    check_error(NULL != f, "fopen");
    int fd = fileno(f);
    
    int fcntl_ret_val = fcntl(fd, F_SETLK, &lock);
    check_error(fcntl_ret_val != -1, "fcntl");
    
    lock.l_len = 0;
    lock.l_start = 0;
    lock.l_pid = getpid();
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;

    if(lock.l_type == F_WRLCK)  printf("F_WRLCK\n");
    if(lock.l_type == F_RDLCK)  printf("F_RDLCK\n");
    if(lock.l_type == F_UNLCK)  printf("F_UNLCK\n");

    while(1);

    return 0;
}