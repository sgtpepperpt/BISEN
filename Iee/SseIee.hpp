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
#include <string.h>
#include "IeeUtils.h"
#include "IeeCrypt.hpp"
#include "QueryEvaluator.hpp"

using namespace std;


class SseIee {
    
private:
    static const char* pipeDir;
    
    IeeCrypt* crypto;
    
    int readServerPipe;
    int writeServerPipe;
    int clientBridgePipe;
    
    void initIee();
    void setup(unsigned char* enc_data, int enc_data_size);
    void add(char* data, int data_size);
    void search(char* data, int data_size);
    void get_docs_from_server(deque<token> &query);
    
public:
    SseIee();
    ~SseIee();
};


#endif /* SseIee_hpp */
