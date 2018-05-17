/* this file is an integration test/show how to use libsodium-sgx;
 * it does not test the correctness of printed results;
 */

#include <sodium.h>
#include <assert.h>
#include <string.h>

/* temporary solution until the integration with sgxmpc is completed */
#include <sodium-sgx-renamed.h>

/* 
 * crypto_hash_sha512

   crypto_hmac_sha256

   crypto_secretbox
   crypto_secretbox_open

   crypto_scalarmult
   crypto_scalarmult_base

   crypto_sign_keypair
   crypto_sign
   crypto_sign_open

   crypto_verify_16
   crypto_verify_32
 *
 */

void print_uca(unsigned char *in, unsigned long long in_len)
{
  int i=0;
  while(in_len-- > 0) { ((i++ & 0x7) == 0) ? printf(" %02x",*in++) : printf("%02x",*in++); }
  printf("\n");
}

/******************************************************************************/
/** SHA512 ********************************************************************/
/******************************************************************************/
int test_crypto_hash_sha512()
{
  /* The following WARNING APPEARS IN sodium/crypto_hash_sha512.h
   *
     * WARNING: Unless you absolutely need to use SHA512 for interoperatibility,
     * purposes, you might want to consider crypto_generichash() instead.
     * Unlike SHA512, crypto_generichash() is not vulnerable to length
     * extension attacks.
   */
  unsigned char out[crypto_hash_sha512_BYTES];
  unsigned char in[4];
  unsigned long long in_len = 4;
  int r;

  printf("\ncrypto_hash_sha512:\n");
  printf("defined macros: \n");
  printf("crypto_hash_sha512_BYTES: %d\n", crypto_hash_sha512_BYTES);

  printf("sha512 of : 0x01020304 : \n");
  in[0] = 0x01; in[1]=0x02; in[2]=0x03; in[3]=0x04; in_len = 4;
  r = sodium_crypto_hash_sha512(out, in, in_len);
  print_uca(out,crypto_hash_sha512_BYTES);

  return r;
}


/******************************************************************************/
/** HMAC-SHA256 ***************************************************************/
/******************************************************************************/
int test_crypto_auth_hmacsha256()
{
  unsigned char out[crypto_auth_hmacsha256_BYTES];
  unsigned char in[4];
  unsigned long long in_len = 4;
  unsigned char key[crypto_auth_hmacsha256_KEYBYTES];
  int r;

  printf("\ncrypto_auth_hmacsha256 && crypto_auth_hmacsha256_verify:\n");
  printf("defined macros: \n");
  printf("crypto_auth_hmacsha256_BYTES : %d\n", crypto_auth_hmacsha256_BYTES);
  printf("crypto_auth_hmacsha256_KEYBYTES : %d\n", crypto_auth_hmacsha256_KEYBYTES);

  printf("hmacsha256 of : 0x01020304  && key : 0x3737..37 \n");
  in[0] = 0x01; in[1]=0x02; in[2]=0x03; in[3]=0x04;
  memset(key, 0x37, sizeof(unsigned char)*crypto_auth_hmacsha256_KEYBYTES);

  r = sodium_crypto_auth_hmacsha256(out, in, in_len,key);
  print_uca(out,crypto_auth_hmacsha256_BYTES);

  // r should be 0  
  r |= sodium_crypto_auth_hmacsha256_verify(out, in, in_len, key);
  if(r != 0) return r;
  
  // r should be != 0
  in[0]++;
  r |= sodium_crypto_auth_hmacsha256_verify(out, in, in_len, key);
  if(r == 0) return -1;

  return 0;
}

/******************************************************************************/
/** SECRETBOX *****************************************************************/
/******************************************************************************/
int test_crypto_secretbox_easy(
  unsigned char nonce[crypto_secretbox_NONCEBYTES],
  unsigned char key[crypto_secretbox_KEYBYTES]
)
{
  printf("\ncrypto_secretbox_easy && crypto_secretbox_open_easy:\n");
  printf("defined macros : easy : \n");
  printf("crypto_secretbox_KEYBYTES : %d\n", crypto_secretbox_KEYBYTES);
  printf("crypto_secretbox_NONCEBYTES : %d\n", crypto_secretbox_NONCEBYTES);
  printf("crypto_secretbox_MACBYTES : %d\n", crypto_secretbox_MACBYTES);
  printf("crypto_secretbox_PRIMITIVE : %s\n", crypto_secretbox_PRIMITIVE);
  printf("crypto_secretbox_MESSAGEBYTES_MAX : %lu\n", crypto_secretbox_MESSAGEBYTES_MAX);

  #define LEN 4

  unsigned char out[crypto_secretbox_MACBYTES + LEN];
  unsigned char in[LEN];
  unsigned char in_d[LEN];
  unsigned long long in_len = LEN;
  int r;

  printf("secretbox_easy of : 0x01020304  && key : ?? && nonce : ?? \n");
  in[0] = 0x01; in[1]=0x02; in[2]=0x03; in[3]=0x04;

  r = sodium_crypto_secretbox_easy(out, in, in_len, nonce, key);
  print_uca(out,crypto_secretbox_MACBYTES+LEN);

  // r should be 0  
  r |= sodium_crypto_secretbox_open_easy(in_d, out, crypto_secretbox_MACBYTES+LEN, nonce, key);
  if(r != 0) return r;

  // in_d == in
  r |= sodium_sodium_memcmp(in, in_d, LEN);
  if(r != 0) return r;

  // r should be != 0
  out[0]++;
  r |= sodium_crypto_secretbox_open_easy(in_d, out, crypto_secretbox_MACBYTES+LEN, nonce, key);
  if(r == 0) return -1;

  return 0;

  #undef LEN
}


/******************************************************************************/
/** NACL SECRETBOX ************************************************************/
/******************************************************************************/
int test_crypto_secretbox(
  unsigned char nonce[crypto_secretbox_NONCEBYTES],
  unsigned char key[crypto_secretbox_KEYBYTES]
)
{
  printf("\ncrypto_secretbox && crypto_secretbox_open:\n");
  printf("defined macros : nacl : \n");
  printf("crypto_secretbox_KEYBYTES : %d\n", crypto_secretbox_KEYBYTES);
  printf("crypto_secretbox_NONCEBYTES : %d\n", crypto_secretbox_NONCEBYTES);
  printf("crypto_secretbox_MACBYTES : %d\n", crypto_secretbox_MACBYTES);
  printf("crypto_secretbox_PRIMITIVE : %s\n", crypto_secretbox_PRIMITIVE);
  printf("crypto_secretbox_MESSAGEBYTES_MAX : %lu\n", crypto_secretbox_MESSAGEBYTES_MAX);
  printf("crypto_secretbox_ZEROBYTES : %d\n", crypto_secretbox_ZEROBYTES);
  printf("crypto_secretbox_BOXZEROBYTES : %d\n", crypto_secretbox_BOXZEROBYTES);

  #define LEN 4

  unsigned char out[crypto_secretbox_BOXZEROBYTES + crypto_secretbox_MACBYTES + LEN];
  unsigned long long out_len = crypto_secretbox_BOXZEROBYTES + crypto_secretbox_MACBYTES + LEN;

  unsigned char in[crypto_secretbox_ZEROBYTES + LEN];
  unsigned char in_d[crypto_secretbox_ZEROBYTES + LEN];
  unsigned long long in_len=crypto_secretbox_ZEROBYTES + LEN;
  int r;

  printf("secretbox of : 0x01020304  && key : ?? && nonce : ?? \n");

  memset(out, 0, sizeof(out));

  memset(in, 0, sizeof(in));
  in[crypto_secretbox_ZEROBYTES+0] = 0x01;
  in[crypto_secretbox_ZEROBYTES+1]=0x02;
  in[crypto_secretbox_ZEROBYTES+2]=0x03;
  in[crypto_secretbox_ZEROBYTES+3]=0x04;

  memset(in_d, 0, sizeof(in_d));

  r = sodium_crypto_secretbox(out, in, in_len, nonce, key);
  print_uca(out,out_len);

  // r should be 0  
  r |= sodium_crypto_secretbox_open(in_d, out, out_len, nonce, key);
  if(r != 0) return r;

  // in_d == in
  r |= sodium_sodium_memcmp(in, in_d, LEN);
  if(r != 0) return r;

  // r should be != 0
  out[crypto_secretbox_BOXZEROBYTES]++;
  r |= sodium_crypto_secretbox_open(in_d, out, out_len, nonce, key);
  if(r == 0) return -1;

  return 0;
  #undef LEN
}

/******************************************************************************/
/** SCALARMULT ****************************************************************/
/******************************************************************************/
int test_crypto_scalarmult()
{
  printf("\ncrypto_scalarmult && crypto_scalarmult_base:\n");
  printf("defined macros : \n");
  printf("crypto_scalarmult_BYTES : %d\n", crypto_scalarmult_BYTES);
  printf("crypto_scalarmult_SCALARBYTES : %d\n", crypto_scalarmult_SCALARBYTES);
  printf("crypto_scalarmult_PRIMITIVE : %s\n", crypto_scalarmult_PRIMITIVE);

  unsigned char q[crypto_scalarmult_BYTES];
  unsigned char n[crypto_scalarmult_SCALARBYTES];
  unsigned char p[crypto_scalarmult_BYTES];
  int r;

  memset(q,0,sizeof(q));
  memset(n,0x37,sizeof(n));
  memset(p,0x59,sizeof(p));

  r = sodium_crypto_scalarmult(q,n,p);
  print_uca(q,crypto_scalarmult_BYTES);

  r |= sodium_crypto_scalarmult_base(q,n);
  print_uca(q,crypto_scalarmult_BYTES);
  
  return r;
}


/******************************************************************************/
/** SIGN **********************************************************************/
/******************************************************************************/
int test_crypto_sign()
{
  printf("\ncrypto_sign_keypair && crypto_sign && crypto_sign_open\n");
  printf("defined macros : \n");
  printf("crypto_sign_BYTES : %d\n", crypto_sign_BYTES);
  printf("crypto_sign_SEEDBYTES : %d\n", crypto_sign_SEEDBYTES);

  printf("crypto_sign_PUBLICKEYBYTES : %d\n", crypto_sign_PUBLICKEYBYTES);
  printf("crypto_sign_SECRETKEYBYTES : %d\n", crypto_sign_SECRETKEYBYTES);
  printf("crypto_sign_MESSAGEBYTES_MAX : %lu\n", crypto_sign_MESSAGEBYTES_MAX);
  printf("crypto_sign_PRIMITIVE : %s\n", crypto_sign_PRIMITIVE);

  #define LEN 4

  unsigned char pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk[crypto_sign_SECRETKEYBYTES];

  unsigned char message[LEN];
  unsigned long long message_len = LEN;
  unsigned char signed_message[LEN + crypto_sign_BYTES];
  unsigned long long signed_message_len = LEN + crypto_sign_BYTES;
  unsigned char recovered_message[LEN];
  unsigned long long recovered_message_len = LEN;
  int r;

  r = sodium_crypto_sign_keypair(pk,sk);
  message[0] = 0x01;
  message[1] = 0x02;
  message[2] = 0x03;
  message[3] = 0x04;

  r |= sodium_crypto_sign(signed_message, &signed_message_len,
                   message, message_len,
                   sk);
  print_uca(signed_message,signed_message_len);
  if(r != 0) return r;

  r |= sodium_crypto_sign_open(recovered_message,&recovered_message_len,
                        signed_message, signed_message_len,
                        pk);
  print_uca(recovered_message,recovered_message_len);

  signed_message[0]++;
  r |= sodium_crypto_sign_open(recovered_message,&recovered_message_len,
                        signed_message, signed_message_len,
                        pk);
  if(r == 0) return -1;

  return 0;
  #undef LEN
}

/******************************************************************************/
/** VERIFY ********************************************************************/
/******************************************************************************/
int test_crypto_verify()
{
  unsigned char a[32];
  unsigned char b[32];
  int r;

  printf("\ncrypto_verify_32 && crypto_verify_16\n");
  printf("defined macros : \n");
  printf("crypto_verify_32_BYTES : %d\n", crypto_verify_32_BYTES);
  printf("crypto_verify_16_BYTES : %d\n", crypto_verify_16_BYTES);

  sodium_randombytes(a,32);
  memcpy(b,a,sizeof(a));

  r = sodium_crypto_verify_32(a,b);
  r |= sodium_crypto_verify_16(a,b);
  print_uca(a,32);
  print_uca(b,32);
  if(r != 0) return r;

  a[0]++;
  r = sodium_crypto_verify_32(a,b);
  if(r == 0) return -1;

  return 0;
}

int
main (int argc, char *argv[])
{
  unsigned char nonce[crypto_secretbox_NONCEBYTES];
  unsigned char key[crypto_secretbox_KEYBYTES];
  memset(nonce, 0x37, sizeof(unsigned char)*crypto_secretbox_NONCEBYTES);
  sodium_crypto_secretbox_keygen(key);

  assert(test_crypto_hash_sha512() == 0);
  assert(test_crypto_auth_hmacsha256() == 0);
  assert(test_crypto_secretbox_easy(nonce,key) == 0);
  assert(test_crypto_secretbox(nonce,key) == 0);
  assert(test_crypto_scalarmult() == 0);
  assert(test_crypto_sign() == 0);
  assert(test_crypto_verify() == 0);

  return 0;
}


