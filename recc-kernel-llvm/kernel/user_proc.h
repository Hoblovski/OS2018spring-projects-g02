#ifndef USER_PROC_H_
#define USER_PROC_H_

#include "kernel_state.h"

void uart1_in_ksvc(void);
void hello_msg_ksvc(void);
void uart1_out_ksvc(void);
void hello_msg_ksvc2(void);
void stupid();

#endif
