#include "machine.h"
#include "common.h"
#include "loader.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

uint32_t excep = 0;

uint32_t n_timint = 0;

void machine_init(machine_t* m)
{
  memset(m, 0, sizeof(*m));
  m->regs[REG_FR] = FRBIT_UART1_OUTRDY;
  m->regs[REG_WR] = 4; // recc's stupid design really
  m->regs[REG_ZR] = 0;
  m->regs[REG_PC] = KSEG_BEGIN;

  m->cycno = 0;
}

#define decode_opcode(i) (((i)>>26) & 0x3F)
#define decode_rx(i) (((i)>>21) & 0x1F)
#define decode_ry(i) (((i)>>16) & 0x1F)
#define decode_rz(i) (((i)>>11) & 0x1F)
#define decode_imm16(i) ((i) & 0xFFFF)
#define decode_imm16sext(i) ((int) ((short) ((i) & 0xFFFF)))
#define decode_imm26(i) ((i) & 0x3FFFFFF)
static inline uint32_t decode_imm26sext(uint32_t i) {
  if (i & (1<<25)) {
    return i | 0xFC000000;
  } else {
    return i & 0x03FFFFFF;
  }
}

#define OPCODE_ADD 0
#define OPCODE_SUB 1
#define OPCODE_MUL 2
#define OPCODE_DIV 3

#define OPCODE_AND 4
#define OPCODE_OR 5
#define OPCODE_XOR 6

#define OPCODE_LD 7
#define OPCODE_ST 8

#define OPCODE_SHR 9
#define OPCODE_SHL 10

#define OPCODE_BEQ 11
#define OPCODE_BLT 12

#define OPCODE_ADDIU 13
#define OPCODE_LUI 14

#define OPCODE_ORI 16
#define OPCODE_LB 17
#define OPCODE_SB 18
#define OPCODE_BNE 19
#define OPCODE_JR 20
#define OPCODE_JALR 21
#define OPCODE_JSUB 22

char* instrname[] = {
  "add", "sub", "mul", "div",
  "and", "or",  "xor", "loa",
  "sto", "shr", "shl", "beq",
  "blt", "addiu", "lui", "BAD",
  "ori", "lb", "sb", "bne",
  "jr", "jalr", "jsub"
};

char* regname[] = {
  "pc", "sp", "fp",  "zr", "fr", "wr", "at", "lr",
  "gp", "v0", "v1",  "a0", "a1", "s0", "s1", "t0",
  "t1", "t2", "epc", "?",  "?",  "?",  "?",  "?",
  "?",  "?",  "?",   "?",  "?",  "?",  "?",  "?",
};

#define INSTR_TYPE_R 1
#define INSTR_TYPE_I 2
#define INSTR_TYPE_J 3

int instr_type(uint32_t opcode)
{
  switch (opcode) {
    case OPCODE_ADD: case OPCODE_SUB: case OPCODE_MUL: case OPCODE_DIV:
    case OPCODE_AND: case OPCODE_OR:  case OPCODE_XOR: case OPCODE_SHR:
    case OPCODE_SHL:
      return INSTR_TYPE_R;
    case OPCODE_LD:  case OPCODE_ST:  case OPCODE_BEQ: case OPCODE_BLT:
    case OPCODE_ADDIU: case OPCODE_LUI: case OPCODE_ORI:
    case OPCODE_LB: case OPCODE_SB: case OPCODE_BNE: case OPCODE_JR:
    case OPCODE_JALR:
      return INSTR_TYPE_I;
    case OPCODE_JSUB:
      return INSTR_TYPE_J;
    default:
      assert(0 && "bad opcode");
  }
}

void dump_fr(machine_t* m)
{
  unsigned r = m->regs[REG_FR];
  if (r & FRBIT_GIE)
    Printf("GIE ");
  if (r & FRBIT_ERET)
    Printf("ERT ");
  if (r & FRBIT_CLKEN)
    Printf("CEN ");
  if (r & FRBIT_CLK)
    Printf("CLK ");
  if (r & FRBIT_UART1_OUTEN)
    Printf("OEN ");
  if (r & FRBIT_UART1_OUT)
    Printf("OUT ");
  if (r & FRBIT_UART1_INEN)
    Printf("IEN ");
  if (r & FRBIT_UART1_IN)
    Printf("IN  ");
  if (r & FRBIT_UART1_OUTRDY)
    Printf("ORD ");
  if (r & FRBIT_UART1_INRDY)
    Printf("IRD ");
}

void dump_inst(machine_t* m, uint32_t inst)
{
  uint32_t opcode = decode_opcode(inst);
  uint32_t rx = decode_rx(inst);
  uint32_t ry = decode_ry(inst);
  uint32_t rz = decode_rz(inst);
  uint32_t imm = decode_imm16(inst);
  uint32_t immsext = decode_imm16sext(inst);
  uint32_t imm26sext = decode_imm26sext(inst);
  switch (instr_type(opcode)) {
    case INSTR_TYPE_R:
      Printf("* [%08X]{%08X} [%-6d] %-6s $%-4s $%-4s $%-4s\n",
          m->regs[REG_PC], inst, m->cycno, instrname[opcode],
          regname[rx], regname[ry], regname[rz]);
      break;
    case INSTR_TYPE_I:
      Printf("* [%08X]{%08X} [%-6d] %-6s $%-4s $%-4s %d (u%d)\n",
          m->regs[REG_PC], inst, m->cycno, instrname[opcode],
          regname[rx], regname[ry], immsext, imm);
      break;
    case INSTR_TYPE_J:
      Printf("* [%08X]{%08X} [%-6d] %-6s %d (h%08X)\n",
          m->regs[REG_PC], inst, m->cycno, instrname[opcode],
          imm26sext, imm26sext);
      break;
    default:
      assert(0 && "bad instr type");
  }
  fflush(stdout);
}

void dump_state(machine_t* m)
{
  Printf("state dump:\n");
  for (int i = 0; i < NUM_REGS; i++)
    Printf("%5s: %08X\n", regname[i], m->regs[i]);
  Printf("state dump finish\n");
}

#define fatal(...) do {\
    Printf("**************** fatal!\n");\
    Printf(__VA_ARGS__);\
    dump_inst(m, inst);\
    dump_state(m);\
    assert(0);\
  } while (0)

// TODO: protection fault should halt machine? or trigger exception?
#define WRITE_REG(m, rx, val) do {\
  if (rx == REG_FR && (m->regs[REG_FR] & FRBIT_PL) && ((m->regs[REG_FR]^(val)) != FRBIT_SYSCALL)) {\
    Printf("protection error: user code at %08X attemps to write %08X to $fr\n", m->regs[REG_PC], val);\
    assert(0);\
  }\
  m->regs[rx] = val;\
} while (0)

void exec_inst(machine_t* m, uint32_t inst)
{
  assert(!((m->regs[REG_PC]) & 3) && "unaligned IF");
  m->regs[REG_PC] += sizeof(uint32_t);
  uint32_t opcode = decode_opcode(inst);
  uint32_t rx = decode_rx(inst);
  uint32_t ry = decode_ry(inst);
  uint32_t rz = decode_rz(inst);
  uint32_t imm = decode_imm16(inst);
  uint32_t immsext = decode_imm16sext(inst);
  uint32_t imm26sext = decode_imm26sext(inst);
  uint32_t la, pa, isport;

#ifdef INSTR_WATCH
#ifdef INSTR_WATCH_START_FROM_FIRST_TIMERINT
  if (n_timint > 0) {
#endif
    dump_inst(m, inst);
#ifdef INSTR_WATCH_START_FROM_FIRST_TIMERINT
  }
#endif
#endif

  int err = 0;
  switch (opcode) {
    case OPCODE_ADD:
      WRITE_REG(m, rx, m->regs[ry] + m->regs[rz]);
      break;
    case OPCODE_SUB:
      WRITE_REG(m, rx, m->regs[ry] - m->regs[rz]);
      break;
    case OPCODE_MUL:
      WRITE_REG(m, rx, m->regs[ry] * m->regs[rz]);
      break;
    case OPCODE_DIV:
      assert(0 && "div not implemented");
      break;
    case OPCODE_AND:
      WRITE_REG(m, rx, m->regs[ry] & m->regs[rz]);
      break;
    case OPCODE_OR:
      WRITE_REG(m, rx, m->regs[ry] | m->regs[rz]);
      break;
    case OPCODE_XOR:
      WRITE_REG(m, rx, m->regs[ry] ^ m->regs[rz]);
      break;
    case OPCODE_LD:
      la = m->regs[ry] + immsext;
      // hw addr v-p translation only happen with paging on
      pa = mmu_la2pa(m, la, &isport, 1);
      if (isport)
        WRITE_REG(m, rx, port_lw(m, pa));
      else
        WRITE_REG(m, rx, mem_lw(m, pa));
      break;
    case OPCODE_ST:
      la = m->regs[ry] + immsext;
      // now assume that there're only .text and .rodata in the image
      if (0xC0000000 <= la && la < img_sz)
          fatal("\n>>> PC=%08X la=%08X val=%08X writing to .text or .rodata!\n",
              m->regs[REG_PC], la, m->regs[rx]);
      pa = mmu_la2pa(m, la, &isport, 1);
      if (isport)
        port_sw(m, pa, m->regs[rx]);
      else
        mem_sw(m, pa, m->regs[rx]);
      break;
    case OPCODE_SHR:
      WRITE_REG(m, rx, m->regs[ry] >> m->regs[rz]);
      break;
    case OPCODE_SHL:
      WRITE_REG(m, rx, m->regs[ry] << m->regs[rz]);
      break;
    case OPCODE_BEQ:
      if (m->regs[rx] == m->regs[ry])
        m->regs[REG_PC] += immsext;
      break;
    case OPCODE_BLT:
      if (m->regs[rx] < m->regs[ry])
        m->regs[REG_PC] += immsext;
      break;
    case OPCODE_ADDIU:
      WRITE_REG(m, rx, m->regs[ry] + immsext);
      break;
    case OPCODE_LUI:
      WRITE_REG(m, rx, imm << 16);
      break;

    case OPCODE_ORI:
      WRITE_REG(m, rx, m->regs[ry] | imm);
      break;
    case OPCODE_LB:
      assert(0 && "lb todo");
      break;
    case OPCODE_SB:
      assert(0 && "sb todo");
      break;
    case OPCODE_BNE:
      if (m->regs[rx] != m->regs[ry])
        m->regs[REG_PC] += immsext;
      break;
    case OPCODE_JR:
      m->regs[REG_PC] = m->regs[rx];
      break;
    case OPCODE_JALR:
      m->regs[REG_LR] = m->regs[REG_PC];
      m->regs[REG_PC] = m->regs[rx];
      break;
    case OPCODE_JSUB:
      m->regs[REG_LR] = m->regs[REG_PC];
      m->regs[REG_PC] += imm26sext;
      break;
    default:
      printf("[%08X]{%08X}: bad opcode %d\n", m->regs[REG_PC], inst, opcode);
      assert(0 && "bad opcode!");
      break;
  }

  uint32_t timer_period = port_lw(m, mmu_la2pa(m, TIMER_PERIOD, NULL, 0));
  m->cycno++;
  if (timer_period != 0 && m->cycno % timer_period == 0) {
    m->regs[REG_FR] |= FRBIT_CLK;
    m->cycno = 0;
  }
}


void check_excep(machine_t* m)
{
  if (m->regs[REG_FR] & FRBIT_HALT) {
    Printf("*** Fatal error! Errno=%d\n", m->regs[11]);
    dump_state(m);
    exit(m->regs[11]);
  }
  // check for ERET
  if (m->regs[REG_FR] & FRBIT_ERET) {
#ifdef EXCEP_WATCH
    Printf("@ [%08X] eret", m->regs[REG_PC]);
    fflush(stdout);
#endif
    assert((!(m->regs[REG_FR] & FRBIT_PL)) && 
        "eret in user mode. have you checked writing to fr in user mode?");
    m->regs[REG_FR] &= ~FRBIT_ERET;
    m->regs[REG_FR] |= FRBIT_GIE; // enable interrupts
    m->regs[REG_PC] = m->regs[REG_EPC] & ~3;
    // epc lowest byte: coming from user mode or not
    if (m->regs[REG_EPC] & 1) {
      // in user mode before exception, return to user mode
      m->regs[REG_FR] |= FRBIT_PL;
    } else {
      // in kernel mode before exception, stay in kernel mode
    }
#ifdef EXCEP_WATCH
    Printf(" to %08X\n", m->regs[REG_PC]);
    dump_fr(m);
    Printf("\n");
#endif
    return;
  }

  if (!(m->regs[REG_FR] & FRBIT_GIE))
    return;

  unsigned excep = 0;
  unsigned regfr = m->regs[REG_FR];

  if (regfr & FRBIT_SYSCALL) {
    if (!(regfr & FRBIT_PL)) {
      Printf("[%08X] syscall in kernel mode\n", m->regs[REG_PC]);
      assert(0);
    }
#ifdef EXCEP_WATCH
    Printf("@ [%08X] syscall %d\n", m->regs[REG_PC], m->regs[11]);
    dump_fr(m);
    Printf("\n");
#endif
    excep = 1;
  }
  if ((regfr & FRBIT_CLKEN) && (regfr & FRBIT_CLK)) {
    n_timint++;
#ifdef EXCEP_WATCH
    Printf("@ [%08X] clk #%d\n", m->regs[REG_PC], n_timint);
    dump_fr(m);
    Printf("\n");
#endif
    excep = 1;
  }
  if ((regfr & FRBIT_UART1_OUTEN) && (regfr & FRBIT_UART1_OUT)) {
#ifdef EXCEP_WATCH
    Printf("@ [%08X] uartout\n", m->regs[REG_PC]);
    dump_fr(m);
    Printf("\n");
#endif
    excep = 1;
  }
  if ((regfr & FRBIT_UART1_INEN) && (regfr & FRBIT_UART1_IN)) {
#ifdef EXCEP_WATCH
    Printf("@ [%08X] uartin\n", m->regs[REG_PC]);
    dump_fr(m);
    Printf("\n");
#endif
    excep = 1;
  }

  if (!excep)
    return;

  m->regs[REG_EPC] = m->regs[REG_PC];
  if (m->regs[REG_FR] & FRBIT_PL)
    m->regs[REG_EPC] |= 1;
  m->regs[REG_FR] &= ~(FRBIT_GIE | FRBIT_PL);  // disable interrupts, enter kernel mode
  m->regs[REG_PC] = port_lw(m, mmu_la2pa(m, IRQ_HANDLER, NULL, 0)); // goto irq handler
}


