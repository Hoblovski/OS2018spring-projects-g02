	.text
	.section .mdebug.abiO32
	.previous

	.globl	putchar_nobusy
	.p2align	2
	.type	putchar_nobusy,@function
	.ent	putchar_nobusy                    # @putchar_nobusy
putchar_nobusy:
	.set	noreorder
	.set	nomacro

# prologue

# unset UART_OUT_READY
  addiu $t0, $zr, 0x200
  addiu $t1, $zr, -1
  xor $t0, $t1, $t0
  and $fr, $fr, $t0

# write arg to UART_OUT_PORT
  lui $t0, 0xFFFF               # t0=0xFFFF0000=UART_OUT_PORT
  sto $a0, 0($t0)

# epilogue
	ret	$lr
	.set	macro
	.set	reorder
	.end	putchar_nobusy
$putchar_nobusy_end:
	.size	putchar_nobusy, ($putchar_nobusy_end)-putchar_nobusy
