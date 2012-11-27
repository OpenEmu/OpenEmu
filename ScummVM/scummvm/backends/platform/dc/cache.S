
	.globl	_flush_instruction_cache

	.align 2

	! Flush the SH instruction cache

_flush_instruction_cache:
	mova	fcc,r0
	mov.l	p2_mask,r1
	or	r1,r0
	jmp	@r0
	nop
	nop
fcc:
	mov.l	ccr_addr,r0
	mov.l	ccr_data,r1
	mov.l	r1,@r0
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	rts
	nop

	.align	2

p2_mask:
	.long	0xa0000000
ccr_addr:
	.long	0xff00001c
ccr_data:
	.word	0x0905
