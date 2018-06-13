#ifndef COMMON_H
#define COMMON_H

#define UART_INIT_WAIT 6000
#define UART_RANDOM_WAIT (rand() % 6000)

#define PERM_CHECK

#undef EXCEP_WATCH

#define WATCH_UART_OUT_DIRECT

#undef INSTR_WATCH
#define INSTR_WATCH_START_FROM_FIRST_TIMERINT

#undef WATCH_DEBUG_STD_OUTPUT

#define Printf(...) do { printf (__VA_ARGS__); fflush(stdout); } while (0)

#endif // COMMON_H
