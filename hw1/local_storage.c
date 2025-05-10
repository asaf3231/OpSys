#include "local_storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>

tls_data_t g_tls[MAX_THREADS];
atomic_flag tls_lock = ATOMIC_FLAG_INIT;  // global spinlock

// Spinlock helpers
static void tls_lock_acquire() {
    while (atomic_flag_test_and_set(&tls_lock)) {
        // spin
    }
}

static void tls_lock_release() {
    atomic_flag_clear(&tls_lock);
}

void init_storage(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        g_tls[i].thread_id = -1;
        g_tls[i].data = NULL;
    }
}

void tls_thread_alloc(void) {
    pthread_t self_id = pthread_self();
    int64_t tid = (int64_t)(uintptr_t)self_id;
    int first_free_slot = -1;

    tls_lock_acquire();

    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            tls_lock_release();
            return; // already allocated
        }
        if (g_tls[i].thread_id == -1 && first_free_slot == -1) {
            first_free_slot = i;
        }
    }

    if (first_free_slot == -1) {
        tls_lock_release();
        printf("thread [%lu] failed to initialize, not enough space\n", (unsigned long)self_id);
        exit(1);
    }

    g_tls[first_free_slot].thread_id = tid;
    g_tls[first_free_slot].data = NULL;

    tls_lock_release();
}

void* get_tls_data(void) {
    pthread_t self_id = pthread_self();
    int64_t tid = (int64_t)(uintptr_t)self_id;

    tls_lock_acquire();

    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            void* result = g_tls[i].data;
            tls_lock_release();
            return result;
        }
    }

    tls_lock_release();
    printf("thread [%lu] hasn't been initialized in the TLS\n", (unsigned long)self_id);
    exit(2);
}

void set_tls_data(void* data) {
    pthread_t self_id = pthread_self();
    int64_t tid = (int64_t)(uintptr_t)self_id;

    tls_lock_acquire();

    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            g_tls[i].data = data;
            tls_lock_release();
            return;
        }
    }

    tls_lock_release();
    printf("thread [%lu] hasn't been initialized in the TLS\n", (unsigned long)self_id);
    exit(2);
}

void tls_thread_free(void) {
    pthread_t self_id = pthread_self();
    int64_t tid = (int64_t)(uintptr_t)self_id;

    tls_lock_acquire();

    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            g_tls[i].thread_id = -1;
            g_tls[i].data = NULL;
            break;
        }
    }

    tls_lock_release();
}