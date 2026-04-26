#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define B_SIZE 20
#define MAX_LEN 100

char shared_buffer[B_SIZE][MAX_LEN];
int buffer_count = 0;
int active_producers; 

omp_lock_t buffer_lock;

int main() {
    int num_prods = 2;
    int num_cons = 2;
    active_producers = num_prods;

    omp_init_lock(&buffer_lock);

    #pragma omp parallel num_threads(num_prods + num_cons)
    {
        int tid = omp_get_thread_num();

        // Producer Logic
        if (tid < num_prods) {
            char filename[30];
            sprintf(filename, "file%d.txt", tid + 1);
            FILE *f = fopen(filename, "r");

            if (!f) {
                printf("Error: P%d couldn't open %s\n", tid, filename);
            } else {
                char line[MAX_LEN];
                while (fgets(line, MAX_LEN, f)) {
                    int stored = 0;
                    while (!stored) {
                        omp_set_lock(&buffer_lock);
                        if (buffer_count < B_SIZE) {
                            strcpy(shared_buffer[buffer_count], line);
                            buffer_count++;
                            stored = 1;
                        }
                        omp_unset_lock(&buffer_lock);
                    }
                }
                fclose(f);
            }
            
            // Signal that one producer is finished
            #pragma omp atomic
            active_producers--;
        }
        // Consumer Logic
        else {
            // Keep running as long as producers are active or buffer has data
            while (active_producers > 0 || buffer_count > 0) {
                char local_line[MAX_LEN] = "";
                int got_data = 0;

                omp_set_lock(&buffer_lock);
                if (buffer_count > 0) {
                    buffer_count--;
                    strcpy(local_line, shared_buffer[buffer_count]);
                    got_data = 1;
                }
                omp_unset_lock(&buffer_lock);

                if (got_data) {
                    // Tokenize the line into words
                    char *token = strtok(local_line, " \t\n");
                    while (token != NULL) {
                        printf("Consumer %d: %s\n", tid, token);
                        token = strtok(NULL, " \t\n");
                    }
                }
            }
        }
    }

    omp_destroy_lock(&buffer_lock);
    return 0;
}