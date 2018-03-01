#ifndef __SGX_MPC_BOX_H
#define __SGX_MPC_BOX_H

int box_init();

int box(
  bytes *omsg,
  size *omsglen,
  const label pid,
  const bytes inmsg,
  const size inmsglen
);

#endif
