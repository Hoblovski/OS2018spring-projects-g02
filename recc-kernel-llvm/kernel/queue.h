#ifndef QUEUE_H_DEFINED_
#define QUEUE_H_DEFINED_

#include "kernel_state.h"

void message_queue_init(struct message_queue * mq);
void message_queue_push(struct message_queue * mq, struct kernel_message* item);
struct kernel_message* message_queue_pop(struct message_queue * mq);
unsigned message_queue_size(struct message_queue * mq);

#endif
