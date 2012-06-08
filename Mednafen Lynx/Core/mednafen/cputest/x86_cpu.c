/*
 * CPU detection code, extracted from mmx.h
 * (c)1997-99 by H. Dietz and R. Fisher
 * Converted to C and improved by Fabrice Bellard.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>
#include "x86_cpu.h"
#include "cputest.h"

/* ebx saving is necessary for PIC. gcc seems unable to see it alone */
#define cpuid(index,eax,ebx,ecx,edx)\
    __asm__ volatile\
        ("mov %%"REG_b", %%"REG_S"\n\t"\
         "cpuid\n\t"\
         "xchg %%"REG_b", %%"REG_S\
         : "=a" (eax), "=S" (ebx),\
           "=c" (ecx), "=d" (edx)\
         : "0" (index));

#define xgetbv(index,eax,edx)                                   \
    __asm__ (".byte 0x0f, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c" (index))

/* Function to test if multimedia instructions are supported...  */
int ff_get_cpu_flags_x86(void)
{
    int rval = 0;
    int eax, ebx, ecx, edx;
    int max_std_level, max_ext_level, std_caps=0, ext_caps=0;
    int family=0, model=0;
    union { int i[3]; char c[12]; } vendor;

#if ARCH_X86_32
    x86_reg a, c;
    __asm__ volatile (
        /* See if CPUID instruction is supported ... */
        /* ... Get copies of EFLAGS into eax and ecx */
        "pushfl\n\t"
        "pop %0\n\t"
        "mov %0, %1\n\t"

        /* ... Toggle the ID bit in one copy and store */
        /*     to the EFLAGS reg */
        "xor $0x200000, %0\n\t"
        "push %0\n\t"
        "popfl\n\t"

        /* ... Get the (hopefully modified) EFLAGS */
        "pushfl\n\t"
        "pop %0\n\t"
        : "=a" (a), "=c" (c)
        :
        : "cc"
        );

    if (a == c)
        return 0; /* CPUID not supported */
#endif

    cpuid(0, max_std_level, vendor.i[0], vendor.i[2], vendor.i[1]);

    if(max_std_level >= 1){
        cpuid(1, eax, ebx, ecx, std_caps);
        family = ((eax>>8)&0xf) + ((eax>>20)&0xff);
        model  = ((eax>>4)&0xf) + ((eax>>12)&0xf0);
        if (std_caps & (1<<23))
            rval |= CPUTEST_FLAG_MMX;
        if (std_caps & (1<<25))
            rval |= CPUTEST_FLAG_MMX2
//#if HAVE_SSE
                  | CPUTEST_FLAG_SSE;
        if (std_caps & (1<<26))
            rval |= CPUTEST_FLAG_SSE2;
        if (ecx & 1)
            rval |= CPUTEST_FLAG_SSE3;
        if (ecx & 0x00000200 )
            rval |= CPUTEST_FLAG_SSSE3;
        if (ecx & 0x00080000 )
            rval |= CPUTEST_FLAG_SSE4;
        if (ecx & 0x00100000 )
            rval |= CPUTEST_FLAG_SSE42;
//#if HAVE_AVX
        /* Check OXSAVE and AVX bits */
        if ((ecx & 0x18000000) == 0x18000000) {
            /* Check for OS support */
            xgetbv(0, eax, edx);
            if ((eax & 0x6) == 0x6)
                rval |= CPUTEST_FLAG_AVX;
        }
//#endif
//#endif
                  ;
    }

    cpuid(0x80000000, max_ext_level, ebx, ecx, edx);

    if(max_ext_level >= 0x80000001){
        cpuid(0x80000001, eax, ebx, ecx, ext_caps);
        if (ext_caps & (1<<31))
            rval |= CPUTEST_FLAG_3DNOW;
        if (ext_caps & (1<<30))
            rval |= CPUTEST_FLAG_3DNOWEXT;
        if (ext_caps & (1<<23))
            rval |= CPUTEST_FLAG_MMX;
        if (ext_caps & (1<<22))
            rval |= CPUTEST_FLAG_MMX2;

        /* Allow for selectively disabling SSE2 functions on AMD processors
           with SSE2 support but not SSE4a. This includes Athlon64, some
           Opteron, and some Sempron processors. MMX, SSE, or 3DNow! are faster
           than SSE2 often enough to utilize this special-case flag.
           CPUTEST_FLAG_SSE2 and CPUTEST_FLAG_SSE2SLOW are both set in this case
           so that SSE2 is used unless explicitly disabled by checking
           CPUTEST_FLAG_SSE2SLOW. */
        if (!strncmp(vendor.c, "AuthenticAMD", 12) &&
            rval & CPUTEST_FLAG_SSE2 && !(ecx & 0x00000040)) {
            rval |= CPUTEST_FLAG_SSE2SLOW;
        }
    }

    if (!strncmp(vendor.c, "GenuineIntel", 12)) {
        if (family == 6 && (model == 9 || model == 13 || model == 14)) {
            /* 6/9 (pentium-m "banias"), 6/13 (pentium-m "dothan"), and 6/14 (core1 "yonah")
            * theoretically support sse2, but it's usually slower than mmx,
            * so let's just pretend they don't. CPUTEST_FLAG_SSE2 is disabled and
            * CPUTEST_FLAG_SSE2SLOW is enabled so that SSE2 is not used unless
            * explicitly enabled by checking CPUTEST_FLAG_SSE2SLOW. The same
            * situation applies for CPUTEST_FLAG_SSE3 and CPUTEST_FLAG_SSE3SLOW. */
            if (rval & CPUTEST_FLAG_SSE2) rval ^= CPUTEST_FLAG_SSE2SLOW|CPUTEST_FLAG_SSE2;
            if (rval & CPUTEST_FLAG_SSE3) rval ^= CPUTEST_FLAG_SSE3SLOW|CPUTEST_FLAG_SSE3;
        }
        /* The Atom processor has SSSE3 support, which is useful in many cases,
         * but sometimes the SSSE3 version is slower than the SSE2 equivalent
         * on the Atom, but is generally faster on other processors supporting
         * SSSE3. This flag allows for selectively disabling certain SSSE3
         * functions on the Atom. */
        if (family == 6 && model == 28)
            rval |= CPUTEST_FLAG_ATOM;
    }

    return rval;
}
