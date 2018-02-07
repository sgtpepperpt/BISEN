//
//  MainUEE.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include "SseServer.hpp"

using namespace std;
#include <iostream>
const char* SseServer::pipeDir = "/tmp/BooleanSSE/";
int SseServer::clientSock;

const int l_size = 32;
const int d_size = 76;

SseServer::SseServer() {
    //init pipe directory
    if(mkdir(pipeDir, 0770) == -1)
        if(errno != EEXIST)
            pee("Failed to mkdir");

    //create server-iee pipe
    char pipeName[256];
    bzero(pipeName,256);
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "server_to_iee");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("Fail to mknod");

    //create iee-server pipe
    char pipeName2[256];
    strcpy(pipeName2, pipeDir);
    strcpy(pipeName2+strlen(pipeName2), "iee_to_server");
    if(mknod(pipeName2, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("Fail to mknod");

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

    while (true) {
        unsigned char cmd;
        socketReceive(readIeePipe, &cmd, sizeof(unsigned char));

        switch (cmd) {
            //setup
            case '1': {
                // delete existing map, only useful for reruns while debugging
                if(I) {
                    map<vector<unsigned char>,unsigned char*>::iterator it;
                    for(it = I->begin(); it != I->end(); ++it)
                        free(it->second);

                    delete I;

                    #ifdef VERBOSE
                    printf("Cleared map!\n");
                    #endif
                }

                I = new map<vector<unsigned char>, unsigned char*>;

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
                unsigned char* l = new unsigned char[l_size];
                socketReceive(readIeePipe, l, l_size);
                vector<unsigned char> l_vector = fillVector(l, l_size);
                delete[] l;

                unsigned char* d = new unsigned char[d_size];
                socketReceive(readIeePipe, d, d_size);
                (*I)[l_vector] = d;
                gettimeofday(&end, NULL);
                total_add_time += timeElapsed(start, end);

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

                unsigned char buff[sizeof(int)];
                socketReceive(readIeePipe, buff, sizeof(int));
                int pos = 0;
                const int counter = readIntFromArr(buff, &pos);
                //cout << "counter size " << counter << endl;
                unsigned char* label = new unsigned char[l_size * counter];
                socketReceive(readIeePipe, label, l_size * counter * sizeof(unsigned char));

                const size_t len = d_size * counter;
                unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * len);
                pos = 0;

                /*for(int x = 0; x < counter; x++){
                    for(int y = 0; y < l_size; y++)
                        printf("%02x", label[x*l_size+y]);
                    printf("\n");
                }*/

                // send the labels for each word occurence
                for (int i = 0; i < counter; i++) {
                    vector<unsigned char> l = fillVector(label + i * l_size, l_size);

                    /*for(unsigned k = 0; k < d_size; k++)
                        printf("%02x", (*I)[l][k]);
                    printf(" \n");*/

                    //socketSend(writeIeePipe, (*I)[l], sizeof(unsigned char) * d_size);
                    memcpy(buffer + i * d_size, (*I)[l], sizeof(unsigned char) * d_size);
                }

                socketSend(writeIeePipe, buffer, len);
                free(buffer);

                gettimeofday(&end2, NULL);
                total_search_time += timeElapsed(start2, end2);

                #ifdef VERBOSE
                printf("Finished Search!\n");
                #endif
                break;
            }
            case '4': {
                printf("## STATS ##\n");
                printf("SERVER Size index: %lu\n", (*I).size());
                printf("SERVER time: total sv add = %6.3lf sec\n", total_add_time/1000000.0);
                printf("SERVER time: total sv search = %6.3lf sec\n", total_search_time/1000000.0);
                break;
            }
            default:
                printf("SseServer unkonwn command: %02x\n", cmd);
        }
    }
}

void* SseServer::bridgeClientIeeThread(void* threadData) {
    //start listening socket
    struct sockaddr_in serv_addr;
    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock < 0)
        pee("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(serverPort);
    if (bind(clientSock, (const struct sockaddr *) &serv_addr,(socklen_t)sizeof(serv_addr)) < 0)
        pee("ERROR on binding");
    listen(clientSock,5);

    //start iee pipe
    char pipeName[256];
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "clientBridge");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("SseServer::bridgeClientIeeThread: Fail to mknod");
    int pipefd = open(pipeName, O_ASYNC | O_WRONLY);

    printf("Finished Server-IEE bridge init! Gonna start listening for client requests!\n");
    while (true) {
        //start listening for client calls
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(clientSock, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("SseServer::bridgeClientIeeThread ERROR on accept");
            continue;
        }
        //receive data
        unsigned char* buf = new unsigned char[sizeof(int)];
        if (receiveAll(newsockfd, buf, sizeof(int)) < 0) {
            close(newsockfd);
            perror("SseServer::bridgeClientIeeThread ERROR reading from socket");
            continue;
        }
        int pos = 0;
        int len = readIntFromArr(buf, &pos);

        unsigned char* bufAll = new unsigned char[len];
        if (receiveAll(newsockfd, bufAll, len) < 0) {
            close(newsockfd);
            pee("ERROR reading from tee pipe");
        }
        close(newsockfd);

        //redirect to iee
        socketSend(pipefd, buf, sizeof(int));
        socketSend(pipefd, bufAll, len);

        delete[] buf;
        delete[] bufAll;
    }
//    pthread_exit(NULL);
}

SseServer::~SseServer() {
    delete[] I;
    close(clientSock);
}

vector<unsigned char> SseServer::fillVector(unsigned char* array, int len){
    vector<unsigned char> v(len);
    for (int i = 0; i < len; i++)
        v[i] = array[i];
    return v;
}
