/*
 * DSP utils
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef CPUTEST_H
#define CPUTEST_H

#define MM_C       0x0000 //plain C (default)
#define MM_IA32ASM 0x0001 //32-bit assembler optimized code (non-MMX)
#define MM_IA64ASM 0x0040 //64-bit assembler optimized code (non-MMX)
#define MM_MMX     0x0002 //standard MMX
#define MM_MMXEXT  0x0004 //SSE integer functions or AMD MMX ext
#define MM_3DNOW   0x0008 //AMD 3DNOW
#define MM_SSE     0x0010 //SSE functions
#define MM_SSE2    0x0020 //PIV SSE2 functions
#define MM_3DNOWEXT 0x0040 //AMD 3DNow! ext.
#define MM_SSE3    0x0080 //Prescot SSE3


#define MM_ALTIVEC	0x1000

int ac_mmflag();

#ifdef HAVE_MMX


static inline void emms(void)
{
    __asm __volatile ("emms;":::"memory");
}

#define emms_c() \
{\
    if (mm_flags & MM_MMX)\
        emms();\
}



#define __align8 __attribute__ ((aligned (8)))

#elif defined(ARCH_ARMV4L)

#define emms_c()

/* This is to use 4 bytes read to the IDCT pointers for some 'zero'
   line ptimizations */
#define __align8 __attribute__ ((aligned (4)))

#elif defined(HAVE_MLIB)
 
#define emms_c()

/* SPARC/VIS IDCT needs 8-byte aligned DCT blocks */
#define __align8 __attribute__ ((aligned (8)))

void dsputil_init_mlib(void);   

#elif defined(ARCH_ALPHA)

#define emms_c()
#define __align8 __attribute__ ((aligned (8)))

#elif defined(ARCH_POWERPC)

#define emms_c()
#define __align8 __attribute__ ((aligned (16)))

#elif defined(HAVE_MMI)

#define emms_c()

#define __align8 __attribute__ ((aligned (16)))

void dsputil_init_mmi(void);   

#else

#define emms_c()

#define __align8

#endif
              
#endif
