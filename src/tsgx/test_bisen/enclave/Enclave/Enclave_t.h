#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "sgx_report.h"
#include "sgx_mpc.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif


int sec_mpc_program(byte** omsg, size* omsglen, label l, const byte* inmsg, size inmsglen);
void tmp_get_report(sgx_target_info_t* tInfo, sgx_report_t* report);
void check_initialization(int* check_val);

sgx_status_t SGX_CDECL fserver(bytes* out, size* outlen, unsigned char* in, size inlen);
sgx_status_t SGX_CDECL untrusted_malloc_bytes(bytes* pointer, size length);
sgx_status_t SGX_CDECL untrusted_free_bytes(bytes* pointer);
sgx_status_t SGX_CDECL get_qe_tInfo(int* retval, sgx_target_info_t* tInfo);
sgx_status_t SGX_CDECL print_msg(const char* msg);
sgx_status_t SGX_CDECL print_int(int val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
