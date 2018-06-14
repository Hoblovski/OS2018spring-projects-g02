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
  unsigned dst_pids[MAX_NUM_PROCESSES];
  unsigned n_dst_pids = 0;
  while(1){
    // check for incoming register messages
    struct kernel_message* reg_msg = k_receive_message_noblock();
    if (reg_msg != NULL && n_dst_pids < MAX_NUM_PROCESSES 
        && reg_msg->type == UARTINREG) {
      dst_pids[n_dst_pids++] = reg_msg->src_pid;
    }
    // wait for input
    k_yield(BLOCKED_ON_UART1_IN_READY);
    unsigned ch = getchar_nobusy();
    printf_direct("[uartin] got char %x\n", ch);
    struct kernel_message msg = { .type = OTHER, .data = ch };
    // send message to registered processes
    k_send_message(&msg, uart1_out_pid); // display input first
    unsigned n_valid = 0;
    for (unsigned i = 0; i < n_dst_pids; i++) {
      unsigned dst_pid = dst_pids[i];
      if (pcbs[dst_pid].state != NOT_ALLOCATED) {
        dst_pids[n_valid++] = dst_pids[i];
        k_send_message(&msg, dst_pid);
      }
    }
    n_dst_pids = n_valid;
  }
}

void uart1_out_ksvc(void){
  while(1){
    k_yield(BLOCKED_ON_UART1_OUT_READY);
    struct kernel_message *msg_recv = k_receive_message();
    printf_direct("[uartout] put char %x\n", msg_recv->data);
    putchar_nobusy(msg_recv->data);
    struct kernel_message msg = { .type = REPLY, .data = 0};
    if (msg_recv->src_pid != uart1_in_pid)
      k_send_message(&msg, msg_recv->src_pid);
  }
}

void stupid(){
  unsigned n_iter = 0;
  unsigned pr = 0;
  struct kernel_message msg;
  unsigned buf[16];
  unsigned bufsz = 0;

  syscall(4, NULL);
  syscall(2, '$');
  while (1) {
    syscall(3, &msg);
    if (msg.data == 13) {
      syscall(2, '\n');
      if (buf[0] == 'q') // quit
        syscall(1, NULL);
      else if (buf[0] == 'a') { // access memory
        unsigned mem = 0;
        for (unsigned i = 2; i < 10; i++) {
          if (buf[i] >= 'a')
            mem = (mem << 4) | (buf[i] - 'a' + 10);
          else
            mem = (mem << 4) | (buf[i] - '0');
        }
        unsigned a = *(unsigned*) mem;
      }
      bufsz = 0;
      syscall(2, '$');
      // now we 
    } else {
      if (bufsz == 15) {
        bufsz = 0;
        syscall(2, '%');
        syscall(2, '\n');
        syscall(2, '$');
      } else {
        buf[bufsz++] = msg.data;
      }
    }
  }
}
