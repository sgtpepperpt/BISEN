#ifndef __SGX_MPC_MACH_H
#define __SGX_MPC_MACH_H

#include "sgx_mpc.h"

#ifndef SGX_MPC_OUTSIDE
 #include "sgx_report.h"
#endif

#ifdef SGX_MPC_OUTSIDE
 #define SGX_MPC_MACH_REPLEN 17 // dummy for size checking
#else
 //#define SGX_MPC_MACH_SIGLEN 64
 #define SGX_MPC_MACH_REPLEN sizeof(sgx_report_t)
#endif

int mach_load(
  void **handle,
  char *filename
);

int mach_run(
  bytes *omsg,
  size *omsglen,
  const void *handle,
  const label l,
  const bytes imsg,
  const size imsglen
);

int mach_quote(
  bytes omsg,
  size omsglen,
  const bytes imsg,
  const size imsglen
);

int mach_verify(
  const bytes imsg,
  const size imsglen,
  const bytes code,
  const size codelen
);

void mach_finalize();

#endif
