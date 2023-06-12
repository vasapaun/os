#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

#define check_error(expr, msg) \
    do { \
        if(!(expr)){ \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(false)

#define ARRAY_MAX (1024)

typedef struct {
    sem_t dataProcessingFinished;
    int array[ARRAY_MAX];
    unsigned arrayLen;
} osInputData;

int sig;

void signal_handler(int signum){
    if(signum == SIGUSR1)   sig = 1;
    else if(signum == SIGUSR2)    sig = 2;
}

void* get_mem_blk(const char* shmpath, size_t* size){
    int shm_fd = shm_open(shmpath, O_RDWR, 0);
    check_error(-1 != shm_fd, "shm_open");

    struct stat finfo;
    check_error(-1 != fstat(shm_fd, &finfo), "stat");

    *size = finfo.st_size;

    void* addr = mmap(NULL, sizeof(osInputData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    check_error(MAP_FAILED != addr, "mmap");

    check_error(-1 != close(shm_fd), "close");

    return addr;
}

int main(int argc, char** argv) {
    check_error(2 == argc, "argc");

    size_t size;
    osInputData* mem_blk = (osInputData*) get_mem_blk(argv[1], &size);

    sig = 0;
    do {
        check_error(SIG_ERR != signal(SIGUSR1, &signal_handler), "signal");
        check_error(SIG_ERR != signal(SIGUSR2, &signal_handler), "signal");
    } while (0 == sig);

    if(1 == sig)      for(size_t i = 0; i < mem_blk->arrayLen; i++) mem_blk->array[i] *= -1;
    else if(2 == sig) for(size_t i = 0; i < mem_blk->arrayLen; i++) mem_blk->array[i] *= 2;

    check_error(-1 != sem_post(&(mem_blk->dataProcessingFinished)), "sem_post");

    check_error(-1 != munmap(mem_blk, size), "munmap");

    return 0;
}