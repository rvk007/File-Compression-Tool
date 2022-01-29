#include <sys/mman.h> 
#include <stdio.h> 
#include <sys/stat.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <ctype.h> 
#include <stdbool.h>
#include <sys/mman.h>

#include "sequential_rle.h"

char last_alpha;
unsigned char last_count;

// print binary data
void print_data(char alpha, unsigned char count){
	fprintf(stdout, "%c%c", alpha, count);
}

// encode a single file using mmap
void encode_file(char *filename, int fileindex) {
    struct stat fs;
    unsigned char alpha_count = 1;

    int fd = open(filename, O_RDONLY, 0644);
    if(fstat(fd, &fs) == -1) {
        perror("Error while reading file stat");
    }

    char *file_content = mmap(NULL , fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    char alpha_prev = file_content[0];
    int ifSame=0;

    if(fileindex){
        if (last_alpha == file_content[0]) ifSame=1;
        else print_data(last_alpha, last_count);
    }

    for (int i=1; i<fs.st_size;i++) {
        if(file_content[i] == alpha_prev) alpha_count ++;
        else{   
            if (ifSame) {   
                print_data(last_alpha, last_count+alpha_count);
                ifSame = 0;
            }
            else print_data(alpha_prev, alpha_count);
            alpha_prev = file_content[i];
            alpha_count = 1;
        }
    }
    last_alpha = alpha_prev;
    last_count = alpha_count;
    
}

// sequential encode
void sequential_rle(char **argv, int start, int fileCount){

    int fileIndex = 0;
    for(int i=start;i<start+fileCount;i++) {
        encode_file(argv[i], fileIndex);
        fileIndex++;
    }
    print_data(last_alpha, last_count);
}
