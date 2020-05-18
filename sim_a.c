#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "buffer.h"
#include "lift.h"
#include "global.h"

/* The main function
 *
 * This initialises the BufferArgs struct (shared resource),
 * the threads, creates the thread and then waits for the threads
 * to terminate. One request thread is created and three lift
 * threads are created.
 */
int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        struct BufferArgs bufferArgs;
        initialiseBuffer(&bufferArgs,atoi(argv[1]), atoi(argv[2]), "sim_out", "sim_input");

        pthread_t liftThreads[3];
        pthread_t requestThread;
        
        if(pthread_mutex_init(&lock_mut, NULL))
        {
            printf("Read mutex init failed");
            return 1;
        }

        if(pthread_cond_init(&full_cond, NULL))
        {
            printf("Full cond init failed");
            return 1;
        }
        if(pthread_cond_init(&empty_cond, NULL))
        {
            printf("Empty cond init failed");
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
	
        if(pthread_join(requestThread, NULL))
        {
            fprintf(stderr, "Error joining request thread");
        }

        for (int i = 0; i < 3; i++)
        {
            if(pthread_join(liftThreads[i], NULL))
            {
                fprintf(stderr, "Error joining lift threads");
            }
        }

        FILE* file = fopen(bufferArgs.writeFileName, "a");
        fprintf(file, "Total number of requests: %d\n", bufferArgs.requestNum);
        fprintf(file, "Total number of movements: %d\n", bufferArgs.movementNum);
        fclose(file);
        
        return 0;
    }
    else
    {
        printf("Error: Incorrect number of agruments");
        return 1;
    }
}
