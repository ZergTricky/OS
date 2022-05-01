#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "pthread.h"
#include "time.h"

int min(int a, int b) {
    return (a < b ? a : b);
}

typedef struct __args {
    int *values;
    int l, r;
    int *result;
} args_t;

void *routine(void *_args) {
    args_t *args = (args_t *) _args;
    int *values = args->values;
    int l = args->l, r = args->r;
    int *res = args->result;

    int flag = 1;
    for (int i = l; i < r; ++i) {
        if (flag) {
            *res = values[i];
            flag = 0;
        } else {
            *res = min(*res, values[i]);
        }
    }

    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Not enough values\n");
        exit(1);
    }

    const int ARRAY_SIZE = atoi(argv[1]);
    const int THREAD_NUMBER = min(ARRAY_SIZE, atoi(argv[2]));
    const int BLOCK_SIZE = ARRAY_SIZE / THREAD_NUMBER;

    int *values = (int *) calloc(ARRAY_SIZE, sizeof(int));

    pthread_t *threads = (pthread_t *) calloc(THREAD_NUMBER, sizeof(pthread_t));
    args_t *args = (args_t *) calloc(THREAD_NUMBER, sizeof(args_t));

    for (int i = 0; i < ARRAY_SIZE; ++i) {
        scanf("%d", &values[i]);
    }
    for (int i = 0; i < THREAD_NUMBER; ++i) {
        args[i].result = (int *) calloc(1, sizeof(int));
        args[i].values = values;
        args[i].l = i * BLOCK_SIZE;
        if (i == THREAD_NUMBER - 1) {
            args[i].r = ARRAY_SIZE;
        } else {
            args[i].r = BLOCK_SIZE * (i + 1);
        }


        pthread_create(&threads[i], NULL, routine, (void *) &args[i]);
    }

    int result = -1;
    int flag = 1;
    for (int i = 0; i < THREAD_NUMBER; ++i) {
        pthread_join(threads[i], NULL);
        if (flag) {
            result = *(args[i].result);
            flag = 0;
        } else {
            result = min(result, *(args[i].result));
        }
    }
    printf("Result is: %d\n", result);

    free(values);
    free(threads);
    free(args);

    return 0;
}