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

int createSharedMemory(int myBufferSize)
{

    g_ba_cid = shm_open("BA Memory", O_RDWR|O_CREAT, 0666);
    g_data_cid = shm_open("Data Memory", O_RDWR|O_CREAT, 0666);
    g_read_s_cid = shm_open("Read Semaphore", O_RDWR|O_CREAT, 0666);
    g_write_s_cid = shm_open("Write Semaphore", O_RDWR|O_CREAT, 0666);
    g_return_data_cid = shm_open("Return Memory", O_RDWR|O_CREAT, 0666);

    if (g_ba_cid == -1 || g_write_s_cid == -1 || g_read_s_cid == -1 || g_data_cid == -1 || g_return_data_cid == -1)
    {
        perror("Shm_open failed\n");
        return -1;
    }

    ftruncate(g_ba_cid, sizeof(struct BufferArgs));
    ftruncate(g_data_cid, 2*myBufferSize*sizeof(int));
    ftruncate(g_write_s_cid, sizeof(sem_t));
    ftruncate(g_read_s_cid, sizeof(sem_t));
    ftruncate(g_return_data_cid, 4*sizeof(int));

    g_ba = (struct BufferArgs*)mmap(NULL, sizeof(struct BufferArgs), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, g_ba_cid, 0);
    g_read_s = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, g_read_s_cid, 0);
    g_write_s = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, g_write_s_cid, 0);
    g_ba->data = (int*)mmap(NULL, 2*myBufferSize*sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, g_data_cid, 0);
    g_return_data = (int*)mmap(NULL, 4*sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, g_return_data_cid, 0);

    return 0;
}

void destroySharedMemory()
{
    munmap(g_ba->data, g_ba->Size*2*sizeof(int));
    munmap(g_ba, sizeof(struct BufferArgs));
    munmap(g_read_s, sizeof(sem_t));
    munmap(g_write_s, sizeof(sem_t));
    munmap(g_return_data, 4*sizeof(int));

    shm_unlink("Data Memory");
    shm_unlink("BA Memory");
    shm_unlink("Read Semaphore");
    shm_unlink("Write Semaphore");
    shm_unlink("Return Memory");
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

	fprintf(stderr, "Total number of requests: %d\n", g_return_data[0]);
	fprintf(stderr, "Total number of movements: %d\n", g_return_data[1]+g_return_data[2]+g_return_data[3]);

    }
    else
    {
        if (isRequest)
        {
	    g_return_data[0] = *(int*)request((void*)g_ba);
	    /*printf("ss %d\n", *(int*)request((void*)g_ba));*/
        }
        else
        {
	    g_return_data[liftId] = *(int*)lift((void*)g_ba);
	    printf("num req is %d\n", liftId);
	    /*lift((void*)g_ba);*/
        }
    }

    return 0;
}


int main(int argc, char* argv[])
{
    createSharedMemory(atoi(argv[1]));
    
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
