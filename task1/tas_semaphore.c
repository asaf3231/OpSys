/*
 * tas_semaphore.c
 *
 * Implementation of a semaphore using the Test-and-Set (TAS) mechanism for thread synchronization.
 *
 * This file provides a semaphore primitive that uses atomic operations to ensure mutual exclusion
 * and synchronization among threads. The semaphore is initialized with a given value, and supports
 * wait (P) and signal (V) operations.
 *
 * Author: Noam Hasson, Asaf Ramati
 */

#include "tas_semaphore.h"
#include <sched.h>

/*
 * semaphore_init
 *
 * Initializes the TAS semaphore with the specified initial value.
 * Sets up the value and lock fields.
 */
void semaphore_init(semaphore* sem, int initial_value) {
    atomic_init(&sem->value, initial_value);
    atomic_flag_clear(&sem->lock);
}

/*
 * semaphore_wait
 *
 * Wait (P) operation for the TAS semaphore.
 * Spins until the lock is acquired, then waits for the value to be positive.
 * Once allowed, the thread decrements the semaphore value and releases the lock.
 */
void semaphore_wait(semaphore* sem) {
    while (1) {
        while (atomic_flag_test_and_set(&sem->lock)) {
            sched_yield();
        }
        if (atomic_load(&sem->value) > 0) {
            atomic_fetch_sub(&sem->value, 1);
            atomic_flag_clear(&sem->lock);
            break;
        }
        atomic_flag_clear(&sem->lock);
        sched_yield();
    }
}

/*
 * semaphore_signal
 *
 * Signal (V) operation for the TAS semaphore.
 * Increments the semaphore value, potentially allowing another waiting thread to proceed.
 */
void semaphore_signal(semaphore* sem) {
    while (atomic_flag_test_and_set(&sem->lock)) {
        sched_yield();
    }
    atomic_fetch_add(&sem->value, 1);
    atomic_flag_clear(&sem->lock);
}
