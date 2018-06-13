#ifndef OP_CPU_H_
#define OP_CPU_H_
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
#define NULL (0)

#define UART1_OUT        0xffff0000u
#define UART1_IN         0xffff0010u
#define IRQ_HANDLER      0xffff0020u
#define TIMER_PERIOD     0xffff0030u
#define PD_POINTER 0xffff0040
#define PAGEFAULT_BADVA 0xffff0050
#define SYSCALL_ID 0xffff0060
#define SYSCALL_ARGS 0xffff0070

#define HALTED_BIT                          (1u << 0u)
#define GLOBAL_INTERRUPT_ENABLE_BIT         (1u << 1u)
#define RTE_BIT                             (1u << 2u)
#define TIMER1_ENABLE_BIT                   (1u << 3u)
#define TIMER1_ASSERTED_BIT                 (1u << 4u)
#define UART1_OUT_ENABLE_BIT                (1u << 5u)
#define UART1_OUT_ASSERTED_BIT              (1u << 6u)
#define UART1_IN_ENABLE_BIT                 (1u << 7u)
#define UART1_IN_ASSERTED_BIT               (1u << 8u)
#define UART1_OUT_READY_BIT                 (1u << 9u)
#define UART1_IN_READY_BIT                  (1u << 10u)
// we don't have divisions at all
// #define DIV_ZERO_ASSERTED_BIT               (1u << 11u)
#define PRIVILEGE_BIT                       (1u << 11u)
#define PAGE_FAULT_EXCEPTION_ASSERTED_BIT   (1u << 12u)
#define PAGEING_ENABLE_BIT                  (1u << 13u)
#define SYSCALL_BIT                         (1u << 14u)

#define INITIAL_TIMER_PERIOD_VALUE 0x10000

#define USEG_BEGIN 0
#define KSEG_BEGIN 0xc0000000
#define PSEG_BEGIN 0xffff0000
// XXX crazy hack: linker can't provide an `_end` symbol
//  so do it myself
#define IMAGE_END (KSEG_BEGIN + 0x10000)

// TODO: rwx check
#define PAGE_SIZE_WIDTH 12
#define PAGE_TABLE_SIZE_WIDTH 10
#define PAGE_DIR_SIZE_WIDTH 10
#define PAGE_SIZE (1<<PAGE_SIZE_WIDTH)
#define PDSHIFT (PAGE_SIZE_WIDTH+PAGE_TABLE_SIZE_WIDTH)
#define PTSHIFT (PAGE_SIZE_WIDTH)
#define PTMASK ((1<<PAGE_TABLE_SIZE_WIDTH)-1)
#define POFFSETMASK ((1<<PAGE_SIZE_WIDTH)-1)
// 128 MB of physical address space
//  actually it should be probed, but anyway -- this stupid hack
#define MEMSIZE (128<<20)
// kernel reserves 1MB of these
#define KMEMSIZE (1<<20)
#define PDE_FLAGS_P 1
#define PTE_FLAGS_P 1
#define GET_PDIDX(addr) ((addr) >> PDSHIFT)
#define GET_PTIDX(addr) (((addr) >> PTSHIFT) & PTMASK)
#define GET_PN(addr) ((addr) & ~POFFSETMASK)
#define GET_POFFSET(addr) ((addr) & POFFSETMASK)
#define PA2KLA(addr) ((addr) + KSEG_BEGIN)
#define KLA2PA(addr) ((addr) - KSEG_BEGIN)
// this bit: is this page frame reserved for kernel, and can't be alloc'ed or freed?
#define PF_KRESERVE 1

#define STACK_SIZE PAGE_SIZE

/* ensure that MAX_NUM_PROCESSES is an exponent of 2 */
#define MAX_NUM_PROCESSES 8
#define MAX_MSGS 20

#define HELLO_MSG_PID_EXPECTED 1
#define UART_IN_PID_EXPECTED 2
#define UART_OUT_PID_EXPECTED 3

#endif
