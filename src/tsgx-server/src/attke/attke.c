#include <stdlib.h>
#include <stdio.h>

#include "attke.h"
#include "crypto.h"


int attke_setup(
  attke_local_state **local_st,
  bytes *sigpk
)
{
  *local_st = (attke_local_state*) malloc(sizeof(attke_local_state));
  if(!*local_st)
  { return SGX_MPC_ERROR; }

  *sigpk =(bytes) malloc(SGX_MPC_PUBLICKEYBYTES);
  if(!*sigpk)
  { free(*local_st);
    return SGX_MPC_ERROR;
  }

  crypto_sign_keypair(*sigpk, (*local_st)->sigsk);
  (*local_st)->stage = ATTKE_L_INITIALIZED;

  return SGX_MPC_OK;
}


/* Allocates output memory. This is originally expecting to be
   invoked on a message of size SGX_MPC_AKE_KEYBYTES+16, which
   includes server ephemeral key and nonce. The response includes
   this, plus the local ephemeral key and a signature. Overall
   size is 16 + 2* + SGX_MPC_AKE_KEYBYTES+SGX_MPC_SIGEXP_BYTES.
   It will conclude if it receives "OK"||nonce (19 bytes), in
   which case it returns the empty message and accepts. */

int attke_local(
  bytes *omsg,
  size *omsglen,
  attke_local_state* st,
  const bytes inmsg,
  const size inmsglen
)
{
  bytes eph_sk;
  bytes eph_pk;
  bytes mtosign;
  int res;

  if(st->stage == ATTKE_L_INITIALIZED)
  {
    // Loc is first invoked on ephemeral public key || nonce
    if(inmsglen != SGX_MPC_AKE_KEYBYTES+16)
    { *omsg=NULL;
      *omsglen=0;
      st->stage = ATTKE_L_ERROR;
      return SGX_MPC_ERROR;
    }

    // TODO : CHECKME : harcoded nonce size
    // store nonce for future usage
    byte_copy(st->nonce, 16, inmsg+SGX_MPC_AKE_KEYBYTES);

    // Generate ephemeral secret key
    eph_sk = (bytes) malloc(SGX_MPC_AKE_SCALARBYTES);
    if(!eph_sk)
    { *omsg=NULL;
      *omsglen=0;
      st->stage = ATTKE_L_ERROR;
      return SGX_MPC_ERROR;
    }

    randombytes(eph_sk, SGX_MPC_AKE_SCALARBYTES);

    // Generate ephemeral public key
    eph_pk = (bytes) malloc(SGX_MPC_AKE_KEYBYTES);
    if(!eph_pk)
    { *omsg=NULL;
      *omsglen=0;
      byte_zero(eph_sk, SGX_MPC_AKE_SCALARBYTES);  
      free(eph_sk);
      st->stage = ATTKE_L_ERROR;
      return SGX_MPC_ERROR;
    }

    crypto_scalarmult_base(eph_pk, eph_sk);

    // Finish DH key agreement
    res = crypto_scalarmult(st->key, eph_sk, inmsg);
    if(res != 0)
    { *omsg=NULL;
      *omsglen=0;
      byte_zero(eph_sk, SGX_MPC_AKE_SCALARBYTES);  
      free(eph_sk);
      byte_zero(eph_pk, SGX_MPC_AKE_KEYBYTES);  
      free(eph_pk);
      st->stage = ATTKE_L_ERROR;
      return SGX_MPC_ERROR;
    }

    // allocate space for output message = inmsg || ephpk and sign
    mtosign = (bytes) malloc(inmsglen+SGX_MPC_AKE_KEYBYTES);
    if(!mtosign)
    { *omsg=NULL;
      *omsglen=0;
      byte_zero(eph_sk, SGX_MPC_AKE_SCALARBYTES);  
      free(eph_sk);
      byte_zero(eph_pk, SGX_MPC_AKE_KEYBYTES);  
      free(eph_pk);
      st->stage = ATTKE_L_ERROR;
      return SGX_MPC_ERROR;
    }

    byte_copy(mtosign, inmsglen, inmsg);
    byte_copy(mtosign + inmsglen, SGX_MPC_AKE_KEYBYTES, eph_pk);

    *omsg = (byte*) malloc(inmsglen + SGX_MPC_AKE_KEYBYTES + SGX_MPC_SIGEXP_BYTES);
    if(!*omsg)
    { *omsglen=0;
      byte_zero(eph_sk,SGX_MPC_AKE_SCALARBYTES);  
      free(eph_sk);
      byte_zero(eph_pk,SGX_MPC_AKE_KEYBYTES);  
      free(eph_pk);
      byte_zero(mtosign,inmsglen+SGX_MPC_AKE_KEYBYTES);
      free(mtosign);
      st->stage = ATTKE_L_ERROR;
      return SGX_MPC_ERROR;
    }

    res = crypto_sign(*omsg, omsglen, mtosign, inmsglen + SGX_MPC_AKE_KEYBYTES, st->sigsk);

    // cleanup
    byte_zero(eph_sk,SGX_MPC_AKE_SCALARBYTES);  
    free(eph_sk);
    byte_zero(eph_pk,SGX_MPC_AKE_KEYBYTES);  
    free(eph_pk);
    byte_zero(mtosign,inmsglen+SGX_MPC_AKE_KEYBYTES);
    free(mtosign);

    // update stage
    st->stage = ATTKE_L_SENT_EPHPK_DERIVED;
    return SGX_MPC_OK;

  } // end (st->stage == ATTKE_L_INITIALIZED)
  else
  if(st->stage == ATTKE_L_SENT_EPHPK_DERIVED)
  {
    // Second invokation, expecting OK || nonce 
    if(! (inmsglen == 3+16 && inmsg[0]=='O' && inmsg[1]=='K' && inmsg[2]=='\0') )
    { *omsg=NULL;
      *omsglen=0;
      st->stage = ATTKE_L_ERROR;
      return SGX_MPC_ERROR;
    }

    // check if it is the same nonce
    res = crypto_verify_16(st->nonce, inmsg+3);
    if(res!=0)
    { *omsg=NULL;
      *omsglen=0;
      st->stage = ATTKE_L_ERROR;
      return SGX_MPC_ERROR;
    }

    st->stage = ATTKE_L_GOTOK_ACCEPT; 
    *omsglen=0;
    return SGX_MPC_OK;
  
  } // end (st->stage == ATTKE_L_SENT_EPHPK_DERIVED)
  else
  { *omsg=NULL;
    *omsglen=0;
    st->stage = ATTKE_L_ERROR;
    return SGX_MPC_ERROR;
  }

}
