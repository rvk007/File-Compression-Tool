#include <sys/mman.h> // PROT_READ
#include <stdio.h> // NULL
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> // for permissions in open()
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "encode_file.h"
#include "utility.h"

// encode a single file using mmap
Data encode_file(char *filename, char last_alpha, int last_count, int fileindex)
{
    struct stat fs;
    Data data;
    char *str_alpha = malloc(1),*str_count = malloc(100000);
    int fd = open(filename, O_RDONLY, 0644), count = 1;
    data.compressed_data = malloc(100000);

    if(fstat(fd, &fs) == -1)
    {
        perror("Error while reading file stat");
    }
    char *file_content = mmap(NULL , fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    char alpha_prev = file_content[0];
    data.if_last_match = file_content[0] == last_alpha;
    if (!data.if_last_match && fileindex!=1)
    {
        printf("%c%u",last_alpha,last_count);
        fflush(stdout);
    }
    
    for (int i=1; i<=fs.st_size;i++)
    {
        if(file_content[i] == alpha_prev) count ++;
        else
        {   
            str_count = itoa(count);
            str_alpha[0] = alpha_prev;
            if (data.if_last_match)
            {
                str_count = itoa(last_count+count);
                // strcat(data.compressed_data, str_alpha);
                // strcat(data.compressed_data, str_count);
                printf("%c%u",alpha_prev,last_count+count);
                fflush(stdout);
                data.if_last_match = false;
            }
            else if (!isalpha(file_content[i]))
            {
                data.last_alpha = alpha_prev;
                data.last_count = count;
            }
            else 
            {
                // strcat(data.compressed_data, str_alpha);
                // strcat(data.compressed_data, str_count);
                printf("%c%u",alpha_prev,count);
                fflush(stdout);
            }
            alpha_prev = file_content[i];
            count = 1;
        }
    }
    data.if_last_match = file_content[0] == last_alpha;
    close(fd);
    return data;
}
