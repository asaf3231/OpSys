#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "tl_semaphore.h"

#define NUM_THREADS 5
#define NUM_ITERATIONS 1000

semaphore sem;
int shared_counter = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        semaphore_wait(&sem);
        
        // Critical Section: Only one thread can modify shared_counter here
        shared_counter++;
        
        semaphore_signal(&sem);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    semaphore_init(&sem, 1); // Semaphore initial value 1 => acts like a mutex

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Expected counter: %d\n", NUM_THREADS * NUM_ITERATIONS);
    printf("Actual counter: %d\n", shared_counter);

    if (shared_counter == NUM_THREADS * NUM_ITERATIONS) {
        printf("✅ Semaphore test passed!\n");
    } else {
        printf("❌ Semaphore test failed!\n");
    }

    return 0;
}