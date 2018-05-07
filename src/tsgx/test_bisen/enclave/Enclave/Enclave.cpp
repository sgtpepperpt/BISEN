#include <string>
#include <cstring>
#include <stdarg.h>
#include <stdio.h> // vsnprintf
#include <vector>
#include <stdlib.h>

#include "sgx_trts.h"
#include "Enclave_t.h" // print_string
#include "sgx_utils.h" // report
#include "sgx_report.h" // sgx_report_t
#include "sgx_uae_service.h" // sgx_get_quote
#include "sgx_report.h"

extern "C"
{
#include "mpc.h"
#include "attke_par.h"
#include "box.h"
#include "mach_iee.h"
#include "crypto.h"
}

using namespace std;

#define SGX_MPC_MACH_REPLEN sizeof(sgx_report_t)
#define STAGE_P 0x00
#define STAGE_Q 0x80


byte seq_stage;
bytes trace[SGX_MPC_NPARTIES];
size tracelen[SGX_MPC_NPARTIES];
int e_was_run_before = 0;

#include "../../f/public_key.h"

void tmp_get_report(
        sgx_target_info_t *tInfo,
        sgx_report_t *report
) {
    byte hash[64];

    // TODO : CHECKME : sgx_rv is not used
    sgx_status_t sgx_rv = SGX_SUCCESS;

    sgx_rv = sgx_create_report(
            tInfo,
            (const sgx_report_data_t *) hash,
            report
    );
}


// to make sure that the enclave is loaded
void check_initialization(int *check_val) {
    *check_val = 99;
}


// to be run on load
int mpc_program_init(void) {
    int i;
    attke_par_init();
    box_init();
    seq_stage = MPC_P_RUNNING_P;
    for (i = 0; i < SGX_MPC_NPARTIES; i++) {
        trace[i] = NULL;
        tracelen[i] = 0;
    }
    return SGX_MPC_OK;
}


int extend_trace(
        const int i,
        const bytes inmsg,
        const size inmsglen) {
    bytes newtrace;

    if (inmsglen == 0) { return SGX_MPC_OK; }

    newtrace = (bytes)
    malloc(tracelen[i] + inmsglen);
    if (!newtrace) { return SGX_MPC_ERROR; }

    byte_copy(newtrace, inmsglen, inmsg);

    if (tracelen[i] > 0) {
        byte_copy(newtrace + inmsglen, tracelen[i], trace[i]);
        free(trace[i]);
    }

    trace[i] = newtrace;
    tracelen[i] += inmsglen;

    return SGX_MPC_OK;
}

int sec_mpc_program(
        byte **omsg,
        size *omsglen,
        label l,
        const byte *inmsg,
        size inmsglen
) {
    bytes kemsg;
    size kemsglen;
    bytes boxoutput;
    size boxoutputlen;
    byte report[SGX_MPC_MACH_REPLEN];
    byte lstage = 0x80 & l;
    byte pid = 0x7f & l;
    int res;
    byte par_stg;

    // init out arguments
    *omsg = NULL;
    *omsglen = 0;

    if (!e_was_run_before) {
        mpc_program_init();
        e_was_run_before = 1;
    }

    if (!(pid >= 1 && pid <= SGX_MPC_NPARTIES)) {
        seq_stage = MPC_P_ERROR;
        return SGX_MPC_ERROR;
    }

    if (seq_stage == MPC_P_RUNNING_P) {
        if (lstage != STAGE_P) {
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        attke_par(&kemsg, &kemsglen, pid,(const bytes)inmsg, inmsglen);

        par_stg = attke_par_getstage();
        if (par_stg == ATTKE_P_ERROR) {
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        res = extend_trace(pid - 1,(const bytes)inmsg, inmsglen);
        if (res != SGX_MPC_OK) {
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        res = extend_trace(pid - 1, kemsg, kemsglen);
        if (res != SGX_MPC_OK) {
            byte_zero(kemsg, kemsglen);
            free(kemsg);
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        res = iee_report(report, trace[pid - 1], tracelen[pid - 1]);
        if (res != SGX_MPC_OK) {
            byte_zero(kemsg, kemsglen);
            free(kemsg);
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        untrusted_malloc_bytes(omsg, kemsglen + SGX_MPC_MACH_REPLEN);
        if (*omsg == NULL) {
            byte_zero(kemsg, kemsglen);
            free(kemsg);
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }
        *omsglen = kemsglen + SGX_MPC_MACH_REPLEN;

#if 0
        if( omsglen != kemsglen + SGX_MPC_MACH_REPLEN)
        { // TODO : CHECKME : there was a note here regarding the second quote in
          //                  commit fab36aa8bbe2e983c47e9d1decc33b597f35e880
          byte_zero(kemsg, kemsglen);
          free(kemsg);
          seq_stage = MPC_P_ERROR;
          return SGX_MPC_ERROR;
        }
#endif

        // ensure msg || rep encoding
        byte_copy(*omsg, kemsglen, kemsg);
        byte_copy((*omsg) + kemsglen, SGX_MPC_MACH_REPLEN, report);
        byte_zero(kemsg, kemsglen);
        free(kemsg);
        if (par_stg == ATTKE_P_FINISHED) { seq_stage = MPC_P_RUNNING_Q; }

        return SGX_MPC_OK;

    } // end (seq_stage == MPC_P_RUNNING_P)
    else if (seq_stage == MPC_P_RUNNING_Q) {
        if (lstage != STAGE_Q) {
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        // TODO : implement variable length out - on branch tsgx-with-variable-len-out
        res = box(&boxoutput, &boxoutputlen, pid,(const bytes)inmsg, inmsglen);

        if (res != SGX_MPC_OK) {
            free(boxoutput);
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        untrusted_malloc_bytes(omsg, boxoutputlen);
        if (*omsg == NULL) {
            free(boxoutput);
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }
        *omsglen = boxoutputlen;
        byte_copy(*omsg, boxoutputlen, boxoutput);
        byte_zero(boxoutput, boxoutputlen);
        free(boxoutput);

#if 0
        if(! (res == SGX_MPC_OK && boxoutputlen == omsglen) )
        { byte_zero(boxoutput, boxoutputlen);
          free(boxoutput);
          seq_stage = MPC_P_ERROR;
          return SGX_MPC_ERROR;
        }
        byte_copy(omsg, boxoutputlen, boxoutput);
        // TODO : was byte_zero and free missing here?
#endif

        return SGX_MPC_OK;
    } else {
        seq_stage = MPC_P_ERROR;
        return SGX_MPC_ERROR;
    }
}
