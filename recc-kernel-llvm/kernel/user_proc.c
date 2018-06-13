#include "user_proc.h"
#include "printf.h"
#include "private_kernel_interface.h"
#include "public_kernel_interface.h"

void hello_msg_ksvc(void){
  printf_direct("You're currently running a very simple microkernel that was built\n");
  printf_direct("for the purposes of demonstrating the 'One Page CPU' design, and\n");
  printf_direct("cross compiler collection.  This microkernel implements inter-process\n");
  printf_direct("communication, premptive context switching, interrupt based I/O, along\n");
  printf_direct("with a very simple timer that counts simulated clock ticks.\n");
  printf_direct("\nSome single-character commands include:\n\n");
  printf_direct("t -  Prints the number of simulated clock ticks since kernel start.\n");
  printf_direct("s -  Prints the stack pointer values of each task.\n");
  printf_direct("p -  Prints the priority of each task.\n");
}

void hello_msg_ksvc2(void){
  while (1) {
    struct kernel_message *msg = k_receive_message();
    printf_direct("Hi? hello_msg_ksvc2 got an message %x from %x\n", msg->data, msg->src_pid);
  }
}

void uart1_in_ksvc(void){
	while(1){
    cur_proc->state = BLOCKED_ON_UART1_IN_READY;
    sched();
    unsigned ch = getchar_nobusy();
    struct kernel_message msg;
    msg.type = OTHER;
    msg.data = ch;
    k_send_message(&msg, 1);
  }
}

void uart1_out_ksvc(void){
  unsigned tot = 5;
  putchar_busy('@');
	while(1){
    struct kernel_message* msg = k_receive_message();
    cur_proc->state = BLOCKED_ON_UART1_OUT_READY;
    sched();
    if (tot != 0) {
      tot--;
      putchar_nobusy('@');
    }
  }
}
