.section ".text"
.align 5
.globl save_ppc_regs
.globl load_ppc_regs

.type save_ppc_regs, @function
.type load_ppc_regs, @function

#if _ARCH_PPC64 || __PPC64__ || __ppc64__ || __powerpc64__
	#define STWD		std
	#define LWZD		ld
	#define GPR_EXTRA	0
#else
	#define STWD		stw
	#define LWZD		lwz
	#define GPR_EXTRA	4
#endif

#define VR_OFF 	0
#define FPR_OFF 512
#define GPR_OFF	768
#define SPR_OFF	1024

#define FPR(n)	FPR_OFF             + 8*n
#define GPR(n)	GPR_OFF + GPR_EXTRA + 8*n
#define SPR(n)	SPR_OFF             + 4*n

#define FPR_NAME(n)	f##n
#define GPR_NAME(n)	r##n

#define REG_MEM( reg, n ) reg##_NAME(n), reg(n)

save_ppc_regs:
	mfcr	r0
	stw	r0, SPR(0) (r3)
	
	mfspr	r0,256	/* VRSAVE */
	stw	r0, SPR(1) (r3)
	
	mfctr	r0
	stw	r0, SPR(2) (r3)
	
	mfxer	r0
	stw	r0, SPR(3) (r3)
	
	STWD	REG_MEM(GPR, 0) (r3)
	STWD	REG_MEM(GPR, 1) (r3)
	STWD	REG_MEM(GPR, 2) (r3)
	STWD	REG_MEM(GPR, 3) (r3)
	STWD	REG_MEM(GPR, 4) (r3)
	STWD	REG_MEM(GPR, 5) (r3)
	STWD	REG_MEM(GPR, 6) (r3)
	STWD	REG_MEM(GPR, 7) (r3)
	STWD	REG_MEM(GPR, 8) (r3)
	STWD	REG_MEM(GPR, 9) (r3)
	STWD	REG_MEM(GPR,10) (r3)
	STWD	REG_MEM(GPR,11) (r3)
	STWD	REG_MEM(GPR,12) (r3)
	STWD	REG_MEM(GPR,13) (r3)
	STWD	REG_MEM(GPR,14) (r3)
	STWD	REG_MEM(GPR,15) (r3)
	STWD	REG_MEM(GPR,16) (r3)
	STWD	REG_MEM(GPR,17) (r3)
	STWD	REG_MEM(GPR,18) (r3)
	STWD	REG_MEM(GPR,19) (r3)
	STWD	REG_MEM(GPR,20) (r3)
	STWD	REG_MEM(GPR,21) (r3)
	STWD	REG_MEM(GPR,22) (r3)
	STWD	REG_MEM(GPR,23) (r3)
	STWD	REG_MEM(GPR,24) (r3)
	STWD	REG_MEM(GPR,25) (r3)
	STWD	REG_MEM(GPR,26) (r3)
	STWD	REG_MEM(GPR,27) (r3)
	STWD	REG_MEM(GPR,28) (r3)
	STWD	REG_MEM(GPR,29) (r3)
	STWD	REG_MEM(GPR,30) (r3)
	STWD	REG_MEM(GPR,31) (r3)
	
	stfd	REG_MEM(FPR, 0) (r3)
	stfd	REG_MEM(FPR, 1) (r3)
	stfd	REG_MEM(FPR, 2) (r3)
	stfd	REG_MEM(FPR, 3) (r3)
	stfd	REG_MEM(FPR, 4) (r3)
	stfd	REG_MEM(FPR, 5) (r3)
	stfd	REG_MEM(FPR, 6) (r3)
	stfd	REG_MEM(FPR, 7) (r3)
	stfd	REG_MEM(FPR, 8) (r3)
	stfd	REG_MEM(FPR, 9) (r3)
	stfd	REG_MEM(FPR,10) (r3)
	stfd	REG_MEM(FPR,11) (r3)
	stfd	REG_MEM(FPR,12) (r3)
	stfd	REG_MEM(FPR,13) (r3)
	stfd	REG_MEM(FPR,14) (r3)
	stfd	REG_MEM(FPR,15) (r3)
	stfd	REG_MEM(FPR,16) (r3)
	stfd	REG_MEM(FPR,17) (r3)
	stfd	REG_MEM(FPR,18) (r3)
	stfd	REG_MEM(FPR,19) (r3)
	stfd	REG_MEM(FPR,20) (r3)
	stfd	REG_MEM(FPR,21) (r3)
	stfd	REG_MEM(FPR,22) (r3)
	stfd	REG_MEM(FPR,23) (r3)
	stfd	REG_MEM(FPR,24) (r3)
	stfd	REG_MEM(FPR,25) (r3)
	stfd	REG_MEM(FPR,26) (r3)
	stfd	REG_MEM(FPR,27) (r3)
	stfd	REG_MEM(FPR,28) (r3)
	stfd	REG_MEM(FPR,29) (r3)
	stfd	REG_MEM(FPR,30) (r3)
	stfd	REG_MEM(FPR,31) (r3)
	
	mffs	f0
	stfd	f0, SPR(4) (r3)
	
	addi	r5, r3, VR_OFF
	stvx	v0, 0, r5
	addi	r5, r5, 16
	stvx	v1, 0, r5
	addi	r5, r5, 16
	stvx	v2, 0, r5
	addi	r5, r5, 16
	stvx	v3, 0, r5
	addi	r5, r5, 16
	stvx	v4, 0, r5
	addi	r5, r5, 16
	stvx	v5, 0, r5
	addi	r5, r5, 16
	stvx	v6, 0, r5
	addi	r5, r5, 16
	stvx	v7, 0, r5
	addi	r5, r5, 16
	stvx	v8, 0, r5
	addi	r5, r5, 16
	stvx	v9, 0, r5
	addi	r5, r5, 16
	stvx	v10, 0, r5
	addi	r5, r5, 16
	stvx	v11, 0, r5
	addi	r5, r5, 16
	stvx	v12, 0, r5
	addi	r5, r5, 16
	stvx	v13, 0, r5
	addi	r5, r5, 16
	stvx	v14, 0, r5
	addi	r5, r5, 16
	stvx	v15, 0, r5
	addi	r5, r5, 16
	stvx	v16, 0, r5
	addi	r5, r5, 16
	stvx	v17, 0, r5
	addi	r5, r5, 16
	stvx	v18, 0, r5
	addi	r5, r5, 16
	stvx	v19, 0, r5
	addi	r5, r5, 16
	stvx	v20, 0, r5
	addi	r5, r5, 16
	stvx	v21, 0, r5
	addi	r5, r5, 16
	stvx	v22, 0, r5
	addi	r5, r5, 16
	stvx	v23, 0, r5
	addi	r5, r5, 16
	stvx	v24, 0, r5
	addi	r5, r5, 16
	stvx	v25, 0, r5
	addi	r5, r5, 16
	stvx	v26, 0, r5
	addi	r5, r5, 16
	stvx	v27, 0, r5
	addi	r5, r5, 16
	stvx	v28, 0, r5
	addi	r5, r5, 16
	stvx	v29, 0, r5
	addi	r5, r5, 16
	stvx	v30, 0, r5
	addi	r5, r5, 16
	stvx	v31, 0, r5

	blr

load_ppc_regs:
	lwz	r0, SPR(0) (r3)
	mtcr	r0
	
	lwz	r0, SPR(1) (r3)
	mtspr	256, r0

	lwz	r0, SPR(2) (r3)
	mtctr	r0
	
	lwz	r0, SPR(3) (r3)
	mtxer	r0
	
	lfd	f0, SPR(4) (r3)
	mtfsf	255, f0
	
	LWZD	REG_MEM(GPR, 0) (r3)

	

	LWZD	REG_MEM(GPR, 4) (r3)
	LWZD	REG_MEM(GPR, 5) (r3)
	LWZD	REG_MEM(GPR, 6) (r3)
	LWZD	REG_MEM(GPR, 7) (r3)
	LWZD	REG_MEM(GPR, 8) (r3)
	LWZD	REG_MEM(GPR, 9) (r3)
	LWZD	REG_MEM(GPR,10) (r3)
	LWZD	REG_MEM(GPR,11) (r3)
	LWZD	REG_MEM(GPR,12) (r3)
	LWZD	REG_MEM(GPR,13) (r3)
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

	lfd	REG_MEM(FPR, 0) (r3)
	lfd	REG_MEM(FPR, 1) (r3)
	lfd	REG_MEM(FPR, 2) (r3)
	lfd	REG_MEM(FPR, 3) (r3)
	lfd	REG_MEM(FPR, 4) (r3)
	lfd	REG_MEM(FPR, 5) (r3)
	lfd	REG_MEM(FPR, 6) (r3)
	lfd	REG_MEM(FPR, 7) (r3)
	lfd	REG_MEM(FPR, 8) (r3)
	lfd	REG_MEM(FPR, 9) (r3)
	lfd	REG_MEM(FPR,10) (r3)
	lfd	REG_MEM(FPR,11) (r3)
	lfd	REG_MEM(FPR,12) (r3)
	lfd	REG_MEM(FPR,13) (r3)
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
	
	addi	r5, r3, VR_OFF
	lvx	v0, 0, r5
	addi	r5, r5, 16
	lvx	v1, 0, r5
	addi	r5, r5, 16
	lvx	v2, 0, r5
	addi	r5, r5, 16
	lvx	v3, 0, r5
	addi	r5, r5, 16
	lvx	v4, 0, r5
	addi	r5, r5, 16
	lvx	v5, 0, r5
	addi	r5, r5, 16
	lvx	v6, 0, r5
	addi	r5, r5, 16
	lvx	v7, 0, r5
	addi	r5, r5, 16
	lvx	v8, 0, r5
	addi	r5, r5, 16
	lvx	v9, 0, r5
	addi	r5, r5, 16
	lvx	v10, 0, r5
	addi	r5, r5, 16
	lvx	v11, 0, r5
	addi	r5, r5, 16
	lvx	v12, 0, r5
	addi	r5, r5, 16
	lvx	v13, 0, r5
	addi	r5, r5, 16
	lvx	v14, 0, r5
	addi	r5, r5, 16
	lvx	v15, 0, r5
	addi	r5, r5, 16
	lvx	v16, 0, r5
	addi	r5, r5, 16
	lvx	v17, 0, r5
	addi	r5, r5, 16
	lvx	v18, 0, r5
	addi	r5, r5, 16
	lvx	v19, 0, r5
	addi	r5, r5, 16
	lvx	v20, 0, r5
	addi	r5, r5, 16
	lvx	v21, 0, r5
	addi	r5, r5, 16
	lvx	v22, 0, r5
	addi	r5, r5, 16
	lvx	v23, 0, r5
	addi	r5, r5, 16
	lvx	v24, 0, r5
	addi	r5, r5, 16
	lvx	v25, 0, r5
	addi	r5, r5, 16
	lvx	v26, 0, r5
	addi	r5, r5, 16
	lvx	v27, 0, r5
	addi	r5, r5, 16
	lvx	v28, 0, r5
	addi	r5, r5, 16
	lvx	v29, 0, r5
	addi	r5, r5, 16
	lvx	v30, 0, r5
	addi	r5, r5, 16
	lvx	v31, 0, r5

	blr
