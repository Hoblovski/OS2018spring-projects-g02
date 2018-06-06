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
#include "printf.h"
#include "private_kernel_interface.h"
#include "user_proc.h"
#include "queue.h"

unsigned int current_task_id = 0;
unsigned int num_clock_ticks = 0;
unsigned int saved_uart1_out_ready = 0;
unsigned int saved_uart1_in_ready = 0;

unsigned alloc_page();

void schedule_next_task(void){
	struct process_control_block * next_task;
	/*  Get the next task */
	if(task_queue_current_count(&ready_queue_p0)){
		next_task = task_queue_pop_start(&ready_queue_p0);
	}else if(task_queue_current_count(&ready_queue_p1)){
		next_task = task_queue_pop_start(&ready_queue_p1);
	}else if(task_queue_current_count(&ready_queue_p2)){
		next_task = task_queue_pop_start(&ready_queue_p2);
	}else{
		next_task = task_queue_pop_start(&ready_queue);
	}
	next_task->state = ACTIVE;
	/*  Set its stack pointer */
	g_current_sp = next_task->ustack;
	current_task_id = next_task->pid;
}

void unblock_tasks_for_event(enum kernel_event event){
	switch (event){
		case CLOCK_TICK_EVENT:{
			struct process_control_block * unblocked_task;
			if(task_queue_current_count(&blocked_on_clock_tick_queue)){
				unblocked_task = task_queue_pop_start(&blocked_on_clock_tick_queue);
				mark_task_ready(unblocked_task);
			}
			break;
		}case UART1_OUT_READY:{
			struct process_control_block * unblocked_task;
			if(task_queue_current_count(&blocked_on_uart1_out_ready_queue) == 0){
				/*  Nothing has blocked on this event yet so save the signal */
        if (saved_uart1_out_ready)
          fatal(9);
        // There should be no previous saved uart signal.  Expect output problems.
				saved_uart1_out_ready = 1;
			}else{
				unblocked_task = task_queue_pop_start(&blocked_on_uart1_out_ready_queue);
				mark_task_ready(unblocked_task);
			}
			break;
		}case UART1_IN_READY:{
			struct process_control_block * unblocked_task;
			if(task_queue_current_count(&blocked_on_uart1_in_ready_queue) == 0){
				/*  Nothing has blocked on this event yet so save the signal */
				if (saved_uart1_in_ready)
          fatal(10);
        // There should be no previous saved uart signal.  Expect input problems.
				saved_uart1_in_ready = 1;
			}else{
				unblocked_task = task_queue_pop_start(&blocked_on_uart1_in_ready_queue);
				mark_task_ready(unblocked_task);
			}
			break;
		}default:{
      fatal(11); // Unhandled unblock event.
			break;
		}
	}
	scheduler();
}

void mark_task_ready(struct process_control_block * pcb){
	pcb->state = READY;
	switch(pcb->priority){
		case 0:{
			task_queue_push_end(&ready_queue_p0, pcb);
			break;
		}case 1:{
			task_queue_push_end(&ready_queue_p1, pcb);
			break;
		}case 2:{
			task_queue_push_end(&ready_queue_p2, pcb);
			break;
		}default:{
			task_queue_push_end(&ready_queue, pcb);
		}
	}
}

void save_current_task_as_ready(void){
	/*  Save the stack pointer of the current task */
	pcbs[current_task_id].ustack = g_current_sp;
	/*  Put current task back on ready queue */
	mark_task_ready(&pcbs[current_task_id]);
}

void save_current_task(struct task_queue * queue, enum process_state state){
	/*  Save the stack pointer of the current task */
	pcbs[current_task_id].ustack = g_current_sp;
	/*  Put current task back on queue */
	pcbs[current_task_id].state = state;
	task_queue_push_end(queue, &pcbs[current_task_id]);
}

unsigned int scheduler(void){
	save_current_task_as_ready();
	schedule_next_task();
	return 3;
}

void k_task_exit(void){
	save_current_task(&zombie_queue, ZOMBIE);
	schedule_next_task();
}

unsigned int k_release_processor(void){
	return scheduler();
}

void k_block_on_event(enum kernel_event event){
	switch(event){
		case CLOCK_TICK_EVENT:{
			save_current_task(&blocked_on_clock_tick_queue, BLOCKED_ON_CLOCK_TICK);
			schedule_next_task();
			break;
		}case UART1_OUT_READY:{
			if(saved_uart1_out_ready){
				/*  Use up the saved signal, and don't block the task. */
				saved_uart1_out_ready = 0;
			}else{
				save_current_task(&blocked_on_uart1_out_ready_queue, BLOCKED_ON_UART1_OUT_READY);
				schedule_next_task();
			}
			break;
		}case UART1_IN_READY:{
			if(saved_uart1_in_ready){
				/*  Use up the saved signal, and don't block the task. */
				saved_uart1_in_ready = 0;
			}else{
				save_current_task(&blocked_on_uart1_in_ready_queue, BLOCKED_ON_UART1_IN_READY);
				schedule_next_task();
			}
			break;
		}default:{
			fatal(12); // Blocking on unknown event.
		}
	}
}

void k_irq_handler(void){
	unsigned int flags_register = read_flags_register();
	if(flags_register & PAGE_FAULT_EXCEPTION_ASSERTED_BIT){
		or_into_flags_register(HALTED_BIT); /*  Halt the processor for now */
		/*  De-assert the bit last, so we can detect nested page fault exceptions */
		deassert_bits_in_flags_register(PAGE_FAULT_EXCEPTION_ASSERTED_BIT);
	}else if(flags_register & UART1_OUT_ASSERTED_BIT){
		deassert_bits_in_flags_register(UART1_OUT_ASSERTED_BIT);
		unblock_tasks_for_event(UART1_OUT_READY);
	}else if(flags_register & UART1_IN_ASSERTED_BIT){
		deassert_bits_in_flags_register(UART1_IN_ASSERTED_BIT);
		unblock_tasks_for_event(UART1_IN_READY);
	}else if(flags_register & TIMER1_ASSERTED_BIT){
		deassert_bits_in_flags_register(TIMER1_ASSERTED_BIT);
		num_clock_ticks++;
		unblock_tasks_for_event(CLOCK_TICK_EVENT);
	}else{
		/*  Something really bad happend. */
		/*  Busy print will affect flags, but in this case everything is broken anyway */
		printf_busy("IRQ FAILURE!\n");
    fatal(20);
	}
}

void k_send_message(struct kernel_message * message, unsigned int destination_pid, struct kernel_message * reply){
	/*  Remember where to store the reply */
	pcbs[current_task_id].reply_message = reply;
	message->source_id = current_task_id;
	if(pcbs[destination_pid].state == BLOCKED_ON_SEND){
		/*  The destination is already blocked on our message send */
		pcbs[current_task_id].state = BLOCKED_ON_RECEIVE;
		mark_task_ready(&pcbs[destination_pid]);
		*(pcbs[destination_pid].recieve_message) = *message;
	}else{
		/*  The destination has not asked for the message yet */
		message_queue_push_end(&pcbs[destination_pid].messages, *message);
		pcbs[current_task_id].state = BLOCKED_ON_REPLY;
	}
	pcbs[current_task_id].ustack = g_current_sp;
	schedule_next_task();
}

void k_receive_message(struct kernel_message * message){
	if(message_queue_current_count(&pcbs[current_task_id].messages) == 0){
		pcbs[current_task_id].state = BLOCKED_ON_SEND;
		pcbs[current_task_id].ustack = g_current_sp;
		pcbs[current_task_id].recieve_message = message;
	}else{
		struct kernel_message m;
		m = message_queue_pop_start(&pcbs[current_task_id].messages);
		*message = m;
		pcbs[message->source_id].state = BLOCKED_ON_REPLY;
		save_current_task_as_ready();
	}
	schedule_next_task();
}

void k_reply_message(struct kernel_message * message, unsigned int destination_pid){
	*(pcbs[destination_pid].reply_message) = *message;
	/*  Unblock the destination task */
	mark_task_ready(&pcbs[destination_pid]);
	/*  Save current task and continue */
	save_current_task_as_ready();
	schedule_next_task();
}

void k_kernel_exit(void){
	/*  Don't need to save any state because we're exiting kernel */
	struct process_control_block * next_task = &pcbs[0]; /* Main entry SP is stored in PCB 0 */
	g_current_sp = next_task->ustack;
}

void set_irq_handler(void (*irq_handler_fcn)(void)){
	void (**irq_handler_fcn_location)(void) = (void (**)(void))IRQ_HANDLER;
	*irq_handler_fcn_location = irq_handler_fcn;
}

void set_timer_period(unsigned int period){
	unsigned int * period_location = (unsigned int *)TIMER_PERIOD;
	*period_location = period;
}

void timer_interrupt_enable(void){
	or_into_flags_register(TIMER1_ENABLE_BIT);
}

void uart1_out_interrupt_enable(void){
	or_into_flags_register(UART1_OUT_ENABLE_BIT);
}

void uart1_in_interrupt_enable(void){
	or_into_flags_register(UART1_IN_ENABLE_BIT);
}

void paging_enable(void){
	or_into_flags_register(PAGEING_ENABLE_BIT);
}

static void new_thread(unsigned int pid, unsigned int priority,
    unsigned int* sp, void (*func)(void))
{
  /* pid 0 is the init process */
  pcbs[pid].pid = pid;
	pcbs[pid].priority = priority;
	pcbs[pid].ustack = sp;
	if (pid != 0) {
    init_task_stack(&(pcbs[pid].ustack), func);
    mark_task_ready(&pcbs[pid]);
  }
  message_queue_init(&pcbs[pid].messages, MAX_NUM_PROCESSES);
}

void mm_init(){
  // initialize pages
  pages = (struct page*) IMAGE_END;
  // page[i]: refer to physical address i*PGSZ ~ (i+1)*PGSZ
  n_pages = MEMSIZE >> PAGE_SIZE_WIDTH;
  n_kpages = KMEMSIZE >> PAGE_SIZE_WIDTH;
  // always reserve the first n_kpages for kernel
  //  TODO: is this a stupid design?
  for (unsigned i = 0; i < n_kpages; i++) {
    pages[i].ref = 0;   // for reserved ones, ref won't be used
    pages[i].flags = PF_KRESERVE;
  }
  // remaining pages usable
  for (unsigned i = n_kpages; i < n_pages; i++) {
    pages[i].ref = 0;   // not used, can be alloc'ed
    pages[i].flags = 0;
  }
}

// returns physical address
//  kernel is able to directly access physical address
unsigned alloc_page(){
  for (int i = n_kpages; i < n_pages; i++)
    if (pages[i].ref == 0) {
      unsigned fr = read_flags_register();
      deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);
      { // interrupt not enabled
        pages[i].ref = 1;
      }
      write_flags_register(fr);
      // zero alloc'ed page
      // XXX: move this code out
      unsigned* page_beg = (unsigned*) 0xC0000000 + (i << PTSHIFT);
      unsigned* page_end = (unsigned*) 0xC0000000 + ((i+1) << PTSHIFT);
      for (unsigned* j = page_beg; j < page_end; j++)
        *j = 0;
      return (i<<PAGE_SIZE_WIDTH);
    }
  fatal(17); // out of memory
  return 0;
}

// when a process gives up the page frame
//  pa must be aligned
void release_page(unsigned pa){
  unsigned fr = read_flags_register();
  deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);
  { // interrupt not enabled
    unsigned pgidx = pa >> PAGE_SIZE_WIDTH;
    pages[pgidx].ref--;
  }
  write_flags_register(fr);
}

// returns linear address to pte
//  linear address is only used for user programs
//  they begin from 0x100 (idea from M$ .com)
pte_t* get_pte(pde_t* pd, unsigned la, unsigned alloc_for_pt){
  pde_t* pde;
  pte_t* pte;
  pde = &(pd[GET_PDIDX(la)]);
  // stupid compiler hack
  if ((*pde & PDE_FLAGS_P) == PDE_FLAGS_P) {
    // page table present
    pte = (pte_t*) GET_PN(*pde);
    pte = &(pte[GET_PTIDX(la)]);
    return pte;
  }
  // page table not present, alloc a page frame for page table
  if (alloc_for_pt) {
    unsigned t;
    t = alloc_page();
    *pde = PA2KLA(t) | PDE_FLAGS_P;
    pte = (pte_t*) GET_PN(*pde);
    pte = &(pte[GET_PTIDX(la)]);
    return pte;
  } else {
    return 0;
  }
}

// with the given paging (pd),
//  if la is not mapped to pa (either not mapped or mapped to another place)
//  then establish a mapping from la to pa
void map_segment(pde_t* pd, unsigned la, unsigned pa) {
  if (GET_POFFSET(la) != GET_POFFSET(pa))
    fatal(18);
  la = GET_PN(la);
  pa = GET_PN(pa);
  pte_t* pte = get_pte(pd, la, 1);

  if (GET_PN(*pte) == pa) {
    // present, la maps to pa
  } else {
    release_page(GET_PN(*pte));
    // offset of *pte is the flags
    *pte = pa | GET_POFFSET(*pte) | PTE_FLAGS_P;
  }
}

struct process_control_block* alloc_proc(void) {
  unsigned rv = 0;
  for (rv = 0; rv < MAX_NUM_PROCESSES; rv++)
    if (pcbs[rv].state == NOT_ALLOCATED)
      break;
  // new process id is rv
  pcbs[rv].pid = rv;
  pcbs[rv].state = NOT_INITIALIZED;
  pcbs[rv].ustack = NULL;
  pcbs[rv].priority = ~0u;
  message_queue_init(&(pcbs[rv].messages), MAX_NUM_PROCESSES);
  pcbs[rv].reply_message = NULL;
  pcbs[rv].recieve_message = NULL;
  return &(pcbs[rv]);
}

void stupid_proc_init(void){
	task_queue_init(&ready_queue_p0);
	task_queue_init(&ready_queue_p1);
	task_queue_init(&ready_queue_p2);
	task_queue_init(&ready_queue);
	task_queue_init(&zombie_queue);
	task_queue_init(&blocked_on_clock_tick_queue);
	task_queue_init(&blocked_on_uart1_out_ready_queue);
	task_queue_init(&blocked_on_uart1_in_ready_queue);

  for (unsigned i = 0; i < MAX_NUM_PROCESSES; i++)
    pcbs[i].state = NOT_ALLOCATED;

  // the first user process can't be created by fork
  idle_proc = alloc_proc();
  idle_proc->pgdir = 0; // idle will never use useg
  idle_proc->kstack = (void*) PA2KLA(alloc_page()); // 1 page is enough
  idle_proc->ustack = 0; // idle will never goto user mode
  idle_proc->state = ACTIVE; // idle is right running
  if (idle_proc->pid != 0) fatal(21);
  idle_proc->priority = ~0u; // least priority
  // idle will never send / recv messages
  cur_proc = idle_proc;
}

void sched(void)
{
  if (cur_proc->state == ACTIVE)
    cur_proc->state = READY;
  // select process `next_proc` to swap in
  struct process_control_block* next_proc = NULL;
  unsigned min_priority = ~0u;
  for (unsigned i = 0; i < MAX_NUM_PROCESSES; i++)
    if (pcbs[i].state == READY && pcbs[i].priority < min_priority) {
      min_priority = pcbs[i].priority;
      next_proc = &(pcbs[i]);
    }
  //
  // context switch
  printf_direct("switching from %X to %X\n",
      cur_proc->pid, next_proc->pid);
  printf_direct("0\n");
  switch_kstack(&(next_proc->kstack), &(cur_proc->kstack));
  printf_direct("1\n");
  use_pgdir(next_proc->pgdir);
  printf_direct("2\n");
  cur_proc = next_proc;
  printf_direct("3\n");
}

void k_kernel_init(void){
  mm_init();
  stupid_proc_init();

	set_irq_handler(irq_handler); /*  Set before paging is enabled, otherwise a page fault doesn't know where to go */
	set_timer_period(INITIAL_TIMER_PERIOD_VALUE);
	timer_interrupt_enable();
	uart1_out_interrupt_enable();
	uart1_in_interrupt_enable();
  paging_enable();
  deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);

  // finished, idle process keeps trying to scheduling to other tasks
  printf_direct("Hello world\n");
  while (1) {
    sched();
    // schedule_next_task();
  }
}
