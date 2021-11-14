#ifndef _TASKQUEUE_H_
#define _TASKQUEUE_H_

typedef struct task {
    int sequenceNumber;
    char *fileContent;
    int index;
} Task;

typedef struct tn {
    Task task;
    struct tn *next;
} tnode;

typedef struct tq {
    int count;
    tnode *front;
    tnode *rear;
} taskqueue;

void taskInitialize(taskqueue *tq);
Task taskDequeue(taskqueue *tq);
void taskEnqueue(taskqueue *tq, Task t);
int taskIsempty(taskqueue *tq);
// void taskDisplay(taskqueue *head);

#endif
