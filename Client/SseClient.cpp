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
    delete[] W;
}

void SseClient::setup() {
    analyzer = new EnglishAnalyzer;
    crypto = new ClientCrypt;   //inits kCom
    W = new map<string,int>;
    
    vector<unsigned char> kCom = crypto->getEncryptedKcom();
    char* buff = new char[kCom.size()];
    for (int i = 0; i < kCom.size(); i++)
        buff[i] = kCom[i];
    
    char buff2[sizeof(int)];
    int pos = 0;
    addIntToArr((int)kCom.size(), buff2, &pos);
    int sockfd = connectAndSend(buff2, sizeof(int));
    
    socketSend (sockfd, buff, kCom.size());
    delete[] buff;
    close(sockfd);
    
    nDocs = 0;
}

void SseClient::add(int d, string w) {
    int c = 0;
    map<string,int>::iterator it = W->find(w);
    if (it != W->end())
        c = it->second + 1;
    
    const int data_size = 2*sizeof(int) + (int)w.size();
    unsigned char* data = new unsigned char[data_size];
    int pos = 0;
    addToArr(&d, sizeof(int), (char*)data, &pos);
    addToArr(&c, sizeof(int), (char*)data, &pos);
    for (int i = 0; i < w.size(); i++)
        addToArr(&w[i], sizeof(char), (char*)data, &pos);
    
    int ciphertext_size = data_size+16;
    unsigned char* ciphertext = new unsigned char[ciphertext_size];
    ciphertext_size = crypto->encryptSymmetric(data, data_size, ciphertext);
    delete[] data;
    
    char buff[sizeof(int)];
    pos = 0;
    addIntToArr(ciphertext_size, buff, &pos);
    int sockfd = connectAndSend(buff, sizeof(int));
    
    socketSend(sockfd, (char*)ciphertext, ciphertext_size);
    close(sockfd);
    delete[] ciphertext;
    
    (*W)[w] = c;
}

void SseClient::addDocs(string textDataset) {
    vector<string> tags;
    listTxtFiles(textDataset, tags);
    vector<string>::iterator tags_it=tags.begin();
    
    while (tags_it != tags.end()) {     //for each txt file in the dataset
        //extract text features (keywords)
        set<string> keywords = analyzer->extractUniqueKeywords(tags_it->c_str());
        
        //get and inc counters
        for (set<string>::iterator it=keywords.begin(); it!=keywords.end(); ++it) {
            int c = 0;
            map<string,int>::iterator counterIt = W->find(*it);
            if (counterIt != textDcount->end())
                c = counterIt->second;
            encryptAndIndex((void*)it->first.c_str(), (int)it->first.size(), c, it->second, &encTextIndex);
            (*textDcount)[it->first] = ++c;
        }
        for (int i = 0; i < numCPU; i++)
            if (pthread_join (encThreads[i], NULL)) pee("Error:unable to join thread");
        cryptoTime += diffSec(start, getTime()); //end benchmark
        
        ++tags_it;

        

    }
}

//void SseClient::encryptAndIndex(void* keyword, int keywordSize, int counter, int docId,
//                                 map<vector<unsigned char>, vector<unsigned char> >* index) {
//    unsigned char k1[SseCrypt::Ksize];
//    int append = 1;
//    crypto->deriveKey(&append, sizeof(int), keyword, keywordSize, k1);
//    unsigned char k2[SseCrypt::Ksize];
//    append = 2;
//    crypto->deriveKey(&append, sizeof(int), keyword, keywordSize, k2);
//    vector<unsigned char> encCounter = crypto->encCounter(k1, (unsigned char*)&counter, sizeof(int));
//    unsigned char idAndScore[2*sizeof(int)];
//    int pos = 0;
//    addIntToArr(docId, (char*)idAndScore, &pos);
////    addIntToArr(tf, (char*)idAndScore, &pos);
//    vector<unsigned char> encData = crypto->encData(k2, idAndScore, pos);
//    pthread_mutex_lock (lock);
//    if (index->find(encCounter) != index->end())
//        pee("Found an unexpected collision in CashClient::encryptAndIndex");
//    (*index)[encCounter] = encData;
//    pthread_mutex_unlock (lock);
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



vector<int> SseClient::search(string query) {
    //process boolean formula of query; operands: AND, OR, NOT, (, ).
    
    
    
    vector<int> results;
    return results;
}


