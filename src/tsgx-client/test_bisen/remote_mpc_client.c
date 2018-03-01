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

void printbuf(unsigned char* b, size_t len) {
    for(unsigned i = 0; i < len; i++) {
        printf("%02x ", b[i]);
    }
    printf("\n-----------------\n");
}

int remote_mach_load(int sock, void **handle, char *filename) {
    size_t len = sizeof(unsigned char) + strlen(filename) + 1;
    const unsigned char op = 'l';

    unsigned char * buffer = (unsigned char *)malloc(sizeof(unsigned char) * len);
    memcpy(buffer, &op, sizeof(unsigned char));
    memcpy(buffer + sizeof(unsigned char), filename, strlen(filename) + 1);

    unsigned char * recv_buffer;
    size_t recv_len = send_and_receive(sock, buffer, len, &recv_buffer);

    // TODO pass to int directly instead of buffer
    int res;
    memcpy(&res, recv_buffer, sizeof(int));

    return res;
}

int remote_mach_quote(int sock, unsigned char* omsg, unsigned long long omsglen, const unsigned char* imsg, const unsigned long long imsglen) {
    // op + omsglen + imsglen + imsg
    size_t len = sizeof(unsigned char) + 2 * sizeof(unsigned long long) + imsglen;
    const unsigned char op = 'q';

    unsigned char * buffer = (unsigned char *)malloc(sizeof(unsigned char) * len);
    memcpy(buffer, &op, sizeof(unsigned char));
    memcpy(buffer + sizeof(unsigned char), &omsglen, sizeof(unsigned long long));
    memcpy(buffer + sizeof(unsigned char) + sizeof(unsigned long long), &imsglen, sizeof(unsigned long long));
    memcpy(buffer + sizeof(unsigned char) + 2 * sizeof(unsigned long long), imsg, imsglen);

    unsigned char * recv_buffer;
    size_t recv_len = send_and_receive(sock, buffer, len, &recv_buffer);

    int res;
    memcpy(&res, recv_buffer, sizeof(int));

    memcpy(omsg, recv_buffer + sizeof(int), omsglen);

    return res;
}

int remote_mach_run(int sock, unsigned char* omsg, unsigned long long omsglen, const unsigned char* imsg, const unsigned long long imsglen) {

}

int remote_mach_verify(int sock, unsigned char* omsg, unsigned long long omsglen, const unsigned char* imsg, const unsigned long long imsglen) {

}

int main(int argc,char **argv) {
    //int res;

    /*bytes msg_lr;
    size msg_lr_len;
    bytes msg_rl;
    size msg_rl_len;

    bytes sigpk_p1 = pubs[0];
    attke_local_state local_st_p1 = lsts[0];

    bytes sigpk_p2 = pubs[1];
    attke_local_state local_st_p2 = lsts[1];
*/
    void *handle = NULL;
    char *filename = "enclave.signed.a";

    int sock = create_socket();
    int res = remote_mach_load(sock, handle, filename);
    printf("res %d\n", res);

    res = mpc_process_init(NULL,0,NULL,1,sigpk_p1,&local_st_p1);
    printf("Initialized\n");
    printf("Status: %d\n",res);
    printf("a\n");

    msg_rl_len = SGX_MPC_AKE_KEYBYTES+16;
    res |= lac_attest(sock, &msg_rl,&msg_rl_len,handle,0x01,NULL,0); /* attested pub params */
    printf("ATTKE PRMS R: Remote -> Local: %llu bytes\n",msg_rl_len);
    printf("Status: %d\n",res);
    printf("b\n");

    return 0;
}
