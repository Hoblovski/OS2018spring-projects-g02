#include "machine.h"
#include <ctype.h>


tlbent_t* tlb_lookfor(machine_t* m, uint32_t la)
{
  uint32_t vpn = GET_PN(la);
  for (int i = 0; i < TLB_SIZE; i++)
    if ((m->tlb[i].vpn == vpn) && (m->tlb[i].flags & TLB_FLAGS_P))
      return &(m->tlb[i]);
  // la not in tlb, select a victim: just by stupid rr
  static int victim = 0;
  victim = (victim + 1) % TLB_SIZE;
  m->tlb[victim].flags &= ~TLB_FLAGS_P;   // invalidate
  return &(m->tlb[victim]);
}


static uint32_t map_port(uint32_t port_addr)
{
  switch (port_addr) {
    case UART1_IN: return 0;
    case UART1_OUT: return 4;
    case IRQ_HANDLER: return 8;
    case TIMER_PERIOD: return 12;
    case MEM_UART_OUT_DIRECT: return 16;
    case PD_POINTER: return 20;
    case PAGEFAULT_BADVA: return 24;
    case SYSCALL_ID: return 28;
    case SYSCALL_ARGS: return 32;
    default:
        if (IS_PORT(port_addr)) {
          Printf("unrecognized port address access: port_addr=%08X\n", port_addr);
          assert(0);
        } else { 
          return ~0u;
        }
  }
}


uint32_t mmu_la2pa(machine_t* m, uint32_t la, uint32_t* isport,
    uint32_t priv_chk)
{
  // check for privilege
  if (priv_chk && (m->regs[REG_FR] & FRBIT_PL) && IS_KLA(la)) {
    if (la != MEM_UART_OUT_DIRECT && la != SYSCALL_ID && la != SYSCALL_ARGS) { // allow debugging output in user mode
      Printf("protection error: user code at %08X attemps to access ",
          m->regs[REG_PC]);
      if (IS_PORT(la)) 
        Printf("port %08X\n", la); 
      else
        Printf("kvaddr %08X\n", la);
      assert(0);
    }
  }
  // check for address mapped to io ports
  uint32_t port_addr = map_port(la);
  if (port_addr != ~0u) {
    if (isport) *isport = 1;
    return port_addr;
  }
  if (isport) *isport = 0;

  // for 0xC0000000 - 0xFFFF0000,
  //  unmapped (probably cached? we don't care 'bout cache)
  if (IS_KLA(la))
    return KLA2PA(la);

  // without paging, pa=la
  if (!(m->regs[REG_FR] & FRBIT_PAGING_ENABLE))
    return la;

  // do v-p translation
  uint32_t offset = GET_POFFSET(la);
  tlbent_t* te = tlb_lookfor(m, la);
  if (!(te->flags & TLB_FLAGS_P)) {
    // not present
    // do hw tlb refill
    // @te: victim
    // TODO: probably refactor here, to mimic hw behaviour
    pde_t* pd = (pde_t*) port_lw(m, mmu_la2pa(m, PD_POINTER, NULL, 0));
    if ((uint32_t) pd < KSEG_BEGIN) {
      Printf("[%08X] pagedir %08X in user space, bad useg addr %08X\n", m->regs[REG_PC], (uint32_t) pd, la);
      assert(0);
    }
    pde_t pde = mem_lw(m, mmu_la2pa(m, (uint32_t) (pd + GET_PDIDX(la)), NULL, 0));
    if (!(pde & PDE_FLAGS_P)) {
      Printf("[%08X] page table not present, pagefault not implemented, badva %08X\n", m->regs[REG_PC], la);
      Printf("pagedir dump:\n");
      for (int i = 0; i < 1024; i++) {
        pde = mem_lw(m, mmu_la2pa(m, (uint32_t) (pd + i), NULL, 0));
        if (pde != 0)
          Printf("%03X %08X\n", i, pde);
      }
      Printf("invalid pdidx: %03X\n", GET_PDIDX(la));
      assert(0);
    }
    pte_t* pt = (pte_t*) GET_PN(pde);
    if ((uint32_t) pt < KSEG_BEGIN) {
      Printf("[%08X] pagetable %08X in user space, bad useg addr %08X\n", m->regs[REG_PC], (uint32_t) pt, la);
      assert(0);
    }
    pte_t pte = mem_lw(m, mmu_la2pa(m, (uint32_t) (pt + GET_PTIDX(la)), NULL, 0));
    if (!(pte & PTE_FLAGS_P)) {
      Printf("[%08X] page not present, pagefault not implemented, badva %08X\n", m->regs[REG_PC], la);
      assert(0);
    }
    // now we get mapping: PN(la) -> PN(pte)
    te->vpn = GET_PN(la);
    te->ppn = GET_PN(pte);
    te->flags = GET_PFLAGS(pte, pde);
  }
  return GET_PN(te->ppn) | offset;
}


uint32_t mem_lw(machine_t* m, uint32_t pa)
{
  if (pa+3 >= MEMSZ_BYTES) {
    printf("out of range memory read\n");
    printf("pc=%08X, bad pa=%08X\n", m->regs[REG_PC], pa);
    assert(0);
  }
  if (pa & 3) {
    Printf("instruction at [%08X] triggers unaligned load word at %08X!\n", m->regs[REG_PC], pa);
    assert(0);
  }
  return *(uint32_t*) &(m->mem[pa]);
}


void mem_sw(machine_t* m, uint32_t pa, uint32_t v)
{
  if (pa+3 >= MEMSZ_BYTES) {
    printf("out of range memory write\n");
    printf("pc=%08X, bad pa=%08X\n", m->regs[REG_PC], pa);
    assert(0);
  }
  if (pa & 3) {
    Printf("instruction at [%08X] triggers unaligned store word to %08X with value %08X!\n", m->regs[REG_PC], pa, v);
    assert(0);
  }
  *(uint32_t*) &(m->mem[pa]) = v;
}


uint32_t port_lw(machine_t* m, uint32_t port_addr)
{
  if (port_addr+3 >= MEMSZ_BYTES) {
    printf("out of range port read\n");
    printf("pc=%08X, bad port_addr=%08X\n", m->regs[REG_PC], port_addr);
    assert(0);
  }
  if (port_addr & 3) {
    Printf("instruction at [%08X] triggers unaligned load word at port %08X!\n", m->regs[REG_PC], port_addr);
    assert(0);
  }
  return *(uint32_t*) &(m->port[port_addr]);
}


void port_sw(machine_t* m, uint32_t port_addr, uint32_t v)
{
  if (port_addr+3 >= MEMSZ_BYTES) {
    printf("out of range port write\n");
    printf("pc=%08X, bad port_addr=%08X\n", m->regs[REG_PC], port_addr);
    assert(0);
  }
  if (port_addr & 3) {
    Printf("instruction at [%08X] triggers unaligned store word at port %08X with value %08X!\n",
        m->regs[REG_PC], port_addr, v);
    assert(0);
  }
#ifdef WATCH_UART_OUT_DIRECT
  if (port_addr == map_port(MEM_UART_OUT_DIRECT))
#ifdef COLOR_OUTPUT
    Printf("%s%c%s", KBLU, v, KNRM);
#else
    Printf("%c", v);
#endif
#endif
  *(uint32_t*) &(m->port[port_addr]) = v;
}
