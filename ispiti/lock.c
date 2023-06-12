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

#define MAX_WORD 50

bool is4digitPos(char* s){
    return strlen(s) == 4 && atoi(s) != 0;
}

int main(int argc, char** argv){
    check_error(2 == argc, "argc");

    FILE* infile = fopen(argv[1], "r+");
    check_error(NULL != infile, "infile fopen");
    int infilefd = fileno(infile);
    check_error(-1 != infilefd, "infilefd fileno");

    struct flock lock;

    char* word = malloc(MAX_WORD * sizeof(char));
    while (1 == fscanf(infile, "%s", word)) {
        if (!is4digitPos(word))    continue;

        lock.l_len = 4;
        lock.l_pid = getpid();
        lock.l_whence = SEEK_SET;
        lock.l_type = F_WRLCK;
        lock.l_start = ftell(infile) - 4;

        int fcntl_ret_val = fcntl(infilefd, F_GETLK, &lock);
        check_error(-1 != fcntl_ret_val, "fcntl infilefd check lock");

        if (F_UNLCK != lock.l_type) continue;

        //lockujemo
        lock.l_start = ftell(infile) - 4;
        check_error(-1 != fcntl(infilefd, F_SETLK, &lock), "fcntl infileds lock");

        //izmenimo sadrzaj broja
        fseek(infile, ftell(infile) - 4, SEEK_SET);
        fprintf(infile, "####");
    }

    fclose(infile);

    return 0;
}