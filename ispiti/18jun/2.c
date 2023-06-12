#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#define check_error(expr, msg) \
    do { \
        if(!(expr)){ \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(false)

#define thread_check_error(expr, msg) \
    do{ \
        int _expr = expr; \
        if(_expr > 0){ \
            errno = _expr; \
            check_error(false, msg); \
        } \
    } while(false)

#define ALPHABET (26)

typedef struct {
    pthread_mutex_t mutex;
    char maxChar;
    unsigned maxCharCount;
} LOCKED;

LOCKED data;

typedef struct {
    char* f;
    char c;
} THREAD_ARGS;

typedef struct
{
    unsigned cntr;
} THREAD_RET_VAL;


void* thread_func(void* arg){
    THREAD_ARGS* args = (THREAD_ARGS*) arg;

    FILE* f = fopen(args->f, "r");
    
    unsigned cntr = 0;
    char currChar = 'a';

    while(fscanf(f, "%c", &currChar) != EOF){
        if(currChar == tolower(args->c)) cntr++;
    }

    check_error(-1 != fclose(f), "thread close");

    if(cntr > data.maxCharCount){
        pthread_mutex_lock(&(data.mutex));
        data.maxChar = args->c;
        data.maxCharCount = cntr;
        pthread_mutex_unlock(&(data.mutex));
    }

    THREAD_RET_VAL* retval = (THREAD_RET_VAL*) malloc(sizeof(THREAD_RET_VAL));
    check_error(NULL != retval, "retval malloc");

    retval->cntr = cntr;

    return (void*) retval;
}

int main(int argc, char** argv){
    check_error(2 == argc, "argc");

    thread_check_error(pthread_mutex_init(&(data.mutex), NULL), "mutex init");

    pthread_t* threads = (pthread_t*) malloc(ALPHABET * sizeof(pthread_t));
    check_error(NULL != threads, "threads malloc");

    THREAD_ARGS* args = (THREAD_ARGS*) malloc(ALPHABET * sizeof(THREAD_ARGS));
    check_error(NULL != args, "args malloc");

    for (int i = 0; i < ALPHABET; i++) {
        args[i].c = 'a' + i;
        args[i].f = argv[1];

        thread_check_error(pthread_create(&threads[i], NULL, &thread_func, &args[i]), "pthread_create");
    }

    unsigned* counters = (unsigned*) malloc(ALPHABET * sizeof(unsigned));
    check_error(NULL != counters, "counters malloc");

    THREAD_RET_VAL* threadRet = NULL;
    void* threadRetVoid = NULL;

    data.maxChar = 'a';
    data.maxCharCount = 0;

    for (int i = 0; i < ALPHABET; i++) {
        thread_check_error(pthread_join(threads[i], threadRetVoid), "pthread_join");
        threadRet = (THREAD_RET_VAL*) threadRetVoid;

        counters[i] = threadRet->cntr;
        printf("%c ", counters[i]);
    }

    printf("\n%c\n", data.maxChar);

    thread_check_error(pthread_mutex_destroy(&(data.mutex)), "pthread_mutex_destroy");
    free(threads);
    free(args);
    free(counters);

    return 0;
}