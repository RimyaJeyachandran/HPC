#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    int rows, cols, i, j;
    int pipe_fd[2];
    struct timespec start, end;
    double t_serial, t_parent, t_child;

    printf("Matrix Rows: ");
    scanf("%d", &rows);
    printf("Matrix Cols: ");
    scanf("%d", &cols);

    int M1[rows][cols], M2[rows][cols];
    int Sum[rows][cols], Diff[rows][cols];

    srand(time(NULL));

    // Initialize matrices with dummy data
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
            M1[i][j] = rand() % 10;
            M2[i][j] = rand() % 10;
        }
    }

    // Part 1: Sequential Logic
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
            Sum[i][j] = M1[i][j] + M2[i][j];
            Diff[i][j] = M1[i][j] - M2[i][j];
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    t_serial = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    // Part 2: Parallel Logic using Fork
    pipe(pipe_fd);
    pid_t worker = fork();

    if(worker == 0) {
        // Child handles Subtraction
        struct timespec c_start, c_end;
        close(pipe_fd[0]);
        
        clock_gettime(CLOCK_MONOTONIC, &c_start);
        for(i = 0; i < rows; i++)
            for(j = 0; j < cols; j++)
                Diff[i][j] = M1[i][j] - M2[i][j];
        clock_gettime(CLOCK_MONOTONIC, &c_end);

        t_child = (c_end.tv_sec - c_start.tv_sec) * 1000.0 + (c_end.tv_nsec - c_start.tv_nsec) / 1000000.0;
        write(pipe_fd[1], &t_child, sizeof(double));
        close(pipe_fd[1]);
        exit(0);
    } 
    else {
        // Parent handles Addition
        struct timespec p_start, p_end;
        close(pipe_fd[1]);

        clock_gettime(CLOCK_MONOTONIC, &p_start);
        for(i = 0; i < rows; i++)
            for(j = 0; j < cols; j++)
                Sum[i][j] = M1[i][j] + M2[i][j];
        clock_gettime(CLOCK_MONOTONIC, &p_end);

        t_parent = (p_end.tv_sec - p_start.tv_sec) * 1000.0 + (p_end.tv_nsec - p_start.tv_nsec) / 1000000.0;
        
        read(pipe_fd[0], &t_child, sizeof(double));
        close(pipe_fd[0]);
        wait(NULL);
    }

    double t_parallel = (t_parent > t_child) ? t_parent : t_child;

    printf("\nResults:\n");
    printf("Serial Completion:   %.4f ms\n", t_serial);
    printf("Parallel Completion: %.4f ms\n", t_parallel);

    return 0;
}