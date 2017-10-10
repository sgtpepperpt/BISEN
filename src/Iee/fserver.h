#ifndef __FSERVER_H_
#define __FSERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h> //read / write / close
#include <fcntl.h> // open
#include <sys/stat.h> // mkdir

#include "types.h"
#include "util/IeeUtils.h"

#define OCALL_READ 0x78
#define OCALL_WRITE 0x77
#define OCALL_OPEN 0x79
#define OCALL_CLOSE 0x80

static void fs_open(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_close(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_read(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_write(bytes* out, size* outlen, const bytes in, const size inlen);
void fserver(bytes* out, size* outlen, const bytes in, const size inlen);

#endif /* __FSERVER_H_ */
