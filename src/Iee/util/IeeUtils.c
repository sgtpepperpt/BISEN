//
//  IeeUtils.cpp
//  MIE
//
//  Created by Bernardo Ferreira on 05/03/15.
//  Copyright (c) 2015 NovaSYS. All rights reserved.
//

#include "IeeUtils.h"

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

    printf("%d\n", ocall_ret);
    for(int i = 0; i < ocall_ret; i++)
        buf[i] = (void*)out[i+8];

    return ocall_ret; //TODO check for errors
}

int ocall_open(const char *path, int oflags)
{
    size_t path_len = iee_strlen(path);

    size_t inlen = 2 * sizeof(int) + (path_len * sizeof(char));
    unsigned char* in = (unsigned char*)malloc(inlen);
    in[0] = OCALL_OPEN;
    printf("before path %d %s\n", path_len, path);
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


void iee_pee(const char *msg)
{
    perror(msg);
    exit(0);
    /*unsigned char * m = (unsigned char *)malloc(32 * sizeof(char));
    for(int i = 1; i < 32; i++)
        m[i] = msg[i];

    fserver(NULL, 0,m, 32);*/
}

void iee_socketSend (int sockfd, unsigned char* buff, unsigned long size) {
    if (iee_sendAll(sockfd, buff, size) < 0)
        iee_pee("ERROR writing to socket");
}

int iee_sendAll(int s, unsigned char *buf, unsigned long len)
{
    long total = 0;        // how many bytes we've sent
    long bytesleft = len; // how many we have left to send
    long n = 0;

    while(total < len) {
        n = ocall_write(s, buf+total, bytesleft);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    
    return n==-1||total!=len ? -1 : 0; // return -1 on failure, 0 on success
}

void iee_socketReceive(int sockfd, unsigned char* buff, unsigned long size) {
    if (iee_receiveAll(sockfd, buff, size) < 0)
        iee_pee("ERROR reading from socket");
}

int iee_receiveAll (int socket, unsigned char* buff, unsigned long len) {
    ssize_t r = 0;
    while (r < len) {
        printf("calling len %d; r %d\n", len, r);
        ssize_t n = ocall_read(socket, buff + r, len-r);
        printf("got %lu\n-------------------------\n", n);
        if (n < 0) iee_pee("ERROR reading from socket");
        r+=n;
    }
    printf("*********** finish iee_receiveAll ***********\n\n");
    return r;
}

void iee_addToArr(const void* val, int size, unsigned char* arr, int* pos) {
    iee_memcpy(&arr[*pos], val, size);
    *pos += size;
}

void iee_readFromArr(const void* val, int size, const unsigned char * arr, int* pos) {
    iee_memcpy((void*)val, &arr[*pos], size);
    *pos += size;
}

void iee_addIntToArr(int val, unsigned char* arr, int* pos) {
    iee_addToArr (&val, sizeof(int), arr, pos);
}

int iee_readIntFromArr(const unsigned char * arr, int* pos) {
    int x;
    iee_readFromArr(&x, sizeof(int), arr, pos);
    return x;
}
/*
void iee_add_ulonglong(unsigned long long val, unsigned char* arr, int* pos) {
    iee_addToArr (&val, sizeof(unsigned long long), arr, pos);
}

unsigned long long iee_read_ulonglong(const unsigned char * arr, int* pos) {
    unsigned long long x;
    iee_readFromArr(&x, sizeof(unsigned long long), arr, pos);
    return x;
}*/

void iee_add_size_t(size_t val, unsigned char* arr, int* pos) {
    iee_addToArr(&val, sizeof(size_t), arr, pos);
}

size_t iee_read_size_t(const unsigned char * arr, int* pos) {
    size_t x;
    iee_readFromArr(&x, sizeof(size_t), arr, pos);
    return x;
}

void iee_add_ssize_t(ssize_t val, unsigned char* arr, int* pos) {
    iee_addToArr(&val, sizeof(ssize_t), arr, pos);
}

ssize_t iee_read_ssize_t(const unsigned char * arr, int* pos) {
    ssize_t x;
    iee_readFromArr(&x, sizeof(ssize_t), arr, pos);
    return x;
}


void *iee_memcpy(void *dest, const void *src, size_t n)
{
    //http://clc-wiki.net/wiki/C_standard_library:string.h:memcpy#Implementation
    char *dp = dest;
    const char *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

size_t iee_strlen(const char* str)
{
    size_t len;
    for(len = 0; str[len]; len++);

    return len;
}
