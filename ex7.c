#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define B_SIZE 5
#define LIMIT 10

typedef struct {
    int x, y;
    char symbol;
} CalcTask;

CalcTask ring_buffer[B_SIZE];
int head = 0, tail = 0;
int is_finished = 0;

sem_t slots, items, mtx;

// Helper to pick a random math operator
char rand_op() {
    char list[] = {'+', '-', '*', '/'};
    return list[rand() % 4];
}

// Producer: Creates math problems
void *producer_fn(void *arg) {
    for (int i = 0; i < LIMIT; i++) {
        CalcTask job;
        job.x = rand() % 50 + 1;
        job.y = rand() % 50 + 1;
        job.symbol = rand_op();

        sem_wait(&slots);
        sem_wait(&mtx);

        ring_buffer[head] = job;
        printf("[PROD] Added: %d %c %d\n", job.x, job.symbol, job.y);
        head = (head + 1) % B_SIZE;

        sem_post(&mtx);
        sem_post(&items);

        usleep(500000); // Sleep 0.5s
    }
    is_finished = 1;
    return NULL;
}

// Worker: Each thread handles one operator type
void *worker_fn(void *arg) {
    char target = *(char *)arg;

    while (1) {
        if (is_finished && head == tail) break;

        sem_wait(&items);
        sem_wait(&mtx);

        CalcTask current = ring_buffer[tail];

        if (current.symbol == target) {
            tail = (tail + 1) % B_SIZE;
            sem_post(&mtx);
            sem_post(&slots);

            int out = 0;
            if (target == '+') out = current.x + current.y;
            else if (target == '-') out = current.x - current.y;
            else if (target == '*') out = current.x * current.y;
            else if (target == '/') out = (current.y != 0) ? current.x / current.y : 0;

            printf("Thread [%c]: %d %c %d = %d\n", target, current.x, target, current.y, out);
        } 
        else {
            // Task doesn't belong to this thread, put it back (post items)
            sem_post(&mtx);
            sem_post(&items);
            usleep(10000); // Tiny pause to let other threads try
        }
    }
    return NULL;
}

int main() {
    pthread_t p_thread, w_threads[4];
    char ops[] = {'+', '-', '*', '/'};

    sem_init(&slots, 0, B_SIZE);
    sem_init(&items, 0, 0);
    sem_init(&mtx, 0, 1);

    srand(time(NULL));

    pthread_create(&p_thread, NULL, producer_fn, NULL);
    for (int i = 0; i < 4; i++) {
        pthread_create(&w_threads[i], NULL, worker_fn, &ops[i]);
    }

    pthread_join(p_thread, NULL);
    
    // Give workers a moment to finish remaining tasks
    sleep(2);
    
    printf("\n--- Task Processing Finished ---\n");
    return 0;
}