#ifndef MACHINE_H
#define MACHINE_H

#include <stdint.h>
#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"

/******************************************************************************/
// register
#define NUM_REGS 32
#define REG_PC 0
#define REG_SP 1
#define REG_FP 2
#define REG_ZR 3
#define REG_FR 4
#define REG_WR 5
#define REG_LR 7
// same as epc. added for my teammates.
#define REG_EPC 18
// CR3 holds the physical address of page directory
#define REG_CR3 19
typedef uint32_t reg_t[NUM_REGS];

// flag register bits
#define FRBIT_HALT (1u << 0u)
#define FRBIT_GIE (1u << 1u)
#define FRBIT_ERET (1u << 2u)
#define FRBIT_CLKEN (1u << 3u)
#define FRBIT_CLK (1u << 4u)
#define FRBIT_UART1_OUTEN (1u << 5u)
#define FRBIT_UART1_OUT (1u << 6u)
#define FRBIT_UART1_INEN (1u << 7u)
#define FRBIT_UART1_IN (1u << 8u)
#define FRBIT_UART1_OUTRDY (1u << 9u)
#define FRBIT_UART1_INRDY (1u << 10u)
// priviledge level, 0 for kernel, 1 for user
#define FRBIT_PL (1u<<11u)
#define FRBIT_PAGEFAULT (1u<<12u)
#define FRBIT_PAGING_ENABLE (1u << 13u)

/******************************************************************************/
// memory and io port
#define MEMSZ_BYTES (128<<20)
typedef uint8_t mem_t[MEMSZ_BYTES];
#define PORTSZ_BYTES 1024
typedef uint8_t port_t[PORTSZ_BYTES];

#define UART1_OUT 0xffff0000
#define UART1_IN 0xffff0010
#define IRQ_HANDLER 0xffff0020
#define TIMER_PERIOD 0xffff0030
#define PD_POINTER 0xffff0040
#define PAGEFAULT_BADVA 0xffff0050
#define EFLAGS 0xffff0060
#define MEM_UART_OUT_DIRECT 0xfffffff0

#define KSEG_BEGIN 0xC0000000
#define PSEG_BEGIN 0xFFFF0000
#define IS_KLA(addr) (((addr)&KSEG_BEGIN) == KSEG_BEGIN)
#define IS_PORT(addr) (((addr)&PSEG_BEGIN) == PSEG_BEGIN) 
#define KLA2PA(kla) ((kla)&~KSEG_BEGIN)

/******************************************************************************/
// hw involved in paging
//
// pte structure:
//    22        2         8
//    framebase reserved  flags
typedef uint32_t pde_t;
typedef uint32_t pte_t;
#define PDSHIFT 21
#define PTMASK 0x7FF
#define PTSHIFT 10
#define POFFSETMASK 0x3FF
#define GET_PDIDX(addr) ((addr) >> PDSHIFT)
#define GET_PTIDX(addr) (((addr) >> PTSHIFT) & PTMASK)
#define GET_PN(addr) ((addr) & ~POFFSETMASK)
#define GET_POFFSET(addr) ((addr) & POFFSETMASK)
#define PDE_FLAGS_P 1
#define PTE_FLAGS_P 1
#define GET_PFLAGS(pte, pde) ((pte) & 0xFF)

// fully associative tlb
#define TLB_SIZE 64
#define TLB_FLAGS_P 1
typedef struct {
  uint32_t vpn;
  uint32_t ppn;
  uint8_t flags;
} tlbent_t;
typedef tlbent_t tlb_t[TLB_SIZE];

/******************************************************************************/
// raw machine, without emulator enforced protection
typedef struct machine_t {
  mem_t mem;      // array of bytes
  port_t port;    // also array of bytes, but seperated from memory
  reg_t regs;     // array of u32's
  tlb_t tlb;      // fully associative
  uint32_t cycno; // number of cycles elapsed since power on
} machine_t;

/******************************************************************************/
// read, write on raw memory (no priviledge)
uint32_t mem_lw(machine_t* m, uint32_t pa);
void mem_sw(machine_t* m, uint32_t pa, uint32_t v);
uint32_t port_lw(machine_t* m, uint32_t port_addr);
void port_sw(machine_t* m, uint32_t port_addr, uint32_t v);

// a stupid vp address translation
tlbent_t* tlb_lookfor(machine_t* m, uint32_t pa);
uint32_t mmu_la2pa(machine_t* m, uint32_t la, uint32_t* isport,
    uint32_t priv_chk);
// machine specific
extern void check_excep(machine_t* m);
extern void exec_inst(machine_t* m, uint32_t inst);
extern void machine_init(machine_t* m);

#endif // MACHINE_H
