/*
  libco.ppc (2010-10-17)
  author: blargg
  license: public domain
*/

/* PowerPC 32/64 using embedded or external asm, with optional
floating-point and AltiVec save/restore */

#define LIBCO_C
#include "libco.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define LIBCO_MPROTECT (__unix__ && !LIBCO_PPC_ASM)

#if LIBCO_MPROTECT
	#include <unistd.h>
	#include <sys/mman.h>
#endif

/* State format (offsets in 32-bit words)

+0	Pointer to swap code
	Rest of function descriptor for entry function
+8	PC
+10	SP
	Special regs
	GPRs
	FPRs
	VRs
	stack
*/

enum { state_size  = 1024 };
enum { above_stack = 2048 };
enum { stack_align = 256  };

static thread_local cothread_t co_active_handle = 0;

/**** Determine environment ****/

#define LIBCO_PPC64 (_ARCH_PPC64 || __PPC64__ || __ppc64__ || __powerpc64__)

/* Whether function calls are indirect through a descriptor,
or are directly to function */
#ifndef LIBCO_PPCDESC
	#if !_CALL_SYSV && (_CALL_AIX || _CALL_AIXDESC || LIBCO_PPC64)
		#define LIBCO_PPCDESC 1
	#endif
#endif

#ifdef LIBCO_PPC_ASM

	#ifdef __cplusplus
		extern "C"
	#endif
	
	/* Swap code is in ppc.S */
	void co_swap_asm( cothread_t, cothread_t );
	#define CO_SWAP_ASM( x, y ) co_swap_asm( x, y )

#else

/* Swap code is here in array. Please leave dieassembly comments,
as they make it easy to see what it does, and reorder instructions
if one wants to see whether that improves performance. */
static const uint32_t libco_ppc_code [] = {
#if LIBCO_PPC64
    0x7d000026, /* mfcr    r8 */
    0xf8240028, /* std     r1,40(r4) */
    0x7d2802a6, /* mflr    r9 */
    0xf9c40048, /* std     r14,72(r4) */
    0xf9e40050, /* std     r15,80(r4) */
    0xfa040058, /* std     r16,88(r4) */
    0xfa240060, /* std     r17,96(r4) */
    0xfa440068, /* std     r18,104(r4) */
    0xfa640070, /* std     r19,112(r4) */
    0xfa840078, /* std     r20,120(r4) */
    0xfaa40080, /* std     r21,128(r4) */
    0xfac40088, /* std     r22,136(r4) */
    0xfae40090, /* std     r23,144(r4) */
    0xfb040098, /* std     r24,152(r4) */
    0xfb2400a0, /* std     r25,160(r4) */
    0xfb4400a8, /* std     r26,168(r4) */
    0xfb6400b0, /* std     r27,176(r4) */
    0xfb8400b8, /* std     r28,184(r4) */
    0xfba400c0, /* std     r29,192(r4) */
    0xfbc400c8, /* std     r30,200(r4) */
    0xfbe400d0, /* std     r31,208(r4) */
    0xf9240020, /* std     r9,32(r4) */
    0xe8e30020, /* ld      r7,32(r3) */
    0xe8230028, /* ld      r1,40(r3) */
    0x48000009, /* bl      1 */
	0x7fe00008, /* trap */
    0x91040030,/*1:stw     r8,48(r4) */
    0x80c30030, /* lwz     r6,48(r3) */
    0x7ce903a6, /* mtctr   r7 */
    0xe9c30048, /* ld      r14,72(r3) */
    0xe9e30050, /* ld      r15,80(r3) */
    0xea030058, /* ld      r16,88(r3) */
    0xea230060, /* ld      r17,96(r3) */
    0xea430068, /* ld      r18,104(r3) */
    0xea630070, /* ld      r19,112(r3) */
    0xea830078, /* ld      r20,120(r3) */
    0xeaa30080, /* ld      r21,128(r3) */
    0xeac30088, /* ld      r22,136(r3) */
    0xeae30090, /* ld      r23,144(r3) */
    0xeb030098, /* ld      r24,152(r3) */
    0xeb2300a0, /* ld      r25,160(r3) */
    0xeb4300a8, /* ld      r26,168(r3) */
    0xeb6300b0, /* ld      r27,176(r3) */
    0xeb8300b8, /* ld      r28,184(r3) */
    0xeba300c0, /* ld      r29,192(r3) */
    0xebc300c8, /* ld      r30,200(r3) */
    0xebe300d0, /* ld      r31,208(r3) */
    0x7ccff120, /* mtcr    r6 */
#else
	0x7d000026, /* mfcr    r8 */
	0x90240028, /* stw     r1,40(r4) */
	0x7d2802a6, /* mflr    r9 */
	0x91a4003c, /* stw     r13,60(r4) */
	0x91c40040, /* stw     r14,64(r4) */
	0x91e40044, /* stw     r15,68(r4) */
	0x92040048, /* stw     r16,72(r4) */
	0x9224004c, /* stw     r17,76(r4) */
	0x92440050, /* stw     r18,80(r4) */
	0x92640054, /* stw     r19,84(r4) */
	0x92840058, /* stw     r20,88(r4) */
	0x92a4005c, /* stw     r21,92(r4) */
	0x92c40060, /* stw     r22,96(r4) */
	0x92e40064, /* stw     r23,100(r4) */
	0x93040068, /* stw     r24,104(r4) */
	0x9324006c, /* stw     r25,108(r4) */
	0x93440070, /* stw     r26,112(r4) */
	0x93640074, /* stw     r27,116(r4) */
	0x93840078, /* stw     r28,120(r4) */
	0x93a4007c, /* stw     r29,124(r4) */
	0x93c40080, /* stw     r30,128(r4) */
	0x93e40084, /* stw     r31,132(r4) */
	0x91240020, /* stw     r9,32(r4) */
	0x80e30020, /* lwz     r7,32(r3) */
	0x80230028, /* lwz     r1,40(r3) */
	0x48000009, /* bl      1 */
	0x7fe00008, /* trap */
	0x91040030,/*1:stw     r8,48(r4) */
	0x80c30030, /* lwz     r6,48(r3) */
	0x7ce903a6, /* mtctr   r7 */
	0x81a3003c, /* lwz     r13,60(r3) */
	0x81c30040, /* lwz     r14,64(r3) */
	0x81e30044, /* lwz     r15,68(r3) */
	0x82030048, /* lwz     r16,72(r3) */
	0x8223004c, /* lwz     r17,76(r3) */
	0x82430050, /* lwz     r18,80(r3) */
	0x82630054, /* lwz     r19,84(r3) */
	0x82830058, /* lwz     r20,88(r3) */
	0x82a3005c, /* lwz     r21,92(r3) */
	0x82c30060, /* lwz     r22,96(r3) */
	0x82e30064, /* lwz     r23,100(r3) */
	0x83030068, /* lwz     r24,104(r3) */
	0x8323006c, /* lwz     r25,108(r3) */
	0x83430070, /* lwz     r26,112(r3) */
	0x83630074, /* lwz     r27,116(r3) */
	0x83830078, /* lwz     r28,120(r3) */
	0x83a3007c, /* lwz     r29,124(r3) */
	0x83c30080, /* lwz     r30,128(r3) */
	0x83e30084, /* lwz     r31,132(r3) */
	0x7ccff120, /* mtcr    r6 */
#endif

#ifndef LIBCO_PPC_NOFP
	0xd9c400e0, /* stfd    f14,224(r4) */
	0xd9e400e8, /* stfd    f15,232(r4) */
	0xda0400f0, /* stfd    f16,240(r4) */
	0xda2400f8, /* stfd    f17,248(r4) */
	0xda440100, /* stfd    f18,256(r4) */
	0xda640108, /* stfd    f19,264(r4) */
	0xda840110, /* stfd    f20,272(r4) */
	0xdaa40118, /* stfd    f21,280(r4) */
	0xdac40120, /* stfd    f22,288(r4) */
	0xdae40128, /* stfd    f23,296(r4) */
	0xdb040130, /* stfd    f24,304(r4) */
	0xdb240138, /* stfd    f25,312(r4) */
	0xdb440140, /* stfd    f26,320(r4) */
	0xdb640148, /* stfd    f27,328(r4) */
	0xdb840150, /* stfd    f28,336(r4) */
	0xdba40158, /* stfd    f29,344(r4) */
	0xdbc40160, /* stfd    f30,352(r4) */
	0xdbe40168, /* stfd    f31,360(r4) */
	0xc9c300e0, /* lfd     f14,224(r3) */
	0xc9e300e8, /* lfd     f15,232(r3) */
	0xca0300f0, /* lfd     f16,240(r3) */
	0xca2300f8, /* lfd     f17,248(r3) */
	0xca430100, /* lfd     f18,256(r3) */
	0xca630108, /* lfd     f19,264(r3) */
	0xca830110, /* lfd     f20,272(r3) */
	0xcaa30118, /* lfd     f21,280(r3) */
	0xcac30120, /* lfd     f22,288(r3) */
	0xcae30128, /* lfd     f23,296(r3) */
	0xcb030130, /* lfd     f24,304(r3) */
	0xcb230138, /* lfd     f25,312(r3) */
	0xcb430140, /* lfd     f26,320(r3) */
	0xcb630148, /* lfd     f27,328(r3) */
	0xcb830150, /* lfd     f28,336(r3) */
	0xcba30158, /* lfd     f29,344(r3) */
	0xcbc30160, /* lfd     f30,352(r3) */
	0xcbe30168, /* lfd     f31,360(r3) */
#endif

#ifdef __ALTIVEC__
	0x7ca042a6, /* mfvrsave r5 */
	0x39040180, /* addi    r8,r4,384 */
	0x39240190, /* addi    r9,r4,400 */
	0x70a00fff, /* andi.   r0,r5,4095 */
	0x90a40034, /* stw     r5,52(r4) */
	0x4182005c, /* beq-    2 */
	0x7e8041ce, /* stvx    v20,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7ea049ce, /* stvx    v21,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7ec041ce, /* stvx    v22,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7ee049ce, /* stvx    v23,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7f0041ce, /* stvx    v24,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7f2049ce, /* stvx    v25,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7f4041ce, /* stvx    v26,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7f6049ce, /* stvx    v27,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7f8041ce, /* stvx    v28,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7fa049ce, /* stvx    v29,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7fc041ce, /* stvx    v30,r0,r8 */
	0x7fe049ce, /* stvx    v31,r0,r9 */
	0x80a30034,/*2:lwz     r5,52(r3) */
	0x39030180, /* addi    r8,r3,384 */
	0x39230190, /* addi    r9,r3,400 */
	0x70a00fff, /* andi.   r0,r5,4095 */
	0x7ca043a6, /* mtvrsave r5 */
	0x4d820420, /* beqctr   */
	0x7e8040ce, /* lvx     v20,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7ea048ce, /* lvx     v21,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7ec040ce, /* lvx     v22,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7ee048ce, /* lvx     v23,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7f0040ce, /* lvx     v24,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7f2048ce, /* lvx     v25,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7f4040ce, /* lvx     v26,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7f6048ce, /* lvx     v27,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7f8040ce, /* lvx     v28,r0,r8 */
	0x39080020, /* addi    r8,r8,32 */
	0x7fa048ce, /* lvx     v29,r0,r9 */
	0x39290020, /* addi    r9,r9,32 */
	0x7fc040ce, /* lvx     v30,r0,r8 */
	0x7fe048ce, /* lvx     v31,r0,r9 */
#endif

	0x4e800420, /* bctr */
};

	#if LIBCO_PPCDESC
		/* Function call goes through indirect descriptor */
		#define CO_SWAP_ASM( x, y ) \
			((void (*)( cothread_t, cothread_t )) (uintptr_t) x)( x, y )
	#else
		/* Function call goes directly to code */
		#define CO_SWAP_ASM( x, y ) \
			((void (*)( cothread_t, cothread_t )) (uintptr_t) libco_ppc_code)( x, y )
	#endif

#endif

static uint32_t* co_create_( unsigned size, uintptr_t entry )
{
	uint32_t* t = (uint32_t*) malloc( size );
	
	(void) entry;
	
	#if LIBCO_PPCDESC
		if ( t )
		{
			/* Copy entry's descriptor */
			memcpy( t, (void*) entry, sizeof (void*) * 3 );
			
			/* Set function pointer to swap routine */
			#ifdef LIBCO_PPC_ASM
				*(const void**) t = *(void**) &co_swap_asm;
			#else
				*(const void**) t = libco_ppc_code;
			#endif
		}
	#endif
	
	return t;
}

cothread_t co_create( unsigned int size, void (*entry_)( void ) )
{
	uintptr_t entry = (uintptr_t) entry_;
	uint32_t* t = NULL;
	
	/* Be sure main thread was successfully allocated */
	if ( co_active() )
	{
		size += state_size + above_stack + stack_align;
		t = co_create_( size, entry );
	}
	
	if ( t )
	{
		uintptr_t sp;
		int shift;
		
		/* Save current registers into new thread, so that any special ones will
		have proper values when thread is begun */
		CO_SWAP_ASM( t, t );
		
		#if LIBCO_PPCDESC
			/* Get real address */
			entry = (uintptr_t) *(void**) entry;
		#endif
		
		/* Put stack near end of block, and align */
		sp = (uintptr_t) t + size - above_stack;
		sp -= sp % stack_align;
		
		/* On PPC32, we save and restore GPRs as 32 bits. For PPC64, we
		save and restore them as 64 bits, regardless of the size the ABI
		uses. So, we manually write pointers at the proper size. We always
		save and restore at the same address, and since PPC is big-endian,
		we must put the low byte first on PPC32. */
		
		/* If uintptr_t is 32 bits, >>32 is undefined behavior, so we do two shifts
		and don't have to care how many bits uintptr_t is. */
		#if LIBCO_PPC64
			shift = 16;
		#else
			shift = 0;
		#endif
		
		/* Set up so entry will be called on next swap */
		t [8] = (uint32_t) (entry >> shift >> shift);
		t [9] = (uint32_t) entry;
		
		t [10] = (uint32_t) (sp >> shift >> shift); 
		t [11] = (uint32_t) sp;
	}
	
	return t;
}

void co_delete( cothread_t t )
{
	free( t );
}

static void co_init_( void )
{
	#if LIBCO_MPROTECT
		/* TODO: pre- and post-pad PPC code so that this doesn't make other
		data executable and writable */
		long page_size = sysconf( _SC_PAGESIZE );
		if ( page_size > 0 )
		{
			uintptr_t align = page_size;
			uintptr_t begin = (uintptr_t) libco_ppc_code;
			uintptr_t end   = begin + sizeof libco_ppc_code;
			
			/* Align beginning and end */
			end   += align - 1;
			end   -= end   % align;
			begin -= begin % align;
			
			mprotect( (void*) begin, end - begin, PROT_READ | PROT_WRITE | PROT_EXEC );
		}
	#endif
    
	co_active_handle = co_create_( state_size, (uintptr_t) &co_switch );
}

cothread_t co_active()
{
	if ( !co_active_handle )
		co_init_();
	
	return co_active_handle;
}

void co_switch( cothread_t t )
{
	cothread_t old = co_active_handle;
	co_active_handle = t;
	
	CO_SWAP_ASM( t, old );
}
