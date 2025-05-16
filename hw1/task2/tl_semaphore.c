/*
 * tl_semaphore.c
 *
 * Implementation of a semaphore using the Ticket Lock mechanism for thread synchronization.
 *
 * This file provides a semaphore primitive that uses a ticket lock to ensure FIFO ordering
 * among waiting threads. The semaphore is initialized with a given value, and supports
 * wait (P) and signal (V) operations.
 *
 * Author: Noam Hasson, Asaf Ramati
 */

#include "tl_semaphore.h"

/*
 * semaphore_init
 *
 * Initializes the ticket lock semaphore with the specified initial value.
 * Sets up the value, ticket, and current ticket counters.
 */
void semaphore_init(semaphore* sem, int initial_value) {
    atomic_init(&sem->value, initial_value);
    atomic_init(&sem->ticket, 0);
    atomic_init(&sem->cur_ticket, 0);
}

/*
 * semaphore_wait
 *
 * Wait (P) operation for the ticket lock semaphore.
 * Each thread obtains a ticket and waits until it is their turn and the semaphore value is positive.
 * Once allowed, the thread decrements the semaphore value and advances the ticket.
 */
void semaphore_wait(semaphore* sem) {
    int my_ticket = atomic_fetch_add(&sem->ticket, 1);
    while(atomic_load(&sem->cur_ticket) != my_ticket){
        sched_yield(); 
    }
    while (atomic_load(&sem->value) <= 0) { 
        sched_yield(); 
    }
    atomic_fetch_sub(&sem->value , 1);
    atomic_fetch_add(&sem->cur_ticket, 1);
}

/*
 * semaphore_signal
 *
 * Signal (V) operation for the ticket lock semaphore.
 * Increments the semaphore value, potentially allowing another waiting thread to proceed.
 */
void semaphore_signal(semaphore* sem) {
    atomic_fetch_add(&sem->value, 1);
}
