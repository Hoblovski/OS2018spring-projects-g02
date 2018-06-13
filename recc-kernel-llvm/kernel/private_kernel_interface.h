#ifndef PRIVATE_KERNEL_INTERFACE_H_
#define PRIVATE_KERNEL_INTERFACE_H_
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

#include "kernel_state.h"

void schedule_next_task(void);
void save_current_task(struct task_queue *, enum process_state);
void save_current_task_as_ready(void);
void mark_task_ready(struct process_control_block *);

void sched(void);

void set_irq_handler(void (*)(void));
void set_timer_period(unsigned int);
void set_level_2_page_pointer(unsigned int *);
void or_into_flags_register(unsigned int);
void deassert_bits_in_flags_register(unsigned int);
unsigned int read_flags_register(void);
void write_flags_register(unsigned);
void fatal(unsigned errno);

void timer_interrupt_enable(void);
void uart1_out_interrupt_enable(void);
void uart1_in_interrupt_enable(void);
void paging_enable(void);

unsigned int init_task_stack(unsigned int **, void (*)(void));
void scheduler(void);
void k_release_processor(void);
void k_task_exit(void);
void k_kernel_exit(void);
void k_kernel_init(void);
void irq_handler(void);
void k_irq_handler(void);

unsigned k_send_message(struct kernel_message * msg, unsigned dstpid);
struct kernel_message* k_receive_message(void);
void k_yield(enum process_state state);

int putchar_nobusy(int);
int getchar_nobusy(void);
unsigned putchar_busy(unsigned);

void use_pgdir(void* pgdir);

unsigned calloc_page();
void release_page(unsigned pa);
pte_t* get_pte(pde_t* pd, unsigned la, unsigned alloc_for_pt);

void switch_kstack(struct process_control_block* newproc, struct process_control_block* oldproc);
#endif
