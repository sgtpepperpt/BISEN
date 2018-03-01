#ifndef __SGX_MPC_CRYPTO_H
#define __SGX_MPC_CRYPTO_H

#include <sodium.h>

void byte_zero(void *yv,long long ylen);
void byte_copy(void *yv,long long ylen,const void *xv);

#endif
