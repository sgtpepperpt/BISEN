#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_sec_mpc_program_t {
	int ms_retval;
	byte** ms_omsg;
	size* ms_omsglen;
	label ms_l;
	byte* ms_inmsg;
	size ms_inmsglen;
} ms_sec_mpc_program_t;

typedef struct ms_tmp_get_report_t {
	sgx_target_info_t* ms_tInfo;
	sgx_report_t* ms_report;
} ms_tmp_get_report_t;

typedef struct ms_check_initialization_t {
	int* ms_check_val;
} ms_check_initialization_t;

typedef struct ms_fserver_t {
	bytes* ms_out;
	size* ms_outlen;
	unsigned char* ms_in;
	size ms_inlen;
} ms_fserver_t;

typedef struct ms_untrusted_malloc_bytes_t {
	bytes* ms_pointer;
	size ms_length;
} ms_untrusted_malloc_bytes_t;

typedef struct ms_untrusted_free_bytes_t {
	bytes* ms_pointer;
} ms_untrusted_free_bytes_t;

typedef struct ms_get_qe_tInfo_t {
	int ms_retval;
	sgx_target_info_t* ms_tInfo;
} ms_get_qe_tInfo_t;

typedef struct ms_print_msg_t {
	char* ms_msg;
} ms_print_msg_t;

typedef struct ms_print_int_t {
	int ms_val;
} ms_print_int_t;

static sgx_status_t SGX_CDECL Enclave_fserver(void* pms)
{
	ms_fserver_t* ms = SGX_CAST(ms_fserver_t*, pms);
	fserver(ms->ms_out, ms->ms_outlen, ms->ms_in, ms->ms_inlen);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_untrusted_malloc_bytes(void* pms)
{
	ms_untrusted_malloc_bytes_t* ms = SGX_CAST(ms_untrusted_malloc_bytes_t*, pms);
	untrusted_malloc_bytes(ms->ms_pointer, ms->ms_length);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_untrusted_free_bytes(void* pms)
{
	ms_untrusted_free_bytes_t* ms = SGX_CAST(ms_untrusted_free_bytes_t*, pms);
	untrusted_free_bytes(ms->ms_pointer);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_get_qe_tInfo(void* pms)
{
	ms_get_qe_tInfo_t* ms = SGX_CAST(ms_get_qe_tInfo_t*, pms);
	ms->ms_retval = get_qe_tInfo(ms->ms_tInfo);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_print_msg(void* pms)
{
	ms_print_msg_t* ms = SGX_CAST(ms_print_msg_t*, pms);
	print_msg((const char*)ms->ms_msg);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_print_int(void* pms)
{
	ms_print_int_t* ms = SGX_CAST(ms_print_int_t*, pms);
	print_int(ms->ms_val);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[6];
} ocall_table_Enclave = {
	6,
	{
		(void*)Enclave_fserver,
		(void*)Enclave_untrusted_malloc_bytes,
		(void*)Enclave_untrusted_free_bytes,
		(void*)Enclave_get_qe_tInfo,
		(void*)Enclave_print_msg,
		(void*)Enclave_print_int,
	}
};
sgx_status_t sec_mpc_program(sgx_enclave_id_t eid, int* retval, byte** omsg, size* omsglen, label l, const byte* inmsg, size inmsglen)
{
	sgx_status_t status;
	ms_sec_mpc_program_t ms;
	ms.ms_omsg = omsg;
	ms.ms_omsglen = omsglen;
	ms.ms_l = l;
	ms.ms_inmsg = (byte*)inmsg;
	ms.ms_inmsglen = inmsglen;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t tmp_get_report(sgx_enclave_id_t eid, sgx_target_info_t* tInfo, sgx_report_t* report)
{
	sgx_status_t status;
	ms_tmp_get_report_t ms;
	ms.ms_tInfo = tInfo;
	ms.ms_report = report;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t check_initialization(sgx_enclave_id_t eid, int* check_val)
{
	sgx_status_t status;
	ms_check_initialization_t ms;
	ms.ms_check_val = check_val;
	status = sgx_ecall(eid, 2, &ocall_table_Enclave, &ms);
	return status;
}

