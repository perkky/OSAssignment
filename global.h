#pragma once

#include "lift.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>


struct BufferArgs* g_ba;
sem_t* empty_sem;
sem_t* full_sem;
sem_t* lock_sem;

pthread_mutex_t lock_mut;
pthread_cond_t full_cond;
pthread_cond_t empty_cond;
