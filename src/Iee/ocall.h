#ifndef OCALL_H_
#define OCALL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h> //read / write / close
#include <fcntl.h> // open

#include "types.h"
#include "util/IeeUtils.h"

#define OCALL_READ 0x78
#define OCALL_WRITE 0x77
#define OCALL_OPEN 0x79
#define OCALL_CLOSE 0x80

void fserver(unsigned char **out, unsigned long long *outlen, const unsigned char * in, const unsigned long long inlen);

#endif /* OCALL_H_ */
