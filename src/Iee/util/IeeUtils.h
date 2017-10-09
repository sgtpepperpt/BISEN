//
//  IeeUtils.h
//  BISEN
//
//  Created by Bernardo Ferreira on 29/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef IeeUtils_h
#define IeeUtils_h

//#include <stdio.h> //TODO remove
#include <unistd.h> // read/write

#include "vec_int.h"
#include "../ocall.h"

// IEE TOKEN DEFINITIONS
#define MAX_QUERY_TOKENS 25 // max tokens per query; vec_token should regrow anyway; iee-only restriction

#define WORD_TOKEN 'w'
#define META_TOKEN 'z'
#define MAX_WORD_SIZE 32 // iee-only restriction
typedef struct iee_token {
    char type;
    int counter;
    char* word;
    vec_int docs;
} iee_token;
// END IEE TOKEN DEFINITIONS

void ocall_printf(const char *fmt, ...);
ssize_t ocall_write(int fildes, const void *buf, size_t nbytes);
ssize_t ocall_read(int fildes, unsigned char* buf, size_t nbytes);
int ocall_open(const char *path, int oflags);
int ocall_close(int fildes);

void iee_pee(const char *msg);

void iee_socketSend(int sockfd, unsigned char* buff, unsigned long size);
int iee_sendAll(int s, unsigned char *buf, unsigned long len);

void iee_socketReceive(int sockfd, unsigned char* buff, unsigned long size);
int iee_receiveAll (int s, unsigned char* buff, unsigned long len);

void iee_addToArr (const void* val, int size, unsigned char* arr, int* pos);
void iee_readFromArr (const void* val, int size, const unsigned char * arr, int* pos);

void iee_addIntToArr (int val, unsigned char* arr, int* pos);
int iee_readIntFromArr (const unsigned char * arr, int* pos);

//void iee_add_ulonglong(unsigned long long val, unsigned char* arr, int* pos);
//unsigned long long iee_read_ulonglong(const unsigned char * arr, int* pos);

void iee_add_size_t(size_t val, unsigned char* arr, int* pos);
size_t iee_read_size_t(const unsigned char * arr, int* pos);

void iee_add_ssize_t(ssize_t val, unsigned char* arr, int* pos);
ssize_t iee_read_ssize_t(const unsigned char * arr, int* pos);

void *iee_memcpy(void *dest, const void *src, size_t n);
size_t iee_strlen(const char* str);

#endif /* IeeUtils_h */
