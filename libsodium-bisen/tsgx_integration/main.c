#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "types.h"

extern void f(
  bytes *out,
  size *out_len,
  const label pid,
  const bytes in,
  const size in_len
);

extern int f_assert(
  size test_index,
  bytes out,
  size outlen
);

extern size test_len;
extern bytes commands[];
extern size commands_sizes[];

int main()
{
  size t;
  bytes out;
  size out_len;

  for(t=0;t<test_len;t++)
  { f(&out, &out_len, 0, commands[t], commands_sizes[t]);
    assert(f_assert(t, out, out_len));
    if(out != NULL) free(out);
  }

  return 0;
}
