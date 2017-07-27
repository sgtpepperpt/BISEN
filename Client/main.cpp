//
//  main.cpp
//  BooleanSSE (it's BISEN now)
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include <stdio.h>
#include "SseClient.hpp"

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
    SseClient client;
    char* data;
    int data_size = client.setup(data);

    const string base_dir = "../Test/parsed/";
    const int num_queries = 10;

    // get list of docs for test
    vector<string> doc_paths;
    client.listTxtFiles(base_dir, doc_paths);

    // add documents from the directory
    set<string> all_words_set;
    for(string doc : doc_paths){
        set<string> text = client.extractUniqueKeywords(base_dir + doc);

        char* data;
        int data_size = client.add_new_document(text, data);

        // add all new words to a set, used later to generate queries
        all_words_set.insert(text.begin(), text.end());
    }

    // generate random queries
    vector<string> all_words(all_words_set.size());
    copy(all_words_set.begin(), all_words_set.end(), all_words.begin());

    // TODO search the queries in BISEN
    for(int i = 0; i < num_queries; i++) {
        string query = client.generate_random_query(all_words);

        char* data;
        int data_size = client.search(query, data);

        cout << query << endl;
    }

    return 0;
}
