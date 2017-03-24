//
//  SSE_simple.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 20/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#include "SseSimple.hpp"

using namespace std;

SseSimple::SseSimple() {
    encTextIndex = new map<vector<unsigned char>,vector<unsigned char> >;
    analyzer = new EnglishAnalyzer;
    crypto = new SseCrypt;
}

SseSimple::~SseSimple() {
    free(encTextIndex);
}

void SseSimple::addDocs(string textDataset, int first, int last, int prefix) {
    map<int,string> tags;
    extractFileNames(textDataset, first, last, tags);
    map<int,string>::iterator tags_it=tags.begin();
    while (tags_it != tags.end()) {
        //extract text features
        vector<string> keywords = analyzer->extractFile(tags_it->second.c_str());
        map<string,int> textTfs;
        for (int j = 0; j < keywords.size(); j++) {
            string keyword = keywords[j];
            map<string,int>::iterator it = textTfs.find(keyword);
            if (it == textTfs.end())
                textTfs[keyword] = 1;
            else
                it->second++;
        }
        //index text features
        for (map<string,int>::iterator it=textTfs.begin(); it!=textTfs.end(); ++it) {
            int c = 0;
            map<string,int>::iterator counterIt = textDcount->find(it->first);
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

void SseSimple::encryptAndIndex(void* keyword, int keywordSize, int counter, int docId,
                                 map<vector<unsigned char>, vector<unsigned char> >* index) {
    unsigned char k1[SseCrypt::Ksize];
    int append = 1;
    crypto->deriveKey(&append, sizeof(int), keyword, keywordSize, k1);
    unsigned char k2[SseCrypt::Ksize];
    append = 2;
    crypto->deriveKey(&append, sizeof(int), keyword, keywordSize, k2);
    vector<unsigned char> encCounter = crypto->encCounter(k1, (unsigned char*)&counter, sizeof(int));
    unsigned char idAndScore[2*sizeof(int)];
    int pos = 0;
    addIntToArr(docId, (char*)idAndScore, &pos);
//    addIntToArr(tf, (char*)idAndScore, &pos);
    vector<unsigned char> encData = crypto->encData(k2, idAndScore, pos);
    pthread_mutex_lock (lock);
    if (index->find(encCounter) != index->end())
        pee("Found an unexpected collision in CashClient::encryptAndIndex");
    (*index)[encCounter] = encData;
    pthread_mutex_unlock (lock);
}
