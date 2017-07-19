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
    client.setup();

    // get list of docs for test
    const string base_dir = "../Test/parsed/";

    vector<string> docs;
    client.listTxtFiles(base_dir, docs);

    // add documents from the directory
    for(string doc : docs)
        client.addDocument(base_dir + doc);

    // TODO generate random queries

    return 0;
}
