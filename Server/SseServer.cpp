//
//  MainUEE.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 16/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include "SseServer.hpp"
#include "Utils.h"

using namespace std;

SseServer::SseServer() {
    //init data structures - can also read persisted state here
    pipeDir = "/tmp/BooleanSSE";
    I = new map<vector<unsigned char>,vector<unsigned char> >;
    W = new map<vector<unsigned char>,vector<unsigned char> >;
    
    //init pipe directory
    if(mkdir(pipeDir, 0770) == -1)
        if(errno != EEXIST)
            pee("Failed to mkdir");
    
    //start iee-server pipe
    char pipeName[256];
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "/iee_to_server");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("Fail to mknod");
    readIeePipe = open(pipeName, O_ASYNC | O_RDONLY);
    
    //start server-iee pipe
    bzero(pipeName,256);
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "/server_to_iee");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("Fail to mknod");
    writeIeePipe = open(pipeName, O_ASYNC | O_WRONLY);
    
    //launch client-iee tunnel
    pthread_t t;
    if (pthread_create(&t, NULL, bridgeClientIeeThread, NULL) != 0)
        pee("Error: unable to create bridgeClientTeeThread");
    
    //start listening for iee calls
    while (true) {
        char cmd;
        if (read(readIeePipe,&cmd,sizeof(char)) < 0)  {
            puts("ERROR reading from pipe");
            continue;
        }
        switch (cmd) {
            //read from W
            case '1':
                //read token from pipe
                int tokenSize;
                read(readIeePipe,&tokenSize,sizeof(int));
                vector<unsigned char> token;
                for (int i = 0; i < tokenSize; i++)
                    read(readIeePipe,&token[i],1);
                
                //access W
                map<vector<unsigned char>,vector<unsigned char>>::iterator it = W->find(token);
                if (it == W->end()) {
                    puts("requested W key not found");
                    break;
                }
                
                //write value to pipe
                for (int i = 0; i < it->second.size(); i++)
                    write(writeIeePipe,&(it->second[i]),1);
                break;
                
            //write to W
            case '2':
                //read token from pipe
                int size;
                read(readIeePipe,&size,sizeof(int));
                vector<unsigned char> token;
                for (int i = 0; i < tokenSize; i++)
                    read(readIeePipe,&token[i],1);
                
                //read value from pipe
                read(readIeePipe,&size,sizeof(int));
                vector<unsigned char> value;
                for (int i = 0; i < size; i++)
                    read(readIeePipe,&value[i],1);
                
                //write to W
                (*W)[token] = value;
                break;
                
            case '3': //read from I
                readI();
                break;
            case '4': //write to I
                writeI();
                break;
            case '5':
                sendClient();
                break;
            default:
                printf("unkonwn command!\n");
        }
    }
}


void* SseServer::bridgeClientIeeThread(void* threadData) {
    //start listening socket
    struct sockaddr_in serv_addr;
    int clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock < 0)
        pee("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(9978);
    if (bind(clientSock, (const struct sockaddr *) &serv_addr,(socklen_t)sizeof(serv_addr)) < 0)
        pee("ERROR on binding");
    listen(clientSock,5);
    
    //start iee pipe
    char pipeName[256];
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "/clientBridge");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("SseServer::bridgeClientIeeThread: Fail to mknod");
    int pipefd = open(pipeName, O_ASYNC | O_WRONLY);
    
    while (true) {
        //start listening for client calls
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(clientSock, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            pee("ERROR on accept");
            continue;
        }
        
        //receive data
        int len;
        if (receiveAll(newsockfd, (char*)&len, sizeof(int)) < 0)  {
            pee("ERROR reading from socket");
            close(newsockfd);
            continue;
        }
        char* bufAll = new char[len];
        if (receiveAll(newsockfd, bufAll, len) < 0) {
            close(newsockfd);
            pee("ERROR reading from tee pipe");
        }
        close(newsockfd);
        
        //redirect to tee
        if (sendall(pipefd, (char*)&len, sizeof(int)) < 0)
            pee("ERROR writing to tee pipe");
        if (sendall(pipefd, bufAll, len) < 0)
            pee("ERROR writing to tee pipe");
        delete[] bufAll;
    }
//    pthread_exit(NULL);
}

SseServer::~SseServer() {
    delete I;
    delete W;
}