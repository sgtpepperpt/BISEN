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
#define DEFAULT_QUERY_TOKENS 25 // max tokens per query; vec_token should regrow anyway; iee-only restriction

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

void pee(const char *msg);

void socketSend(int sockfd, char* buff, long size);
int sendAll(int s, char *buf, long len);

void socketReceive(int sockfd, char* buff, long size);
int receiveAll (int s, char* buff, long len);

void addToArr (void* val, int size, char* arr, int* pos);
void readFromArr (void* val, int size, char* arr, int* pos);

void addIntToArr (int val, char* arr, int* pos);
int readIntFromArr (char* arr, int* pos);

void *memcpy(void *dest, const void *src, size_t n);

#endif /* IeeUtils_h */
