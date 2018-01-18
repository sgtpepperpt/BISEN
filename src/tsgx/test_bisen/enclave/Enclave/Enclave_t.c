#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */

#include <errno.h>
#include <string.h> /* for memcpy etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)


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

static sgx_status_t SGX_CDECL sgx_sec_mpc_program(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_sec_mpc_program_t));
	ms_sec_mpc_program_t* ms = SGX_CAST(ms_sec_mpc_program_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	byte** _tmp_omsg = ms->ms_omsg;
	size_t _len_omsg = sizeof(*_tmp_omsg);
	byte** _in_omsg = NULL;
	size* _tmp_omsglen = ms->ms_omsglen;
	size_t _len_omsglen = sizeof(*_tmp_omsglen);
	size* _in_omsglen = NULL;
	byte* _tmp_inmsg = ms->ms_inmsg;
	size _tmp_inmsglen = ms->ms_inmsglen;
	size_t _len_inmsg = _tmp_inmsglen * sizeof(*_tmp_inmsg);
	byte* _in_inmsg = NULL;

	if (sizeof(*_tmp_inmsg) != 0 &&
		(size_t)_tmp_inmsglen > (SIZE_MAX / sizeof(*_tmp_inmsg))) {
		status = SGX_ERROR_INVALID_PARAMETER;
		goto err;
	}

	CHECK_UNIQUE_POINTER(_tmp_omsg, _len_omsg);
	CHECK_UNIQUE_POINTER(_tmp_omsglen, _len_omsglen);
	CHECK_UNIQUE_POINTER(_tmp_inmsg, _len_inmsg);

	if (_tmp_omsg != NULL && _len_omsg != 0) {
		if ((_in_omsg = (byte**)malloc(_len_omsg)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_omsg, 0, _len_omsg);
	}
	if (_tmp_omsglen != NULL && _len_omsglen != 0) {
		if ((_in_omsglen = (size*)malloc(_len_omsglen)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_omsglen, 0, _len_omsglen);
	}
	if (_tmp_inmsg != NULL && _len_inmsg != 0) {
		_in_inmsg = (byte*)malloc(_len_inmsg);
		if (_in_inmsg == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy((void*)_in_inmsg, _tmp_inmsg, _len_inmsg);
	}
	ms->ms_retval = sec_mpc_program(_in_omsg, _in_omsglen, ms->ms_l, (const byte*)_in_inmsg, _tmp_inmsglen);
err:
	if (_in_omsg) {
		memcpy(_tmp_omsg, _in_omsg, _len_omsg);
		free(_in_omsg);
	}
	if (_in_omsglen) {
		memcpy(_tmp_omsglen, _in_omsglen, _len_omsglen);
		free(_in_omsglen);
	}
	if (_in_inmsg) free((void*)_in_inmsg);

	return status;
}

static sgx_status_t SGX_CDECL sgx_tmp_get_report(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_tmp_get_report_t));
	ms_tmp_get_report_t* ms = SGX_CAST(ms_tmp_get_report_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	sgx_target_info_t* _tmp_tInfo = ms->ms_tInfo;
	size_t _len_tInfo = sizeof(*_tmp_tInfo);
	sgx_target_info_t* _in_tInfo = NULL;
	sgx_report_t* _tmp_report = ms->ms_report;
	size_t _len_report = sizeof(*_tmp_report);
	sgx_report_t* _in_report = NULL;

	CHECK_UNIQUE_POINTER(_tmp_tInfo, _len_tInfo);
	CHECK_UNIQUE_POINTER(_tmp_report, _len_report);

	if (_tmp_tInfo != NULL && _len_tInfo != 0) {
		_in_tInfo = (sgx_target_info_t*)malloc(_len_tInfo);
		if (_in_tInfo == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_tInfo, _tmp_tInfo, _len_tInfo);
	}
	if (_tmp_report != NULL && _len_report != 0) {
		if ((_in_report = (sgx_report_t*)malloc(_len_report)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_report, 0, _len_report);
	}
	tmp_get_report(_in_tInfo, _in_report);
err:
	if (_in_tInfo) free(_in_tInfo);
	if (_in_report) {
		memcpy(_tmp_report, _in_report, _len_report);
		free(_in_report);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_check_initialization(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_check_initialization_t));
	ms_check_initialization_t* ms = SGX_CAST(ms_check_initialization_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	int* _tmp_check_val = ms->ms_check_val;
	size_t _len_check_val = sizeof(*_tmp_check_val);
	int* _in_check_val = NULL;

	CHECK_UNIQUE_POINTER(_tmp_check_val, _len_check_val);

	if (_tmp_check_val != NULL && _len_check_val != 0) {
		if ((_in_check_val = (int*)malloc(_len_check_val)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_check_val, 0, _len_check_val);
	}
	check_initialization(_in_check_val);
err:
	if (_in_check_val) {
		memcpy(_tmp_check_val, _in_check_val, _len_check_val);
		free(_in_check_val);
	}

	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv;} ecall_table[3];
} g_ecall_table = {
	3,
	{
		{(void*)(uintptr_t)sgx_sec_mpc_program, 0},
		{(void*)(uintptr_t)sgx_tmp_get_report, 0},
		{(void*)(uintptr_t)sgx_check_initialization, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[6][3];
} g_dyn_entry_table = {
	6,
	{
		{0, 0, 0, },
		{0, 0, 0, },
		{0, 0, 0, },
		{0, 0, 0, },
		{0, 0, 0, },
		{0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL fserver(bytes* out, size* outlen, unsigned char* in, size inlen)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_out = sizeof(*out);
	size_t _len_outlen = sizeof(*outlen);
	size_t _len_in = inlen * sizeof(*in);

	ms_fserver_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_fserver_t);
	void *__tmp = NULL;

	ocalloc_size += (out != NULL && sgx_is_within_enclave(out, _len_out)) ? _len_out : 0;
	ocalloc_size += (outlen != NULL && sgx_is_within_enclave(outlen, _len_outlen)) ? _len_outlen : 0;
	ocalloc_size += (in != NULL && sgx_is_within_enclave(in, _len_in)) ? _len_in : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_fserver_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_fserver_t));

	if (out != NULL && sgx_is_within_enclave(out, _len_out)) {
		ms->ms_out = (bytes*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_out);
		memset(ms->ms_out, 0, _len_out);
	} else if (out == NULL) {
		ms->ms_out = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (outlen != NULL && sgx_is_within_enclave(outlen, _len_outlen)) {
		ms->ms_outlen = (size*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_outlen);
		memset(ms->ms_outlen, 0, _len_outlen);
	} else if (outlen == NULL) {
		ms->ms_outlen = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (in != NULL && sgx_is_within_enclave(in, _len_in)) {
		ms->ms_in = (unsigned char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_in);
		memcpy(ms->ms_in, in, _len_in);
	} else if (in == NULL) {
		ms->ms_in = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_inlen = inlen;
	status = sgx_ocall(0, ms);

	if (out) memcpy((void*)out, ms->ms_out, _len_out);
	if (outlen) memcpy((void*)outlen, ms->ms_outlen, _len_outlen);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL untrusted_malloc_bytes(bytes* pointer, size length)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pointer = sizeof(*pointer);

	ms_untrusted_malloc_bytes_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_untrusted_malloc_bytes_t);
	void *__tmp = NULL;

	ocalloc_size += (pointer != NULL && sgx_is_within_enclave(pointer, _len_pointer)) ? _len_pointer : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_untrusted_malloc_bytes_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_untrusted_malloc_bytes_t));

	if (pointer != NULL && sgx_is_within_enclave(pointer, _len_pointer)) {
		ms->ms_pointer = (bytes*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_pointer);
		memset(ms->ms_pointer, 0, _len_pointer);
	} else if (pointer == NULL) {
		ms->ms_pointer = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_length = length;
	status = sgx_ocall(1, ms);

	if (pointer) memcpy((void*)pointer, ms->ms_pointer, _len_pointer);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL untrusted_free_bytes(bytes* pointer)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pointer = sizeof(*pointer);

	ms_untrusted_free_bytes_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_untrusted_free_bytes_t);
	void *__tmp = NULL;

	ocalloc_size += (pointer != NULL && sgx_is_within_enclave(pointer, _len_pointer)) ? _len_pointer : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_untrusted_free_bytes_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_untrusted_free_bytes_t));

	if (pointer != NULL && sgx_is_within_enclave(pointer, _len_pointer)) {
		ms->ms_pointer = (bytes*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_pointer);
		memset(ms->ms_pointer, 0, _len_pointer);
	} else if (pointer == NULL) {
		ms->ms_pointer = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(2, ms);

	if (pointer) memcpy((void*)pointer, ms->ms_pointer, _len_pointer);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL get_qe_tInfo(int* retval, sgx_target_info_t* tInfo)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_tInfo = sizeof(*tInfo);

	ms_get_qe_tInfo_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_get_qe_tInfo_t);
	void *__tmp = NULL;

	ocalloc_size += (tInfo != NULL && sgx_is_within_enclave(tInfo, _len_tInfo)) ? _len_tInfo : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_get_qe_tInfo_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_get_qe_tInfo_t));

	if (tInfo != NULL && sgx_is_within_enclave(tInfo, _len_tInfo)) {
		ms->ms_tInfo = (sgx_target_info_t*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_tInfo);
		memset(ms->ms_tInfo, 0, _len_tInfo);
	} else if (tInfo == NULL) {
		ms->ms_tInfo = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(3, ms);

	if (retval) *retval = ms->ms_retval;
	if (tInfo) memcpy((void*)tInfo, ms->ms_tInfo, _len_tInfo);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL print_msg(const char* msg)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_msg = msg ? strlen(msg) + 1 : 0;

	ms_print_msg_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_print_msg_t);
	void *__tmp = NULL;

	ocalloc_size += (msg != NULL && sgx_is_within_enclave(msg, _len_msg)) ? _len_msg : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_print_msg_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_print_msg_t));

	if (msg != NULL && sgx_is_within_enclave(msg, _len_msg)) {
		ms->ms_msg = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_msg);
		memcpy((void*)ms->ms_msg, msg, _len_msg);
	} else if (msg == NULL) {
		ms->ms_msg = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(4, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL print_int(int val)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_print_int_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_print_int_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_print_int_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_print_int_t));

	ms->ms_val = val;
	status = sgx_ocall(5, ms);


	sgx_ocfree();
	return status;
}

