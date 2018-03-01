#include <stdlib.h>

#include "mach.h"
#include "lac_attest.h"
#include "crypto.h"

extern int SGX_MPC_MACH_SIGLEN; //

/* allocates output buffer that will be passed over to the enclave
   for filling, so it needs to get the expected size of the output.
   the expected output length should _not_ include signature or 
   report overhead, but on exit it will include the length of the 
   complete buffer, which will include the quoting signature in
   case of attested outputs. In the case of non-attested outputs
   it will be unchanged (it should therefore include the aead
   expansion factor SGX_MPC_AEAD_EXPBYTES). */

int lac_attest(
  bytes *omsg,
  size *omsglen,
  const void *handle,
  const label l,
  const bytes imsg,
  const size imsglen
)
{
  bytes msgrep;
  size msgreplen;
  int res;

  // init out arguments
  *omsg = NULL;
  *omsglen = 0;
  byte lstage = 0x80 & l;

  if(lstage == STAGE_ATT) // STAGE_ATT is defined in lac.h as 0x00
  {
    // we assume that attested outputs are msg || rep encoded
    // we must get *omsglen as the expected size of the (non-attested) output
    msgreplen = 0;
    msgrep = NULL;

    if( mach_run(&msgrep, &msgreplen, handle, l, imsg, imsglen) != SGX_MPC_OK )
    { *omsg = NULL;
      *omsglen = 0;
      return SGX_MPC_ERROR;
    }

    // ensures that quoted outputs are msg || sig encoded
    // ensures fresh memory so repmsg can be freed
    *omsglen = (msgreplen-SGX_MPC_MACH_REPLEN) + SGX_MPC_MACH_SIGLEN;
    *omsg = (bytes) malloc(*omsglen);
    if(*omsg == NULL)
    { *omsglen = 0;
      byte_zero(msgrep,msgreplen);
      free(msgrep);
      return SGX_MPC_ERROR;
    }

    res = mach_quote(*omsg, *omsglen, msgrep, msgreplen);
    byte_zero(msgrep, msgreplen);
    free(msgrep);
    return res;
  }
  else
  {
    if( mach_run(omsg, omsglen, handle, l, imsg, imsglen) != SGX_MPC_OK )
    { return SGX_MPC_ERROR;
    }
    return SGX_MPC_OK;
  }
#if 0
  bytes msgrep;
  size msgreplen;
  int res;

  byte lstage = 0x80 & l;

  if(lstage == STAGE_ATT) // STAGE_ATT is defined in lac.h as 0x00
  {
    // we assume that attested outputs are msg || rep encoded
    // we must get *omsglen as the expected size of the (non-attested) output
    msgreplen = *omsglen + SGX_MPC_MACH_REPLEN;
    msgrep = (bytes) malloc(msgreplen);

    if(!msgrep)
    { *omsg = NULL;
      *omsglen = 0;
      return SGX_MPC_ERROR;
    }

    if( mach_run(msgrep, msgreplen, handle, l, imsg, imsglen) != SGX_MPC_OK )
    { *omsg=NULL;
      *omsglen=0;
      byte_zero(msgrep, msgreplen);
      free(msgrep);
      return SGX_MPC_ERROR;
    }

    // ensures that quoted outputs are msg || sig encoded
    // ensures fresh memory so repmsg can be freed
    *omsg = (bytes) malloc(*omsglen + SGX_MPC_MACH_SIGLEN);
    *omsglen = *omsglen + SGX_MPC_MACH_SIGLEN;
    if (!*omsg) {
      *omsglen = 0;
      byte_zero(msgrep,*omsglen + SGX_MPC_MACH_REPLEN); // TODO: ?? msgreplen ??
      free(msgrep);
      return SGX_MPC_ERROR;
    }

    res = mach_quote(*omsg, *omsglen, msgrep, msgreplen);
    byte_zero(msgrep,msgreplen);
    free(msgrep);

    return res;
  }
  else
  {
    *omsg = (bytes)malloc(*omsglen);
    if (!*omsg)
    { *omsglen=0;
      return SGX_MPC_ERROR;
    }

    if( mach_run(*omsg, *omsglen, handle, l, imsg, imsglen) != SGX_MPC_OK )
    { byte_zero(*omsg, *omsglen);
      free(*omsg);
      return SGX_MPC_ERROR;
    }
    return SGX_MPC_OK;
  }
#endif
}
