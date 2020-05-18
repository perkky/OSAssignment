#include "buffer.h"

/*
 * This function takes a pointer to the BufferArg struct and initialises it
 * based on the input parameters.
 * If the implementation is using pthreads, the data is malloc'd, otherwise
 * it is allocated using mmap in another function to allow it to be shared 
 * between processes
 */
void initialiseBuffer(struct BufferArgs* myBufferArg, int myBufferSize, int mySleepTime, char* myWriteFileName, char* myReadFileName)
{
    myBufferArg->writeIndex = 0;
    myBufferArg->readIndex = 0;
    myBufferArg->Size = myBufferSize;
    myBufferArg->sleepTime = mySleepTime;
    myBufferArg->writeFileName = myWriteFileName;
    myBufferArg->numUsed = 0;
    myBufferArg->readFile = fopen(myReadFileName, "r");
    myBufferArg->requestNum = 0;
    myBufferArg->movementNum = 0;
    myBufferArg->isFinished = false;
	
#ifndef PROCESS
    myBufferArg->data = (int*)malloc(2*myBufferSize*sizeof(int));
#endif

}

bool isFull(struct BufferArgs* myBufferArg)
{
    return myBufferArg->Size == myBufferArg->numUsed;
}
bool isEmpty(struct BufferArgs* myBufferArg)
{
    return myBufferArg->numUsed == 0;
}
