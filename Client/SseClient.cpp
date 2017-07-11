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
    //setup operation does initializations
}

SseClient::~SseClient() {
    delete analyzer;
    delete crypto;
    delete W;
    close(querySocket);
}

void SseClient::setup() {
    //init data structures
    openQueryResponseSocket();
    analyzer = new EnglishAnalyzer;
    crypto = new ClientCrypt;   //inits kCom
    W = new map<string,int>;    /**TODO persist W*/
    nDocs = 0;

    //get encrypted kCom and init buffers
    vector<unsigned char> kCom = crypto->getEncryptedKcom();
    const int data_size = (int)kCom.size();
    char* data = new char[data_size];
    int pos = 0;

    //prepare data buffers
    for (int i = 0; i < kCom.size(); i++)
        addToArr(&kCom[i], sizeof(unsigned char), data, &pos);

    //send data
    char buff[sizeof(int)];
    pos = 0;
    addIntToArr(data_size, buff, &pos);
    int sockfd = connectAndSend(buff, sizeof(int));
    socketSend(sockfd, data, data_size);

    delete[] data;
    close(sockfd);
}

int SseClient::newDoc() {
    return nDocs++;
}

void SseClient::addDocument(string fname) {
    int id = newDoc();

    set<string> text = analyzer->extractUniqueKeywords(fname);
    addWords(id, text);

    printf("Finished add document %d\n", id);
}

void SseClient::addWords(int d, set<string> words) {
    int data_size = sizeof(char);

    // first iteration: to get the size of the buffer to allocate
    for(string w : words) {
        data_size += 2*sizeof(int) + (int)w.size() + 1;
    }

    //allocate data buffer
    unsigned char* data = new unsigned char[data_size];

    char op = 'a';
    int pos = 0;
    addToArr(&op, sizeof(char), (char*)data, &pos);

    // second iteration: to fill the buffer
    for(string w : words) {
        //get counter c for w
        int c = 1; // with new words, this is the first instance of it
        map<string,int>::iterator it = W->find(w);
        if (it != W->end())
            c = it->second + 1;
            
        // update counter c
        (*W)[w] = c;

        addIntToArr(d, (char*)data, &pos);
        addIntToArr(c - 1, (char*)data, &pos); // counter starts at 1, so -1 for indexing
        for (int i = 0; i < w.size(); i++)
            addToArr(&w[i], sizeof(char), (char*)data, &pos);
        
        char term = '\0';
        addToArr(&term, sizeof(char), (char*)data, &pos);
    }

    //encrypt data
    int ciphertext_size = data_size+16;
    unsigned char* ciphertext = new unsigned char[ciphertext_size];
    ciphertext_size = crypto->encryptSymmetric(data, data_size, ciphertext);
    delete[] data;

    //send data
    char buff[sizeof(int)];
    pos = 0;
    addIntToArr(ciphertext_size, buff, &pos);
    int sockfd = connectAndSend(buff, sizeof(int));
    socketSend(sockfd, (char*)ciphertext, ciphertext_size);

    close(sockfd);
    delete[] ciphertext;
}

//boolean operands: AND, OR, NOT, (, )
vector<int> SseClient::search(string query) {
    // parse the query into token structs and apply the shunting yard algorithm
    vector<token> infix_query = tokenize(query);
    vector<token> rpn = shunting_yard(infix_query);
    
    int data_size = sizeof(char); // char from op

    // first query iteration: to get needed size and counters
    for(int i = 0; i < rpn.size(); i++) {
        token *tkn = &rpn[i];

        if(tkn->type == WORD_TOKEN) {
            map<string,int>::iterator counterIt = W->find(tkn->word);
            if(counterIt != W->end())
                tkn->counter = counterIt->second;
            else
                tkn->counter = 0;

            //printf("counter %s %d\n", tkn->word.c_str(), tkn->counter);
            data_size += sizeof(char) + sizeof(int) + (tkn->word.size() + 1);
        } else {
            data_size += sizeof(char);
        }
    }

    // add number of documents to the data structure, needed for NOT
    token t;
    t.type = META_TOKEN;
    t.counter = nDocs;

    rpn.push_back(t);
    data_size += sizeof(char) + sizeof(int);

    //prepare query
    unsigned char* data = new unsigned char[data_size];
    int pos = 0;

    char op = 's';
    addToArr(&op, sizeof(char), (char*)data, &pos);

    // second query iteration: to fill "data" buffer
    for(vector<token>::iterator it = rpn.begin(); it != rpn.end(); ++it) {
        token tkn = *it;

        addToArr(&(tkn.type), sizeof(char), (char*)data, &pos);

        if(tkn.type == WORD_TOKEN) {
            addIntToArr(tkn.counter, (char*)data, &pos);

            string word = tkn.word;
            for (int i = 0; i < word.size(); i++)
                addToArr(&word[i], sizeof(char), (char*)data, &pos);

            char term = '\0';
            addToArr(&term, sizeof(char), (char*)data, &pos);
        } else if(tkn.type == META_TOKEN) {
            addIntToArr(tkn.counter, (char*)data, &pos);
        }
    }

    //encrypt query
    int ciphertext_size = data_size + 16;
    unsigned char* ciphertext = new unsigned char[ciphertext_size];
    ciphertext_size = crypto->encryptSymmetric(data, data_size, ciphertext);
    delete[] data;

    //send query
    char buff[sizeof(int)];
    pos = 0;
    addIntToArr(ciphertext_size, buff, &pos);

    int sockfd = connectAndSend(buff, sizeof(int));
    socketSend(sockfd, (char*)ciphertext, ciphertext_size);
    delete[] ciphertext;
    close(sockfd);

    //open socket and receive results
    /**TODO Cliente abrir socket não é muito bom; Implementar solução melhor*/
    int response_sockfd = acceptQueryResponseSocket();
    bzero(buff, sizeof(int));
    socketReceive(response_sockfd, buff, sizeof(int));
    pos = 0;

    const int enc_result_size = readIntFromArr(buff, &pos);
    unsigned char* enc_result_buff = new unsigned char[enc_result_size];
    socketReceive(response_sockfd, (char*) enc_result_buff, enc_result_size);
    close(response_sockfd);

    //decrypt results
    unsigned char* result_buff = new unsigned char[enc_result_size];
    const int result_size = crypto->decryptSymmetric(enc_result_buff, enc_result_size, result_buff);
    delete[] enc_result_buff;

    //process results
    const int nDocs = result_size / sizeof(int);
    vector<int> results(nDocs);
    pos = 0;
    for (int i = 0; i < nDocs; i++) {
        results[i] = readIntFromArr((char*)result_buff, &pos);
    }

    delete[] result_buff;
    return results;
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
