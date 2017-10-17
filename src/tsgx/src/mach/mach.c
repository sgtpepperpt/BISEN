#include <stdlib.h>
#include <stdio.h> // printf TODO remove
#include <string.h> // memset

#include "mach.h"
#include "mpc_program.h"
#include "crypto.h"

#ifdef  SGX_MPC_OUTSIDE
 #include "mpc_program.h"
#else
 #include "sgx_urts.h"
 #include "Enclave_u.h"
 #include "sgx_report.h" // sgx_report_t
 #include "sgx_uae_service.h" // sgx_get_quote
#endif


#ifndef  SGX_MPC_OUTSIDE
 static sgx_spid_t spid;
 static sgx_enclave_id_t tmp_eid;
#endif

int SGX_MPC_MACH_SIGLEN;

#ifndef  SGX_MPC_OUTSIDE
int get_qe_tInfo(sgx_target_info_t *tInfo)
{
  sgx_status_t ret_sts = SGX_SUCCESS;
  sgx_epid_group_id_t p_gid;

  ret_sts = sgx_init_quote(tInfo, &p_gid);

  if (ret_sts != SGX_SUCCESS)
  { return -1; }

  return 0;  
}
#endif

void print_msg (const char * msg)
{
  printf("\tEnclave [%s]\n", msg);
}

void print_int(int val)
{
  printf("\tEnclave [%d]\n", val);
}

int mach_load(void **handle, char *filename)
{
  #ifdef  SGX_MPC_OUTSIDE
   *handle = NULL; 
   SGX_MPC_MACH_SIGLEN = 13; 
   return SGX_MPC_OK;
 
  #else
   sgx_status_t ret;
   int  ret_v;
   int launch_token_update;
   sgx_enclave_id_t enclave_id;
   sgx_launch_token_t launch_token;

   ret = SGX_SUCCESS;
   launch_token_update = 0;
   enclave_id = 0;

   // initialize SGX_MPC_MACH_SIGLEN
   uint32_t quote_size = 0;
   // ret = sgx_get_quote_size(NULL, &quote_size); // this function is deprecated
   ret = sgx_calc_quote_size(NULL, 0, &quote_size);  

   if(ret != SGX_SUCCESS)
   { return SGX_MPC_ERROR; }

   SGX_MPC_MACH_SIGLEN = quote_size;
    
   // initialize enclave
   memset(&launch_token, 0, sizeof(sgx_launch_token_t));
    
   ret = sgx_create_enclave(
           filename,
           SGX_DEBUG_FLAG,
           &launch_token,
           &launch_token_update,
           &enclave_id, NULL
         );

   if(SGX_SUCCESS == ret)
   { ret = check_initialization(enclave_id, &ret_v);
     if(SGX_SUCCESS != ret)
     { printf("checking initialization failed\n");
       return SGX_MPC_ERROR;
     } 
     tmp_eid = enclave_id;
     *handle = (void*) malloc(sizeof(enclave_id));
     memcpy(*handle, &enclave_id, sizeof(enclave_id));
     ret_v = SGX_MPC_OK;
   }
   else
   { *handle = NULL;
     ret_v = SGX_MPC_ERROR;
   } 
   
   memcpy(&spid, sp_id_value, sizeof(sgx_spid_t)); 
   return ret_v;
  #endif
}

// Calls the enclave. omsglen should match what mpc_program
// is expecting to see. This should be guaraneeed by lac_attest

int mach_run(
  bytes *omsg, 
  size *omsglen,
  const void *handle,
  const label l,
  const bytes imsg,
  const size imsglen
)
{
  #ifdef  SGX_MPC_OUTSIDE
   // TODO : FIXME : define correct behavior for SGX_MPC_OUTSIDE
   #error SGX_MPC_OUTSIDE NOT SUPPORTED ATM
   //return mpc_program(omsg,omsglen,l,imsg,imsglen);
  #else
   sgx_enclave_id_t eid;
   sgx_status_t sgx_rv;
   int rv;
 
   memcpy(&eid, handle, sizeof(eid));
   sgx_rv = SGX_SUCCESS;
   rv = 0;
    
   sgx_rv = sec_mpc_program(eid, &rv, omsg, omsglen, l, imsg, imsglen);
   return sgx_rv == SGX_SUCCESS ? SGX_MPC_OK : SGX_MPC_ERROR;
  #endif
}


#ifndef SGX_MPC_OUTSIDE
void print_sgx_rv(sgx_status_t ret)
{
  switch (ret)
  {
    case SGX_SUCCESS :
      printf("SGX_SUCCESS\n");
      break;

    case SGX_ERROR_INVALID_ENCLAVE :
      printf("SGX_ERROR_INVALID_ENCLAVE\n");
      break;

    case SGX_ERROR_INVALID_PARAMETER :
      printf("SGX_ERROR_INVALID_PARAMETER\n");
      break;

    case SGX_ERROR_OUT_OF_MEMORY :
      printf("SGX_ERROR_OUT_OF_MEMORY\n");
      break;

    case SGX_ERROR_ENCLAVE_FILE_ACCESS :
      printf("SGX_ERROR_ENCLAVE_FILE_ACCESS\n");
      break;

    case SGX_ERROR_INVALID_METADATA :
      printf("SGX_ERROR_INVALID_METADATA\n");
      break;

    case SGX_ERROR_INVALID_VERSION :
      printf("SGX_ERROR_INVALID_VERSION\n");
      break;

    case SGX_ERROR_INVALID_SIGNATURE :
      printf("SGX_ERROR_INVALID_SIGNATURE\n");
      break;

    case SGX_ERROR_OUT_OF_EPC :
      printf("SGX_ERROR_OUT_OF_EPC\n");
      break;

    case SGX_ERROR_NO_DEVICE :
      printf("SGX_ERROR_NO_DEVICE\n");
      break;

    case SGX_ERROR_MEMORY_MAP_CONFLICT :
      printf("SGX_ERROR_MEMORY_MAP_CONFLICT\n");
      break;

    case SGX_ERROR_DEVICE_BUSY :
      printf("SGX_ERROR_DEVICE_BUSY\n");
      break;

    case SGX_ERROR_MODE_INCOMPATIBLE :
      printf("SGX_ERROR_MODE_INCOMPATIBLE\n");
      break;

    case SGX_ERROR_SERVICE_UNAVAILABLE :
      printf("SGX_ERROR_SERVICE_UNAVAILABLE\n");
      break;

    default:  
      printf("unknown SGX F\n");
  }
}
#endif


// Converts report into quote. contracts SGX_MPC_MACH_REPLEN and
// expands SGX_MPC_MACH_SIGLEN. omsglen and imsglen should be
// consistent with this.

int mach_quote(
  bytes omsg,
  size omsglen,
  const bytes imsg,
  const size imsglen
)
{
  #ifdef  SGX_MPC_OUTSIDE
   if(omsglen - SGX_MPC_MACH_SIGLEN != imsglen - SGX_MPC_MACH_REPLEN)
   { return SGX_MPC_ERROR; }

   byte_copy(omsg,imsglen-SGX_MPC_MACH_REPLEN,imsg);
   return SGX_MPC_OK;

  #else

   // will take whatever comes out of mach_run
   // should produce whatever is verified by mach_verify

   // TODO : CHECKME : some comments and code were present (and later removed)
   //                  at commit 281ae8e6df9e8314db582ce8eff30b350c17ddbe 

   sgx_target_info_t qeTInf;
   sgx_status_t sgx_rv;
   sgx_report_t sgx_report;
   sgx_quote_t *sgx_q;

   sgx_q = (sgx_quote_t *) malloc(SGX_MPC_MACH_SIGLEN);
   int r = get_qe_tInfo(&qeTInf);

   if (r != 0)
   { return SGX_MPC_ERROR; }

   sgx_rv = tmp_get_report(tmp_eid, &qeTInf, &sgx_report);

   sgx_q = (sgx_quote_t *) malloc(SGX_MPC_MACH_SIGLEN);

   sgx_rv = sgx_get_quote(
              &sgx_report,
              SGX_UNLINKABLE_SIGNATURE,
              &spid,
              NULL, // p_nonce
              NULL, // signatuer revoke list 
              0, // size of revoke list
              NULL, // sgx_report_t *p_qe_report,
              sgx_q,
              SGX_MPC_MACH_SIGLEN
            );
   free(sgx_q);

   // TODO : CHECKME : DOES NOT RETURN ON NON SUCCESS
   if (sgx_rv != SGX_SUCCESS)
   { printf("quote could not be generated"); }
   
   byte_copy(omsg,imsglen-SGX_MPC_MACH_REPLEN,imsg);
   return SGX_MPC_OK;
  #endif
}


int mach_verify(
  const bytes imsg,
  const size imsglen,
  const bytes code,
  const size codelen
)
{
  #ifdef  SGX_MPC_OUTSIDE
   return SGX_MPC_OK;
  #else

   // TODO: Contact Intel
   // for now : dummy hash computation to simulate Intel API call
   byte hash[64];
   crypto_hash_sha512(hash,imsg,imsglen);
   memcmp(hash, hash, 64);
   return SGX_MPC_OK;
  #endif
}

void mach_finalize(const void *handle)
{
  #ifndef  SGX_MPC_OUTSIDE
   sgx_enclave_id_t   eid;
   memcpy(&eid, handle, sizeof(eid));
   sgx_destroy_enclave(eid);
   printf("enclave destroyed\n");
  #endif
}

