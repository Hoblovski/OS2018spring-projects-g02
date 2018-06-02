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

#define PAGE_SIZE_WIDTH 10
#define PAGE_TABLE_SIZE_WIDTH 11
#define PAGE_DIR_SIZE_WIDTH 11
// 128 MB of physical address space
//  actually it should be probed, but anyway -- this stupid hack
#define MEMSIZE (128<<20)
// kernel reserves 1MB of these
#define KMEMSIZE (1<<20)

#define OP_CPU_PAGE_SIZE (1 << PAGE_SIZE_WIDTH)

#define PAGE_OFFSET_MASK (0xFFFFFFFF >> (32u - PAGE_SIZE_WIDTH))
#define LEVEL_1_PAGE_TABLE_INDEX_MASK 0x001FFC00
#define LEVEL_2_PAGE_TABLE_INDEX_MASK 0xFFE00000

#define LEVEL_1_PAGE_TABLE_ENTRY_EXECUTE_BIT (1 << 0)
#define LEVEL_1_PAGE_TABLE_ENTRY_WRITE_BIT   (1 << 1)
#define LEVEL_1_PAGE_TABLE_ENTRY_READ_BIT    (1 << 2)
#define LEVEL_2_PAGE_TABLE_ENTRY_INITIALIZED (0x1 << 9)
#define LEVEL_1_PAGE_TABLE_ENTRY_INITIALIZED (0x1 << 9)

#define BITS_PER_BRANCH_DIST  9u
#define BITS_PER_LITERAL     16u
#define BITS_PER_OP_CODE      5u
#define OP_CODE_OFFSET       27u
#define BITS_PER_REGISTER     9u
#define ra_OFFSET            18u
#define rb_OFFSET            (ra_OFFSET - BITS_PER_REGISTER)
#define rc_OFFSET            (rb_OFFSET - BITS_PER_REGISTER)

#define UNSHIFTED_OP_CODE_MASK   (0xFFFFFFFF >> (32u - BITS_PER_OP_CODE))
#define OP_CODE_MASK             (UNSHIFTED_OP_CODE_MASK << OP_CODE_OFFSET)
#define UNSHIFTED_REGISTER_MASK  (0xFFFFFFFF >> (32u - BITS_PER_REGISTER))
#define ra_MASK                  (UNSHIFTED_REGISTER_MASK << ra_OFFSET)
#define rb_MASK                  (UNSHIFTED_REGISTER_MASK << rb_OFFSET)
#define rc_MASK                  (UNSHIFTED_REGISTER_MASK << rc_OFFSET)
#define LITERAL_MASK             (0xFFFFFFFF >> (32u - BITS_PER_LITERAL))
#define BRANCH_DISTANCE_MASK     (0xFFFFFFFF >> (32u - BITS_PER_BRANCH_DIST))

#define BRANCH_DISTANCE_SIGN_BIT 0x100

#define MAX_LL_CONSTANT 0xFFFF
#define MAX_BRANCH_POS 255
#define MAX_BRANCH_NEG 256

#define UART1_OUT        0xffff0000u
#define UART1_IN         0xffff0010u
#define IRQ_HANDLER      0xffff0020u
#define TIMER_PERIOD     0xffff0030u

#define PC_index 0u
#define SP_index 1u
#define FR_index 4u
#define WR_index 5u

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
#define DIV_ZERO_ASSERTED_BIT               (1u << 11u)
#define PAGE_FAULT_EXCEPTION_ASSERTED_BIT   (1u << 12u)
#define PAGEING_ENABLE_BIT                  (1u << 13u)

#define INITIAL_TIMER_PERIOD_VALUE 0xA000

#define USEG_BEGIN 0
#define KSEG_BEGIN 0xc0000000
#define PSEG_BEGIN 0xffff0000
#endif
