#include "lift.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "global.h"
#include <pthread.h>
#include <unistd.h>
#include "buffer.h"

void writeRequestToFile(char* myFileName, int myStartFloor, int myEndFloor, int myRequestNum)
{
    FILE* file = fopen(myFileName, "a");
    fprintf(file, "---------------------------------------------\n");
    fprintf(file, "New Lift Request From Floor %d to Floor %d\n", myStartFloor, myEndFloor);
    fprintf(file, "Request No: %d\n", myRequestNum);
    fprintf(file, "---------------------------------------------\n");
    fclose(file);
}

void writeLiftLogToFile(char* myFileName, int myOriginalFloor, int myStartFloor, int myEndFloor, int myMovement, int myRequestNum, int myTotalFloorsTraveled)
{
    FILE* file = fopen(myFileName, "a");
    fprintf(file, "---------------------------------------------\n");
    fprintf(file, "Previous position: Floor %d\n", myOriginalFloor);
    fprintf(file, "Request: Floor %d to Floor %d\n", myStartFloor, myEndFloor);
    fprintf(file, "Detail opebations:\n");
    fprintf(file, "\tGo from Floor %d to Floor %d\n", myOriginalFloor, myStartFloor);
    fprintf(file, "\tGo from Floor %d to Floor %d\n", myStartFloor, myEndFloor);
    fprintf(file, "\t#movement for this request: %d\n", myMovement);
    fprintf(file, "\t#request: %d\n", myRequestNum);
    fprintf(file, "\tTotal #movement: %d\n", myTotalFloorsTraveled);
    fprintf(file, "Current position: Floor %d\n", myEndFloor);
    fprintf(file, "---------------------------------------------\n");
    fclose(file);
}

/* The request function is run by the thread/process aiming to emulate the request.
 * A void ptr of BufferArgs is passed into it to access shared resources. 
 * The input file is read line by line, with the lift request being added to the buffer.
 * It locks the critical section regardless of if pthreads are being used or processes
 * are being used.
 */
void* request(void* ptr)
{
    struct BufferArgs* ba = (struct BufferArgs*)ptr;
    int startFloor = 0, endFloor = 0;

    while (fscanf(ba->readFile, "%d %d",&startFloor,&endFloor) != EOF)
    {

        if (startFloor >0 && startFloor <21 && endFloor >0 && endFloor <21)
        {
#ifdef PROCESS
            sem_wait(empty_sem);
            sem_wait(lock_sem);
#else
            pthread_mutex_lock(&lock_mut);

            //Wait if the buffer is full
            if (isFull(ba)) pthread_cond_wait(&full_cond, &lock_mut);
#endif 
            
            ba->data[2*ba->writeIndex] = startFloor;
            ba->data[2*ba->writeIndex+1] = endFloor;
            ba->numUsed++;
            ba->writeIndex = (ba->writeIndex+1 ) % ba->Size;
            ba->requestNum++;
            
            writeRequestToFile(ba->writeFileName, startFloor, endFloor,ba->requestNum);

#ifdef PROCESS
            sem_post(lock_sem);
            sem_post(full_sem);
#else
            pthread_mutex_unlock(&lock_mut);
            pthread_cond_signal(&empty_cond);
#endif
        }
    }

    ba->isFinished = true;

    return NULL;
}

/* The lift function is run by 3 threads/processes to emulate a lift.
 *
 * It takes a void ptr to a BufferArgs struct to access shared resources.
 * It checks to see if there is a lift request in he buffer, and if there
 * is, only one thread/process is allowed into the critical section to
 * consume the lift request. Works with both threads and processes
 */
void* lift(void* ptr)
{
    struct BufferArgs* ba = (struct BufferArgs*)ptr;
    int startFloor = 0, endFloor = 0;
    int movement = 0;
    int totalFloorsTraveled = 0;
    int requestNum = 0;

    while(!ba->isFinished || !isEmpty(ba))
    {
#ifdef PROCESS
        sem_wait(full_sem);
        sem_wait(lock_sem);
        
        //If the buffer is now empty and no more requests are being added, terminate immediately
        if (ba->isFinished && isEmpty(ba)) return NULL;
#else
        pthread_mutex_lock(&lock_mut);

        //If the buffer is now empty and no more requests are being added, terminate immediately
        if (ba->isFinished && isEmpty(ba)) return NULL;
        else if (isEmpty(ba)) pthread_cond_wait(&empty_cond, &lock_mut);
#endif

        int originalFloor = endFloor;
        startFloor = ba->data[ba->readIndex*2];
        endFloor = ba->data[ba->readIndex*2+1];
        ba->readIndex = (ba->readIndex+1 ) % ba->Size;
        ba->numUsed--;
        movement = abs(originalFloor-startFloor) + abs(endFloor-startFloor);
        ba->movementNum += movement;
        totalFloorsTraveled += movement;
        requestNum++;

        writeLiftLogToFile(ba->writeFileName, originalFloor, startFloor, endFloor, movement, requestNum, totalFloorsTraveled);

#ifdef PROCESS
        sem_post(lock_sem);
        sem_post(empty_sem);
#else
        pthread_mutex_unlock(&lock_mut);
        pthread_cond_signal(&full_cond);
#endif

        sleep(ba->sleepTime);
    }

    return NULL; 
}
