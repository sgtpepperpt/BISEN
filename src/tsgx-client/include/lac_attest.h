#ifndef __SGX_MPC_LAC_ATTEST_H
#define __SGX_MPC_LAC_ATTEST_H

#include "sgx_mpc.h"
#include "lac.h"

int lac_attest(
    int sock,
  bytes *omsg,
  size *omsglen,
  const void *handle,
  const label l,
  const bytes imsg,
  const size imsglen
);

#endif
