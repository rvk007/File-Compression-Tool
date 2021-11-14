#ifndef _RESULTQUEUE_H_
#define _RESULTQUEUE_H_
#define MAX_SIZE 4096

typedef struct result {
    int sequenceNumber;
    char *character;
    unsigned char *count;
    int bufferLen;
} Result;

typedef struct rn {
    Result *result;
    struct rn *next;
} rnode;

typedef struct rq {
    int count;
    rnode *front;
    rnode *rear;
} resultqueue;

void resultInitialize(resultqueue *rq);
Result* resultDequeue(resultqueue *rq);
void resultEnqueue(resultqueue *rq, Result *r);
int resultIsempty(resultqueue *rq);
int resultFrontIndex(resultqueue *rq);

#endif
