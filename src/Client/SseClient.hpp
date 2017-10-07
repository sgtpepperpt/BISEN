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

#include "../Common/Utils.h"
#include "../Common/Definitions.h"
#include "EnglishAnalyzer.h"
#include "QueryParser.hpp"

extern "C" {
    #include "ClientCrypt.h"
}

using namespace std;

class SseClient {

protected:
    EnglishAnalyzer* analyzer;
    QueryParser* parser;
    map<string,int>* W;
    int querySocket;
    int nDocs;

    void openQueryResponseSocket();
    int acceptQueryResponseSocket();

public:
    SseClient();
    ~SseClient();

    unsigned long long setup(unsigned char** data);
    unsigned long long add_new_document(set<string> text, unsigned char** data);
    unsigned long long add_words(int doc_id, set<string> words, unsigned char** data);
    int search(string query, unsigned char** data);

    // functions for testing purposes
    string generate_random_query(vector<string> all_words, const int size, const int not_prob, const int and_prob);
    void listTxtFiles (string path, vector<string>& docs);
    set<string> extractUniqueKeywords(string fname);

private:
    int newDoc();
    string get_random_segment(vector<string> segments);    
};
    
#endif /* SseClient_hpp */
