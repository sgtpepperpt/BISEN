//
//  IeeCrypt.hpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 29/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef IeeCrypt_hpp
#define IeeCrypt_hpp

#include <stdio.h>
#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include "IeeUtils.h"

static const char* ieePrivFile = getenv("IEE_PRIV_FILE") ? getenv("IEE_PRIV_FILE") : "Data/Client/BooleanSSE/IeePriv.pem";

using namespace std;

class IeeCrypt {
    
private:
    RSA* IeePrivK;
    unsigned char* kCom;
    unsigned char* kEnc;
    unsigned char* kF;
    
    void spc_rand(unsigned char *buf, int l);
    
public:
    static const int symBlocksize = 16;
    static const int fBlocksize = 20;
    
    IeeCrypt();
    ~IeeCrypt();
    void initKeys();
    void storeKcom(vector<unsigned char> key);
    bool hasStoredKcom();
    vector<unsigned char> decryptPublic (unsigned char* data, int size);
    int encryptSymmetric (unsigned char* data, int size, unsigned char* ciphertext, unsigned char* key);
    int decryptSymmetric (unsigned char* plaintext, unsigned char* ciphertext, int ciphertextSize, unsigned char* key);
    void f (unsigned char* key, unsigned char* data, int dataSize, unsigned char* md);
    unsigned char* get_kF();
    unsigned char* get_kCom();
    unsigned char* get_kEnc();
};



#endif /* IeeCrypt_hpp */
