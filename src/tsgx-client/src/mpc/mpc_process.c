#include <stdlib.h>
#include <stdio.h>

#include "attke.h"
#include "attke.h"
#include "ae.h"
#include "lac_verify.h"
#include "mpc_process.h"
#include "crypto.h"

#define STAGE_P 0x00
#define STAGE_Q 0x80

static byte proc_stage;
static byte counter_in_local[SGX_MPC_AEAD_NONCEBYTES];
static byte counter_out_local[SGX_MPC_AEAD_NONCEBYTES];


int inc_counter_local(
  bytes counter
)
{
  int i,sum,carry;

  sum = (int) counter[0] + 1;
  counter[0] = (byte) sum;
  carry = sum >> 8;

  for(i=1; i < SGX_MPC_AEAD_NONCEBYTES; i++)
  { sum = (int) counter[i] + carry;
    counter[i] = (byte) sum;
    carry = sum >> 8;
  }
  return SGX_MPC_OK;
}


static attke_local_state *attke_st;
static bytes sigpk;
static label pid;
static bytes lastm;
static size lastmlen;

// assumes code and prms refs can be kept
int mpc_process_init(
    int sock,
  const bytes code,
  const size codelen,
  const bytes prms,
  const label partyid,
  const bytes pk,
  attke_local_state* st
)
{
  pid = partyid;
  proc_stage = MPC_P_RUNNING_P;

  byte_zero(counter_in_local, SGX_MPC_AEAD_NONCEBYTES);
  byte_zero(counter_out_local, SGX_MPC_AEAD_NONCEBYTES);

  counter_out_local[0] = 1;
  lastm = NULL;
  lastmlen = 0;
  attke_st=st;
  sigpk = pk;

  return lac_verify_init(sock, code, codelen, prms, partyid);
}

/* in the first stage, incoming messages are always expected to come
   from the server side in attested form
   in second stage, inmsg is either an incoming msg for the server,
   or the next input to be sent
   this is indicated by the newinput flag
   if it's and incoming message, then omsg is an output
   otherwise, omsg is the message to be sent */
int mpc_process(
    int sock,
  bytes *omsg,
  size *omsglen,
  const label l,
  const bytes inmsg,
  const size inmsglen,
  const int newinput
)
{
  int res;
  bytes kemsgin;
  size kemsginlen;
  byte lstage = 0x80 & l;
  byte mypid = 0x7f & l;

  if(mypid != pid)
  { *omsg = NULL;
    *omsglen = 0;
    proc_stage = MPC_P_ERROR;
    return SGX_MPC_ERROR;
  }

  if(proc_stage == MPC_P_RUNNING_P)
  {
    if(lstage != STAGE_P)
    { *omsg = NULL;
      *omsglen = 0;
      proc_stage = MPC_P_ERROR;
      return SGX_MPC_ERROR;
    }

    res = lac_verify(sock, &kemsgin, &kemsginlen, pid, lastm, lastmlen, inmsg, inmsglen);
    if(res != SGX_MPC_OK)
    { *omsg = NULL;
      *omsglen = 0;
      proc_stage = MPC_P_ERROR;
      return SGX_MPC_ERROR;
    }

    attke_local(omsg, omsglen, attke_st, kemsgin, kemsginlen);
    byte_zero(kemsgin, kemsginlen);
    free(kemsgin);

    if(attke_st->stage == ATTKE_L_ERROR)
    { *omsg = NULL;
      *omsglen = 0;
      proc_stage = MPC_P_ERROR;
      return SGX_MPC_ERROR;
    }

    if(lastm)
    { free(lastm); }

    lastm = (bytes) malloc(*omsglen);
    if(!lastm)
    { *omsg = NULL;
      *omsglen = 0;
      proc_stage = MPC_P_ERROR;
      return SGX_MPC_ERROR;
    }

    byte_copy(lastm, *omsglen, *omsg);
    lastmlen = *omsglen;

    if(attke_st->stage == ATTKE_L_GOTOK_ACCEPT)
    { proc_stage = MPC_P_RUNNING_Q; }

    return SGX_MPC_OK;

  } // end (proc_stage == MPC_P_RUNNING_P)
  else
  if(proc_stage == MPC_P_RUNNING_Q)
  {
    if(lstage != STAGE_Q)
    { *omsg = NULL;
      *omsglen = 0;
      proc_stage = MPC_P_ERROR;
      return SGX_MPC_ERROR;
    }

    if(newinput) // input requested
    {
      res = ae_enc(omsg, omsglen, inmsg, inmsglen, attke_st->key, counter_in_local);
      if(res != SGX_MPC_OK)
      { *omsg = NULL;
        *omsglen = 0;
        proc_stage = MPC_P_ERROR;
        return SGX_MPC_ERROR;
      }

      inc_counter_local(counter_in_local);
      inc_counter_local(counter_in_local);
      return SGX_MPC_OK;
    }
    else // output delivered
    {
      res = ae_dec(omsg, omsglen, inmsg, inmsglen, attke_st->key, counter_out_local);
      if(res!=SGX_MPC_OK)
      { *omsg = NULL;
        *omsglen = 0;
        proc_stage = MPC_P_ERROR;
        return SGX_MPC_ERROR;
      }

      inc_counter_local(counter_out_local);
      inc_counter_local(counter_out_local);
      return SGX_MPC_OK;
    }

  } // end (proc_stage == MPC_P_RUNNING_Q)
  else
  { *omsg = NULL;
    *omsglen = 0;
    proc_stage = MPC_P_ERROR;
    return SGX_MPC_ERROR;
  }
}

#ifdef SGX_MPC_TEST
int bypass_local_state_for_test(
  label partyid,
  const bytes pk,
  attke_local_state* st
)
{
  pid = partyid;
  proc_stage = MPC_P_RUNNING_Q;

  byte_zero(counter_in_local, SGX_MPC_AEAD_NONCEBYTES);
  byte_zero(counter_out_local, SGX_MPC_AEAD_NONCEBYTES);

  counter_out_local[0]=1;
  lastm = NULL;
  lastmlen = 0;
  attke_st = st;
  sigpk = pk;

  return SGX_MPC_OK;
}
#endif
