//
//  SseIee.cpp
//  BISEN
//
//  Created by Bernardo Ferreira on 15/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include "SseIee.h"

const char* pipeDir = "/tmp/BooleanSSE/";

void print_buffer(const char* name, const unsigned char * buf, const unsigned long long len) {
    /*ocall_printf("%s size: %llu\n", name, len);
    for(unsigned i = 0; i < len; i++)
        ocall_printf("%02x", buf[i]);
    ocall_printf("\n");*/
}

// IEE entry point
void f(bytes* out, size* out_len, const unsigned long long pid, const bytes in, const size in_len) {
    #ifdef VERBOSE
    //ocall_strprint("\n***** Entering IEE *****\n");
    #endif

    ocall_strprint("\n***** Entering IEE *****\n");

    // set out variables
    *out = NULL;
    *out_len = 0;

    //setup operation
    if(in[0] == OP_SETUP)
        setup(out, out_len, in, in_len);
    //add / update operation
    else if (in[0] == OP_ADD)
        add(out, out_len, in, in_len);
    //search operation
    else if (in[0] == OP_SRC)
        search(out, out_len, in, in_len);
    else if (in[0] == OP_BENCH)
        benchmarking_print();

    #ifdef VERBOSE
    //ocall_strprint("\n***** Leaving IEE *****\n\n");
    #endif
}

static void benchmarking_print() {
    // BENCHMARK : tell server to print statistics
    // this instruction can be safely removed if wanted
    int tmp_buff_len = sizeof(char);
    unsigned char* tmp_buff = (unsigned char*)malloc(sizeof(unsigned char));
    tmp_buff[0] = '4';
    iee_socketSend(writeServerPipe, tmp_buff, sizeof(unsigned char));
}

unsigned char *count;

static void init_pipes() {
    count = (unsigned char*) malloc(550000);
    char pipeName[256];

    //start server-iee pipe
    strncpy(pipeName, pipeDir, strlen(pipeDir)+1 /*copy \0*/);
    strncpy(pipeName + strlen(pipeName), "server_to_iee", strlen("server_to_iee"));

    ocall_strprint("Opening read pipe!\n");
    readServerPipe = ocall_open("/tmp/BooleanSSE/server_to_iee", O_ASYNC | O_RDONLY);

    //start iee-server pipe
    iee_bzero(pipeName, 256);
    strncpy(pipeName, pipeDir, strlen(pipeDir));
    strncpy(pipeName+strlen(pipeName), "iee_to_server", strlen("iee_to_server"));

    ocall_strprint("Opening write pipe!\n");
    writeServerPipe = ocall_open("/tmp/BooleanSSE/iee_to_server", O_ASYNC | O_WRONLY);
    if(writeServerPipe < 0){
        ocall_strprint("Write pipe opening error! Terminating...\n");
        ocall_exit(-1);
    }

    ocall_strprint("Finished IEE init! Gonna start listening for client requests through bridge!\n");
}

static void destroy_pipes() {
    ocall_close(readServerPipe);
    ocall_close(writeServerPipe);
}

static void setup(bytes* out, size* out_len, const bytes in, const size in_len) {
    #ifdef VERBOSE
    ocall_strprint("IEE: Starting Setup!\n");
    #endif

    init_pipes();
    int pos = 1;

    // read kEnc
    const int kEnc_size = iee_readIntFromArr(in, &pos);

    unsigned char* kEnc = (unsigned char*)malloc(sizeof(unsigned char) * kEnc_size);
    iee_readFromArr(kEnc, kEnc_size, in, &pos);

    // read kF
    const int kF_size = iee_readIntFromArr(in, &pos);
    unsigned char* kF = (unsigned char*)malloc(sizeof(unsigned char) * kF_size);
    iee_readFromArr(kF, kF_size, in, &pos);

    /*printf("kEnc size %d\n", kEnc_size);
    printf("kF size %d\n", kF_size);*/

    // Keys are  given by the client (as input message)
    setKeys(kEnc, kF);

    // tell server to init index I
    unsigned char op = '1';
    iee_socketSend(writeServerPipe, &op, sizeof(char));

    // output message
    *out_len = 1;
    *out = (unsigned char*)malloc(sizeof(unsigned char));
    (*out)[0] = RES_OK;

    #ifdef VERBOSE
    ocall_strprint("IEE: Finished Setup!\n");
    #endif
}

static void add(bytes* out, size* out_len, const bytes in, const size in_len) {
    #ifdef VERBOSE
    //ocall_strprint("IEE: Started add!\n");
    #endif

    // read buffer
    int pos = 1;
    while(pos < in_len) {
        //get d,c,w from array
        const int doc_id = iee_readIntFromArr(in, &pos);
        const int c = iee_readIntFromArr(in, &pos);

        // read kW
        unsigned char* kW = (unsigned char*)malloc(sizeof(unsigned char) * H_BYTES);
        iee_readFromArr(kW, H_BYTES, in, &pos);

        //calculate index position label
        unsigned char* label = (unsigned char*)malloc(sizeof(unsigned char) * H_BYTES);
        c_hmac(label, (unsigned char*)&c, sizeof(int), kW);
        free(kW);

        /*for(unsigned xx = 0; xx < H_BYTES; xx++)
            printf("%02x ", label[xx]);
        printf(" : %d %d\n", d, c);*/

        // generate nonce
        unsigned char* nonce = (unsigned char*)malloc(sizeof(unsigned char) * C_NONCESIZE);
        for(int i = 0; i < C_NONCESIZE; i++)
            nonce[i] = 0x00;

        //calculate index value - enc_data
        const size_t unenc_size = H_BYTES + sizeof(int);
        unsigned char* unenc_data = (unsigned char*)malloc(sizeof(unsigned char) * unenc_size);
        iee_memcpy(unenc_data, label, H_BYTES);
        iee_memcpy(unenc_data + H_BYTES, &doc_id, sizeof(int));

        const size_t enc_size = unenc_size + C_EXPBYTES;
        unsigned char* enc_data = (unsigned char*)malloc(sizeof(unsigned char) * enc_size);
        memset(enc_data, 0, sizeof(unsigned char) * enc_size); // fix syscall param write(buf) points to uninitialised byte(s)
        c_encrypt(enc_data, unenc_data, unenc_size, nonce, get_kEnc());
        free(nonce);
        free(unenc_data);

        //send label and enc_data to server
        unsigned char op = '2';
        //ocall_printf("add %d %d\n", H_BYTES, enc_data_size);
        iee_socketSend(writeServerPipe, &op, sizeof(unsigned char));
        iee_socketSend(writeServerPipe, (unsigned char*)label, H_BYTES);
        iee_socketSend(writeServerPipe, (unsigned char*)enc_data, enc_size);

        /*print_buffer("add label", label, H_BYTES);
        print_buffer("add enc", enc_data, enc_data_size);
        ocall_strprint("\n");*/
        free(label);
        free(enc_data);
    }

    #ifdef VERBOSE
    //ocall_strprint("Finished add in IEE!\n");
    #endif

    // output message
    *out_len = 1;
    *out = (unsigned char*)malloc(sizeof(unsigned char));
    (*out)[0] = RES_OK;
}

static void get_docs_from_server(vec_token *query, unsigned count_words) {
    #ifdef VERBOSE
    ocall_strprint("Requesting docs from server!\n");
    #endif

    // initialise array to hold all tokens in random order
    iee_token *rand[count_words];
    for(unsigned i = 0; i < count_words; i++)
        rand[i] = NULL;

    // randomly fill the array with the tokens we need
    for(unsigned i = 0; i < vt_size(*query); i++) {
        // ignore non-word tokens
        if(query->array[i].type != WORD_TOKEN)
            continue;

        // choose a random unoccupied position from the rand array
        int pos;
        do {
            pos = c_random_uint_range(0, count_words);
        } while(rand[pos] != NULL);

        rand[pos] = &(*query).array[i];
    }

    /*for(unsigned ii = 0; ii < count_words; ii++) {
        if(rand[ii])
            printf("%c %s\n", rand[ii]->type, rand[ii]->word);
        else
            printf("%d\n", rand[ii]);
    }
    printf("\n");*/

    #ifdef VERBOSE
    ocall_strprint("Randomised positions!\n");
    #endif

    // request the documents from the server
    for(unsigned i = 0; i < count_words; i++) {
        iee_token *tkn = rand[i];
        //printf("word %d/%d\n", i, count_words);

        //cout << "counter for " << tkn->word << " is " << tkn->counter << endl;
        if(tkn->counter == 0) {
            vec_int dummy;
            tkn->docs = dummy;
        }

        //calculate relevant index positions
        unsigned char** labels = (unsigned char**)malloc(sizeof(unsigned char*) * tkn->counter);
        for (int c = 0; c < tkn->counter; c++) {
            labels[c] = (unsigned char*)malloc(sizeof(unsigned char) * H_BYTES);
            c_hmac(labels[c], (unsigned char*)&c, sizeof(int), tkn->kW);
            //print_buffer("label", labels[c], H_BYTES);
        }

        //free(kW);

        // generate 0-filled nonce
        unsigned char* nonce = (unsigned char*)malloc(sizeof(unsigned char) * C_NONCESIZE);
        for(unsigned j = 0; j < C_NONCESIZE; j++)
            nonce[j] = 0x00;

        int max_batch_size = 2000;

        ocall_strprint("requesting to server!\n");
        //request index positions from server
        int len = sizeof(char) + sizeof(int) + H_BYTES * max_batch_size;
        unsigned char* buff = (unsigned char*)malloc(sizeof(unsigned char) * len);

        char op = '3';
        int pos = 0;
        //iee_memcpy(buff, &op, sizeof(unsigned char));
        iee_addToArr(&op, sizeof(unsigned char), buff, &pos);

        // contains the hmac for verif, the doc id, and the enc's exp
        const size_t enc_len = H_BYTES + sizeof(int) + C_EXPBYTES; // 44 + H_BYTES (32)
        unsigned char* enc_data = (unsigned char*)malloc(sizeof(unsigned char) * (enc_len * tkn->counter));
        //printf("will have %d\n", tkn->counter);

        for(int j = 0; j < tkn->counter; j+=min(tkn->counter, max_batch_size)) {
            int will_get = min(tkn->counter - j, max_batch_size);
            //printf("will get %d\n", will_get);
            iee_addToArr(&will_get, sizeof(int), buff, &pos);
            //iee_memcpy(buff + 1, &will_get, sizeof(int));

            // add the labels
            for(int k = 0; k < will_get; k++) {
                iee_addToArr(labels[j+k], H_BYTES, buff, &pos);
                //iee_memcpy(buff + 1 + sizeof(int) + j * H_BYTES, labels[j+k], sizeof(unsigned char) * H_BYTES);
            }

            /*for(unsigned x = 0; x < will_get; x++){
                for(unsigned y = 0; y < H_BYTES; y++)
                    printf("%02x", labels[j+x][y]);
                printf("\n");
            }*/

            iee_socketSend(writeServerPipe, buff, sizeof(char) + sizeof(int) + H_BYTES * will_get);
/*
            for(int x = 0; x < sizeof(char) + sizeof(int) + H_BYTES * will_get; x++){
                printf("%02x", buff[x]);
            }*/

            // receive response
            iee_socketReceive(readServerPipe, enc_data + j * enc_len, enc_len * will_get);
            pos = 1; // reset pos to "beginning" of buff
        }

        free(buff);

        ocall_strprint("got from sv!\n");

        const size_t unenc_len = H_BYTES + sizeof(int);
        unsigned char* unenc_data = (unsigned char*)malloc(sizeof(unsigned char) * unenc_len);

        // holds doc ids as ints
        size_t doc_buff_len = tkn->counter * sizeof(int);
        unsigned char* doc_buff = (unsigned char*)malloc(sizeof(unsigned char) * doc_buff_len);
        memset(doc_buff, 0, sizeof(unsigned char) * doc_buff_len); // fix valgrind warning about vi_contains
        pos = 0;

        for (int j = 0; j < tkn->counter; j++) {
            c_decrypt(unenc_data, enc_data + (enc_len * j), enc_len, nonce, get_kEnc());

            /*for(unsigned x = 0; x < unenc_len; x++)
                printf("%02x", unenc_data[x]);
            printf("\n");*/

            /*for(unsigned x = 0; x < enc_len; x++)
                printf("%02x", enc_data[x]);
            printf("\n");*/

            unsigned char* label_verif = (unsigned char*)malloc(sizeof(unsigned char) * H_BYTES);
            iee_memcpy(label_verif, unenc_data, H_BYTES);
            iee_memcpy(doc_buff + j * sizeof(int), unenc_data + H_BYTES, sizeof(int));
            pos += sizeof(int);

            /*for(unsigned x = 0; x < H_BYTES; x++)
                printf("%02x", label_verif[x]);
            printf(" : \n");

            for(unsigned y = 0; y < H_BYTES; y++)
                printf("%02x", labels[j][y]);
            printf(" : \n");*/

            for(unsigned x = 0; x < H_BYTES; x++) {
                if(label_verif[x] != labels[j][x]) {
                    ocall_strprint("Label verification doesn't match! Exit\n");
                    ocall_exit(-1);
                }
            }

            //ocall_strprint("Verification made, ok\n");

            // delete keys from memory
            iee_bzero(enc_data, C_KEYSIZE);
            iee_bzero(unenc_data, C_KEYSIZE);

            free(label_verif);
        }

        free(nonce);
        for (int j = 0; j < tkn->counter; j++)
            free(labels[j]);
        free(labels);

        // generate int vector
        const int nr_docs = doc_buff_len / sizeof(int);
        //printf("nr docs %d\n", nr_docs);
        vec_int docs; // TODO check if this is always sorted
                      // else has to be sorted in evaluator; may not be needed for vec_int (as of October may not really be needed)
        vi_init(&docs, nr_docs);
        pos = 0;
        for (int j = 0; j < nr_docs; j++) {
            int tmp = -1;
            iee_memcpy(&tmp, doc_buff + pos, sizeof(int));
            pos += sizeof(int);
            //printf("doc %d\n", tmp);
            vi_push_back(&docs, tmp);
        }

        // insert result into token's struct
        tkn->docs = docs;
        //printf("docs retrieved %s\n", tkn->word);

        free(doc_buff);
        free(enc_data);
        free(unenc_data);
        //printf("done\n");
    }

    #ifdef VERBOSE
    ocall_strprint("Got all docs from server!\n\n");
    #endif
}

static void search(bytes* out, size* out_len, const bytes in, const size in_len) {
    #ifdef VERBOSE
    ocall_strprint("Search!\n");
    #endif
    /*for(int i = 0; i < in_len; i++)
        printf("%02x", in[i]);
    printf("\n");*/

    vec_token query;
    vt_init(&query, MAX_QUERY_TOKENS);

    int nDocs = -1;
    int count_words = 0; // useful for get_docs_from_server

    //read in
    int pos = 1;
    while(pos < in_len) {

        iee_token tkn;
        tkn.kW = NULL;

        char* tmp_type = (char*)malloc(sizeof(char));
        iee_readFromArr(tmp_type, 1, in, &pos);

        tkn.type = tmp_type[0];
        free(tmp_type);

        if(tkn.type == WORD_TOKEN) {
            count_words++;

            // read counter
            tkn.counter = iee_readIntFromArr(in, &pos);

            // read kW
            tkn.kW = (unsigned char*)malloc(sizeof(unsigned char) * H_BYTES);
            iee_readFromArr(tkn.kW, H_BYTES, in, &pos);

        } else if(tkn.type == META_TOKEN) {
            nDocs = iee_readIntFromArr(in, &pos);
            continue;
        }

        vt_push_back(&query, tkn);
    }

    // get documents from uee
    get_docs_from_server(&query, count_words);

    #ifdef VERBOSE
    /*ocall_strprint("parsed: ");
    for(unsigned i = 0; i < vt_size(query); i++) {
        iee_token x = query.array[i];
        if(x.type == WORD_TOKEN) {
            ocall_printf("%s (", x.word);
            for(unsigned i = 0; i < vi_size(x.docs); i++) {
                if(i < vi_size(x.docs) - 1)
                    ocall_printf("%i,", x.docs.array[i]);
                else
                    ocall_printf("%i); ", x.docs.array[i]);
            }
        } else {
            ocall_printf("%c ", x.type);
        }
    }
    ocall_strprint("\n\n");*/
    #endif

    //calculate boolean formula
    vec_int response_docs = evaluate(query, nDocs, count);

    #ifdef VERBOSE
    ocall_strprint("Query Evaluated in IEE!\n");
    #endif

    // return query results
    unsigned long long output_size = vi_size(response_docs) * sizeof(int);
    *out = (unsigned char*)malloc(sizeof(unsigned char) * output_size);
    pos = 0;

    for(unsigned i = 0; i < vi_size(response_docs); i++) {
        //ocall_printf("%d ", response_docs.array[i]);
        iee_addIntToArr(response_docs.array[i], *out, &pos);
    }
    //ocall_printf("\n");

    // free the buffers in the iee_tokens
    for(unsigned i = 0; i < vt_size(query); i++) {
        iee_token t = query.array[i];
        free(t.kW);
    }

    vt_destroy(&query);
    vi_destroy(&response_docs);

    #ifdef VERBOSE
    ocall_strprint("Finished Search!\n");
    #endif

    *out_len = sizeof(unsigned char) * output_size;benchmarking_print();
}
