#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>

#define check_error(expr, msg) \
    do { \
        if (!(expr)){ \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(false)

#define pthread_check_error(expr, msg) \
    do { \
        int _expr = expr; \
        if (_expr > 0){ \
            errno = _expr; \
            check_error(false, msg); \
        } \
    } while(false)

#define UNUSED(x) ((void) x)

double global_sum = 0;
pthread_mutex_t mutex;

typedef struct{
    double** vectors;
    int thread_vector_pos;
    int thread_vector_len;
    double thread_exp;
} THREAD_ARGS;

void* thread_func(void* args){
    THREAD_ARGS* arg = (THREAD_ARGS*) args;

    double local_sum = 0;
    int pos = arg->thread_vector_pos;
    int len = arg->thread_vector_len;
    double exp = arg->thread_exp;

    for(int i = 0; i < len; i++)    local_sum += pow(arg->vectors[pos][i], exp);

    pthread_mutex_lock(&mutex);
    global_sum += local_sum;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char** argv){
    check_error(argc == 1, "argc");
    UNUSED(argv);

    pthread_check_error(pthread_mutex_init(&mutex, NULL), "mutex init");

    double p;
    int vectors_width, vectors_height;
    scanf("%lf %d %d", &p, &vectors_width, &vectors_height);

    double** vectors = (double**) malloc(vectors_height * sizeof(double*));
    check_error(vectors != NULL, "vectors malloc");

    for(int i = 0; i < vectors_height; i++){
        vectors[i] = (double*) malloc(vectors_width * sizeof(double));
        check_error(vectors[i] != NULL, "vector malloc");
        for(int j = 0; j < vectors_width; j++)  scanf("%lf", &vectors[i][j]);
    }

    pthread_t* threads = (pthread_t*) malloc(vectors_height * sizeof(pthread_t));
    check_error(threads != NULL, "threads malloc");

    THREAD_ARGS* args = (THREAD_ARGS*) malloc(vectors_height * sizeof(THREAD_ARGS));
    check_error(args != NULL, "args malloc");

    for(int i = 0; i < vectors_height; i++){
        args[i].thread_exp = p;
        args[i].thread_vector_len = vectors_width;
        args[i].thread_vector_pos = i;
        args[i].vectors = vectors;
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }

    for(int i = 0; i < vectors_height; i++){
        pthread_join(threads[i], NULL);
        free(vectors[i]);
    }
    
    free(threads);
    free(args);

    pthread_mutex_destroy(&mutex);

    printf("%lf\n", pow(global_sum, 1.0/p));

    return 0;
}