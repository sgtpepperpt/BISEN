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
#include <sys/time.h>

using namespace std;

double server_diff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }

    return ((double)temp.tv_sec * 1000000000.0 + (double)temp.tv_nsec) / 1000000.0;
}

size_t aa = 0;
size_t db_size = 0;

typedef struct client_data {
    int socket;
} client_data;

void* process_client(void* args) {
    client_data* data = (client_data*)args;
    int client_socket = data->socket;
    free(data);

#if SPARSE_MAP
    uee_map I;
#endif

#if REDIS
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        if (c) {
            printf("Error: %s\n", c->errstr);
            // handle error
        } else {
            printf("Can't allocate redis context\n");
        }
    }
#endif

    int count_searches = 0;

    double total_add_time = 0;
    double total_search_time = 0;

    double total_add_time_network = 0;
    double total_search_time_network = 0;

    size_t nr_search_batches = 0;

    while (true) {
        unsigned char cmd;
        socketReceive(client_socket, &cmd, sizeof(unsigned char));

        switch (cmd) {
            //setup
            case '1': {
                // delete existing map, only useful for reruns while debugging
#if SPARSE_MAP
                if(I.size()) {
                    //#ifdef VERBOSE
                    printf("Size before: %lu\n", I.size());
                    //#endif

                    uee_map::iterator it;
                    for(it = I.begin(); it != I.end(); ++it) {
                        free(it->first);
                        free(it->second);
                    }

                    I.clear();
                }
#endif
#if REDIS
                redisReply* reply = (redisReply*)redisCommand(c, "FLUSHALL");
                if (!reply) {
                    printf("error redis flushall!\n");
                }
                db_size = 0;
#endif
                //#ifdef VERBOSE
                printf("Cleared map!\n");
                printf("------------------------\n");
                //#endif

#ifdef VERBOSE
                printf("Finished Setup!\n");
#endif
                break;
            }
                // add
            case '2': {
#ifdef VERBOSE
                printf("Started Add!\n");
#endif
                //struct timespec start, end;
                //clock_gettime(CLOCK_REALTIME, &start);
                //clock_t start = clock();

                size_t batch_size;

                struct timespec startn, endn;
                clock_gettime(CLOCK_REALTIME, &startn);

                socketReceive(client_socket, (unsigned char*)&batch_size, sizeof(size_t));

                clock_gettime(CLOCK_REALTIME, &endn);
                total_add_time_network += server_diff(startn, endn);


                uint8_t* buffer = (uint8_t*)malloc(batch_size * (l_size + d_size));

                clock_gettime(CLOCK_REALTIME, &startn);
                socketReceive(client_socket, buffer, batch_size * (l_size + d_size));
                clock_gettime(CLOCK_REALTIME, &endn);
                total_add_time_network += server_diff(startn, endn);

                struct timespec start, end;
                clock_gettime(CLOCK_REALTIME, &start);
                for (size_t i = 0; i < batch_size; i++) {
                    void* l = buffer + i * (l_size + d_size);
                    void* d = buffer + i * (l_size + d_size) + l_size;

#if SPARSE_MAP
                    I[l] = d;
#endif

#if REDIS
                    redisAppendCommand(c, "SET %b %b", l, (size_t)l_size, d, (size_t)d_size);
#endif
                }
#if REDIS
                free(buffer);
#endif
                clock_gettime(CLOCK_REALTIME, &end);
                total_add_time += server_diff(start, end);

#if REDIS
                redisReply* reply;
                for (size_t i = 0; i < batch_size; i++) {
                    redisGetReply(c, (void**)&reply);

                    if(!reply) {
                        printf("reply error %d\n", c->err);
                        exit(1);
                    }

                    freeReplyObject(reply);
                }

                db_size += batch_size; // only for redis / cassandra
#endif

                //clock_t end = clock();

                /*clock_gettime(CLOCK_REALTIME, &end);
                total_add_time += server_diff(start, end);*/

                //printf("partial %lf\n", e);

                /*if (aa++ % 50000 == 0) {
#if REDIS
                    printf("Size of index: %lu\n", db_size);
#endif
#if SPARSE_MAP
                    printf("Size of index: %lu\n", I.size());
#endif
                }*/


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

                unsigned counter;
                struct timespec startn, endn;
                clock_gettime(CLOCK_REALTIME, &startn);

                socketReceive(client_socket, (unsigned char*)&counter, sizeof(unsigned));

                clock_gettime(CLOCK_REALTIME, &endn);
                total_search_time_network += server_diff(startn, endn);

                unsigned char* label = new unsigned char[l_size * counter];

                clock_gettime(CLOCK_REALTIME, &startn);
                socketReceive(client_socket, label, l_size * counter * sizeof(unsigned char));
                clock_gettime(CLOCK_REALTIME, &endn);
                total_search_time_network += server_diff(startn, endn);

                unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * d_size * counter);

                struct timespec start, end;
                clock_gettime(CLOCK_REALTIME, &start);

                /*for(int x = 0; x < counter; x++){
                    for(int y = 0; y < l_size; y++)
                        printf("%02x", label[x*l_size+y]);
                    printf("\n");
                }*/

                // send the labels for each word occurence
                for (unsigned i = 0; i < counter; i++) {
                    /*for(unsigned i = 0; i < l_size; i++)
                        printf("%02x", ((unsigned char*)label + i * l_size)[i]);
                    printf("\n");*/

#if SPARSE_MAP
                    if(!I[label + i * l_size]) {
                        printf("Label not found! Exit\n");
                        exit(1);
                    }

                    memcpy(buffer + i * d_size, I[label + i * l_size], d_size);
                }
#endif
#if REDIS
                    redisAppendCommand(c, "GET %b", label + i * l_size, l_size);
                }

                redisReply* reply;
                for (unsigned i = 0; i < counter; i++) {
                    redisGetReply(c, (void**)&reply);
                    if (!reply) {
                        printf("error redis get!\n");
                    }
                    //printf("%d %p\n", i, (*I)[l]);
                    /*for(unsigned k = 0; k < d_size; k++)
                        printf("%02x", reply->str[k]);
                    printf(" \n");*/

                    memcpy(buffer + i * d_size, reply->str, d_size);
                    freeReplyObject(reply);
                }
#endif

                clock_gettime(CLOCK_REALTIME, &end);
                total_search_time += server_diff(start, end);

                clock_gettime(CLOCK_REALTIME, &startn);
                socketSend(client_socket, buffer, d_size * counter);

                clock_gettime(CLOCK_REALTIME, &endn);
                total_search_time_network += server_diff(startn, endn);

                free(buffer);
                nr_search_batches++;

#ifdef VERBOSE
                printf("Finished Search!\n");
#endif
                break;
            }
            case '4': {
                // this instruction is for benchmarking only and can be safely
                // removed if wanted

                if(!count_searches) {
                    printf("SERVER time: total sv add = %6.3lf sec\n", total_add_time / 1000.0);
                    printf("SERVER time: total sv add network = %6.3lf sec\n", total_add_time_network / 1000.0);
                }

                printf("## STATS %d ##\n", count_searches++);
                printf("SERVER Seen search batches: %lu\n", nr_search_batches);
#if SPARSE_MAP
                printf("SERVER Size index: %lu\n", I.size());
#endif
#if REDIS
                printf("SERVER Size index: %lu\n", db_size);
#endif
                printf("SERVER time: total sv search = %6.3lf sec\n", total_search_time / 1000.0);
                printf("SERVER time: total sv search network = %6.3lf sec\n", total_search_time_network / 1000.0);
                total_search_time = 0;
                break;
            }
            default:
                printf("SseServer unkonwn command: %02x\n", cmd);
        }
    }
}

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

    while(true) {
        client_data *data = (client_data *)malloc(sizeof(client_data));

        if ((data->socket = accept(listen_socket, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
            printf("Accept failed!\n");
            exit(1);
        }

        printf("------------------------------------------\nClient connected (%s)\n", inet_ntoa(client_addr.sin_addr));

        pthread_t tid;
        pthread_create(&tid, NULL, process_client, data);
    }
}

SseServer::~SseServer() {
    //delete[] I;
}
