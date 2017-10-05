//
//  IeeUtils.cpp
//  MIE
//
//  Created by Bernardo Ferreira on 05/03/15.
//  Copyright (c) 2015 NovaSYS. All rights reserved.
//

#include "IeeUtils.h"

void pee(const char *msg)
{
    perror(msg);
    exit(0);
    /*unsigned char * m = (unsigned char *)malloc(32 * sizeof(char));
    for(int i = 1; i < 32; i++)
        m[i] = msg[i];

    fserver(NULL, 0,m, 32);*/
}

void socketSend (int sockfd, char* buff, long size) {
    if (sendAll(sockfd, buff, size) < 0)
        pee("ERROR writing to socket");
}

int sendAll(int s, char *buf, long len)
{
    long total = 0;        // how many bytes we've sent
    long bytesleft = len; // how many we have left to send
    long n = 0;
    
    while(total < len) {
        n = write(s, buf+total, bytesleft);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    
    return n==-1||total!=len ? -1 : 0; // return -1 on failure, 0 on success
}

void socketReceive(int sockfd, char* buff, long size) {
    if (receiveAll(sockfd, buff, size) < 0)
        pee("ERROR reading from socket");
}

int receiveAll (int socket, char* buff, long len) {
    int r = 0;
    while (r < len) {
        ssize_t n = read(socket,&buff[r],len-r);
        if (n < 0) pee("ERROR reading from socket");
        r+=n;
    }
    return r;
}

void addToArr (void* val, int size, char* arr, int* pos) {
    memcpy(&arr[*pos], val, size);
    *pos += size;
}

void addIntToArr (int val, char* arr, int* pos) {
    addToArr (&val, sizeof(int), arr, pos);
}

void addFloatToArr (float val, char* arr, int* pos) {
    long x = (long)val;
    addToArr (&x, sizeof(long), arr, pos);
}

void readFromArr (void* val, int size, char* arr, int* pos) {
    memcpy(val, &arr[*pos], size);
    *pos += size;
}

int readIntFromArr (char* arr, int* pos) {
    int x;
    readFromArr(&x, sizeof(int), arr, pos);
    return x;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    //http://clc-wiki.net/wiki/C_standard_library:string.h:memcpy#Implementation
    char *dp = dest;
    const char *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}
