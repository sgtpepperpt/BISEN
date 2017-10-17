#ifndef __SGX_MPC_SCHEDULER_H
#define __SGX_MPC_SCHEDULER_H

#include "message_queue.h"

int scheduler_init();

int scheduler(
  bytes *omsg,
  size *omsglen,
  label *olabel,
  message_queue* *pending_messages
);

#endif
