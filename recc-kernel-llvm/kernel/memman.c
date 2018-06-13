#include "memman.h"
#include "private_kernel_interface.h"

void memset(void* ptr, unsigned val, unsigned len){
  for (unsigned* i = (unsigned*) ptr; i < ((unsigned*) ptr) + len; i++)
    *i = val;
}

void memcpy(void* dst, const void* src, unsigned len){
  unsigned ass;
  if ((ass = len & 3))
    fatal(30);
  unsigned* d = dst;
  const unsigned* s = src;
  len >>= 2;
  while (len-- != 0)
    *(d++) = *(s++);
}

