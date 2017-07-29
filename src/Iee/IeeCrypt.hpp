//
//  IeeCrypt.hpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 29/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef IeeCrypt_hpp
#define IeeCrypt_hpp

#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include "IeeUtils.h"  //TODO TEMP_DEBUG_CRYPT
#include <string.h>  //TODO TEMP_DEBUG_CRYPT

static const char* ieePrivFile = getenv("IEE_PRIV_FILE") ? getenv("IEE_PRIV_FILE") : "Data/Client/BooleanSSE/IeePriv.pem"; //TODO TEMP_DEBUG_CRYPT

using namespace std;

unsigned int spc_rand_uint_range(int min, int max);
unsigned int spc_rand_uint();
unsigned char* spc_rand(unsigned char *buf, int l);

class IeeCrypt {
private:
    RSA* IeePrivK;
    //unsigned char* kCom;

    // keys sent by the client
    unsigned char* kEnc;
    unsigned char* kF;

public:
    static const int symBlocksize = 16;
    static const int fBlocksize = 20;

    IeeCrypt();
    ~IeeCrypt();

    int decryptPublic (unsigned char* plaintext, unsigned char* ciphertext, int ciphertextSize);
    int encryptSymmetric (unsigned char* data, int size, unsigned char* ciphertext, unsigned char* key);
    int decryptSymmetric (unsigned char* plaintext, unsigned char* ciphertext, int ciphertextSize, unsigned char* key);
    void f (unsigned char* key, unsigned char* data, int dataSize, unsigned char* md);

    //bool hasStoredKcom();

    void setKeys(unsigned char* kEnc, unsigned char* kF);

    //unsigned char* get_kCom();
    unsigned char* get_kF();
    unsigned char* get_kEnc();
};

#endif /* IeeCrypt_hpp */
