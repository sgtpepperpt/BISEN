#include "ocall.h"

void ocall_printf(const char *fmt, ...)
{
    //TODO do something
    /*#define BUFSIZ 512
    char buf[BUFSIZ+1] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf+1, BUFSIZ+1, fmt, ap);
    va_end(ap);

    // op code
    buf[0] = 0x70;

    unsigned char * output;
    unsigned long long output_size;
    fserver(&output, &output_size, buf, BUFSIZ+1);*/
}

int ocall_open(const char *path, int oflags)
{
    size_t path_len = iee_strlen(path);

    size_t inlen = 2 * sizeof(int) + (path_len * sizeof(char));
    unsigned char* in = (unsigned char*)malloc(inlen);
    in[0] = OCALL_OPEN;
    printf("before path %lu %s\n", path_len, path);
    int pos = 1;
    iee_addIntToArr(oflags, in, &pos);
    iee_addIntToArr(path_len, in, &pos);
    iee_addToArr((const void *)path, path_len, in, &pos);

    // execute ocall
    unsigned char * out;
    unsigned long long outlen;
    fserver(&out, &outlen, in, inlen);

    // read ocall output
    pos = 0;
    int ocall_ret = iee_readIntFromArr(out, &pos);
    printf("ret ocall: %d\n", ocall_ret);

    return ocall_ret; //TODO check for errors
}

int ocall_close(int fildes)
{
    size_t inlen = sizeof(int);
    unsigned char* in = (unsigned char*)malloc(inlen);
    in[0] = OCALL_CLOSE;

    int pos = 1;
    iee_addIntToArr(fildes, in, &pos);

    // execute ocall
    unsigned char * out;
    unsigned long long outlen;
    fserver(&out, &outlen, in, inlen);

    // read ocall output
    pos = 0;
    int ocall_ret = iee_readIntFromArr(out, &pos);
    printf("ret close ocall: %d\n", ocall_ret);

    return ocall_ret; //TODO check for errors
}

ssize_t ocall_read(int fildes, unsigned char* buf, size_t nbytes)
{
    size_t inlen = sizeof(unsigned char) + sizeof(int) + sizeof(size_t);
    unsigned char* in = (unsigned char*)malloc(inlen);
    in[0] = OCALL_READ;

    int pos = 1;
    iee_addIntToArr(fildes, in, &pos);
    iee_add_size_t(nbytes, in, &pos);
    printf("nbytes pre ocall: %lu\n", nbytes);

    // execute ocall, which returns the out buffer, whose data
    // we copy to the original buf, allocated outside of this call
    unsigned char * out;
    unsigned long long outlen;
    fserver(&out, &outlen, in, inlen);

    // read ocall output
    pos = 0;
    ssize_t ocall_ret = iee_read_ssize_t(out, &pos);
    printf("ret ocall: %lu\n", ocall_ret);

    printf("%ld\n", ocall_ret);
    
    for(int i = 0; i < ocall_ret; i++)
        buf[i] = (void*)out[i+8];

    return ocall_ret; //TODO check for errors
}

ssize_t ocall_write(int fildes, const void *buf, size_t nbytes)
{
    size_t buf_size = sizeof(unsigned char) + sizeof(int) + sizeof(size_t) + nbytes * sizeof(unsigned char);
    unsigned char* buffer = (unsigned char*)malloc(buf_size);
    buffer[0] = OCALL_WRITE;

    int pos = 1;
    iee_addIntToArr(fildes, buffer, &pos);
    iee_add_size_t(nbytes, buffer, &pos);
    iee_addToArr((const void *)buf, nbytes, buffer, &pos);

    // execute ocall
    unsigned char * output;
    unsigned long long output_size;
    fserver(&output, &output_size, buffer, buf_size);

    // read ocall output
    pos = 0;
    ssize_t ocall_ret = iee_read_ssize_t(output, &pos);
    //printf("ret: %lu\n", ocall_ret);

    free(buffer);
    free(output);
    return ocall_ret; //TODO check for errors
}

static void fs_open(bytes* out, size* outlen, const bytes in, const size inlen)
{
    printf("Open ocall\n");

    // read values from buffer
    int pos = 1;
    int oflags = iee_readIntFromArr(in, &pos);
    int path_len = iee_readIntFromArr(in, &pos);

    //TODO pass string as last element, \0 will cut it off in open
    char* path = (char*)malloc(sizeof(char) * path_len);
    iee_readFromArr(path, path_len, in, &pos);
    printf("path %d %s\n", path_len, path);
    // execute open syscall
    int res = open(path, oflags);
    printf("ret open: %d\n", res);

    // prepare response
    pos = 0;
    *outlen = sizeof(int);
    *out = (void *)malloc(*outlen);

    iee_addIntToArr(res, *out, &pos);
}

static void fs_close(bytes* out, size* outlen, const bytes in, const size inlen)
{
    printf("CLOSE ocall\n");

    // read values from buffer
    int pos = 1;
    int fildes = iee_readIntFromArr(in, &pos);

    // execute close syscall
    int res = close(fildes);
    printf("ret close: %d\n", res);

    // prepare response
    pos = 0;
    *outlen = sizeof(int);
    *out = (void *)malloc(*outlen);

    iee_addIntToArr(res, *out, &pos);
}

static void fs_read(bytes* out, size* outlen, const bytes in, const size inlen)
{
    printf("Read ocall\n");

    // read values from buffer
    int pos = 1;
    int fildes = iee_readIntFromArr(in, &pos);
    size_t nbytes = iee_read_size_t(in, &pos);
    printf("nbytes in ocall: %lu\n", nbytes);

    // write return value to output
    // TODO this buffer should hold both the return and the read byte array from start
    *outlen = sizeof(ssize_t) + sizeof(unsigned char) * nbytes;
    *out = (void *)malloc(*outlen);

    // execute read syscall
    ssize_t res = read(fildes, *out+8, nbytes);
    printf("ret read: %lu\n", res);

    pos = 0;
    iee_add_ssize_t(res, *out, &pos);
    //iee_addToArr(((const void *)out)+8, sizeof(unsigned char) * nbytes, *out, &pos);
    for(unsigned i = 0; i < nbytes; i++)
        printf("%02x", ((unsigned char*)out)[i+8]);
    printf("\n");
}

static void fs_write(bytes* out, size* outlen, const bytes in, const size inlen)
{
    //printf("Write ocall\n");

    // read values from buffer
    int pos = 1;
    int fildes = iee_readIntFromArr(in, &pos);
    size_t nbytes = iee_read_size_t(in, &pos);

    void *buf = (void *)malloc(sizeof(unsigned char) * nbytes);
    iee_readFromArr(buf, nbytes, in, &pos);
 
    // execute write syscall
    ssize_t res = write(fildes, buf, nbytes);
    //printf("ret: %lu\n", res);

    // write output
    pos = 0;
    *out = (unsigned char *)malloc(sizeof(ssize_t));
    *outlen = sizeof(ssize_t);

    iee_add_ssize_t(res, *out, &pos);
}

static void fserver(bytes* out, size* outlen, const bytes in, const size inlen)
{
    // set out variables
    *out = NULL;
    *outlen = 0;

    if(in[0] == OCALL_WRITE)
        fs_write(out, outlen, in, inlen);

    else if(in[0] == OCALL_READ)
        fs_read(out, outlen, in, inlen);

    else if(in[0] == OCALL_OPEN)
        fs_open(out, outlen, in, inlen);

    else if(in[0] == OCALL_CLOSE)
        fs_close(out, outlen, in, inlen);
}
