//
//  IeeUtils.h
//  BISEN
//
//  Created by Bernardo Ferreira on 29/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef __IEEUTILS_H
#define __IEEUTILS_H

#include "vec_int.h"
#include "../ocall.h"

// IEE TOKEN DEFINITIONS
#define MAX_QUERY_TOKENS 25 // max tokens per query; vec_token should regrow anyway; iee-only restriction
#define MAX_WORD_SIZE 128 // iee-only restriction; to simplify buffer handling

#define WORD_TOKEN 'w'
#define META_TOKEN 'z'
typedef struct iee_token {
    char type;
    int counter;
    unsigned char* kW;
    vec_int docs;
} iee_token;
// END IEE TOKEN DEFINITIONS

#define min(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

void iee_pee(const char *msg);

void iee_socketSend(int sockfd, const void* buff, unsigned long size);
int iee_sendAll(int s, const void* buf, unsigned long len);

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
void iee_bzero(void *s, size_t n);
char* iee_strcpy (char* dst, const char* src);

#endif /* __IEEUTILS_H */
