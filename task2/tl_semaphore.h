#ifndef TL_SEMAPHORE_H
#define TL_SEMAPHORE_H

#include <stdatomic.h>

/*
 * Define the semaphore type for the Ticket Lock implementation.
 * Write your struct details in this file..
 */
typedef struct {
    atomic_int cur_ticket;
    atomic_int ticket;
    atomic_int value; 

} semaphore;

/*
 * Initializes the semaphore pointed to by 'sem' with the specified initial value.
 */
void semaphore_init(semaphore* sem, int initial_value);

/*
 * Decrements the semaphore (wait operation).
 */
void semaphore_wait(semaphore* sem);

/*
 * Increments the semaphore (signal operation).
 */
void semaphore_signal(semaphore* sem);

#endif // TL_SEMAPHORE_H
