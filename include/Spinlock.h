#ifndef __SPINLOCK_H
#define __SPINLOCK_H
#include "Cpu.h"
#include "Type.h"

// Mutual exclusion lock.
struct Spinlock {
    u8 locked;       // Is the lock held?

    // For debugging:
    char *name;        // Name of lock.
    struct Cpu* cpu;   // The cpu holding the lock.
};

// Initialize a spinlock
void initLock(struct Spinlock*, char*);

// Acquire the spinlock
// Must be used with releaseLock()
void acquireLock(struct Spinlock*);

// Release the spinlock
// Must be used with acquireLock()
void releaseLock(struct Spinlock*);

// Check whether this cpu is holding the lock
// Interrupts must be off
int holding(struct Spinlock*);

#endif
