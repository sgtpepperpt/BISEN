#include "crypto.h"

void c_random(
  unsigned char *buf,
  unsigned long long len
)
{
  randombytes_buf(buf, sizeof(unsigned char)*len);
}

// -1 failure, 0 ok
int c_encrypt(
  unsigned char *ciphertext, // message_len + C_EXPBYTES
  const unsigned char *message, // message_len
  unsigned long long message_len,
  const unsigned char *key // C_KEYSIZE
)
{
  randombytes_buf(ciphertext, sizeof(unsigned char)*crypto_secretbox_NONCEBYTES);
  return crypto_secretbox_easy(ciphertext+crypto_secretbox_NONCEBYTES,
                               message, message_len,
                               ciphertext, key);
}

// -1 failure, 0 ok
int c_decrypt(
  unsigned char *message, // ciphertext_len - C_EXPBYTES
  const unsigned char *ciphertext, // ciphertext_len
  unsigned long long ciphertext_len,
  const unsigned char *key // C_KEYSIZE
)
{
  return crypto_secretbox_open_easy(message, ciphertext+crypto_secretbox_NONCEBYTES,
                                    ciphertext_len-crypto_secretbox_NONCEBYTES,
                                    ciphertext, key);
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
int c_hmac_verify(
  const unsigned char *h, // H_BYTES
  const unsigned char *in, // inlen
  unsigned long long inlen,
  const unsigned char *k // H_KEYSIZE
)
{
  return crypto_auth_hmacsha256_verify(h, in, inlen, k);
}
