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

#define COLOR_OUTPUT

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#endif // COMMON_H
