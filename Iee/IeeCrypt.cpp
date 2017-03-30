//
//  IeeCrypt.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 29/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#include "IeeCrypt.hpp"

using namespace std;

IeeCrypt::IeeCrypt() {
    //read IEE private key from disk
    char keyPath[256];
    strcpy(keyPath, homePath);
    strcpy(keyPath+strlen(keyPath), "Data/Client/BooleanSSE/IeePriv.pem");
    FILE* f = fopen(keyPath, "rb");
    IeePrivK = PEM_read_RSAPrivateKey(f, NULL, NULL, NULL);
    fclose(f);
}

IeeCrypt::~IeeCrypt() {
    delete IeePrivK;
    delete[] kCom;
    delete[] kEnc;
    delete[] kW;
    delete[] kI;
}

void IeeCrypt::storeKcom(vector<unsigned char> key) {
    kCom = new unsigned char[key.size()];
    for (int i = 0; i < key.size(); i++)
        kCom[i] = key[i];
}

bool IeeCrypt::hasStoredKcom() {
    return kCom != NULL;
}

vector<unsigned char> IeeCrypt::decryptPublic (unsigned char* data, int size) {
    unsigned char* decrypt = new unsigned char[size];
    int decSize = RSA_private_decrypt(size, data, decrypt, IeePrivK, RSA_PKCS1_OAEP_PADDING);
    if (decSize == -1)
        pee("IeeCrypt::decryptPublic - couldn't decrypt.");
    vector<unsigned char> result;
    result.resize(decSize);
    for (int i = 0; i < decSize; i++)
        result[i] = decrypt[i];
    return result;
}

void IeeCrypt::initKeys() {
    kEnc = new unsigned char[symKsize];
    spc_rand(kEnc, symKsize);
    
    kW = new unsigned char[fKsize];
    spc_rand(kW, fKsize);
    
    kI = new unsigned char[fKsize];
    spc_rand(kI, fKsize);
}

void IeeCrypt::spc_rand(unsigned char *buf, int l) {
    if (!RAND_bytes(buf, l))
        pee("The PRNG is not seeded!\n");
}
