#include "memman.h"
#include "private_kernel_interface.h"

void memsetw(void* ptr, unsigned val, unsigned count){
  for (unsigned* i = (unsigned*) ptr; i < ((unsigned*) ptr) + count; i++)
    *i = val;
}

void memcpyw(void* dst, const void* src, unsigned count){
  unsigned ass;
  unsigned* d = dst;
  const unsigned* s = src;
  while (count-- != 0)
    *(d++) = *(s++);
}

unsigned loadbyteu(const char* a){
  unsigned r = ((unsigned) a) & 3;
  unsigned rv = *(unsigned*) (((unsigned) a) & ~3);
  return (rv >> (r << 3)) & 0xFF;
}


