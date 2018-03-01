#include <stdlib.h>
#include "sgx_mpc.h"

void untrusted_malloc_bytes(
  bytes *pointer,
  size length
)
{
  *pointer = (bytes) malloc(sizeof(byte) * length);
}
