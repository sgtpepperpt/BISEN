//
//  main.cpp
//  BooleanSSE (it's BISEN now)
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include <stdio.h>
#include "../Client/SseClient.hpp"
#include "../Server/SseServer.hpp"
#include "../Iee/SseIee.hpp"
// TODO : INCLUDES

void printResults (vector<int> results) {
    if(!results.size()) {
        printf("none\n");
        return;
    }

    for(int i = 0; i < results.size(); i++)
        printf("%i ", results[i]);
    printf("\n");
}

int main(int argc, const char * argv[]) {

    setvbuf(stdout, NULL, _IONBF, 0);

    SseServer server;
    server.setup();

    SseIee iee;

    SseClient client;
    char* data;
    int data_size = client.setup(data);

    ////////////////////////////////////////////////////////////////////////////
    // SETUP ///////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    char* output;
    int output_size = iee.f(data, data_size, output);

    const string base_dir = "../Test/parsed/";
    const int num_queries = 10;

    // get list of docs for test
    vector<string> doc_paths;
    client.listTxtFiles(base_dir, doc_paths);

    // add documents from the directory
    set<string> all_words_set;
    for(string doc : doc_paths){
        set<string> text = client.extractUniqueKeywords(base_dir + doc);

        // generate the byte* to send to the server
        char* data;
        int data_size = client.add_new_document(text, data);

        // TODO : CALL SERVER WITH BYTEARRAY
        // int SseIee::f(char* data, int data_size, char* output) {

        // add all new words to a set, used later to generate queries
        all_words_set.insert(text.begin(), text.end());
    }


    ////////////////////////////////////////////////////////////////////////////
    // QUERIES /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////


    // generate random queries
    vector<string> all_words(all_words_set.size());
    copy(all_words_set.begin(), all_words_set.end(), all_words.begin());

    for(int i = 0; i < num_queries; i++) {
        string query = client.generate_random_query(all_words);

        // TODO : CALL SERVER WITH BYTEARRAY
        char* data;
        int data_size = client.search(query, data);


        cout << query << endl;
    }

    return 0;
}
