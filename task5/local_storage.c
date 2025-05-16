/*
 * local_storage.c
 * ----------------
 * Thread-local storage (TLS) implementation using a global array and a spinlock.
 * Each thread can allocate, set, get, and free its own storage slot.
 *
 * Functions:
 *   - init_storage: Initialize the TLS array.
 *   - tls_thread_alloc: Allocate a TLS slot for the calling thread.
 *   - get_tls_data: Retrieve the TLS data pointer for the calling thread.
 *   - set_tls_data: Set the TLS data pointer for the calling thread.
 *   - tls_thread_free: Free the TLS slot for the calling thread.
 *
 * Synchronization is provided by a global spinlock (atomic_flag).
 *
 * Author: Noam Hasson, Asaf Ramati
 */

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

// Initialize all TLS slots to unused
void init_storage(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        g_tls[i].thread_id = -1;
        g_tls[i].data = NULL;
    }
}

// Allocate a TLS slot for the calling thread
void tls_thread_alloc(void) {
    pthread_t self_id = pthread_self();
    int64_t tid = (int64_t)(uintptr_t)self_id;
    int first_free_slot = -1;

    tls_lock_acquire();

    // Search for existing slot or first free slot
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            tls_lock_release();
            return; // already allocated
        }
        if (g_tls[i].thread_id == -1 && first_free_slot == -1) {
            first_free_slot = i;
        }
    }

    // No free slot found
    if (first_free_slot == -1) {
        tls_lock_release();
        printf("thread [%lu] failed to initialize, not enough space\n", (unsigned long)self_id);
        exit(1);
    }

    // Allocate slot
    g_tls[first_free_slot].thread_id = tid;
    g_tls[first_free_slot].data = NULL;

    tls_lock_release();
}

// Get the TLS data pointer for the calling thread
void* get_tls_data(void) {
    pthread_t self_id = pthread_self();
    int64_t tid = (int64_t)(uintptr_t)self_id;

    tls_lock_acquire();

    // Search for the thread's slot
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

// Set the TLS data pointer for the calling thread
void set_tls_data(void* data) {
    pthread_t self_id = pthread_self();
    int64_t tid = (int64_t)(uintptr_t)self_id;

    tls_lock_acquire();

    // Search for the thread's slot
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

// Free the TLS slot for the calling thread
void tls_thread_free(void) {
    pthread_t self_id = pthread_self();
    int64_t tid = (int64_t)(uintptr_t)self_id;

    tls_lock_acquire();

    // Search for the thread's slot and free it
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            g_tls[i].thread_id = -1;
            g_tls[i].data = NULL;
            break;
        }
    }

    tls_lock_release();
}