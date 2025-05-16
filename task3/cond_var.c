/*
 * cond_var.c
 *
 * Implementation of condition variables and ticket locks for thread synchronization.
 *
 * This file provides a simple condition variable mechanism using atomic flags and counters,
 * as well as a FIFO ticket lock for mutual exclusion. These primitives are designed to be
 * used in multi-threaded environments for safe coordination between threads.
 *
 * Author: [Your Name]
 * Date: [Today's Date]
 */

#include "cond_var.h"
#include <sched.h>

// Initializes the condition variable: sets the flag to false and waiters to 0
void condition_variable_init(condition_variable* cv) {
    atomic_flag_clear(&cv->lock);       // Initially, no signal
    atomic_init(&cv->waiters, 0);       // No threads waiting
}

// Initializes the ticket lock to its initial state
void ticketlock_init(ticket_lock* lock) {
    atomic_init(&lock->ticket, 0);
    atomic_init(&lock->cur_ticket, 0);
}

/*
 * condition_variable_wait
 *
 * Causes the calling thread to wait on the condition variable.
 * The thread increments the waiters count, releases the external lock,
 * and spins until the signal flag is cleared by another thread.
 * Upon waking, the thread reacquires the external lock and decrements the waiters count.
 */
void condition_variable_wait(condition_variable* cv, ticket_lock* ext_lock) {
    atomic_fetch_add(&cv->waiters, 1); // Mark this thread as a waiter
    ticketlock_release(ext_lock);      // Release the external lock while waiting
    while (atomic_flag_test_and_set(&cv->lock)) { // Spin until signaled
        sched_yield(); 
    }   
    ticketlock_acquire(ext_lock);      // Reacquire the external lock before returning
    atomic_fetch_sub(&cv->waiters, 1); // This thread is no longer waiting
}

/*
 * ticketlock_acquire
 *
 * Acquires the ticket lock (FIFO spinlock). Each thread gets a ticket and waits
 * until its ticket is the current one, ensuring fair access.
 */
void ticketlock_acquire(ticket_lock* lock) {
    int my_ticket = atomic_fetch_add(&lock->ticket, 1); // Get a ticket number
    while (atomic_load(&lock->cur_ticket) != my_ticket) {
        sched_yield(); 
    }
}

/*
 * ticketlock_release
 *
 * Releases the ticket lock, allowing the next ticket holder to proceed.
 */
void ticketlock_release(ticket_lock* lock) {
    atomic_fetch_add(&lock->cur_ticket, 1); // Advance to the next ticket
}

/*
 * condition_variable_signal
 *
 * Wakes up a single waiting thread, if any, by clearing the signal flag.
 */
void condition_variable_signal(condition_variable* cv) {
    if (atomic_load(&cv->waiters) > 0) {
        atomic_flag_clear(&cv->lock);
    }
}

/*
 * condition_variable_broadcast
 *
 * Wakes up all waiting threads by repeatedly clearing the signal flag.
 * Each clear allows one waiting thread to proceed.
 */
void condition_variable_broadcast(condition_variable* cv) {
    int w = atomic_load(&cv->waiters);
    for (int i = 0; i < w; i++) {
        atomic_flag_clear(&cv->lock); // Wake one thread per clear
        sched_yield();
    }
}
