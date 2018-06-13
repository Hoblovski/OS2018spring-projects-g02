/*
    Copyright 2016 Robert Elder Software Inc.
    
    Licensed under the Apache License, Version 2.0 (the "License"); you may not 
    use this file except in compliance with the License.  You may obtain a copy 
    of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software 
    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT 
    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the 
    License for the specific language governing permissions and limitations 
    under the License.
*/
#include "user_proc.h"
#include "printf.h"
#include "private_kernel_interface.h"
#include "public_kernel_interface.h"

// should be used by user process
// user should not do putchar_busy
int putchar(int c){
	struct kernel_message putchar_m;
	struct kernel_message putchar_reply;
	putchar_m.message_type = OUTPUT_CHARACTER;
	putchar_m.data = c;
	send_message(&putchar_m, 0 /* TODO */, &putchar_reply);
	switch(putchar_reply.message_type){
		case MESSAGE_ACKNOWLEDGED:{
			break;
		}default:{
      fatal(1); // Unknown message type.
    }
  }
  return 0;
}

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

void uart1_in_ksvc(void){
	while(1){
    cur_proc->state = BLOCKED_ON_UART1_IN_READY;
    sched();
    unsigned ch = getchar_nobusy();
  }
}

void uart1_out_ksvc(void){
  unsigned tot = 5;
  putchar_busy('@');
	while(1){
    cur_proc->state = BLOCKED_ON_UART1_OUT_READY;
    sched();
    if (tot != 0) {
      tot--;
      putchar_nobusy('@');
    }
  }
}
