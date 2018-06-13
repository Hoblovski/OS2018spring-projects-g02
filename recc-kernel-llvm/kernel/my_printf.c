#include <stdarg.h>
#include "private_kernel_interface.h"

unsigned loadbyteu(const char* a)
{
  unsigned r = ((unsigned) a) & 3;
  unsigned rv = *(unsigned*) (((unsigned) a) & ~3);
  return (rv >> (r << 3)) & 0xFF;
}


const char dig2hex[] = "0123456789abcdef";
const char dig2hexbig[] = "0123456789ABCDEF";

void put_hex_uart(unsigned v, unsigned bigletter)
{
  unsigned outp[8];
  unsigned ptr = 0;
  const char* d2h;
  if (bigletter)
    d2h = dig2hexbig;
  else
    d2h = dig2hex;
  if (v == 0) {
    putchar_busy('0');
    return;
  }
  while (v) {
    unsigned l4b = v & 0xF;
    unsigned ctp = '0' + l4b;
    if (l4b > 9) {
      if (bigletter)
        ctp = 'A' + l4b - 10;
      else
        ctp = 'a' + l4b - 10;
    }
    outp[ptr++] = ctp;
    v >>= 4;
  }
  ptr--;
  while (ptr != ~0u)
    putchar_busy(outp[ptr--]);
}


// by default is busy
unsigned printf_busy(const char* fmt, ...)
{
  va_list al;
  va_start(al, fmt);

  unsigned i = 0;
  unsigned spec_found = 0;
  while (1) {
    i = loadbyteu(fmt);
    if (i == 0)
      break;
    if (!spec_found) {
      if (i == '%')
        spec_found = 1;
      else
        putchar_busy(i);
    } else {
      switch (i) {
        case 'x':
          put_hex_uart(va_arg(al, unsigned), 0);
          break;
        case 'X':
          put_hex_uart(va_arg(al, unsigned), 1);
          break;
        default:
          putchar_busy(i);
          break;
      }
      spec_found = 0;
    }
    fmt++;
  }
  va_end(al);
  return 0;
}

// direct printf using bkdoor of emulator

void put_hex_uart_direct(unsigned v, unsigned bigletter)
{
  unsigned outp[8];
  unsigned ptr = 0;
  const char* d2h;
  if (bigletter)
    d2h = dig2hexbig;
  else
    d2h = dig2hex;
  if (!v) {
    *(unsigned*) 0xFFFFFFF0 = '0';
    return;
  }
  while (v) {
    unsigned l4b = v & 0xF;
    unsigned ctp = '0' + l4b;
    if (l4b > 9) {
      if (bigletter)
        ctp = 'A' + l4b - 10;
      else
        ctp = 'a' + l4b - 10;
    }
    outp[ptr++] = ctp;
    v >>= 4;
  }
  ptr--;
  while (ptr != ~0u)
    *(unsigned*) 0xFFFFFFF0 = outp[ptr--];
}


unsigned printf_direct(const char* fmt, ...)
{
  va_list al;
  va_start(al, fmt);

  unsigned i = 0;
  unsigned spec_found = 0;
  while (1) {
    i = loadbyteu(fmt);
    if (i == 0)
      break;
    if (!spec_found) {
      if (i == '%')
        spec_found = 1;
      else
        *(unsigned*) 0xFFFFFFF0 = i;
    } else {
      switch (i) {
        case 'x':
          put_hex_uart_direct(va_arg(al, unsigned), 0);
          break;
        case 'X':
          put_hex_uart_direct(va_arg(al, unsigned), 1);
          break;
        default:
          *(unsigned*) 0xFFFFFFF0 = i;
          break;
      }
      spec_found = 0;
    }
    fmt++;
  }
  va_end(al);
  return 0;
}
