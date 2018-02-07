#include "fserver.h"

static void fs_strprint(bytes* out, size* outlen, const bytes in, const size inlen)
{
    // read values from buffer
    int pos = 1;

    // execute printf syscall
    printf("%s", in+1); // ignore op char

    // prepare response
    pos = 0;
    *outlen = sizeof(int);
    *out = (void *)malloc(*outlen);

    iee_addIntToArr(0, *out, &pos);
}

static void fs_open(bytes* out, size* outlen, const bytes in, const size inlen)
{
    //printf("Open ocall\n");

    // read values from buffer
    int pos = 1;
    int oflags = iee_readIntFromArr(in, &pos);
    size_t path_len = iee_read_size_t(in, &pos)-1;

    char* path = (char*)malloc(sizeof(char) * (path_len+1));
    strncpy(path, (const char *)in + pos, path_len);
    path[path_len] = 0x00;

    //printf("path %s %d %lu %02x\n", path, oflags, path_len,in[pos + path_len-2]);
    // execute open syscall
    int res = open(path, oflags);
    //printf("ret open: %d\n", res);

    free(path);

    // prepare response
    pos = 0;
    *outlen = sizeof(int);
    *out = (void *)malloc(*outlen);

    iee_addIntToArr(res, *out, &pos);
}

static void fs_close(bytes* out, size* outlen, const bytes in, const size inlen)
{
    //printf("CLOSE ocall\n");

    // read values from buffer
    int pos = 1;
    int fildes = iee_readIntFromArr(in, &pos);

    // execute close syscall
    int res = close(fildes);
    //printf("ret close: %d\n", res);

    // prepare response
    pos = 0;
    *outlen = sizeof(int);
    *out = (void *)malloc(*outlen);

    iee_addIntToArr(res, *out, &pos);
}

static void fs_read(bytes* out, size* outlen, const bytes in, const size inlen)
{
    //printf("Read ocall\n");

    // read values from buffer
    int pos = 1;
    int fildes = iee_readIntFromArr(in, &pos);
    size_t nbytes = iee_read_size_t(in, &pos);
    //printf("nbytes in ocall: %lu\n", nbytes);

    // write return value to output
    // TODO this buffer should hold both the return and the read byte array from start
    *outlen = sizeof(ssize_t) + sizeof(unsigned char) * nbytes;
    *out = (void *)malloc(*outlen);

    // execute read syscall
    ssize_t res = read(fildes, *out+8, nbytes);
    //printf("ret read: %lu\n", res);

    pos = 0;
    iee_add_ssize_t(res, *out, &pos);
    //iee_addToArr(((const void *)out)+8, sizeof(unsigned char) * nbytes, *out, &pos);
    /*for(unsigned i = 0; i < nbytes; i++)
        printf("%02x", ((unsigned char*)out)[i+8]);
    printf("\n");*/
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
    free(buf);

    // write output
    pos = 0;
    *out = (unsigned char *)malloc(sizeof(ssize_t));
    *outlen = sizeof(ssize_t);

    iee_add_ssize_t(res, *out, &pos);
    //printf("leave write\n");
}

static void fs_exit(bytes* out, size* outlen, const bytes in, const size inlen)
{
    printf("EXIT ocall\n");

    // read values from buffer
    int pos = 1;
    int status = iee_readIntFromArr(in, &pos);

    // execute close syscall
    exit(status);
    printf("We definitely should't be running after exiting...");

    // prepare response
    pos = 0;
    *outlen = sizeof(int);
    *out = (void *)malloc(*outlen);

    iee_addIntToArr(-1, *out, &pos);
}

void fserver(bytes* out, size* outlen, const bytes in, const size inlen)
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

    else if(in[0] == OCALL_STRPRNT)
        fs_strprint(out, outlen, in, inlen);

    else if(in[0] == OCALL_EXIT)
        fs_exit(out, outlen, in, inlen);
}
