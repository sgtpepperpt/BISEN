//
//  MainUEE.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include "SseServer.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

//int SseServer::clientSock;
//unsigned long debug_i = 0;

SseServer::SseServer() {
    const int server_port = 7899;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int listen_socket;
    if ((listen_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Could not create socket!\n");
        exit(1);
    }

    int res = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &res, sizeof(res)) == -1) {
        printf("Could not set socket options!\n");
        exit(1);
    }

    if ((bind(listen_socket, (struct sockaddr *) &server_addr, sizeof(server_addr))) < 0) {
        printf("Could not bind socket!\n");
        exit(1);
    }

    if (listen(listen_socket, 16) < 0) {
        printf("Could not open socket for listening!\n");
        exit(1);
    }

    //start listening for iee calls
    printf("Finished Server init! Gonna start listening for IEE requests!\n");

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = 0;

    printf("Listening for requests...\n");


    int client_socket;
    if ((client_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
        printf("Accept failed!\n");
        exit(1);
    }

    printf("Client connected (%s)\n", inet_ntoa(client_addr.sin_addr));

    long total_add_time = 0;
    long total_search_time = 0;
    size_t nr_search_batches = 0;

    while (true) {
        unsigned char cmd;
        socketReceive(client_socket, &cmd, sizeof(unsigned char));

        switch (cmd) {
            //setup
            case '1': {
                // delete existing map, only useful for reruns while debugging

                if(I.size()) {
                    //#ifdef VERBOSE
                    printf("Size before: %lu\n", I.size());
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
                    printf("Size now: %lu\n", I.size());
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

                size_t batch_size;
                socketReceive(client_socket, (unsigned char*)&batch_size, sizeof(size_t));

                for(size_t i = 0; i < batch_size; i++) {
                    void* l = malloc(l_size);
                    socketReceive(client_socket, (unsigned char*)l, l_size);

                    /* for(unsigned i = 0; i < 32; i++)
                         printf("%02x", ((char*)l)[i]);
                     printf("\n");*/

                    void* d = malloc(d_size);
                    socketReceive(client_socket, (unsigned char*)d, d_size);

/*
                    for(unsigned i = 0; i < 32; i++)
                        printf("%02x", ((unsigned char*)l)[i]);
                    printf("\n");
                    for(unsigned i = 0; i < 80; i++)
                        printf("%02x", ((unsigned char*)d)[i]);
                    printf("\n\n");*/


                    I[l] = d;
                }

                gettimeofday(&end, NULL);
                total_add_time += util_time_elapsed(start, end);

                //if(debug_i++ % 100000 == 0)
                  //  printf("pairs: %lu\n", I.size());
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
                socketReceive(client_socket, (unsigned char*)&counter, sizeof(unsigned));

                //cout << "counter size " << counter << endl;

                unsigned char* label = new unsigned char[l_size * counter];
                socketReceive(client_socket, label, l_size * counter * sizeof(unsigned char));

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

                socketSend(client_socket, buffer, d_size * counter);
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
