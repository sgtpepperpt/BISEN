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

#include "EnglishAnalyzer.h"
#include "ClientCrypt.hpp"
#include "ClientUtils.h"

using namespace std;


class SseClient {
    
protected:

    EnglishAnalyzer* analyzer;
    ClientCrypt* crypto;
    map<vector<unsigned char>,vector<unsigned char> >* W;
    
    void listTxtFiles (std::string path, std::vector<std::string>& docs);
    
public:
    SseClient();
    ~SseClient();
    
    void setup();
    void addDocs(string textDataset);
    vector<int> search(string query);
};
    
#endif /* SseClient_hpp */
