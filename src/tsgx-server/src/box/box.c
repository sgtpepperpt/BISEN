#include <stdlib.h>

#include "attke_par.h"
#include "ae.h"
#include "box.h"
#include "crypto.h"

// f code will be linked later
extern void f(
  bytes *out,
  size *outlen,
  const label pid,
  const bytes in,
  const size inlen
);

static byte counter_in[SGX_MPC_NPARTIES][SGX_MPC_AEAD_NONCEBYTES]; 
static byte counter_out[SGX_MPC_NPARTIES][SGX_MPC_AEAD_NONCEBYTES]; 

int inc_counter(
  bytes counter
)
{
  int i,sum,carry;
  sum = (int) counter[0] + 1;
  counter[0] = (byte) sum;
  carry=sum >> 8;

  for(i=1; i < SGX_MPC_AEAD_NONCEBYTES; i++)
  { sum = (int) counter[i] + carry;
    counter[i] = (byte) sum;
    carry=sum >> 8;    
  }

  return SGX_MPC_OK;
}

// to be run on load
int box_init()
{
  int i;
  for(i=0; i < SGX_MPC_NPARTIES; i++)
  { byte_zero(counter_in[i], SGX_MPC_AEAD_NONCEBYTES);
    byte_zero(counter_out[i], SGX_MPC_AEAD_NONCEBYTES);
    counter_out[i][0] = 1;
  }

  return SGX_MPC_OK;
}

// allocates output memory. adds SGX_MPC_AEAD_EXPBYTES to whatever f returns
int box(
  bytes *omsg,
  size *omsglen,
  const label pid,
  const bytes inmsg,
  const size inmsglen
)
{
  bytes out;
  size outlen;
  bytes in;
  size inlen;
  byte key[SGX_MPC_AEAD_KEYBYTES];
  int res;
  
  if(! (pid >= 1 && pid <= SGX_MPC_NPARTIES) )
  { *omsg = NULL;
    *omsglen = 0;
    return SGX_MPC_ERROR;
  }

  attke_par_getkey(key,pid);

  res = ae_dec(&in, &inlen, inmsg, inmsglen, key, counter_in[pid-1]);
  if(res != SGX_MPC_OK)
  { *omsg = NULL;
    *omsglen = 0;
    byte_zero(key, SGX_MPC_AEAD_KEYBYTES);
    return SGX_MPC_ERROR;
  }
  
  inc_counter(counter_in[pid-1]);
  inc_counter(counter_in[pid-1]);

  f(&out, &outlen, pid, in, inlen);

  res = ae_enc(omsg, omsglen, out, outlen, key, counter_out[pid-1]); 

  free(in);
  free(out);

  if(res != SGX_MPC_OK)
  { *omsg = NULL;
    *omsglen = 0;
    byte_zero(key,SGX_MPC_AEAD_KEYBYTES);
    return SGX_MPC_ERROR;
  }

  inc_counter(counter_out[pid-1]);
  inc_counter(counter_out[pid-1]);

  return SGX_MPC_OK;
}
