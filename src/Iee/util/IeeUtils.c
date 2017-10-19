//
//  IeeUtils.cpp
//  MIE
//
//  Created by Bernardo Ferreira on 05/03/15.
//  Copyright (c) 2015 NovaSYS. All rights reserved.
//

#include "IeeUtils.h"

void iee_pee(const char *msg)
{
    ocall_strprint(msg);
    ocall_exit(-1);
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
        //printf("calling len %lu; r %ld\n", len, r);
        ssize_t n = ocall_read(socket, buff + r, len-r);
        //printf("got %lu\n-------------------------\n", n);
        if (n < 0) iee_pee("ERROR reading from socket");
        r+=n;
    }
    //printf("*********** finish iee_receiveAll ***********\n\n");
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

void iee_bzero(void *s, size_t n)
{
    for(unsigned i = 0; i < n; i++)
        *((char*)(s+i)) = 0x00;
}

char* iee_strcpy (char* dst, const char* src){
    unsigned i = 0;
    do {
        dst[i] = src[i];
    } while(src[i++]);
    return dst;
}
