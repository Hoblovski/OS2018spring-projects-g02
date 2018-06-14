#ifndef KERNEL_STATE_H_
#define KERNEL_STATE_H_
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

#include "kernel_defs.h"

enum process_state {
  BLOCKED_ON_MESSAGE         = 2,
  BLOCKED_ON_CLOCK_TICK      = 4,
  BLOCKED_ON_UART1_IN_READY  = 5,
  BLOCKED_ON_UART1_OUT_READY = 6,
  READY                      = 7,
  ZOMBIE                     = 9,
  NOT_ALLOCATED              = 10,
  NOT_INITIALIZED            = 11,
};


enum kernel_event {
  CLOCK_TICK_EVENT, UART1_OUT_READY, UART1_IN_READY
};


enum kernel_message_type {
  OTHER, REPLY, UARTINREG
};


typedef unsigned pte_t;
typedef unsigned pde_t;


struct kernel_message{
	enum kernel_message_type type;
	unsigned int data;
	unsigned int src_pid;
};


struct message_queue {
	unsigned int start;
	unsigned int end;
	struct kernel_message items[MAX_NUM_PROCESSES];
};


/*
 * PCB:
 *  PCB->priority is divided into 4 classes, from 0(highest) to >3(lowest).
 */
struct process_control_block{
	void* ustack; // 0  might be actually not user stack, just misnomer
  void* kstack; // 4
  void* pgdir; // 8
	enum process_state state;
	unsigned int pid;
	unsigned int priority;
  struct message_queue mq;
};


struct page {
  unsigned ref;
  unsigned flags;
};


extern struct process_control_block pcbs[MAX_NUM_PROCESSES];
extern struct process_control_block* cur_proc;
extern struct process_control_block* idle_proc;
extern unsigned int init_stack[STACK_SIZE];

extern unsigned n_pages;
extern unsigned n_kpages;
extern struct page* pages;

// no one should ever modify them since assignment!
extern unsigned uart1_in_pid;
extern unsigned uart1_out_pid;
#endif
