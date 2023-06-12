#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#define check_error(expr, msg) \
    do { \
        if(!(expr)) { \
            perror(msg); \
            exit(EXIT_FAILURE); \
            } \
        } while(false)

#define pthread_check_error(pthread_err, msg)\
    do\
    {\
        int _pthread_err = (pthread_err);\
        if (_pthread_err > 0)\
        {\
            check_error(false, msg);\
            errno = _pthread_err;\
        }\
    } while (0)

#define UNUSED(x) ((void) x)

typedef struct{
    double localMax;
} THREAD_RET_VAL;

typedef struct {
    int first_thread_vector;
    int num_of_thread_vectors;
    int len_of_thread_vectors;
    double** vectors;
} THREAD_ARGS;

void* thread_func(void* arg){
    THREAD_ARGS* args = (THREAD_ARGS*) arg;
    THREAD_RET_VAL* ret = (THREAD_RET_VAL*) malloc(sizeof(THREAD_RET_VAL));
    check_error(ret != NULL, "THREAD_RET_VAL malloc");

    double currNorm;
    double maxNorm = -__DBL_MAX__;

    for(int i = args->first_thread_vector; i < args->first_thread_vector + args->num_of_thread_vectors; i++){
        currNorm = 0;
        for(int j = 0; j < args->len_of_thread_vectors; j++) currNorm += (args->vectors[i][j] * args->vectors[i][j]);

        currNorm = sqrt(currNorm);
        maxNorm = (currNorm > maxNorm) ? currNorm : maxNorm;
    }

    ret->localMax = maxNorm;
    return (void*) ret;
}

int main(int argc, char** argv){
    check_error(argc == 1, "argc");
    UNUSED(argv);

    int num_of_vectors, len_of_vectors, num_of_vectors_per_thread;
    scanf("%d %d %d", &num_of_vectors, &len_of_vectors, &num_of_vectors_per_thread);
    int num_of_threads = num_of_vectors / num_of_vectors_per_thread;

    double** vectors = (double**) malloc(num_of_vectors * sizeof(double*));
    check_error(vectors != NULL, "vectors malloc");

    for(int i = 0; i < num_of_vectors; i++){
        vectors[i] = (double*) malloc(len_of_vectors * sizeof(double));
        check_error (vectors[i] != NULL, "vectors[i] malloc");
        for(int j = 0; j < len_of_vectors; j++) scanf("%lf", &vectors[i][j]);
    }

    pthread_t* threads = (pthread_t*) malloc(num_of_threads * sizeof(pthread_t));
    check_error(threads != NULL, "threads malloc");

    THREAD_ARGS* args = (THREAD_ARGS*) malloc(num_of_threads * sizeof(THREAD_ARGS));
    check_error(args != NULL, "thread_args malloc");

    for(int i = 0; i < num_of_threads; i++){
        args[i].first_thread_vector = i * num_of_vectors_per_thread;
        args[i].len_of_thread_vectors = len_of_vectors;
        args[i].num_of_thread_vectors = num_of_vectors_per_thread;
        args[i].vectors = vectors;
        pthread_check_error(pthread_create(&threads[i], 0, &thread_func, &args[i]), "pthread_create");
    }

    double globalMax = - __DBL_MAX__;
    int thridx = 1;

    for(int i = 0; i < num_of_threads; i++){
        void* retval;
        pthread_check_error(pthread_join(threads[i], &retval), "pthread_join");
        THREAD_RET_VAL* thrret = (THREAD_RET_VAL*) retval;
        if(thrret->localMax > globalMax){
            globalMax = thrret->localMax;
            thridx = i;
        }
    }

    printf("%d %lf\n", thridx, globalMax);

    for(int i = 0; i < num_of_vectors; i++) free(vectors[i]);
    free(vectors);

    return 0;
}