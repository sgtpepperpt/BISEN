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
        char buff[sizeof(int)];
        socketReceive(clientBridgePipe, buff, sizeof(int));
        int pos = 0;
        const int enc_data_size = readIntFromArr(buff, &pos);
        
        unsigned char* enc_data = new unsigned char[enc_data_size];
        socketReceive(clientBridgePipe, (char*)enc_data, enc_data_size);
        
        //process request
        if (!crypto->hasStoredKcom()) {
            //if kCom is NULL, can only accept setup operation
            setup(enc_data, enc_data_size);
        } else {
            //already has kCom, decrypt with it and perform update or search
            char* data = new char[enc_data_size];
            int data_size = crypto->decryptSymmetric((unsigned char*)data, enc_data, enc_data_size, crypto->get_kCom());
            
            //add / update operation
            if (data[0] == 'a')
                add(data, data_size);
                
            //search operation
            else if (data[0] == 's')
                search(data, data_size);
            
            delete[] data;
        }
        delete[] enc_data;
    }
}


SseIee::~SseIee() {
    crypto->~IeeCrypt();
    close(readServerPipe);
    close(writeServerPipe);
    close(clientBridgePipe);
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
    
    printf("Finished IEE init! Gonna start listening for client requests through bridge!\n");
}


void SseIee::setup(unsigned char* enc_data, int enc_data_size) {
    vector<unsigned char> data = crypto->decryptPublic(enc_data, enc_data_size);
    crypto->storeKcom(data);
    crypto->initKeys();
    
    //tell server to init index I
    char op = '1';
    socketSend(writeServerPipe, &op, sizeof(char));
    printf("Finished Setup!\n");
}


void SseIee::add(char* data, int data_len) {
    //get d,c,w from array
    int pos = 1;
    const int d = readIntFromArr(data, &pos);
    const int c = readIntFromArr(data, &pos);
    const int w_size = data_len - pos;
    char* w = new char [w_size];
    readFromArr(w, w_size, data, &pos);
    
    //calculate key kW
    unsigned char* kW = new unsigned char[crypto->fBlocksize];
    crypto->f(crypto->get_kF(), (unsigned char*)w, w_size, kW);
    delete[] w;
    
    //calculate index position label
    unsigned char* label = new unsigned char[crypto->fBlocksize];
    crypto->f(kW, (unsigned char*)&c, sizeof(int), label);
    delete[] kW;
    
    //calculate index value enc_data
    int enc_data_size = sizeof(int)+crypto->symBlocksize;
    unsigned char* enc_data = new unsigned char[enc_data_size];
    enc_data_size = crypto->encryptSymmetric((unsigned char*)&d, sizeof(int), enc_data, crypto->get_kEnc());
    
    //send label and enc_data to server
    char op = '2';
    socketSend(writeServerPipe, &op, sizeof(char));
    socketSend(writeServerPipe, (char*)label, crypto->fBlocksize);
    socketSend(writeServerPipe, (char*)enc_data, enc_data_size);
    
    delete[] label;
    delete[] enc_data;
    printf("Finished Add!\n");
}


void SseIee::search(char* buffer, int query_size) {
	queue<token> query;
	
	//read buffer
	int pos = 1;
	while(pos < query_size) {
		token tkn;
	
		char* tmp_type = new char[1];
    	readFromArr(tmp_type, 1, buffer, &pos);
    	
    	tkn.type = tmp_type[0];
    	
    	if(tkn.type == 't') {
    		// read counter
    		tkn.counter = readIntFromArr(buffer, &pos) + 1;
    		printf("counter is %d\n", tkn.counter);
    		
    		// read word
    		char* tmp;
    		do {
				tmp = new char[1];
				readFromArr(tmp, 1, buffer, &pos);
    			
    			tkn.word += tmp[0];
    		} while(tmp[0] != '\0');
    		
    		cout<< "word is "<< tkn.word<<endl;
    	}
    	
    	query.push(tkn);
    }
       
    printf("query size %d\n", query_size);
    /*
    //calculate key kW
    unsigned char* kW = new unsigned char[crypto->fBlocksize];
    crypto->f(crypto->get_kF(), (unsigned char*)buff, len, kW);
    delete[] buff;
    
    //calculate relevant index positions
    vector< vector<unsigned char> > labels (counter);
    unsigned char* l = new unsigned char[crypto->fBlocksize];
    for (int c = 0; c < counter; c++) {
        crypto->f(kW, (unsigned char*)&c, sizeof(int), l);
        vector<unsigned char> label (crypto->fBlocksize);
        for (int i = 0; i < crypto->fBlocksize; i++)
            label[i] = l[i];
        labels[c] = label;
        bzero(l, crypto->fBlocksize);
    }
    delete[] l;
    delete[] kW;
    
    //randomize index postions
    /**TODO*/
    
    //request index positions from server
    /*len = sizeof(char) + sizeof(int) + counter * crypto->fBlocksize;
    buff = new char[len];
    char op = '3';
    pos = 0;
    addToArr(&op, sizeof(char), buff, &pos);
    addIntToArr(counter, buff, &pos);
    for (int i = 0; i < counter; i++)
        for (int j = 0; j < crypto->fBlocksize; j++)
            addToArr(&(labels[i][j]), sizeof(unsigned char), buff, &pos);
    
    socketSend(writeServerPipe, buff, len);
    delete[] buff;
    
    //decrypt query results
    len = counter * sizeof(int);
    buff = new char[len];
    unsigned char* enc_data = new unsigned char[crypto->symBlocksize];
    unsigned char* data = new unsigned char[crypto->symBlocksize];
    pos = 0;
    for (int i = 0; i < counter; i++) {
        socketReceive(readServerPipe, (char*)enc_data, crypto->symBlocksize);
        crypto->decryptSymmetric(data, enc_data, crypto->symBlocksize, crypto->get_kEnc());
        addToArr((char*)data, sizeof(int), buff, &pos);

/** Another way of doing it
        int d = -1;
        memcpy(&d, data, sizeof(int));
        addIntToArr(d, buff, &pos); 
 **//*
        bzero(enc_data, crypto->symBlocksize);
        bzero(data, crypto->symBlocksize);
    }
    
    delete[] enc_data;
    delete[] data;
    
    //calculate boolean formula here; for now its sinlge keyword

    //send query results with kCom
    int enc_results_size = len + crypto->symBlocksize;
    unsigned char* enc_results = new unsigned char[enc_results_size];
    enc_results_size = crypto->encryptSymmetric((unsigned char*)buff, len, enc_results, crypto->get_kCom());
    delete[] buff;
    
    //send results to client
    op = '4';
    socketSend(writeServerPipe, &op, sizeof(char));
    
    buff = new char[sizeof(int)];
    pos = 0;
    addIntToArr(enc_results_size, buff, &pos);
    socketSend(writeServerPipe, buff, sizeof(int));
    delete[] buff;
    
    socketSend(writeServerPipe, (char*)enc_results, enc_results_size);*/
    printf("Finished Search!\n");
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

