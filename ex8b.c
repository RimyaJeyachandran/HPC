#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define VAL 5.0

void run_test(long N) {
    // Dynamic allocation
    double *data = malloc(N * sizeof(double));
    if (!data) return;

    // Fill array
    for (long i = 0; i < N; i++) {
        data[i] = (double)i;
    }

    // Benchmark the parallel loop
    double t1 = omp_get_wtime();

    #pragma omp parallel for schedule(static)
    for (long i = 0; i < N; i++) {
        data[i] += VAL;
    }

    double t2 = omp_get_wtime();

    // Console Report
    printf("--- N = %ld ---\n", N);
    printf("Execution Time: %f sec\n", t2 - t1);
    
    // Check specific points: start, mid, end
    long check_points[] = {0, N / 2, N - 1};
    printf("Idx\tBefore\tAdded\tAfter\n");
    for (int j = 0; j < 3; j++) {
        long p = check_points[j];
        printf("%ld\t%.1f\t%.1f\t%.1f\n", p, (double)p, VAL, data[p]);
    }
    printf("\n");

    free(data);
}

int main() {
    // Test various magnitudes
    long test_sizes[] = {10000, 1000000, 10000000};

    for (int i = 0; i < 3; i++) {
        run_test(test_sizes[i]);
    }

    return 0;
}