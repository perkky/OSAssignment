#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#include "lift.h"
#include "global.h"

int createSharedMemory()
{

    g_ba_cid = shm_open("Data Memory", O_RDWR|O_CREAT, 0666);
    g_read_s_cid = shm_open("Read Semaphore", O_RDWR|O_CREAT, 0666);
    g_write_s_cid = shm_open("Write Semaphore", O_RDWR|O_CREAT, 0666);

    if (g_ba_cid == -1 || g_write_s_cid == -1 || g_read_s_cid == -1)
    {
        perror("Shm_open failed\n");
        return -1;
    }

    ftruncate(g_ba_cid, sizeof(struct BufferArgs));
    ftruncate(g_write_s_cid, sizeof(sem_t));
    ftruncate(g_read_s_cid, sizeof(sem_t));

    g_ba = (struct BufferArgs*)mmap(NULL, sizeof(struct BufferArgs), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, g_ba_cid, 0);
    g_read_s = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, g_read_s_cid, 0);
    g_write_s = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, g_write_s_cid, 0);

    return 0;
}

void destroySharedMemory()
{
    munmap(g_ba, sizeof(struct BufferArgs));
    munmap(g_read_s, sizeof(sem_t));
    munmap(g_write_s, sizeof(sem_t));

    shm_unlink("Data Memory");
    shm_unlink("Read Semaphore");
    shm_unlink("Write Semaphore");

}
int createProcesses(int numRequests, int numLifts)
{
    int parentPID = getpid();
    bool isRequest = false; 
    int childPID = 0;

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
    createSharedMemory();
    
    initialiseBuffer(g_ba, atoi(argv[1]),atoi(argv[2]),"sim_out","sim_input");

    sem_init(g_write_s, 1, 1);

    sem_init(g_read_s, 1, 1);

    if (createProcesses(1,3) == -1)
    {
        return -1;
    }

    sem_destroy(g_read_s);
    sem_destroy(g_write_s);
    destroySharedMemory();
}
