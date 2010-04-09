/* Cpu detection code, extracted from mmx.h ((c)1997-99 by H. Dietz
   and R. Fisher). Converted to C and improved by Fabrice Bellard */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cputest.h"

/* ebx saving is necessary for PIC. gcc seems unable to see it alone */
#define cpuid(index,eax,ebx,ecx,edx)\
    __asm __volatile\
	("movl %%ebx, %%esi\n\t"\
         "cpuid\n\t"\
         "xchgl %%ebx, %%esi"\
         : "=a" (eax), "=S" (ebx),\
           "=c" (ecx), "=d" (edx)\
         : "a" (index));

#define CPUID_STD_MMX          0x00800000
#define CPUID_STD_SSE          0x02000000
#define CPUID_STD_SSE2         0x04000000
#define CPUID_STD_SSE3         0x00000001  // ECX!
#define CPUID_EXT_AMD_3DNOW    0x80000000
#define CPUID_EXT_AMD_3DNOWEXT 0x40000000
#define CPUID_EXT_AMD_MMXEXT   0x00400000
#define CPUID_EXT_CYR_MMX      0x00800000
#define CPUID_EXT_CYR_MMXEXT   0x01000000

/* Function to test if multimedia instructions are supported...  */
static int mm_support(void)
{
#ifdef ARCH_X86
    unsigned int rval;
    unsigned int eax, ebx, ecx, edx;
    char vendor[13] = "UnknownVndr";
    
    __asm__ __volatile__ (
                          /* See if CPUID instruction is supported ... */
                          /* ... Get copies of EFLAGS into eax and ecx */
                          "pushf\n\t"
#ifdef __x86_64__
                          "pop %%rax\n\t"
#else
                          "popl %0\n\t"
#endif
                          "movl %0, %1\n\t"
                          
                          /* ... Toggle the ID bit in one copy and store */
                          /*     to the EFLAGS reg */
                          "xorl $0x200000, %0\n\t"
#ifdef __x86_64__
			  "push %%rax\n\t"
#else
                          "push %0\n\t"
#endif
                          "popf\n\t"
                          
                          /* ... Get the (hopefully modified) EFLAGS */
                          "pushf\n\t"
#ifdef __x86_64__
                          "pop %%rax\n\t"
#else
                          "popl %0\n\t"
#endif
			#ifdef __x86_64__
			 : "=a" (eax), "=c" (ecx)
			#else
                          : "=a" (eax), "=c" (ecx)
			#endif
                          :
                          : "cc" 
                          );
    
    if (eax == ecx)
        return 0; /* CPUID not supported */
    
    cpuid(0, eax, ebx, ecx, edx);

    /* save the vendor string */
    *(unsigned int *)vendor = ebx;
    *(unsigned int *)&vendor[4] = edx;
    *(unsigned int *)&vendor[8] = ecx;

    rval = 0;

    /* highest cpuid is 0, no standard features */
    if (eax == 0)
        return rval;

    /* get standard features */
    cpuid(1, eax, ebx, ecx, edx); 
    //printf("CPUID 1 eax=0x%8.8x ebx=0x%8.8x ecx=0x%8.8x edx=0x%8.8x\n", eax, ebx, ecx, edx);
    if (edx & CPUID_STD_MMX)
        rval |= MM_MMX;
    if (edx & CPUID_STD_SSE)
        rval |= MM_MMXEXT | MM_SSE;
    if (edx & CPUID_STD_SSE2)
        rval |= MM_SSE2;
    //if (ecx & CPUID_STD_SSE3)
    //    rval |= MM_SSE3;

    /* check for extended feature flags support */
    cpuid(0x80000000, eax, ebx, ecx, edx);
    //printf("CPUID 8-0 eax=0x%8.8x ebx=0x%8.8x ecx=0x%8.8x edx=0x%8.8x\n", eax, ebx, ecx, edx);
    if (eax < 0x80000001)
        return rval;

    /* get extended feature flags */
    cpuid(0x80000001, eax, ebx, ecx, edx);
    //printf("CPUID 8-1 eax=0x%8.8x ebx=0x%8.8x ecx=0x%8.8x edx=0x%8.8x\n", eax, ebx, ecx, edx);

    /* AMD-specific extensions */
    if (strcmp(vendor, "AuthenticAMD") == 0) {
        if (edx & CPUID_EXT_AMD_3DNOW)
            rval |= MM_3DNOW;
        if (edx & CPUID_EXT_AMD_3DNOWEXT)
            rval |= MM_3DNOWEXT;
        /* if no MMXEXT already, check AMD-specific flag */
        if (((rval & MM_MMXEXT) == 0) && (edx & CPUID_EXT_AMD_MMXEXT))
            rval |= MM_MMXEXT;
    }

    /* Cyrix-specific extensions */
    else if (strcmp(vendor, "CyrixInstead") == 0) {
        /* The Cyrix CPU Detection Guide says that CPUID level 1 should 
           return the proper MMX feature flag, but just in case */
        if (((rval & MM_MMX) == 0) && (edx & CPUID_EXT_CYR_MMX))
            rval |= MM_MMX;
        /* if no MMXEXT already, check Cyrix-specific flag */
        if (((rval & MM_MMXEXT) == 0) && (edx & CPUID_EXT_CYR_MMXEXT))
            rval |= MM_MMXEXT;
    }

	//printf("%08x\n", rval);
    return rval;
#else // not X86
    return 0;
#endif
}

int ac_mmflag(void) 
{
 int mm_flag;

 mm_flag = mm_support();
#ifdef ARCH_X86
 mm_flag |= MM_IA32ASM;
#endif

#ifdef ARCH_POWERPC
 mm_flag |= MM_ALTIVEC; // FIXME for actual detection!
#endif
  return(mm_flag);
}
