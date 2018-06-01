#include "uart.h"

/*
 * feed a character into the machine (from emulator display)
 */
unsigned uart_feed(struct machine_t* m, unsigned c){
  if (m->regs[REG_FR] & FRBIT_UART1_INRDY)
    return 1; // machine cannot consume input character c
  // now that machine can consume c, just by going into the uart interrupt
  // generate an interrupt
  m->regs[REG_FR] |= FRBIT_UART1_INRDY;
  m->regs[REG_FR] |= FRBIT_UART1_IN;
  port_sw(m, mmu_la2pa(m, UART1_IN, NULL), c);
	return 0; // machine consumed input character
}

/*
 * pulls a character out from machine (onto emulator display)
 */
unsigned uart_request(struct machine_t * m, unsigned * rtn){
  static int wait = UART_INIT_WAIT;
  if (m->regs[REG_FR] & FRBIT_UART1_OUTRDY)
    return 1; // the machine has nothing to emit
  if (wait == -1) {
    // the machine has something to emit, pull it out
    m->regs[REG_FR] |= FRBIT_UART1_OUTRDY;
    m->regs[REG_FR] |= FRBIT_UART1_OUT;
    // pull output
    wait = UART_RANDOM_WAIT;
    *rtn = port_lw(m, mmu_la2pa(m, UART1_OUT, NULL));
    return 0;
  } else {
    wait--;
    return 1;
  }
}
