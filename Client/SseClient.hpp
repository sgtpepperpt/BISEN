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
#include "QueryParser.hpp"

using namespace std;

class SseClient {

protected:
    EnglishAnalyzer* analyzer;
    ClientCrypt* crypto;
    QueryParser* parser;
    map<string,int>* W;
    int querySocket;
    int nDocs;

    void openQueryResponseSocket();
    int acceptQueryResponseSocket();

public:
    SseClient();
    ~SseClient();

    void setup();

    set<string> extractUniqueKeywords(string fname);
    void addDocument(set<string> text);
    vector<int> search(string query);
    //void addDocs(string textDataset);
    void listTxtFiles (string path, vector<string>& docs);
    string generate_random_query(vector<string> all_words);

private:
    int newDoc();
    void addWords(int d, set<string> words);
    string get_random_segment(vector<string> segments);
};
    
#endif /* SseClient_hpp */
