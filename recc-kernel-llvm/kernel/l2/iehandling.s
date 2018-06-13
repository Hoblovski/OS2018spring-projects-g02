	.text
	.section .mdebug.abiO32
	.previous


##############################################################################
# 调用系统函数
#   和系统调用不同, 不是通过指令, 而是函数调用
#   调用系统函数 -> 进入 "内核态" (关中断, 切换栈) -> 真正的系统函数 -> 返回...

	.globl	reply_message
	.p2align	2
	.type	reply_message,@function
	.ent	reply_message                    # @taskexit
reply_message:
	.set	noreorder
	.set	nomacro

# 模仿中断中保存 PC
  addiu $sp, $sp, -4
  sto $lr, 0($sp)

# 保存 GPR 到 current 栈上
  addiu $sp, $sp, -44
  sto $v0, 0($sp)
  sto $v1, 4($sp)
  sto $a0, 8($sp)
  sto $a1, 12($sp)
  sto $s0, 16($sp)
  sto $s1, 20($sp)
  sto $t0, 24($sp)
  sto $t1, 28($sp)
  sto $t2, 32($sp)
  sto $fp, 36($sp)
  sto $lr, 40($sp)

  # 参数放在 a0, a1
  addiu $sp, $sp, -12
  sto $a1, 8($sp)
  sto $a0, 4($sp)
  addiu $a0, $zr, 2
  sto $a0, 0($sp)

  # a0: 参数的个数, a1: 系统函数的真正地址
	lui	$a1, %hi(k_reply_message)
	ori	$a1, $a1, %lo(k_reply_message)
	lui	$t1, %hi(do_kernel_method)
	ori	$t1, $t1, %lo(do_kernel_method)
  jr $t1 # 不返回了

	.set	macro
	.set	reorder
	.end	reply_message
$reply_message_end:
	.size	reply_message, ($reply_message_end)-reply_message


	.globl	receive_message
	.p2align	2
	.type	receive_message,@function
	.ent	receive_message                    # @taskexit
receive_message:
	.set	noreorder
	.set	nomacro

# 模仿中断中保存 PC
  addiu $sp, $sp, -4
  sto $lr, 0($sp)

# 保存 GPR 到 current 栈上
  addiu $sp, $sp, -44
  sto $v0, 0($sp)
  sto $v1, 4($sp)
  sto $a0, 8($sp)
  sto $a1, 12($sp)
  sto $s0, 16($sp)
  sto $s1, 20($sp)
  sto $t0, 24($sp)
  sto $t1, 28($sp)
  sto $t2, 32($sp)
  sto $fp, 36($sp)
  sto $lr, 40($sp)

  # a0
  addiu $sp, $sp, -8
  sto $a0, 4($sp)
  addiu $a0, $zr, 1
  sto $a0, 0($sp)

	lui	$a1, %hi(k_receive_message)
	ori	$a1, $a1, %lo(k_receive_message)
	lui	$t1, %hi(do_kernel_method)
	ori	$t1, $t1, %lo(do_kernel_method)
  jr $t1

	.set	macro
	.set	reorder
	.end	receive_message
$receive_message_end:
	.size	receive_message, ($receive_message_end)-receive_message


	.globl	send_message
	.p2align	2
	.type	send_message,@function
	.ent	send_message                    # @taskexit
send_message:
	.set	noreorder
	.set	nomacro

  loa $t0, 8($sp)

# 模仿中断中保存 PC
  addiu $sp, $sp, -4
  sto $lr, 0($sp)

# 保存 GPR 到 current 栈上
  addiu $sp, $sp, -44
  sto $v0, 0($sp)
  sto $v1, 4($sp)
  sto $a0, 8($sp)
  sto $a1, 12($sp)
  sto $s0, 16($sp)
  sto $s1, 20($sp)
  sto $t0, 24($sp)
  sto $t1, 28($sp)
  sto $t2, 32($sp)
  sto $fp, 36($sp)
  sto $lr, 40($sp)

  # 三个参数, a0, a1, 8($sp).   (参考代码容易看出)
  addiu $sp, $sp, -16
  sto $a0, 4($sp)
  sto $a1, 8($sp)
  sto $t0, 12($sp)
  addiu $a0, $zr, 3
  sto $a0, 0($sp)

	lui	$a1, %hi(k_send_message)
	ori	$a1, $a1, %lo(k_send_message)
	lui	$t1, %hi(do_kernel_method)
	ori	$t1, $t1, %lo(do_kernel_method)
  jr $t1

	.set	macro
	.set	reorder
	.end	send_message
$send_message_end:
	.size	send_message, ($send_message_end)-send_message


	.globl	block_on_event
	.p2align	2
	.type	block_on_event,@function
	.ent	block_on_event                    # @taskexit
block_on_event:
	.set	noreorder
	.set	nomacro

# 模仿中断中保存 PC
  addiu $sp, $sp, -4
  sto $lr, 0($sp)

# 保存 GPR 到 current 栈上
  addiu $sp, $sp, -44
  sto $v0, 0($sp)
  sto $v1, 4($sp)
  sto $a0, 8($sp)
  sto $a1, 12($sp)
  sto $s0, 16($sp)
  sto $s1, 20($sp)
  sto $t0, 24($sp)
  sto $t1, 28($sp)
  sto $t2, 32($sp)
  sto $fp, 36($sp)
  sto $lr, 40($sp)

  # 一个参数 a0
  addiu $sp, $sp, -8
  sto $a0, 4($sp)
  addiu $a0, $zr, 1
  sto $a0, 0($sp)

	lui	$a1, %hi(k_block_on_event)
	ori	$a1, $a1, %lo(k_block_on_event)
	lui	$t1, %hi(do_kernel_method)
	ori	$t1, $t1, %lo(do_kernel_method)
  jr $t1

	.set	macro
	.set	reorder
	.end	block_on_event
$block_on_event_end:
	.size	block_on_event, ($block_on_event_end)-block_on_event



	.globl	kernel_exit
	.p2align	2
	.type	kernel_exit,@function
	.ent	kernel_exit                    # @taskexit
kernel_exit:
	.set	noreorder
	.set	nomacro

# 模仿中断中保存 PC
  addiu $sp, $sp, -4
  sto $lr, 0($sp)

# 保存 GPR 到 current 栈上
  addiu $sp, $sp, -44
  sto $v0, 0($sp)
  sto $v1, 4($sp)
  sto $a0, 8($sp)
  sto $a1, 12($sp)
  sto $s0, 16($sp)
  sto $s1, 20($sp)
  sto $t0, 24($sp)
  sto $t1, 28($sp)
  sto $t2, 32($sp)
  sto $fp, 36($sp)
  sto $lr, 40($sp)

  # no args
  addiu $sp, $sp, -4
  add $a0, $zr, $zr
  sto $a0, 0($sp)

	lui	$a1, %hi(k_kernel_exit)
	ori	$a1, $a1, %lo(k_kernel_exit)
	lui	$t1, %hi(do_kernel_method)
	ori	$t1, $t1, %lo(do_kernel_method)
  jr $t1

	.set	macro
	.set	reorder
	.end	kernel_exit
$kernel_exit_end:
	.size	kernel_exit, ($kernel_exit_end)-kernel_exit


	.globl	release_processor
	.p2align	2
	.type	release_processor,@function
	.ent	release_processor                    # @taskexit
release_processor:
	.set	noreorder
	.set	nomacro

# 模仿中断中保存 PC
  addiu $sp, $sp, -4
  sto $lr, 0($sp)

# 保存 GPR 到 current 栈上
  addiu $sp, $sp, -44
  sto $v0, 0($sp)
  sto $v1, 4($sp)
  sto $a0, 8($sp)
  sto $a1, 12($sp)
  sto $s0, 16($sp)
  sto $s1, 20($sp)
  sto $t0, 24($sp)
  sto $t1, 28($sp)
  sto $t2, 32($sp)
  sto $fp, 36($sp)
  sto $lr, 40($sp)

  # no args
  addiu $sp, $sp, -4
  add $a0, $zr, $zr
  sto $a0, 0($sp)

	lui	$a1, %hi(k_release_processor)
	ori	$a1, $a1, %lo(k_release_processor)
	lui	$t1, %hi(do_kernel_method)
	ori	$t1, $t1, %lo(do_kernel_method)
  jr $t1

	.set	macro
	.set	reorder
	.end	release_processor
$release_processor_end:
	.size	release_processor, ($release_processor_end)-release_processor


##############################################################################
# 进入 "内核态"
do_kernel_method:
# 现在  栈还是 current 栈, 上面内容
#       0($sp)  系统函数的参数个数
#       4($sp)  第一个参数
#       ...     第二个参数
#       ...     第三个参数
#       还没有使用四个参数的系统函数
#       ?($sp)  返回地址 ($lr)
# 现在  a0: 参数个数 (和栈顶相同)
#       a1: 系统函数的真正地址

  addiu $t0, $zr, -3  # -3 = ~2
  and $fr, $fr, $t0   # 关中断

# 保存 current 栈
# 注意, 当我们返回到 current 栈是希望栈顶就是返回地址
  # TODO
# 切换到系统栈
  # TODO

# 现在 current 栈的情况
#     |           |   高地址
#     +-----------+
#     | a2        |       12
#     +-----------+
#     | a1        |       8
#     +-----------+
#     | a0        |       4
#     +-----------+
#     | nargs     |       0
#     +-----------+
#     |           |   低地址
#
# XXX: 现在不支持超过三个参数的系统函数!
#
#   $t2: current 栈, $sp: kernel 栈
  # TODO
  jalr $a1

# ...
# 从具体的系统函数返回
# 现在 g_current_sp 会改变了
# 我们希望看到的, 切换到另一个线程时, 他的栈的情况
#     |           |   高地址
#     +-----------+
#     | retaddr   |       0($sp)
#     +-----------+
#     |           |   低地址

# 不用恢复内核栈的 sp: 内核栈的 g_kernel_sp 是只读的
# 切换回到 current 栈

  # now never here (no kernel method except kernel_init)
  addiu $a0, $zr, -1
  addiu $fr, $fr, 1
  # TODO
  beq $zr, $zr, do_eret

##############################################################################
# 中断处理
#   发生中断时, 硬件:
#       push PC           压入返回地址
#       FR &= ~FRBIT_GIE  关中断
#       jmp [IRQ_HANDLER] 从 IRQ_HANDLER = 0xFFFF0020 读取中断向量, 跳转过去

	.globl	irq_handler
	.p2align	2
	.type	irq_handler,@function
	.ent	irq_handler                    # @taskexit
irq_handler:
	.set	noreorder
	.set	nomacro

# epc 被保存到当前栈上.
# 虽然后面把当前 $sp 保存到 cur_proc->ustack,
# 但是这不意味当前栈一定是用户栈, 当前栈仍然可能是内核栈
  addiu $sp, $sp, -4
  sto $epc, 0($sp)

# 保存 GPR 到 current 栈上
  addiu $sp, $sp, -44
  sto $v0, 0($sp)
  sto $v1, 4($sp)
  sto $a0, 8($sp)
  sto $a1, 12($sp)
  sto $s0, 16($sp)
  sto $s1, 20($sp)
  sto $t0, 24($sp)
  sto $t1, 28($sp)
  sto $t2, 32($sp)
  sto $fp, 36($sp)
  sto $lr, 40($sp)

  lui $t0, %hi(cur_proc)
  ori $t0, $t0, %lo(cur_proc)
  loa $t0, 0($t0)
  sto $sp, 0($t0)         # cur_proc->ustack = $sp

  # 判断是不是用户态过来的, 如果是, 那么切换到 (为空的) 内核栈
  lui $t0, -1
  loa $t0, 0x60($t0)
  addiu $t1, $zr, 2048
  and $t0, $t0, $t1
  beq $t0, $zr, $f2

  addiu $a0, $zr, -2
  addiu $fr, $fr, 1     # 现在还没有用户态太好的支持

$f2:
# 跳转到服务例程
  lui	$t0, %hi(k_irq_handler)
  ori	$t0, $t0, %lo(k_irq_handler)
  jalr $t0

# ...
# 从服务例程返回回来

irq_ret:
# 切换到应用栈
  lui	$t0, %hi(cur_proc)
  ori	$t0, $t0, %lo(cur_proc)
  loa $t0, 0($t0)
  loa $sp, 0($t0)       # $sp = cur_proc->ustack, cur_proc 可能已经变了
  beq $zr, $zr, do_eret

do_eret:
# 恢复 GPR
  loa $v0, 0($sp)
  loa $v1, 4($sp)
  loa $a0, 8($sp)
  loa $a1, 12($sp)
  loa $s0, 16($sp)
  loa $s1, 20($sp)
  loa $t0, 24($sp)
  loa $t1, 28($sp)
  loa $t2, 32($sp)
  loa $fp, 36($sp)
  loa $lr, 40($sp)
  addiu $sp, $sp, 44

# epc 被保存到用户栈上
  loa $epc, 0($sp)
  addiu $sp, $sp, 4

# 执行 eret
  or $fr, $fr, $wr

	.set	macro
	.set	reorder
	.end	irq_handler
$irq_handler_end:
	.size	irq_handler, ($irq_handler_end)-irq_handler



	.globl	proc_start
	.p2align	2
	.type	proc_start,@function
	.ent	proc_start
proc_start:
	.set	noreorder
	.set	nomacro
  
  # on entry, a0 should hold the entry point

  # enable interrupts
  #   proc_start is from switch_kstack, during which interrupts are disabled
  addiu $t0, $zr, 2
  or $fr, $fr, $t0    

  jalr $a0

  lui $t0, %hi(k_task_exit)
  ori $t0, $t0, %lo(k_task_exit)
  jr $t0

	.set	macro
	.set	reorder
	.end	proc_start
$proc_start_end:
	.size	proc_start, ($proc_start_end)-proc_start
