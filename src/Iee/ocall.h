#ifndef __OCALL_H_
#define __OCALL_H_

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

void ocall_printf(const char *fmt, ...);
ssize_t ocall_write(int fildes, const void *buf, size_t nbytes);
ssize_t ocall_read(int fildes, unsigned char* buf, size_t nbytes);
int ocall_open(const char *path, int oflags);
int ocall_close(int fildes);

static void fs_open(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_close(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_read(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_write(bytes* out, size* outlen, const bytes in, const size inlen);
static void fserver(bytes* out, size* outlen, const bytes in, const size inlen);

#endif /* __OCALL_H_ */
