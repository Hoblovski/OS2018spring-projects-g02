  .text

	.globl	main
	.p2align	2
	.type	main,@function
	.ent	main										# @main
main:
	.set	noreorder
	.set	nomacro
# BB#0:

	lui	$sp, %hi(init_stack)
	ori	$sp, $sp, %lo(init_stack)
  # TODO: stupid hack, use a macro?
	addiu $sp, $sp, 4092
	# using init stack

	lui	$t0, %hi(kernel_init)
	ori	$t0, $t0, %lo(kernel_init)
	jalr $t0

	# never here, raise errno -1
	addiu $a0, $zr, -1
  addiu $fr, $fr, 1

	.set	macro
	.set	reorder
	.end	main
$func_end0:
	.size	main, ($func_end0)-main


	.globl	kernel_init
	.p2align	2
	.type	kernel_init,@function
	.ent	kernel_init                    # @taskexit
kernel_init:
	.set	noreorder
	.set	nomacro

	lui	$t0, %hi(k_kernel_init)
	ori	$t0, $t0, %lo(k_kernel_init)
  jr $t0

	.set	macro
	.set	reorder
	.end	kernel_init
$kernel_init_end:
	.size	kernel_init, ($kernel_init_end)-kernel_init


