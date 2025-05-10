#include "tl_semaphore.h"

/*
 * TODO: Implement semaphore_init for the Ticket Lock semaphore.
 */
void semaphore_init(semaphore* sem, int initial_value) {
    // TODO: Define the structure and initialize the semaphore.
    atomic_init(&sem->value, initial_value);
    atomic_init(&sem->ticket, 0);
    atomic_init(&sem->cur_ticket, 0);
}

/*
 * TODO: Implement semaphore_wait using the Ticket Lock mechanism.
 */
void semaphore_wait(semaphore* sem) {
    // TODO: Obtain a ticket and wait until it is your turn; then decrement the semaphore value.
    int my_ticket = atomic_fetch_add(&sem->ticket, 1);
    while(atomic_load(&sem->cur_ticket) != my_ticket){
    }

    while (atomic_load(&sem->value) <= 0) { 
    }
    atomic_fetch_sub(&sem->value , 1);
    atomic_fetch_add(&sem->cur_ticket, 1);

}

/*
 * TODO: Implement semaphore_signal using the Ticket Lock mechanism.
 */
void semaphore_signal(semaphore* sem) {
    // TODO: Increment the semaphore value and allow the next ticket holder to proceed.
    atomic_fetch_add(&sem->value, 1);
}
