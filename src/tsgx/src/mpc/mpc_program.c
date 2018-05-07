#include <stdlib.h>
#include "mpc_program.h"
#include "attke_par.h"
#include "box.h"
#include "mach_iee.h"
#include "crypto.h"

#define STAGE_P 0x00
#define STAGE_Q 0x80

static byte seq_stage;
static bytes trace[SGX_MPC_NPARTIES];
static size tracelen[SGX_MPC_NPARTIES];
static int was_run_before = 0;

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
        const size inmsglen
) {
    bytes newtrace;

    if (inmsglen == 0) { return SGX_MPC_OK; }

    newtrace = (bytes) malloc(tracelen[i] + inmsglen);
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

int mpc_program(
        bytes omsg,
        size omsglen,
        const label l,
        const bytes inmsg,
        const size inmsglen
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

    if (!was_run_before) {
        mpc_program_init();
        was_run_before = 1;
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

        attke_par(&kemsg, &kemsglen, pid, inmsg, inmsglen);
        par_stg = attke_par_getstage();
        if (par_stg == ATTKE_P_ERROR) {
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        res = extend_trace(pid - 1, inmsg, inmsglen);
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

        if (omsglen != kemsglen + SGX_MPC_MACH_REPLEN) {
            byte_zero(kemsg, kemsglen);
            free(kemsg);
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        // ensure msg || rep encoding
        byte_copy(omsg, kemsglen, kemsg);
        byte_copy(omsg + kemsglen, SGX_MPC_MACH_REPLEN, report);
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

        res = box(&boxoutput, &boxoutputlen, pid, inmsg, inmsglen);
        if (!(res == SGX_MPC_OK && boxoutputlen == omsglen)) {
            byte_zero(boxoutput, boxoutputlen);
            free(boxoutput);
            seq_stage = MPC_P_ERROR;
            return SGX_MPC_ERROR;
        }

        byte_copy(omsg, boxoutputlen, boxoutput);

        return SGX_MPC_OK;

    } else {
        seq_stage = MPC_P_ERROR;
        return SGX_MPC_ERROR;
    }
}
