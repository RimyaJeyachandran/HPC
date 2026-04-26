#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Global matrices
int **matA, **matB, **res_add, **res_mul;
int N;

// Helper for timing in milliseconds
double calc_time(struct timespec s, struct timespec e) {
    return (e.tv_sec - s.tv_sec) * 1000.0 + (e.tv_nsec - s.tv_nsec) / 1000000.0;
}

void* do_addition(void* arg) {
    struct timespec ts, te;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            res_add[i][j] = matA[i][j] + matB[i][j];
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &te);
    double *res = malloc(sizeof(double));
    *res = calc_time(ts, te);
    printf(">> Addition thread finished.\n");
    return (void*)res;
}

void* do_multiplication(void* arg) {
    struct timespec ts, te;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            res_mul[i][j] = 0;
            for (int k = 0; k < N; k++) {
                res_mul[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &te);
    double *res = malloc(sizeof(double));
    *res = calc_time(ts, te);
    printf(">> Multiplication thread finished.\n");
    return (void*)res;
}

void print_matrix(char *label, int **m) {
    printf("\n--- %s ---\n", label);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) printf("%d\t", m[i][j]);
        printf("\n");
    }
}

int main() {
    pthread_t t1, t2;
    double *elapsed1, *elapsed2;

    printf("Dimension (N): ");
    if(scanf("%d", &N) != 1) return 1;

    srand(time(NULL));

    // Allocation block
    matA = malloc(N * sizeof(int*));
    matB = malloc(N * sizeof(int*));
    res_add = malloc(N * sizeof(int*));
    res_mul = malloc(N * sizeof(int*));

    for (int i = 0; i < N; i++) {
        matA[i] = malloc(N * sizeof(int));
        matB[i] = malloc(N * sizeof(int));
        res_add[i] = malloc(N * sizeof(int));
        res_mul[i] = malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) {
            matA[i][j] = rand() % 10;
            matB[i][j] = rand() % 10;
        }
    }

    // Launch threads
    pthread_create(&t1, NULL, do_addition, NULL);
    pthread_create(&t2, NULL, do_multiplication, NULL);

    // Sync and get times
    pthread_join(t1, (void**)&elapsed1);
    pthread_join(t2, (void**)&elapsed2);

    printf("\n[Stats] Add: %.4f ms | Mul: %.4f ms\n", *elapsed1, *elapsed2);

    if (N <= 5) {
        print_matrix("Matrix A", matA);
        print_matrix("Matrix B", matB);
        print_matrix("Sum", res_add);
        print_matrix("Product", res_mul);
    } else {
        printf("\nMatrix too large to print.\n");
    }

    // Cleanup
    for (int i = 0; i < N; i++) {
        free(matA[i]); free(matB[i]); free(res_add[i]); free(res_mul[i]);
    }
    free(matA); free(matB); free(res_add); free(res_mul);
    free(elapsed1); free(elapsed2);

    return 0;
}