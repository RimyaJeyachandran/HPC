#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

int main() {
    int n_size;
    printf("Enter matrix dimension (N): ");
    if (scanf("%d", &n_size) != 1) return 1;

    // Use descriptive names for the iterators
    int r, c, k;

    // Memory allocation for the matrices
    int **matA = malloc(n_size * sizeof(int *));
    int **matB = malloc(n_size * sizeof(int *));
    int **matC = malloc(n_size * sizeof(int *));
    int (*piping)[2] = malloc(n_size * sizeof(int[2]));

    for (r = 0; r < n_size; r++) {
        matA[r] = malloc(n_size * sizeof(int));
        matB[r] = malloc(n_size * sizeof(int));
        matC[r] = malloc(n_size * sizeof(int));
        pipe(piping[r]); // Initialize pipes here
    }

    // Fill Matrix A and B with random values (0-9)
    for (r = 0; r < n_size; r++) {
        for (c = 0; c < n_size; c++) {
            matA[r][c] = rand() % 10;
            matB[r][c] = rand() % 10;
        }
    }

    // Parallel processing: One child per row
    for (r = 0; r < n_size; r++) {
        pid_t p = fork();
        if (p == 0) {
            struct timeval t_start, t_end;
            int *row_res = malloc(n_size * sizeof(int));
            
            gettimeofday(&t_start, NULL);
            for (c = 0; c < n_size; c++) {
                row_res[c] = 0;
                for (k = 0; k < n_size; k++) {
                    row_res[c] += matA[r][k] * matB[k][c];
                }
            }
            gettimeofday(&t_end, NULL);

            double duration = (t_end.tv_sec - t_start.tv_sec) + 
                             (t_end.tv_usec - t_start.tv_usec) / 1e6;

            close(piping[r][0]);
            write(piping[r][1], row_res, n_size * sizeof(int));
            write(piping[r][1], &duration, sizeof(double));
            close(piping[r][1]);
            
            free(row_res);
            exit(0);
        }
    }

    // Parent collects results
    double max_parallel_time = 0;
    for (r = 0; r < n_size; r++) {
        int *temp_row = malloc(n_size * sizeof(int));
        double c_time;

        close(piping[r][1]);
        read(piping[r][0], temp_row, n_size * sizeof(int));
        read(piping[r][0], &c_time, sizeof(double));
        
        for (c = 0; c < n_size; c++) matC[r][c] = temp_row[c];
        if (c_time > max_parallel_time) max_parallel_time = c_time;
        
        close(piping[r][0]);
        free(temp_row);
    }

    // Standard Serial Calculation for comparison
    struct timeval s1, s2;
    gettimeofday(&s1, NULL);
    for (r = 0; r < n_size; r++) {
        for (c = 0; c < n_size; c++) {
            int sum = 0;
            for (k = 0; k < n_size; k++) sum += matA[r][k] * matB[k][c];
        }
    }
    gettimeofday(&s2, NULL);
    double serial_time = (s2.tv_sec - s1.tv_sec) + (s2.tv_usec - s1.tv_usec) / 1e6;

    printf("\n--- Timing Results ---\n");
    printf("Parallel Time: %.3f ms\n", max_parallel_time * 1000);
    printf("Serial Time:   %.3f ms\n", serial_time * 1000);

    return 0;
}