#include <stdlib.h>
#include "sgx_mpc.h"

void untrusted_free_bytes(
  bytes *pointer
)
{
  free(*pointer);
  *pointer = NULL;
}
