#include <stdlib.h>

#include "attke_remote.h"
#include "crypto.h"

#ifndef SGX_MPC_OUTSIDE
 #include "Enclave_t.h" 
#endif


int attke_remote_init(
  attke_remote_state *st
)
{
  // TODO : CHECKME : exactly the same code 
  //                  st->nonce size should be in a MACRO not hardcoded
  #ifdef SGX_MPC_OUTSIDE    
   // generate nonce
   randombytes(st->nonce, 16);
   // generate ephemeral secret key
   randombytes(st->eph_sk, SGX_MPC_AKE_SCALARBYTES);

  #else
    randombytes(st->nonce, 16);
    randombytes(st->eph_sk, SGX_MPC_AKE_SCALARBYTES);
  #endif

  // Generate ephemeral public key
  crypto_scalarmult_base(st->eph_pk, st->eph_sk);
  st->stage = ATTKE_R_INIT;
  return SGX_MPC_OK;
}


/* Allocates output memory. This is originally expecting to be
   invoked on an empry message and replies with a message of size 
   SGX_MPC_AKE_KEYBYTES+16, which includes server-side ephemeral 
   key and nonce. It then expects to receive the clien't ephemeral
   key, together with an echo of the first message, all signed.
   The second input is therefore expected to have size 16 + 2* + 
   SGX_MPC_AKE_KEYBYTES+SGX_MPC_SIGEXP_BYTES. If all goes well,
   it answers with "OK"||nonce (19 bytes) and accepts. */

int attke_remote(
  bytes *omsg,
  size *omsglen,
  attke_remote_state *st,
  const bytes inmsg,
  const size inmsglen,
  const bytes sigpk
)
{
  bytes smsg;
  size smsglen;
  int res;

  if(st->stage == ATTKE_R_INIT)
  { 
    // Remote is first invoked on empty message
    if(inmsglen != 0)
    { *omsg=NULL;
      *omsglen=0;
      st->stage = ATTKE_R_ERROR;
      return SGX_MPC_ERROR;
    }

    // allocate space for output message and copy over
    *omsg =(bytes) malloc(SGX_MPC_AKE_KEYBYTES + 16);
    if(! *omsg)
    { *omsglen=0;
      st->stage = ATTKE_R_ERROR;
      return SGX_MPC_ERROR;
    }

    byte_copy(*omsg, SGX_MPC_AKE_KEYBYTES, st->eph_pk);
    byte_copy(*omsg+SGX_MPC_AKE_KEYBYTES, 16, st->nonce);
    *omsglen = SGX_MPC_AKE_KEYBYTES+16;
    
    // update stage
    st->stage = ATTKE_R_SENT_EPHPK_WAIT;
    return SGX_MPC_OK;

  }
  else
  if(st->stage == ATTKE_R_SENT_EPHPK_WAIT)
  {
    // Second invokation, expecting signed ephemeral public key || previously sent message
    if(! (inmsglen >= SGX_MPC_SIGEXP_BYTES + 2*SGX_MPC_AKE_KEYBYTES + 16 && 
          inmsglen <= SGX_MPC_SIGEXP_BYTES + 2*SGX_MPC_AKE_KEYBYTES + 16 + SGX_MPC_SIGEXP_BYTES)
      )
    { *omsg=NULL;
      *omsglen=0;
      st->stage = ATTKE_R_ERROR;
      return SGX_MPC_ERROR;
    }

    // Nacl/Sodium signature syntax is message recovering
    // TODO: CHECKME : inmsglen - crypto_sign_BYTES should be enough
    smsg = (bytes) malloc(inmsglen); // need to provide as much space as signed message length
    if(!smsg)
    { *omsg=NULL;
      *omsglen=0;
      st->stage = ATTKE_R_ERROR;
      return SGX_MPC_ERROR;
    }

    res = crypto_sign_open(smsg,&smsglen,inmsg,inmsglen,sigpk); 
    // todo verify trace sent msg || received msg
    if(! (res == 0 && smsglen == 2*SGX_MPC_AKE_KEYBYTES+16) )
    { *omsg=NULL;
      *omsglen=0;
      byte_zero(smsg, inmsglen);  
      free(smsg);
      st->stage = ATTKE_R_ERROR;
      return SGX_MPC_ERROR;
    }

    res = crypto_verify_32(st->eph_pk, smsg);
    res |= crypto_verify_16(st->nonce, smsg+SGX_MPC_AKE_KEYBYTES);
    if(res != 0)
    { *omsg=NULL;
      *omsglen=0;
      byte_zero(smsg, inmsglen);  
      free(smsg);
      st->stage = ATTKE_R_ERROR;
      return SGX_MPC_ERROR;
    }

    // Finish DH key agreement and compute key
    res = crypto_scalarmult(st->key, st->eph_sk, smsg+SGX_MPC_AKE_KEYBYTES+16);
    if(res != 0)
    { *omsg=NULL;
      *omsglen=0;
      byte_zero(smsg, inmsglen);  
      free(smsg);
      st->stage = ATTKE_R_ERROR;
      return SGX_MPC_ERROR;
    }

    // Send OK message
    *omsg = (bytes) malloc(3+16);
    if(!*omsg)
    { *omsglen=0;
      byte_zero(smsg,inmsglen);  
      free(smsg);
      st->stage = ATTKE_R_ERROR;
      return SGX_MPC_ERROR;
    }

    (*omsg)[0]='O';
    (*omsg)[1]='K';
    (*omsg)[2]='\0';
    byte_copy((*omsg)+3, 16, st->nonce);        
    *omsglen = 3+16;

    // cleanup
    byte_zero(smsg,inmsglen);  
    free(smsg);

    // Update stage
    st->stage = ATTKE_R_SENTOK_ACCEPT; 
    return SGX_MPC_OK;

  } // end (st->stage == ATTKE_R_SENT_EPHPK_WAIT)
  else
  { *omsg=NULL;
    *omsglen=0;
    st->stage = ATTKE_R_ERROR;
    return SGX_MPC_ERROR;
  }
}
