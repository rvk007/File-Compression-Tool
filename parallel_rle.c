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
#define RESULT_SIZE 262144

typedef struct result {
    int sequenceNumber;
    char first_alpha;
    unsigned char first_count;
    char last_alpha;
    unsigned char last_count;
    unsigned char *midData;
    int bufferLen;
} Result;

Result **resBuffer;
taskqueue *tq;
int TASKS=0;
int TOTAL_TASK=0;
int SEQUENCE_NUMBER=0;
int PRINT_INDEX=0;
char last_alpha=' ';
int last_alpha_count=0;
pthread_mutex_t mutexQueue;
pthread_mutex_t print;
pthread_cond_t condQueue;
pthread_cond_t printQueue;


// function to print the compressed data
void printdata(){
    while(PRINT_INDEX<TOTAL_TASK) {
        pthread_mutex_lock(&print);
        while (resBuffer[PRINT_INDEX]==NULL) {
            pthread_cond_wait(&printQueue, &print);
        }
        Result *r = resBuffer[PRINT_INDEX];
        pthread_mutex_unlock(&print);
        
        if(PRINT_INDEX){   
            if (r->first_alpha == last_alpha){
                fprintf(stdout, "%c%c", r->first_alpha, r->first_count+last_alpha_count);
            }
            else{
                fprintf(stdout, "%c%c", last_alpha, last_alpha_count);
                fprintf(stdout, "%c%c", r->first_alpha, r->first_count);
            }
        }
        else{
            fprintf(stdout, "%c%c", r->first_alpha,r->first_count);
        }
        fprintf(stdout, "%.*s", r->bufferLen, r->midData);

        last_alpha = r->last_alpha;
        last_alpha_count = r->last_count;
        PRINT_INDEX++;

        free(r->midData);
        free(r);
    }
}

// funtion to read the file and store the data in the buffer
void executetask(Task t){
    int contentSize = t.index+MAX_SIZE<=t.fileSize ? MAX_SIZE : t.fileSize-t.index;
    int start = t.index;
    char alpha_prev = t.fileContent[start];
    unsigned char alpha_count = 1;
    int first = 1;

    char first_alpha;
    unsigned char first_count;
    char last_alpha;
    unsigned char last_count;
    int bufferLen = 0;
    unsigned char *midData = (unsigned char *)malloc(sizeof(unsigned char) * (2*MAX_SIZE));

    for (int i=start+1; i<start+contentSize;i++) {   
        if(t.fileContent[i] == alpha_prev) alpha_count ++;
        else {
            if (first){
                first_alpha = alpha_prev;
                first_count = alpha_count;
                first=0;
            }
            else{
                midData[bufferLen] = alpha_prev;
                midData[bufferLen+1] = alpha_count;
                bufferLen += 2;
            }
            alpha_prev = t.fileContent[i];
            alpha_count = 1;
        } 
    }
    last_alpha = alpha_prev;
    last_count = alpha_count;

    pthread_mutex_lock(&print);
    Result *r = (Result *)malloc(sizeof(Result));
    r->first_alpha = first_alpha;
    r->first_count = first_count;
    r->last_alpha = last_alpha;
    r->last_count = last_count;
    r->bufferLen = bufferLen;
    r->midData = midData;
    resBuffer[t.sequenceNumber] = r;
    pthread_cond_signal(&printQueue);
    pthread_mutex_unlock(&print);
}

// add task to the Task Queue
void submitTask(char *fileContent, int fileSize, int numberOfTasks){
    for(int i=0;i<numberOfTasks;i++){
        pthread_mutex_lock(&mutexQueue);
        Task t = {
            .sequenceNumber = SEQUENCE_NUMBER,
            .fileContent = fileContent,
            .fileSize = fileSize,
            .index = i*MAX_SIZE
        };
        SEQUENCE_NUMBER++;
        taskEnqueue(tq, t);
        TASKS++;
        pthread_cond_signal(&condQueue);
        pthread_mutex_unlock(&mutexQueue);
    }
}

// thread routine function
void* allocateThread(void *args){
    while(1){
        pthread_mutex_lock(&mutexQueue);
        while (TASKS == 0){
            pthread_cond_wait(&condQueue, &mutexQueue);
        }
        TASKS--;
        Task t = taskDequeue(tq);
        pthread_mutex_unlock(&mutexQueue);
        executetask(t);
    }
}

// parallel encode
void paralle_rle(char **argv, int start, int fileCount, int threadCount){   
    pthread_t thread[threadCount];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    pthread_mutex_init(&print, NULL);
    pthread_cond_init(&printQueue, NULL);

    // initialize the task queue
    tq = malloc(sizeof(taskqueue));
    taskInitialize(tq);

    // initialize the result buffer
    resBuffer = (Result **)malloc(sizeof(Result *) * RESULT_SIZE);

    // create all the threads
    for(int i=0; i<threadCount; i++) {
        if (pthread_create(&thread[i], NULL, &allocateThread, NULL) != 0)
            perror("Error while creating a thread");
    }

    // create tasks
    for(int i=start; i<start+fileCount; i++) {   
        struct stat fs;
        int fd = open(argv[i], O_RDONLY, 0644);
        if(fstat(fd, &fs) == -1) {
            perror("Error while reading file stat");
        }
        char *fileContent = mmap(NULL , fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);

        // create and submit all the tasks
        int numberOfTasks = fs.st_size%MAX_SIZE==0 ? fs.st_size%MAX_SIZE : floor(fs.st_size/MAX_SIZE)+1;
        pthread_mutex_lock(&mutexQueue);
        TOTAL_TASK += numberOfTasks;
        pthread_mutex_unlock(&mutexQueue);
        submitTask(fileContent, fs.st_size, numberOfTasks);
    }

    // print the result
    printdata();
    // print the last alpha
    fprintf(stdout, "%c%c", last_alpha, last_alpha_count);
    free(resBuffer);
}
