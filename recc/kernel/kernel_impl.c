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
#include "printf_busy.h"
#include "private_kernel_interface.h"
#include "../emulators/c/op-cpu.h"
#include <assert.h>

unsigned int current_task_id = 0;
unsigned int num_clock_ticks = 0;
unsigned int saved_uart1_out_ready = 0;
unsigned int saved_uart1_in_ready = 0;
unsigned int current_timer_period = 0xA000;
unsigned int interrupts_this_period = 0;

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
	g_current_sp = next_task->stack_pointer;
	current_task_id = next_task->pid;
}

void unblock_tasks_for_event(enum kernel_event event){
	switch (event){
		case CLOCK_TICK_EVENT:{
			struct process_control_block * unblocked_task;
			if(task_queue_current_count(&blocked_on_clock_tick_queue)){
				unblocked_task = task_queue_pop_start(&blocked_on_clock_tick_queue); 
				add_task_to_ready_queue(unblocked_task);
			}
			break;
		}case UART1_OUT_READY:{
			struct process_control_block * unblocked_task;
			if(task_queue_current_count(&blocked_on_uart1_out_ready_queue) == 0){
				/*  Nothing has blocked on this event yet so save the signal */
				assert(!saved_uart1_out_ready && "There should be no previous saved uart signal.  Expect output problems.");
				saved_uart1_out_ready = 1;
			}else{
				unblocked_task = task_queue_pop_start(&blocked_on_uart1_out_ready_queue); 
				add_task_to_ready_queue(unblocked_task);
			}
			break;
		}case UART1_IN_READY:{
			struct process_control_block * unblocked_task;
			if(task_queue_current_count(&blocked_on_uart1_in_ready_queue) == 0){
				/*  Nothing has blocked on this event yet so save the signal */
				assert(!saved_uart1_in_ready && "There should be no previous saved uart signal.  Expect input problems.");
				saved_uart1_in_ready = 1;
			}else{
				unblocked_task = task_queue_pop_start(&blocked_on_uart1_in_ready_queue); 
				add_task_to_ready_queue(unblocked_task);
			}
			break;
		}default:{
			assert(0 && "Unhandled unblock event.\n");
			break;
		}
	}
	scheduler();
}

void add_task_to_ready_queue(struct process_control_block * pcb){
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
	pcbs[current_task_id].stack_pointer = g_current_sp;
	/*  Put current task back on ready queue */
	add_task_to_ready_queue(&pcbs[current_task_id]);
}

void save_current_task(struct task_queue * queue, enum process_state state){
	/*  Save the stack pointer of the current task */
	pcbs[current_task_id].stack_pointer = g_current_sp;
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
			assert(0 && "Blocking on unknown event.\n");
		}
	}
}

void k_irq_handler(void){
	unsigned int flags_register = read_flags_register();
	if(flags_register & PAGE_FAULT_EXCEPTION_ASSERTED_BIT){
		/*  Handle page fault exception 
		handle_page_fault_exception();*/
		or_into_flags_register(HALTED_BIT); /*  Halt the processor for now */
		/*  De-assert the bit last, so we can detect nested page fault exceptions */
		deassert_bits_in_flags_register(PAGE_FAULT_EXCEPTION_ASSERTED_BIT);
	}else if(flags_register & TIMER1_ASSERTED_BIT){
		deassert_bits_in_flags_register(TIMER1_ASSERTED_BIT);
		num_clock_ticks++;
		unblock_tasks_for_event(CLOCK_TICK_EVENT);
	}else if(flags_register & UART1_OUT_ASSERTED_BIT){
		deassert_bits_in_flags_register(UART1_OUT_ASSERTED_BIT);
		unblock_tasks_for_event(UART1_OUT_READY);
	}else if(flags_register & UART1_IN_ASSERTED_BIT){
		deassert_bits_in_flags_register(UART1_IN_ASSERTED_BIT);
		unblock_tasks_for_event(UART1_IN_READY);
	}else{
		/*  Something really bad happend. */
		/*  Busy print will affect flags, but in this case everything is broken anyway */
		printf_busy("IRQ FAILURE!\n");
		or_into_flags_register(HALTED_BIT); /*  Halt the processor */
	}

	/*  Used as a stress test for the kernel:  Try out all interleavings of interrupts and timer periods */
	if(interrupts_this_period == 20){
		interrupts_this_period = 0;
		current_timer_period++;
		set_timer_period(current_timer_period);
	}
	interrupts_this_period++;
	scheduler();
}

void k_send_message(struct kernel_message * message, unsigned int destination_pid, struct kernel_message * reply){
	/*  Remember where to store the reply */
	pcbs[current_task_id].reply_message = reply;
	message->source_id = current_task_id;
	if(pcbs[destination_pid].state == BLOCKED_ON_SEND){
		/*  The destination is already blocked on our message send */
		pcbs[current_task_id].state = BLOCKED_ON_RECEIVE;
		add_task_to_ready_queue(&pcbs[destination_pid]);
		*(pcbs[destination_pid].recieve_message) = *message;
	}else{
		/*  The destination has not asked for the message yet */
		message_queue_push_end(&pcbs[destination_pid].messages, *message); 
		pcbs[current_task_id].state = BLOCKED_ON_REPLY;
	}
	pcbs[current_task_id].stack_pointer = g_current_sp;
	schedule_next_task();
}

void k_receive_message(struct kernel_message * message){
	if(message_queue_current_count(&pcbs[current_task_id].messages) == 0){
		pcbs[current_task_id].state = BLOCKED_ON_SEND;
		pcbs[current_task_id].stack_pointer = g_current_sp;
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
	add_task_to_ready_queue(&pcbs[destination_pid]);
	/*  Save current task and continue */
	save_current_task_as_ready();
	schedule_next_task();
}

void k_kernel_exit(void){
	/*  Don't need to save any state because we're exiting kernel */
	struct process_control_block * next_task = &pcbs[0]; /* Main entry SP is stored in PCB 0 */
	g_current_sp = next_task->stack_pointer;
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

void page_fault_exception_interrupt_enable(void){
	or_into_flags_register(PAGEING_ENABLE_BIT);
}

static void new_thread(unsigned int pid, unsigned int priority,
    unsigned int* sp, void (*func)(void))
{
  /* pid 0 is the init process */
  pcbs[pid].pid = pid;
	pcbs[pid].priority = priority;
	pcbs[pid].stack_pointer = sp;
	if (pid != 0) {
    init_task_stack(&(pcbs[pid].stack_pointer), func);
    add_task_to_ready_queue(&pcbs[pid]);
  }
  message_queue_init(&pcbs[pid].messages, MAX_NUM_PROCESSES);
}

void k_kernel_init(void){
	unsigned int i;

	set_irq_handler(irq_handler); /*  Set before paging is enabled, otherwise a page fault doesn't know where to go */

	task_queue_init(&ready_queue_p0, MAX_NUM_PROCESSES);
	task_queue_init(&ready_queue_p1, MAX_NUM_PROCESSES);
	task_queue_init(&ready_queue_p2, MAX_NUM_PROCESSES);
	task_queue_init(&ready_queue, MAX_NUM_PROCESSES);
	task_queue_init(&zombie_queue, MAX_NUM_PROCESSES);
	task_queue_init(&blocked_on_clock_tick_queue, MAX_NUM_PROCESSES);
	task_queue_init(&blocked_on_uart1_out_ready_queue, MAX_NUM_PROCESSES);
	task_queue_init(&blocked_on_uart1_in_ready_queue, MAX_NUM_PROCESSES);

	pcbs[0].state = ACTIVE; 
  /*  Task 0 is not really a task, it is the 'int main' that we might want to return to later for graceful exit. */

  new_thread(PID_INIT,
      5, g_current_sp, (void (*)(void)) 0);
  new_thread(PID_USER_PROC_1,
      5, &user_proc_1_stack[STACK_SIZE-1], user_proc_1);
  new_thread(PID_CLOCK_COUNTER,
      2, &user_proc_3_stack[STACK_SIZE-1], clock_tick_counter);
  new_thread(PID_UART1_OUT_READY_NOTIFIER, 
      0, &user_proc_5_stack[STACK_SIZE-1], uart1_out_ready_notifier);
  new_thread(PID_UART1_OUT_SERVER,
      1, &user_proc_6_stack[STACK_SIZE-1], uart1_out_server);
  new_thread(PID_UART1_IN_READY_NOTIFIER,
      0, &user_proc_7_stack[STACK_SIZE-1], uart1_in_ready_notifier);
  new_thread(PID_UART1_IN_SERVER,
      1, &user_proc_8_stack[STACK_SIZE-1], uart1_in_server);
  new_thread(PID_COMMAND_SERVER,
      3, &user_proc_9_stack[STACK_SIZE-1], command_server);

	set_timer_period(current_timer_period);
	timer_interrupt_enable();
	uart1_out_interrupt_enable();
	uart1_in_interrupt_enable();
  printf_busy("Kernel load success!\n");
	schedule_next_task();
}
