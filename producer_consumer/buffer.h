#ifndef BUFFER_H
#define BUFFER_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SHM_KEY 5678
#define SEM_KEY 6789
#define BUFFER_SIZE 10

// Shared buffer structure
typedef struct {
    int in;
    int out;
    int count;
    int buffer[BUFFER_SIZE];
} SharedBuffer;

// Semaphore index
#define SEM_MUTEX 0
#define SEM_EMPTY 1
#define SEM_FULL  2

#endif // BUFFER_H
