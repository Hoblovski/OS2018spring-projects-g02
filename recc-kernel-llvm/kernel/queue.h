#ifndef QUEUE_H_DEFINED_
#define QUEUE_H_DEFINED_
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

void ready_queue_push(struct task_queue* tq, struct process_control_block* proc);

void task_queue_init(struct task_queue *);

void task_queue_push_end(struct task_queue *, void *);

void * task_queue_pop_start(struct task_queue *); 

unsigned int task_queue_current_count(struct task_queue *); 

void message_queue_init(struct message_queue *, unsigned int);

void message_queue_push_end(struct message_queue *, struct kernel_message);

struct kernel_message message_queue_pop_start(struct message_queue *); 

unsigned int message_queue_current_count(struct message_queue *); 

#endif
