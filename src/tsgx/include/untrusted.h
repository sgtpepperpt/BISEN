#ifndef __SGX_MPC_UNTRUSTED_H
#define __SGX_MPC_UNTRUSTED_H

#include "sgx_mpc.h"

void untrusted_malloc_bytes(
  bytes *pointer,
  size length
);

#endif
