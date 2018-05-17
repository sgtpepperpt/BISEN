#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "attke.h"
#include "lac_attest.h"
#include "mach.h"
#include "mpc_process.h"
#include "time_definitions.h"

#include "f/public_key.h"
#include "secret_key.h"

#include "f/commands.h"

extern size_t nr_updates;
extern size_t nr_searches;
extern size test_len;
extern bytes* commands;
extern size* commands_sizes;

// Utility functions
void read_arr(const void* val, int size, const unsigned char* arr, int* pos) {
    memcpy((void*)val, &arr[*pos], size);
    *pos += size;
}

int read_int(const unsigned char* arr, int* pos) {
    int x;
    read_arr(&x, sizeof(int), arr, pos);
    return x;
}

double fserver_diff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }

    return ((double)temp.tv_sec * 1000000000.0 + (double)temp.tv_nsec) / 1000000.0;
}

// TODO: free values, add quotes
int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
#ifdef SGX_MPC_OUTSIDE
#error SGX_MPC_OUTSIDE NOT SUPPORTED ATM
#endif

    if(!getenv("DATASET_FILE")) {
        printf("Dataset file (DATASET_FILE) path not set!");
        exit(1);
    }

    struct timespec ts1, ts2;
    clock_gettime(CLOCK_REALTIME, &ts1);

    int res;

    bytes msg_lr;
    size msg_lr_len;
    bytes msg_rl;
    size msg_rl_len;

    bytes sigpk_p1 = pubs[0];
    attke_local_state local_st_p1 = lsts[0];

    bytes sigpk_p2 = pubs[1];
    attke_local_state local_st_p2 = lsts[1];

    //int i;
    void* handle = NULL;
    char* filename = "enclave.signed.a";

    if (mach_load(&handle, filename) != 0) {
        printf("enclave could not be loaded\n");
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    //printf("############### PARTY 1 ATTKE ###############\n");

    res = mpc_process_init(NULL, 0, NULL, 1, sigpk_p1, &local_st_p1);
    printf("Initialized\n");
    printf("Status: %d\n", res);

    msg_rl_len = SGX_MPC_AKE_KEYBYTES + 16;
    res |= lac_attest(&msg_rl, &msg_rl_len, handle, 0x01, NULL, 0); /* attested pub params */
    printf("ATTKE PRMS R: Remote -> Local: %llu bytes\n", msg_rl_len);
    printf("Status: %d\n", res);

    res |= mpc_process(&msg_lr, &msg_lr_len, 0x01, msg_rl, msg_rl_len, 0); /* signed pub params */
    printf("ATTKE PRMS L: Local -> Remote: %llu bytes\n", msg_lr_len);
    printf("Status: %d\n", res);

    msg_rl_len = 3 + 16;
    res |= lac_attest(&msg_rl, &msg_rl_len, handle, 0x01, msg_lr, msg_lr_len); /* accept and OK */
    printf("ATTKE OK R: Remote -> Local: %llu bytes\n", msg_rl_len);
    printf("Status: %d\n", res);

    res |= mpc_process(&msg_lr, &msg_lr_len, 0x01, msg_rl, msg_rl_len, 0); /* accept */
    printf("OK Delivered Locally\n");
    printf("Status: %d\n", res);

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    //printf("############### PARTY 2 ###############\n");

    res = mpc_process_init(NULL, 0, NULL, 2, sigpk_p2, &local_st_p2);
    //printf("Initialized\n");
    //printf("Status: %d\n",res);

    msg_rl_len = SGX_MPC_AKE_KEYBYTES + 16;
    res |= lac_attest(&msg_rl, &msg_rl_len, handle, 0x02, NULL, 0); /* attested pub params */
    //printf("ATTKE PRMS R: Remote -> Local: %llu bytes\n",msg_rl_len);
    //printf("Status: %d\n",res);

    res |= mpc_process(&msg_lr, &msg_lr_len, 0x02, msg_rl, msg_rl_len, 0); /* signed pub params */
    //printf("ATTKE PRMS L: Local -> Remote: %llu bytes\n",msg_lr_len);
    //printf("Status: %d\n",res);

    msg_rl_len = 3 + 16;
    res |= lac_attest(&msg_rl, &msg_rl_len, handle, 0x02, msg_lr, msg_lr_len); /* accept and OK */
    //printf("ATTKE OK R: Remote -> Local: %llu bytes\n",msg_rl_len);
    //printf("Status: %d\n",res);

    res |= mpc_process(&msg_lr, &msg_lr_len, 0x02, msg_rl, msg_rl_len, 0); /* accept */
    //printf("OK Delivered Locally\n");
    //printf("Status: %d\n",res);

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    clock_gettime(CLOCK_REALTIME, &ts2);
    double elapsed_init = fserver_diff(ts1, ts2);
    printf("Init time = %6.3lf seconds\n", elapsed_init);

    // init output file
    FILE *in_f = fopen(getenv("DATASET_FILE"),"rb");
    if (!in_f) {
        printf("Error opening test file!\n");
        exit(-1);
    }

    printf("DATASET_FILE: %s\n", getenv("DATASET_FILE"));

    // get file size
    //size_t f_size = get_file_size(in_f);
    //printf("File size: %lu\n", f_size);

    // count number of queries
    size_t nr_updates = 0;
    size_t nr_searches = 0;

    if(fread(&nr_updates, sizeof(size_t), 1, in_f) != 1) {
        printf("Error reading file!\n");
        exit(-1);
    }

    if(fread(&nr_searches, sizeof(size_t), 1, in_f) != 1) {
        printf("Error reading file!\n");
        exit(-1);
    }

    const size_t test_len = 1 + nr_updates + nr_searches;
    printf("nr updates: %lu, nr searches: %lu, test len: %lu\n", nr_updates, nr_searches, test_len);

    printf("############### CALLING F ###############\n");

    // INIT
    {
        size cmd_len;
        if(fread(&cmd_len, sizeof(size), 1, in_f) != 1) {
            printf("Error reading file!\n");
            exit(-1);
        }

        unsigned char* cmd = (unsigned char*)malloc(cmd_len * sizeof(unsigned char*));
        //memset(commands[current_query], commands_sizes[current_query] * sizeof(unsigned char*));
        if(fread(cmd, cmd_len, 1, in_f) != 1) {
            printf("Error reading file!\n");
            exit(-1);
        }

        res |= mpc_process(&msg_lr, &msg_lr_len, 0x82, cmd, cmd_len, 1); /* encode first input */
        free(cmd);

        msg_rl_len = 0;
        res |= lac_attest(&msg_rl, &msg_rl_len, handle, 0x82, msg_lr, msg_lr_len); /* deliver first input get first output */
        free(msg_lr);

        res |= mpc_process(&msg_lr, &msg_lr_len, 0x82, msg_rl, msg_rl_len, 0); /* decrypt first output */
        free(msg_lr);
        free(msg_rl);
    }

    printf("Init done\n");

    // ADD
    double add_time_file = 0;

    struct timespec start, end, t1, t2;
    clock_gettime(CLOCK_REALTIME, &start);

    for (int cmd_index = 0; cmd_index < nr_updates; cmd_index++) {
        clock_gettime(CLOCK_REALTIME, &t1);
        size cmd_len;
        if(fread(&cmd_len, sizeof(size), 1, in_f) != 1) {
            printf("Error reading file!\n");
            exit(-1);
        }

        unsigned char* cmd = (unsigned char*)malloc(cmd_len * sizeof(unsigned char*));
        //memset(commands[current_query], commands_sizes[current_query] * sizeof(unsigned char*));
        if(fread(cmd, cmd_len, 1, in_f) != 1) {
            printf("Error reading file!\n");
            exit(-1);
        }

        clock_gettime(CLOCK_REALTIME, &t2);
        add_time_file += fserver_diff(t1, t2);

        res |= mpc_process(&msg_lr, &msg_lr_len, 0x82, cmd, cmd_len, 1); /* encode first input */
        free(cmd);

        //printf("Send Key:Local -> Remote: %llu bytes\n",msg_lr_len);
        //printf("Status: %d\n",res);

        //msg_rl_len = commands_ret_sizes[cmd_index] + SGX_MPC_AEAD_EXPBYTES;
        msg_rl_len = 0;
        res |= lac_attest(&msg_rl, &msg_rl_len, handle, 0x82, msg_lr, msg_lr_len); /* deliver first input get first output */
        free(msg_lr);

        //printf("Answer Output: Remote -> Local: %llu bytes\n",msg_rl_len);
        //printf("Status: %d\n",res);

        res |= mpc_process(&msg_lr, &msg_lr_len, 0x82, msg_rl, msg_rl_len, 0); /* decrypt first output */

        if (!((res == SGX_MPC_OK))) {
            printf("Error detected: %d, length %llu : \n", res, msg_lr_len);
            return -1;
        }

        /*if (!(cmd_index % 5000)) {
            printf("MPC update: (%d/%lu)\n", cmd_index, nr_updates);
        }*/

        free(msg_lr);
        free(msg_rl);
    }

    clock_gettime(CLOCK_REALTIME, &end);
    //printf("time of file io: %6.3lf\n", add_time_file / 1000.0);
    printf("MPC time: total iee add = %6.3lf seconds!\n", (fserver_diff(start, end) - add_time_file)/ 1000.0);

    // SEARCH
    for (int cmd_index = 0; cmd_index < nr_searches; cmd_index++) {
        size cmd_len;
        if(fread(&cmd_len, sizeof(size), 1, in_f) != 1) {
            printf("Error reading file!\n");
            exit(-1);
        }

        unsigned char* cmd = (unsigned char*)malloc(cmd_len * sizeof(unsigned char*));
        //memset(commands[current_query], commands_sizes[current_query] * sizeof(unsigned char*));
        if(fread(cmd, cmd_len, 1, in_f) != 1) {
            printf("Error reading file!\n");
            exit(-1);
        }

        struct timespec start, end;
        clock_gettime(CLOCK_REALTIME, &start);

        res |= mpc_process(&msg_lr, &msg_lr_len, 0x82, cmd, cmd_len, 1); /* encode first input */
        free(cmd);

        //printf("Send Key:Local -> Remote: %llu bytes\n",msg_lr_len);
        //printf("Status: %d\n",res);

        //msg_rl_len = commands_ret_sizes[cmd_index] + SGX_MPC_AEAD_EXPBYTES;
        msg_rl_len = 0;
        res |= lac_attest(&msg_rl, &msg_rl_len, handle, 0x82, msg_lr, msg_lr_len); /* deliver first input get first output */
        free(msg_lr);

        //printf("Answer Output: Remote -> Local: %llu bytes\n",msg_rl_len);
        //printf("Status: %d\n",res);

        res |= mpc_process(&msg_lr, &msg_lr_len, 0x82, msg_rl, msg_rl_len, 0); /* decrypt first output */

        clock_gettime(CLOCK_REALTIME, &end);

        if (!((res == SGX_MPC_OK))) {
            printf("Error detected: %d, length %llu : \n", res, msg_lr_len);
            return -1;
        }

        const int n_docs = msg_lr_len / sizeof(int);
        printf("MPC Search(%06d) time: %6.3lf seconds (%d docs)\n", cmd_index, fserver_diff(start, end) / 1000.0, n_docs);

        free(msg_lr);
        free(msg_rl);
    }

    // BENCHMARK ONLY - Make server print stats //
    /*unsigned char benchmark_op = 0x5;
    res |= mpc_process(&msg_lr,&msg_lr_len,0x82,&benchmark_op,1,1);
    msg_rl_len = 0;
    res |= lac_attest(&msg_rl,&msg_rl_len,handle,0x82,msg_lr,msg_lr_len);
    free(msg_lr);
    res |= mpc_process(&msg_lr,&msg_lr_len,0x82, msg_rl,msg_rl_len,0);

    if (!((res == SGX_MPC_OK))) {
        printf("Error detected: %d, length %llu : \n", res, msg_lr_len);
        return -1;
    }

    free(msg_lr);
    free(msg_rl);*/
    // END BENCHMARK ONLY //

    mach_finalize(handle);

    return SGX_MPC_OK;
}
