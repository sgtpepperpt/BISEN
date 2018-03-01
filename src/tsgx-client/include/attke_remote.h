#ifndef __SGX_MPC_ATTKE_REMOTE_H
#define __SGX_MPC_ATTKE_REMOTE_H

#include "sgx_mpc.h"

#define ATTKE_R_INIT 1
#define ATTKE_R_SENT_EPHPK_WAIT 2
#define ATTKE_R_SENTOK_ACCEPT 4
#define ATTKE_R_ERROR 8

typedef struct {
  byte stage;
  byte eph_sk[SGX_MPC_AKE_SCALARBYTES];
  byte eph_pk[SGX_MPC_AKE_KEYBYTES];
  byte nonce[16];
  byte key[SGX_MPC_AEAD_KEYBYTES];
} attke_remote_state;

int attke_remote_init(
 attke_remote_state *st
);

int attke_remote(
  bytes *omsg,
  size *omsglen,
  attke_remote_state *st,
  const bytes inmsg,
  const size inmsglen,
  const bytes sigpk
);

#endif
