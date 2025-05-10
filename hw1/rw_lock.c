#include "rw_lock.h"

void rwlock_init(rwlock* lock) {
    atomic_init(&lock->readers, 0);
    atomic_flag_clear(&lock->writer);
}

void rwlock_acquire_read(rwlock* lock) {
    while (1)
    {
        while(atomic_load(&lock -> writer)){
        }

        atomic_fetch_add(&lock->readers, 1);
        
        if(!atomic_load(&lock -> writer)){
            break;
        }
        
        atomic_fetch_sub(&lock->readers, 1);
    }
}

void rwlock_release_read(rwlock* lock) {
    atomic_fetch_sub(&lock->readers, 1);
}

void rwlock_acquire_write(rwlock* lock) {
    while(atomic_flag_test_and_set(&lock -> writer)){
    }
    while(atomic_load(&lock -> readers) != 0 ){
    }
}

void rwlock_release_write(rwlock* lock) {
    atomic_flag_clear(&lock -> writer);
}
