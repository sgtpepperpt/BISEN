//
//  main.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include <stdio.h>
#include "SseClient.hpp"

int main(int argc, const char * argv[]) {
    SseClient client;
    client.addDocs("/Users/bernardo/Datasets/flickr_tags");
//    client.addDocs("/Users/bernardo/Datasets/enron");
    
    return 0;
}
