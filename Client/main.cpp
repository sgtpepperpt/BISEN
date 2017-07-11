//
//  main.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include <stdio.h>
#include "SseClient.hpp"

void printResults (vector<int> results) {
    for(int i = 0; i < results.size(); i++)
        printf("%i ", results[i]);
    printf("\n");
}

int main(int argc, const char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    SseClient client;

    client.setup();
    /*
    for(int i = 0; i < 4; i++)
        client.newDoc();

    client.addWord(0, "ola");
    client.addWord(1, "ola");
    client.addWord(2, "ola");

    client.addWord(1, "viva");
    client.addWord(3, "viva");

    client.addWord(1, "camarada");

    client.addWord(1, "howdy");
    client.addWord(3, "howdy");

    printResults( client.search("! (! camarada)") );
    printResults( client.search("ola && viva") );
    printResults( client.search("ola && ( viva || howdy )") );
    printResults( client.search("camarada") );
    printResults( client.search("howdy") );*/


    client.addDocument("text/1984_1.txt");
    client.addDocument("text/1984_2.txt");

    printResults(client.search("big && brother && !chestnut"));

//    client.addDocs("/Users/bernardo/Datasets/flickr_tags");
//    client.addDocs("/Users/bernardo/Datasets/enron");
    return 0;
}
