#include <stdio.h>
#include <string.h>
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

// TODO: free values, add quotes
int main(int argc, char** argv) {
#ifdef SGX_MPC_OUTSIDE
#error SGX_MPC_OUTSIDE NOT SUPPORTED ATM
#endif

    if(!getenv("DATASET_FILE")) {
        printf("Dataset file (DATASET_FILE) path not set!");
        exit(1);
    }

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

    size_t update_counter = 0;
    size_t search_counter = 0;

    if(fread(&nr_updates, sizeof(size_t), 1, in_f) != 1) {
        printf("Error reading file!\n");
        exit(-1);
    }

    if(fread(&nr_searches, sizeof(size_t), 1, in_f) != 1) {
        printf("Error reading file!\n");
        exit(-1);
    }

    long elapsed = 0;

    const size_t test_len = 1 + nr_updates + nr_searches;
    printf("nr updates: %lu, nr searches: %lu, test len: %lu\n", nr_updates, nr_searches, test_len);

    printf("############### CALLING F ###############\n");
    for (int cmd_index = 0; cmd_index < test_len; cmd_index++) {
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

        struct timeval start, end;
        gettimeofday(&start, NULL);

        res |= mpc_process(&msg_lr, &msg_lr_len, 0x82, cmd, cmd_len, 1); /* encode first input */
        gettimeofday(&end, NULL);
        elapsed += timeElapsed(start, end);

        free(cmd);

        //printf("Send Key:Local -> Remote: %llu bytes\n",msg_lr_len);
        //printf("Status: %d\n",res);

        //msg_rl_len = commands_ret_sizes[cmd_index] + SGX_MPC_AEAD_EXPBYTES;
        msg_rl_len = 0;
        gettimeofday(&start, NULL);
        res |= lac_attest(&msg_rl, &msg_rl_len, handle, 0x82, msg_lr, msg_lr_len); /* deliver first input get first output */
        gettimeofday(&end, NULL);
        elapsed += timeElapsed(start, end);

        //printf("Answer Output: Remote -> Local: %llu bytes\n",msg_rl_len);
        //printf("Status: %d\n",res);

        free(msg_lr);

        gettimeofday(&start, NULL);
        res |= mpc_process(&msg_lr, &msg_lr_len, 0x82, msg_rl, msg_rl_len, 0); /* decrypt first output */
        gettimeofday(&end, NULL);
        elapsed += timeElapsed(start, end);

        //printf("Output Locally Received\n");
        //printf("Status: %d\n",res);

        /*for(unsigned i = 0; i < msg_lr_len; i++)
            printf("%02x", msg_lr[i]);
        printf("\n");*/

        if (!((res == SGX_MPC_OK))) {
            printf("Error detected: %d, length %llu : \n", res, msg_lr_len);
            return -1;
        }

        /*printf("(%d/%llu) OK: Output : %llu\n", cmd_index, test_len, msg_lr_len);
        for(i=0;i<msg_lr_len;i++)
            printf("%02x", msg_lr[i]);
        printf("\n");*/

        // tests are composed of 1 setup, nr_docs adds, and then searches
        if (cmd_index == 0) {
            printf("MPC Did setup\n");
            elapsed = 0;
        } else if (update_counter < nr_updates) {
            //printf("Update %lu/%lu\n", update_counter, nr_updates);
            update_counter++;

            if (!(update_counter % 5000)) {
                printf("MPC update: (%lu/%lu)\n", update_counter, nr_updates);
                printf("time: add = %6.3lf seconds!\n", elapsed / 1000000.0);
            }

            if (update_counter == nr_updates) {
                printf("MPC time: total iee add = %6.3lf seconds!\n", elapsed / 1000000.0);
                elapsed = 0;
            }

        } else if (search_counter < nr_searches) {
            const int n_docs = msg_lr_len / sizeof(int);
            printf("MPC Search(%06lu) result: %d docs\n", search_counter, n_docs);
            printf("MPC time: iee total search = %6.3lf seconds!\n", elapsed / 1000000.0);
            elapsed = 0;
            /*int pos = 0;
            for (int i = 0; i < n_docs; i++) {
                int d = read_int(msg_lr, &pos);
                printf("%d ", d);
            }
            printf("\n");*/
            search_counter++;

            /*if(search_counter == nr_searches) {
                printf("MPC time: iee total search = %6.3lf seconds!\n", elapsed/1000000.0 );
                elapsed = 0;
            }*/

        } else {
            printf("MPC There shouldn't be more operations here\n");
            exit(-1);
        }

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
