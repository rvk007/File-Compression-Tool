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

int main(int argc,char **argv)
{   
    char last_alpha;
    int last_count = 0;
    bool if_last_match = false;
    // char *compressed_data = malloc(1000000);

    Data file_data;
    for(int i=1;i<=argc-1;i++)
    {
        file_data = encode_file(argv[i], last_alpha, last_count, i);
        // if (file_data.if_last_match) x =10;//strcat(compressed_data, file_data.compressed_data);
        // else
        // {
        //     printf("%c%u",last_alpha,last_count);
        //     fflush(stdout);
        //     // update_compressed_data(compressed_data, last_alpha, last_count);
        //     // strcat(compressed_data, file_data.compressed_data);
        // }
        last_alpha = file_data.last_alpha;
        last_count = file_data.last_count;
        if_last_match = file_data.if_last_match;
    }
    printf("%c%u",last_alpha,last_count);
    fflush(stdout);
    // update_compressed_data(compressed_data, last_alpha, last_count);
    
}
