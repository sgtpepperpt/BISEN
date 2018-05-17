#include <stdlib.h>
#include "types.h"
#include "crypto.h"

// local keys 
byte ckey[C_KEYSIZE];
byte hkey[H_KEYSIZE];

// 0x01 : setup_secret_keys
static void f_setup_secret_keys(
  bytes *out,
  size *outlen
)
{
  c_random(ckey,C_KEYSIZE);
  c_random(hkey,H_KEYSIZE);
  *out = (bytes) malloc(sizeof(byte)*1);
  (*out)[0] = 0x00;
  *outlen = 1;
}

// 0x02 : create_or_reset_file
static void f_create_or_reset_file(
  bytes *out,
  size *outlen,
  const bytes in,
  const size inlen
)
{
  fserver(out, outlen, in, inlen);
}

// 0x3 : add byte string to file :: note : toy example, only works for small inputs
static void f_add_byte_string_to_file(
  bytes *out,
  size *outlen,
  const bytes in,
  const size inlen
)
{
  size ciphertext_len = in[1]/*byte string len*/+C_EXPBYTES/*expansion bytes*/;
  size fserver_inlen = 1/*opcode*/+1/*ciphertext len*/+ciphertext_len;
  byte fserver_in[fserver_inlen];

  fserver_in[0] = in[0];
  fserver_in[1] = (byte) ciphertext_len;
  c_encrypt(fserver_in+2, in+2, in[1], ckey);
  fserver(out, outlen, fserver_in, fserver_inlen);
}

// 0x4 : get byte string from file
static void f_get_byte_string_from_file(
  bytes *out,
  size *outlen,
  const bytes in,
  const size inlen
)
{
  int r, i;
  fserver(out, outlen, in, inlen);
  byte decrypted[(*outlen)-C_EXPBYTES];
  r = c_decrypt(decrypted, *out, *outlen, ckey);
  (*out)[0] = r&0xff;
  for(i=0;i<(*outlen)-C_EXPBYTES;i++)
    (*out)[i+1] = decrypted[i];
  for(;i<(*outlen)-1;i++)
    (*out)[i+1] = 0;
  *outlen = (*outlen)-C_EXPBYTES+1;
}

void f(
  bytes *out,
  size *outlen,
  const label pid,
  const bytes in,
  const size inlen
)
{
  // set out variables
  *out = NULL;
  *outlen = 0;

  // 0x1 : setup secret key
  if(in[0] == 0x01)
  { f_setup_secret_keys(out,outlen);
    return;
  }

  // 0x2 : create or reset file
  if(in[0] == 0x02)
  { f_create_or_reset_file(out,outlen,in,inlen);
    return;
  }

  // 0x3 : add byte string to file
  if(in[0] == 0x03)
  { f_add_byte_string_to_file(out,outlen,in,inlen);
    return;
  }

  // 0x4 : get byte string from file
  if(in[0] == 0x04)
  { f_get_byte_string_from_file(out,outlen,in,inlen);
    return;
  }
}



