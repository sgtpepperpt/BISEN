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

#include "../Client/SseClient.hpp"

extern "C" {
#include "../Common/Utils.h"
#include "../Iee/SseIee.h"
#include "../Iee/types.h"
}

int main(int argc, const char* argv[]) {
    vector<string> queries;
//    queries.push_back("enron && time");
//    queries.push_back("!enron && !time && !inform && !work && !call && !discuss && meet && week && receiv && dai");
//    queries.push_back("!enron && !time && !inform && !work && !call && !discuss && !meet && !week && !receiv && !dai");
//    queries.push_back("!(enron && time && inform && work && call && discuss && meet && week && receiv && dai)");
//    queries.push_back("!enron || time || inform || work || call || discuss || meet || week || receiv || dai");
//    queries.push_back("!enron || !time || !inform || !work || !call || discuss || meet || week || receiv || dai");
//    queries.push_back("!enron || !time || !inform || !work || !call || !discuss || !meet || !week || !receiv || !dai");
//    queries.push_back("!(enron || time || inform || work || call || discuss || meet || week || receiv || dai)");

    queries.push_back("enron && time");
    queries.push_back("enron && time");
    queries.push_back("enron && time");
    queries.push_back("enron && time");
    /*queries.push_back("enron && time && call && work && inform");
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
    queries.push_back("!(enron || time)");*/

    // init client
    SseClient client;
    unsigned char* data;
    unsigned long long data_size;

    // init output file
    FILE* out_f = fopen("bisen_benchmark", "wb");
    if (!out_f) {
        printf("Error opening output file!\n");
        exit(-1);
    }

    const char* dataset_dir = getenv("DATASET_DIR");
    if (!dataset_dir) {
        printf("DATASET_DIR not defined!\n");
        exit(1);
    }

    // leave space in file for nr of updates
    fseek(out_f, sizeof(size_t), SEEK_SET);

    // write number of searches to benchmark file
    size_t nr_searches = queries.size();//getenv("NUM_QUERIES")? atoi(getenv("NUM_QUERIES")) : 0;
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

    // get list of docs for test
    vector<string> doc_paths;
    client.listTxtFiles(dataset_dir, doc_paths);

    size_t nr_updates = 0;
    for (const string doc : doc_paths) {
        printf("%lu\n", nr_updates);
#ifdef VERBOSE
        if(!(i % 1000))
            printf("GENCLI update: (%d/%d)\n",i,nr_updates);
#endif

        gettimeofday(&start, NULL);

        vector<set<string>> docs = client.extractUniqueKeywords_wiki(dataset_dir + doc);
        nr_updates += docs.size();

        /*set<string>::iterator iter;
        for(iter=text.begin(); iter!=text.end();++iter){
            cout<<(*iter)<< " ";
        }*/

        for (const set<string> text : docs) {
            // generate the byte* to send to the server
            data_size = client.add_new_document(text, &data);
            gettimeofday(&end, NULL);
            total_add_time += util_time_elapsed(start, end);

#ifdef LOCALTEST
            gettimeofday(&start, NULL);

            //print_buffer("Data", data, data_size);
            output_size = 0;
            f(&output, &output_size, 0, (const bytes) data, data_size);
            //print_buffer("Output", output, output_size);
            free(output);

            gettimeofday(&end, NULL);
            total_sim_add_time += util_time_elapsed(start, end);
            //print_buffer("Output", output, output_size);
#endif

            // write to benchmark file
            fwrite(&data_size, sizeof(unsigned long long), 1, out_f);
            fwrite(data, sizeof(unsigned char), data_size, out_f);

            free(data);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // QUERIES /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    int total_search_time = 0;
    int total_sim_search_time = 0;

    for (unsigned k = 0; k < queries.size(); k++) {
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
        total_search_time += util_time_elapsed(start, end);

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
        total_sim_search_time += util_time_elapsed(start, end);

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

    // add number of updates to beginning of file
    fseek(out_f, 0, SEEK_SET);
    fwrite(&nr_updates, sizeof(size_t), 1, out_f);

    printf("GENCLI add queries: %lu\n", nr_updates);
    printf("GENCLI nr search queries: %lu\n", nr_searches);

    printf("GENCLI time: total client add = %6.3lf s!\n", total_add_time / 1000000.0);


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
