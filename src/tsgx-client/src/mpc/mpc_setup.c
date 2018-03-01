/* Based on NaCl code by DJ Bernstein */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "attke.h"

int open_write(const char *fn)
{
  int fd = open(fn,O_CREAT | O_WRONLY | O_NONBLOCK,0644);
  if (fd == -1) return -1;
  fcntl(fd,F_SETFD,1);
  return fd;
}

int writeall(int fd,const void *x,long long xlen)
{
  long long w;
  while (xlen > 0) {
    w = xlen;
    if (w > 1048576) w = 1048576;
    w = write(fd,x,w);
    if (w < 0) {
      return -1;
    }
    x += w;
    xlen -= w;
  }
  return 0;
}

int writesync(int fd, const void *x, long long xlen)
{
  if (writeall(fd,x,xlen) == -1) return -1;
  return fsync(fd);
}

int savesync(const char *fn, const void *x, long long xlen)
{
  int fd;
  int r;
  fd = open_write(fn);
  if (fd == -1) return -1;
  r = writesync(fd,x,xlen);
  close(fd);
  return r;
}

int main(int argc,char **argv)
{
  char *d;
  attke_local_state *local_st;
  bytes sigpk;
  int res;
  
  if (!argv[0]) return SGX_MPC_ERROR;
  if (!argv[1]) return SGX_MPC_ERROR;
  d = argv[1];

  umask(022);
  if (mkdir(d,0755) == -1) return SGX_MPC_ERROR;
  if (chdir(d) == -1) return SGX_MPC_ERROR;
  
  res = attke_setup(&local_st, &sigpk);
  if (res != SGX_MPC_OK) return SGX_MPC_ERROR;

  savesync("publickey",sigpk,SGX_MPC_PUBLICKEYBYTES);
  umask(077);
  savesync("localstate",local_st,sizeof(attke_local_state));
  return SGX_MPC_OK;
}
