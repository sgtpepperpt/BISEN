//
//  SseIee.h
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 27/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef SseIee_hpp
#define SseIee_hpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <assert.h>
#include <vector>
#include <map>

using namespace std;


class SseIee {
    
private:
    static const char* pipeDir;
    
    int readServerPipe;
    int writeServerPipe;
    int clientBridgePipe;
    
    
public:
    SseIee();
    ~SseIee();
    
};


#endif /* SseIee_hpp */
