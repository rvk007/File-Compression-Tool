#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h> 
#include <stdio.h> 
#include <sys/stat.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <ctype.h> 
#include <stdbool.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>

#include "taskQueue.h"

#define MAX_SIZE 4096

typedef struct resultBuffer {
    char character[MAX_SIZE];
    unsigned char count[MAX_SIZE];
} ResultBuffer;
ResultBuffer buffer[100]; //?? sequenceNumber

queue *q;
int TOTAL_TASK=0;
// int PRINT_INDEX=-1;
char last_alpha;
int last_alpha_count=0;
pthread_mutex_t mutexQueue;
// pthread_mutex_t submit;
pthread_mutex_t print;
pthread_cond_t condQueue;
pthread_cond_t printQueue;

void executetask(Task t)
{
    char content[MAX_SIZE];
    memcpy(content, t.fileContent+t.index, sizeof content);
    char alpha_prev = content[0];
    unsigned char alpha_count = 1;
    int store = 0;
    int notecount = 0;
    for (int i=1; i<=MAX_SIZE;i++)
    {   
        if(content[i] == alpha_prev) alpha_count ++;
        else
        {   
            buffer[t.sequenceNumber].character[store] = alpha_prev;
            buffer[t.sequenceNumber].count[store] = alpha_count;
            notecount += alpha_count;
            store ++;
            alpha_prev = content[i];
            alpha_count = 1;
        }   
    }
    // PRINT_INDEX++;
    // pthread_cond_signal(&printQueue);
}

void tobinary(char alpha, unsigned char count){
	fwrite(&alpha, 1, 1, stdout);
	fwrite(&count, 1, 1, stdout);
}

void ifBufferDataAvailable(int printIndex)
{
    while(1)
    {
        if(buffer[printIndex].character[0] && buffer[printIndex].count[0])
        {
            pthread_cond_signal(&printQueue);
            break;
        }
    }
}

void printdata(int printIndex)
{
    pthread_mutex_lock(&print);
    while (!(buffer[printIndex].character[0] && buffer[printIndex].count[0]))
    {
        pthread_cond_wait(&printQueue, &print);
    }
    int bufferLen = strlen(buffer[printIndex].character);
    int start=0;
    if(printIndex)
    {
        if (buffer[printIndex].character[0] == last_alpha)
        {
            tobinary(buffer[printIndex].character[0], buffer[printIndex].count[0]+last_alpha_count);
            start=1;
        }
        else
        {
            tobinary(last_alpha, last_alpha_count);
            start=0;
        }
    }
    for(int i=start;i<bufferLen-1;i++)
    {
        tobinary(buffer[printIndex].character[i], buffer[printIndex].count[i]);
    }
    last_alpha = buffer[printIndex].character[bufferLen-1];
    last_alpha_count = buffer[printIndex].count[bufferLen-1];
    pthread_mutex_unlock(&print);
}

void submitTask(Task t)
{
    pthread_mutex_lock(&mutexQueue);
    enqueue(q, t);
    TOTAL_TASK++;
    pthread_cond_signal(&condQueue);
    pthread_mutex_unlock(&mutexQueue);
}

void* allocateThread(void *args)
{
    while(1){
        pthread_mutex_lock(&mutexQueue);
        while (TOTAL_TASK == 0)
        {
            pthread_cond_wait(&condQueue, &mutexQueue);
        }
        TOTAL_TASK--;
        Task t = dequeue(q);
        pthread_mutex_unlock(&mutexQueue);
        executetask(t);
    }
}

void paralle_rle(char **argv, int start, int fileCount, int threadCount)
{   
    int THREAD_COUNT = threadCount;
    int sequenceNumber = 0;
    pthread_t thread[THREAD_COUNT];
    pthread_mutex_init(&mutexQueue, NULL);
    // pthread_mutex_init(&submit, NULL);
    pthread_cond_init(&condQueue, NULL);
    pthread_mutex_init(&print, NULL);
    pthread_cond_init(&printQueue, NULL);

    // initialize the task queue
    q = malloc(sizeof(queue));
    initialize(q);

    // create all the threads
    for(int i=0; i<THREAD_COUNT; i++)
    {
        if (pthread_create(&thread[i], NULL, &allocateThread, NULL) != 0)
            perror("Error while creating a thread");
    }

    // create tasks
    for(int i=start; i<start+fileCount; i++)
    {
        struct stat fs;
        int fd = open(argv[i], O_RDONLY, 0644);
        if(fstat(fd, &fs) == -1)
        {
            perror("Error while reading file stat");
        }
        char *fileContent = mmap(NULL , fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

        // create and submit all the tasks
        int numberOfTasks = fs.st_size%MAX_SIZE==0 ? fs.st_size%MAX_SIZE : floor(fs.st_size/MAX_SIZE)+1;
        for(int i=0;i<numberOfTasks;i++)
        {   
            Task t = {
                .sequenceNumber = sequenceNumber,
                .fileContent = fileContent,
                .index = i*MAX_SIZE
            };
            sequenceNumber++;
            submitTask(t);
            // printdata();
        }
        close(fd);
    }

    for (int i=0;i<sequenceNumber;i++)
    {   
        ifBufferDataAvailable(i);
        printdata(i);
    }
    tobinary(last_alpha, last_alpha_count);
    // wait for threads to finish
    for(int i=0; i<THREAD_COUNT; i++)
    {
        if(pthread_join(thread[i], NULL) != 0)
            perror("Error while joining the threads");
    }
    
    // destroy all the mutex and cond variables
    pthread_mutex_destroy(&mutexQueue);
    // pthread_mutex_destroy(&submit);
    pthread_mutex_destroy(&print);
    pthread_cond_destroy(&condQueue);
    pthread_cond_destroy(&printQueue);
}
