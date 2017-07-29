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
    //TODO descomentado para debug antes do SGX TEMP_DEBUG_CRYPT
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
    //delete[] kCom;

    delete[] kEnc;
    delete[] kF;
}
/*
bool IeeCrypt::hasStoredKcom() {
    return kCom != NULL;
}*/

//#if 0
//TEMP_DEBUG_CRYPT
int IeeCrypt::decryptPublic (unsigned char* plaintext, unsigned char* ciphertext, int ciphertextSize) {
    //unsigned char* decrypt = new unsigned char[size];
    int decSize = RSA_private_decrypt(ciphertextSize, ciphertext, plaintext, IeePrivK, RSA_PKCS1_OAEP_PADDING);
    if (decSize == -1)
        pee("IeeCrypt::decryptPublic - couldn't decrypt.");
        
    return decSize;
   /* vector<unsigned char> result;
    result.resize(decSize);
    for (int i = 0; i < decSize; i++)
        result[i] = plaintext[i];
    //delete[] decrypt;
    return result;*/
}
//#endif

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
    
  return 0;
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
    return 0;    
}

void IeeCrypt::f(unsigned char* key, unsigned char* data, int dataSize, unsigned char* md) {
    unsigned int mdSize;
    HMAC(EVP_sha1(), key, fBlocksize, data, dataSize, md, &mdSize);
    if (mdSize != fBlocksize)
        pee("CashCrypt::hmac - md size of different from expected\n");
}

unsigned char* spc_rand(unsigned char *buf, int l) {
    //TODO was commented TEMP_DEBUG_CRYPT
    if (!RAND_bytes(buf, l)) {
        fprintf(stderr, "The PRNG is not seeded!\n");
        abort(  );
    }
    
    return buf;
}

unsigned int spc_rand_uint() {
    unsigned int res = -1;
    spc_rand((unsigned char *)&res, sizeof(unsigned int));
    
    return res;
}

unsigned int spc_rand_uint_range(int min, int max) {
//    if (max < min) abort();
    if(max < min)
        return max + (spc_rand_uint() % static_cast<int>(min - max));

    return min + (spc_rand_uint() % static_cast<int>(max - min));
}

void IeeCrypt::setKeys(unsigned char* kEnc, unsigned char* kF) {
    //this->kCom = kCom;
    this->kEnc = kEnc;
    this->kF = kF;
}
/*
unsigned char* IeeCrypt::get_kCom() {
    return kCom;
}*/

unsigned char* IeeCrypt::get_kF() {
    return kF;
}

unsigned char* IeeCrypt::get_kEnc() {
    return kEnc;
}
