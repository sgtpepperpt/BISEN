#ifndef __SGX_MPC_MACH_IEE_H
#define __SGX_MPC_MACH_IEE_H

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

int iee_report(
  bytes report,
  const bytes imsg,
  const size imsglen
);

#endif
