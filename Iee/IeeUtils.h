//
//  IeeUtils.hpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 29/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef IeeUtils_hpp
#define IeeUtils_hpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <queue>
#include <stack>

#include <stdint.h> // added to work on linux
#include <arpa/inet.h> // added to work on linux

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

static const char* homePath = getenv("HOME_DIR") ? getenv("HOME_DIR") : "/Users/bernardo/";

uint64_t pack754(long double f, unsigned bits, unsigned expbits);

long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

void pee(const char *msg);

void socketSend (int sockfd, char* buff, long size);

int sendall(int s, char *buf, long len);

void socketReceive(int sockfd, char* buff, long size);

int receiveAll (int socket, char* buff, long len);

void addToArr (void* val, int size, char* arr, int* pos);

void addIntToArr (int val, char* arr, int* pos);

void addFloatToArr (float val, char* arr, int* pos);

void readFromArr (void* val, int size, char* arr, int* pos);

int readIntFromArr (char* arr, int* pos);

float readFloatFromArr (char* arr, int* pos);

double readDoubleFromArr (char* arr, int* pos);

#endif /* IeeUtils_hpp */
