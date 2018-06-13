#include "user_proc.h"
#include "printf.h"
#include "private_kernel_interface.h"
#include "public_kernel_interface.h"

void hello_msg_ksvc(void){
  printf("You're currently running the micro micro-kernel created\n");
  printf("as the project of opearting systems course of Tsinghua \n");
  printf("university. This micro micro-kernel implemented lab1,  \n");
  printf("lab2, lab3, lab4, lab5 and possibly lab6.\n");
}

void uart1_in_ksvc(void){
  while(1){
    cur_proc->state = BLOCKED_ON_UART1_IN_READY;
    sched();
    unsigned ch = getchar_nobusy();
    struct kernel_message msg = {
      .type = OTHER,
      .data = ch
    };
    k_send_message(&msg, uart1_out_pid);
  }
}

void uart1_out_ksvc(void){
  while(1){
    k_yield(BLOCKED_ON_UART1_OUT_READY);
    struct kernel_message *msg = k_receive_message();
    putchar_busy(msg->data);
  }
}

void stupid(){
  unsigned n_iter = 0;
  while (1) {
    n_iter++;
    // compiler hack
    unsigned ass;
    if ((ass = n_iter & 0x3FFF) == 0) {
      *(unsigned*) 0xFFFFFFF0 = '~';
    }
  }
}
