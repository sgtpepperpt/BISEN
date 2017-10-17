#ifndef __COMMANDS_H
#define __COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include "types.h"

// exported
size test_len;
bytes* commands;
size* commands_sizes;

void generate_commands();
size count_queries_file(FILE *f, size_t f_size);
size_t get_file_size(FILE *f);

#endif
