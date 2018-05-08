#ifndef __COMMANDS_H
#define __COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include "types.h"

// exported


long timeElapsed (struct timeval start, struct timeval end);

void generate_commands();
//size count_queries_file(FILE *f, size_t f_size);
size_t get_file_size(FILE *f);

#endif
