//
//  IeeCrypt.c
//  BooleanSSE
//
//  Created by Guilherme Borges
//  Copyright © 2017 Guilherme Borges. All rights reserved.
//

#include "IeeCrypt.h"

void setKeys(unsigned char* kEnc, unsigned char* kF) {
    kEnc = kEnc;
    kF = kF;
}

void destroyKeys() {
    free(kEnc);
    free(kF);
}

unsigned char* get_kF() {
    return kF;
}

unsigned char* get_kEnc() {
    return kEnc;
}

void c_random(unsigned char *buf, unsigned long long len)
{
    randombytes_buf(buf, sizeof(unsigned char)*len);
}

unsigned int c_random_uint() {
    return randombytes_random();
}

unsigned int c_random_uint_range(int min, int max) {
    if(max < min)
        return max + (c_random_uint() % (int)(min - max));

    return min + (c_random_uint() % (int)(max - min));
}

// -1 failure, 0 ok
int c_encrypt(
  unsigned char *ciphertext, // message_len + C_EXPBYTES
  const unsigned char *message, // message_len
  unsigned long long message_len,
  const unsigned char *nonce, // C_NONCESIZE
  const unsigned char *key // C_KEYSIZE
)
{
  return crypto_secretbox_easy(ciphertext+crypto_secretbox_NONCEBYTES,
                               message, message_len,
                               nonce, key);
}

// -1 failure, 0 ok
int c_decrypt(
  unsigned char *decrypted, // ciphertext_len - C_EXPBYTES
  const unsigned char *ciphertext, // ciphertext_len
  unsigned long long ciphertext_len,
  const unsigned char *nonce, // C_NONCESIZE
  const unsigned char *key // C_KEYSIZE
)
{
  return crypto_secretbox_open_easy(decrypted, ciphertext+crypto_secretbox_NONCEBYTES,
                                    ciphertext_len-crypto_secretbox_NONCEBYTES,
                                    nonce, key);
}

// -1 failure, 0 ok
int c_hmac(
  unsigned char *out, // H_BYTES
  const unsigned char *in, // inlen
  unsigned long long inlen,
  const unsigned char *k // H_KEYSIZE
)
{
  return crypto_auth_hmacsha256(out, in, inlen, k);
}

// -1 failure, 0 ok
int c_hmac_verify(const unsigned char *h, const unsigned char *in, unsigned long long inlen, const unsigned char *k)
{
  return crypto_auth_hmacsha256_verify(h, in, inlen, k);
}
