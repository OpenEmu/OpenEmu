/*
 * libco.ppc-elf
 * author: Kernigh
 * license: public domain
 *
 * PowerPC 32-bit ELF implementation of libco (for compile with GCC),
 * ported from PowerPC Mac OS X implementation (ppc.s) by Vas Crabb.
 * This ELF version works for OpenBSD, and might also work for FreeBSD,
 * NetBSD and Linux.
 *
 * Note 1:	This implementation does not handle the AltiVec/VMX
 *		registers, because the ELF ABI does not mention them,
 *		and my OpenBSD system is not using them.
 *
 * Note 2:	If you want position-independent code, then you must
 *		define __PIC__. gcc -fpic or -fPIC defines __PIC__, but
 *		gcc -fpie or -fPIE might not. If you want to use -fpie
 *		or -fPIE, then you might need a manual definition:
 *			gcc -fpie -D__PIC__=1
 *			gcc -fPIE -D__PIC__=2
 *
 * The ELF ABI is "System V Application Binary Interface, PowerPC
 * Processor Supplement", which you can get from
 *   <http://refspecs.linux-foundation.org/elf/elfspec_ppc.pdf>
 * (PDF file, hosted by Linux Foundation).
 *
 * ELF and Mac OS X use similar conventions to allocate the registers,
 * and to pass arguments and return values through registers. The main
 * differences are that ELF has a slightly different stack format, that
 * symbols are different (and without an extra underscore at the start),
 * and that the assembly syntax is different.
 *
 * A function may destroy the values of volatile registers, but must
 * preserve the values of nonvolatile registers. So the co_switch()
 * function only saves the nonvolatile registers.
 *
 * [nonvolatile registers in ELF]
 *   %r1, %r14..%r31
 *   %f14..%f31
 *   %cr2..%cr4 in cr
 *
 * [volatile registers in ELF]
 *   %r0, %r3..%r10
 *   %f0..%f13
 *   %cr0, %cr1, %cr5..%cr7 in cr
 *   ctr, lr, xer
 *
 * lr (link register) is volatile, but it contains the return address,
 * so co_switch must save lr.
 *
 * %r13 is the small data pointer. This is constant across threads, so
 * co_switch() does not touch %r13.
 *
 * %r2 is a reserved register, so co_switch() does not touch %r2. Some
 * systems might borrow an idea from the PowerPC Embedded ABI, and might
 * use %r2 as a small read-only data pointer, which is constant across
 * threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef void * cothread_t;

/*
 * co_active_context is either in a global offset table (if we are
 * compiling -fPIC or -fPIE) or has an absolute position.
 */
static void *co_main_stack_pointer;
static cothread_t co_active_context = &co_main_stack_pointer;

extern cothread_t co_active() {
  return co_active_context;
}

/*
 * Embedded assembly.
 *
 * We are not using the percent-sign substitution feature,
 * so we must write "%r1", not "%%r1".
 *
 * We always write 'bl malloc@plt', not 'bl malloc'. The '@plt'
 * is necessary in position-indepent code and seems to have no
 * significant effect in fixed-position code.
 *
 * We never use the 'lmw' or 'stmw' instructions. The ELF ABI
 * mentions that these instructions "are usually slower than
 * a sequence of other instructions that have the same effect."
 * We instead use sequences of 'lwz' or 'stz' instructions.
 */
__asm__("\n"
"### embedded assembly						\n"
".section \".text\"						\n"
"	.balign	4						\n"
"								\n"
/*
 * void co_switch(co_thread to %r3)
 *
 * Allocate our stack frame of 240 bytes:
 * Old      New        Value
 *  4(%r1)   244(%r1)   return address, used by us
 *  0(%r1)   240(%r1)   frame pointer
 *           232(%r1)   %f31
 *           224(%r1)   %f30
 *                      ...
 *            96(%r1)   %f14
 *            92(%r1)   %r31
 *            88(%r1)   %r30
 *                      ...
 *            24(%r1)   %r14
 *            20(%r1)   condition register
 *             8(%r1)   padding of 12 bytes
 *             4(%r1)   return address, never used
 *             0(%r1)   frame pointer
 *
 * Save our registers in our stack frame.
 * Save our stack pointer in 0(%r4).
 * Switch to the stack of the other thread.
 * Restore registers and return.
 */
"	.globl	co_switch					\n"
"	.type	co_switch, @function				\n"
"co_switch:							\n"
"	mflr	%r0		# %r0 = return address		\n"
"	mfcr	%r9		# %r9 = condition register	\n"
"	stwu	%r1, -240(%r1)	# allocate stack frame		\n"
"								\n"
"	stw	%r0, 244(%r1)	# save return address		\n"
"	stfd	%f31, 232(%r1)	# save floating-point regs	\n"
"	stfd	%f30, 224(%r1)					\n"
"	stfd	%f29, 216(%r1)					\n"
"	stfd	%f28, 208(%r1)					\n"
"	stfd	%f27, 200(%r1)					\n"
"	stfd	%f26, 192(%r1)					\n"
"	stfd	%f25, 184(%r1)					\n"
"	stfd	%f24, 176(%r1)					\n"
"	stfd	%f23, 168(%r1)					\n"
"	stfd	%f22, 160(%r1)					\n"
"	stfd	%f21, 152(%r1)					\n"
"	stfd	%f20, 144(%r1)					\n"
"	stfd	%f19, 136(%r1)					\n"
"	stfd	%f18, 128(%r1)					\n"
"	stfd	%f17, 120(%r1)					\n"
"	stfd	%f16, 112(%r1)					\n"
"	stfd	%f16, 104(%r1)					\n"
"	stfd	%f14, 96(%r1)					\n"
"	stw	%r31, 92(%r1)	# save general-purpose regs	\n"
"	stw	%r30, 88(%r1)					\n"
"	stw	%r29, 84(%r1)					\n"
"	stw	%r28, 80(%r1)					\n"
"	stw	%r27, 76(%r1)					\n"
"	stw	%r26, 72(%r1)					\n"
"	stw	%r25, 68(%r1)					\n"
"	stw	%r24, 64(%r1)					\n"
"	stw	%r23, 60(%r1)					\n"
"	stw	%r22, 56(%r1)					\n"
"	stw	%r21, 52(%r1)					\n"
"	stw	%r20, 48(%r1)					\n"
"	stw	%r19, 44(%r1)					\n"
"	stw	%r18, 40(%r1)					\n"
"	stw	%r17, 36(%r1)					\n"
"	stw	%r16, 32(%r1)					\n"
"	stw	%r15, 28(%r1)					\n"
"	stw	%r14, 24(%r1)					\n"
"	stw	%r9, 20(%r1)	# save condition reg		\n"
"								\n"
"	# save current context, set new context			\n"
"	#   %r4 = co_active_context				\n"
"	#   co_active_context = %r3				\n"
#if __PIC__ == 2
"	# position-independent code, large model (-fPIC)	\n"
"	bl	_GLOBAL_OFFSET_TABLE_@local-4			\n"
"	mflr	%r8		# %r8 = address of got		\n"
"	addis	%r7, %r8, co_active_context@got@ha		\n"
"	lwz	%r6, co_active_context@got@l(%r7)		\n"
"	lwz	%r4, 0(%r6)					\n"
"	stw	%r3, 0(%r6)					\n"
#elif __PIC__ == 1
"	# position-independent code, small model (-fpic)	\n"
"	bl	_GLOBAL_OFFSET_TABLE_@local-4			\n"
"	mflr	%r8		# %r8 = address of got		\n"
"	lwz	%r7, co_active_context@got(%r8)			\n"
"	lwz	%r4, 0(%r7)					\n"
"	stw	%r3, 0(%r7)					\n"
#else
"	# fixed-position code					\n"
"	lis	%r8, co_active_context@ha			\n"
"	lwz	%r4, co_active_context@l(%r8)			\n"
"	stw	%r3, co_active_context@l(%r8)			\n"
#endif
"								\n"
"	# save current stack pointer				\n"
"	stw	%r1, 0(%r4)					\n"
"	# get new stack pointer					\n"
"	lwz	%r1, 0(%r3)					\n"
"								\n"
"	lwz	%r0, 244(%r1)	# get return address		\n"
"	lfd	%f31, 232(%r1)	# restore floating-point regs	\n"
"	lfd	%f30, 224(%r1)					\n"
"	lfd	%f29, 216(%r1)					\n"
"	lfd	%f28, 208(%r1)					\n"
"	lfd	%f27, 200(%r1)					\n"
"	lfd	%f26, 192(%r1)					\n"
"	lfd	%f25, 184(%r1)					\n"
"	lfd	%f24, 176(%r1)					\n"
"	lfd	%f23, 168(%r1)					\n"
"	lfd	%f22, 160(%r1)					\n"
"	lfd	%f21, 152(%r1)					\n"
"	lfd	%f20, 144(%r1)					\n"
"	lfd	%f19, 136(%r1)					\n"
"	lfd	%f18, 128(%r1)					\n"
"	lfd	%f17, 120(%r1)					\n"
"	lfd	%f16, 112(%r1)					\n"
"	lfd	%f16, 104(%r1)					\n"
"	lfd	%f14, 96(%r1)					\n"
"	lwz	%r31, 92(%r1)	# restore general-purpose regs	\n"
"	lwz	%r30, 88(%r1)					\n"
"	lwz	%r29, 84(%r1)					\n"
"	lwz	%r28, 80(%r1)					\n"
"	lwz	%r27, 76(%r1)					\n"
"	lwz	%r26, 72(%r1)					\n"
"	lwz	%r25, 68(%r1)					\n"
"	lwz	%r24, 64(%r1)					\n"
"	lwz	%r23, 60(%r1)					\n"
"	lwz	%r22, 56(%r1)					\n"
"	lwz	%r21, 52(%r1)					\n"
"	lwz	%r20, 48(%r1)					\n"
"	lwz	%r19, 44(%r1)					\n"
"	lwz	%r18, 40(%r1)					\n"
"	lwz	%r17, 36(%r1)					\n"
"	lwz	%r16, 32(%r1)					\n"
"	lwz	%r15, 28(%r1)					\n"
"	lwz	%r14, 24(%r1)					\n"
"	lwz	%r9, 20(%r1)	# get condition reg		\n"
"								\n"
"	addi	%r1, %r1, 240	# free stack frame		\n"
"	mtlr	%r0		# restore return address	\n"
"	mtcr	%r9		# restore condition register	\n"
"	blr			# return			\n"
"	.size	co_switch, . - co_switch			\n"
"								\n"
/*
 * cothread_t %r3 co_create(unsigned int stack_size %r3,
 *			    void (*coentry %r4)())
 *
 * Allocate a new stack, such that when you co_switch to that
 * stack, then co_switch returns to coentry.
 */
"	.globl	co_create					\n"
"	.type	co_create, @function				\n"
"co_create:							\n"
"	mflr	%r0		# %r0 = return address		\n"
"	stwu	%r1, -16(%r1)	# allocate my stack frame	\n"
"	stw	%r0, 20(%r1)	# save return address		\n"
"	stw	%r31, 12(%r1)	# save %r31			\n"
"	stw	%r30, 8(%r1)	# save %r30			\n"
"								\n"
"	mr	%r30, %r3	# %r30 = stack_size		\n"
"	mr	%r31, %r4	# %r31 = coentry		\n"
"								\n"
"	# Call malloc(stack_size %r3) to allocate stack;	\n"
"	# malloc() probably uses good alignment.		\n"
"	#							\n"
"	bl	malloc@plt	# returns %r3 = low end		\n"
"	cmpwi	%r3, 0		# if returned NULL,		\n"
"	beq-	1f		#   then abort			\n"
"								\n"
"	# we return %r3 = low end of stack			\n"
"	add	%r4, %r3, %r30	# %r4 = high end of stack	\n"
"								\n"
"	# uncomment if malloc() uses wrong alignment		\n"
"	#rlwinm	%r4,%r4,0,0,27	# force 16-byte alignment 	\n"
"								\n"
	/*
	 * Allocate two stack frames:
	 *   16 bytes for stack frame with return address
	 *  240 bytes for co_switch stack frame
	 *
	 * Old         New         Value
	 *   -8(%r4)    248(%r5)    padding of 8 bytes
	 *  -12(%r4)    244(%r5)    return address = coentry
	 *  -16(%r4)    240(%r5)    frame pointer = NULL
	 *              232(%r5)    %f31 = 0
	 *                          ...
	 *               20(%r5)    condition register = 0
	 *                0(%r5)    frame pointer
	 */
"	li	%r9, (240-20)/4+1				\n"
"	addi	%r5, %r4, -16	# allocate first stack frame	\n"
"	li	%r0, 0						\n"
"	stwu	%r5, -240(%r5)	# allocate second stack frame	\n"
"	li	%r8, 20						\n"
"	mtctr	%r9		# loop %r9 times		\n"
"2:	# loop to store zero to 20(%r5) through 240(%r5)	\n"
"	stwx	%r0, %r5, %r8					\n"
"	addi	%r8, %r8, 4	# index += 4			\n"
"	bdnz+	2b		# ctr -= 1, branch if nonzero	\n"
"								\n"
"	stw	%r31, 244(%r5)	# return address = coentry	\n"
"	stw	%r5, 0(%r3)	# save stack pointer		\n"
"								\n"
"	lwz	%r0, 20(%r1)	# get return address		\n"
"	lwz	%r31, 12(%r1)	# restore %r31			\n"
"	lwz	%r30, 8(%r1)	# restore %r30			\n"
"	mtlr	%r0		# restore return address	\n"
"	addi	%r1, %r1, 16	# free stack frame		\n"
"	blr			# return			\n"
"								\n"
"1:	b	abort@plt	# branch 1f to abort		\n"
"	.size	co_create, . - co_create			\n"
"								\n"
/*
 * void co_delete(cothread_t) => void free(void *)
 */
"	.globl	co_delete					\n"
"	.type	co_delete, @function				\n"
"co_delete:							\n"
"	b	free@plt					\n"
"								\n"
);

#ifdef __cplusplus
}
#endif
