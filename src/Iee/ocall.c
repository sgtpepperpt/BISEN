#include "ocall.h"

void ocall_printf(const char *fmt, ...)
{
    //DEBUG ONLY, NOT WORKING IN SGX
/*    #undef BUFSIZ
    #define BUFSIZ 512
    #include <stdarg.h>
    char buf[BUFSIZ+1] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf+1, BUFSIZ+1, fmt, ap);
    va_end(ap);

    ocall_strprint(buf);*/
}

void ocall_strprint(const char *str)
{
    size_t len = iee_strlen(str);

    size_t inlen = sizeof(int) + (len * sizeof(char));
    unsigned char* in = (unsigned char*)malloc(inlen);
    in[0] = OCALL_STRPRNT;

    int pos = 1;
    iee_addToArr((const void *)str, len+1, in, &pos); // len + 1 to include \0

    // execute ocall
    unsigned char * out;
    unsigned long long outlen;
    fserver(&out, &outlen, in, inlen);
    free(in);

    // read ocall output
    pos = 0;
    int ocall_ret = iee_readIntFromArr(out, &pos);
/*    untrusted_free_bytes(&out);*/
    //printf("ret ocall: %d\n", ocall_ret);// not ocall_printf, recursion...
}

int ocall_open(const char *path, int oflags)
{
    size_t path_len = strlen(path)+1;

    size_t inlen = sizeof(int) + sizeof(size_t) + (path_len * sizeof(char));
    unsigned char* in = (unsigned char*)malloc(inlen);
    in[0] = OCALL_OPEN;

    int pos = 1;
    iee_addIntToArr(oflags, in, &pos);
    iee_add_size_t(path_len, in, &pos);
    iee_addToArr((const void *)path, path_len, in, &pos);

    // execute ocall
    unsigned char * out;
    unsigned long long outlen;
    fserver(&out, &outlen, in, inlen);
    free(in);

    // read ocall output
    pos = 0;
    int ocall_ret = iee_readIntFromArr(out, &pos);
/*    untrusted_free_bytes(&out);*/
    ocall_printf("ret ocall_open: %d\n", ocall_ret);

    return ocall_ret;
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
    free(in);

    // read ocall output
    pos = 0;
    int ocall_ret = iee_readIntFromArr(out, &pos);
/*    untrusted_free_bytes(&out);*/
    //ocall_printf("ret close ocall: %d\n", ocall_ret);

    return ocall_ret;
}

ssize_t ocall_read(int fildes, unsigned char* buf, size_t nbytes)
{
    size_t inlen = sizeof(unsigned char) + sizeof(int) + sizeof(size_t);
    unsigned char* in = (unsigned char*)malloc(inlen);
    in[0] = OCALL_READ;

    int pos = 1;
    iee_addIntToArr(fildes, in, &pos);
    iee_add_size_t(nbytes, in, &pos);
    //ocall_printf("nbytes pre ocall: %lu\n", nbytes);

    // execute ocall, which returns the out buffer, whose data
    // we copy to the original buf, allocated outside of this call
    unsigned char * out;
    unsigned long long outlen;
    fserver(&out, &outlen, in, inlen);
    free(in);

    // read ocall output
    pos = 0;
    ssize_t ocall_ret = iee_read_ssize_t(out, &pos);
    //ocall_printf("ret ocall_read: %lu\n", ocall_ret);

    //ocall_printf("%ld\n", ocall_ret);
    
    for(int i = 0; i < ocall_ret; i++)
        buf[i] = (void*)out[i+8];
/*    untrusted_free_bytes(&out);*/

    return ocall_ret;
}

ssize_t ocall_write(int fildes, const void *buf, size_t nbytes)
{
    size_t buf_size = sizeof(unsigned char) + sizeof(int) + sizeof(size_t) + nbytes * sizeof(unsigned char);
    unsigned char* buffer = (unsigned char*)malloc(buf_size);
    buffer[0] = OCALL_WRITE;
    //ocall_printf("ocall write %lu\n", nbytes);
    int pos = 1;
    iee_addIntToArr(fildes, buffer, &pos);
    iee_add_size_t(nbytes, buffer, &pos);
    iee_addToArr((const void *)buf, nbytes, buffer, &pos);

    // execute ocall
    unsigned char * output;
    unsigned long long output_size;
    fserver(&output, &output_size, buffer, buf_size);
    free(buffer);

    //ocall_strprint("done out of ocall\n");
    // read ocall output
    pos = 0;
    ssize_t ocall_ret = iee_read_ssize_t(output, &pos);
/*    untrusted_free_bytes(&output);*/
    //ocall_printf("ret: %lu\n", ocall_ret);

    //free(output);
    //ocall_strprint("here\n");

    return ocall_ret;
}

int ocall_exit(int status)
{
    size_t inlen = sizeof(int);
    unsigned char* in = (unsigned char*)malloc(inlen);
    in[0] = OCALL_EXIT;

    int pos = 1;
    iee_addIntToArr(status, in, &pos);

    // execute ocall
    unsigned char * out;
    unsigned long long outlen;
    fserver(&out, &outlen, in, inlen);
    free(in);

    // read ocall output
    pos = 0;
    int ocall_ret = iee_readIntFromArr(out, &pos);
/*    untrusted_free_bytes(&out);*/
    //ocall_printf("ret status ocall: %d\n", ocall_ret);

    return ocall_ret;
}
