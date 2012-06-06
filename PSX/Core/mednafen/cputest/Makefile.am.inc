mednafen_SOURCES	+= cputest/cputest.c

if ARCH_X86
mednafen_SOURCES	+= cputest/x86_cpu.c
endif

if ARCH_POWERPC
mednafen_SOURCES	+= cputest/ppc_cpu.c
endif

#if ARCH_ARM
#mednafen_SOURCES	+= cputest/arm_cpu.c
#endif
