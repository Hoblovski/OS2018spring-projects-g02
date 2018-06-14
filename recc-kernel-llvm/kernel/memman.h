#ifndef MEMMAN_H
#define MEMMAN_H

void memsetw(void* ptr, unsigned val, unsigned count);
void memcpyw(void* dst, const void* src, unsigned count);
unsigned loadbyteu(const char* a);


#endif // MEMMAN_H
