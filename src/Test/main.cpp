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
//#define NUM_QUERIES 1

// query size will be aprox. between
// [QUERY_WORD_COUNT, QUERY_WORD_COUNT * 2]
#define QUERY_WORD_COUNT 5

// probabilities between 0 and 100
#define NOT_PROBABILITY 0
#define AND_PROBABILITY 50

/////////////////////////// END TESTING PARAMETERS ///////////////////////////

// DEBUGGING
//#include <map>
//map<int, int> f;

/*int count_occurences(vector<int> v, int elem) {
    int count = 0;
    for(unsigned i = 0; i < v.size(); i++) {
        if(v[i] == elem)
            count++;
    }
    return count;
}*/

void printResults (vector<int> results) {
    //#ifdef VERBOSE
    if(!results.size()) {
        printf("none\n");
        return;
    }

    int crisis = 0;

    sort(results.begin(), results.end());
    for(unsigned i = 0; i < results.size(); i++) {
        printf("%i ", results[i]);
        /*if(count_occurences(results, results[i]) > 1)
            crisis = 1;*/
    }

    printf("\n");
    //f[results.size()]++;
    //#endif

   /* if(crisis)
        exit(-1);*/
}

void print_buffer(const char* name, const unsigned char * buf, const unsigned long long len) {
    printf("%s size: %llu\n", name, len);
    for(unsigned i = 0; i < len; i++)
        printf("%02x", buf[i]);
    printf("\n");
}

int main(int argc, const char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    vector <string> queries;
//    queries.push_back("enron && time");
//    queries.push_back("!enron && !time && !inform && !work && !call && !discuss && meet && week && receiv && dai");
//    queries.push_back("!enron && !time && !inform && !work && !call && !discuss && !meet && !week && !receiv && !dai");
//    queries.push_back("!(enron && time && inform && work && call && discuss && meet && week && receiv && dai)");
//    queries.push_back("!enron || time || inform || work || call || discuss || meet || week || receiv || dai");
//    queries.push_back("!enron || !time || !inform || !work || !call || discuss || meet || week || receiv || dai");
//    queries.push_back("!enron || !time || !inform || !work || !call || !discuss || !meet || !week || !receiv || !dai");
//    queries.push_back("!(enron || time || inform || work || call || discuss || meet || week || receiv || dai)");

    queries.push_back("enron && time");
    queries.push_back("enron && time && call && work && inform");
    queries.push_back("enron && time && inform && work && call && discuss && meet && week && receiv && dai");
    queries.push_back("enron || time");
    queries.push_back("enron || time || call || work || inform");
    queries.push_back("enron || time || inform || work || call || discuss || meet || week || receiv || dai");
    queries.push_back("(call || enron) && (time || attach)");
    queries.push_back("(call || enron) && (time || attach) && (inform || work) && (meet || week)");
    queries.push_back("(call && enron) || (time && attach)");
    queries.push_back("(call && enron) || (time && attach) || (inform && work) || (meet && week)");
    queries.push_back("!enron && !time");
    queries.push_back("!(enron && time)");
    queries.push_back("!enron || !time");
    queries.push_back("!(enron || time)");

    // init iee
    // init_pipes();

    // init client
    SseClient client;
    unsigned char* data;
    unsigned long long data_size;

    // init output file
    FILE *out_f = fopen("bisen_benchmark", "wb");
    if (!out_f) {
		printf("Error opening output file!\n");
		exit(-1);
	}

    const char* dataset_dir = getenv("DATASET_DIR");

    // get list of docs for test
    vector<string> doc_paths;
    client.listTxtFiles(dataset_dir, doc_paths);

    // write number of adds / updates and searches to benchmark file
    size_t nr_updates = doc_paths.size();
    size_t nr_searches = queries.size();//getenv("NUM_QUERIES")? atoi(getenv("NUM_QUERIES")) : 0;
    fwrite(&nr_updates, sizeof(size_t), 1, out_f);
    fwrite(&nr_searches, sizeof(size_t), 1, out_f);

    ////////////////////////////////////////////////////////////////////////////
    // SETUP ///////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    data_size = client.setup(&data);
    //print_buffer("Data", data, data_size);

    // write to benchmark file
    fwrite(&data_size, sizeof(unsigned long long), 1, out_f);
    fwrite(data, sizeof(unsigned char), data_size, out_f);

    #ifdef VERBOSE
    printf("GENCLI Starting IEE communication\n");
    #endif

    #ifdef LOCALTEST
    unsigned char * output;
    unsigned long long output_size;
    f(&output, &output_size, 0, (const bytes) data, data_size);
    //print_buffer("Output", output, output_size);
    free(output);
    #endif

    free(data);

    ////////////////////////////////////////////////////////////////////////////
    // UPDATE //////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    int count = 0;

    long total_add_time = 0;
    long total_sim_add_time = 0;

    struct timeval start, end;

    // add documents from the directory
    set<string> all_words_set; // for client-side random query generation only
    for(unsigned i = 0; i < nr_updates; i++) {
        #ifdef VERBOSE
        if(!(i % 1000))
            printf("GENCLI update: (%d/%d)\n",i,nr_updates);
        #endif

        string doc = doc_paths[i];
        gettimeofday(&start, NULL);
        set<string> text = client.extractUniqueKeywords(dataset_dir + doc);


        /*set<string>::iterator iter;
        for(iter=text.begin(); iter!=text.end();++iter){
            cout<<(*iter)<< " ";
        }*/

        // generate the byte* to send to the server
        unsigned char* data;
        unsigned long long data_size = client.add_new_document(text, &data);
        gettimeofday(&end, NULL);
        total_add_time += timeElapsed(start, end);

        #ifdef LOCALTEST
        gettimeofday(&start, NULL);

        //print_buffer("Data", data, data_size);
        output_size = 0;
        f(&output, &output_size, 0, (const bytes) data, data_size);
        //print_buffer("Output", output, output_size);
        free(output);

        gettimeofday(&end, NULL);
        total_sim_add_time += timeElapsed(start, end);
        //print_buffer("Output", output, output_size);
        #endif

        // write to benchmark file
        fwrite(&data_size, sizeof(unsigned long long), 1, out_f);
        fwrite(data, sizeof(unsigned char), data_size, out_f);

        free(data);

        // add all new words to a set, used later to generate queries
        all_words_set.insert(text.begin(), text.end());
    }

    //client.list_words(); // used to get word frequencies in dataset

    ////////////////////////////////////////////////////////////////////////////
    // QUERIES /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // generate random queries
    vector<string> all_words(all_words_set.size());
    copy(all_words_set.begin(), all_words_set.end(), all_words.begin());

    int total_search_time = 0;
    int total_sim_search_time = 0;

    for(unsigned k = 0; k < queries.size(); k++) {
        string query = queries[k];
        //printf("query %s\n", query.c_str());
        //for(unsigned i = 0; i < nr_searches; i++) {
            //string query;

            /*if(getenv("QUERY"))
                query = getenv("QUERY");
            else
                query = client.generate_random_query(all_words, QUERY_WORD_COUNT, NOT_PROBABILITY, AND_PROBABILITY);*/

            //#ifdef VERBOSE
            printf("\n----------------------------\n");
            printf("Query %d: %s\n", k, query.c_str());
            //#endif

            gettimeofday(&start, NULL);
            unsigned char* data;
            unsigned long long data_size = client.search(query, &data);
            gettimeofday(&end, NULL);
            total_search_time += timeElapsed(start, end);

            #ifdef VERBOSE
            /*for(int i = 0; i < data_size; i++){
                if(data[i] >= 0x71 && data[i] <= 0x7A)
                    printf("%c ", data[i]);
                else
                    printf("%02x", data[i]);
            }
            printf("\n");*/
            #endif

            // write to benchmark file
            fwrite(&data_size, sizeof(unsigned long long), 1, out_f);
            fwrite(data, sizeof(unsigned char), data_size, out_f);

            #ifdef LOCALTEST
            //print_buffer("Data", data, data_size);
            gettimeofday(&start, NULL);
            output_size = 0;
            f(&output, &output_size, 0, (const bytes) data, data_size);
            gettimeofday(&end, NULL);
            //print_buffer("Output", output, output_size);
            total_sim_search_time += timeElapsed(start, end);

            //process results
            const int nDocs = output_size / sizeof(int);

            #ifdef VERBOSE
            printf("GENCLI Number of docs: %d\n", nDocs);
            #endif

            /*vector<int> results(nDocs);
            int pos = 0;
            for (int i = 0; i < nDocs; i++) {
                results[i] = readIntFromArr(output, &pos);
            }

            printResults(results);*/
            free(output);
            #endif

            free(data);
        //}

        printf("GENCLI time: client search = %6.6lf s!\n", total_search_time / 1000000.0);
        total_search_time = 0;
    }

    printf("GENCLI add queries: %lu\n", nr_updates);
    printf("GENCLI nr search queries: %lu\n", nr_searches);

    printf("GENCLI time: total client add = %6.3lf s!\n", total_add_time/1000000.0);


    #ifdef LOCALTEST
    printf("LTEST GENCLI time: client add = %6.3lf s!\n", total_sim_add_time/1000000.0);
    printf("LTEST GENCLI time: total search = %6.6lf s!\n", total_sim_search_time/1000000.0);
    #endif

    //TODO hack just to compile, no idea why needed, doesn't affect sgx
    unsigned char x[1];
    int xx = 0;
    int xy = readIntFromArr(x, &xx);

    //for (auto const& x : f)
    //    cout << x.first << ':' << x.second << endl;

    // close benchamrk file
    fclose(out_f);

    return 0;
}
