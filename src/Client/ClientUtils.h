//
//  Utils.h
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 04/05/15.
//  Copyright (c) 2015 NovaSYS. All rights reserved.
//
#ifndef __BooleanSSE__ClientUtils__
#define __BooleanSSE__ClientUtils__

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
#include <string>
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

#include "../Common/Definitions.h"
#include "../Common/Utils.h"

// CLIENT TOKEN DEFINITIONS
// client tokens do not need the doc vector, as the iee ones do
#define WORD_TOKEN 'w'
#define META_TOKEN 'z'
typedef struct client_token {
    char type;
    int counter;
    std::string word;
} client_token;
// END CLIENT TOKEN DEFINITIONS

#endif /* defined(__BooleanSSE__ClientUtils__) */
