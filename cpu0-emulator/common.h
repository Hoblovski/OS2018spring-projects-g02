#ifndef COMMON_H
#define COMMON_H

#define UART_INIT_WAIT 10000
#define UART_RANDOM_WAIT (rand() % 100000)

#define PERM_CHECK

#define EXCEP_WATCH

#define WATCH_UART_OUT_DIRECT

#undef INSTR_WATCH
#define INSTR_WATCH_START_FROM_FIRST_TIMERINT

#undef WATCH_DEBUG_STD_OUTPUT

#define Printf(...) do { printf (__VA_ARGS__); fflush(stdout); } while (0)

#endif // COMMON_H
