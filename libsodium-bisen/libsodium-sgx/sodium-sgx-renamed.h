#ifndef sodium_sgx_renamed_H
#define sodium_sgx_renamed_H

void sodium_memzero(void *const pnt, const size_t len);
int sodium_sodium_memcmp(const void *const b1_, const void *const b2_, size_t len);
void * sodium_memmove(void * destination, const void * source, size_t num);
void sodium_randombytes(unsigned char *output, unsigned long long len);
void sodium_randombytes_buf(void * const buf, const size_t size);


int sodium_crypto_hash_sha512(unsigned char *out, const unsigned char *in,
                       unsigned long long inlen);


int sodium_crypto_auth_hmacsha256(unsigned char *out,
                           const unsigned char *in,
                           unsigned long long inlen,
                           const unsigned char *k);

int sodium_crypto_auth_hmacsha256_verify(const unsigned char *h,
                                  const unsigned char *in,
                                  unsigned long long inlen,
                                  const unsigned char *k);

void sodium_crypto_secretbox_keygen(unsigned char k[crypto_secretbox_KEYBYTES]);

int sodium_crypto_secretbox_easy(unsigned char *c, const unsigned char *m,
                          unsigned long long mlen, const unsigned char *n,
                          const unsigned char *k);

int sodium_crypto_secretbox_open_easy(unsigned char *m, const unsigned char *c,
                               unsigned long long clen, const unsigned char *n,
                               const unsigned char *k);

int sodium_crypto_secretbox(unsigned char *c, const unsigned char *m,
                     unsigned long long mlen, const unsigned char *n,
                     const unsigned char *k);

int sodium_crypto_secretbox_open(unsigned char *m, const unsigned char *c,
                          unsigned long long clen, const unsigned char *n,
                          const unsigned char *k);

int sodium_crypto_scalarmult_base(unsigned char *q, const unsigned char *n);

int sodium_crypto_scalarmult(unsigned char *q, const unsigned char *n,
                      const unsigned char *p);

int sodium_crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int sodium_crypto_sign(unsigned char *sm, unsigned long long *smlen_p,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk);

int sodium_crypto_sign_open(unsigned char *m, unsigned long long *mlen_p,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk);

int sodium_crypto_verify_32(const unsigned char *x, const unsigned char *y);

int sodium_crypto_verify_16(const unsigned char *x, const unsigned char *y);

#endif
