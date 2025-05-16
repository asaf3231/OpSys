/*
 * rw_lock.c
 *
 * Implementation of a readers-writer lock using atomic operations, ticket locks, and condition variables.
 *
 * This lock allows multiple readers to access a shared resource concurrently, but writers require exclusive access.
 * The implementation ensures fairness and prevents race conditions in a multi-threaded environment.
 *
 * Author: Noam Hasson, Asaf Ramat
 */

#include "rw_lock.h"

/*
 * Initializes the readers-writer lock structure.
 * Sets the readers count to 0, clears the writer flag, and initializes the condition variable and ticket lock.
 */
void rwlock_init(rwlock* lock) {
    atomic_init(&lock->readers, 0);
    atomic_flag_clear(&lock->writer);
    condition_variable_init(&lock->cv);
    ticketlock_init(&lock->lock);
}

/*
 * Acquires the lock for reading.
 * Multiple readers can hold the lock concurrently as long as no writer is active.
 * If a writer is active, the reader waits on the condition variable.
 */
void rwlock_acquire_read(rwlock* lock) {
    while (1) {
        ticketlock_acquire(&lock->lock);
        if (!atomic_flag_test_and_set(&lock->writer)) {
            atomic_flag_clear(&lock->writer);  // just a peek
            atomic_fetch_add(&lock->readers, 1);
            ticketlock_release(&lock->lock);
            break;
        }
        condition_variable_wait(&lock->cv, &lock->lock);  // releases and reacquires internally
        ticketlock_release(&lock -> lock);
    }
}

/*
 * Releases the lock after reading.
 * Decrements the readers count. If this was the last reader, signals a waiting writer.
 */
void rwlock_release_read(rwlock* lock) {
    int remaining = atomic_fetch_sub(&lock->readers, 1) - 1;
    if (remaining == 0) {
        condition_variable_signal(&lock->cv);
    }
}

/*
 * Acquires the lock for writing.
 * Waits until there are no readers and no other writer is active.
 * Sets the writer flag to indicate exclusive access.
 */
void rwlock_acquire_write(rwlock* lock) {
    ticketlock_acquire(&lock->lock);
    while (atomic_load(&lock->readers) > 0 || atomic_flag_test_and_set(&lock->writer)) {
        condition_variable_wait(&lock->cv, &lock->lock);
    }
    // Explicitly set writer flag (though it's already set above, this clarifies intention)
    atomic_flag_test_and_set(&lock->writer);
    ticketlock_release(&lock->lock);
}

/*
 * Releases the lock after writing.
 * Clears the writer flag and broadcasts to all waiting threads (readers and writers).
 */
void rwlock_release_write(rwlock* lock) {
    atomic_flag_clear(&lock->writer);
    condition_variable_broadcast(&lock->cv);
}