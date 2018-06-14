#ifndef COMMON_H
#define COMMON_H

#undef LOADER_ALLOC_STACK
#ifdef LOADER_ALLOC_STACK
#define STACK_SIZE 0x10000
#define STACK_POS 0xF0000000
#endif

#define UART_INIT_WAIT 6000
#define UART_RANDOM_WAIT (rand() % 6000)

#define MAPPED_POS 0x300000
#define MAPPED_SIZE 0x100

#define PERM_CHECK

#undef EXCEP_WATCH

#define WATCH_UART_OUT_DIRECT

#undef INSTR_WATCH

#undef MEM_W_WATCH

#undef MEM_R_WATCH

#undef WATCH_DEBUG_STD_OUTPUT

#define Printf(...) do { printf (__VA_ARGS__); fflush(stdout); } while (0)

#endif // COMMON_H
