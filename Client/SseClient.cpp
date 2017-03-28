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
    encTextIndex = new map<vector<unsigned char>,vector<unsigned char> >;
    analyzer = new EnglishAnalyzer;
    crypto = new SseCrypt;
}

SseClient::~SseClient() {
    free(encTextIndex);
}

void SseClient::addDocs(string textDataset, int first, int last, int prefix) {
    vector<string> tags;
    listTxtFiles(textDataset, tags);
    vector<string>::iterator tags_it=tags.begin();
    //for each txt file in the dataset
    while (tags_it != tags.end()) {
        //extract text features (keywords)
        vector<string> keywords = analyzer->extractFile(tags_it->c_str());
        map<string,int> textTfs;
        for (int j = 0; j < keywords.size(); j++) {
            string keyword = keywords[j];
            map<string,int>::iterator it = textTfs.find(keyword);
            //counts occurence frequency of keyword in document; only used for ranked searching
            if (it == textTfs.end())
                textTfs[keyword] = 1;
            else
                it->second++;
        }
        //index text features (keywords)
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

void SseClient::encryptAndIndex(void* keyword, int keywordSize, int counter, int docId,
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
        pee ("SseSimple::listTxtFiles couldn't open dir");
}



}


