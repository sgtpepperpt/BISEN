//
//  main.cpp
//  BooleanSSE (it's BISEN now)
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "../Client/SseClient.hpp"
#include "../Server/SseServer.hpp"
#include "../Iee/SseIee.hpp"
//#include "../Iee/SseIeeF.hpp"
#include "../Definitions.h"
#include "../Utils.h"

void printResults (vector<int> results) {
    #ifdef VERBOSE
    if(!results.size()) {
        printf("none\n");
        return;
    }

    sort (results.begin(), results.end());
    for(unsigned i = 0; i < results.size(); i++)
        printf("%i ", results[i]);
    printf("\n");
    #endif
}

/*
void f(
  char **out,
  size_t *outlen,
  size_t pid,
  char *in,
  size_t inlen
)
*/

int main(int argc, const char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

//    SseServer server;
    //server.setup();

    SseIee iee;

    SseClient client;
    char* data;
    int data_size = client.setup(&data);

    ////////////////////////////////////////////////////////////////////////////
    // SETUP ///////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    #ifdef VERBOSE
    printf("Starting IEE communication\n");
    #endif

    char* output;
    int output_size = iee.f(data, data_size, &output);
    //size_t output_size; f(&output, &output_size, 0, data, data_size);

    const string base_dir = "../Test/parsed/";
    const int num_queries = 1000;

    // get list of docs for test
    vector<string> doc_paths;
    client.listTxtFiles(base_dir, doc_paths);

    // add documents from the directory
    set<string> all_words_set;
    for(string doc : doc_paths){
        set<string> text = client.extractUniqueKeywords(base_dir + doc);

        // generate the byte* to send to the server
        char* data;
        int data_size = client.add_new_document(text, &data);

        // int SseIee::f(char* data, int data_size, char* output)
        output_size = iee.f(data, data_size, &output);
        //f(&output, &output_size, 0, data, data_size);

        // add all new words to a set, used later to generate queries
        all_words_set.insert(text.begin(), text.end());
    }

    ////////////////////////////////////////////////////////////////////////////
    // QUERIES /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // generate random queries
    vector<string> all_words(all_words_set.size());
    copy(all_words_set.begin(), all_words_set.end(), all_words.begin());

    for(unsigned i = 0; i < num_queries; i++) {
        string query = client.generate_random_query(all_words);

        #ifdef VERBOSE
        cout << "\n----------------------------\nquery: " << query << endl;
        #endif

        char* data;
        int data_size = client.search(query, &data);

        #ifdef VERBOSE
        printf("\n***** Entering IEE *****\n");
        #endif

        // int SseIee::f(char* data, int data_size, char* output)
        output_size = iee.f(data, data_size, &output);
        //f(&output, &output_size, 0, data, data_size);

        #ifdef VERBOSE
        printf("\n***** Leaving IEE *****\n");
        #endif

        //process results
        const int nDocs = output_size / sizeof(int);

        #ifdef VERBOSE
        printf("\n***** CLIENT RESULT *****\n");
        printf("number of docs: %d\n", nDocs);
        #endif

        vector<int> results(nDocs);
        int pos = 0;
        for (int i = 0; i < nDocs; i++) {
            results[i] = readIntFromArr((char*)output, &pos);
        }

        printResults(results);
    }

    return 0;
}
