#ifndef __OCALL_H_
#define __OCALL_H_

#include "fserver.h"
#include "util/IeeUtils.h"

void ocall_printf(const char *fmt, ...);

int ocall_open(const char *path, int oflags);
int ocall_close(int fildes);
ssize_t ocall_write(int fildes, const void *buf, size_t nbytes);
ssize_t ocall_read(int fildes, unsigned char* buf, size_t nbytes);

#endif /* __OCALL_H_ */
