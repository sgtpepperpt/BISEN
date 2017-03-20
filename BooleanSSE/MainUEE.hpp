//
//  MainUEE.hpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#ifndef MainUEE_hpp
#define MainUEE_hpp

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

class MainUEE {
    
private:
    static const char* pipeDir;
    
    int readTeePipe;
    int writeTeePipe;
    map<vector<unsigned char>,vector<unsigned char> >* I;
    map<vector<unsigned char>,vector<unsigned char> >* W;
    
    
    static void* bridgeClientTeeThread(void* threadData);
    void addKeyword (int newsockfd);
    void search (int newsockfd);
    void remove (int newsockfd);
    
public:
    MainUEE();
    ~MainUEE();
    
};

#endif /* MainUEE_hpp */
