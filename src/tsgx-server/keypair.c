#include <sodium.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void print_public_keys(FILE *f, unsigned char *pk1, unsigned char *pk2)
{
  int i;

  fprintf(f, "#include \"sgx_mpc.h\"\n\n");
  fprintf(f,"byte pubs[2][SGX_MPC_PUBLICKEYBYTES] = {\n");
  fprintf(f," {");

  for(i=0;i<crypto_sign_PUBLICKEYBYTES-1;i++)
  { if(i!=0 && (i&7)==0) fprintf(f,"\n  ");
    fprintf(f,"0x%02X,",pk1[i]); }
  fprintf(f,"0x%02X},\n\n {",pk1[crypto_sign_PUBLICKEYBYTES-1]);

  for(i=0;i<crypto_sign_PUBLICKEYBYTES-1;i++)
  { if(i!=0 && (i&7)==0) fprintf(f,"\n  ");
    fprintf(f,"0x%02X,",pk2[i]);}
  fprintf(f,"0x%02X}\n};\n",pk2[crypto_sign_PUBLICKEYBYTES-1]);
}

static void print_secret_keys(FILE *f, unsigned char *sk1, unsigned char *sk2)
{
  int i;

  fprintf(f, "#include \"attke.h\"\n\n");
  fprintf(f,"attke_local_state lsts[2] = {\n");
  fprintf(f," {0x01,{\n  ");

  for(i=0;i<crypto_sign_SECRETKEYBYTES-1;i++)
  { if(i!=0 && (i&7)==0) fprintf(f,"\n  ");
    fprintf(f,"0x%02X,",sk1[i]); }
  fprintf(f,"0x%02X}},\n\n {0x01,{\n  ",sk1[crypto_sign_SECRETKEYBYTES-1]);

  for(i=0;i<crypto_sign_SECRETKEYBYTES-1;i++)
  { if(i!=0 && (i&7)==0) fprintf(f,"\n  ");
    fprintf(f,"0x%02X,",sk2[i]);}
  fprintf(f,"0x%02X}}\n};\n",sk2[crypto_sign_SECRETKEYBYTES-1]);
}

int gen_crypto_sign_keypair(const char *sk_fn, const char *pk_fn)
{
  int r;
  unsigned char pk1[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk1[crypto_sign_SECRETKEYBYTES];
  unsigned char sd1[crypto_sign_SEEDBYTES];

  unsigned char pk2[crypto_sign_PUBLICKEYBYTES];
  unsigned char sk2[crypto_sign_SECRETKEYBYTES];
  unsigned char sd2[crypto_sign_SEEDBYTES];

  FILE *sk_f, *pk_f;

  sk_f = fopen(sk_fn, "w");
  assert(sk_f != NULL);

  pk_f = fopen(pk_fn, "w");
  assert(pk_f != NULL);

  randombytes_buf(sd1, sizeof(sd1));
  r = crypto_sign_seed_keypair(pk1,sk1,sd1);
  assert(r==0);

  randombytes_buf(sd2, sizeof(sd2));
  r = crypto_sign_seed_keypair(pk2,sk2,sd2);
  assert(r==0);

  print_public_keys(pk_f, pk1, pk2);
  print_secret_keys(sk_f, sk1, sk2);
 
  fclose(sk_f);
  fclose(pk_f);

  return 0;
}


int
main(int argc, char *argv[])
{
  if(argc != 3)
  { printf("usage: ./keypair test_***/secret_key.h test_***/f/public_key.h\n");
    return -1;
  }

  gen_crypto_sign_keypair(argv[1],argv[2]);

  return 0;
}

