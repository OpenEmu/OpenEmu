/*---------------------------------------------------------------------------------
	$Id$

	Copyright (C) 2005
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.

	$Log: interruptDispatcher.s,v $
	Revision 1.10  2007/08/11 06:00:23  wntrmute
	make nesting really work

	Revision 1.9  2007/01/10 15:48:27  wntrmute
	remove unused code

	Revision 1.8  2006/12/16 09:10:02  wntrmute
	acknowledge interrupt before calling handler

	Revision 1.7  2006/04/26 05:11:31  wntrmute
	rebase dtcm, take __irq_flags and __irq_vector from linker script
	move arm7 irq vector & irq flags to actual locations

	Revision 1.6  2006/04/23 18:19:15  wntrmute
	reworked interrupt code to allow dtcm moving

	Revision 1.5  2005/12/12 13:01:55  wntrmute
	disable interrupts on return from user handler

	Revision 1.4  2005/10/21 22:43:42  wntrmute
	restore REG_IME on exit from null handler

	Revision 1.3  2005/09/27 18:21:53  wntrmute
	safer nested interrupt support

	Revision 1.2  2005/09/04 16:37:01  wntrmute
	check for NULL handler

	Revision 1.1  2005/09/03 17:09:35  wntrmute
	added interworking aware interrupt dispatcher


---------------------------------------------------------------------------------*/

#ifdef ARM7
	.text
#endif

#ifdef ARM9
	.section	.itcm,"ax",%progbits
#endif

	.extern	irqTable
	.code 32

	.global	OurIntrMain
@---------------------------------------------------------------------------------
OurIntrMain:
@---------------------------------------------------------------------------------
	mov	r3, #0x4000000		@ REG_BASE

	ldr	r1, [r3, #0x208]	@ r1 = IME
	str	r3, [r3, #0x208]	@ disable IME
	mrs	r0, spsr
	stmfd	sp!, {r0-r1,r3,lr}	@ {spsr, IME, REG_BASE, lr_irq}

	ldr	r1, [r3,#0x210]		@ REG_IE
	ldr	r2, [r3,#0x214]		@ REG_IF
	and	r1,r1,r2

	ldr	r0,=__irq_flags		@ defined by linker script

	ldr	r2,[r0]
	orr	r2,r2,r1
	str	r2,[r0]

	ldr	r2,=irqTable
@---------------------------------------------------------------------------------
findIRQ:
@---------------------------------------------------------------------------------
	ldr r0, [r2, #4]
	cmp	r0,#0
	beq	no_handler
	ands	r0, r0, r1
	bne	jump_intr
	add	r2, r2, #8
	b	findIRQ

@---------------------------------------------------------------------------------
no_handler:
@---------------------------------------------------------------------------------
	str	r1, [r3, #0x0214]	@ IF Clear
	ldmfd   sp!, {r0-r1,r3,lr}	@ {spsr, IME, REG_BASE, lr_irq}
	str	r1, [r3, #0x208]	@ restore REG_IME
	mov	pc,lr

@---------------------------------------------------------------------------------
jump_intr:
@---------------------------------------------------------------------------------
	ldr	r1, [r2]		@ user IRQ handler address
	cmp	r1, #0
	bne	got_handler
	mov	r1, r0
	b	no_handler
@---------------------------------------------------------------------------------
got_handler:
@---------------------------------------------------------------------------------

	mrs	r2, cpsr
	bic	r2, r2, #0xdf		@ \__
	orr	r2, r2, #0x1f		@ /  --> Enable IRQ & FIQ. Set CPU mode to System.
	msr	cpsr,r2

	str	r0, [r3, #0x0214]	@ IF Clear

	sub	r13,r13,#256
	push	{lr}
	adr	lr, IntrRet
	bx	r1

@---------------------------------------------------------------------------------
IntrRet:
@---------------------------------------------------------------------------------
	mov	r3, #0x4000000		@ REG_BASE
	str	r3, [r3, #0x208]	@ disable IME
	pop	{lr}
	add	r13,r13,#256

	mrs	r3, cpsr
	bic	r3, r3, #0xdf		@ \__
	orr	r3, r3, #0x92		@ /  --> Disable IRQ. Enable FIQ. Set CPU mode to IRQ.
	msr	cpsr, r3

	ldmfd   sp!, {r0-r1,r3,lr}	@ {spsr, IME, REG_BASE, lr_irq}
	msr	spsr, r0		@ restore spsr
	str	r1, [r3, #0x208]	@ restore REG_IME
	mov	pc,lr

	.pool
	.end
