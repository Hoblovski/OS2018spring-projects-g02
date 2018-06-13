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
#include "queue.h"

struct task_queue ready_queue_p0;
struct task_queue ready_queue_p1;
struct task_queue ready_queue_p2;
struct task_queue ready_queue;
struct task_queue zombie_queue;
struct task_queue blocked_on_clock_tick_queue;
struct task_queue blocked_on_uart1_out_ready_queue;
struct task_queue blocked_on_uart1_in_ready_queue;
struct task_queue blocked_on_send_queue;
struct task_queue blocked_on_receive_queue;
struct task_queue blocked_on_reply_queue;

struct process_control_block pcbs[MAX_NUM_PROCESSES];
unsigned int init_stack[STACK_SIZE];
struct process_control_block* cur_proc;
struct process_control_block* idle_proc;

struct page* pages;
unsigned n_pages;
unsigned n_kpages;
