#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "types.h"

static char filename[] = "storage.txt";

static void fs_create_or_reset_file(
  bytes *out,
  size *outlen
)
{
  FILE *f = fopen(filename, "w");
  fclose(f);
  *out = (bytes) malloc(sizeof(byte)*1);
  (*out)[0] = 0x00;
  *outlen = 1;
}

static void fs_add_byte_string_to_file(
  bytes *out,
  size *outlen,
  const bytes in,
  const size inlen
)
{
  FILE *f = fopen(filename, "a");
  for(int i=0;i<in[0];i++)
    fprintf(f, "%02x",in[1+i]);
  fprintf(f,"\n");
  fclose(f);
  *out = (bytes) malloc(sizeof(byte)*1);
  (*out)[0] = 0x00;
  *outlen = 1;
}

static void fs_get_byte_string_from_file(
  bytes *out,
  size *outlen,
  const bytes in,
  const size inlen
)
{
  int i;
  char line[512];
  size linelen;
  unsigned int x;
  FILE *f = fopen(filename, "r");
  i = 1; // convention : line numbers start at 1
  while(fgets(line, sizeof(line), f) && i<in[0])
    i++;
  linelen = (strlen(line)) >> 1;
  *out = (bytes) malloc(sizeof(byte)*linelen);
  *outlen = linelen;
  i=0;
  while(sscanf(&(line[i<<1]),"%02x",&x)==1)
  { (*out)[i] = (unsigned char)x; i++; }
}

void fserver(
  bytes *out,
  size *outlen,
  const bytes in,
  const size inlen
)
{
    // set out variables
    *out = NULL;
    *outlen = 0;

    if(in[0] == 0xFF) {
        char* msg = (char*)malloc((inlen - 1) * sizeof(char));
        for(int i = 1; i < inlen; i++)
            msg[i - 1] = in[i];

        perror(msg);
        exit(0);
    }

  // 0x2 : create or reset file
  if(in[0] == 0x02)
  { fs_create_or_reset_file(out,outlen);
    return;
  }

  // 0x3 : add byte string to file
  if(in[0] == 0x03)
  { fs_add_byte_string_to_file(out,outlen,in+1,inlen-1);
    return;
  }

  // 0x4 : get byte string from file
  if(in[0] == 0x04)
  { fs_get_byte_string_from_file(out,outlen,in+1,inlen-1);
    return;
  }

}
