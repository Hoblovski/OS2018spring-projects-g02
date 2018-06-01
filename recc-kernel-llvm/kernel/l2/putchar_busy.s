	.text
	.section .mdebug.abiO32
	.previous

	.globl	putchar_busy
	.p2align	2
	.type	putchar_busy,@function
	.ent	putchar_busy                    # @putchar_busy
putchar_busy:
	.set	noreorder
	.set	nomacro

# prologue

# loop while ((FR & UART_OUT_READY) == 0)
  addiu $t0, $zr, 0x200
$eval_fr_uart_out_ready_bit:
  and $t1, $fr, $t0
  beq $t1, $zr, $eval_fr_uart_out_ready_bit

# unset UART_OUT_READY
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
	.end	putchar_busy
$putchar_busy_end:
	.size	putchar_busy, ($putchar_busy_end)-putchar_busy
