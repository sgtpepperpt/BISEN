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
    strcpy(keyPath+strlen(keyPath), ieePrivFile);
    FILE* f = fopen(keyPath, "rb");
    IeePrivK = PEM_read_RSAPrivateKey(f, NULL, NULL, NULL);
    fclose(f);
}

IeeCrypt::~IeeCrypt() {
    delete IeePrivK;
    delete[] kCom;
    delete[] kEnc;
    delete[] kF;
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
    delete[] decrypt;
    return result;
}

int IeeCrypt::encryptSymmetric (unsigned char* data, int size, unsigned char* ciphertext, unsigned char* key) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    unsigned char iv[16] = {0};
//    unsigned char* ciphertext = new char[size+16];
    
    if(!(ctx = EVP_CIPHER_CTX_new()))
        pee("CashCrypt::encrypt - could not create ctx\n");
    
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
        pee("CashCrypt::encrypt - could not init encrypt\n");
    
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, data, size))
        pee("CashCrypt::encrypt - could not encrypt update\n");
    ciphertext_len = len;
    
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        pee("CashCrypt::encrypt - could not encrypt final\n");
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
    
//    vector<unsigned char> v;
//    v.resize(ciphertext_len);
//    for (int i = 0; i < ciphertext_len; i++)
//        v[i] = ciphertext[i];
//    delete[] ciphertext;
//    return v;
}

int IeeCrypt::decryptSymmetric (unsigned char* plaintext, unsigned char* ciphertext, int ciphertextSize, unsigned char* key) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
//    unsigned char* plaintext = new unsigned char[ciphertextSize];
    unsigned char iv[symBlocksize] = {0}; //for testing; should be replaced with random iv
    
    if (!(ctx = EVP_CIPHER_CTX_new()))
        pee("IeeCrypt::decryptSymmetric - could not create ctx\n");
    
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
        pee("IeeCrypt::decryptSymmetric - could not init decrypt\n");
    
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertextSize))
        pee("IeeCrypt::decryptSymmetric - could not decrypt update\n");
    plaintext_len = len;
    
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        pee("ServerUtil::decrypt - could not decrypt final\n");
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    
    return plaintext_len;
    
//    vector<unsigned char> v;
//    v.resize(plaintext_len);
//    for (int i = 0; i < plaintext_len; i++)
//        v[i] = plaintext[i];
//    delete[] plaintext;
//    return v;
    
}

void IeeCrypt::f(unsigned char* key, unsigned char* data, int dataSize, unsigned char* md) {
    unsigned int mdSize;
    HMAC(EVP_sha1(), key, fBlocksize, data, dataSize, md, &mdSize);
    if (mdSize != fBlocksize)
        pee("CashCrypt::hmac - md size of different from expected\n");
}

void IeeCrypt::initKeys() {
    kEnc = new unsigned char[symBlocksize];
    spc_rand(kEnc, symBlocksize);
    
    kF = new unsigned char[fBlocksize];
    spc_rand(kF, fBlocksize);
}

void IeeCrypt::spc_rand(unsigned char *buf, int l) {
    if (!RAND_bytes(buf, l))
        pee("The PRNG is not seeded!\n");
}

unsigned char* IeeCrypt::get_kF() {
    return kF;
}

unsigned char* IeeCrypt::get_kCom() {
    return kCom;
}

unsigned char* IeeCrypt::get_kEnc() {
    return kEnc;
}

unsigned int IeeCrypt::spc_rand_uint() {
    unsigned int res;
    spc_rand((unsigned char *)&res, sizeof(unsigned int));
    return res;
}

unsigned int IeeCrypt::spc_rand_uint_range(int min, int max) {
    if (max < min) abort();
    return spc_rand_uint() * (max - min) + min;
}
