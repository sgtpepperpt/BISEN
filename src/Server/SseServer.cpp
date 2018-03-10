//
//  MainUEE.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include "SseServer.hpp"

using namespace std;

const char* SseServer::pipeDir = "/tmp/BooleanSSE/";
//int SseServer::clientSock;

SseServer::SseServer() {
    //init pipe directory
    if(mkdir(pipeDir, 0770) == -1) {
        if(errno != EEXIST)
            pee("Failed to mkdir");
    }

    //create server-iee pipe
    char pipeName[256];
    bzero(pipeName,256);
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "server_to_iee");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1) {
        if(errno != EEXIST)
            pee("Fail to mknod");
    }

    //create iee-server pipe
    char pipeName2[256];
    strcpy(pipeName2, pipeDir);
    strcpy(pipeName2+strlen(pipeName2), "iee_to_server");
    if(mknod(pipeName2, S_IFIFO | 0770, 0) == -1) {
        if(errno != EEXIST)
            pee("Fail to mknod");
    }

    printf("Opening write pipe!\n");
	writeIeePipe = open(pipeName, O_ASYNC | O_WRONLY);

	//start listening on pipes; must open write first as read blocks
	printf("Opening read pipe!\n");
    readIeePipe = open(pipeName2, O_ASYNC | O_RDONLY);

    //launch client-iee tunnel
/*	//not being used anymore, client comunicates with iee directly for testing
	pthread_t t;
    if (pthread_create(&t, NULL, bridgeClientIeeThread, NULL) != 0)
        pee("Error: unable to create bridgeClientTeeThread");
 */
    //start listening for iee calls
    printf("Finished Server init! Gonna start listening for IEE requests!\n");
    long total_add_time = 0;
    long total_search_time = 0;
    size_t nr_search_batches = 0;

    while (true) {
        unsigned char cmd;
        socketReceive(readIeePipe, &cmd, sizeof(unsigned char));

        switch (cmd) {
            //setup
            case '1': {
                // delete existing map, only useful for reruns while debugging

                if(I.size()) {
                    //#ifdef VERBOSE
                    printf("Size before: %d\n", I.size());
                    //#endif

                    unordered_map<void *, void *, VoidHash, VoidEqual>::iterator it;
                    for(it = I.begin(); it != I.end(); ++it) {
                        free(it->first);
                        free(it->second);
                    }

                    I.clear();

                    //delete I;

                    //#ifdef VERBOSE
                    printf("Cleared map!\n");
                    printf("Size now: %d\n", I.size());
                    //#endif
                }

                #ifdef VERBOSE
                printf("Finished Setup!\n");
                #endif
                break;
            }
            // add
            case '2': {
                struct timeval start, end;
                #ifdef VERBOSE
                printf("Started Add!\n");
                #endif

                gettimeofday(&start, NULL);
                void* l = malloc(l_size);
                socketReceive(readIeePipe, (unsigned char*)l, l_size);

                void* d = malloc(d_size);
                socketReceive(readIeePipe, (unsigned char*)d, d_size);

                I[l] = d;

                gettimeofday(&end, NULL);
                total_add_time += util_time_elapsed(start, end);

                #ifdef VERBOSE
                printf("Finished Add!\n");
                #endif
                break;
            }
            // search - get index positions
            case '3': {
                #ifdef VERBOSE
                printf("Started Search!\n");
                #endif

                struct timeval start2, end2;
                gettimeofday(&start2, NULL);

                unsigned counter;
                socketReceive(readIeePipe, (unsigned char*)&counter, sizeof(unsigned));

                //cout << "counter size " << counter << endl;

                unsigned char* label = new unsigned char[l_size * counter];
                socketReceive(readIeePipe, label, l_size * counter * sizeof(unsigned char));

                unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * d_size * counter);

                /*for(int x = 0; x < counter; x++){
                    for(int y = 0; y < l_size; y++)
                        printf("%02x", label[x*l_size+y]);
                    printf("\n");
                }*/

                // send the labels for each word occurence
                for (unsigned i = 0; i < counter; i++) {
                    if(!I[label + i * l_size]) {
                        printf("Label not found! Exit\n");
                        exit(1);
                    }

                    //printf("%d %p\n", i, (*I)[l]);
                    /*for(unsigned k = 0; k < d_size; k++)
                        printf("%02x", (*I)[l][k]);
                    printf(" \n");*/

                    memcpy(buffer + i * d_size, I[label + i * l_size], d_size);
                }

                socketSend(writeIeePipe, buffer, d_size * counter);
                free(buffer);

                gettimeofday(&end2, NULL);
                total_search_time += util_time_elapsed(start2, end2);

                nr_search_batches++;

                #ifdef VERBOSE
                printf("Finished Search!\n");
                #endif
                break;
            }
            case '4': {
                // this instruction is for benchmarking only and can be safely
                // removed if wanted
                printf("## STATS ##\n");
                printf("SERVER Seen search batches: %lu\n", nr_search_batches);
                printf("SERVER Size index: %lu\n", I.size());
                printf("SERVER time: total sv add = %6.3lf sec\n", total_add_time/1000000.0);
                printf("SERVER time: total sv search = %6.3lf sec\n", total_search_time/1000000.0);
                total_search_time = 0;
                break;
            }
            default:
                printf("SseServer unkonwn command: %02x\n", cmd);
        }
    }
}

SseServer::~SseServer() {
    //delete[] I;
    //close(clientSock);
}
