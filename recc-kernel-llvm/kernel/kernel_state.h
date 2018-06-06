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
  BLOCKED_ON_SEND, BLOCKED_ON_RECEIVE, BLOCKED_ON_REPLY,
  BLOCKED_ON_CLOCK_TICK, BLOCKED_ON_UART1_IN_READY, BLOCKED_ON_UART1_OUT_READY,
  READY, ACTIVE, ZOMBIE,
  NOT_ALLOCATED, NOT_INITIALIZED,
};


enum kernel_event {
  CLOCK_TICK_EVENT, UART1_OUT_READY, UART1_IN_READY
};


enum kernel_message_type {
  UART1_IN_READY_NOTIFY, UART1_OUT_READY_NOTIFY, CLOCK_TICK_NOTIFY,
  MESSAGE_ACKNOWLEDGED, OUTPUT_CHARACTER
};


typedef unsigned pte_t;
typedef unsigned pde_t;


struct kernel_message{
	enum kernel_message_type message_type;
	unsigned int data;
	unsigned int source_id;
};


struct task_queue {
	unsigned int start;
	unsigned int end;
	unsigned int current_count;
	void * items[MAX_NUM_PROCESSES];
};


struct message_queue {
	unsigned int start;
	unsigned int end;
	unsigned int current_count;
	unsigned int size;
	struct kernel_message items[MAX_NUM_PROCESSES];
};


/*
 * PCB:
 *  PCB->priority is divided into 4 classes, from 0(highest) to >3(lowest).
 */
struct process_control_block{
	void* ustack;
  void* kstack;
  void* pgdir;
	enum process_state state;
	unsigned int pid;
	unsigned int priority;
	struct message_queue messages;
	struct kernel_message * reply_message;
	struct kernel_message * recieve_message;
};


struct page {
  unsigned ref;
  unsigned flags;
};


extern struct task_queue ready_queue_p0;
extern struct task_queue ready_queue_p1;
extern struct task_queue ready_queue_p2;
extern struct task_queue ready_queue;
extern struct task_queue zombie_queue;
extern struct task_queue blocked_on_clock_tick_queue;
extern struct task_queue blocked_on_uart1_out_ready_queue;
extern struct task_queue blocked_on_uart1_in_ready_queue;

extern unsigned int * g_kernel_sp;
extern unsigned int * g_current_sp;
extern unsigned int kernel_stack[STACK_SIZE];
extern struct process_control_block pcbs[MAX_NUM_PROCESSES];
extern struct process_control_block* cur_proc;
extern struct process_control_block* idle_proc;
extern unsigned int user_proc_1_stack[STACK_SIZE];
extern unsigned int user_proc_2_stack[STACK_SIZE];
extern unsigned int user_proc_3_stack[STACK_SIZE];
extern unsigned int user_proc_4_stack[STACK_SIZE];
extern unsigned int user_proc_5_stack[STACK_SIZE];
extern unsigned int user_proc_6_stack[STACK_SIZE];
extern unsigned int user_proc_7_stack[STACK_SIZE];

extern unsigned n_pages;
extern unsigned n_kpages;
extern struct page* pages;
#endif
