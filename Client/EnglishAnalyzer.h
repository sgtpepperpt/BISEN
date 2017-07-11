//
//  EnglishAnalyzer.h
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 13/03/15.
//  Copyright (c) 2015 NovaSYS. All rights reserved.
//

#ifndef __BooleanSSE__EnglishAnalyzer__
#define __BooleanSSE__EnglishAnalyzer__

#include <vector>
#include <set>
#include <string>
#include <stdlib.h>
#include <ctype.h>
#include "ClientUtils.h"
#include "PorterStemmer.c"

using namespace std;

class EnglishAnalyzer {
    #define INC 50
    char * s;
    int i_max;
    set<string> stopWords;

    void increase_s();

public:
    EnglishAnalyzer();
    ~EnglishAnalyzer();
    set<string> extractUniqueKeywords(string fname);
    bool isStopWord(string word);
    
};
#endif /* defined(__BooleanSSE__EnglishAnalyzer__) */
