#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <float.h>
#include <stdbool.h>

#define check_error(expr, msg) \
    do{ \
        if(!(expr)){ \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#define thr_check_error(expr, msg) \
    do{ \
        int _expr = expr; \
        if(_expr > 0) { \
            errno = _expr; \
            check_error(false, msg); \
        } \
    } while(0)

typedef struct{
    int vek_len;
    int n_vek;
    int prvi_vektor;
    int posl_vektor;
    float** vektori;
} THREAD_ARGS;

typedef struct{
    float ret;
} THREAD_RET_VAL;

void* threadFunction(void* arg){
    THREAD_ARGS* args = (THREAD_ARGS*) arg;
    THREAD_RET_VAL* local_max = malloc(sizeof(THREAD_RET_VAL));
    check_error(local_max != NULL, "local max malloc");
    local_max->ret = FLT_MIN;

    float trenutni_proizvod = 0;
    for(int i = args->prvi_vektor; i < args->n_vek + args->prvi_vektor; i++){
        trenutni_proizvod = 0;
        for(int j = 0; j < args->vek_len; j++) trenutni_proizvod += args->vektori[i][j] * args->vektori[args->posl_vektor][j];
        local_max->ret = (trenutni_proizvod > local_max->ret) ? trenutni_proizvod : local_max->ret;
    }
    return (void*) local_max;
}

int main(){
    int n_vek, vek_len, n_niti;
    scanf("%d %d %d", &n_vek, &vek_len, &n_niti);
    
    float** vektori = (float **) malloc((n_vek+1) * sizeof(float *));
    check_error(vektori != NULL, "vektori malloc");

    for(int i = 0; i < n_vek + 1; i++){
        vektori[i] = (float*) malloc(vek_len * sizeof(float));
        check_error(vektori[i] != NULL, "vektor malloc");
        for(int j = 0; j < vek_len; j++)  scanf("%f", &vektori[i][j]);
    }

    pthread_t* threads = (pthread_t*) malloc(n_niti * sizeof(pthread_t));
    check_error(threads != NULL, "threads malloc");
    THREAD_ARGS* thr_args = (THREAD_ARGS*) malloc(n_niti * sizeof(THREAD_ARGS));
    check_error(thr_args != NULL, "thr args malloc");

    for(int i = 0; i < n_niti; i++){
        thr_args[i].vek_len = vek_len;  
        thr_args[i].n_vek = n_vek / n_niti;
        thr_args[i].prvi_vektor = i * thr_args[i].n_vek;
        thr_args[i].posl_vektor = n_vek;
        thr_args[i].vektori = vektori;
        thr_check_error(pthread_create(&threads[i], 0, &threadFunction, &thr_args[i]), "pthread create");    
    }

    float maxval = -FLT_MAX;
    int maxidx = -1;
    for(int i = 0; i < n_niti; i++){
        void* ret;
        thr_check_error(pthread_join(threads[i], &ret), "pthread join");
        THREAD_RET_VAL* retval = (THREAD_RET_VAL*) ret;
        if(retval->ret > maxval){
            maxval = retval->ret;
            maxidx = i;
        }
        free(retval);
    }

    //oslobadjanje zauzete memorije
    for(int i = 0; i < n_vek; i++)  free(vektori[i]);
    free(vektori);
    free(threads);
    free(thr_args);

    printf("%d %g\n", maxidx + 1, maxval);

    return 0;
}