/***************************************************************************
 * Gens: CPU Flags.                                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "cpuflags.h"

// IA32 CPU flags
// Intel: http://download.intel.com/design/processor/applnots/24161832.pdf
// AMD: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf

// CPUID function 1: Family & Features

// Flags stored in the edx register.
#define CPUFLAG_IA32_EDX_MMX		(1 << 23)
#define CPUFLAG_IA32_EDX_SSE		(1 << 25)
#define CPUFLAG_IA32_EDX_SSE2		(1 << 26)

// Flags stored in the ecx register.
#define CPUFLAG_IA32_ECX_SSE3		(1 << 0)
#define CPUFLAG_IA32_ECX_SSSE3		(1 << 9)
#define CPUFLAG_IA32_ECX_SSE41		(1 << 19)
#define CPUFLAG_IA32_ECX_SSE42		(1 << 20)

// CPUID function 0x80000001: Extended Family & Features

// Flags stored in the edx register.
#define CPUFLAG_IA32_EXT_EDX_MMXEXT	(1 << 22)
#define CPUFLAG_IA32_EXT_EDX_3DNOW	(1 << 31)
#define CPUFLAG_IA32_EXT_EDX_3DNOWEXT	(1 << 30)

// Flags stored in the ecx register.
#define CPUFLAG_IA32_EXT_ECX_SSE4A	(1 << 6)
#define CPUFLAG_IA32_EXT_ECX_SSE5	(1 << 11)

// CPUID functions.
#define CPUID_MAX_FUNCTIONS		((unsigned int)(0x00000000))
#define CPUID_FAMILY_FEATURES		((unsigned int)(0x00000001))
#define CPUID_MAX_EXT_FUNCTIONS		((unsigned int)(0x80000000))
#define CPUID_EXT_FAMILY_FEATURES	((unsigned int)(0x80000001))

// CPUID macro with PIC support.
// See http://gcc.gnu.org/ml/gcc-patches/2007-09/msg00324.html
#if defined(__i386__) && defined(__PIC__)
#define __cpuid(level, a, b, c, d)				\
	__asm__ (						\
		"xchgl	%%ebx, %1\n"				\
		"cpuid\n"					\
		"xchgl	%%ebx, %1\n"				\
		: "=a" (a), "=r" (b), "=c" (c), "=d" (d)	\
		: "0" (level)					\
		)
#else
#define __cpuid(level, a, b, c, d)				\
	__asm__ (						\
		"cpuid\n"					\
		: "=a" (a), "=b" (b), "=c" (c), "=d" (d)	\
		: "0" (level)					\
		)
#endif

// CPU Flags
unsigned int CPU_Flags = 0;


/**
 * getCPUFlags(): Get the CPU flags.
 * @return CPU flags.
 */
unsigned int getCPUFlags(void)
{
#if !defined(GENS_X86_ASM)
	
	// x86 asm code has been turned off.
	// Don't check for any CPU flags.
	return 0;
	
#elif defined(__i386__) || defined(__amd64__)
	// IA32/x86_64.
	
	// Check if cpuid is supported.
	unsigned int _eax, _ebx, _ecx, _edx;
	
#if defined(__i386__)
	__asm__ (
		"pushfl\n"
		"popl %%eax\n"
		"movl %%eax, %%edx\n"
		"xorl $0x200000, %%eax\n"
		"pushl %%eax\n"
		"popfl\n"
		"pushfl\n"
		"popl %%eax\n"
		"xorl %%edx, %%eax\n"
		"andl $0x200000, %%eax"
		:	"=a" (_eax)	// Output
		);
#else /* defined(__amd64__) */
	__asm__ (
		"pushfq\n"
		"popq %%rax\n"
		"movl %%eax, %%edx\n"
		"xorl $0x200000, %%eax\n"
		"pushq %%rax\n"
		"popfq\n"
		"pushfq\n"
		"popq %%rax\n"
		"xorl %%edx, %%eax\n"
		"andl $0x200000, %%eax"
		:	"=a" (_eax)	// Output
		);
#endif
	
	if (!_eax)
	{
		// CPUID is not supported.
		// This CPU must be a 486 or older.
		return 0;
	}
	
	// CPUID is supported.
	// Check if the CPUID Features function (Function 1) is supported.
	unsigned int maxFunc;
	__cpuid(CPUID_MAX_FUNCTIONS, maxFunc, _ebx, _ecx, _edx);
	
	if (!maxFunc)
	{
		// No CPUID functions are supported.
		return 0;
	}
	
	// Get the CPU feature flags.
	__cpuid(CPUID_FAMILY_FEATURES, _eax, _ebx, _ecx, _edx);
	
	// Check the feature flags.
	CPU_Flags = 0;
	
	if (_edx & CPUFLAG_IA32_EDX_MMX)
		CPU_Flags |= CPUFLAG_MMX;
	if (_edx & CPUFLAG_IA32_EDX_SSE)
		CPU_Flags |= CPUFLAG_SSE;
	if (_edx & CPUFLAG_IA32_EDX_SSE2)
		CPU_Flags |= CPUFLAG_SSE2;
	if (_ecx & CPUFLAG_IA32_ECX_SSE3)
		CPU_Flags |= CPUFLAG_SSE3;
	if (_ecx & CPUFLAG_IA32_ECX_SSSE3)
		CPU_Flags |= CPUFLAG_SSSE3;
	if (_ecx & CPUFLAG_IA32_ECX_SSE41)
		CPU_Flags |= CPUFLAG_SSE41;
	if (_ecx & CPUFLAG_IA32_ECX_SSE42)
		CPU_Flags |= CPUFLAG_SSE42;
	
	// Check if the CPUID Extended Features function (Function 0x80000001) is supported.
	__cpuid(CPUID_MAX_EXT_FUNCTIONS, maxFunc, _ebx, _ecx, _edx);
	if (maxFunc >= CPUID_EXT_FAMILY_FEATURES)
	{
		// CPUID Extended Features are supported.
		__cpuid(CPUID_EXT_FAMILY_FEATURES, _eax, _ebx, _ecx, _edx);
		
		// Check the extended feature flags.
		if (_edx & CPUFLAG_IA32_EXT_EDX_MMXEXT)
			CPU_Flags |= CPUFLAG_MMXEXT;
		if (_edx & CPUFLAG_IA32_EXT_EDX_3DNOW)
			CPU_Flags |= CPUFLAG_3DNOW;
		if (_edx & CPUFLAG_IA32_EXT_EDX_3DNOWEXT)
			CPU_Flags |= CPUFLAG_3DNOWEXT;
		if (_ecx & CPUFLAG_IA32_EXT_ECX_SSE4A)
			CPU_Flags |= CPUFLAG_SSE4A;
		if (_ecx & CPUFLAG_IA32_EXT_ECX_SSE5)
			CPU_Flags |= CPUFLAG_SSE5;
	}
	
	// Return the CPU flags.
	return CPU_Flags;
	
#else
	// No flags for this CPU.
	return 0;
	
#endif
}
