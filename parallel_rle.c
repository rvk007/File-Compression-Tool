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
#include "resultQueue.h"

#define MAX_SIZE 4096

taskqueue *tq;
resultqueue *rq;
int TASKS=0;
int TOTAL_TASK=0;
int SEQUENCE_NUMBER=0;
int JOIN_THREADS=0;
int FINISHED_THREADS=0;
int PRINT_INDEX=0;
char last_alpha=' ';
int last_alpha_count=0;
pthread_mutex_t mutexQueue;
pthread_mutex_t print;
pthread_cond_t condQueue;
pthread_cond_t printQueue;

void executetask(Task t){
    char content[MAX_SIZE];
    memcpy(content, t.fileContent+t.index, sizeof content);
    char alpha_prev = content[0];
    unsigned char alpha_count = 1;
    int bufferLen = 0;
    int notecount = 0;
    char *chars = (char *)malloc(sizeof(char)*MAX_SIZE);
    unsigned char *counts = (unsigned char *)malloc(sizeof(unsigned char) * MAX_SIZE);
    for (int i=1; i<=MAX_SIZE;i++)
    {   
        if(content[i] == alpha_prev) alpha_count ++;
        else
        {   
            chars[bufferLen] = alpha_prev;
            counts[bufferLen] = alpha_count;
            notecount += alpha_count;
            bufferLen ++;
            alpha_prev = content[i];
            alpha_count = 1;
        }   
    }
    
    pthread_mutex_lock(&print);
    Result *r = (Result *)malloc(sizeof(Result));
    r->character = chars;
    r->count = counts;
    r->sequenceNumber = t.sequenceNumber;
    r->bufferLen = bufferLen-1;
    resultEnqueue(rq, r);
    pthread_cond_signal(&printQueue);
    pthread_mutex_unlock(&print);
}

void tobinary(char alpha, unsigned char count){
	fwrite(&alpha, 1, 1, stdout);
	fwrite(&count, 1, 1, stdout);
}

void printdata(int threadCount){
    while(PRINT_INDEX<TOTAL_TASK){
        pthread_mutex_lock(&print);
        // printf("resultIsempty %d %d %d %d\n",resultIsempty(rq), resultFrontIndex(rq),PRINT_INDEX, FINISHED_THREADS );
        while (resultIsempty(rq) && (resultFrontIndex(rq)!=PRINT_INDEX))// && FINISHED_THREADS<threadCount))
        {
            // puts("1");
            pthread_cond_wait(&printQueue, &print);
        }
        // puts("2");
        Result *r = resultDequeue(rq);
        pthread_mutex_unlock(&print);
        int start=0;
        
        if(PRINT_INDEX)
        {   
            if (r->character[0] == last_alpha)
            {
                tobinary(r->character[0], r->count[0]+last_alpha_count);
                // printf("%c%d",r->character[0], r->count[0]+last_alpha_count);
                start=1;
            }
            else
            {
                tobinary(last_alpha, last_alpha_count);
                // printf("%c%d",last_alpha, last_alpha_count);
                start=0;
            }
        }
        for(int i=start;i<r->bufferLen;i++)
        {
            tobinary(r->character[i], r->count[i]);
            // printf("%c%d",r->character[i], r->count[i]);
        }
        last_alpha = r->character[r->bufferLen];
        last_alpha_count = r->count[r->bufferLen];
        PRINT_INDEX++;

        free(r->character);
        free(r->count);
        free(r);
    }
}

void submitTask(char *fileContent, int numberOfTasks){
    for(int i=0;i<numberOfTasks;i++){
        pthread_mutex_lock(&mutexQueue);
        Task t = {
            .sequenceNumber = SEQUENCE_NUMBER,
            .fileContent = fileContent,
            .index = i*MAX_SIZE
        };
        SEQUENCE_NUMBER++;
        taskEnqueue(tq, t);
        TASKS++;
        pthread_cond_signal(&condQueue);
        pthread_mutex_unlock(&mutexQueue);
    }
}

void* allocateThread(void *args){
    while(1){
        pthread_mutex_lock(&mutexQueue);
        // if(TASKS == 0 && TOTAL_TASK>0 && taskIsempty(tq)){
        //     FINISHED_THREADS++;
        //     // pthread_cond_signal(&printQueue);
        //     pthread_mutex_unlock(&mutexQueue);
        //     break;
        // }
        while (TASKS == 0){
            pthread_cond_wait(&condQueue, &mutexQueue);
        }
        TASKS--;
        Task t = taskDequeue(tq);
        pthread_mutex_unlock(&mutexQueue);
        executetask(t);
    }
}

void paralle_rle(char **argv, int start, int fileCount, int threadCount){   
    pthread_t thread[threadCount];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    pthread_mutex_init(&print, NULL);
    pthread_cond_init(&printQueue, NULL);

    // initialize the task queue
    tq = malloc(sizeof(taskqueue));
    taskInitialize(tq);

    // initialize the result queue
    rq = malloc(sizeof(resultqueue));
    resultInitialize(rq);

    // create all the threads
    for(int i=0; i<threadCount; i++)
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
        close(fd);

        // create and submit all the tasks
        int numberOfTasks = fs.st_size%MAX_SIZE==0 ? fs.st_size%MAX_SIZE : floor(fs.st_size/MAX_SIZE)+1;
        pthread_mutex_lock(&mutexQueue);
        TOTAL_TASK += numberOfTasks;
        pthread_mutex_unlock(&mutexQueue);
        submitTask(fileContent, numberOfTasks);
        printdata(threadCount);
        
    }
    tobinary(last_alpha, last_alpha_count);  
}
