#pragma once

#include "lift.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

/*Contains the global variables used by either to pthread implemetation or 
 *the process implementation. This is where the semaphores, mutex, conditions
 *and global BufferArgs ins declared.
 */

struct BufferArgs* g_ba;
sem_t* empty_sem;
sem_t* full_sem;
sem_t* lock_sem;

pthread_mutex_t lock_mut;
pthread_cond_t full_cond;
pthread_cond_t empty_cond;
