/***
 References:
 1. https://www.youtube.com/watch?v=d9s_d28yJq0&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2
 2. https://stackoverflow.com/questions/10451992/how-should-a-thread-pool-be-implemented-in-c
 3. https://nachtimwald.com/2019/04/12/thread-pool-in-c/
***/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "sequential_rle.h"
#include "parallel_rle.h"

int main(int argc, char **argv){
    int start, fileCount, i, threadCount=0, argJ=0;
    for(i=0;i<argc;i++) {
        if(strcmp(argv[i],"-j")==0) {
            argJ = 1;
            threadCount=atoi(argv[i+1]);
            break;
        }
    }

    if (argJ) {
        start = i+2; fileCount=argc-(i+1)-1;
        if(threadCount == 0) return 0;
        else if (threadCount == 1) sequential_rle(argv, start, fileCount);
        else if (threadCount>1) paralle_rle(argv, start, fileCount, threadCount);
    } else {
        start = 1; fileCount=argc-1;
        sequential_rle(argv, start, fileCount);
    }
    return 0;
}
