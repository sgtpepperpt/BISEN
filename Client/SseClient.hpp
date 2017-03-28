//
//  SSE_simple.hpp
//  BooleanSSE
//
//  A simple SSE scheme, with all required functionality. Includes both client and server functionality
//  in one class, for simplification. Inline comments differentiate respective parts of the code.
//
//  Created by Bernardo Ferreira on 20/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef SseClient_hpp
#define SseClient_hpp

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <string>
#include "EnglishAnalyzer.h"
#include "SSECrypt.hpp"
#include "Utils.h"

using namespace std;


class SseClient {
    
protected:

    EnglishAnalyzer* analyzer;
    SseCrypt* crypto;
    
    map<vector<unsigned char>,vector<unsigned char> >* encTextIndex;
    map<string,int>* textDcount;
    
    void listTxtFiles (std::string path, std::vector<std::string>& docs);
    
public:
    SseClient();
    ~SseClient();
    
    void addDocs(string textDataset, int first, int last, int prefix);
    vector<int> search(string textPath);
};
    
#endif /* SseClient_hpp */
