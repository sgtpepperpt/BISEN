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

///////////////////////////// TESTING PARAMETERS /////////////////////////////
#define NUM_QUERIES 1
#define DATASET_DIR "../Data/parsed/"

// query size will be aprox. between
// [QUERY_WORD_COUNT, QUERY_WORD_COUNT * 2]
#define QUERY_WORD_COUNT 5

// probabilities between 0 and 100
#define NOT_PROBABILITY 5
#define AND_PROBABILITY 30

/////////////////////////// END TESTING PARAMETERS ///////////////////////////

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
    // init_pipes();

    // init output file
    FILE *out_f = fopen("test_all","wb");
    if (!out_f) {
		printf("Error opening output file!\n");
		exit(-1);
	}

    // init client
    SseClient client;
    unsigned char* data;
    unsigned long long data_size;

    ////////////////////////////////////////////////////////////////////////////
    // SETUP ///////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    data_size = client.setup(&data);
    //print_buffer("Data", data, data_size);

    // write to benchmark file
    fwrite(&data_size, sizeof(unsigned long long), 1, out_f);
    fwrite(data, sizeof(unsigned char), data_size, out_f);

    #ifdef VERBOSE
    printf("Starting IEE communication\n");
    #endif

    unsigned char * output;
    unsigned long long output_size;
    f(&output, &output_size, 0, (const bytes) data, data_size);

    // write to benchmark file
    fwrite(&data_size, sizeof(unsigned long long), 1, out_f);
    fwrite(data, sizeof(unsigned char), data_size, out_f);

    //print_buffer("Output", output, output_size);

    free(data);
    free(output);

    // get list of docs for test
    vector<string> doc_paths;
    client.listTxtFiles(DATASET_DIR, doc_paths);

    ////////////////////////////////////////////////////////////////////////////
    // UPDATE //////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // add documents from the directory
    set<string> all_words_set; // for client-side random query generation only
    for(string doc : doc_paths){
        set<string> text = client.extractUniqueKeywords(DATASET_DIR + doc);

        // generate the byte* to send to the server
        unsigned char* data;
        unsigned long long data_size = client.add_new_document(text, &data);

        //print_buffer("Data", data, data_size);

        output_size = 0;
        f(&output, &output_size, 0, (const bytes) data, data_size);

        //print_buffer("Output", output, output_size);

        // write to benchmark file
        fwrite(&data_size, sizeof(unsigned long long), 1, out_f);
        fwrite(data, sizeof(unsigned char), data_size, out_f);

        free(data);
        free(output);

        // add all new words to a set, used later to generate queries
        all_words_set.insert(text.begin(), text.end());
    }

    ////////////////////////////////////////////////////////////////////////////
    // QUERIES /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // generate random queries
    vector<string> all_words(all_words_set.size());
    copy(all_words_set.begin(), all_words_set.end(), all_words.begin());

    for(unsigned i = 0; i < NUM_QUERIES; i++) {
        string query = client.generate_random_query(all_words,
                            QUERY_WORD_COUNT, NOT_PROBABILITY, AND_PROBABILITY);

        #ifdef VERBOSE
       // cout << "\n----------------------------\nQuery: " << query << endl;
        #endif

        unsigned char* data;
        unsigned long long data_size = client.search(query, &data);

        //print_buffer("Data", data, data_size);
        output_size = 0;
        f(&output, &output_size, 0, (const bytes) data, data_size);
        //print_buffer("Output", output, output_size);

        // write to benchmark file
        fwrite(&data_size, sizeof(unsigned long long), 1, out_f);
        fwrite(data, sizeof(unsigned char), data_size, out_f);

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
        free(data);
        free(output);
    }

    //for (auto const& x : f)
    //    cout << x.first << ':' << x.second << endl;

    // close benchamrk file
    fclose(out_f);

    return 0;
}
