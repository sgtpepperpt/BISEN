//
//  CashCrypt.cpp
//  MIE
//
//  Created by Bernardo Ferreira on 01/10/15.
//  Copyright © 2015 NovaSYS. All rights reserved.
//


#include "ClientCrypt.hpp"


using namespace std;

ClientCrypt::ClientCrypt() {
    //read IEE public key from disk
    string keyFilename = homePath;
    FILE* f = fopen((keyFilename+"Data/Client/BooleanSSE/IeePub.pem").c_str(), "rb");
    IeePubK = PEM_read_RSA_PUBKEY(f, NULL, NULL, NULL);
    fclose(f);
    
    //read symmetric key from disk or generate and persist
    keyFilename = homePath;
    f = fopen((keyFilename+"Data/Client/BooleanSSE/Kcom").c_str(), "rb");
    Kcom = new unsigned char[symKsize]; //Kcom = (unsigned char*)malloc(symKsize);
    if (f != NULL)
        fread (Kcom, 1, symKsize, f);
    else {
        spc_rand(Kcom, symKsize);
        f = fopen((keyFilename+"Data/Client/BooleanSSE/Kcom").c_str(), "wb");
        fwrite(Kcom, 1, symKsize, f);
    }
    fclose(f);
}


ClientCrypt::~ClientCrypt() {
    delete[] Kcom;
    delete IeePubK;
}


vector<unsigned char> ClientCrypt::getEncryptedKcom() {
    return encryptPublic(Kcom, symKsize);
}


vector<unsigned char> ClientCrypt::encryptPublic (unsigned char* data, int size) {
    unsigned char* encrypt = (unsigned char*)malloc(pubKsize);
    if (encrypt == NULL) pee("ClientCrypt::encryptPublic - malloc error in encrypt.");
    int encrypt_len;
    if((encrypt_len = RSA_public_encrypt(size, data, encrypt, IeePubK, RSA_PKCS1_OAEP_PADDING)) == -1)
        pee("CashCrypt::encryptPublic - could not encrypt data.");
        
    vector<unsigned char> v;
    v.resize(encrypt_len);
    for (int i = 0; i < encrypt_len; i++)
        v[i] = encrypt[i];
    free(encrypt);
    return v;
}

vector<unsigned char> ClientCrypt::encryptSymmetric (unsigned char* data, int size) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    unsigned char iv[16] = {0};
    unsigned char* ciphertext = (unsigned char*)malloc(size+16);
    
    if(!(ctx = EVP_CIPHER_CTX_new()))
        pee("CashCrypt::encrypt - could not create ctx\n");
    
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, Kcom, iv)) 
        pee("CashCrypt::encrypt - could not init encrypt\n");
    
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, data, size))
        pee("CashCrypt::encrypt - could not encrypt update\n");
    ciphertext_len = len;
    
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        pee("CashCrypt::encrypt - could not encrypt final\n");
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    vector<unsigned char> v;
    v.resize(ciphertext_len);
    for (int i = 0; i < ciphertext_len; i++)
        v[i] = ciphertext[i];
    free(ciphertext);
    return v;
}


int ClientCrypt::decryptSymmetric (unsigned char* ciphertext, int ciphertextSize, unsigned char* plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    unsigned char iv[16] = {0};
    
    if (!(ctx = EVP_CIPHER_CTX_new()))
        pee("ServerUtil::decrypt - could not create ctx\n");
    
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, Kcom, iv)) //key will have 20 bytes but aes128 will only use the first 16
        pee("ServerUtil::decrypt - could not init decrypt\n");
    
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertextSize))
        pee("ServerUtil::decrypt - could not decrypt update\n");
    plaintext_len = len;
    
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        pee("ServerUtil::decrypt - could not decrypt final\n");
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
 
//     vector<unsigned char> v;
//     v.resize(plaintext_len);
//     for (int i = 0; i < plaintext_len; i++)
//     v[i] = plaintext[i];
//     free(plaintext);
//     return v;
 
    return plaintext_len;
}

unsigned char* ClientCrypt::spc_rand(unsigned char *buf, int l) {
    if (!RAND_bytes(buf, l)) {
        fprintf(stderr, "The PRNG is not seeded!\n");
        abort(  );
    }
    return buf;
}

unsigned int ClientCrypt::spc_rand_uint() {
    unsigned int res;
    spc_rand((unsigned char *)&res, sizeof(unsigned int));
    return res;
}

float ClientCrypt::spc_rand_real(void) {
    return ((float)spc_rand_uint()) / (float)UINT_MAX;
}

float ClientCrypt::spc_rand_real_range(float min, float max) {
    if (max < min) abort();
    return spc_rand_real() * (max - min) + min;
}
