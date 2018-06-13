#include "queue.h"
#include "private_kernel_interface.h"
#include "memman.h"

void message_queue_init(struct message_queue * mq){
  mq->start = 0;
  mq->end = 0;
}

void message_queue_push(struct message_queue * mq, struct kernel_message* item){
  if (((mq->end+1) & (MAX_NUM_PROCESSES-1)) == mq->start) {
    fatal(15); // Message mq is full.
  }

  memcpy(&(mq->items[mq->end]), item, sizeof(struct kernel_message));
  mq->end = ((mq->end+1) & (MAX_NUM_PROCESSES-1));
}

struct kernel_message* message_queue_pop(struct message_queue * mq){
  if (mq->start == mq->end)
    fatal(16); // mq is empty.

  struct kernel_message* item;
  item = &(mq->items[mq->start]);
  mq->start = ((mq->start+1) & (MAX_NUM_PROCESSES-1));
  return item;
}

unsigned message_queue_size(struct message_queue * mq){
  return (mq->end + MAX_NUM_PROCESSES - mq->start) & (MAX_NUM_PROCESSES-1);
}
