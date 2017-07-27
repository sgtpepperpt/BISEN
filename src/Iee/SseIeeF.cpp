#include "SseIee.hpp"
#include "SseIeeF.hpp"
#include <stdlib.h>

using namespace std;

SseIee sse;

void f(
  char **out,
  size_t *outlen,
  size_t pid,
  char *in,
  size_t inlen
)
{
  *outlen = sse.f(in, (int)inlen, out);
}
