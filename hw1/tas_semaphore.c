#include "tas_semaphore.h"

/*
 * TODO: Implement semaphore_init using a TAS spinlock.
 */
void semaphore_init(semaphore* sem, int initial_value) {
    // TODO: Define the structure and initialize the semaphore.
    atomic_init(&sem->value, initial_value);
    atomic_flag_clear(&sem->lock);
}

/*
 * TODO: Implement semaphore_wait using the TAS spinlock mechanism.
 */


 void semaphore_wait(semaphore* sem) {
    while (1) {
        // Acquire the lock
        while (atomic_flag_test_and_set(&sem->lock)) {
        }

        // Inside critical section: check if we can decrement
        if (atomic_load(&sem->value) > 0) {
            atomic_fetch_sub(&sem->value, 1);
            atomic_flag_clear(&sem->lock); // Release the lock
            return;
        }

        // Not enough resources → release lock and retry
        atomic_flag_clear(&sem->lock);
    }
}

/*
 * TODO: Implement semaphore_signal using the TAS spinlock mechanism.
 */
void semaphore_signal(semaphore* sem) {
    // TODO: Acquire the spinlock, increment the semaphore value, then release the spinlock.
    while (atomic_flag_test_and_set(&sem->lock)) {
    }

    atomic_fetch_add(&sem->value, 1);
    atomic_flag_clear(&sem->lock);
}
