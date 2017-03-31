//
//  MainTEE.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 15/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include "SseIee.hpp"

using namespace std;

const char* SseIee::pipeDir = "/tmp/BooleanSSE/";

SseIee::SseIee() {
    initIee();

    //start listening for client calls through bridge
    while (true) {
        //receive data
        char* buf = new char[sizeof(int)];
        if (receiveAll(clientBridgePipe, buf, sizeof(int)) < 0)
            pee("SseIee::SseIee - ERROR reading len from pipe");
        int pos = 0;
        int len = readIntFromArr(buf, &pos);
        delete[] buf;
        
        char* bufAll = new char[len];
        if (receiveAll(clientBridgePipe, bufAll, len) < 0)
            pee("SseIee::SseIee - ERROR reading data from pipe");
        
        //if kCom is NULL, can only accept setup operation
        if (!crypto->hasStoredKcom()) {
            vector<unsigned char> data = crypto->decryptPublic((unsigned char*)bufAll, len);
            crypto->storeKcom(data);
            crypto->initKeys();
            //tell server to init I
            char op = '1';
            sendall(writeServerPipe, &op, sizeof(char));
        }
        
//        switch (cmd) {
//                //read from W
//            case '1':
//                //read token from pipe
//                int tokenSize;
//                read(readIeePipe,&tokenSize,sizeof(int));
//                vector<unsigned char> token;
//                for (int i = 0; i < tokenSize; i++)
//                    read(readIeePipe,&token[i],1);
//                
//                //access W
//                map<vector<unsigned char>,vector<unsigned char>>::iterator it = W->find(token);
//                if (it == W->end()) {
//                    puts("requested W key not found");
//                    break;
//                }
//                
//                //write value to pipe
//                for (int i = 0; i < it->second.size(); i++)
//                    write(writeIeePipe,&(it->second[i]),1);
//                break;
//        }
    }
    
}


SseIee::~SseIee() {
    
}


void SseIee::initIee() {
    crypto = new IeeCrypt;
    
    //init pipe directory
    if(mkdir(pipeDir, 0770) == -1)
        if(errno != EEXIST)
            pee("Failed to mkdir");
    
    //start server-iee pipe
    char pipeName[256];
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "server_to_iee");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("Fail to mknod");
    readServerPipe = open(pipeName, O_ASYNC | O_RDONLY);
    
    //start iee-server pipe
    bzero(pipeName,256);
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "iee_to_server");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("Fail to mknod");
    writeServerPipe = open(pipeName, O_ASYNC | O_WRONLY);
    
    //start iee pipe
    bzero(pipeName,256);
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "clientBridge");
    if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        if(errno != EEXIST)
            pee("SseServer::bridgeClientIeeThread: Fail to mknod");
    clientBridgePipe = open(pipeName, O_ASYNC | O_RDONLY);
}




//void initServer (int newsockfd) {
//    int port = 5566;
//    int srvr_fd;
//    int clnt_fd;
//    char buf[1];
//    struct sockaddr_in addr;
//    
//    srvr_fd = socket(PF_INET, SOCK_STREAM, 0);
//    
//    if (srvr_fd == -1) {
//        sgx_exit(NULL);
//    }
//    
//    memset(&addr, 0, sizeof(addr));
//    addr.sin_family = AF_INET;
//    addr.sin_port = htons(port);
//    addr.sin_addr.s_addr = INADDR_ANY;
//    
//    if (bind(srvr_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
//        sgx_exit(NULL);
//    }
//    
//    if (listen(srvr_fd, 10) != 0) {
//        sgx_exit(NULL);
//    }
//    
//    while (1) {
//        struct sockaddr_in addr;
//        socklen_t len = sizeof(addr);
//        clnt_fd = accept(srvr_fd, (struct sockaddr *)&addr, &len);
//        if (clnt_fd < 0) {
//            puts("ERROR on accept\n");
//            continue;
//        }
//        
//        memset(buf, 0, 1);
//        //int n = sgx_read(clnt_fd, buf, 255);
//        int n = recv(clnt_fd, buf, 1, 0);
//        if (n < 0)
//            puts("ERROR on read\n");
//        
//        //puts(buf);
//        switch (buf[0]) {
//            case 'i':
//                initServer(newsockfd);
//                break;
//            case 'a':
//                receiveDocs(newsockfd);
//                break;
//            case 's':
//                this->search(newsockfd);
//                break;
//            default:
//                printf("unkonwn command!\n");
//        }
//        
//        
//        //n = sgx_write(clnt_fd, "Successfully received", 21);
//        n = send(clnt_fd, "Successfully received", 21, 0);
//        if (n < 0)
//            puts("ERROR on write\n");
//        
//        close(clnt_fd);
//    }
//    
//    close(srvr_fd);
//    
//    sgx_exit(NULL);
//}

