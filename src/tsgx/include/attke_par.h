#ifndef __SGX_MPC_ATTKE_PAR_H
#define __SGX_MPC_ATTKE_PAR_H

#include "sgx_mpc.h"

#define ATTKE_P_RUNNING 1
#define ATTKE_P_FINISHED 2
#define ATTKE_P_ERROR 4

int attke_par_init(void);

int attke_par(
  bytes *omsg,
  size *omsglen,
  const label pid,
  const bytes inmsg,
  const size inmsglen
);

int attke_par_getkey(
  bytes key,
  const label pid
);

byte attke_par_getstage();

#endif
