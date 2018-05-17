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

int main(int argc, char** argv) {
    int modify_existing = 0;
    char* filename = (char*)"bisen_benchmark_wiki";

    int use_cutoff = 0;
    unsigned long cutoff_point = 0;

    // parse terminal arguments
    int c;
    while ((c = getopt(argc, argv, "m:c:")) != -1) {
        switch (c) {
            case 'm':
                modify_existing = 1;
                filename = optarg;
                break;
            case 'c':
                use_cutoff = 1;
                cutoff_point = stoul(optarg);
                break;
            case '?':
                if (optopt == 'c')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(1);
            default:
                exit(-1);
        }
    }

    if(use_cutoff)
        printf("Updates cut off at %lu\n", cutoff_point);
    else
        printf("Full dataset updates\n");

    // define queries for test
    vector<string> queries;
    /*queries.push_back("portugal");
    queries.push_back("!portugal");

    queries.push_back("time");
    queries.push_back("person");
    queries.push_back("year");
    queries.push_back("way");
    queries.push_back("day");
    queries.push_back("thing");
    queries.push_back("man");
    queries.push_back("world");
    queries.push_back("life");
    queries.push_back("hand");

    queries.push_back("history");
    queries.push_back("country");
    queries.push_back("born");
    queries.push_back("lisbon");
    queries.push_back("york");
    queries.push_back("paris");

    queries.push_back("time && person");
    queries.push_back("time && person && year && way && day");
    queries.push_back("time && person && year && way && day && thing && man && world && life && hand");

    queries.push_back("time || person");
    queries.push_back("time || person || year || way || day");
    queries.push_back("time || person || year || way || day || thing || man || world || life || hand");
    */
    queries.push_back("!time && person && year && way && day && thing && man && world && life && hand");
    queries.push_back("!time && person && year && way && day && thing && man && world && life && hand");
    
    queries.push_back("!time && person && year && way && day && thing && man && world && life && hand");
    queries.push_back("!time && !person && !year && !way && !day && thing && man && world && life && hand");
    queries.push_back("!time && !person && !year && !way && !day && !thing && !man && !world && !life && !hand");
    queries.push_back("!(time && person && year && way && day && thing && man && world && life && hand)");

    /*queries.push_back("(time && person) || (year && way)");
    queries.push_back("(time && person) || (year && way) || (day && thing) || (man && world)");
    queries.push_back("(time || person) && (year || way)");
    queries.push_back("(time || person) && (year || way) && (day || thing) && (man || world)");
*/
    // init client
    SseClient client;
    unsigned char* data;
    unsigned long long data_size;

    // init output file
    FILE* out_f;

    if(modify_existing) {
        out_f = fopen(filename, "rb+");
    } else {
        out_f = fopen(filename, "wb");
    }

    if (!out_f) {
        printf("Error opening output file!\n");
        exit(-1);
    }

    const char* dataset_dir = getenv("DATASET_DIR");
    if (!dataset_dir) {
        printf("DATASET_DIR not defined!\n");
        exit(1);
    }

    struct timeval start, end;
    size_t nr_updates = 0;
    size_t nr_searches = queries.size();//getenv("NUM_QUERIES")? atoi(getenv("NUM_QUERIES")) : 0;

    // do setup and update
    if(!modify_existing){
        // leave space in file for nr of updates
        fseek(out_f, sizeof(size_t), SEEK_SET);

        // write number of searches to benchmark file
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

        // get list of docs for test
        vector<string> doc_paths;
        client.listTxtFiles(dataset_dir, doc_paths);

        for (const string doc : doc_paths) {
            //printf("%lu\n", nr_updates);
#ifdef VERBOSE
            if(!(i % 1000))
            printf("GENCLI update: (%d/%d)\n",i,nr_updates);
#endif

            // extract keywords from a 1M, multiple article, file
            gettimeofday(&start, NULL);

            vector<set<string>> docs = client.extractUniqueKeywords_wiki(dataset_dir + doc);
            nr_updates += docs.size();

            gettimeofday(&end, NULL);
            total_add_time += util_time_elapsed(start, end);

            /*set<string>::iterator iter;
            for(iter=text.begin(); iter!=text.end();++iter){
                cout<<(*iter)<< " ";
            }*/

            for (const set<string> text : docs) {
                // generate the byte* to send to the server
                gettimeofday(&start, NULL);
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

            if(use_cutoff && nr_updates > cutoff_point)
                break;
        }

        printf("GENCLI time: total client add = %6.3lf s!\n", total_add_time / 1000000.0);
    } else {
        // read updates and jump over nr_searches
        fread(&nr_updates, sizeof(size_t), 1, out_f);
        fseek(out_f, sizeof(size_t), SEEK_SET);

        // advance over all updates up to the search part
        for (size_t i = 0; i < nr_updates; ++i) {
            size_t len;
            fread(&len, sizeof(size_t), 1, out_f);
            fseek(out_f, len, SEEK_SET);
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
        data_size = client.search(query, &data);
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

    if(!modify_existing) {
        // add number of updates to beginning of file
        fseek(out_f, 0, SEEK_SET);
        fwrite(&nr_updates, sizeof(size_t), 1, out_f);

        printf("GENCLI add queries: %lu\n", nr_updates);
    } else {
        // add number of searches to beginning of file
        fseek(out_f, sizeof(size_t), SEEK_SET);
        fwrite(&nr_searches, sizeof(size_t), 1, out_f);
    }

    printf("GENCLI nr search queries: %lu\n", nr_searches);


#ifdef LOCALTEST
    printf("LTEST GENCLI time: client add = %6.3lf s!\n", total_sim_add_time/1000000.0);
    printf("LTEST GENCLI time: total search = %6.6lf s!\n", total_sim_search_time/1000000.0);
#endif

    //for (auto const& x : f)
    //    cout << x.first << ':' << x.second << endl;

    // close benchamrk file
    fclose(out_f);

    return 0;
}
