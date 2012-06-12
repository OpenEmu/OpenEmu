/*
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

#include "cputest.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static int flags, checked = 0;


void cputest_force_flags(int arg)
{
    flags   = arg;
    checked = 1;
}

int cputest_get_flags(void)
{
    if (checked)
        return flags;

//    if (ARCH_ARM) flags = ff_get_cpu_flags_arm();
#if ARCH_POWERPC
    flags = ff_get_cpu_flags_ppc();
#endif

#if ARCH_X86
    flags = ff_get_cpu_flags_x86();
#endif

    checked = 1;
    return flags;
}

#if 0
//#ifdef TEST

#undef printf
#include <stdio.h>

int main(void)
{
    int cpu_flags = av_get_cpu_flags();

    printf("cpu_flags = 0x%08X\n", cpu_flags);
    printf("cpu_flags = %s%s%s%s%s%s%s%s%s%s%s%s%s\n",
#if   ARCH_ARM
           cpu_flags & CPUTEST_FLAG_IWMMXT   ? "IWMMXT "     : "",
#elif ARCH_POWERPC
           cpu_flags & CPUTEST_FLAG_ALTIVEC  ? "ALTIVEC "    : "",
#elif ARCH_X86
           cpu_flags & CPUTEST_FLAG_MMX      ? "MMX "        : "",
           cpu_flags & CPUTEST_FLAG_MMX2     ? "MMX2 "       : "",
           cpu_flags & CPUTEST_FLAG_SSE      ? "SSE "        : "",
           cpu_flags & CPUTEST_FLAG_SSE2     ? "SSE2 "       : "",
           cpu_flags & CPUTEST_FLAG_SSE2SLOW ? "SSE2(slow) " : "",
           cpu_flags & CPUTEST_FLAG_SSE3     ? "SSE3 "       : "",
           cpu_flags & CPUTEST_FLAG_SSE3SLOW ? "SSE3(slow) " : "",
           cpu_flags & CPUTEST_FLAG_SSSE3    ? "SSSE3 "      : "",
           cpu_flags & CPUTEST_FLAG_ATOM     ? "Atom "       : "",
           cpu_flags & CPUTEST_FLAG_SSE4     ? "SSE4.1 "     : "",
           cpu_flags & CPUTEST_FLAG_SSE42    ? "SSE4.2 "     : "",
           cpu_flags & CPUTEST_FLAG_AVX      ? "AVX "        : "",
           cpu_flags & CPUTEST_FLAG_3DNOW    ? "3DNow "      : "",
           cpu_flags & CPUTEST_FLAG_3DNOWEXT ? "3DNowExt "   : "");
#endif
    return 0;
}

#endif
