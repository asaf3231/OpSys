#include <stdbool.h>
#include <stdio.h>
#include "rw_lock.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

rwlock g_lock;
volatile bool g_flag = false;

void *function_writer(void* a) {
    for (int i = 0; i < 100000; i++ ) {
        rwlock_acquire_write(&g_lock);
        printf("Writer thread %lu acquired lock\n", pthread_self());

        g_flag = true;
        sched_yield();

        if (g_flag == false) {
            printf("Bug!\n");
            exit(1);
        }
        sched_yield();
        g_flag = false;
        // leave write
        rwlock_release_write(&g_lock);
        printf("Writer thread %lu release lock\n", pthread_self());
    }
    return NULL;
}

void *function_reader(void* a) {
    for (int i = 0; i < 100000; i++ ) {
        rwlock_acquire_read(&g_lock);
        volatile bool cur = g_flag;
        sched_yield();
        if (cur != g_flag) {
            printf("Bug!\n");
            exit(1);
        }
        // leave write
        rwlock_release_read(&g_lock);

    }
    return NULL;
}


int main() {
    rwlock_init(&g_lock);
    pthread_t threads1[10];
    pthread_t threads2[10];

    for (int i = 0; i < 10; i++) {
        pthread_create(&threads1[i], NULL, &function_writer, NULL);
    }

    for (int i = 0; i < 10; i++) {
        pthread_create(&threads2[i], NULL, &function_reader, NULL);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(threads1[i], NULL);
    }
    
    for (int i = 0; i < 10; i++) {
        pthread_join(threads2[i], NULL);
    }

    printf("good\n");
    return 0;
}