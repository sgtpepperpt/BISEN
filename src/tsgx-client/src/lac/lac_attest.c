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
    int sock,
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

    if( remote_mach_run(sock, &msgrep, &msgreplen, handle, l, imsg, imsglen) != SGX_MPC_OK )
    { *omsg = NULL;
      *omsglen = 0;
      return SGX_MPC_ERROR;
    }

    // ensures that quoted outputs are msg || sig encoded
    // ensures fresh memory so repmsg can be freed
    *omsglen = (msgreplen-SGX_MPC_MACH_REPLEN) + SGX_MPC_MACH_SIGLEN;
    *omsg = (bytes) malloc(*omsglen);
    if(*omsg == NULL) {
        *omsglen = 0;
        byte_zero(msgrep,msgreplen);
        free(msgrep);
        return SGX_MPC_ERROR;
    }
    printf("will quote\n");
    res = remote_mach_quote(sock, *omsg, *omsglen, msgrep, msgreplen);
    printf("%d\n", res);
    byte_zero(msgrep, msgreplen);
    free(msgrep);
    return res;
  }
  else
  {
    if( remote_mach_run(sock, omsg, omsglen, handle, l, imsg, imsglen) != SGX_MPC_OK )
    { return SGX_MPC_ERROR;
    }
    return SGX_MPC_OK;
  }
}
