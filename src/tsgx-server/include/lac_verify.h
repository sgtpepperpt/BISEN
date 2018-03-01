#ifndef __SGX_MPC_LAC_VERIFY_H
#define __SGX_MPC_LAC_VERIFY_H

#include "sgx_mpc.h"
#include "lac.h"

int lac_verify_init(
  const bytes cde,
  const size cdelen,
  const bytes prms,
  const label partyid
);

int lac_verify(
  bytes *omsg,
  size *omsglen,
  const label l,
  const bytes in,
  const size inlen,
  const bytes out,
  const size outlen
);

#endif
