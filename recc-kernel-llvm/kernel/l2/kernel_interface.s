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

