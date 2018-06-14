#include "printf.h"
#include "private_kernel_interface.h"
#include "user_proc.h"
#include "queue.h"
#include "memman.h"

unsigned int current_task_id = 0;
unsigned int num_clock_ticks = 0;
unsigned int saved_uart1_out_ready = 0;
unsigned int saved_uart1_in_ready = 0;

static struct process_control_block*
select_task(enum process_state state)
{
  struct process_control_block * task = NULL;
  unsigned min_priority = ~0u;
  // stupid selector
  for (unsigned i = 0; i < MAX_NUM_PROCESSES; i++)
    if (pcbs[i].state == state && pcbs[i].priority < min_priority) {
      task = &(pcbs[i]);
      min_priority = pcbs[i].priority;
    }
  return task;
}

void unblock_tasks_for_event(enum kernel_event event){
  switch (event){
    case CLOCK_TICK_EVENT:{
      struct process_control_block* unblocked_task = select_task(
          BLOCKED_ON_CLOCK_TICK);
      if (unblocked_task != NULL)
        unblocked_task->state = READY;
      break;
    }case UART1_OUT_READY:{
      struct process_control_block* unblocked_task = select_task(
          BLOCKED_ON_UART1_OUT_READY);
      if (unblocked_task != NULL)
        unblocked_task->state = READY;
      break;
    }case UART1_IN_READY:{
      struct process_control_block* unblocked_task = select_task(
          BLOCKED_ON_UART1_IN_READY);
      if (unblocked_task != NULL)
        unblocked_task->state = READY;
      break;
    }default:{
      fatal(11); // bad event
      break;
    }
  }
  // a sched() can be here
}

void clean_mm(struct process_control_block* pcb){
  // stupid iterating over pgdir and pgtab
  unsigned fr = read_flags_register() & GLOBAL_INTERRUPT_ENABLE_BIT;
  deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);
  { // interrupt not enabled
    for (unsigned* pdep = pcb->pgdir; pdep < ((unsigned*) pcb->pgdir) + (PAGE_SIZE>>2); pdep++) {
      if ((*pdep & PDE_FLAGS_P) != PDE_FLAGS_P) continue;
      unsigned* pt = (unsigned*) PA2KLA(GET_PN(*pdep));
      for (unsigned* ptep = pt; ptep < pt + (PAGE_SIZE>>2); ptep++) {
        if ((*ptep & PTE_FLAGS_P) != PTE_FLAGS_P) continue;
        unsigned pagepa = (unsigned) GET_PN(*ptep);
        release_page(pagepa);
      }
      release_page((unsigned) KLA2PA(pt));
    }
    release_page((unsigned) KLA2PA(pcb->pgdir));
  }
  or_into_flags_register(fr);
}

void k_yield(enum process_state state){
  cur_proc->state = state;
  sched();
}

void k_task_exit(void){
  if (cur_proc == idle_proc)
    fatal(40); // idle_proc can't exit
  cur_proc->state = NOT_ALLOCATED;
  // TODO: clean_mm. now has some bug caused by vm.
  //  clean_mm(cur_proc);
  // TODO: release kstack page
  //    release_page(KLA2PA(cur_proc->kstack));
  // directly releasing is dangerous because we are executing on 
  //  kstack
  sched();
}

void k_syscall(unsigned id, unsigned args){
  switch (id) {
    case SYSCALL_ID_EXIT:
      k_task_exit();
      break;
    case SYSCALL_ID_PUTC: do {
        struct kernel_message msg = {
          .type = OTHER,
          .data = args
        };
        k_send_message(&msg, uart1_out_pid);
      } while (0);
      break;
    case SYSCALL_ID_RECVMSG: do {
        struct kernel_message* msg = k_receive_message();
        memcpyw((void*) args, (void*) msg, sizeof(struct kernel_message)>>2);
        } while (0);
        break;
    default:
      // TODO: fault for unknown syscalls?
      break;
  }
}

void k_irq_handler(void){
  unsigned int flags_register = read_flags_register();
  // wtf? compiler random behaviour?!
  unsigned ass;
  if((ass = flags_register & PAGE_FAULT_EXCEPTION_ASSERTED_BIT)){
    printf_direct("pf\n");
    or_into_flags_register(HALTED_BIT); /*  Halt the processor for now */
    /*  De-assert the bit last, so we can detect nested page fault exceptions */
    // TODO
    // deassert_bits_in_flags_register(PAGE_FAULT_EXCEPTION_ASSERTED_BIT);
  }else if((ass = flags_register & UART1_OUT_ASSERTED_BIT)){
    deassert_bits_in_flags_register(UART1_OUT_ASSERTED_BIT);
    unblock_tasks_for_event(UART1_OUT_READY);
  }else if((ass = flags_register & UART1_IN_ASSERTED_BIT)){
    deassert_bits_in_flags_register(UART1_IN_ASSERTED_BIT);
    unblock_tasks_for_event(UART1_IN_READY);
  }else if((ass = flags_register & TIMER1_ASSERTED_BIT)){
    if ((num_clock_ticks & 63) == 32)
      printf_direct("tick %x\n", num_clock_ticks);
    deassert_bits_in_flags_register(TIMER1_ASSERTED_BIT);
    num_clock_ticks++;
    unblock_tasks_for_event(CLOCK_TICK_EVENT);
  }else if ((ass = flags_register & SYSCALL_BIT)){
    deassert_bits_in_flags_register(SYSCALL_BIT);
    unsigned syscall_id = read_syscall_id();
    unsigned syscall_args = read_syscall_args();
    printf_direct("syscall %x %x", syscall_id, syscall_args);
    k_syscall(syscall_id, syscall_args);
  }else{
    /*  Something really bad happend. */
    fatal(20);
  }
  sched();
}

unsigned k_send_message(struct kernel_message * msg, unsigned dstpid){
  // msg must already be in kernel space
  printf_direct("msg %x -> %x", cur_proc->pid, dstpid);
  struct process_control_block* dstproc = &(pcbs[dstpid]);
  msg->src_pid = cur_proc->pid;
  if (msg != NULL && message_queue_size(&(dstproc->mq)) < MAX_NUM_PROCESSES - 1) {
    printf_direct("\n");
    unsigned fr = read_flags_register() & GLOBAL_INTERRUPT_ENABLE_BIT;
    deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);
    { // interrupt not enabled
      message_queue_push(&(dstproc->mq), msg);
    }
    or_into_flags_register(fr);
    dstproc->state = READY;
    return 0;
  } else {
    printf_direct("dropped \n");
    return 1;
  }
}

struct kernel_message* k_receive_message(void) {
  if (message_queue_size(&(cur_proc->mq)) != 0)
    return message_queue_pop(&(cur_proc->mq));
  cur_proc->state = BLOCKED_ON_MESSAGE;
  sched();
  if (message_queue_size(&(cur_proc->mq)) == 0)
    fatal(42);
  struct kernel_message* rv = NULL;
  unsigned fr = read_flags_register() & GLOBAL_INTERRUPT_ENABLE_BIT;
  deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);
  { // interrupt not enabled
    rv = message_queue_pop(&(cur_proc->mq));
  }
  or_into_flags_register(fr);
  return rv;
}

void set_irq_handler(void (*irq_handler_fcn)(void)){
  void (**irq_handler_fcn_location)(void) = (void (**)(void))IRQ_HANDLER;
  *irq_handler_fcn_location = irq_handler_fcn;
}

void set_timer_period(unsigned int period){
  unsigned int * period_location = (unsigned int *)TIMER_PERIOD;
  *period_location = period;
}

void mm_init(){
  // initialize pages
  pages = (struct page*) IMAGE_END;
  // page[i]: refer to physical address i*PGSZ ~ (i+1)*PGSZ
  n_pages = MEMSIZE >> PAGE_SIZE_WIDTH;
  n_kpages = KMEMSIZE >> PAGE_SIZE_WIDTH;
  // always reserve the first n_kpages for kernel
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
//
// stupid slow algorithm
unsigned calloc_page(){
  for (int i = n_kpages; i < n_pages; i++)
    if (pages[i].ref == 0) {
      unsigned fr = read_flags_register() & GLOBAL_INTERRUPT_ENABLE_BIT;
      deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);
      { // interrupt not enabled
        pages[i].ref = 1;
      }
      or_into_flags_register(fr);
      // zero alloc'ed page
      memsetw((void*) PA2KLA((unsigned) (i<<PTSHIFT)), 0, PAGE_SIZE>>2);
      return (i<<PAGE_SIZE_WIDTH);
    }
  fatal(17); // out of memory
  return 0;
}

// when a process gives up the page frame
//  pa must be aligned
void release_page(unsigned pa){
  unsigned fr = read_flags_register() & GLOBAL_INTERRUPT_ENABLE_BIT;
  deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);
  { // interrupt not enabled
    unsigned pgidx = pa >> PAGE_SIZE_WIDTH;
    if (pages[pgidx].ref == 0) fatal(41); // double free?!
    pages[pgidx].ref--;
  }
  or_into_flags_register(fr);
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
    t = calloc_page();
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
  printf_direct("map seg: la %x  -> pa %x\n", la, pa);
  if (GET_POFFSET(la) != GET_POFFSET(pa))
    fatal(18);
  la = GET_PN(la);
  pa = GET_PN(pa);
  pte_t* pte = get_pte(pd, la, 1);

  if (GET_PN(*pte) == pa) {
    // present, la maps to pa
  } else {
    if (GET_PN(*pte) != 0)
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
  pcbs[rv].pgdir = NULL;
  pcbs[rv].ustack = NULL;
  pcbs[rv].kstack = NULL;
  pcbs[rv].priority = ~0u;
  message_queue_init(&(pcbs[rv].mq));
  return &(pcbs[rv]);
}

struct process_control_block* create_kernel_thread(void (*fn)(void),
    enum process_state init_state, unsigned init_priority){
  struct process_control_block* proc = alloc_proc();
  proc->pgdir = (void*) PA2KLA((unsigned) calloc_page());
  proc->kstack = (void*) PA2KLA((unsigned) calloc_page()) + PAGE_SIZE - 4;
  proc->ustack = 0;
  proc->state = init_state;
  proc->priority = init_priority;
  init_kstack(&(proc->kstack), fn);
  printf_direct("created kernel thread %x\n", proc->pid);
  return proc;
}

// creating user process
struct process_control_block* create_process(
    void* text, unsigned textsize,
    void* data, unsigned datasize,
    unsigned init_priority, enum process_state init_state){
  // compiler hack
  unsigned ass;
  if ((ass = textsize & 3))
    fatal(44);

  struct process_control_block* proc = alloc_proc();

  proc->pgdir = (void*) PA2KLA((unsigned) calloc_page());

  proc->kstack = (void*) PA2KLA((unsigned) calloc_page()) + PAGE_SIZE - 4;
  init_ukstack(&(proc->kstack));

  unsigned ustackpa = (unsigned) calloc_page();
  proc->ustack = (void*) PA2KLA(ustackpa + PAGE_SIZE - 4); // now kla
  init_ustack(&(proc->ustack), 0x40000);
  proc->ustack = (void*) (GET_PN(KSEG_BEGIN - PAGE_SIZE) 
      | GET_POFFSET((unsigned) proc->ustack));
  printf_direct("ustack %x\n", proc->ustack);
  map_segment(proc->pgdir, GET_PN((unsigned) proc->ustack), ustackpa);

  proc->state = init_state;
  proc->priority = init_priority;

  // XXX: big hack
  //  don't begin from 0, NULL points to 0
  //  also better not value too small; people make mistakes,
  //  e.g. forget to add `&`.
  unsigned ptr = 0x40000;
  for (unsigned copied = 0; copied < textsize; copied += PAGE_SIZE) {
    unsigned pagepa = (unsigned) calloc_page();
    memcpyw((void*) PA2KLA(pagepa),
        (void*) (((unsigned) text) + copied), // doesn't have to be page aligned
        PAGE_SIZE>>2);  // might copy some extra garbage! bad protection
    map_segment(proc->pgdir, ptr, pagepa);
    ptr += PAGE_SIZE;
  }
  for (unsigned copied = 0; copied < datasize; copied += PAGE_SIZE) {
    unsigned pagepa = (unsigned) calloc_page();
    memcpyw((void*) PA2KLA(pagepa),
        (void*) (((unsigned) data) + copied), // doesn't have to be page aligned
        PAGE_SIZE>>2);  // might copy some extra garbage! bad protection
    map_segment(proc->pgdir, ptr, pagepa);
    ptr += PAGE_SIZE;
  }
  // no bss man
  // hey we don't even have a disk, everything's in ram

  printf_direct("created process %x\n", proc->pid);

  return proc;
}

void stupid_proc_init(void){
  for (unsigned i = 0; i < MAX_NUM_PROCESSES; i++)
    pcbs[i].state = NOT_ALLOCATED;

  // idle thread 0
  idle_proc = alloc_proc();
  idle_proc->kstack = (void*) ((unsigned) init_stack + PAGE_SIZE - 4);
  idle_proc->state = READY; // idle is right running
  if (idle_proc->pid != 0) fatal(21);
  idle_proc->priority = ~0u - 1; // least priority
  cur_proc = idle_proc;


  create_kernel_thread(hello_msg_ksvc, BLOCKED_ON_CLOCK_TICK, ~0u-4);
  struct process_control_block* uart1_in_kthr = create_kernel_thread(
      uart1_in_ksvc, READY, ~0u-3);
  uart1_in_pid = uart1_in_kthr->pid;
  struct process_control_block* uart1_out_kthr = create_kernel_thread(
      uart1_out_ksvc, READY, ~0u-3);
  uart1_out_pid = uart1_out_kthr->pid;

  create_process(stupid, 4096, NULL, 0, ~0u-2, READY);
}

void sched(void)
{
  unsigned fr = read_flags_register() & GLOBAL_INTERRUPT_ENABLE_BIT;
  deassert_bits_in_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);
  { // interrupt not enabled
    // select process `next_proc` to swap in
    struct process_control_block* next_proc = NULL;
    unsigned min_priority = ~0u;
    for (unsigned i = 0; i < MAX_NUM_PROCESSES; i++) {
      if (pcbs[i].state == READY && pcbs[i].priority < min_priority) {
        min_priority = pcbs[i].priority;
        next_proc = &(pcbs[i]);
      }
    }
    //
    if (cur_proc->pid != next_proc->pid)
      printf_direct("sched %x -> %x\n", cur_proc->pid, next_proc->pid);
    if (cur_proc != next_proc) {
      use_pgdir(next_proc->pgdir);
      struct process_control_block* old_proc = cur_proc;
      cur_proc = next_proc;
      switch_kstack(next_proc, old_proc);
    }
  }
  or_into_flags_register(fr);
}

void k_kernel_init(void){
  printf_direct("(THU.CST) OS is loading\n");

  mm_init();

  set_irq_handler(irq_handler); /*  Set before paging is enabled, otherwise a page fault doesn't know where to go */
  set_timer_period(INITIAL_TIMER_PERIOD_VALUE);
  or_into_flags_register(TIMER1_ENABLE_BIT);
  or_into_flags_register(UART1_OUT_ENABLE_BIT);
  or_into_flags_register(UART1_IN_ENABLE_BIT);
  or_into_flags_register(PAGEING_ENABLE_BIT);

  stupid_proc_init();

  // this function won't go back to do_kernel_method
  //  so no eret, you have to enable interrupt your self
  or_into_flags_register(GLOBAL_INTERRUPT_ENABLE_BIT);

  // idle keeps looping
  while (1) { }
}
