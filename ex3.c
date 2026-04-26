#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUF_SIZE 100

// Helper to flip to upper
void make_upper(char *s) {
    while (*s) {
        *s = toupper((unsigned char)*s);
        s++;
    }
}

// Logic for checking palindrome
int check_pal(char *s) {
    int len = strlen(s);
    for (int i = 0; i < len / 2; i++) {
        if (s[i] != s[len - 1 - i]) return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    int rank, sz;
    char word_list[][BUF_SIZE] = {"madam", "hello", "level", "world", "radar"};
    char buffer[BUF_SIZE];
    
    MPI_Status stat;
    double t1, t2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &sz);

    t1 = MPI_Wtime();

    if (rank != 0) {
        // Each process picks a word and sends it based on parity
        char my_word[BUF_SIZE];
        strcpy(my_word, word_list[rank % 5]);

        int my_tag = (rank % 2 != 0) ? 1 : 2;
        MPI_Send(my_word, strlen(my_word) + 1, MPI_CHAR, 0, my_tag, MPI_COMM_WORLD);

        t2 = MPI_Wtime();
        printf("P%d finished in %f sec\n", rank, t2 - t1);
    } 
    else {
        // Master process loop
        for (int i = 1; i < sz; i++) {
            MPI_Recv(buffer, BUF_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            
            int src = stat.MPI_SOURCE;
            int type = stat.MPI_TAG;

            printf("\nMaster: Got msg from P%d (Tag %d)\n", src, type);
            
            if (type == 1) {
                make_upper(buffer);
                printf("Result (Upper): %s\n", buffer);
            } 
            else {
                if (check_pal(buffer))
                    printf("Result: '%s' is a palindrome\n", buffer);
                else
                    printf("Result: '%s' is not a palindrome\n", buffer);
            }
        }
        t2 = MPI_Wtime();
        printf("\nTotal Master time: %f sec\n", t2 - t1);
    }

    MPI_Finalize();
    return 0;
}