#ifndef __SGX_MPC_MPC_PROCESS_H
#define __SGX_MPC_MPC_PROCESS_H

#include "sgx_mpc.h"
#include "mpc.h"

int mpc_process_init(
  const bytes code,
  const size codelen,
  const bytes prms, 
  const label partyid,
  const bytes pk,
  attke_local_state* st
);

int mpc_process(
  bytes *omsg,
  size *omsglen,
  const label l,
  const bytes inmsg,
  const size inmsglen,
  const int newinput
);

#ifdef SGX_MPC_TEST
 int bypass_local_state_for_test(
   label partyid,
   const bytes pk,
   attke_local_state* st
 );
#endif

#endif
