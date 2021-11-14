#include <stdio.h>
#include <stdlib.h>

#include "taskQueue.h"

#define FULL 262144

void taskInitialize(taskqueue *q){
    q->front = NULL;
    q->rear = NULL;
}

int taskIsempty(taskqueue *q){
    return (q->front == NULL);
}

void taskEnqueue(taskqueue *q, Task t){
    tnode *tmp;
    tmp = malloc(sizeof(tnode));
    tmp->task = t;
    tmp->next = NULL;
    if (q->rear == NULL){
        q->front = q->rear = tmp;
        return;
    }
    q->rear->next = tmp;
    q->rear = tmp;
}

Task taskDequeue(taskqueue *q){
    // if (q->front == NULL)
    //     return;
    // Store previous front and move front one node ahead
    Task task = q->front->task;
    tnode *temp = q->front;
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
 
    free(temp);
    return task;
}


