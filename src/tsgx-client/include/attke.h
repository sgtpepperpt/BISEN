#ifndef __SGX_MPC_ATTKE_H
#define __SGX_MPC_ATTKE_H

#include "sgx_mpc.h"

#define ATTKE_L_INITIALIZED 1
#define ATTKE_L_SENT_EPHPK_DERIVED 2
#define ATTKE_L_GOTOK_ACCEPT 4
#define ATTKE_L_ERROR 8

typedef struct {
  byte stage;
  byte sigsk[SGX_MPC_SECRETKEYBYTES];
  byte nonce[16];
  byte key[SGX_MPC_AEAD_KEYBYTES];
} attke_local_state;

int attke_setup(
  attke_local_state **local_st,
  bytes *sigpk
);

int attke_local(
  bytes *omsg,
  size *omsglen,
  attke_local_state* st,
  const bytes inmsg,
  const size inmsglen
);

#endif
