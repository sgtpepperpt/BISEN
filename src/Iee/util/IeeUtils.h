//
//  IeeUtils.h
//  BISEN
//
//  Created by Bernardo Ferreira on 29/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef IeeUtils_h
#define IeeUtils_h

#include <stdio.h> //TODO remove
#include <unistd.h> // read/write

#include "vec_int.h"

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

void iee_pee(const char *msg);

void iee_socketSend(int sockfd, unsigned char* buff, long size);
int iee_sendAll(int s, unsigned char *buf, long len);

void iee_socketReceive(int sockfd, unsigned char* buff, long size);
int iee_receiveAll (int s, unsigned char* buff, long len);

void iee_addToArr (void* val, int size, unsigned char* arr, int* pos);
void iee_readFromArr (void* val, int size, const unsigned char * arr, int* pos);

void iee_addIntToArr (int val, unsigned char* arr, int* pos);
int iee_readIntFromArr (const unsigned char * arr, int* pos);

void *iee_memcpy(void *dest, const void *src, size_t n);

#endif /* IeeUtils_h */
