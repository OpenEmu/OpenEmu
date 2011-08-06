// Core processor state swap function.
// Only used by ppc.c if LIBCO_PPC_ASM is defined.
// Main purpose of this asm file is to allow easy development of improvements.

// If this isn't available, you can just do -mregnames when compiling
#include "ppc-asm.h"

.section ".text"
.align 5
.globl co_swap_asm
.type co_swap_asm, @function

// Wrap GPR load/stores so that we can use the same code for
// PPC and PPC64.
#if _ARCH_PPC64 || __PPC64__ || __ppc64__ || __powerpc64__
	#define STWD		std
	#define LWZD		ld
	#define GPR_SIZE	8
#else
	#define LIBCO_PPC32 1
	#define STWD		stw
	#define LWZD		lwz
	#define GPR_SIZE	4
#endif

// Offsets of values in memory
#define PC_OFF		32
#define SP_OFF		40
#define CR_OFF		48
#define VRSAVE_OFF	52
#define GPR_OFF		56
#define FPR_OFF 	224
#define VR_OFF 		384

// Avoid stupid coding errors by letting the preprocessor
// keep things consistent for us.
#define FPR(n)	FPR_OFF - 8*14 + 8*n
#define GPR(n)	GPR_OFF - GPR_SIZE*12 + GPR_SIZE*n

#define FPR_NAME(n)	f##n
#define GPR_NAME(n)	r##n

// REG_MEM(FPR,31) turns into f31,FPR_OFF+8*(31-14)
#define REG_MEM( reg, n ) reg##_NAME(n), reg(n)

// void co_swap_asm( cothread_t new, cothread_t old )
co_swap_asm:
	// We must save current state into old, and load
	// new state from new. Might be called with old=new,
	// so we must not load a particular register before
	// having saved it. I don't think this hurts
	// performance, since there are enough instructions
	// to mix in.
	
	// Improve performance by reordering things somewhat.
	// Try to keep GPR saves/loads in block, for clarity.
	
	/*  new		old
	r3  pointer
	r4		pointer
	r5  flags	flags
	r6  CR
	r7  PC
	r8  		CR
	r9  		PC
	*/
	
	mfcr	r8
	STWD	sp, SP_OFF (r4)
	mflr	r9 
	
	  // Save GPRs
#if LIBCO_PPC32
	  STWD	REG_MEM(GPR,13) (r4)
#endif
	  STWD	REG_MEM(GPR,14) (r4)
	  STWD	REG_MEM(GPR,15) (r4)
	  STWD	REG_MEM(GPR,16) (r4)
	  STWD	REG_MEM(GPR,17) (r4)
	  STWD	REG_MEM(GPR,18) (r4)
	  STWD	REG_MEM(GPR,19) (r4)
	  STWD	REG_MEM(GPR,20) (r4)
	  STWD	REG_MEM(GPR,21) (r4)
	  STWD	REG_MEM(GPR,22) (r4)
	  STWD	REG_MEM(GPR,23) (r4)
	  STWD	REG_MEM(GPR,24) (r4)
	  STWD	REG_MEM(GPR,25) (r4)
	  STWD	REG_MEM(GPR,26) (r4)
	  STWD	REG_MEM(GPR,27) (r4)
	  STWD	REG_MEM(GPR,28) (r4)
	  STWD	REG_MEM(GPR,29) (r4)
	  STWD	REG_MEM(GPR,30) (r4)
	  STWD	REG_MEM(GPR,31) (r4)
	
	STWD	r9, PC_OFF (r4)
	LWZD	r7, PC_OFF (r3)
	LWZD	sp, SP_OFF (r3)
	bl		1f	// Crash if entry function returns
	trap
1:	stw	r8, CR_OFF (r4)
	lwz	r6, CR_OFF (r3)
	mtctr	r7

	  // Load GPRs
#if LIBCO_PPC32
	  LWZD	REG_MEM(GPR,13) (r3)
#endif
	  LWZD	REG_MEM(GPR,14) (r3)
	  LWZD	REG_MEM(GPR,15) (r3)
	  LWZD	REG_MEM(GPR,16) (r3)
	  LWZD	REG_MEM(GPR,17) (r3)
	  LWZD	REG_MEM(GPR,18) (r3)
	  LWZD	REG_MEM(GPR,19) (r3)
	  LWZD	REG_MEM(GPR,20) (r3)
	  LWZD	REG_MEM(GPR,21) (r3)
	  LWZD	REG_MEM(GPR,22) (r3)
	  LWZD	REG_MEM(GPR,23) (r3)
	  LWZD	REG_MEM(GPR,24) (r3)
	  LWZD	REG_MEM(GPR,25) (r3)
	  LWZD	REG_MEM(GPR,26) (r3)
	  LWZD	REG_MEM(GPR,27) (r3)
	  LWZD	REG_MEM(GPR,28) (r3)
	  LWZD	REG_MEM(GPR,29) (r3)
	  LWZD	REG_MEM(GPR,30) (r3)
	  LWZD	REG_MEM(GPR,31) (r3)
	
	mtcr	r6

#ifndef LIBCO_PPC_NOFP
	// Save FPRs
	stfd	REG_MEM(FPR,14) (r4)
	stfd	REG_MEM(FPR,15) (r4)
	stfd	REG_MEM(FPR,16) (r4)
	stfd	REG_MEM(FPR,17) (r4)
	stfd	REG_MEM(FPR,18) (r4)
	stfd	REG_MEM(FPR,19) (r4)
	stfd	REG_MEM(FPR,20) (r4)
	stfd	REG_MEM(FPR,21) (r4)
	stfd	REG_MEM(FPR,22) (r4)
	stfd	REG_MEM(FPR,23) (r4)
	stfd	REG_MEM(FPR,24) (r4)
	stfd	REG_MEM(FPR,25) (r4)
	stfd	REG_MEM(FPR,26) (r4)
	stfd	REG_MEM(FPR,27) (r4)
	stfd	REG_MEM(FPR,28) (r4)
	stfd	REG_MEM(FPR,29) (r4)
	stfd	REG_MEM(FPR,30) (r4)
	stfd	REG_MEM(FPR,31) (r4)
	
	// Load FPRs
	lfd	REG_MEM(FPR,14) (r3)
	lfd	REG_MEM(FPR,15) (r3)
	lfd	REG_MEM(FPR,16) (r3)
	lfd	REG_MEM(FPR,17) (r3)
	lfd	REG_MEM(FPR,18) (r3)
	lfd	REG_MEM(FPR,19) (r3)
	lfd	REG_MEM(FPR,20) (r3)
	lfd	REG_MEM(FPR,21) (r3)
	lfd	REG_MEM(FPR,22) (r3)
	lfd	REG_MEM(FPR,23) (r3)
	lfd	REG_MEM(FPR,24) (r3)
	lfd	REG_MEM(FPR,25) (r3)
	lfd	REG_MEM(FPR,26) (r3)
	lfd	REG_MEM(FPR,27) (r3)
	lfd	REG_MEM(FPR,28) (r3)
	lfd	REG_MEM(FPR,29) (r3)
	lfd	REG_MEM(FPR,30) (r3)
	lfd	REG_MEM(FPR,31) (r3)
#endif

#ifdef __ALTIVEC__
	// Performance testing showed that conditionally saving/restoring
	// individual registers was worse than just doing them all.
	
	// Save VRSAVE
	mfspr	r5, 256		// get VRSAVE
	addi	r8, r4,VR_OFF
	addi	r9, r4,VR_OFF + 16
	andi.	r0, r5,0x0FFF	// see whether anything to save
	stw	r5, VRSAVE_OFF(r4)
	beq		2f		// skip if nothing to save

	// Save VRs
	
	// we use two pointers to avoid stalls
	stvx	v20,0,r8
	addi	r8,r8,32
	
	stvx	v21,0,r9
	addi	r9,r9,32
	
	stvx	v22,0,r8
	addi	r8,r8,32
	
	stvx	v23,0,r9
	addi	r9,r9,32
	
	stvx	v24,0,r8
	addi	r8,r8,32
	
	stvx	v25,0,r9
	addi	r9,r9,32
	
	stvx	v26,0,r8
	addi	r8,r8,32
	
	stvx	v27,0,r9
	addi	r9,r9,32
	
	stvx	v28,0,r8
	addi	r8,r8,32
	
	stvx	v29,0,r9
	addi	r9,r9,32
	
	stvx	v30,0,r8
	
	stvx	v31,0,r9

2:
	// Load VRSAVE
	lwz	r5, VRSAVE_OFF(r3)
	addi	r8, r3,VR_OFF
	addi	r9, r3,VR_OFF + 16
	andi.	r0, r5,0x0FFF
	mtspr	256,r5		// set VRSAVE
	beqctr			// return if nothing to restore

	// Load VRs
	lvx	v20,0,r8
	addi	r8,r8,32
	
	lvx	v21,0,r9
	addi	r9,r9,32
	
	lvx	v22,0,r8
	addi	r8,r8,32
	
	lvx	v23,0,r9
	addi	r9,r9,32
	
	lvx	v24,0,r8
	addi	r8,r8,32
	
	lvx	v25,0,r9
	addi	r9,r9,32
	
	lvx	v26,0,r8
	addi	r8,r8,32
	
	lvx	v27,0,r9
	addi	r9,r9,32
	
	lvx	v28,0,r8
	addi	r8,r8,32
	
	lvx	v29,0,r9
	addi	r9,r9,32
	
	lvx	v30,0,r8
	
	lvx	v31,0,r9
#endif

	bctr
