#pragma once

#include "lift.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

struct BufferArgs* g_ba;
sem_t* g_read_s;
sem_t* g_write_s;
int g_ba_cid, g_read_s_cid, g_write_s_cid, g_data_cid;
pthread_mutex_t readLock;
pthread_mutex_t writeLock;
