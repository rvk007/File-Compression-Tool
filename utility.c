#include <sys/mman.h> // PROT_READ
#include <stdio.h> // NULL
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> // for permissions in open()
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "utility.h"

// char* itoa(int val)
// {
// 	int base = 10;
// 	static char buf[32] = {0};
// 	int i = 30;
// 	for(; val && i ; --i, val /= base)
//     {
//         buf[i] = "0123456789abcdef"[val % base];
//     }
// 	return &buf[i+1];
// }

// void update_compressed_data(char *compressed_data, char last_alpha, int last_count)
// {
//     char *str_alpha = malloc(1);
//     char *str_count = malloc(100000);

//     str_alpha[0] = last_alpha;
//     strcat(compressed_data, str_alpha);
//     str_count = itoa(last_count);
//     strcat(compressed_data, str_count);
// }
