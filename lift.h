#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h> 

struct BufferArgs
{
    int writeIndex;
    int readIndex;
    int numUsed;
    int Size;
    int* data;
    int sleepTime;
    FILE* writeFile;
    FILE* readFile;
    bool isFinished;
};

void initialiseBuffer(struct BufferArgs* myBufferArg, int myBufferSize, int mySleepTime, char* myWriteFileName, char* myReadFileName);

void* request(void* ptr);

void* lift(void* ptr);
