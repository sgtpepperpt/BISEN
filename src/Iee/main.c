#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "types.h"
#include "src/SseIee.h"

#include <string.h>
/*
extern void f(
  bytes *out,
  size *out_len,
  const label pid,
  const bytes in,
  const size in_len
);
*/
extern int f_assert(
  size test_index,
  bytes out,
  size outlen
);

int main()
{
    init_pipes();
    return 0;
}
