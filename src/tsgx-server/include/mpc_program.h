#ifndef __SGX_MPC_MPC_PROGRAM_H
#define __SGX_MPC_MPC_PROGRAM_H

#include "sgx_mpc.h"
#include "mpc.h"

int mpc_program(
  bytes omsg, 
  size omsglen,
  const label l,
  const bytes inmsg,
  const size inmsglen
);

int mpc_program_init(void);

#endif

