//
//  MainTEE.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 15/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

/*
 Required changes are tagged with [BP]

 Modifications:
 -> There is no connection between client and IEE. Client requests will be given as input messages to SseIee.
 -> SseIee runs with a message already on plaintext. Secure channel encryption/decryption is taken care of outside SseIee.
 -> Thus, KCom no longer exists here.
 -> Pipe connections will thus only be required for Iee->Server and Server->Iee. 
*/

#include "SseIee.hpp"

using namespace std;

const char* SseIee::pipeDir = "/tmp/BooleanSSE/";
// [BP] - Must receive a message, which will be either two keys (for the setup), or an Add/Search command.
SseIee::SseIee() { 
    initIee();

    //start listening for client calls through bridge 
    // [BP] - client calls are now given as input messages. No pipes or decryption necessary.
    while (true) {
        //receive data
        char buff[sizeof(int)];
        socketReceive(clientBridgePipe, buff, sizeof(int));
        int pos = 0;
        const int enc_data_size = readIntFromArr(buff, &pos);

        unsigned char* enc_data = new unsigned char[enc_data_size];
        socketReceive(clientBridgePipe, (char*)enc_data, enc_data_size);
        
        //process request
        // [BP] - Kcom does not exist. This check cannot allow for requests to be processed before setup.
        if (!crypto->hasStoredKcom()) {
            //if kCom is NULL, can only accept setup operation
            setup(enc_data, enc_data_size);
        } else {
            //already has kCom, decrypt with it and perform update or search
            // [BP] - Received message is already decrypted.
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
    // [BP] - This pipe will no longer exist
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
    // [BP] - This is no longer necessary, since requests are now inputs to SseIee.
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
    // [BP] - Decryption is no longer necessary, neither is storeKcom.
    vector<unsigned char> data = crypto->decryptPublic(enc_data, enc_data_size);

    //TODO redo so as to use data instead of copying to array
    char buff[data.size()];
    for(int i = 0; i < data.size(); i++) {
        buff[i] = data[i];
    }

    int pos = 0;

    // read kCom
    const int kCom_size = readIntFromArr(buff, &pos);
    unsigned char* kCom = new unsigned char[kCom_size];
    readFromArr(kCom, kCom_size, buff, &pos);

    // read kEnc
    const int kEnc_size = readIntFromArr(buff, &pos);
    unsigned char* kEnc = new unsigned char[kEnc_size];
    readFromArr(kEnc, kEnc_size, buff, &pos);

    // read kF
    const int kF_size = readIntFromArr(buff, &pos);
    unsigned char* kF = new unsigned char[kF_size];
    readFromArr(kF, kCom_size, buff, &pos);
    
    /*for(int i = 0; i < kF_size; i++)
        printf("%02x ", kF[i]);
    printf("\n");*/

    //TODO storeKcom is no longer necessary.
    crypto->storeKcom(kCom);
    // [BP] - Keys will be given by the client (as input message)
    crypto->setKeys(kEnc, kF);

    //tell server to init index I
    char op = '1';
    socketSend(writeServerPipe, &op, sizeof(char));
    printf("Finished Setup!\n");
}


void SseIee::add(char* data, int data_len) {
    // read buffer
    int pos = 1;
    while(pos < data_len) {
        //get d,c,w from array
        const int d = readIntFromArr(data, &pos);
        const int c = readIntFromArr(data, &pos);

        // read word
        string word;
        char* tmp;
        do {
            tmp = new char[1];
            readFromArr(tmp, 1, data, &pos);
            word += tmp[0];
        } while(tmp[0] != '\0');
        delete[] tmp;

        const char* w = word.c_str();

        //calculate key kW
        unsigned char* kW = new unsigned char[crypto->fBlocksize];
        crypto->f(crypto->get_kF(), (unsigned char*)w, strlen(w), kW);

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
    }

    printf("Finished Add!\n");
}

void SseIee::get_docs_from_server(deque<token> &query) {
    // initialise array to hold all tokens in random order
    token *rand[query.size()];
    for(int i = 0; i < query.size(); i++)
        rand[i] = NULL;

    // randomly fill the array with the tokens we need
    for(int i = 0; i < query.size(); i++) {
        // ignore non-word tokens
        if(query[i].type != WORD_TOKEN)
            continue;

        // choose a random unoccupied position from the rand array
        int pos;
        do {
            pos = spc_rand_uint_range(0, query.size());
        } while(rand[pos] != NULL);

        rand[pos] = &query[i];
    }

    // request the documents from the server
    for(int i = 0; i < query.size(); i++) {
        token *tkn = rand[i];

        // ignore operators for document searching
        if(tkn == NULL)
            continue;
        
        //cout << "counter for " << tkn->word << " is " << tkn->counter << endl;
        if(tkn->counter == 0) {
            vector<int> dummy;
            tkn->docs = dummy;
        }

        const char* word_str = tkn->word.c_str();

        //calculate key kW
        unsigned char* kW = new unsigned char[crypto->fBlocksize];
        crypto->f(crypto->get_kF(), (unsigned char*)word_str, strlen(word_str), kW);

        //calculate relevant index positions
        vector< vector<unsigned char> > labels (tkn->counter);
        unsigned char* l = new unsigned char[crypto->fBlocksize];
        for (int c = 0; c < tkn->counter; c++) {
            crypto->f(kW, (unsigned char*)&c, sizeof(int), l);
            vector<unsigned char> label (crypto->fBlocksize);
            for (int i = 0; i < crypto->fBlocksize; i++)
                label[i] = l[i];
            labels[c] = label;
            bzero(l, crypto->fBlocksize);
        }
        delete[] l;
        delete[] kW;

        //request index positions from server
        int len = sizeof(char) + sizeof(int) + tkn->counter * crypto->fBlocksize;
        char* buff = new char[len];
        char op = '3';
        int pos = 0;
        addToArr(&op, sizeof(char), buff, &pos);
        addIntToArr(tkn->counter, buff, &pos);
        for (int i = 0; i < tkn->counter; i++)
            for (int j = 0; j < crypto->fBlocksize; j++)
                addToArr(&(labels[i][j]), sizeof(unsigned char), buff, &pos);

        socketSend(writeServerPipe, buff, len);
        delete[] buff;

        //decrypt query results
        len = tkn->counter * sizeof(int);
        buff = new char[len];
        unsigned char* enc_data = new unsigned char[crypto->symBlocksize];
        unsigned char* data = new unsigned char[crypto->symBlocksize];
        pos = 0;
        for (int i = 0; i < tkn->counter; i++) {
            socketReceive(readServerPipe, (char*)enc_data, crypto->symBlocksize);

            crypto->decryptSymmetric(data, enc_data, crypto->symBlocksize, crypto->get_kEnc());
            addToArr((char*)data, sizeof(int), buff, &pos);
            //cout << "recv ." << buff << "." << endl;
            /** Another way of doing it
                int d = -1;
                memcpy(&d, data, sizeof(int));
                addIntToArr(d, buff, &pos);
            **/
            bzero(enc_data, crypto->symBlocksize);
            bzero(data, crypto->symBlocksize);
        }

        const int nr_docs = len / sizeof(int);
        vector<int> docs(nr_docs); // TODO check if this is always sorted, else has to be sorted in evaluator
        pos = 0;
        for (int i = 0; i < nr_docs; i++) {
            memcpy(&docs[i], buff+pos, sizeof(int));
            pos += sizeof(int);
        }

        // insert result into token's struct
        tkn->docs = docs;

        delete[] enc_data;
        delete[] data;
    }
}

void SseIee::search(char* buffer, int query_size) {
    cout << "search!" << endl;
    deque<token> query; //TODO for boolean eval should be queue, but we have to iterate twice before that for now...
    int nDocs;

    //read buffer
    int pos = 1;
    while(pos < query_size) {
        token tkn;

        char* tmp_type = new char[1];
        readFromArr(tmp_type, 1, buffer, &pos);

        tkn.type = tmp_type[0];
        delete[] tmp_type;

        if(tkn.type == WORD_TOKEN) {
            // read counter
            tkn.counter = readIntFromArr(buffer, &pos);

            // read word
            char* tmp;
            do {
                tmp = new char[1];
                readFromArr(tmp, 1, buffer, &pos);

                tkn.word += tmp[0];
            } while(tmp[0] != '\0');

            delete[] tmp;
        } else if(tkn.type == META_TOKEN) {
            nDocs = readIntFromArr(buffer, &pos);
            continue;
        }

        query.push_back(tkn);
    }

    // get documents from uee
    get_docs_from_server(query);

    //calculate boolean formula
    vector<int> response_docs = evaluate(query, nDocs);

    //send query results with kCom
    // [BP] - Instead of encrypting with kCom and sending via pipe, it must simply be returned by SseIee in plaintext.
    int len = response_docs.size() * sizeof(int);
    char* buff = new char[len];
    pos = 0;

    for(int i = 0; i < response_docs.size(); i++) {
        addIntToArr(response_docs[i], buff, &pos);
    }

    int enc_results_size = len + crypto->symBlocksize;
    unsigned char* enc_results = new unsigned char[enc_results_size];
    enc_results_size = crypto->encryptSymmetric((unsigned char*)buff, len, enc_results, crypto->get_kCom());
    delete[] buff;

    //send results to client
    char op = '4';
    socketSend(writeServerPipe, &op, sizeof(char));

    buff = new char[sizeof(int)];
    pos = 0;
    addIntToArr(enc_results_size, buff, &pos);
    socketSend(writeServerPipe, buff, sizeof(int));
    delete[] buff;

    socketSend(writeServerPipe, (char*)enc_results, enc_results_size);
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

