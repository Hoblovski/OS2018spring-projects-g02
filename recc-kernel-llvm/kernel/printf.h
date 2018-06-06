#ifndef PRINTF_H
#define PRINTF_H

#define printf(...) printf_busy(__VA_ARGS__)
unsigned printf_busy(const char * fmt, ...);
unsigned printf_direct(const char* fmt, ...);

#endif // PRINTF_H
