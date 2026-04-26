#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define B_SIZE 5
#define W_LEN 50
#define WORKERS 3

// Input queue
char in_buf[B_SIZE][W_LEN];
int in_count = 0, in_h = 0, in_t = 0;
pthread_mutex_t in_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t in_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t in_empty = PTHREAD_COND_INITIALIZER;

// Output queue
char out_buf[B_SIZE][W_LEN];
int out_status[B_SIZE]; // 1=Good, 0=Bad
int out_count = 0, out_h = 0, out_t = 0;
pthread_mutex_t out_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t out_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t out_empty = PTHREAD_COND_INITIALIZER;

char dict[100][W_LEN];
int d_size = 0;
int stop_signal = 0;

void* checker_thread(void* arg) {
    while (1) {
        char word[W_LEN];

        // Fetch from input
        pthread_mutex_lock(&in_mtx);
        while (in_count == 0 && !stop_signal) {
            pthread_cond_wait(&in_empty, &in_mtx);
        }

        if (in_count == 0 && stop_signal) {
            pthread_mutex_unlock(&in_mtx);
            break;
        }

        strcpy(word, in_buf[in_h]);
        in_h = (in_h + 1) % B_SIZE;
        in_count--;
        pthread_cond_signal(&in_full);
        pthread_mutex_unlock(&in_mtx);

        // Logic: Search dictionary
        int found = 0;
        for (int i = 0; i < d_size; i++) {
            if (strcmp(word, dict[i]) == 0) {
                found = 1;
                break;
            }
        }

        // Push to output
        pthread_mutex_lock(&out_mtx);
        while (out_count == B_SIZE) {
            pthread_cond_wait(&out_full, &out_mtx);
        }
        strcpy(out_buf[out_t], word);
        out_status[out_t] = found;
        out_t = (out_t + 1) % B_SIZE;
        out_count++;
        pthread_cond_signal(&out_empty);
        pthread_mutex_unlock(&out_mtx);
    }
    return NULL;
}

int main() {
    int n_check;

    printf("Dict size: ");
    scanf("%d", &d_size);
    for (int i = 0; i < d_size; i++) {
        printf("Word %d: ", i + 1);
        scanf("%s", dict[i]);
    }

    printf("\nHow many words to check? ");
    scanf("%d", &n_check);
    char words[n_check][W_LEN];
    for (int i = 0; i < n_check; i++) {
        printf("Check %d: ", i + 1);
        scanf("%s", words[i]);
    }

    pthread_t th[WORKERS];
    for (int i = 0; i < WORKERS; i++)
        pthread_create(&th[i], NULL, checker_thread, NULL);

    // Producer: Main thread fills input queue
    for (int i = 0; i < n_check; i++) {
        pthread_mutex_lock(&in_mtx);
        while (in_count == B_SIZE) {
            pthread_cond_wait(&in_full, &in_mtx);
        }
        strcpy(in_buf[in_t], words[i]);
        in_t = (in_t + 1) % B_SIZE;
        in_count++;
        pthread_cond_signal(&in_empty);
        pthread_mutex_unlock(&in_mtx);
    }

    // Done producing
    pthread_mutex_lock(&in_mtx);
    stop_signal = 1;
    pthread_cond_broadcast(&in_empty);
    pthread_mutex_unlock(&in_mtx);

    // Consumer: Display results
    for (int i = 0; i < n_check; i++) {
        pthread_mutex_lock(&out_mtx);
        while (out_count == 0) {
            pthread_cond_wait(&out_empty, &out_mtx);
        }
        printf("Word: %s -> %s\n", out_buf[out_h], out_status[out_h] ? "VALID" : "INVALID");
        out_h = (out_h + 1) % B_SIZE;
        out_count--;
        pthread_cond_signal(&out_full);
        pthread_mutex_unlock(&out_mtx);
    }

    for (int i = 0; i < WORKERS; i++) pthread_join(th[i], NULL);
    printf("\nFinished processing.\n");

    return 0;
}