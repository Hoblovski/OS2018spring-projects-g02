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
#include "queue.h"
#include "fatal.h"

void task_queue_init(struct task_queue * queue, unsigned int size) {
	queue->start = 0;
	queue->end = 0;
	queue->current_count = 0;
	queue->size = size;
}

void task_queue_push_end(struct task_queue * queue, void * item) {
	if (((queue->end + 1) & 7) == queue->start) {
    fatal(13); // Task queue is full.
	}
	
	queue->items[queue->end] = item;
	queue->end = ((queue->end + 1) & 7);
	queue->current_count += 1;
}

void * task_queue_pop_start(struct task_queue * queue) {
	void * item;
	if (queue->start == queue->end) {
    fatal(14); // Task queue is empty.
		return (void *)0;
	}

	item = queue->items[queue->start];

	queue->start = ((queue->start + 1) & 7);
	queue->current_count -= 1;
	return item;
}

unsigned int task_queue_current_count(struct task_queue * queue){
	return queue->current_count;
}

void message_queue_init(struct message_queue * queue, unsigned int size){
	queue->start = 0;
	queue->end = 0;
	queue->current_count = 0;
	queue->size = size;
}

void message_queue_push_end(struct message_queue * queue, struct kernel_message item){
	if (((queue->end + 1) & 7) == queue->start) {
		fatal(15); // Message queue is full.
	}
	
	queue->items[queue->end] = item;
	queue->end = ((queue->end + 1) & 7);
	queue->current_count += 1;
}

struct kernel_message message_queue_pop_start(struct message_queue * queue){
	struct kernel_message item;
	if (queue->start == queue->end) {
		fatal(16); // Message queue is empty.
		return item;
	}

	item = queue->items[queue->start];

	queue->start = ((queue->start + 1) & 7);
	queue->current_count -= 1;
	return item;
}


unsigned int message_queue_current_count(struct message_queue * queue){
	return queue->current_count;
}
