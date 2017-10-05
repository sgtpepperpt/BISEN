//
//  IeeCrypt.h
//  BooleanSSE
//
//  Created by Guilherme Borges.
//  Copyright © 2017 Guilherme Borges. All rights reserved.
//

#ifndef ClientCrypt_h
#define ClientCrypt_h

#include <sodium.h>

#define C_KEYSIZE   crypto_secretbox_KEYBYTES
#define C_NONCESIZE crypto_secretbox_NONCEBYTES
#define C_EXPBYTES (crypto_secretbox_NONCEBYTES+crypto_secretbox_MACBYTES)

#define H_KEYSIZE crypto_auth_hmacsha256_KEYBYTES
#define H_BYTES crypto_auth_hmacsha256_BYTES

const int symBlocksize = 16;
const int fBlocksize = 32;

// keys sent by the client
static unsigned char* kEnc;
static unsigned char* kF;

void init_crypt();
void destroy_crypt();

unsigned char* get_kF();
unsigned char* get_kEnc();

// RANDOM FUNCTIONS
void c_random(unsigned char *buf, unsigned long long len);
unsigned int c_random_uint();
unsigned int c_random_uint_range(int min, int max);

// CRYPTO FUNCTIONS
// -1 failure, 0 ok

int c_encrypt(
  unsigned char *ciphertext, // message_len + C_EXPBYTES
  const unsigned char *message, // message_len
  unsigned long long message_len,
  const unsigned char *nonce, // C_NONCESIZE
  const unsigned char *key // C_KEYSIZE
);

int c_decrypt(
  unsigned char *decrypted, // ciphertext_len - C_EXPBYTES
  const unsigned char *ciphertext, // ciphertext_len
  unsigned long long ciphertext_len,
  const unsigned char *nonce, // C_NONCESIZE
  const unsigned char *key // C_KEYSIZE
);

int c_hmac(
  unsigned char *out, // H_BYTES
  const unsigned char *in, // inlen
  unsigned long long inlen,
  const unsigned char *k // H_KEYSIZE
);

int c_hmac_verify(
  const unsigned char *h, // H_BYTES
  const unsigned char *in, // inlen
  unsigned long long inlen,
  const unsigned char *k // H_KEYSIZE
);

#endif /* ClientCrypt_h */
