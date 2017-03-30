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
#include "IeeUtils.h"

using namespace std;

class IeeCrypt {
    
private:
    static const int symKsize = 16;
    static const int fKsize = 20;
    
    RSA* IeePrivK;
    unsigned char* kCom;
    unsigned char* kEnc;
    unsigned char* kW;
    unsigned char* kI;
    
    void spc_rand(unsigned char *buf, int l);
    
public:
    IeeCrypt();
    ~IeeCrypt();
    void storeKcom(vector<unsigned char> key);
    bool hasStoredKcom();
    vector<unsigned char> decryptPublic (unsigned char* data, int size);
    void initKeys();
};



#endif /* IeeCrypt_hpp */
