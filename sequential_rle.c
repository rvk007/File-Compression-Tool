#include <sys/mman.h> 
#include <stdio.h> 
#include <sys/stat.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <ctype.h> 
#include <stdbool.h>
#include <sys/mman.h>

#include "sequential_rle.h"

typedef struct data {
    char last_alpha;
    unsigned char last_count;
} Data;

void print_data(char alpha, unsigned char count){
	fwrite(&alpha, 1, 1, stdout);
	fwrite(&count, 1, 1, stdout);
}

// encode a single file using mmap
Data encode_file(char *filename, char last_alpha, int last_count, int fileindex){
    struct stat fs;
    Data data;
    unsigned char count = 1;

    int fd = open(filename, O_RDONLY, 0644);
    if(fstat(fd, &fs) == -1)
    {
        perror("Error while reading file stat");
    }

    char *file_content = mmap(NULL , fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    char alpha_prev = file_content[0];
    bool if_last_match = file_content[0] == last_alpha;
    if (!if_last_match && fileindex!=1) print_data(last_alpha, last_count);
    
    for (int i=1; i<=fs.st_size;i++)
    {
        if(file_content[i] == alpha_prev) count ++;
        else{   
            if (if_last_match)
            {   
                print_data(last_alpha, last_count+count);
                if_last_match = false;
            }
            else if (!isalpha(file_content[i]))
            {
                data.last_alpha = alpha_prev;
                data.last_count = count;
            }
            else print_data(alpha_prev, count);
            alpha_prev = file_content[i];
            count = 1;
        }
    }
    close(fd);
    return data;
}

void sequential_rle(char **argv, int start, int fileCount){
    char last_alpha;
    int last_count = 0;

    Data file_data;
    for(int i=start;i<start+fileCount;i++)
    {
        file_data = encode_file(argv[i], last_alpha, last_count, i);
        last_alpha = file_data.last_alpha;
        last_count = file_data.last_count;
    }
    print_data(last_alpha, last_count);
}
