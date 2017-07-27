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
#include "../Definitions.h"
#include "../Utils.h"

void printResults (vector<int> results) {
    if(!results.size()) {
        printf("none\n");
        return;
    }

    for(unsigned i = 0; i < results.size(); i++)
        printf("%i ", results[i]);
    printf("\n");
}

int main(int argc, const char * argv[]) {

    setvbuf(stdout, NULL, _IONBF, 0);

    SseServer server;

    return 0;
}
