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
#include <deque>
#include <set>
#include <algorithm>
#include <vector> //still needed?
#include <stack>
#include <map>

#include <stdint.h> // added to work on linux
#include <arpa/inet.h> // added to work on linux

#include "../Definitions.h"
#include "../Utils.h"

// IEE TOKEN DEFINITIONS
#define WORD_TOKEN 'w'
#define META_TOKEN 'z'
#define MAX_WORD_SIZE 32
typedef struct iee_token {
    char type;
    int counter;
    char* word;
    vec_int docs;
} iee_token;
// END IEE TOKEN DEFINITIONS

#endif /* IeeUtils_hpp */
