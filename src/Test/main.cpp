//
//  main.cpp
//  BISEN
//
//  Created by Bernardo Ferreira and Guilherme Borges.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "../Common/Utils.h"
#include "../Client/SseClient.hpp"

extern "C" {
#include "../Iee/SseIee.h"
#include "../Iee/types.h"
}

// DEBUGGING
//#include <map>
//map<int, int> f;

/*
extern void f(
  bytes *out,
  size *out_len,
  const label pid,
  const bytes in,
  const size in_len
);
*/
extern int f_assert(
  size test_index,
  bytes out,
  size outlen
);

void printResults (vector<int> results) {
    #ifdef VERBOSE
    if(!results.size()) {
        printf("none\n");
        return;
    }

    sort(results.begin(), results.end());
    for(unsigned i = 0; i < results.size(); i++)
        printf("%i ", results[i]);

    printf("\n");
    //f[results.size()]++;
    #endif
}

void print_buffer(const char* name, const unsigned char * buf, const unsigned long long len) {
    printf("%s size: %llu\n", name, len);
    for(unsigned i = 0; i < len; i++)
        printf("%02x", buf[i]);
    printf("\n");
}

int main(int argc, const char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    // init iee
    init_pipes();

    SseClient client;
    unsigned char* data;
    unsigned long long data_size = client.setup(&data);

    //print_buffer("Data", data, data_size);

    ////////////////////////////////////////////////////////////////////////////
    // SETUP ///////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    printf("%d %d\n", crypto_auth_hmacsha256_KEYBYTES, crypto_secretbox_KEYBYTES);
    #ifdef VERBOSE
    printf("Starting IEE communication\n");
    #endif

    unsigned char * output;
    unsigned long long output_size;
    f(&output, &output_size, 0, (const unsigned char*) data, data_size);

    //print_buffer("Output", output, output_size);

    //free(data);
    //free(output);

    const string base_dir = "../Data/parsed/";
    const int num_queries = 1;

    // random query parameters
    const int size = 2; // size will be about between [size, size * 2]
    const int not_prob = 5;
    const int and_prob = 30;

    // get list of docs for test
    vector<string> doc_paths;
    client.listTxtFiles(base_dir, doc_paths);

    ////////////////////////////////////////////////////////////////////////////
    // UPDATE //////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // add documents from the directory
    set<string> all_words_set; // for client-side random query generation only
    for(string doc : doc_paths){
        set<string> text = client.extractUniqueKeywords(base_dir + doc);

        // generate the byte* to send to the server
        unsigned char* data;
        unsigned long long data_size = client.add_new_document(text, &data);

        //print_buffer("Data", data, data_size);

        output_size = 0;
        f(&output, &output_size, 0, (const unsigned char*) data, data_size);

        //print_buffer("Output", output, output_size);

        // add all new words to a set, used later to generate queries
        all_words_set.insert(text.begin(), text.end());
    }

    ////////////////////////////////////////////////////////////////////////////
    // QUERIES /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // generate random queries
    vector<string> all_words(all_words_set.size());
    copy(all_words_set.begin(), all_words_set.end(), all_words.begin());

    for(unsigned i = 0; i < num_queries; i++) {
        string query = "sherlock && holmes || ze";//client.generate_random_query(all_words, size, not_prob, and_prob);

        #ifdef VERBOSE
       // cout << "\n----------------------------\nQuery: " << query << endl;
        #endif

        unsigned char* data;
        unsigned long long data_size = client.search(query, &data);

        //print_buffer("Data", data, data_size);
        output_size = 0;
        f(&output, &output_size, 0, (const unsigned char*) data, data_size);
        //print_buffer("Output", output, output_size);

        //process results
        const int nDocs = output_size / sizeof(int);

        #ifdef VERBOSE
        printf("Number of docs: %d\n", nDocs);
        #endif

        vector<int> results(nDocs);
        int pos = 0;
        for (int i = 0; i < nDocs; i++) {
            results[i] = readIntFromArr(output, &pos); // sem esta linha, o client nao compila... porque?? TODO TODO
        }

        printResults(results);
    }

    //for (auto const& x : f)
    //    cout << x.first << ':' << x.second << endl;

    return 0;
}
