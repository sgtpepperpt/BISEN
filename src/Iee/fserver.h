#ifndef __FSERVER_H_
#define __FSERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h> // read / write / close
#include <fcntl.h> // open
#include <sys/stat.h> // mkdir

#include "types.h"
#include "util/IeeUtils.h"

#define OCALL_STRPRNT 0x99
#define OCALL_READ 0x67
#define OCALL_WRITE 0x68
#define OCALL_OPEN 0x69
#define OCALL_CLOSE 0x70
#define OCALL_EXIT 0x71

// printing
static void fs_strprint(bytes* out, size* outlen, const bytes in, const size inlen);

// pipe io
static void fs_open(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_close(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_read(bytes* out, size* outlen, const bytes in, const size inlen);
static void fs_write(bytes* out, size* outlen, const bytes in, const size inlen);

// misc
static void fs_exit(bytes* out, size* outlen, const bytes in, const size inlen);

extern void fserver(bytes* out, size* outlen, const bytes in, const size inlen);

#endif /* __FSERVER_H_ */
