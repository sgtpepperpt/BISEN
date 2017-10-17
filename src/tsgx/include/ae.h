#ifndef __SGX_MPC_AE_H
#define __SGX_MPC_AE_H

#include "sgx_mpc.h"

int ae_enc(
  bytes *c,
  size *clen, 
  const bytes m,
  const size mlen,
  const bytes k,
  const bytes nonce
);

int ae_dec(
  bytes *m,
  size *mlen, 
  const bytes c,
  const size clen,
  const bytes k,
  const bytes nonce
);

#endif
