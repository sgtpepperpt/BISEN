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
    perror(msg);
    exit(0);
    /*unsigned char * m = (unsigned char *)malloc(32 * sizeof(char));
    for(int i = 1; i < 32; i++)
        m[i] = msg[i];

    fserver(NULL, 0,m, 32);*/
}

void iee_socketSend (int sockfd, unsigned char* buff, long size) {
    if (iee_sendAll(sockfd, buff, size) < 0)
        iee_pee("ERROR writing to socket");
}

int iee_sendAll(int s, unsigned char *buf, long len)
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

void iee_socketReceive(int sockfd, unsigned char* buff, long size) {
    if (iee_receiveAll(sockfd, buff, size) < 0)
        iee_pee("ERROR reading from socket");
}

int iee_receiveAll (int socket, unsigned char* buff, long len) {
    int r = 0;
    while (r < len) {
        ssize_t n = read(socket,&buff[r],len-r);
        if (n < 0) iee_pee("ERROR reading from socket");
        r+=n;
    }
    return r;
}

void iee_addToArr (void* val, int size, unsigned char* arr, int* pos) {
    iee_memcpy(&arr[*pos], val, size);
    *pos += size;
}

void iee_addIntToArr (int val, unsigned char* arr, int* pos) {
    iee_addToArr (&val, sizeof(int), arr, pos);
}

void iee_readFromArr (void* val, int size, const unsigned char * arr, int* pos) {
    iee_memcpy(val, &arr[*pos], size);
    *pos += size;
}

int iee_readIntFromArr (const unsigned char * arr, int* pos) {
    int x;
    iee_readFromArr(&x, sizeof(int), arr, pos);
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
