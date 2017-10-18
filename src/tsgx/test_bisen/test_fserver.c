#include <stdio.h>
#include <string.h>

#include "attke.h"
#include "lac_attest.h"
#include "mach.h"
#include "mpc_process.h"
#include "time_definitions.h"

#include "f/public_key.h"
#include "secret_key.h"

#include "f/commands.h"

extern size test_len;
extern bytes* commands;
extern size* commands_sizes;
#define TEST_DOCS 1000 // TODO pass this from the commands.c too

// Utility functions
void read_arr(const void* val, int size, const unsigned char * arr, int* pos) {
    memcpy((void*)val, &arr[*pos], size);
    *pos += size;
}

int read_int(const unsigned char * arr, int* pos) {
    int x;
    read_arr(&x, sizeof(int), arr, pos);
    return x;
}

// TODO: free values, add quotes
int main(int argc,char **argv)
{
  #ifdef SGX_MPC_OUTSIDE
    #error SGX_MPC_OUTSIDE NOT SUPPORTED ATM
  #endif

  // read commands from input file
  generate_commands();

  int res;

  bytes msg_lr;
  size msg_lr_len;
  bytes msg_rl;
  size msg_rl_len;

  bytes sigpk_p1 = pubs[0];
  attke_local_state local_st_p1 = lsts[0];

  bytes sigpk_p2 = pubs[1];
  attke_local_state local_st_p2 = lsts[1];

  int cmd_index;
  int i;
  void *handle = NULL;
  char *filename = "enclave.signed.a";

  if( mach_load(&handle,filename) != 0 )
  { printf("enclave could not be loaded\n");
    return 0;
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////  

  printf("############### PARTY 1 ATTKE ###############\n");

  res = mpc_process_init(NULL,0,NULL,1,sigpk_p1,&local_st_p1);
  printf("Initialized\n");
  printf("Status: %d\n",res);

  msg_rl_len = SGX_MPC_AKE_KEYBYTES+16;
  res |= lac_attest(&msg_rl,&msg_rl_len,handle,0x01,NULL,0); /* attested pub params */
  printf("ATTKE PRMS R: Remote -> Local: %llu bytes\n",msg_rl_len);
  printf("Status: %d\n",res);

  res |= mpc_process(&msg_lr,&msg_lr_len,0x01,msg_rl,msg_rl_len,0); /* signed pub params */
  printf("ATTKE PRMS L: Local -> Remote: %llu bytes\n",msg_lr_len);
  printf("Status: %d\n",res);

  msg_rl_len = 3+16;
  res |= lac_attest(&msg_rl,&msg_rl_len,handle,0x01,msg_lr,msg_lr_len); /* accept and OK */
  printf("ATTKE OK R: Remote -> Local: %llu bytes\n",msg_rl_len);
  printf("Status: %d\n",res);

  res |= mpc_process(&msg_lr,&msg_lr_len,0x01,msg_rl,msg_rl_len,0); /* accept */
  printf("OK Delivered Locally\n");
  printf("Status: %d\n",res);

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  printf("############### PARTY 2 ###############\n");

  res = mpc_process_init(NULL,0,NULL,2,sigpk_p2,&local_st_p2);
  printf("Initialized\n");
  printf("Status: %d\n",res);

  msg_rl_len = SGX_MPC_AKE_KEYBYTES+16;
  res |= lac_attest(&msg_rl,&msg_rl_len,handle,0x02,NULL,0); /* attested pub params */
  printf("ATTKE PRMS R: Remote -> Local: %llu bytes\n",msg_rl_len);
  printf("Status: %d\n",res);

  res |= mpc_process(&msg_lr,&msg_lr_len,0x02,msg_rl,msg_rl_len,0); /* signed pub params */
  printf("ATTKE PRMS L: Local -> Remote: %llu bytes\n",msg_lr_len);
  printf("Status: %d\n",res);

  msg_rl_len = 3+16;
  res |= lac_attest(&msg_rl,&msg_rl_len,handle,0x02,msg_lr,msg_lr_len); /* accept and OK */
  printf("ATTKE OK R: Remote -> Local: %llu bytes\n",msg_rl_len);
  printf("Status: %d\n",res);

  res |= mpc_process(&msg_lr,&msg_lr_len,0x02,msg_rl,msg_rl_len,0); /* accept */
  printf("OK Delivered Locally\n");
  printf("Status: %d\n",res);

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  printf("############### CALLING F ###############\n");

  for(cmd_index=0; cmd_index<test_len; cmd_index++) {
    res |= mpc_process(&msg_lr,&msg_lr_len,0x82,commands[cmd_index],commands_sizes[cmd_index],1); /* encode first input */
    printf("Send Key:Local -> Remote: %llu bytes\n",msg_lr_len);
    printf("Status: %d\n",res);

    //msg_rl_len = commands_ret_sizes[cmd_index] + SGX_MPC_AEAD_EXPBYTES;
    msg_rl_len = 0;
    res |= lac_attest(&msg_rl,&msg_rl_len,handle,0x82,msg_lr,msg_lr_len); /* deliver first input get first output */
    printf("Answer Output: Remote -> Local: %llu bytes\n",msg_rl_len);
    printf("Status: %d\n",res);

    free(msg_lr);

    res |= mpc_process(&msg_lr,&msg_lr_len,0x82, msg_rl,msg_rl_len,0); /* decrypt first output */
    printf("Output Locally Received\n");
    printf("Status: %d\n",res);

    for(unsigned i = 0; i < msg_lr_len; i++)
        printf("%02x", msg_lr[i]);
    printf("\n");

    if (!((res == SGX_MPC_OK))) {
      printf("Error detected: %d, length %llu : \n", res, msg_lr_len);
      return -1;
    }

    printf("(%d/%llu) OK: Output : %llu\n", cmd_index, test_len, msg_lr_len);
    for(i=0;i<msg_lr_len;i++)
        printf("%02x", msg_lr[i]);
    printf("\n");

    // tests are composed of 1 setup, "TEST_DOCS" adds, and then searches
    if(cmd_index > TEST_DOCS + 1) {
        const int n_docs = msg_lr_len / sizeof(int);
        printf("Search result: %d docs\n", n_docs);

        int pos = 0;
        for (int i = 0; i < n_docs; i++) {
            int d = read_int(msg_lr, &pos);
            printf("%d ", d);
        }
        printf("\n");
    }

    free(msg_lr);
    free(msg_rl);
  }

  mach_finalize(handle);

  return SGX_MPC_OK;
}
