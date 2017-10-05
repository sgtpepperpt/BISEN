#include <stdio.h>
#include "types.h"

// 0x1 : setup secret key
byte cmd1[] = {0x01};

// 0x2 : create or reset file
byte cmd2[] = {0x02};

// 0x3 : add byte string to file
byte cmd3_1[] = {0x03, 0x03, 0xAA, 0xBB, 0xCC};
byte cmd3_2[] = {0x03, 0x02, 0xAA, 0xBB};
byte cmd3_3[] = {0x03, 0x01, 0xAA};

// 0x4 : get byte string from file
byte cmd4_1[] = {0x04, 0x02};
byte cmd4_2[] = {0x04, 0x02};
byte cmd4_3[] = {0x04, 0x03};
byte cmd4_4[] = {0x04, 0x03};
byte cmd4_5[] = {0x04, 0x01};

// exported 
#define TL 10 
size test_len = TL;
bytes commands[TL] = { cmd1, cmd2, cmd3_1, cmd3_2, cmd3_3, cmd4_1, cmd4_2, cmd4_3, cmd4_4, cmd4_5 };
size commands_sizes[TL] = {1,1,   5,4,3,   2,2,2,2,2};

int f_assert(
  size test_index,
  bytes out,
  size outlen
)
{
  if(test_index>=0)
  { printf("%llu - %02x : ",outlen, out[0]);
    for(int i=1;i<outlen;i++)
      printf("%02x",out[i]);
    printf("\n");
  }

  return 1;
}
