#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

#define check_error(cond, msg)\
    do\
    {\
        if (!(cond))\
        {\
            perror(msg);\
            fprintf(stderr, "file: %s\nfunc: %s\nline: %d\n", __FILE__, __func__, __LINE__);\
            exit(EXIT_FAILURE);\
        }\
    } while (0)

#define pthread_check_error(pthread_err, msg)\
    do\
    {\
        int _pthread_err = pthread_err;\
        if (_pthread_err > 0)\
        {\
            check_error(false, msg);\
            errno = _pthread_err;\
        }\
    } while (0)
    

#define UNUSED(x) ((void)x)

typedef struct 
{
    float **vectors;
    int first_vector;
    int last_vector;
    int num_of_vectors;
    int n;
} THREAD_ARGS;

typedef struct
{
    float local_max;
} THREAD_RETURN_VAL;

void *thread_func(void *args);

int main(int argc, char **argv)
{
    UNUSED(argv);
    check_error(1 == argc, "no arguments needed"); 

    int m, n, k;
    scanf("%d%d%d", &m, &n, &k);

    float **vectors = (float **)malloc((m + 1) * sizeof(float *));
    check_error(NULL != vectors, "malloc");
    for (int i = 0; i < m + 1; i++)
    {
        vectors[i] = (float *)malloc(n * sizeof(float));
        check_error(NULL != vectors[i], "malloc");

        for (int j = 0; j < n; j++)
        {
            scanf("%f", &vectors[i][j]);
        }
    }

    pthread_t *thread_ids = (pthread_t *)malloc(k * sizeof(pthread_t));
    check_error(NULL != thread_ids, "malloc");

    THREAD_ARGS *args = (THREAD_ARGS *)malloc(k * sizeof(THREAD_ARGS));
    check_error(NULL != args, "malloc");

    // kreiranje niti
    for (int i = 0; i < k; i++)
    {
        args[i].first_vector = i * m / k;
        args[i].last_vector = m;
        args[i].num_of_vectors = m / k;
        args[i].vectors = vectors;
        args[i].n = n;
        pthread_check_error(pthread_create(&thread_ids[i], 0, &thread_func, &args[i]), "pthread_create");
    }
    
    // racunanje globalnog maksimuma prikupljanjem povratnih vrednosti niti
    float global_max = -FLT_MAX;
    int index = -1;
    for (int i = 0; i < k; i++)
    {
        void *ret_val;
        pthread_check_error(pthread_join(thread_ids[i], &ret_val), "pthread_join");
        
        THREAD_RETURN_VAL *ret = (THREAD_RETURN_VAL *)ret_val;
        if (global_max < ret->local_max)
        {
            global_max = ret->local_max;
            index = i;
        }
        free(ret_val);
    }
    
    printf("%d %g\n", index + 1, global_max);

    // oslobadjanje zauzete memorije
    for (int i = 0; i < m + 1; i++)
    {
        free(vectors[i]);
    }
    free(vectors);
    free(thread_ids);
    free(args);
    
    return 0;
}

void *thread_func(void *args)
{
    THREAD_ARGS *arg = (THREAD_ARGS *)args;
    
    THREAD_RETURN_VAL *ret = (THREAD_RETURN_VAL *)malloc(sizeof(THREAD_RETURN_VAL));
    check_error(NULL != ret, "malloc");

    ret->local_max = -FLT_MAX;
    for (int i = 0; i < arg->num_of_vectors; i++)
    {
        float s = 0.0;
        // racunanje skalarnog proizvoda
        for (int j = 0; j < arg->n; j++)
        {
            s += arg->vectors[arg->first_vector + i][j] * arg->vectors[arg->last_vector][j];
        }
    
        ret->local_max = (ret->local_max < s)? s : ret->local_max;
    }
    
    return (void *)ret;
}