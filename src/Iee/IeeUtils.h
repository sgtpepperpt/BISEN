//
//  IeeUtils.h
//  BISEN
//
//  Created by Bernardo Ferreira on 29/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef IeeUtils_hpp
#define IeeUtils_hpp

#include <stdio.h>
#include <stdlib.h>

#include "vec_int.h"
#include "../Definitions.h"

#define DEFAULT_QUERY_TOKENS 25 // max tokens per query; vec_token should regrow anyway; iee-only restriction

//static const char* homePath = getenv("HOME_DIR") ? getenv("HOME_DIR") : "/Users/bernardo/";


// IEE TOKEN DEFINITIONS
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
int sendall(int s, char *buf, long len);
int connectAndSend (char* buff, long size);
void socketSend (int sockfd, char* buff, long size);
void socketReceive(int sockfd, char* buff, long size);
int receiveAll (int s, char* buff, long len);

void addToArr (void* val, int size, char* arr, int* pos);
void addIntToArr (int val, char* arr, int* pos);

void readFromArr (void* val, int size, char* arr, int* pos);
int readIntFromArr (char* arr, int* pos);

#endif /* IeeUtils_hpp */
