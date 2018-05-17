#ifndef __CRYPTO__
#define __CRYPTO__

#include <sodium.h>

#define C_KEYSIZE  crypto_secretbox_KEYBYTES
#define C_EXPBYTES (crypto_secretbox_NONCEBYTES+crypto_secretbox_MACBYTES)

#define H_KEYSIZE crypto_auth_hmacsha256_KEYBYTES
#define H_BYTES crypto_auth_hmacsha256_BYTES

void c_random(
  unsigned char *buf,
  unsigned long long len
);

// -1 failure, 0 ok
int c_encrypt(
  unsigned char *ciphertext, // message_len + C_EXPBYTES
  const unsigned char *message, // message_len
  unsigned long long message_len,
  const unsigned char *key // C_KEYSIZE
);

// -1 failure, 0 ok
int c_decrypt(
  unsigned char *message, // ciphertext_len - C_EXPBYTES
  const unsigned char *ciphertext, // ciphertext_len
  unsigned long long ciphertext_len,
  const unsigned char *key // C_KEYSIZE
);

// -1 failure, 0 ok
int c_hmac(
  unsigned char *out, // H_BYTES
  const unsigned char *in, // inlen
  unsigned long long inlen,
  const unsigned char *k // H_KEYSIZE
);

// -1 failure, 0 ok
int c_hmac_verify(
  const unsigned char *h, // H_BYTES
  const unsigned char *in, // inlen
  unsigned long long inlen,
  const unsigned char *k // H_KEYSIZE
);

#endif
