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


static uint32_t map_port_noassert(uint32_t port_addr)
{
  switch (port_addr) {
    case UART1_IN: return 0;
    case UART1_OUT: return 4;
    case IRQ_HANDLER: return 8;
    case TIMER_PERIOD: return 12;
    case MEM_UART_OUT_DIRECT: return 16;
    default: return ~0u;
  }
}


uint32_t mmu_la2pa(machine_t* m, uint32_t la, uint32_t* isport)
{
  // check for address mapped to io ports
  uint32_t port_addr = map_port_noassert(la);
  if (port_addr != ~0u) {
    if (isport) *isport = 1;
    return port_addr;
  }

  // without paging, pa=la
  if (isport) *isport = 0;
  if (!(m->regs[REG_FR] & FRBIT_PAGING_ENABLE))
    return la;

  // do v-p translation
  if (isport) *isport = 0;
  uint32_t offset = GET_POFFSET(la);
  tlbent_t* te = tlb_lookfor(m, la);
  if (!(te->flags & TLB_FLAGS_P)) {
    // not present
    // do hw tlb refill
    // @te: victim
    // TODO: probably refactor here, to mimic hw behaviour
    pde_t* pd = (pde_t*) m->regs[REG_CR3];
    pde_t pde = *(uint32_t*) (pd + GET_PDIDX(la));
    assert((pde & PDE_FLAGS_P) && "bad memory access: page table not present, pagefault not implemented\n");
    pte_t* pt = (pte_t*) GET_PN(pde);
    pte_t pte = *(uint32_t*) (pt + GET_PTIDX(la));
    assert((pte & PTE_FLAGS_P) && "bad memory access: page not present, pagefault not implemented\n");
    // now we get mapping: PN(la) -> PN(pte)
    te->vpn = GET_PN(la);
    te->ppn = GET_PN(pte);
    // TODO: in the future remove this stupid hardcode
    te->flags = TLB_FLAGS_P;
  }
  return GET_PN(te->ppn) | offset;
}


static uint32_t map_port(uint32_t port_addr)
{
  switch (port_addr) {
    case UART1_IN: return 0;
    case UART1_OUT: return 4;
    case IRQ_HANDLER: return 8;
    case TIMER_PERIOD: return 12;
    case MEM_UART_OUT_DIRECT: return 16;
    default: Printf("bad port %08X\n", port_addr); assert(0 && "unknown port");
  }
}


uint32_t mem_lw(machine_t* m, uint32_t pa)
{
  if (pa+3 >= MEMSZ_BYTES) {
    printf("out of range memory read\n");
    printf("pc=%08X, bad pa=%08X\n", m->regs[REG_PC], pa);
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
  *(uint32_t*) &(m->mem[pa]) = v;
}


uint32_t port_lw(machine_t* m, uint32_t port_addr)
{
  if (port_addr+3 >= MEMSZ_BYTES) {
    printf("out of range port read\n");
    printf("pc=%08X, bad port_addr=%08X\n", m->regs[REG_PC], port_addr);
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
  if (port_addr == map_port_noassert(MEM_UART_OUT_DIRECT))
    printf("%% %08X (dec=% 10d) (char=%c)\n", v, v, v);
  *(uint32_t*) &(m->port[port_addr]) = v;
}
