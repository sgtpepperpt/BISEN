//
//  Utils.h
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 04/05/15.
//  Copyright (c) 2015 NovaSYS. All rights reserved.
//
#ifndef __BooleanSSE__Utils__
#define __BooleanSSE__Utils__

#include <pthread.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <stack>
#include <queue>
#include <stdint.h>
#include <math.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "zlib.h"
#include "portable_endian.h"
#include "Definitions.h"

#define  LOGI(...)  fprintf(stdout,__VA_ARGS__)

uint64_t pack754(long double f, unsigned bits, unsigned expbits);

long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

struct timespec getTime();

struct timespec diff(struct timespec start, struct timespec end);

double diffSec(struct timespec start, struct timespec end);

std::string getHexRepresentation(const unsigned char * Bytes, size_t Length);

void pee(const char *msg);

int sendall(int s, char *buf, long len);

int connectAndSend (char* buff, long size);

void socketSend (int sockfd, char* buff, long size);

void socketReceive(int sockfd, char* buff, long size);

//void socketReceiveAck(int sockfd);

int receiveAll (int s, char* buff, long len);

void addToArr (void* val, int size, char* arr, int* pos);

void addIntToArr (int val, char* arr, int* pos);

void addFloatToArr (float val, char* arr, int* pos);

void readFromArr (void* val, int size, char* arr, int* pos);

int readIntFromArr (char* arr, int* pos);

float readFloatFromArr (char* arr, int* pos);

double readDoubleFromArr (char* arr, int* pos);

#endif /* defined(__BooleanSSE__ClientUtils__) */
