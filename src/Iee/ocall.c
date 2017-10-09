#include "ocall.h"

static char filename[] = "storage.txt";
/*
static void fs_open(unsigned char **out, unsigned long long *outlen, const unsigned char * in, const unsigned long long inlen)
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
}*/

void fserver(unsigned char **out, unsigned long long *outlen, const unsigned char * in, const unsigned long long inlen)
{
    // set out variables
    *out = NULL;
    *outlen = 0;

    if(in[0] == OCALL_WRITE) {
        //printf("Write ocall\n");

        // read values from buffer
        int pos = 1;
        int fildes = iee_readIntFromArr(in, &pos);
        size_t nbytes = iee_read_size_t(in, &pos);

        void *buf = (void *)malloc(sizeof(unsigned char) * nbytes);
        iee_readFromArr(buf, nbytes, in, &pos);
     
        // execute write syscall
        ssize_t res = write(fildes, buf, nbytes);
        //printf("ret: %lu\n", res);

        // write output
        pos = 0;
        *out = (unsigned char *)malloc(sizeof(ssize_t));
        *outlen = sizeof(ssize_t);

        iee_add_ssize_t(res, *out, &pos);

    } else if(in[0] == OCALL_READ) {
        printf("Read ocall\n");

        // read values from buffer
        int pos = 1;
        int fildes = iee_readIntFromArr(in, &pos);
        size_t nbytes = iee_read_size_t(in, &pos);
        printf("nbytes in ocall: %lu\n", nbytes);

        // write return value to output
        // TODO this buffer should hold both the return and the read byte array from start
        *outlen = sizeof(ssize_t) + sizeof(unsigned char) * nbytes;
        *out = (void *)malloc(*outlen);

        // execute read syscall
        ssize_t res = read(fildes, *out+8, nbytes);
        printf("ret read: %lu\n", res);

        pos = 0;
        iee_add_ssize_t(res, *out, &pos);
        //iee_addToArr(((const void *)out)+8, sizeof(unsigned char) * nbytes, *out, &pos);
        for(unsigned i = 0; i < nbytes; i++)
            printf("%02x", ((unsigned char*)out)[i+8]);
        printf("\n");
    } else if(in[0] == OCALL_OPEN) {
        printf("Open ocall\n");
        
        // read values from buffer
        int pos = 1;
        int oflags = iee_readIntFromArr(in, &pos);
        int path_len = iee_readIntFromArr(in, &pos);

        //TODO pass string as last element, \0 will cut it off in open
        char* path = (char*)malloc(sizeof(char) * path_len);
        iee_readFromArr(path, path_len, in, &pos);
        printf("path %d %s\n", path_len, path);
        // execute open syscall
        int res = open(path, oflags);
        printf("ret open: %d\n", res);

        // prepare response
        pos = 0;
        *outlen = sizeof(int);
        *out = (void *)malloc(*outlen);

        iee_addIntToArr(res, *out, &pos);
    } else if(in[0] == OCALL_CLOSE) {
       printf("CLOSE ocall\n");
        
        // read values from buffer
        int pos = 1;
        int fildes = iee_readIntFromArr(in, &pos);

        // execute close syscall
        int res = close(fildes);
        printf("ret close: %d\n", res);

        // prepare response
        pos = 0;
        *outlen = sizeof(int);
        *out = (void *)malloc(*outlen);

        iee_addIntToArr(res, *out, &pos);
    }
}
