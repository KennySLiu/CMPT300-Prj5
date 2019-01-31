#ifndef __KLOCK_H__
#define __KLOCK_H__

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "list_sol.h"

typedef struct {
    pthread_mutex_t mutex;
    int id;
} SmartLock;

void init_lock(SmartLock* lock);
int lock(SmartLock* lock);
void unlock(SmartLock* lock);
void cleanup();

#endif
