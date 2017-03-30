//
//  CashCrypt.hpp
//  MIE
//
//  Created by Bernardo Ferreira on 01/10/15.
//  Copyright © 2015 NovaSYS. All rights reserved.
//

#ifndef ClientCrypt_hpp
#define ClientCrypt_hpp

#include <stdint.h>
#include <string>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include "ClientUtils.h"



using namespace std;

class ClientCrypt {
    
    RSA* IeePubK;
    unsigned char* Kcom;
    
    unsigned char* spc_rand(unsigned char *buf, int l);
    unsigned int spc_rand_uint();
    float spc_rand_real(void);
    float spc_rand_real_range(float min, float max);
    
public:
    
    static const unsigned int pubKsize = 128; //128 bits
    static const unsigned int symKsize = 16; //128 bits
    
    ClientCrypt();
    ~ClientCrypt();
    
    vector<unsigned char> encryptPublic (unsigned char* data, int size);
    vector<unsigned char> encryptSymmetric (unsigned char* data, int size);
    int decryptSymmetric (unsigned char* ciphertext, int ciphertextSize, unsigned char* plaintext);
    vector<unsigned char> getEncryptedKcom();
    
};


#endif /* ClientCrypt_hpp */
