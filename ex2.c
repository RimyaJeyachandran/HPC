#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>

#define BUFFER_SIZE 100
#define TAG_ODD 1
#define TAG_EVEN 2

// Simple palindrome check: compares start and end characters
int check_palindrome(char *str) {
    int start = 0;
    int end = strlen(str) - 1;

    while (start < end) {
        if (str[start++] != str[end--]) 
            return 0;
    }
    return 1;
}

int main(int argc, char** argv) {
    int rank, size;
    char buffer[BUFFER_SIZE];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank != 0) {
        // Workers: Odd ranks send TAG_ODD, Evens send TAG_EVEN
        int tag = (rank % 2 != 0) ? TAG_ODD : TAG_EVEN;
        const char* label = (rank % 2 != 0) ? "OddRank" : "EvenRank";
        
        sprintf(buffer, "%s%d", label, rank);
        
        MPI_Send(buffer, strlen(buffer) + 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
        printf("[Process %d] Sent: %s (Tag: %d)\n", rank, buffer, tag);

    } else {
        // Master (Rank 0) logic
        MPI_Status status;
        printf("Master starting listener for %d processes...\n", size - 1);

        for (int i = 1; i < size; i++) {
            MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            int source = status.MPI_SOURCE;

            if (status.MPI_TAG == TAG_ODD) {
                // Task for odd ranks: Convert to uppercase
                for (int j = 0; buffer[j]; j++) {
                    buffer[j] = toupper((unsigned char)buffer[j]);
                }
                printf("From Rank %d (ODD): Upper -> %s\n", source, buffer);
            } 
            else if (status.MPI_TAG == TAG_EVEN) {
                // Task for even ranks: Palindrome check
                if (check_palindrome(buffer)) {
                    printf("From Rank %d (EVEN): '%s' is a palindrome.\n", source, buffer);
                } else {
                    printf("From Rank %d (EVEN): '%s' is not a palindrome.\n", source, buffer);
                }
            }
        }
        printf("Master: All messages processed.\n");
    }

    MPI_Finalize();
    return 0;
}