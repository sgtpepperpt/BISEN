//
//  Definitions.hpp
//  BISEN
//
//  Created by Guilherme Borges on 27/07/17.
//  Copyright © 2017 Guilherme Borges. All rights reserved.
//

#ifndef Definitions_hpp
#define Definitions_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector> // TODO remove for SGX

#include "vec_int.h"

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

static const char* homePath = getenv("HOME_DIR") ? getenv("HOME_DIR") : "/Users/bernardo/"; //TODO make this better for interchangeability
//static const char* homePath = "/Users/tiago/";

static const char* clientIP = "127.0.0.1";//"54.194.253.119";
static const char* serverIP = "127.0.0.1";//"54.194.253.119";
static const int serverPort = 9978;
static const int clientPort = 9979;

// TOKEN DEFINITIONS
#define WORD_TOKEN 'w'
#define META_TOKEN 'z'
#define MAX_WORD_SIZE 32
typedef struct token {
    char type;
    int counter;
    char* word;
    vec_int docs;
} token;
// END TOKEN DEFINITIONS

#endif /* Definitions_hpp */
