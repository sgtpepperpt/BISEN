#include <stdlib.h>

#include "lac_verify.h"
#include "mach.h"
#include "crypto.h"

static bytes trace_local;
static size tracelen_local;
static bytes code;
static size codelen;
static bytes verkey;
static label pid; // we only use one label

extern int SGX_MPC_MACH_SIGLEN;


// assumes code and verkey refs can be kept
int lac_verify_init(
    int sock,
  const bytes cde,
  const size cdelen,
  const bytes prms,
  const label partyid
)
{
  trace_local = NULL;
  tracelen_local = 0;
  code = cde;
  codelen = cdelen;
  verkey = prms;

  if(! (partyid >= 1 && partyid <= SGX_MPC_NPARTIES) )
  { return SGX_MPC_ERROR; }

  pid = partyid;

  return SGX_MPC_OK;
}


int extend_trace_local(
  const bytes inmsg,
  const size inmsglen
)
{
  bytes newtrace;

  if(inmsglen == 0)
  { return  SGX_MPC_OK; }

  newtrace = (bytes) malloc(tracelen_local + inmsglen);
  if(!newtrace)
  { return SGX_MPC_ERROR; }

  byte_copy(newtrace, inmsglen, inmsg);

  if(tracelen_local > 0)
  { byte_copy(newtrace + inmsglen, tracelen_local, trace_local);
    free(trace_local);
  }

  trace_local = newtrace;
  tracelen_local += inmsglen;

  return SGX_MPC_OK;
}

// Gets an attested output and returns the actual output (stripping the quote)
// if all goes well. Contracts SGX_MPC_MACH_SIGLEN.
int lac_verify(
    int sock,
    bytes *omsg,
    size *omsglen,
    const label l,
    const bytes in,
    const size inlen,
    const bytes out,
    const size outlen
)
{
  int res;

  byte lstage = 0x80 & l;
  byte mypid = 0x7f & l;

  if(! (pid == mypid) )
  { *omsg = NULL;
    *omsglen = 0;
    return SGX_MPC_ERROR;
  }

  if(lstage == STAGE_ATT)
  {
    res = extend_trace_local(in, inlen);
    if(res != SGX_MPC_OK)
    { *omsg = NULL;
      *omsglen = 0;
      return SGX_MPC_ERROR;
    }

    res = extend_trace_local(out, outlen);
    if (res != SGX_MPC_OK) {
      *omsg = NULL;
      *omsglen = 0;
      return SGX_MPC_ERROR;
    }

    res = remote_mach_verify(sock, trace_local, tracelen_local, code, codelen);
    if(res != SGX_MPC_OK)
    { *omsg = NULL;
      *omsglen = 0;
      return SGX_MPC_ERROR;
    }

    *omsg = (bytes) malloc(outlen - SGX_MPC_MACH_SIGLEN);
    if(!*omsg)
    { *omsglen = 0;
      return SGX_MPC_ERROR;
    }

    // Assume message || signature encoding
    byte_copy(*omsg, outlen - SGX_MPC_MACH_SIGLEN, out);
    *omsglen = outlen - SGX_MPC_MACH_SIGLEN;

    return SGX_MPC_OK;

  } // end (lstage == STAGE_ATT)
  else
  { *omsg=out;
    *omsglen=outlen;
    return SGX_MPC_OK;
  }
}
