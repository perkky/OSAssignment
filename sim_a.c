#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "lift.h"
#include "global.h"

int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        struct BufferArgs bufferArgs;
        initialiseBuffer(&bufferArgs,atoi(argv[1]), atoi(argv[2]), "sim_out", "sim_input");

        pthread_t liftThreads[3];
        pthread_t requestThread;
        
        if(pthread_mutex_init(&readLock, NULL))
        {
            printf("Read mutex init failed");
            return 1;
        }

        if(pthread_mutex_init(&writeLock, NULL))
        {
            printf("Write mutex init failed");
            return 1;
        }

        if(pthread_create(&requestThread, NULL, request, &bufferArgs))
        {
            fprintf(stderr, "Error creating request thread");
            return 1;
        }

        for (int i = 0; i < 3; i++)
        {
            if(pthread_create(&liftThreads[i], NULL, lift, &bufferArgs))
            {
                fprintf(stderr, "Error creating lift threads");
                return 1;
            }
        }
        /*printf("x is %d\n", x);*/
	
	int numReq = 0, numMove = 0;
        void* tmp_ptr;
        if(pthread_join(requestThread, &tmp_ptr))
        {
            fprintf(stderr, "Error joining request thread");
        }
        numReq = *(int*)tmp_ptr;

        for (int i = 0; i < 3; i++)
        {
            if(pthread_join(liftThreads[i], &tmp_ptr))
            {
                fprintf(stderr, "Error joining lift threads");
            }
	    numMove += *(int*)tmp_ptr;
        }

	fprintf(stderr, "Total number of requests: %d\n", numReq);
	fprintf(stderr, "Total number of movements: %d\n", numMove);
        /*printf("x is %d\n", x);*/
        return 0;
    }
    else
    {
        printf("Error: Incorrect number of agruments");
        return 1;
    }
}
