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
    printf("gona open  write pipe!\n");
	writeIeePipe = open(pipeName, O_ASYNC | O_WRONLY);
    
    //create iee-server pipe
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "iee_to_server");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("Fail to mknod");
	
	//start listening on pipes; must open write first as read blocks
	printf("gona open read pipe!\n");
    readIeePipe = open(pipeName, O_ASYNC | O_RDONLY);
    printf("done!\n");
    
    //launch client-iee tunnel
/*	//not being used anymore, client comunicates with iee directly for testing
	pthread_t t;
    if (pthread_create(&t, NULL, bridgeClientIeeThread, NULL) != 0)
        pee("Error: unable to create bridgeClientTeeThread");
 */   
    //start listening for iee calls
    printf("Finished Server init! Gonna start listening for IEE requests!\n");
    while (true) {
        char cmd;
        socketReceive(readIeePipe, &cmd, sizeof(char));
        
        switch (cmd) {
            //setup
            case '1': {
                I = new map<vector<unsigned char>,vector<unsigned char> >;
                printf("Finished Setup!\n");
                break;
            }
            // add
            case '2': {
                const int l_size = 20;
                unsigned char* l = new unsigned char[l_size];
                socketReceive(readIeePipe, (char*)l, l_size);
                vector<unsigned char> l_vector = fillVector(l, l_size);
                delete[] l;
                
                const int d_size = 16;
                unsigned char* d = new unsigned char[d_size];
                socketReceive(readIeePipe, (char*)d, d_size);
                vector<unsigned char> d_vector = fillVector(d, d_size);
                delete[] d;
                
                (*I)[l_vector] = d_vector;
                printf("Finished Add!\n");
                break;
            }
            // search - get index positions
            case '3': {
                char buff[sizeof(int)];
                socketReceive(readIeePipe, buff, sizeof(int));
                int pos = 0;
                const int counter = readIntFromArr(buff, &pos);
                //cout << "counter size " << counter << endl;
                unsigned char* label = new unsigned char[20];
                for (int i = 0; i < counter; i++) {
                    socketReceive(readIeePipe, (char*)label, 20);
                    vector<unsigned char> l = fillVector(label, 20);
                    vector<unsigned char> enc_d = (*I)[l];
                    //cout << "enc_d size " << enc_d.size() << endl;
                    for (unsigned j = 0; j < enc_d.size(); j++)
                        socketSend(writeIeePipe, (char*)&enc_d[j], sizeof(unsigned char));
                }
                printf("Finished Search 1st part!\n");
                break;
            }
            // search - send response to client
            case '4': {
                //get data size
                char buff[sizeof(int)];
                socketReceive(readIeePipe, buff, sizeof(int));
                
                //get data
                int pos = 0;
                const int data_size = readIntFromArr(buff, &pos);
                char* data = new char[data_size];
                socketReceive(readIeePipe, data, data_size);
                
                //send data to client
                int sockfd = connectAndSend(buff, sizeof(int));
                socketSend(sockfd, data, data_size);
                
                close(sockfd);
                delete[] data;
                printf("Finished Search 2nd part!\n");
                break;
            }
            default:
                printf("SseServer unkonwn command!\n");
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
        char* buf = new char[sizeof(int)];
        if (receiveAll(newsockfd, buf, sizeof(int)) < 0) {
            close(newsockfd);
            perror("SseServer::bridgeClientIeeThread ERROR reading from socket");
            continue;
        }
        int pos = 0;
        int len = readIntFromArr(buf, &pos);
        
        char* bufAll = new char[len];
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
