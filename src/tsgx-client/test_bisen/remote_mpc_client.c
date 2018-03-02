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

#include "../../Common/Utils.h"
#include "client.c"
#include "mach.h"

void printbuf(unsigned char* b, size_t len) {
    for(unsigned i = 0; i < len; i++) {
        printf("%02x ", b[i]);
    }
    printf("\n-----------------\n");
}

int remote_mach_load(int sock, void **handle, char *filename) {
    //printf("performing remote_mach_load\n");
    size_t len = sizeof(unsigned char) + strlen(filename) + 1;
    const unsigned char op = 'l';

    unsigned char * buffer = (unsigned char *)malloc(sizeof(unsigned char) * len);
    memcpy(buffer, &op, sizeof(unsigned char));
    memcpy(buffer + sizeof(unsigned char), filename, strlen(filename) + 1);

    //unsigned char * recv_buffer;
    int res;
    size_t recv_len = send_and_receive(sock, buffer, len, (void*)&res);
    //memcpy(&res, recv_buffer, sizeof(int));

    free(buffer);
    //free(recv_buffer);
    return res;
}

int remote_mach_quote(int sock, bytes omsg, size omsglen, const bytes imsg, const size imsglen) {
    //printf("performing remote_mach_quote\n");
    // op + omsglen + imsglen + imsg
    size_t len = sizeof(unsigned char) + 2 * sizeof(unsigned long long) + imsglen;
    const unsigned char op = 'q';

    void* buffer = malloc(sizeof(unsigned char) * len);
    memcpy(buffer, &op, sizeof(unsigned char));
    memcpy(buffer + sizeof(unsigned char), &omsglen, sizeof(unsigned long long));
    memcpy(buffer + sizeof(unsigned char) + sizeof(unsigned long long), &imsglen, sizeof(unsigned long long));
    memcpy(buffer + sizeof(unsigned char) + 2 * sizeof(unsigned long long), imsg, imsglen);

    void* recv_buffer;
    size_t recv_len = send_and_receive(sock, buffer, len, &recv_buffer);

    int res;
    memcpy(&res, recv_buffer, sizeof(int));
    memcpy(omsg, recv_buffer + sizeof(int), omsglen);

    free(buffer);
    free(recv_buffer);
    return res;
}

int remote_mach_run(int sock, bytes *omsg, size *omsglen, const void *handle, const label l, const bytes imsg, const size imsglen) {
    //printf("performing remote_mach_run\n");
    // op + l + *omsglen + imsglen + imsg
    size_t len = sizeof(unsigned char) + sizeof(label) + /*2 * */ sizeof(unsigned long long) + imsglen;
    const unsigned char op = 'r';

    void* buffer = malloc(sizeof(unsigned char) * len);
    void* tmp = buffer;

    memcpy(tmp, &op, sizeof(unsigned char));
    tmp += sizeof(unsigned char);

    memcpy(tmp, &l, sizeof(label));
    tmp += sizeof(label);

    //memcpy(tmp, omsglen, sizeof(unsigned long long));
    //tmp += sizeof(unsigned long long);

    memcpy(tmp, &imsglen, sizeof(unsigned long long));
    tmp += sizeof(unsigned long long);

    memcpy(tmp, imsg, imsglen);
    //printf("created buffer\n");
    void* recv_buffer;
    size_t recv_len = send_and_receive(sock, buffer, len, &recv_buffer);
    //printbuf(recv_buffer, recv_len);

    //printf("sent and received\n");
    int res;
    memcpy(&res, recv_buffer, sizeof(int));

    memcpy(omsglen, recv_buffer + sizeof(int), sizeof(size));
    //printf("%llu\n", *omsglen);
    *omsg = (bytes)malloc(sizeof(unsigned char) * *omsglen);
    memcpy(*omsg, recv_buffer + sizeof(int) + sizeof(size), *omsglen);

    free(buffer);
    free(recv_buffer);

    return res;
}

int remote_mach_verify(int sock, const bytes imsg, const size imsglen, const bytes code, const size codelen) {
    //printf("performing remote_mach_verify\n");
    // op + imsglen + imsg + codelen + code
    size_t len = sizeof(unsigned char) + 2 * sizeof(unsigned long long) + imsglen + codelen;
    const unsigned char op = 'v';

    void* buffer = malloc(sizeof(unsigned char) * len);
    void* tmp = buffer;

    memcpy(tmp, &op, sizeof(unsigned char));
    tmp += sizeof(unsigned char);

    memcpy(tmp, &imsglen, sizeof(unsigned long long));
    tmp += sizeof(unsigned long long);

    memcpy(tmp, imsg, imsglen);
    tmp += imsglen;

    memcpy(tmp, code, codelen);
    tmp += codelen;

    memcpy(tmp, &codelen, sizeof(unsigned long long));
    tmp += sizeof(unsigned long long);

    void* recv_buffer;
    size_t recv_len = send_and_receive(sock, buffer, len, &recv_buffer);

    int res;
    memcpy(&res, recv_buffer, sizeof(int));

    return res;
}

void remote_mach_finalize(int sock) {
    printf("performing remote_mach_finalize\n");
    // op
    size_t len = sizeof(unsigned char);
    const unsigned char op = 'f';

    void* buffer = malloc(sizeof(unsigned char) * len);
    memcpy(buffer, &op, sizeof(unsigned char));

    void* recv_buffer;
    /*size_t recv_len = */send_and_receive(sock, buffer, len, &recv_buffer);

    int res;
    memcpy(&res, recv_buffer, sizeof(int));

    free(buffer);
    free(recv_buffer);
}

int main(int argc,char **argv) {
    //int res;
    generate_commands();
    int cmd_index;

    bytes msg_lr;
    size msg_lr_len;
    bytes msg_rl;
    size msg_rl_len;

    bytes sigpk_p1 = pubs[0];
    attke_local_state local_st_p1 = lsts[0];

    bytes sigpk_p2 = pubs[1];
    attke_local_state local_st_p2 = lsts[1];

    void *handle = NULL;
    char *filename = "enclave.signed.a";

    int sock = create_socket();
    int res = remote_mach_load(sock, handle, filename);
    printf("res %d\n", res);

    res = mpc_process_init(sock, NULL,0,NULL,1,sigpk_p1,&local_st_p1);
    printf("Initialized\n");
    printf("Status: %d\n",res);
    printf("a\n");

    msg_rl_len = SGX_MPC_AKE_KEYBYTES+16;
    res |= lac_attest(sock, &msg_rl,&msg_rl_len,handle,0x01,NULL,0); /* attested pub params */
    printf("ATTKE PRMS R: Remote -> Local: %llu bytes\n",msg_rl_len);
    printf("Status: %d\n",res);
    printf("b\n");

    res |= mpc_process(sock, &msg_lr,&msg_lr_len,0x01,msg_rl,msg_rl_len,0); /* signed pub params */
    printf("ATTKE PRMS L: Local -> Remote: %llu bytes\n",msg_lr_len);
    printf("Status: %d\n",res);

    msg_rl_len = 3+16;
    res |= lac_attest(sock, &msg_rl,&msg_rl_len,handle,0x01,msg_lr,msg_lr_len); /* accept and OK */
    printf("ATTKE OK R: Remote -> Local: %llu bytes\n",msg_rl_len);
    printf("Status: %d\n",res);

    res |= mpc_process(sock, &msg_lr,&msg_lr_len,0x01,msg_rl,msg_rl_len,0); /* accept */
    printf("OK Delivered Locally\n");
    printf("Status: %d\n",res);


    printf("############### PARTY 2 ###############\n");
    res = mpc_process_init(sock, NULL,0,NULL,2,sigpk_p2,&local_st_p2);
    printf("Initialized\n");
    printf("Status: %d\n",res);

    msg_rl_len = SGX_MPC_AKE_KEYBYTES+16;
    res |= lac_attest(sock, &msg_rl,&msg_rl_len,handle,0x02,NULL,0); /* attested pub params */
    printf("ATTKE PRMS R: Remote -> Local: %llu bytes\n",msg_rl_len);
    printf("Status: %d\n",res);

    res |= mpc_process(sock, &msg_lr,&msg_lr_len,0x02,msg_rl,msg_rl_len,0); /* signed pub params */
    printf("ATTKE PRMS L: Local -> Remote: %llu bytes\n",msg_lr_len);
    printf("Status: %d\n",res);

    msg_rl_len = 3+16;
    res |= lac_attest(sock, &msg_rl,&msg_rl_len,handle,0x02,msg_lr,msg_lr_len); /* accept and OK */
    printf("ATTKE OK R: Remote -> Local: %llu bytes\n",msg_rl_len);
    printf("Status: %d\n",res);

    res |= mpc_process(sock, &msg_lr,&msg_lr_len,0x02,msg_rl,msg_rl_len,0); /* accept */
    printf("OK Delivered Locally\n");
    printf("Status: %d\n",res);

    printf("############### CALLING F ###############\n");

    size_t update_counter = 0;
    size_t search_counter = 0;

    long elapsed = 0;

    printf("nr updates: %lu, nr searches: %lu, test len: %llu\n", nr_updates, nr_searches, test_len);

    for(cmd_index=0; cmd_index<test_len; cmd_index++) {
        struct timeval start, end;

        gettimeofday(&start, NULL);
        res |= mpc_process(sock, &msg_lr,&msg_lr_len,0x82,commands[cmd_index],commands_sizes[cmd_index],1); /* encode first input */
        gettimeofday(&end, NULL);
        elapsed += timeElapsed(start, end);

        //printf("Send Key:Local -> Remote: %llu bytes\n",msg_lr_len);
        //printf("Status: %d\n",res);

        //msg_rl_len = commands_ret_sizes[cmd_index] + SGX_MPC_AEAD_EXPBYTES;
        msg_rl_len = 0;
        gettimeofday(&start, NULL);
        res |= lac_attest(sock, &msg_rl,&msg_rl_len,handle,0x82,msg_lr,msg_lr_len); /* deliver first input get first output */
        gettimeofday(&end, NULL);
        elapsed += timeElapsed(start, end);

        free(msg_lr);

        gettimeofday(&start, NULL);
        res |= mpc_process(sock, &msg_lr,&msg_lr_len,0x82, msg_rl,msg_rl_len,0); /* decrypt first output */
        gettimeofday(&end, NULL);
        elapsed += timeElapsed(start, end);

        if (!((res == SGX_MPC_OK))) {
            printf("Error detected: %d, length %llu : \n", res, msg_lr_len);
            return -1;
        }

        // tests are composed of 1 setup, nr_docs adds, and then searches
        if(cmd_index == 0 ) {
            printf("MPC Did setup\n");
            elapsed = 0;
        } else if(update_counter < nr_updates) {
            //printf("Update %lu/%lu\n", update_counter, nr_updates);
            update_counter++;

            if(!(update_counter % 1000))
                printf("MPC update: (%lu/%lu)\n", update_counter, nr_updates);

            if(update_counter == nr_updates) {
                printf("MPC time: total iee add = %6.3lf seconds!\n", elapsed/1000000.0 );
                elapsed = 0;
            }

        } else if(search_counter < nr_searches) {
            const int n_docs = msg_lr_len / sizeof(int);
            printf("MPC Search(%06lu) result: %d docs\n", search_counter, n_docs);
            printf("MPC time: iee total search = %6.3lf seconds!\n", elapsed/1000000.0 );
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

    remote_mach_finalize(sock);

    return 0;
}
