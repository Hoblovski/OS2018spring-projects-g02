	.text
	.section .mdebug.abiO32
	.previous

	.globl	getchar_nobusy
	.p2align	2
	.type	getchar_nobusy,@function
	.ent	getchar_nobusy                    # @getchar_nobusy
getchar_nobusy:
	.set	noreorder
	.set	nomacro

# prologue
	addiu	$sp, $sp, -24

# unset bit UART_IN_READY of FR
  addiu $t0, $zr, 0x400
  addiu $t1, $zr, -1
  xor $t0, $t1, $t0
  and $fr, $fr, $t0

# read UART_IN_PORT
  lui $t0, 0xFFFF
  loa $v0, 0x10($t0)   # 0xFFFF0010=UART_IN_PORT

# epilogue
	addiu	$sp, $sp, 24
	ret	$lr
	.set	macro
	.set	reorder
	.end	getchar_nobusy
$getchar_nobusy_end:
	.size	getchar_nobusy, ($getchar_nobusy_end)-getchar_nobusy
