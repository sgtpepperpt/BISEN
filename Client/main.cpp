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

    client.add(1, "ola");
    client.add(2, "ola");
    client.add(3, "ola");

    client.add(1, "viva");
    client.add(3, "viva");

    client.add(3, "camarada");

    client.add(2, "howdy");
    client.add(4, "howdy");

    printResults( client.search("ola && viva") );
    printResults( client.search("ola && viva || howdy") );
    //printResults( client.search("camarada") );
    //printResults( client.search("howdy") );



//    client.addDocs("/Users/bernardo/Datasets/flickr_tags");
//    client.addDocs("/Users/bernardo/Datasets/enron");
    return 0;
}


