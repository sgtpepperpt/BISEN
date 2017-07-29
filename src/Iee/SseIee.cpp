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
    crypto = new IeeCrypt;
    init_pipes();
}

// ponto de entrada do IEE
// enc_output deve ser NULL
int SseIee::process(char* data, int data_size, char** output) {
    //char* plaintext = new char[ciphertext_size];
    //const int plaintext_size = decrypt_data(plaintext, ciphertext, ciphertext_size);
    
    const int output_size = f(data, data_size, output);
    //delete[] plaintext;
    
    /*const int enc_output_size = output_size + crypto->symBlocksize;
    enc_output = new char[enc_output_size];
    return crypto->encryptSymmetric((unsigned char*)output, output_size, (unsigned char*)enc_output, crypto->get_kCom());*/
    return output_size;
}

int SseIee::f(char* data, int data_size, char** output) {
    //setup operation
    if(data[0] == 'i')
        setup(data, data_size);
    //add / update operation
    else if (data[0] == 'a')
        add(data, data_size);
    //search operation
    else if (data[0] == 's')
        return search(data, data_size, output);
        
    return -1;
}

/*int SseIee::decrypt_data(char* plaintext, char* ciphertext, int ciphertext_size) {
    if(crypto->hasStoredKcom()) {
        return crypto->decryptSymmetric((unsigned char*)plaintext, (unsigned char*)ciphertext, ciphertext_size, crypto->get_kCom());
    } else {
        return crypto->decryptPublic((unsigned char*)plaintext, (unsigned char*)ciphertext, ciphertext_size);
    }
}*/

SseIee::~SseIee() {
    crypto->~IeeCrypt();
    close(readServerPipe);
    close(writeServerPipe);
}

void SseIee::init_pipes() {
    //init pipe directory
    if(mkdir(pipeDir, 0770) == -1)
        if(errno != EEXIST)
            pee("Failed to mkdir");

    char pipeName[256];

    //start server-iee pipe
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "server_to_iee");
	//not necessary, server creates file.
    //if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
        //if(errno != EEXIST)
            //pee("Fail to mknod");
    
    printf("opening read pipe!\n");
    readServerPipe = open(pipeName, O_ASYNC | O_RDONLY);

    //start iee-server pipe
    bzero(pipeName,256);
    strcpy(pipeName, pipeDir);
    strcpy(pipeName+strlen(pipeName), "iee_to_server");
	//not necessary, server creates file.
    //if(mknod(pipeName, S_IFIFO | 0770, 0) == -1)
    //    if(errno != EEXIST)
    //       pee("Fail to mknod");

    printf("opening write pipe!\n");
    writeServerPipe = open(pipeName, O_ASYNC | O_WRONLY);
    printf("donarino!\n");
    printf("Finished IEE init! Gonna start listening for client requests through bridge!\n");
}


void SseIee::setup(char* data, int data_size) {
    // [BP] - Decryption is no longer necessary, neither is storeKcom.
    //vector<unsigned char> data = crypto->decryptPublic(enc_data, enc_data_size);

    int pos = 1;

    // read kCom
    /*const int kCom_size = readIntFromArr(data, &pos);
    unsigned char* kCom = new unsigned char[kCom_size];
    readFromArr(kCom, kCom_size, data, &pos);*/

    // read kEnc
    const int kEnc_size = (int) readIntFromArr(data, &pos);
    unsigned char* kEnc = (unsigned char*)malloc(sizeof(unsigned char) * kEnc_size);
    readFromArr(kEnc, kEnc_size, data, &pos);

    // read kF
    const int kF_size = readIntFromArr(data, &pos);
    unsigned char* kF = (unsigned char*)malloc(sizeof(unsigned char) * kF_size);
    readFromArr(kF, kF_size, data, &pos);

    /*for(int i = 0; i < kF_size; i++)
        printf("%02x ", kF[i]);
    printf("\n");*/

    // [BP] - Keys will be given by the client (as input message)
    crypto->setKeys(kEnc, kF);

    //tell server to init index I
    char op = '1';
    socketSend(writeServerPipe, &op, sizeof(char));

    printf("Finished Setup!\n");
}

void SseIee::add(char* data, int data_len) {
    #ifdef VERBOSE
    printf("Started add in IEE!\n");
    #endif

    // read buffer
    int pos = 1;
    while(pos < data_len) {
        //get d,c,w from array
        const int d = readIntFromArr(data, &pos);
        const int c = readIntFromArr(data, &pos);

        // read word
        char* word = (char*)malloc(sizeof(char) * MAX_WORD_SIZE);
        char* tmp = (char*)malloc(sizeof(char));
        int counter = 0;

        do {
            readFromArr(tmp, 1, data, &pos);
            word[counter++] = tmp[0];
        } while(tmp[0] != '\0' && counter < MAX_WORD_SIZE);
        free(tmp);

        // guarantee string is terminated
        word[MAX_WORD_SIZE - 1] = '\0';

        //calculate key kW
        unsigned char* kW = (unsigned char*)malloc(sizeof(unsigned char) * crypto->fBlocksize);
        crypto->f(crypto->get_kF(), (unsigned char*)word, strlen(word), kW);

        //calculate index position label
        unsigned char* label = (unsigned char*)malloc(sizeof(unsigned char) * crypto->fBlocksize);
        crypto->f(kW, (unsigned char*)&c, sizeof(int), label);
        free(kW);

        //calculate index value enc_data
        int enc_data_size = sizeof(int) + crypto->symBlocksize;
        unsigned char* enc_data = (unsigned char*)malloc(sizeof(unsigned char) * enc_data_size);
        enc_data_size = crypto->encryptSymmetric((unsigned char*)&d, sizeof(int), enc_data, crypto->get_kEnc());

        //send label and enc_data to server
        char op = '2';
        socketSend(writeServerPipe, &op, sizeof(char));
        socketSend(writeServerPipe, (char*)label, crypto->fBlocksize);
        socketSend(writeServerPipe, (char*)enc_data, enc_data_size);

        free(label);
        free(enc_data);
    }
    
    #ifdef VERBOSE
    printf("Finished add in IEE!\n");
    #endif
}

void SseIee::get_docs_from_server(vec_token &query, unsigned count_words) {
    #ifdef VERBOSE
    printf("Requesting docs from server!\n");
    #endif

    // initialise array to hold all tokens in random order
    iee_token *rand[count_words];
    for(unsigned i = 0; i < count_words; i++)
        rand[i] = NULL;

    // randomly fill the array with the tokens we need
    for(unsigned i = 0; i < size(query); i++) {
        // ignore non-word tokens
        if(query.array[i].type != WORD_TOKEN)
            continue;

        /*for(unsigned ii = 0; ii < count_words; ii++) {
            if(rand[ii])
                printf("%c %s\n", rand[ii]->type, rand[ii]->word);
            else
                printf("%d\n", rand[ii]);
        }
        printf("\n");*/

        // choose a random unoccupied position from the rand array
        int pos;
        do {
            pos = spc_rand_uint_range(0, count_words);
        } while(rand[pos] != NULL);

        rand[pos] = &query.array[i];
    }

    #ifdef VERBOSE
    printf("Randomized positions!\n");
    #endif

    // request the documents from the server
    for(unsigned i = 0; i < count_words; i++) {
        iee_token *tkn = rand[i];

        //cout << "counter for " << tkn->word << " is " << tkn->counter << endl;
        if(tkn->counter == 0) {
            vec_int dummy;
            tkn->docs = dummy;
        }

        //calculate key kW
        unsigned char* kW = (unsigned char*)malloc(sizeof(unsigned char) * crypto->fBlocksize);
        crypto->f(crypto->get_kF(), (unsigned char*)tkn->word, strlen(tkn->word), kW);

        //calculate relevant index positions
        unsigned char** labels = (unsigned char**)malloc(sizeof(unsigned char*) * tkn->counter);
        unsigned char* l = (unsigned char*)malloc(sizeof(unsigned char) * crypto->fBlocksize);
        for (int c = 0; c < tkn->counter; c++) {
            crypto->f(kW, (unsigned char*)&c, sizeof(int), l);
            unsigned char* label = (unsigned char*)malloc(sizeof(unsigned char) * crypto->fBlocksize);

            for (int i = 0; i < crypto->fBlocksize; i++)
                label[i] = l[i];

            labels[c] = label;
            bzero(l, crypto->fBlocksize);
        }

        free(l);
        free(kW);

        //request index positions from server
        int len = sizeof(char) + sizeof(int) + tkn->counter * crypto->fBlocksize;
        char* buff = (char*)malloc(sizeof(char)* len);
        char op = '3';
        int pos = 0;
        addToArr(&op, sizeof(char), buff, &pos);
        addIntToArr(tkn->counter, buff, &pos);
        for (int i = 0; i < tkn->counter; i++)
            for (int j = 0; j < crypto->fBlocksize; j++)
                addToArr(&(labels[i][j]), sizeof(unsigned char), buff, &pos);

        socketSend(writeServerPipe, buff, len);
        free(buff);

        for (int i = 0; i < tkn->counter; i++)
            free(labels[i]);
        free(labels);

        //decrypt query results
        len = tkn->counter * sizeof(int);
        buff = (char*)malloc(sizeof(char)* len);

        unsigned char* enc_data = (unsigned char*)malloc(sizeof(unsigned char)* crypto->symBlocksize);
        unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char)* crypto->symBlocksize);
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

        // generate int vector
        const int nr_docs = len / sizeof(int);
        vec_int docs; // TODO check if this is always sorted
                      // else has to be sorted in evaluator; may not be needed for vec_int
        init(&docs, nr_docs);
        pos = 0;
        for (int i = 0; i < nr_docs; i++) {
            int tmp = -1;
            memcpy(&tmp, buff + pos, sizeof(int));
            pos += sizeof(int);

            push_back(&docs, tmp);
        }

        // insert result into token's struct
        tkn->docs = docs;

        free(enc_data);
        free(data);
    }

    #ifdef VERBOSE
    printf("Got all docs from server!\n\n");
    #endif
}

int SseIee::search(char* buffer, int query_size, char** output) {
    #ifdef VERBOSE
    printf("Search!\n");
    #endif

    vec_token query;
    init(&query, DEFAULT_QUERY_TOKENS);

    int nDocs = -1;
    int count_words = 0; // useful for get_docs_from_server

    //read buffer
    int pos = 1;
    while(pos < query_size) {
        iee_token tkn;
        tkn.word = NULL;

        char* tmp_type = (char*)malloc(sizeof(char));
        readFromArr(tmp_type, 1, buffer, &pos);

        tkn.type = tmp_type[0];
        free(tmp_type);

        if(tkn.type == WORD_TOKEN) {
            count_words++;

            // read counter
            tkn.counter = readIntFromArr(buffer, &pos);

            // read word
            tkn.word = (char*) malloc(sizeof(char) * MAX_WORD_SIZE);
            char* tmp = (char*)malloc(sizeof(char)); // TODO could this be more efficient since we're copying from
                                                     // one char array to another and then to a third one?
                                                     // (buffer->tmp->tkn.word)
            int counter = 0;
            do {
                readFromArr(tmp, 1, buffer, &pos);
                tkn.word[counter++] = tmp[0];
            } while(tmp[0] != '\0' && counter < MAX_WORD_SIZE);
            free(tmp);

            // guarantee string is terminated
            tkn.word[MAX_WORD_SIZE - 1] = '\0';
        } else if(tkn.type == META_TOKEN) {
            nDocs = readIntFromArr(buffer, &pos);
            continue;
        }

        push_back(&query, tkn);
    }

    // get documents from uee
    get_docs_from_server(query, count_words);

    #ifdef VERBOSE
    printf("parsed: ");
    for(unsigned i = 0; i < size(query); i++) {
        iee_token x = query.array[i];
        if(x.type == WORD_TOKEN) {
            printf("%s (", x.word);
            for(unsigned i = 0; i < size(x.docs); i++) {
                if(i < size(x.docs) - 1)
                    printf("%i,", x.docs.array[i]);
                else
                    printf("%i); ", x.docs.array[i]);
            }
        }

        else
            printf("%c ", x.type);
    }
    printf("\n\n");
    #endif

    //calculate boolean formula
    vec_int response_docs = evaluate(query, nDocs);

    #ifdef VERBOSE
    printf("Query Evaluated in IEE!\n");
    #endif    

    // return query results
    int output_size = size(response_docs) * sizeof(int);
    *output = (char*)malloc(sizeof(char) * output_size);
    pos = 0;

    for(unsigned i = 0; i < size(response_docs); i++) {
        //cout <<  response_docs[i] << endl;
        addIntToArr(response_docs.array[i], *output, &pos);
    }

    destroy(&response_docs);

    #ifdef VERBOSE
    printf("Finished Search!\n");
    #endif
    return output_size;
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

