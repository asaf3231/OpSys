#include "cond_var.h"
#include <sched.h>

// Initializes the condition variable: sets the flag to false and waiters to 0
void condition_variable_init(condition_variable* cv) {
    atomic_flag_clear(&cv->lock);       // Initially, no signal
    atomic_init(&cv->waiters, 0);       // No threads waiting
}

void ticketlock_init(ticket_lock* lock) {
    atomic_init(&lock->ticket, 0);
    atomic_init(&lock->cur_ticket, 0);
}

// Causes the calling thread to wait on the condition variable
void condition_variable_wait(condition_variable* cv, ticket_lock* ext_lock) {
    // Mark this thread as a waiter
    atomic_fetch_add(&cv->waiters, 1);

    // Release the external lock while waiting
    ticketlock_release(ext_lock);

    // Spin until the signal flag is set by another thread
    while (atomic_flag_test_and_set(&cv->lock)) {
        // Busy wait
    }

    // Reacquire the external lock before returning
    ticketlock_acquire(ext_lock);

    // This thread is no longer waiting
    atomic_fetch_sub(&cv->waiters, 1);
}

// Acquires the ticket lock (FIFO spinlock)
void ticketlock_acquire(ticket_lock* lock) {
    int my_ticket = atomic_fetch_add(&lock->ticket, 1); // Get a ticket number
    while (atomic_load(&lock->cur_ticket) != my_ticket) {
        // Busy wait until it's our turn
    }
}

// Releases the ticket lock, allowing the next ticket holder to proceed
void ticketlock_release(ticket_lock* lock) {
    atomic_fetch_add(&lock->cur_ticket, 1); // Advance to the next ticket
}


/*
 * TODO: Implement condition_variable_signal.
 */
void condition_variable_signal(condition_variable* cv) {
    if (atomic_load(&cv->waiters) > 0) {
        atomic_flag_clear(&cv->lock);
    }
}

void condition_variable_broadcast(condition_variable* cv) {
    int w = atomic_load(&cv->waiters);

    for (int i = 0; i < w; i++) {
        atomic_flag_clear(&cv->lock); // Wake one thread per clear
        sched_yield();
    }
}
