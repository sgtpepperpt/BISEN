#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_satus_t etc. */

#include "sgx_report.h"
#include "sgx_mpc.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void SGX_UBRIDGE(SGX_NOCONVENTION, fserver, (bytes* out, size* outlen, unsigned char* in, size inlen));
void SGX_UBRIDGE(SGX_NOCONVENTION, untrusted_malloc_bytes, (bytes* pointer, size length));
int SGX_UBRIDGE(SGX_NOCONVENTION, get_qe_tInfo, (sgx_target_info_t* tInfo));
void SGX_UBRIDGE(SGX_NOCONVENTION, print_msg, (const char* msg));
void SGX_UBRIDGE(SGX_NOCONVENTION, print_int, (int val));

sgx_status_t sec_mpc_program(sgx_enclave_id_t eid, int* retval, byte** omsg, size* omsglen, label l, const byte* inmsg, size inmsglen);
sgx_status_t tmp_get_report(sgx_enclave_id_t eid, sgx_target_info_t* tInfo, sgx_report_t* report);
sgx_status_t check_initialization(sgx_enclave_id_t eid, int* check_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
