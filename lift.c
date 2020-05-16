#include "lift.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "global.h"
#include <pthread.h>
#include <unistd.h>

void initialiseBuffer(struct BufferArgs* myBufferArg, int myBufferSize, int mySleepTime, char* myWriteFileName, char* myReadFileName)
{
    myBufferArg->writeIndex = 0;
    myBufferArg->readIndex = 0;
    myBufferArg->Size = myBufferSize;
    myBufferArg->sleepTime = mySleepTime;
    myBufferArg->writeFile = fopen(myWriteFileName, "w+");
    myBufferArg->numUsed = 0;
    myBufferArg->readFile = fopen(myReadFileName, "r");
    myBufferArg->isFinished = false;

#ifndef PROCESS
    myBufferArg->data = (int*)malloc(2*myBufferSize*sizeof(int));
#endif

}

void* request(void* ptr)
{
    struct BufferArgs* ba = (struct BufferArgs*)ptr;
    int startFloor = 0, endFloor = 0;
    int* requestNum = (int*)malloc(sizeof(int));
    *requestNum = 0;

    while (fscanf(ba->readFile, "%d %d",&startFloor,&endFloor) != EOF)
    {

        ba->data[ba->writeIndex*2] = startFloor;
        ba->data[ba->writeIndex*2+1] = endFloor;

        ba->writeIndex = (ba->writeIndex +1) % ba->Size;
        ba->numUsed++;

#ifdef PROCESS
        sem_wait(g_write_s);
#else
        pthread_mutex_lock(&writeLock);
#endif

        fprintf(stderr, "New Lift Request From Floor %d to Floor %d\n",startFloor, endFloor);
        fprintf(stderr, "Request No: %d\n\n", ++(*requestNum));

#ifdef PROCESS
        sem_post(g_write_s);
#else
        pthread_mutex_unlock(&writeLock);
#endif

        //If this would make the buffer full, wait until the buffer is not full
        while(ba->numUsed == ba->Size);
    }

    ba->isFinished = true;

    return (void*)requestNum;
}

void* lift(void* ptr)
{
    printf("Lift start\n");
    struct BufferArgs* ba = (struct BufferArgs*)ptr;
    int totalFloorsTbaveled = 0;
    int startFloor = 0, endFloor = 0;
    int requestNum = 0;
    while (!(ba->isFinished && ba->numUsed<=0))
    {
        requestNum++;
        int originalFloor = endFloor;
        int movement = 0;

        //Read the floors form the buffer
#ifdef PROCESS
        sem_wait(g_read_s);
#else
        pthread_mutex_lock(&readLock);
#endif

        //Wait for the buffer to have something to read
        while (!ba->isFinished && ba->numUsed <= 0);

        startFloor = ba->data[ba->readIndex*2];
        endFloor = ba->data[ba->readIndex*2+1];
        ba->readIndex = (ba->readIndex + 1) % ba->Size;
        ba->numUsed--;

#ifdef PROCESS
        sem_post(g_read_s);
#else
        pthread_mutex_unlock(&readLock);
#endif

        movement = abs(originalFloor-startFloor) + abs(endFloor-startFloor);
        totalFloorsTbaveled += movement;
        sleep(ba->sleepTime);

#ifdef PROCESS
        sem_wait(g_write_s);
#else
        pthread_mutex_lock(&writeLock);
#endif

        //Write to the file
        fprintf(stderr, "Previous position: Floor %d\n", originalFloor);
        fprintf(stderr, "Request: Floor %d to Floor %d\n", startFloor, endFloor);
        fprintf(stderr, "Detail opebations:\n");
        fprintf(stderr, "\tGo from Floor %d to Floor %d\n", originalFloor, startFloor);
        fprintf(stderr, "\tGo from Floor %d to Floor %d\n", startFloor, endFloor);
        fprintf(stderr, "\tGo from Floor %d to Floor %d\n", startFloor, endFloor);
        fprintf(stderr, "\t#movement for this request: %d\n", movement);
        fprintf(stderr, "\t#request: %d\n", requestNum);
        fprintf(stderr, "\tTotal #movement: %d\n", totalFloorsTbaveled);
        fprintf(stderr, "Current position: Floor %d\n\n", endFloor);

#ifdef PROCESS
        sem_post(g_write_s);
#else
        pthread_mutex_unlock(&writeLock);
#endif
    }

    printf("Lift finished\n");

    int* movement_ptr = (int*)malloc(sizeof(int));
    *movement_ptr = totalFloorsTbaveled;

    return movement_ptr; 
}
