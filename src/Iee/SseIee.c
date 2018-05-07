//
//  SseIee.cpp
//  BISEN
//
//  Created by Bernardo Ferreira on 15/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include "SseIee.h"

const char* pipeDir = "/tmp/BooleanSSE/";

static int last_ndocs;
static unsigned char* aux_bool;

void print_buffer(const char* name, const unsigned char* buf, const unsigned long long len) {
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
    iee_socketSend(server_socket, tmp_buff, sizeof(unsigned char));
}

static void init_pipes() {
    /*char pipeName[256];

    //start server-iee pipe
    strncpy(pipeName, pipeDir, strlen(pipeDir)+1);
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
    }*/

    server_socket = ocall_sock_open("localhost", 7899);

    ocall_strprint("Finished IEE init! Gonna start listening for client requests through bridge!\n");
}

static void destroy_pipes() {
    ocall_close(server_socket);
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
    iee_socketSend(server_socket, &op, sizeof(char));

    // init aux_bool buffer
    aux_bool = NULL;
    last_ndocs = 0;

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
    //ocall_print_string("IEE: Started add!\n");
#endif

    // read buffer
    void* tmp = in + 1; // exclude op
    const size_t to_recv_len = in_len - sizeof(unsigned char);

    // generate nonce
    unsigned char* nonce = (unsigned char*)malloc(sizeof(unsigned char) * C_NONCESIZE);
    for(int i = 0; i < C_NONCESIZE; i++)
        nonce[i] = 0x00;

    // size of single buffers
    const size_t recv_len = H_BYTES + 2 * sizeof(int);
    const size_t d_unenc_len = H_BYTES + sizeof(int);
    const size_t d_enc_len = d_unenc_len + C_EXPBYTES;

    // size of batch requests to server
    size_t to_recv = to_recv_len / recv_len;

    // op to server
    const unsigned char op = '2';

    // buffers for reuse while receiving
    unsigned char* unenc_data = (unsigned char*)malloc(sizeof(unsigned char) * d_unenc_len);
    unsigned char* kW = (unsigned char*)malloc(sizeof(unsigned char) * H_BYTES);

    // will contain at most max_batch_size (l,id*) pairs
    const size_t pair_size = H_BYTES + d_enc_len;
    void* batch_buffer = malloc(MAX_BATCH_UPDATE * pair_size);

    while(to_recv) {
        memset(batch_buffer, 0, MAX_BATCH_UPDATE * (H_BYTES + d_enc_len)); // fix syscall param write(buf) points to uninitialised byte(s)

        // calculate size and get batch from client
        size_t batch_size = min(to_recv, MAX_BATCH_UPDATE);
        for (unsigned i = 0; i < batch_size; i++) {
            void* label = batch_buffer + i * pair_size;
            void* d = label + H_BYTES;

            //get doc_id, counter, kW from array
            int doc_id;
            memcpy(&doc_id, tmp, sizeof(int));
            tmp = (char*)tmp + sizeof(int);

            int counter;
            memcpy(&counter, tmp, sizeof(int));
            tmp = (char*)tmp + sizeof(int);

            // read kW
            memcpy(kW, tmp, H_BYTES);
            tmp = (char*)tmp + H_BYTES;

            // calculate "label" (key) and add to batch_buffer
            c_hmac(label, (unsigned char*)&counter, sizeof(int), kW);

            // calculate "id*" (entry)
            // hmac + doc_id
            memcpy(unenc_data, label, H_BYTES);
            memcpy(unenc_data + H_BYTES, &doc_id, sizeof(int));

            // store in batch_buffer
            c_encrypt(d, unenc_data, d_unenc_len, nonce, get_kEnc());
/*
            for(unsigned j = 0; j < 40; j++)
                printf("%02x", ((unsigned char*)unenc_data)[j]);
            printf(": dec\n")
            for(unsigned k = 0; k < 80; k++)
                printf("%02x", ((unsigned char*)d)[k]);
            printf(": cif\n");*/
        }

        // send batch to server
        iee_socketSend(server_socket, &op, sizeof(unsigned char));
        iee_socketSend(server_socket, &batch_size, sizeof(size_t));
        iee_socketSend(server_socket, batch_buffer, batch_size * pair_size);

        to_recv -= batch_size;
    }

    free(kW);
    free(unenc_data);
    free(batch_buffer);
    free(nonce);

#ifdef VERBOSE
    //ocall_print_string("Finished update in IEE!\n");
#endif

    // output message
    *out_len = 1;
    *out = (unsigned char*)malloc(sizeof(unsigned char));
    (*out)[0] = RES_OK;
}

static void get_docs_from_server(vec_token *query, const unsigned count_words, const unsigned total_labels) {
    #ifdef VERBOSE
    ocall_strprint("Requesting docs from server!\n");
    #endif

    // size of batch requests to server
    const unsigned max_batch_size = 2000;

    // initialise array to hold all tokens in random order
    /*iee_token *rand[count_words];
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
    }*/

    typedef struct {
        iee_token *tkn;
        unsigned counter_val;
    } label_request;

    // generate 0-filled nonce
    unsigned char* nonce = (unsigned char*)malloc(sizeof(unsigned char) * C_NONCESIZE);
    for(unsigned j = 0; j < C_NONCESIZE; j++)
        nonce[j] = 0x00;

    // used to hold all labels in random order
    label_request* labels = (label_request*)malloc(sizeof(label_request) * total_labels);
    memset(labels, 0, sizeof(label_request) * total_labels);

    // iterate over all the needed words, and then over all its occurences (given by the counter)
    // and fill the requests array
    int k = 0;
    for(unsigned i = 0; i < vt_size(*query); i++) {
        // ignore non-word tokens
        if(query->array[i].type != WORD_TOKEN)
            continue;

        for(unsigned j = 0; j < query->array[i].counter; j++) {
            int r = c_random_uint_range(0, k+1);
            if(r != k) {
                labels[k].tkn = labels[r].tkn;
                labels[k].counter_val = labels[r].counter_val;
            }

            labels[r].tkn = &(*query).array[i];
            labels[r].counter_val = j;

            k++;
            /*printf("apointer %p\n", labels[j]);
            printf("%p\n", labels[j].tkn);
            printf("%d\n", labels[j].counter_val);*/
        }
        //printf("-----------------------\n");
    }

    //printf("############################\n");
/*
    // shuffle requests (fisher yates)
    for(unsigned i = 0; i < total_labels - 1; i++) {


        iee_token * tmp_tkn = labels[r].tkn;
        unsigned tmp_ctr = labels[r].counter_val;

        labels[r].tkn = labels[i].tkn;
        labels[r].counter_val = labels[i].counter_val;

        labels[i].tkn = tmp_tkn;
        labels[i].counter_val = tmp_ctr;
    }
*/
    #ifdef VERBOSE
    ocall_strprint("Randomised positions!\n");
    #endif

    /************************ ALLOCATE DATA STRUCTURES ************************/
    // buffer for server requests
    // (always max_batch_size, may not be filled if not needed)
    // op + batch_size + labels of H_BYTES len
    size_t req_len = H_BYTES;
    unsigned char* req_buff = (unsigned char*)malloc(sizeof(unsigned char) * (sizeof(char) + sizeof(unsigned) + req_len * max_batch_size));

    // put the op code in the buffer
    const char op = '3';
    iee_memcpy(req_buff, &op, sizeof(unsigned char));

    // buffer for encrypted server responses
    // contains the hmac for verif, the doc id, and the encryption's exp
    const size_t res_len = H_BYTES + sizeof(int) + C_EXPBYTES; // 44 + H_BYTES (32)
    unsigned char* res_buff = (unsigned char*)malloc(sizeof(unsigned char) * (res_len * max_batch_size));

    // buffer for decrypted server responses (holds one)
    const size_t dec_len = H_BYTES + sizeof(int);
    unsigned char* dec_buff = (unsigned char*)malloc(sizeof(unsigned char) * dec_len);
    /********************** END ALLOCATE DATA STRUCTURES **********************/

    unsigned label_pos = 0;
    unsigned batch_size = min(total_labels - label_pos, max_batch_size);

    // request labels to server
    while (label_pos < total_labels) {
        // put batch_size in buffer
        iee_memcpy(req_buff + sizeof(char), &batch_size, sizeof(unsigned));

        // aux pointer
        unsigned char* tmp = req_buff + sizeof(char) + sizeof(unsigned);

        // fill the buffer with labels
        for(unsigned i = 0; i < batch_size; i++) {
            label_request* req = &(labels[label_pos + i]);
            /*printf("pointer %p\n", req);
            printf("%p\n", req->tkn);
            printf("%d\n", req->counter_val);
            printf("-----------------------\n");*/
            c_hmac(tmp + i * req_len, (unsigned char*)&(req->counter_val), sizeof(int), req->tkn->kW);
        }

        /*printf("%d batch\n", batch_size);
        for(unsigned x = 0; x < batch_size*req_len; x++)
            printf("%02x", tmp[x]);
        printf(" : \n");*/

        // send message to server and receive response
        //ocall_strprint("aRequesting to server!\n");
        iee_socketSend(server_socket, req_buff, sizeof(char) + sizeof(unsigned) + req_len * batch_size);
        iee_socketReceive(server_socket, res_buff, res_len * batch_size);
        //ocall_strprint("Got from server!\n");

        // decrypt and fill the destination data structs
        for(unsigned i = 0; i < batch_size; i++) {
            label_request* req = &labels[label_pos + i];
            c_decrypt(dec_buff, res_buff + (res_len * i), res_len, nonce, get_kEnc());

            // aux pointer for req
            unsigned char* tmp = req_buff + sizeof(char) + sizeof(unsigned);

            // verify
            for(unsigned j = 0; j < H_BYTES; j++) {
                if(dec_buff[j] != (tmp + i * req_len)[j]) {
                    ocall_strprint("Label verification doesn't match! Exit\n");
                    ocall_exit(-1);
                }
            }

            iee_memcpy(&req->tkn->docs.array[req->counter_val], dec_buff + H_BYTES, sizeof(int));

            /*for(unsigned x = 0; x < H_BYTES; x++)
                printf("%02x", label_verif[x]);
            printf(" : \n");*/
        }

        label_pos += batch_size;
        batch_size = min(total_labels - label_pos, max_batch_size);
    }

    free(labels);
    free(nonce);

    iee_bzero(req_buff, req_len * max_batch_size);
    iee_bzero(res_buff, res_len * max_batch_size);
    iee_bzero(dec_buff, dec_len);

    free(req_buff);
    free(res_buff);
    free(dec_buff);

    /*for(unsigned ii = 0; ii < count_words; ii++) {
        if(rand[ii])
            printf("%c %s\n", rand[ii]->type, rand[ii]->word);
        else
            printf("%d\n", rand[ii]);
    }
    printf("\n");*/

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
    int count_labels = 0; // useful for get_docs_from_server

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
            count_labels += tkn.counter;

            // read kW
            tkn.kW = (unsigned char*)malloc(sizeof(unsigned char) * H_BYTES);
            iee_readFromArr(tkn.kW, H_BYTES, in, &pos);

            // create the vector that will hold the docs
            vi_init(&tkn.docs, tkn.counter);
            tkn.docs.counter = tkn.counter;
        } else if(tkn.type == META_TOKEN) {
            nDocs = iee_readIntFromArr(in, &pos);
            continue;
        }

        vt_push_back(&query, tkn);
    }

    // get documents from uee
    get_docs_from_server(&query, count_words, count_labels);

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

    // ensure aux_bool has space for all docs
    if(nDocs > last_ndocs) {
        //ocall_strprint("Realloc aux bool buffer\n");
        aux_bool = (unsigned char *)realloc(aux_bool, sizeof(unsigned char) * nDocs);
        last_ndocs = nDocs;
    }

    //calculate boolean formula
    vec_int response_docs = evaluate(query, nDocs, aux_bool);

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
