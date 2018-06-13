	.text
	.section .mdebug.abiO32
	.previous

##############################################################################
	.globl	or_into_flags_register
	.p2align	2
	.type	or_into_flags_register,@function
	.ent	or_into_flags_register                    # @taskexit
or_into_flags_register:
	.set	noreorder
	.set	nomacro

  or $fr, $fr, $a0
  ret $lr

	.set	macro
	.set	reorder
	.end	or_into_flags_register
$or_into_flags_register_end:
	.size	or_into_flags_register, ($or_into_flags_register_end)-or_into_flags_register


##############################################################################
	.globl	deassert_bits_in_flags_register
	.p2align	2
	.type	deassert_bits_in_flags_register,@function
	.ent	deassert_bits_in_flags_register                    # @taskexit
deassert_bits_in_flags_register:
	.set	noreorder
	.set	nomacro

  addiu $t0, $zr, -1
  xor $t0, $t0, $a0
  and $fr, $fr, $t0
	ret $lr

	.set	macro
	.set	reorder
	.end	deassert_bits_in_flags_register
$deassert_bits_in_flags_register_end:
	.size	deassert_bits_in_flags_register, ($deassert_bits_in_flags_register_end)-deassert_bits_in_flags_register


##############################################################################

	.globl	write_flags_register
	.p2align	2
	.type	write_flags_register,@function
	.ent	write_flags_register                    # @taskexit
write_flags_register:
	.set	noreorder
	.set	nomacro

  add $fr, $a0, $zr
	ret $lr

	.set	macro
	.set	reorder
	.end	write_flags_register
$write_flags_register_end:
	.size	write_flags_register, ($write_flags_register_end)-write_flags_register

##############################################################################
	.globl	read_flags_register
	.p2align	2
	.type	read_flags_register,@function
	.ent	read_flags_register                    # @taskexit
read_flags_register:
	.set	noreorder
	.set	nomacro

  add $v0, $fr, $zr
  ret $lr

	.set	macro
	.set	reorder
	.end	read_flags_register
$read_flags_register_end:
	.size	read_flags_register, ($read_flags_register_end)-read_flags_register


##############################################################################
	.globl	init_task_stack
	.p2align	2
	.type	init_task_stack,@function
	.ent	init_task_stack                    # @taskexit
init_task_stack:
	.set	noreorder
	.set	nomacro

  # init_task_stack( task_sp_addr, task_pc )
  #   task_sp_addr: intptr_t*,  -> new task's SP
  #   task_pc: from where should this task start execution

  # task stack initial content:
  #
  # top      0    PC (where does the task start)


  loa $t0, 0($a0)

# 模仿中断中保存 PC: 函数的地址
  addiu $t0, $t0, -4
  sto $a1, 0($t0)

# 保存 GPR 到 current 栈上
  addiu $t0, $t0, -44
  sto $zr, 0($t0)
  sto $zr, 4($t0)
  sto $zr, 8($t0)
  sto $zr, 12($t0)
  sto $zr, 16($t0)
  sto $zr, 20($t0)
  sto $zr, 24($t0)
  sto $zr, 28($t0)
  sto $zr, 32($t0)
  sto $zr, 36($t0)
  sto $zr, 40($t0)

  sto $t0, 0($a0)
  ret $lr

	.set	macro
	.set	reorder
	.end	init_task_stack
$init_task_stack_end:
	.size	init_task_stack, ($init_task_stack_end)-init_task_stack


##############################################################################
	.globl	fatal
	.p2align	2
	.type	fatal,@function
	.ent	fatal                    # @fatal
fatal:
	.set	noreorder
	.set	nomacro

  # in a0 holds the fatal cause
  addiu $t1, $zr, 1
  or $fr, $fr, $t1
  # halts the processor

	.set	macro
	.set	reorder
	.end	fatal
$fatal_end:
	.size	fatal, ($fatal_end)-fatal


##############################################################################
	.globl	switch_kstack
	.p2align	2
	.type	switch_kstack,@function
	.ent	switch_kstack
switch_kstack:
	.set	noreorder
	.set	nomacro

  addiu $sp, $sp, -44
  sto $v0, 40($sp)
  sto $v1, 36($sp)
  sto $a0, 32($sp)
  sto $a1, 28($sp)
  sto $s0, 24($sp)
  sto $s1, 20($sp)
  sto $t0, 16($sp)
  sto $t1, 12($sp)
  sto $t2, 8($sp)
  sto $fp, 4($sp)
  sto $lr, 0($sp)
  sto $sp, 4($a1)

  loa $sp, 4($a0)
  loa $v0, 40($sp)
  loa $v1, 36($sp)
  loa $a0, 32($sp)
  loa $a1, 28($sp)
  loa $s0, 24($sp)
  loa $s1, 20($sp)
  loa $t0, 16($sp)
  loa $t1, 12($sp)
  loa $t2, 8($sp)
  loa $fp, 4($sp)
  loa $lr, 0($sp)
  addiu $sp, $sp, 44

  ret $lr

	.set	macro
	.set	reorder
	.end	switch_kstack
$switch_kstack_end:
	.size	switch_kstack, ($switch_kstack_end)-switch_kstack


##############################################################################
	.globl	use_pgdir
	.p2align	2
	.type	use_pgdir,@function
	.ent	use_pgdir                    # @use_pgdir
use_pgdir:
	.set	noreorder
	.set	nomacro

  # t0 is caller save so safe to use
  lui $t0, 0xFFFF
  ori $t0, $t0, 0x0040
  sto $a0, 0($t0)
  ret $lr

	.set	macro
	.set	reorder
	.end	use_pgdir
$use_pgdir_end:
	.size	use_pgdir, ($use_pgdir_end)-use_pgdir


##############################################################################
	.globl	init_kstack
	.p2align	2
	.type	init_kstack,@function
	.ent	init_kstack
init_kstack:
	.set	noreorder
	.set	nomacro

  loa $t0, 0($a0)
  addiu $t0, $t0, -44
  sto $zr, 40($t0) # v0
  sto $zr, 36($t0) # v1
  sto $a1, 32($t0) # a0    = pc_init
  sto $zr, 28($t0) # a1
  sto $zr, 24($t0) # s0
  sto $zr, 20($t0) # s1
  sto $zr, 16($t0) # t0
  sto $zr, 12($t0) # t1
  sto $zr, 8($t0)  # t2
  sto $zr, 4($t0)  # fp
  lui $t1, %hi(kproc_start)
  ori $t1, $t1, %lo(kproc_start)
  sto $t1, 0($t0)  # lr    = kproc_start
  sto $t0, 0($a0)

  ret $lr

	.set	macro
	.set	reorder
	.end	init_kstack
$init_kstack_end:
	.size	init_kstack, ($init_kstack_end)-init_kstack


##############################################################################

# TODO: stupid. 能否把这三个初始化栈的函数统一起来之类的
#   现在这看起来太愚蠢了, init_ukstack, init_ustack, init_kstack
	.globl	init_ukstack
	.p2align	2
	.type	init_ukstack,@function
	.ent	init_ukstack
init_ukstack:
	.set	noreorder
	.set	nomacro

  loa $t0, 0($a0)
  addiu $t0, $t0, -44
  sto $zr, 40($t0) # v0
  sto $zr, 36($t0) # v1
  sto $zr, 32($t0) # a0    = pc_init
  sto $zr, 28($t0) # a1
  sto $zr, 24($t0) # s0
  sto $zr, 20($t0) # s1
  sto $zr, 16($t0) # t0
  sto $zr, 12($t0) # t1
  sto $zr, 8($t0)  # t2
  sto $zr, 4($t0)  # fp
  lui $t1, %hi(irq_ret)
  ori $t1, $t1, %lo(irq_ret)
  sto $t1, 0($t0)  # lr    = kproc_start
  sto $t0, 0($a0)

  ret $lr

	.set	macro
	.set	reorder
	.end	init_ukstack
$init_ukstack_end:
	.size	init_ukstack, ($init_ukstack_end)-init_ukstack


##############################################################################
	.globl	init_ustack
	.p2align	2
	.type	init_ustack,@function
	.ent	init_ustack
init_ustack:
	.set	noreorder
	.set	nomacro

  loa $t0, 0($a0)
  addiu $t0, $t0, -4
  addiu $a1, $a1, 1 # a1 = a1 | 1, epc with lsb=1: from user mode
  sto $a1, 0($t0)  # epc
  addiu $t0, $t0, -44
  sto $zr, 40($t0) # v0
  sto $zr, 36($t0) # v1
  sto $zr, 32($t0) # a0    = pc_init
  sto $zr, 28($t0) # a1
  sto $zr, 24($t0) # s0
  sto $zr, 20($t0) # s1
  sto $zr, 16($t0) # t0
  sto $zr, 12($t0) # t1
  sto $zr, 8($t0)  # t2
  sto $zr, 4($t0)  # fp
  sto $zr, 0($t0)  # lr    
  sto $t0, 0($a0)

  ret $lr

	.set	macro
	.set	reorder
	.end	init_ustack
$init_ustack_end:
	.size	init_ustack, ($init_ustack_end)-init_ustack


##############################################################################
	.globl	read_stack_register
	.p2align	2
	.type	read_stack_register,@function
	.ent	read_stack_register
read_stack_register:
	.set	noreorder
	.set	nomacro

  add $v0, $sp, $zr
  ret $lr

	.set	macro
	.set	reorder
	.end	read_stack_register
$read_stack_register_end:
	.size	read_stack_register, ($read_stack_register_end)-read_stack_register

