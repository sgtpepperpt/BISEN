//
//  MainUEE.hpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#ifndef SseServer_hpp
#define SseServer_hpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <err.h>
#include <assert.h>
#include <vector>
#include <map>
#include <pthread.h>
#include "ServerUtils.hpp"

using namespace std;

class SseServer {
    
private:
    static const char* pipeDir;

    int readIeePipe;
    int writeIeePipe;
    static int clientSock;
    map<vector<unsigned char>,vector<unsigned char> >* I;

    static void* bridgeClientIeeThread(void* threadData);
    vector<unsigned char> fillVector(unsigned char* array, int len);
    
public:
    SseServer();
    ~SseServer();
};

#endif /* MainUEE_hpp */
