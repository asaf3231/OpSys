/*
 * cp_pattern.c
 *
 * Implementation of the producer-consumer pattern using ticket locks and condition variables.
 *
 * This program creates multiple producer and consumer threads that share a queue.
 * Producers generate unique random numbers and enqueue them, while consumers dequeue
 * and process the numbers. Synchronization is achieved using ticket locks and condition variables.
 *
 * Author: Noam Hasson, Asaf Ramati
 */

#include "cp_pattern.h"
#include <stdio.h>
#include <stdlib.h>
#include "cond_var.h"
#include <pthread.h>
#include <sched.h>

#define MAX_NUM 1000000

char seen[MAX_NUM] = {0};

// Node in the linked queue
typedef struct node {
    int value;
    struct node* next;
} node_t;

// Queue front and back pointers
node_t* queue_head = NULL;
node_t* queue_tail = NULL;

// Synchronization for the queue
ticket_lock queue_lock;
condition_variable is_empty;


// Counters and termination flag
atomic_int produced_count = 0; // number of items produced
atomic_int stop_flag = 0;      // 1 = stop consumers

// Lock for synchronized printing
ticket_lock print_lock;

condition_variable produced_done;

pthread_t* prod_threads = NULL;
pthread_t* cons_threads = NULL;
int global_num_producers = 0;
int global_num_consumers = 0;

/*
 * Producer thread function.
 *
 * Each producer generates random numbers, ensuring uniqueness, and enqueues them into the shared queue.
 * The producer stops when the maximum number of items has been produced.
 */
void* producer_thread(void* arg);

/*
 * Consumer thread function.
 *
 * Each consumer dequeues numbers from the shared queue and checks if they are divisible by 6.
 * The consumer stops when the stop flag is set and the queue is empty.
 */
void* consumer_thread(void* arg);

/*
 * Start the producer-consumer process.
 *
 * This function initializes synchronization primitives, allocates memory for thread handles,
 * and creates the specified number of producer and consumer threads.
 *
 * Parameters:
 *  - consumers: Number of consumer threads to create.
 *  - producers: Number of producer threads to create.
 *  - seed: Seed for the random number generator.
 */
void start_consumers_producers(int consumers, int producers, int seed) {

    // Print configuration
    printf("  Number of Consumers : %d\n", consumers);
    printf("  Number of Producers: %d\n", producers);
    printf("  Seed:      %d\n", seed);

    srand(seed);

    ticketlock_init(&queue_lock);         // for synchronizing queue access
    ticketlock_init(&print_lock);         // for synchronized printing
    condition_variable_init(&is_empty);   // for waking consumers when queue is not empty
    condition_variable_init(&produced_done);

    global_num_producers = producers;
    global_num_consumers = consumers;

    prod_threads = malloc(sizeof(pthread_t) * producers);
    cons_threads = malloc(sizeof(pthread_t) * consumers);
    
    if (prod_threads == NULL) {
        fprintf(stderr, "Failed to allocate memory for producer threads\n");
        exit(1);
    }

    
    if (cons_threads == NULL) {
        fprintf(stderr, "Failed to allocate memory for consumer threads\n");
        exit(1);
    }

    // Create producer threads
    for (int i = 0; i < producers; i++) {
        int err = pthread_create(&prod_threads[i], NULL, producer_thread, NULL);
        if (err != 0) {
            fprintf(stderr, "Error creating producer thread %d (code %d)\n", i, err);
            exit(1);
        }
    }

    // Create consumer threads
    for (int i = 0; i < consumers; i++) {
        int err = pthread_create(&cons_threads[i], NULL, consumer_thread, NULL);
        if (err != 0) {
            fprintf(stderr, "Error creating consumer thread %d (code %d)\n", i, err);
            exit(1);
        }
    }
}

/*
 * Producer thread function.
 *
 * Each producer generates random numbers, ensuring uniqueness, and enqueues them into the shared queue.
 * The producer stops when the maximum number of items has been produced.
 */
void* producer_thread(void* arg){

    while ((atomic_load(&produced_count) < MAX_NUM)){
        int num = rand() % MAX_NUM;

        ticketlock_acquire(&queue_lock);

        if (!seen[num]) {
            seen[num] = 1;

            printf("Producer %lu generated number: %d\n", (unsigned long)pthread_self(), num);

            node_t* new_node = malloc(sizeof(node_t));
            if (!new_node) {
                fprintf(stderr, "malloc failed\n");
                exit(1);
            }

            new_node->value = num;
            new_node->next = NULL;

            if (queue_tail) {
                queue_tail->next = new_node;
            } else {
                queue_head = new_node;
            }
            queue_tail = new_node;

            int prev = atomic_fetch_add(&produced_count, 1);
            if (prev + 1 == MAX_NUM) {
                condition_variable_signal(&produced_done);
            }

            condition_variable_signal(&is_empty);
        }
        ticketlock_release(&queue_lock);
    }

    

    return NULL;
}   

/*
 * Consumer thread function.
 *
 * Each consumer dequeues numbers from the shared queue and checks if they are divisible by 6.
 * The consumer stops when the stop flag is set and the queue is empty.
 */
void* consumer_thread(void* arg) {
    while (1) {
        ticketlock_acquire(&queue_lock);

        while (queue_head == NULL) {
            if (atomic_load(&stop_flag)) {
                ticketlock_release(&queue_lock);
                return NULL;
            }
            condition_variable_wait(&is_empty, &queue_lock);
        }

        // dequeue
        node_t* node = queue_head;
        int num = node->value;
        queue_head = node->next;
        if (queue_head == NULL) {
            queue_tail = NULL;
        }

        ticketlock_release(&queue_lock);
        free(node);

        // check and print
        char buffer[100];
        snprintf(buffer, sizeof(buffer),
            "Consumer %lu checked %d. Is it divisible by 6? %s",
            (unsigned long)pthread_self(), num,
            (num % 6 == 0) ? "True" : "False");

        print_msg(buffer);
    }
}

/*
 * Stop all consumer threads.
 *
 * This function sets the stop flag and wakes up all waiting consumers.
 */
void stop_consumers() {
    atomic_store(&stop_flag, 1);
    condition_variable_broadcast(&is_empty); 
}

/*
 * Print a message with synchronized access.
 *
 * This function ensures that messages are printed without interference from other threads.
 *
 * Parameters:
 *  - msg: The message to print.
 */
void print_msg(const char* msg) {
    ticketlock_acquire(&print_lock);
    printf("%s\n", msg);
    ticketlock_release(&print_lock);
}

/*
 * Wait until all numbers have been produced.
 *
 * This function blocks until all numbers between 0 and MAX_NUM have been produced by the producers.
 */
void wait_until_producers_produced_all_numbers() {
    ticketlock_acquire(&queue_lock);
    while (atomic_load(&produced_count) < MAX_NUM) {
        condition_variable_wait(&produced_done, &queue_lock);
    }
    ticketlock_release(&queue_lock);
}

/*
 * Wait until the queue is empty.
 *
 * This function blocks until the queue is empty and all work is done. If the queue is already empty,
 * it returns immediately.
 */
void wait_consumers_queue_empty() {
    while (1) {
        ticketlock_acquire(&queue_lock);

        if (queue_head == NULL && atomic_load(&produced_count) == MAX_NUM) {
            ticketlock_release(&queue_lock);
            return; // All work is done and queue is empty
        }

        ticketlock_release(&queue_lock);
        sched_yield(); // Let consumers run
    }

}

/*
 * Main function.
 *
 * This function parses command-line arguments, starts the producer-consumer process,
 * waits for threads to finish, and cleans up resources.
 *
 * Parameters:
 *  - argc: Number of command-line arguments.
 *  - argv: Array of command-line arguments.
 *
 * Returns:
 *  - Exit status code.
 */
int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: cp pattern [consumers] [producers] [seed]\n");
        exit(1);
    }
    
    int consumers = atoi(argv[1]);
    int producers = atoi(argv[2]);
    int seed = atoi(argv[3]);

    if (consumers <= 0 || producers <= 0 || seed <= 0) {
        fprintf(stderr, "usage: cp pattern [consumers] [producers] [seed]\n");
        exit(1);
    }

    start_consumers_producers(consumers, producers, seed);
    wait_until_producers_produced_all_numbers();
    wait_consumers_queue_empty();
    stop_consumers();

    for (int i = 0; i < global_num_producers; i++) {
        pthread_join(prod_threads[i], NULL);
    }
    
    for (int i = 0; i < global_num_consumers; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    return 0;
}
