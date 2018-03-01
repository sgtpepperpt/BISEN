#include <stdlib.h>

#include "attke_remote.h"
#include "attke_par.h"
#include "crypto.h"

#ifndef SGX_MPC_OUTSIDE
 #include "Enclave_t.h" 
#endif

// dynamically generated file will fix Public Keys
extern byte pubs[SGX_MPC_NPARTIES][SGX_MPC_PUBLICKEYBYTES];
static attke_remote_state states[SGX_MPC_NPARTIES];
static byte par_stage;


// code to be run on load
int attke_par_init(void)
{
  int i;
  for (i=0;i<SGX_MPC_NPARTIES;i++) 
  { attke_remote_init(&states[i]); }
  par_stage = ATTKE_P_RUNNING;
  return SGX_MPC_OK;
}


// code to be run on activation until all key exchanges accept.
// it passes down calls to the appropriate attke instance.
int attke_par(
  bytes *omsg,
  size *omsglen,
  const label pid,
  const bytes inmsg,
  const size inmsglen
)
{
  int i;
  byte stg_acc_der;
  byte stg_acc_err;

  if(! (pid >= 1 && pid <= SGX_MPC_NPARTIES) )
  { *omsg=NULL;
    *omsglen=0;
    par_stage = ATTKE_P_ERROR;
    return SGX_MPC_ERROR;
  }

  if(par_stage == ATTKE_P_RUNNING)
  { attke_remote(omsg, omsglen, &states[pid-1], inmsg, inmsglen, pubs[pid-1]);
    stg_acc_der = ATTKE_R_SENTOK_ACCEPT;
    stg_acc_err = 0;

    for(i=0;i<SGX_MPC_NPARTIES;i++)
    { stg_acc_der &= states[i].stage;
      stg_acc_err |= states[i].stage;
    }

    if(stg_acc_der == ATTKE_R_SENTOK_ACCEPT)
    { // all in accept state
      par_stage = ATTKE_P_FINISHED;
      return SGX_MPC_OK;
    }
    else
    if( (stg_acc_err & ATTKE_R_ERROR) == 0 )
    { // something is still running
      return SGX_MPC_OK;
    }
    else
    { return SGX_MPC_ERROR;
    }

  }
  else
  { *omsg=NULL;
    *omsglen=0;
    par_stage = ATTKE_P_ERROR;
    return SGX_MPC_ERROR;
  }
}


// to be run after everything finished to get the keys
int attke_par_getkey(
  bytes key,
  const label pid
)
{
  if(! (par_stage == ATTKE_P_FINISHED && 
        pid >= 1                      && 
        pid <= SGX_MPC_NPARTIES       )
    )
  { return SGX_MPC_ERROR; }

  byte_copy(key, SGX_MPC_AEAD_KEYBYTES, states[pid-1].key);
  return SGX_MPC_OK;
}

byte attke_par_getstage()
{
  return par_stage;
}

