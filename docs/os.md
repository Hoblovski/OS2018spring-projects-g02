# 操作系统实现
本次作业中, 在硬件上能够运行原本的 recc 内核.
但是 recc 内核中没有虚拟内存, 也没有用户态.

新的 kernel 初步加入了分页以及不同特权级.
但是实现还非常 naive, 包括没有事实上的异常 (保护异常, 缺页异常直接挂机).
没有外存, 不会有 swap in 和 swap out.

新的内核代码位于 `recc-kernel-llvm/` 中, 实现的功能对应 ucore 中有
1. 中断处理, 外设隔离
2. 物理内存管理和虚拟内存
3. 内核线程和线程切换
4. 用户进程以及相关保护

## 目录结构
```
├── include
│   └── stdarg.h:   用于实现变参 varargs
├── kernel
│   ├── kernel_defs.h:  内核所有的 #define 常量
│   ├── kernel_impl.c:  内核关键函数的实现
│   ├── kernel_state.c: 内核中需要的 enum, struct 等的定义
│   ├── kernel_state.h: 内核中需要的 enum, struct 等的定义
│   ├── l2
│   │   ├── entry.s:  开机入口程序
│   │   ├── getchar_nobusy.s:  串口可读时读取自负
│   │   ├── iehandling.s:   中断和异常的处理
│   │   ├── kernel_interface.s:   需要用汇编实现的内核函数
│   │   ├── putchar_busy.s:   忙等的串口输出
│   │   └── putchar_nobusy.s: 串口输出
│   ├── memman.c:   内存处理, 主要是模拟 load / store byte
│   ├── memman.h:   内存处理, 主要是模拟 load / store byte
│   ├── my_printf.c: printf 的实现, 只支持 %x 修饰符
│   ├── printf.h: printf 的实现
│   ├── private_kernel_interface.h: 只在内部能使用的内核函数 (TODO: 清理)
│   ├── public_kernel_interface.h:  外部能够使用的内核函数 (TODO: 清理)
│   ├── queue.c: 消息队列的实现
│   ├── queue.h: 消息队列的实现
│   ├── user_proc.c: 包含内核线程和用户进程的实现
│   └── user_proc.h: 包含内核线程和用户进程的实现
├── Makefile
└── script.ld:  链接脚本
```

## 中断异常处理
当前实现只支持中断, 异常会被检测, 但是导致挂机而非跳转到异常向量.
**因此以下叙述都针对中断**.

### 发生中断时, 硬件的工作
发生异常时, 保证如下条件满足
1. fr 寄存器中全局中断使能位已经被设置 (软件控制)
2. fr 寄存器中, 该中断对应的使能位已经被设置 (软件控制)
2. fr 寄存器中, 该中断对应的 asserted 位已经被设置 (硬件负责)
硬件检测到中断, 需要
1. 关中断
2. 保存 EPC: EPC 最低位是发生中断时特权级, 1 为 user, 0 为 kernel.
  EPC 第 31-2 位为发生中断时的 PC, 这里 PC 指下一条指令的地址.
3. 进入内核态
4. 从 `IRQ_HANDLER` 中读取异常处理程序的地址, 跳转到这个地址.
`IRQ_HANDLER` 定义在 `kernel_defs.h` 中, 值为 `0xFFFF0020`

### 发生中断时, 软件的工作
从 `irq_handler` (在 `iehandling.s` 中) 开始的, 有如下工作
1. 保存寄存器到栈上
2. 保存栈地址到当前进程的 PCB 中
3. 如果发生中断时程序在用户态, 那么切换到内核栈.
  这时候内核栈应当是空的, 因为不会有执行历史.
4. 跳转到 `k_irq_handler` (在 `kernel_impl.c` 中), 由 C 程序来判断中断类型, 执行相应 ISR (Interrupt Service Routine).
5. 从当前进程的 PCB 中读取此前的栈地址. 这时可能已经发生了进程切换, 导致当前进程不再是第 2. 步中的进程
6. 从栈上恢复寄存器
7. 从中断返回 (通过一条指令 `or $fr, $fr, $wr` 完成, 相当于 x86 中 `iret` 指令, mips 中 `eret` 指令).
  具体说, 是原子地执行
  - 开中断
  - 从 EPC 恢复特权级
  - 返回到 EPC 指向的地址

### 有的问题
* 不支持嵌套中断, 中断优先级
* 最重要的, 没有实质上的异常

## 物理内存管理和虚拟内存
采用页式虚拟内存, 虚拟地址和物理地址均为 32 位长, 页大小 4KB.

### 内存布局
```
+------------------------+ <--00000000
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|      USEG  (3GB)       |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
+------------------------+ <--C0000000
|                        |
|                        |
|                        |
|                        |
|      KSEG  (~1GB)      |
|                        |
|                        |
|                        |
+------------------------+ <--FFFF0000
|                        |
|      PSEG  (64KB)      |
|                        |
+------------------------+ <--FFFFFFFF
```

如上图, 内存分为三个部分.

* `USEG` (user segment): 用户态程序应当在这一块运行.
  虚拟地址如果在这一块, 则需要 MMU 使用 TLB 中的页表项翻译成物理地址.
  TLB 缺失需要是硬件处理.

* `KSEG` (kernel segment): 这一块地址只有内核能访问,
  位于这里面的虚拟地址和其物理地址的对应关系是
  `PA = VA - 0xC0000000`.

* `PSEG` (port segment): 这一块主要是 I/O 端口, 包括特殊内存地址如 `IRQ_HANDLER`.
  在硬件实现上这一块可能并不对应实际的主存, 比如 `IRQ_HANDLER` 可以用一个寄存器来存储.

### 硬件部分
有一个 `PD_POINTER` (在 `machine.h` 中), 它是一个特殊的内存单元, 其中存放当前页目录的地址.
它相当于 x86 的 CR3 寄存器.

在状态寄存器中有一位 `FRBIT_PAGING_ENABLE` (在 `machine.h` 中), 这一位为 1 则表示
使能页模式, 否则物理地址等于虚拟地址.

需要 MMU 和 TLB. 模拟器中相关函数是 `mmu_la2pa` 和 `tlb_lookfor`,
以及 `struct machine_t` 的 `tlb` 域.

TLB 缺失后的重填都是硬件完成, TLB 对软件是完全透明的.

### 软件部分
有一个 `struct Page` 的数组 `pages` (参见 `kernel_state.[ch]`).
对每一个物理页, 都有一个对应的数组项.
其中包含信息如该页是否可用 (没有任何进程正在使用它).

内核完成分配物理页和释放物理页的功能.
具体对应 `calloc_page` 和 `release_page` 函数 (在 `kernel_impl.c` 中), 
它们通过 pages 完成对物理页的分配和释放.
现在的实现中, 一旦物理内存耗尽就会挂机.

内核完成建立虚实映射的功能.
通过 `map_segment(pgdir, la, pa)` (在 `kernel_impl.c` 中) 完成, 其把 `la` 所在的虚拟页映射到 `pa` 所在的物理页.
`map_segment` 调用 `get_pte(pgdir, la, create)` 函数 (在 `kernel_impl.c` 中), 后者用于查找页表项 (和 ucore 中一样).

具体虚拟内存只在用户程序中才涉及, 参见后面进程管理部分. 
内核线程只使用 `KSEG` 和 `PSEG`.

## 进程管理

### 硬件部分
为了实现进程管理, 硬件需要
1. 时钟中断: `TIMER_PERIOD` (在 `kernel_defs.h` 中) 地址的内容是多少个周期触发一次时钟中断.
2. 提供区分不同 cache 的方式, 这里 cache 也包含我们的 TLB (认为是页表项的 cache).
  如提供指令 flush 缓存, 或者缓存的标志位上加一个进程 pid. 这个我们没有实现, **是一个 bug**.

### 软件部分
有一个 `struct process_control_block` 的数组 `pcbs` (在 `kernel_state.[ch]` 中).
对每個进程, 都有一个数组项对应. 可以通过 `alloc_proc` 函数 (在 `kernel_state.[ch]` 中) 分配.

内核中有进程调度的函数 `sched` (在 `kernel_impl.c` 中),
其中使用一个非常 naive 的 8 行算法选择换入的进程 `next_proc`,
之后从 `cur_proc` (参见 `kernel_state.[ch]) 切换到 `next_proc`.

切换的过程全程关中断.
切换最重要的一步就是切换内核栈,
因为栈上面保存了所有寄存器, 以及执行流 (即保存了返回地址),
这个工作由 `switch_kstack` 函数 (参见 `kernel_interface.s`) 完成.

### 用户态进程的权限管理

用户态进程的权限管理显然只能通过硬件实现, 硬件需要监视用户态程序的如下越权行为
1. 执行特权指令, 如 `eret`, `or $fr, $fr, 1` 等
2. 访问 `KSEG` 和 `PSEG`.

在我们不完整的实现中, 一旦发生保护错误 (i.e. 用户程序越权), 硬件就会挂机.
事实上应当也类似中断, 设置中断类型之后跳转到处理程序.

## 线程通讯

内核线程之间通过消息通讯, 基本原语是
* `send(msg, dest_pid)`
* `recv()`
具体代码在 `kernel_impl.c` 中 `k_send_message` 和 `k_receive_message` 函数.

用户线程和内核线程使用系统调用来通讯.

