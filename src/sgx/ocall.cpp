/* ocall.cpp
 * Assuming the code is compiled under linux
 */

// These includes may be optional
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int ocall_open(const char* filename, int mode) {
    return open(filename, mode);
}

int ocall_read(int file, void *buf, unsigned int size) {
    return read(file, buf, size);
}

int ocall_write(int file, void *buf, unsigned int size) {
    return write(file, buf, size);
}

void ocall_close(int file) {
    close(file);
}

int ocall_mknod(const char* filename, mode_t mode, dev_t dev) {
    return mknod(filename, mode, dev);
}
