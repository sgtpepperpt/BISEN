#include "mach_iee.h"
#include "crypto.h"

#ifndef	SGX_MPC_OUTSIDE
#include "sgx_utils.h"	// sgx_create_report
#include "Enclave_t.h"
#endif


/* Fills the report which should be SGX_MPC_MACH_REPLEN */
int iee_report(bytes report,const bytes imsg,const size imsglen) {
#ifdef	SGX_MPC_OUTSIDE
	/* there's no MAC */
	return SGX_MPC_OK;
#else
	/* compute a hash of imsg */
	byte hash[64];
	crypto_hash_sha512(hash,imsg,imsglen);

	/* cal sgx_create_report on hash, which is the correct size */

	sgx_status_t 		sgx_rv = SGX_SUCCESS;
	sgx_target_info_t 	tInfo;
	int res = 0;

	get_qe_tInfo(&res, &tInfo);
		
	sgx_rv = sgx_create_report(
		(const sgx_target_info_t *)&tInfo,
		(const sgx_report_data_t *)hash,
		(sgx_report_t *)report);
			
	return (sgx_rv == SGX_SUCCESS && res == 0) ? 
		SGX_MPC_OK :  
		SGX_MPC_ERROR;
	
#endif
}

