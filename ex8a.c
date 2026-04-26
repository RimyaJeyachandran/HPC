#include <stdio.h>
#include <omp.h>

int main() {
    int n = 10;
    int arr[10];

    // Initialize
    for(int i = 0; i < n; i++) {
        arr[i] = i * 2;
    }

    // Parallel processing with static scheduling
    // Each thread takes chunks of 2 elements
    #pragma omp parallel for schedule(static, 2)
    for(int i = 0; i < n; i++) {
        int tid = omp_get_thread_num();
        printf("Thread %d working on index %d\n", tid, i);
        arr[i] += 7;
    }

    printf("\nResulting Array:\n");
    for(int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}