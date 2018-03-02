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

int remote_mach_load(
    int sock,
    void **handle,
    char *filename
);

int remote_mach_run(
    int sock,
    bytes *omsg,
    size *omsglen,
    const void *handle,
    const label l,
    const bytes imsg,
    const size imsglen
);

int remote_mach_quote(
    int sock,
    bytes omsg,
    size omsglen,
    const bytes imsg,
    const size imsglen
);

int remote_mach_verify(
    int sock,
    const bytes imsg,
    const size imsglen,
    const bytes code,
    const size codelen
);

void remote_mach_finalize(int sock);

#endif
