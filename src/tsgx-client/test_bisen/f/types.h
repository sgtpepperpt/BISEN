#ifndef __TYPES__
#define __TYPES__

#include "sgx_mpc.h"

void f(
  bytes *out,
  size *outlen,
  const label pid,
  const bytes in,
  const size inlen
);

void fserver(
  bytes *out,
  size *outlen,
  const bytes in,
  const size inlen
);

#endif 
