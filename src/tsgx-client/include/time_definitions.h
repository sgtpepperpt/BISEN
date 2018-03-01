#ifndef __SMC_TIME_DEFS_H__
#define __SMC_TIME_DEFS_H__

typedef unsigned long long timepoint_t;

static __inline__ timepoint_t rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi)<<32);
}

#endif
