#include <stdio.h>
#include <stdlib.h>

#include "resultQueue.h"

#define FULL 262144

void resultInitialize(resultqueue *q){
    q->front = NULL;
    q->rear = NULL;
}

int resultIsempty(resultqueue *q){
    return (q->front == NULL);
}

int resultFrontIndex(resultqueue *q){
    if(!resultIsempty(q)) return q->front->result->sequenceNumber;
    return -1;
}
void resultEnqueue(resultqueue *q, Result *r){
    rnode *res;
    res = malloc(sizeof(rnode));
    res->result = r;
    res->next = NULL;
    if (q->rear == NULL)
    {
        q->front = q->rear = res;
        return;
    }
    rnode *temp = q->front;
    if(res->result->sequenceNumber < temp->result->sequenceNumber) {
        res->next = temp;
        q->front = res;
    } else {
        while(temp->next != NULL && temp->next->result->sequenceNumber < res->result->sequenceNumber) {
            temp = temp->next;
        }
        res->next = temp->next;
        temp->next = res;
    }
}

Result* resultDequeue(resultqueue *q){
    // if (q->front == NULL)
    //     return;
    // Store previous front and move front one node ahead
    Result *result = q->front->result;
    rnode *temp = q->front;
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
 
    free(temp);
    return result;
}

