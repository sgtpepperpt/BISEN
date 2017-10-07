//
//  SSE_simple.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 20/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#include "SseClient.hpp"

using namespace std;

SseClient::SseClient() {
    // init data structures
    //openQueryResponseSocket();
    client_init_crypt();
    analyzer = new EnglishAnalyzer;
    W = new map<string,int>;    /**TODO persist W*/
    nDocs = 0;
}

SseClient::~SseClient() {
    client_destroy_crypt();
    delete analyzer;
    delete W;
    close(querySocket);
}

unsigned long long SseClient::setup(unsigned char** data) {
    // get keys
    //unsigned char* kCom = crypto->getKcom();
    unsigned char* kEnc = client_get_kEnc();
    unsigned char* kF = client_get_kF();

    // pack the keys into a buffer
    unsigned long long data_size = sizeof(unsigned char)
                                   + 2 * sizeof(int)
                                   + client_symBlocksize * sizeof(unsigned char)
                                   + client_fBlocksize * sizeof(unsigned char);
    *data = new unsigned char[data_size];

    unsigned char op = 'i';
    int pos = 0;
    addToArr(&op, sizeof(unsigned char), *data, &pos);

    // TODO client envia o kCom, deve ser lido pela framework do norte
    // add kCom to buffer
    /*addIntToArr(symBlocksize, data, &pos);
    for (int i = 0; i < symBlocksize; i++)
        addToArr(&kCom[i], sizeof(unsigned char), data, &pos);*/

    // add kEnc to buffer
    addIntToArr(client_symBlocksize, *data, &pos);
    for (int i = 0; i < client_symBlocksize; i++)
        addToArr(&kEnc[i], sizeof(unsigned char), *data, &pos);

    // add kF to buffer
    addIntToArr(client_fBlocksize, *data, &pos);
    for (int i = 0; i < client_fBlocksize; i++)
        addToArr(&kF[i], sizeof(unsigned char), *data, &pos);

    return data_size;
}

int SseClient::newDoc() {
    return nDocs++;
}

set<string> SseClient::extractUniqueKeywords(string fname) {
    return analyzer->extractUniqueKeywords(fname);
}

unsigned long long SseClient::add_new_document(set<string> text, unsigned char** data) {
    int id = newDoc();

    //timeval start, end;
    //gettimeofday(&start, 0);

    // add words to the newly generated document
    unsigned long long data_size = add_words(id, text, data);

    //gettimeofday(&end, 0);
    //unsigned long long elapsed = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000;

    //printf("Finished add document #%d (%zu words, %llu ms)\n", id, text.size(), elapsed);

    return data_size;
}

unsigned long long SseClient::add_words(int doc_id, set<string> words, unsigned char** data) {
    unsigned long long data_size = sizeof(unsigned char);

    // first iteration: to get the size of the buffer to allocate
    for(string w : words) {
        data_size += 2*sizeof(int) + (int)w.size() + 1;
    }

    //allocate data buffer
    *data = new unsigned char[data_size];

    unsigned char op = 'a';
    int pos = 0;
    addToArr(&op, sizeof(unsigned char), *data, &pos);

    // second iteration: to fill the buffer
    for(string w : words) {
        //get counter c for w
        int c = 1; // with new words, this is the first instance of it
        map<string,int>::iterator it = W->find(w);
        if (it != W->end())
            c = it->second + 1;

        // update counter c
        (*W)[w] = c;

        addIntToArr(doc_id, *data, &pos);
        addIntToArr(c - 1, *data, &pos); // counter starts at 1, so -1 for indexing
        for (unsigned i = 0; i < w.size(); i++)
            addToArr(&w[i], sizeof(unsigned char), *data, &pos);
        
        unsigned char term = '\0';
        addToArr(&term, sizeof(unsigned char), *data, &pos);
    }

    return data_size;
}

//boolean operands: AND, OR, NOT, (, )
int SseClient::search(string query, unsigned char** data) {
    // parse the query into token structs and apply the shunting yard algorithm
    vector<client_token> infix_query = parser->tokenize(query);
    vector<client_token> rpn = parser->shunting_yard(infix_query);

    int data_size = sizeof(unsigned char); // char from op

    // first query iteration: to get needed size and counters
    for(unsigned i = 0; i < rpn.size(); i++) {
        client_token *tkn = &rpn[i];

        if(tkn->type == WORD_TOKEN) {
            map<string,int>::iterator counterIt = W->find(tkn->word);
            if(counterIt != W->end())
                tkn->counter = counterIt->second;
            else
                tkn->counter = 0;

            //printf("counter %s %d\n", tkn->word.c_str(), tkn->counter);
            data_size += sizeof(unsigned char) + sizeof(int) + (tkn->word.size() + 1);
        } else {
            data_size += sizeof(unsigned char);
        }
    }

    // add number of documents to the data structure, needed for NOT
    client_token t;
    t.type = META_TOKEN;
    t.counter = nDocs;

    rpn.push_back(t);
    data_size += sizeof(unsigned char) + sizeof(int);

    //prepare query
    *data = new unsigned char[data_size];
    int pos = 0;

    unsigned char op = 's';
    addToArr(&op, sizeof(unsigned char), *data, &pos);

    // second query iteration: to fill "data" buffer
    for(vector<client_token>::iterator it = rpn.begin(); it != rpn.end(); ++it) {
        client_token tkn = *it;

        addToArr(&(tkn.type), sizeof(unsigned char), *data, &pos);

        if(tkn.type == WORD_TOKEN) {
            addIntToArr(tkn.counter, *data, &pos);

            for (unsigned i = 0; i < tkn.word.size(); i++)
                addToArr(&tkn.word[i], sizeof(unsigned char), *data, &pos);

            char term = '\0';
            addToArr(&term, sizeof(unsigned char), *data, &pos);
        } else if(tkn.type == META_TOKEN) {
            addIntToArr(tkn.counter, *data, &pos);
        }
    }

    return data_size;
}

void SseClient::openQueryResponseSocket() {
    struct sockaddr_in serv_addr;
    querySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (querySocket < 0)
        pee("SseClient::getQueryResponseSocket ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(clientPort);
    if (bind(querySocket, (const struct sockaddr *) &serv_addr,(socklen_t)sizeof(serv_addr)) < 0)
        pee("SseClient::getQueryResponseSocket ERROR on binding");
    listen(querySocket,5);
}

int SseClient::acceptQueryResponseSocket() {
    int newsockfd = -1;
    while (newsockfd < 0) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        newsockfd = accept(querySocket, (struct sockaddr *) &cli_addr, &clilen);
    }
    return newsockfd;
}

//void SseClient::addDocs(string textDataset) {
//    vector<string> tags;
//    listTxtFiles(textDataset, tags);
//    vector<string>::iterator tags_it=tags.begin();
//
//    while (tags_it != tags.end()) {     //for each txt file in the dataset
//        //extract text features (keywords)
//        set<string> keywords = analyzer->extractUniqueKeywords(tags_it->c_str());
//
//        //get and inc counters
//        for (set<string>::iterator it=keywords.begin(); it!=keywords.end(); ++it) {
//            int c = 0;
//            map<string,int>::iterator counterIt = W->find(*it);
//            if (counterIt != textDcount->end())
//                c = counterIt->second;
//            encryptAndIndex((void*)it->first.c_str(), (int)it->first.size(), c, it->second, &encTextIndex);
//            (*textDcount)[it->first] = ++c;
//        }
//        for (int i = 0; i < numCPU; i++)
//            if (pthread_join (encThreads[i], NULL)) pee("Error:unable to join thread");
//        cryptoTime += diffSec(start, getTime()); //end benchmark
//
//        ++tags_it;
//
//
//
//    }
//}

void SseClient::listTxtFiles (std::string path, std::vector<std::string>& docs) {
    DIR* dir = opendir (path.c_str());
    if (dir) {
        struct dirent* hFile;
        while ((hFile = readdir (dir)) != NULL ) {
            if ( !strcmp( hFile->d_name, "."  ) || !strcmp( hFile->d_name, ".." ) || hFile->d_name[0] == '.' ) continue;
            std::string fname = hFile->d_name;
            const size_t pos = fname.find(".txt");
            if (pos != std::string::npos) {
//                std::string idString = fname.substr(4,pos-4);
//                const int id = atoi(idString.c_str());
                std::string fullPath = path;
                path += fname;
                docs.push_back(fname);
            }
        }
        closedir (dir);
    } else
        pee ("SseClient::listTxtFiles couldn't open dataset dir.");
}

string SseClient::get_random_segment(vector<string> segments) {
    return segments[client_c_random_uint_range(0, segments.size())];
}

string SseClient::generate_random_query(vector<string> all_words, const int size, const int not_prob, const int and_prob) {
    const int lone_word_prob = 50;
    const int par_prob = 50;

    // generate small segments
    queue<string> segments;
    for(int i = 0; i < size; i++) {
        int lone_word_rand = client_c_random_uint_range(0, 100);

        if(lone_word_rand < lone_word_prob) {
            string word = get_random_segment(all_words);

            int not_rand = client_c_random_uint_range(0, 100);
            if(not_rand < not_prob)
                segments.push("!" + word);
            else
                segments.push(word);
        } else {
            // AND or OR
            string word1 = get_random_segment(all_words);
            string word2 = get_random_segment(all_words);

            // choose operator
            int op_prob = client_c_random_uint_range(0, 100);
            string op = op_prob < and_prob ? " && " : " || ";

            // form either a simple, parenthesis or negated segment
            int par_prob_rand = client_c_random_uint_range(0, 100);
            if(par_prob_rand < par_prob) {
                int not_rand = client_c_random_uint_range(0, 100);
                if(not_rand < not_prob)
                    segments.push("!(" + word1 + op + word2 + ")");
                else
                    segments.push("(" + word1 + op + word2 + ")");
            } else {
                segments.push(word1 + op + word2);
            }
        }
    }

    while(segments.size() > 1) {
        // pop two segments from the queue
        string seg1 = segments.front();
        segments.pop();
        string seg2 = segments.front();
        segments.pop();

        // choose operator
        int op_prob = client_c_random_uint_range(0, 100);
        string op = op_prob < and_prob  ? " && " : " || ";

        // form either a simple, parenthesis or negated segment
        int par_prob_rand = client_c_random_uint_range(0, 100);
        if(par_prob_rand < par_prob) {
            int not_rand = client_c_random_uint_range(0, 100);
            if(not_rand < not_prob)
                segments.push("!(" + seg1 + op + seg2 + ")");
            else
                segments.push("(" + seg1 + op + seg2 + ")");
        } else {
            segments.push(seg1 + op + seg2);
        }
    }

    return segments.front();
}
