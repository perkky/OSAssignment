#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#include "buffer.h"
#include "lift.h"
#include "global.h"

int createSharedMemory(int myBufferSize)
{
    g_ba = (struct BufferArgs*)mmap(NULL, sizeof(struct BufferArgs), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    full_sem = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    empty_sem = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    lock_sem = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    g_ba->data = (int*)mmap(NULL, 2*myBufferSize*sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);

    return 0;
}

void destroySharedMemory()
{
    munmap(g_ba->data, g_ba->Size*2*sizeof(int));
    munmap(g_ba, sizeof(struct BufferArgs));
    munmap(empty_sem, sizeof(sem_t));
    munmap(full_sem, sizeof(sem_t));
    munmap(lock_sem, sizeof(sem_t));
}

int createProcesses(int numRequests, int numLifts)
{
    int parentPID = getpid();
    bool isRequest = false; 
    int childPID = 0;
    int liftId = 0;

    signal(SIGCHLD, SIG_IGN);

    for (int i = 0; i < numRequests; i++)
    {
        if (getpid() == parentPID)
        {
            childPID = fork();
            isRequest = true;

            if (childPID == -1)
            {
                perror("Fork has failed");
                return -1;
            }
        }
    }

    for (int i = 0; i < numLifts; i++)
    {
        if (getpid() == parentPID)
        {
            childPID = fork();
            isRequest = false;
	    liftId++;

            if (childPID == -1)
            {
                perror("Fork has failed");
                return -1;
            }
        }
    }

    if (getpid() == parentPID)
    {
        for (int i = 0; i < numLifts+numRequests; i++)
        {
            wait(NULL);
        }
        FILE* file = fopen(g_ba->writeFileName, "a");
        fprintf(file, "Total number of requests: %d\n", g_ba->requestNum);
        fprintf(file, "Total number of movements: %d\n", g_ba->movementNum);
        fclose(file);
    }
    else
    {
        if (isRequest)
        {
            request((void*)g_ba);
        }
        else
        {
            lift((void*)g_ba);
        }
    }

    return 0;
}


int main(int argc, char* argv[])
{
    createSharedMemory(atoi(argv[1]));
    
    initialiseBuffer(g_ba, atoi(argv[1]), atoi(argv[2]),"sim_out","sim_input");

    sem_init(empty_sem, 1, atoi(argv[1]));
    sem_init(full_sem, 1, 0);
    sem_init(lock_sem, 1, 1);

    if (createProcesses(1,3) == -1)
    {
        return -1;
    }


    sem_destroy(empty_sem);
    sem_destroy(full_sem);
    sem_destroy(lock_sem);
    destroySharedMemory();
}
