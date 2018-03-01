#ifndef __SGX_MPC_H
#define __SGX_MPC_H

#include <stdint.h>

// SGX_MPC_NPARTIES must be defined if not, default to 2 party test

// TODO : define SGX_MPC_NPARTIES as a makefile argument
#ifndef SGX_MPC_NPARTIES
 #define SGX_MPC_NPARTIES 2
#endif

#define SGX_MPC_OK 0
#define SGX_MPC_ERROR 1

typedef unsigned char byte;
typedef unsigned char *bytes;
typedef unsigned long long size;
typedef unsigned long label;

#define SGX_MPC_AEAD_NONCEBYTES 24
#define SGX_MPC_AEAD_EXPBYTES 16
#define SGX_MPC_AEAD_KEYBYTES 32

#define SGX_MPC_AKE_KEYBYTES 32
#define SGX_MPC_AKE_SCALARBYTES 32

#define SGX_MPC_SECRETKEYBYTES 64
#define SGX_MPC_PUBLICKEYBYTES 32

#define SGX_MPC_SIGEXP_BYTES 64 

static const uint8_t sp_id_value[] = {
  0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

#endif
