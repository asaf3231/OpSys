#ifndef COND_VAR_H
#define COND_VAR_H

#include <stdatomic.h>

/*
 * Define the condition variable type.
 */
typedef struct {
    atomic_flag lock;
    atomic_int waiters; 
} condition_variable;

/*
 * Define the ticket lock type, which may be used as the external lock.
 */
typedef struct {
    atomic_int cur_ticket;
    atomic_int ticket;
} ticket_lock;

/*
 * Initializes the condition variable pointed to by 'cv'.
 */
void condition_variable_init(condition_variable* cv);

/*
 * Causes the calling thread to wait on the condition variable 'cv'.
 * The thread should release the external lock 'ext_lock' while waiting and reacquire it before returning.
 */
void condition_variable_wait(condition_variable* cv, ticket_lock* ext_lock);

/*
 * Wakes up one thread waiting on the condition variable 'cv'.
 */
void condition_variable_signal(condition_variable* cv);

/*
 * Wakes up all threads waiting on the condition variable 'cv'.
 */
void condition_variable_broadcast(condition_variable* cv);

/*
 * Ticket lock function declarations — needed for correct compilation
 */
void ticketlock_init(ticket_lock* lock);
void ticketlock_acquire(ticket_lock* lock);
void ticketlock_release(ticket_lock* lock);

#endif // COND_VAR_H