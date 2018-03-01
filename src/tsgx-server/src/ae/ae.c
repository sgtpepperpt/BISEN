#include <stdlib.h>

#include "crypto.h"
#include "ae.h"

int ae_enc(
  bytes *c,
  size *clen, 
  const bytes m,
  const size mlen,
  const bytes k,
  const bytes nonce
)
{
  int res;

  *clen = mlen + crypto_secretbox_MACBYTES;
  *c = (bytes) malloc(*clen);
  if(!(*c))
  { *clen = 0;
    return SGX_MPC_ERROR;
  }

  res = crypto_secretbox_easy(*c, m, mlen, nonce, k);
  if(res != 0)
  { byte_zero(*c, *clen);
    free(*c);
    *c = NULL;
    *clen = 0;
    return SGX_MPC_ERROR;
  }

  return SGX_MPC_OK;
}

int ae_dec(
  bytes *m,
  size *mlen, 
  const bytes c,
  const size clen,
  const bytes k,
  const bytes nonce
)
{
  int res;

  *mlen = clen - crypto_secretbox_MACBYTES;
  *m = (bytes) malloc(*mlen);
  if(!*m)
  { *mlen = 0;
    return SGX_MPC_ERROR;
  }

  res = crypto_secretbox_open_easy(*m, c, clen, nonce, k);
  if(res != 0)
  { byte_zero(*m,*mlen);
    free(*m);
    *m = NULL;
    *mlen = 0;
    return SGX_MPC_ERROR;
  }

  return SGX_MPC_OK;
}
