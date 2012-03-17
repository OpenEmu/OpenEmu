/*
** Starscream 680x0 emulation library
** Copyright 1997, 1998, 1999 Neill Corlett
** Modified by Stéphane Dallongeville (1999, 2000, 2001, 2002)
** Used for the main 68000 CPU emulation in Gens
**
** Refer to STARDOC.TXT for terms of use, API reference, and directions on
** how to compile.
*/

#define VERSION "M0.26d"

/***************************************************************************/
/*
** NOTE
**
** All 68020-related variables and functions are currently experimental, and
** unsupported.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/***************************************************************************/
/*
** Register Usage
** --------------
**
** This is fairly consistent throughout the file.  Occasionally, EAX or EDX
** will be used as a scratch register (in some cases the x86 instruction set
** demands this).
**
** EAX: Bit   0  : V flag
**      Bit  1-7 : MUST BE ZERO
**      Bit   8  : C flag
**      Bit  14  : Z flag
**      Bit  15  : N flag
**      Bit 16-31: undefined
** EBX: Lower 16 bits: Current instruction word or register number
**      Upper 16 bits: zero
** ECX: Primary data
** EDX: Primary address
** EBP: Current base offset of PC
** ESI: Current PC, including base offset
** EDI: Cycle counter
*/
/***************************************************************************/
/*
**  68010 Loop Mode Timing
**  ----------------------
**
**  Loop mode is implemented entirely in the DBRA instruction.  It will
**  detect when it's supposed to be in loop mode, and adjust its timing when
**  applicable.
**
**  The __loopmode variable controls when loop mode is active.  It is set to
**  1 after an eligible loop is completed.  It is set to 0 when the loop
**  terminates, or when an interrupt / exception occurs.
**
**  Loop info byte:
**
**  Bits 1-3:  Continuation cycles / 2
**  Bits 4-6:  Termination cycles / 2
**  Bits 7-0:  (Expiration cycles - continuation cycles) / 2
**  (bit 7 wraps around to bit 0)
**
**  With the loop info byte in AL:
**     To get the continuation cycles:
**          and eax,byte 0Eh
**     To get the termination cycles:
**          shr al,3
**          and eax,byte 0Eh
**     To get the continue/expire difference:
**          rol al,2
**          and eax,byte 06h
**
**  Default = DBh
**           (11011011)
**                101 : 101 = 5   2*5            = 10 continuation cycles
**             101    : 101 = 5   2*5            = 10 termination cycles
**            1      1:  11 = 3   2*3 = 6   10+6 = 16 expiration cycles
**
**  (10/10/16 corresponds to the normal DBRA timing behavior)
*/
/***************************************************************************/
/*
** Algorithm for trace checkpoint in s680x0exec:
**
** If the SR trace flag is set {
**   Set the trace trickybit.  This differentiates us from the out-of-time
**    case above.
**   Set cycles_leftover = cycles_needed.
**   Force a context switch.
** } otherwise {
**   Clear the trace trickybit.
** }
** Begin the fetch/decode/execute loop as usual.
**
**
** In selected ret_timing routines:
**
** Subtract usual number of cycles from edi
** If I'm out of time (edi is negative) {
**   Jump to execend with SF set, as usual.
** } otherwise (guaranteed at least one more instruction) {
**   Jump to the s680x0exec trace checkpoint.
** }
**
**
** Make sure that the group 1 exception handler clears the trace trickybit.
**
**
** Upon reaching execend:
**
** If the trace trickybit is set {
**   Set cycles_needed = cycles_leftover.
**   Add cycles_needed to edi.
**   Generate a trace exception (clearing the SR trace flag in the process).
**   Clear the trace trickybit.
**   If edi is positive, resume the fetch/decode/execute loop.
**    Otherwise, fall through to the usual epilogue code.
** }
*/
/***************************************************************************/
/*
** Rebasing notes
** --------------
**
** Cached rebase happens on:
**  * JMP, JSR, RTS, RTE, RTR, RTD
**  * Exceptions (except hardware interrupts)
**
** Uncached rebase happens on:
**  * Entry to s680x0exec()
**  * Hardware interrupts
**  * Supervisor flag change
**  * Cache disable/invalidate (68020+)
**
** I can't think of any good reason why the hardware interrupt case should be
** uncached, except it happens to be convenient.
*/

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   dword;

static int use_stack   = -1;
static int hog         = -1;
static int addressbits = -1;
static int cputype     = -1;
static int quiet       = 0;
static char *sourcename = NULL;

/* This counts the number of instruction handling routines.  There's not much
** point to it except for curiosity. */
static int routine_counter = 0;

/* Misc. constants */
static char *x86ax   [5] = {"?", "al"  , "ax"  , "?", "eax"  };
static char *x86bx   [5] = {"?", "bl"  , "bx"  , "?", "ebx"  };
static char *x86cx   [5] = {"?", "cl"  , "cx"  , "?", "ecx"  };
static char *x86dx   [5] = {"?", "dl"  , "dx"  , "?", "edx"  };
static char *sizename[5] = {"?", "byte", "word", "?", "dword"};
static int quickvalue[8] = {8, 1, 2, 3, 4, 5, 6, 7};
static char direction[2] = {'r','l'};

/* Output file where code will be emitted */
static FILE *codefile;

/* Line number - used to make temporary labels i.e. "ln1234" */
static int linenum;

/* Effective address modes */
enum eamode {
	dreg, areg, aind, ainc, adec, adsp,
	axdp, absw, absl, pcdp, pcxd, immd
};

/* Loop information (68010) */
static int loop_c_cycles;
static int loop_t_cycles;
static int loop_x_cycles;
static unsigned char loopinfo[0x10000];

/*
** Misc. global variables which are used while generating instruction
** handling routines.  Some of these may assume more than one role.
*/
static enum eamode main_eamode;   /* EA mode, usually source */
static enum eamode main_destmode; /* EA mode, destination (for MOVE) */
static int main_size;             /* Operand size (1, 2, or 4) */
static int sizedef;               /* Size field in instruction word */
static int main_reg;              /* Register number */
static int main_cc;               /* Condition code (0-F) */
static int main_dr;               /* Direction (right or left) */
static int main_ir;               /* Immediate or register (for shifts) */
static int main_qv;               /* Quick value */

/* Emit a line of code (format string with other junk) */
static void emit(const char *fmt, ...) {
	va_list a;
	va_start(a, fmt);
	if(codefile) {
		vfprintf(codefile, fmt, a);
	} else {
		fprintf(stderr, "Bad news: Tried to emit() to null file\n");
		exit(1);
	}
}

/* Dump all options.  This is delivered to stderr and to the code file. */
static void optiondump(FILE *o, char *prefix) {
	fprintf(o, "%sCPU type: %d (%d-bit addresses)\n", prefix,
		cputype, addressbits);
	fprintf(o, "%sIdentifiers begin with \"%s\"\n", prefix,
		sourcename);
	fprintf(o, "%s%s calling conventions\n", prefix,
		use_stack ? "Stack" : "Register");
	fprintf(o, "%sHog mode: %s\n", prefix,
		hog ? "On" : "Off");
}

static void gen_banner(void) {
	emit("; Generated by STARSCREAM version " VERSION "\n");
	emit("; For assembly by NASM only\n");
	emit(";\n");
	emit("; Options:\n");
	optiondump(codefile, "; *  ");
	emit(";\n");
	emit("bits 32\n");
}

static void align(int n) {
	emit("times ($$-$)&%d db 0\n", n - 1);
}

static void maskaddress(char *reg) {
	if(addressbits < 32) {
		emit("and %s,%d\n", reg, (1 << addressbits) - 1);
	}
}

static void begin_source_proc(char *fname) {
	emit("global %s%s\n", sourcename, fname);
	emit("global _%s%s\n", sourcename, fname);
	emit("global %s%s_\n", sourcename, fname);
	emit("%s%s:\n", sourcename, fname);
	emit("_%s%s:\n", sourcename, fname);
	emit("%s%s_:\n", sourcename, fname);
}

/* Generate variables */
static void gen_variables(void) {
	emit("section .data\n");
	emit("bits 32\n");

	emit("\n");
	emit("\textern Ram_68k\n");

// TODO: Port from Gens Rerecording
#if 0
	emit("\textern _GensTrace\n");

	emit("\textern _hook_read_byte\n");
	emit("\textern _hook_read_word\n");
	emit("\textern _hook_read_dword\n");
	emit("\textern _hook_write_byte\n");
	emit("\textern _hook_write_word\n");
	emit("\textern _hook_write_dword\n");
	emit("\textern _hook_pc\n");
	emit("\textern _hook_address\n");
	emit("\textern _hook_value\n");
//	emit("\textern _Fix_Codes\n");
#endif
	
	emit("\textern Rom_Data\n");
	emit("\textern Rom_Size\n");
	emit("\n");

	emit("global %scontext\n", sourcename);
	emit("global _%scontext\n", sourcename);
	emit("global _%scontext\n", sourcename);
	align(8);
	emit("%scontext:\n", sourcename);
	emit("_%scontext:\n", sourcename);
	emit("contextbegin:\n");
	/*
	** CONTEXTINFO_MEM16
	** CONTEXTINFO_MEM16FC
	**
	** 16-bit memory interface
	*/
	if(cputype <= 68010) {
		emit("__fetch                dd 0\n");
		emit("__readbyte             dd 0\n");
		emit("__readword             dd 0\n");
		emit("__writebyte            dd 0\n");
		emit("__writeword            dd 0\n");
		emit("__s_fetch              dd 0\n");
		emit("__s_readbyte           dd 0\n");
		emit("__s_readword           dd 0\n");
		emit("__s_writebyte          dd 0\n");
		emit("__s_writeword          dd 0\n");
		emit("__u_fetch              dd 0\n");
		emit("__u_readbyte           dd 0\n");
		emit("__u_readword           dd 0\n");
		emit("__u_writebyte          dd 0\n");
		emit("__u_writeword          dd 0\n");
		if(cputype == 68010) {
			emit("__f_readbyte           dd 0\n");
			emit("__f_readword           dd 0\n");
			emit("__f_writebyte          dd 0\n");
			emit("__f_writeword          dd 0\n");
		}
	/*
	** CONTEXTINFO_MEM32
	**
	** 32-bit memory interface
	*/
	} else {
		emit("__fetch                dd 0\n");
		emit("__readbus              dd 0\n");
		emit("__writebus             dd 0\n");
		emit("__s_fetch              dd 0\n");
		emit("__s_readbus            dd 0\n");
		emit("__s_writebus           dd 0\n");
		emit("__u_fetch              dd 0\n");
		emit("__u_readbus            dd 0\n");
		emit("__u_writebus           dd 0\n");
		emit("__f_readbus            dd 0\n");
		emit("__f_writebus           dd 0\n");
	}
	/*
	** CONTEXTINFO_COMMON
	**
	** Registers and other info common to all CPU types
	**
	** It should be noted that on a double fault, bit 0 of both __pc and
	** __interrupts will be set to 1.
	*/
	if(cputype >= 68000) {
		emit("__resethandler         dd 0\n");
		emit("__reg:\n");
		emit("__dreg                 dd 0,0,0,0,0,0,0,0\n");
		emit("__areg                 dd 0,0,0,0,0,0,0\n");
		emit("__a7                   dd 0\n");
		emit("__asp                  dd 0\n");
		emit("__pc                   dd 0\n");
		emit("__odometer             dd 0\n");
		/* Bit 4 of __interrupts = stopped state */
		emit("__interrupts           db 0,0,0,0,0,0,0,0\n");
		emit("__sr                   dw 0\n");
	}
	/*
	** CONTEXTINFO_68000SPECIFIC
	*/
	if(cputype == 68000) {
		emit("__contextfiller00      dw 0\n");
	}
	/*
	** CONTEXTINFO_68010
	**
	** Registers used on the 68010 and higher
	*/
	if(cputype >= 68010) {
		emit("__sfc                  db 0\n");
		emit("__dfc                  db 0\n");
		emit("__vbr                  dd 0\n");
		emit("__bkpthandler          dd 0\n");
	}
	/*
	** CONTEXTINFO_68010SPECIFIC
	**
	** Registers used only on the 68010
	*/
	if(cputype == 68010) {
		emit("__loopmode             db 0\n");
		emit("__contextfiller10      db 0,0,0\n");
	}
	/*
	** CONTEXTINFO_68020
	**
	** Registers used on the 68020 and higher
	*/
	if(cputype >= 68020) {
		/*
		** 68020 stack pointer rules (tentative)
		**
		** First of all, the 68000/68010 stack pointer behavior has
		** not changed:
		**
		** 1. In supervisor mode, __a7 contains the supervisor stack
		**    pointer and __asp contains the user stack pointer.
		** 2. In user mode, __a7 contains the user stack pointer and
		**    __asp contains the supervisor stack pointer.
		**
		** The only difference is that the "supervisor stack pointer"
		** can be either ISP or MSP.  __xsp contains whichever stack
		** pointer is _not_ the current "supervisor stack pointer".
		**
		** Here's a table summarizing the above rules:
		**
		**   S M | __a7 __asp __xsp
		**   ----+-----------------
		**   0 0 |  USP   ISP   MSP
		**   0 1 |  USP   MSP   ISP
		**   1 0 |  ISP   USP   MSP
		**   1 1 |  MSP   USP   ISP
		**
		** As usual, whenever SR changes, we have to play Stack
		** Pointer Switcheroo:
		**
		**  * If S changes: swap __asp and __a7 (as usual)
		**  * If M changes:
		**    - If S=0, swap __xsp and __asp
		**    - If S=1, swap __xsp and __a7
		*/
		emit("__xsp                  dd 0\n");
	}
/*	align(4);*/
	emit("__cycles_needed        dd 0\n");
	emit("__cycles_leftover      dd 0\n");
	emit("__fetch_region_start   dd 0\n");/* Fetch region cache */
	emit("__fetch_region_end     dd 0\n");
	emit("__xflag                db 0\n");
	/*
	**  Format of __execinfo:
	**  Bit 0:  s680x0exec currently running
	**  Bit 1:  PC out of bounds
	**  Bit 2:  Special I/O section
	**
	** "Special I/O section" is enabled during group 0 exception
	** processing, and it means a couple things:
	**   * Address and bus errors will not be tolerated (the CPU will
	**     just keel over and die).  Therefore, information such as the
	**     current PC is not relevant.
	**   * Registers are not necessarily live.  Since special I/O
	**     sections are guaranteed not to cause exceptions, this is not a
	**     problem.
	*/
	emit("__execinfo             db 0\n");
	emit("__trace_trickybit      db 0\n");/* Pending trace exception */
	emit("__filler               db 0\n");
	emit("__io_cycle_counter     dd -1\n");/*always -1 when idle*/
	emit("__io_fetchbase         dd 0\n");
	emit("__io_fetchbased_pc     dd 0\n");
	emit("__access_address       dd 0\n");
	emit("\n\n");
	emit("; Dirty variable (Gens)\n\n");
	emit("save_01				dd 0\n");
	emit("save_02				dd 0\n");
	emit("contextend:\n");
}

/* Prepare to leave into the cold, dark world of compiled C code */
static void airlock_exit(void) {
	emit("mov [__io_cycle_counter],edi\n");
	emit("mov [__io_fetchbase],ebp\n");
	emit("mov [__io_fetchbased_pc],esi\n");
	emit("push ebx\n");
	emit("push eax\n");
}

/* Prepare to return to the warm fuzzy world of assembly code
** (where everybody knows your name) */
static void airlock_enter(void) {
	emit("pop eax\n");
	emit("pop ebx\n");
	emit("mov edi,[__io_cycle_counter]\n");
	emit("mov ebp,[__io_fetchbase]\n");
	emit("mov esi,[__io_fetchbased_pc]\n");
}

enum { airlock_stacksize = 8 };

static void cache_ccr(void) {
	emit("mov al,[__sr]\n");    /* read CCR -> AL */                 /* ????????000XNZVC */
	emit("mov ah,al\n");        /* copy to AH */                     /* 000XNZVC000XNZVC */
	emit("and ax,0C10h\n");     /* isolate NZ...X */                 /* 0000NZ00000X0000 */
	emit("shl ah,3\n");       /* put NZ almost where we want it */   /* 0NZ00000000X0000 */
	emit("shr al,4\n");         /* shift X flag into bit 0 */        /* 0NZ000000000000X */
	emit("mov [__xflag],al\n"); /* store X flag */                   /* 0NZ000000000000X al -> xflag */
	emit("mov al,[__sr]\n");    /* read CCR -> AL again */           /* 0NZ00000000XNZVC */
	emit("and al,3\n");         /* isolate VC */                     /* 0NZ00000000000VC */
	emit("shr al,1\n");         /* just V */                         /* 0NZ000000000000V carry */
	emit("adc ah,ah\n");        /* append C to rest of flags */      /* NZ00000C0000000V */
}

static void writeback_ccr(void) {
	emit("shr ah,1\n");         /* C flag -> x86 carry */            /* 0NZ?????0000000V carry */
	emit("adc ax,ax\n");        /* append to V flag */               /* NZ?????0000000VC */
	emit("and ax,0C003h\n");    /* isolate NZ.......VC */            /* NZ000000000000VC */
	emit("or ah,[__xflag]\n");  /* load X flag */                    /* NZ00000X000000VC */
	emit("ror ah,4\n");         /* now we have XNZ....VC */          /* 000XNZ00000000VC */
	emit("or al,ah\n");         /* OR them together */               /* 000XNZ00000XNZVC */
	emit("mov [__sr],al\n");    /* store the result */               /* 000XNZ00000XNZVC al -> sr */
}

/*
** This will make edi _very_ negative... far enough negative that the
** leftover cycle incorporation at the end of s68000exec() shouldn't be
** enough to make it positive.
*/
static void force_context_switch(void) {
	emit("sub edi,[__cycles_needed]\n");
	emit("mov dword[__cycles_needed],0\n");
}

/*
** Put all the unused cycles in the leftover cycle bank, so we can call
** attention to the tricky bit processor.
*/
static void force_trickybit_process(void) {
	emit("inc edi\n");
	emit("add [__cycles_leftover],edi\n");
	emit("or edi,byte -1\n");/* smaller than a mov */
}

/* "newpc" has been renamed to this */
void perform_cached_rebase(void);

/* Copy either __s_* or __u_* memory map pointers */
static void copy_memory_map(char *map, char *reg) {
	emit("mov %s,[__%s_fetch]\n", reg, map);
	emit("mov [__fetch],%s\n", reg);
	if(cputype < 68020) {
		emit("mov %s,[__%s_readbyte]\n" , reg, map);
		emit("mov [__readbyte],%s\n" , reg);
		emit("mov %s,[__%s_readword]\n" , reg, map);
		emit("mov [__readword],%s\n" , reg);
		emit("mov %s,[__%s_writebyte]\n", reg, map);
		emit("mov [__writebyte],%s\n", reg);
		emit("mov %s,[__%s_writeword]\n", reg, map);
		emit("mov [__writeword],%s\n", reg);
	} else {
		emit("mov %s,[__%s_readbus]\n"  , reg, map);
		emit("mov [__readbus],%s\n"  , reg);
		emit("mov %s,[__%s_writebus]\n" , reg, map);
		emit("mov [__writebus],%s\n" , reg);
	}
}

/***************************************************************************/

static void gen_interface(void) {
	emit("section .text\n");
	emit("bits 32\n");

	// Symbol redefines for ELF
	emit("\n");
	emit("\t%%ifdef __OBJ_ELF\n");
	emit("\t\t%%define _Int_Ack Int_Ack\n");
	emit("\t%%endif\n");
	emit("\n");
	
	emit("\n");
	emit("\textern M68K_RB\n");
	emit("\textern M68K_RW\n");
	emit("\textern M68K_WB\n");
	emit("\textern M68K_WW\n");
	emit("\textern _Int_Ack\n");
	emit("\n");

	emit("top:\n");

/***************************************************************************/
/*
**  s680x0init()
**
**  Entry: Nothing
**  Exit:  Zero
**
**  This must be called before anything else.  It decompresses the main jump
**  table (and loop info, in the case of the 68010).
*/
	begin_source_proc("init");

	emit("pushad\n");
	emit("mov edi,__jmptbl\n");
	emit("mov esi,__jmptblcomp\n");
	if(cputype == 68010) {
		emit("mov ebx,__looptbl\n");
	}
	emit(".decomp:\n");
	emit("lodsd\n");
	emit("mov ecx,eax\n");
	emit("and eax,0FFFFFFh\n");
	emit("shr ecx,24\n");
	emit("add eax,top\n");
	emit("inc ecx\n");
	if(cputype == 68010) {
		emit("mov ebp,ecx\n");
	}
	emit(".jloop:\n");
	emit("mov [edi],eax\n");
	emit("add edi,byte 4\n");
	emit("dec ecx\n");
	emit("jnz short .jloop\n");
	if(cputype == 68010) {
		emit("lodsb\n");
		emit(".lloop:\n");
		emit("mov [ebx],al\n");
		emit("inc ebx\n");
		emit("dec ebp\n");
		emit("jnz short .lloop\n");
	}
	emit("cmp edi,__jmptbl+262144\n");
	emit("jne short .decomp\n");

	emit("popad\n");
	emit("xor eax,eax\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0exec(cycles)
**
**  // Entry: EAX = # cycles to execute
**  Entry: EAX = odometer wanted
**  Exit:  80000000h:  success
**         80000001h:  PC out of range
**         80000002h:  unsupported stack frame
**         80000003h:  already executed
**         80000004h:  stopped
**         FFFFFFFFh:  CPU is dead because of a double fault
**       < 80000000h:  invalid instruction = address of invalid instr.
*/
	begin_source_proc("exec");

	if(use_stack) emit("mov eax,[esp+4]\n");

	/*
	** Check for stopped and double-faulted states.
	*/

emit("sub eax, [__odometer]\n");
emit("jbe .already_done\n");

	emit("test byte[__interrupts],0x10\n");
	emit("jz .notstopped\n");
	emit("test byte[__pc],1\n");
	emit("jz .notfaulted\n");
	emit("or eax,byte -1\n");
	emit("ret\n");
	emit(".notfaulted:\n");
	emit("add [__odometer],eax\n");
	emit("mov eax,80000004h\n");
	emit("ret\n");

emit(".already_done\n");

	emit("mov eax,80000003h\n");
	emit("ret\n");

	emit(".notstopped:\n");

	emit("push ebp\n");
	emit("push ebx\n");
	emit("push ecx\n");
	emit("push edx\n");
	emit("push esi\n");
	emit("push edi\n");

	emit("mov [__cycles_needed],eax\n");
	emit("mov edi,eax\n");/* store # of cycles to execute */
	emit("dec edi\n");

	emit("xor ebx,ebx\n");
	emit("mov esi,[__pc]\n");
	cache_ccr();
	emit("xor ebp,ebp\n");
	emit("mov byte[__execinfo],1\n");

	/*
	** Force an uncached re-base.
	** This fulfills the "Entry to s680x0exec()" case.
	*/
	emit("call basefunction\n");
	emit("add esi,ebp\n");
	emit("test byte[__execinfo],2\n"); /* Check for PC out of bounds */
	emit("jnz near exec_bounderror\n");

	emit("mov dword[__cycles_leftover],0\n");/* an extra precaution */

	/* PPL and Trace checkpoint */
	emit("exec_checkpoint:\n");
	emit("js short execquit\n");

	/* Check PPL */
	emit("mov cl,[__sr+1]\n");
	emit("mov ch,[__interrupts]\n");
	emit("and cl,byte 7\n");
	emit("cmp ch,byte 7\n");
	emit("je short .yesint\n");
	emit("cmp cl,ch\n");
	emit("jae short .noint\n");
	emit(".yesint:\n");
	emit("call flush_interrupts\n");
	/* Force an uncached re-base */
	emit("call basefunction\n");
	emit("add esi,ebp\n");

emit("or edi,edi\n");
emit("js near execquit\n");

	emit("test byte[__execinfo],2\n"); /* Check for PC out of bounds */
	emit("jnz near exec_bounderror\n");
	emit(".noint:\n");

	emit("execloop:\n");
/*	emit("xor ebx,ebx\n");suffice to say, bits 16-31 should be zero... */
	emit("mov bx,[esi]\n");
	emit("add esi,byte 2\n");
	
// TODO: Port from Gens Rerecording
#if 0
	emit("pushad\n");
	emit("sub esi,ebp\n");
	emit("sub esi,byte 2\n");
	emit("mov [_hook_pc],esi\n");
	emit("call _GensTrace\n");
	emit("popad\n");
#endif
			     
	emit("jmp dword[__jmptbl+ebx*4]\n");
	/* Traditional loop - used when hog mode is off */
	if(!hog) {
		emit("execend:\n");
		emit("jns short execloop\n");
	}
	emit("execquit:\n");
	emit("execquit_notrace:\n");

	/*
	** Look for pending interrupts that exceed the current PPL.  These
	** are higher priority and are therefore processed last (the ISR will
	** end up getting control).
	*/
	emit("mov ch,[__interrupts]\n");
	emit("mov cl,[__sr+1]\n");
	emit("and ch,byte 7\n");
	emit("and cl,byte 7\n");
	emit("cmp ch,byte 7\n");
	emit("je short execquit_yesinterrupt\n");
	emit("cmp cl,ch\n");
	emit("jae short execquit_nointerrupt\n");
	emit("execquit_yesinterrupt:\n");
	emit("call flush_interrupts\n");

	/*
	** Force an uncached re-base.
	** This fulfills the "Hardware interrupt" case.
	*/
	emit("call basefunction\n");
	emit("add esi,ebp\n");
	emit("test byte[__execinfo],2\n"); /* Check for PC out of bounds */
	emit("jnz short exec_bounderror\n");
	emit("execquit_nointerrupt:\n");

	/*
	** Incorporate leftover cycles (if any) and see if we should keep
	** running.
	*/
	emit("add edi,[__cycles_leftover]\n");
	emit("mov dword[__cycles_leftover],0\n");
	emit("jns short execloop\n");

	/* Leave s680x0exec with "Success" code. */
	emit("mov ecx,80000000h\n");

	/*
	** Exit the s680x0exec routine.  By this time the return code should
	** already be in ecx.
	*/
	emit("execexit:\n");
	emit("sub esi,ebp\n");
	writeback_ccr();
	emit("mov [__pc],esi\n");
	emit("inc edi\n");
	emit("mov edx,[__cycles_needed]\n");
	emit("sub edx,edi\n");
	emit("add [__odometer],edx\n");
	emit("mov byte[__execinfo],0\n");

	/*
	** Remember: __io_cycle_counter is always -1 when idle!
	**
	** This prevents us from having to check __execinfo during the
	** readOdometer / tripOdometer calls.
	*/
	emit("mov dword[__cycles_needed],0\n");
	emit("mov dword[__io_cycle_counter],-1\n");

	emit("mov eax,ecx\n");/* return code */
	emit("pop edi\n");
	emit("pop esi\n");
	emit("pop edx\n");
	emit("pop ecx\n");
	emit("pop ebx\n");
	emit("pop ebp\n");
	emit("ret\n");

	/*
	** Leave s680x0exec with "Out of bounds" code.
	*/
	emit("exec_bounderror:\n");
	emit("mov ecx,80000001h\n");
	emit("jmp short execexit\n");

	/*
	** Invalid instruction handler
	*/
	emit("invalidins:\n");
	emit("sub esi,byte 2\n");  /* back up one word */
	emit("mov ecx,esi\n");/* get address in ecx */
	emit("sub ecx,ebp\n");/* subtract base */
	maskaddress("ecx");
	if(addressbits == 32) {
		emit("and ecx,7FFFFFFFh\n");
	}
/*	emit("or byte[__stopped],2\n");*/
	emit("jmp short execexit\n");

/***************************************************************************/
/*
**  s680x0reset()
**
**  Entry: Nothing
**  Exit:  0 on success
**         1 on failure:
**           * if there's no Supervisor Program entry for address 0
**           * if s680x0exec() is active
**        -1 on double fault
*/
	begin_source_proc("reset");

	emit("mov eax,1\n");
	emit("test [__execinfo],al\n"); /* Ensure s680x0exec() inactive */
	emit("jnz near .return\n");
	emit("cmp dword[__s_fetch],0\n");
	emit("je near .return\n");
	emit("dec eax\n");
	emit("mov [__execinfo],al\n");
	emit("sub eax,byte 16\n");
	emit(".gp:\n");
	emit("mov dword[__reg+64+eax*4],0\n");
	emit("inc eax\n");
	emit("jnz short .gp\n");
	emit("mov [__asp],eax\n");
	if(cputype >= 68020) emit("mov [__xsp],eax\n");
	/* Set up SR for no tracing, supervisor mode, ISP, PPL 7 */
	emit("mov word[__sr],2700h\n");
	if(cputype >= 68010) {
		emit("mov [__vbr],eax\n");
		emit("mov [__sfc],al\n");
		emit("mov [__dfc],al\n");
	}
	if(cputype == 68010) {
		emit("mov [__loopmode],al\n");
 	}
	/* Copy supervisor address space information */
	copy_memory_map("s", "eax");
	/* Generate Supervisor Program Space reads to get the initial PC and
	** SSP/ISP */
	emit("mov eax,1\n"); /* assume failure */
	emit("mov [__pc],eax\n");
	emit("mov byte [__interrupts],0x10\n");	/* stopped */
	emit("push esi\n");
	emit("push ebp\n");
	emit("xor esi,esi\n");
	emit("call basefunction\n");/* will preserve eax */
	emit("test byte[__execinfo],2\n");
	emit("jnz short .exit\n");
	emit("add esi,ebp\n");
	emit("mov eax,[esi]\n");
	emit("rol eax,16\n");
	emit("mov [__a7],eax\n");
	emit("mov eax,[esi+4]\n");
	emit("rol eax,16\n");
	emit("mov [__pc],eax\n");
	/* An address error here will cause a double fault */
	emit("and eax,byte 1\n");
	emit("mov byte [__interrupts],0\n");
	emit("neg eax\n"); /* -1 on double fault, 0 on success */
	emit(".exit:\n");
	emit("pop ebp\n");
	emit("pop esi\n");
	emit(".return:\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0interrupt(level, vector)
**
**  Entry: EAX = interrupt level
**         EDX = vector (-1 for auto, -2 for spurious)
**  Exit:  EAX = 0 on success
**               1 on failure, previous vector exists
**               2 on invalid input
*/
	begin_source_proc("interrupt");

	if(use_stack) {
		emit("mov al,[esp+4]\n");  /* al = level  */
	}

	/*
	** Commit the given interrupt and vector number.
	*/
	emit("mov [__interrupts],al\n");
	emit("mov eax,[__io_cycle_counter]\n");
	emit("inc eax\n");
	emit("add [__cycles_leftover],eax\n");
	emit("mov dword[__io_cycle_counter],-1\n");
	emit("xor eax,eax\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0flushInterrupts()
**
**  Flushes all pending interrupts.
**
**  Entry: Nothing
**  Exit:  Nothing
*/
	begin_source_proc("flushInterrupts");

	/*
	** If s680x0exec() is already running, then the interrupts are about
	** to get flushed anyway.  So ignore this call.
	*/
	emit("test byte[__execinfo],1\n");
	emit("jnz short .noflush\n");
	/* Make registers "live" */
	emit("pushad\n");

	emit("mov cl,[__sr+1]\n");
	emit("mov ch,[__interrupts]\n");
	emit("and cl,byte 7\n");
	emit("cmp ch,byte 7\n");
	emit("je short .yesint_1\n");
	emit("cmp cl,ch\n");
	emit("jae short .noint_1\n");

	emit(".yesint_1\n");
	emit("mov esi,[__pc]\n");
	emit("xor ebp,ebp\n");
	emit("xor edi,edi\n"); /* well, semi-live */
	emit("call flush_interrupts\n");
	emit("sub [__odometer],edi\n"); /* edi will be <= 0 here */
	emit("mov [__pc],esi\n"); /* PC guaranteed unbased */

	emit(".noint_1\n");
	emit("popad\n");
	emit(".noflush:\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0GetContextSize()
**
**  Entry: Nothing
**  Exit:  Size of context array (in bytes)
*/
	begin_source_proc("GetContextSize");

	emit("mov eax,contextend-contextbegin\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0GetContext(context)
**
**  Entry: Address of context in EAX
**  Exit:  Nothing
*/
	begin_source_proc("GetContext");

	emit("push edx\n");
	emit("push edi\n");
	if(use_stack) emit("mov edi,[esp+12]\n");
	else          emit("mov edi,eax\n");
	emit("%%assign i 0\n");
	emit("%%rep ((contextend-contextbegin) / 8)\n");
	emit("  mov eax,[contextbegin+i+0]\n");
	emit("  mov edx,[contextbegin+i+4]\n");
	emit("  mov [edi+i+0],eax\n");
	emit("  mov [edi+i+4],edx\n");
	emit("%%assign i i+8\n");
	emit("%%endrep\n");
	emit("%%if ((contextend-contextbegin) %% 8)!=0\n");
	emit("  mov eax,[contextbegin+i+0]\n");
	emit("  mov [edi+i+0],eax\n");
	emit("%%endif\n");
	emit("pop edi\n");
	emit("pop edx\n");
	emit("xor eax,eax\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0SetContext(context)
**
**  Entry: Address of context in EAX
**  Exit:  Nothing
*/
	begin_source_proc("SetContext");

	emit("push edx\n");
	emit("push esi\n");
	if(use_stack) emit("mov esi,[esp+12]\n");
	else          emit("mov esi,eax\n");
	emit("%%assign i 0\n");
	emit("%%rep ((contextend-contextbegin) / 8)\n");
	emit("  mov eax,[esi+i+0]\n");
	emit("  mov edx,[esi+i+4]\n");
	emit("  mov [contextbegin+i+0],eax\n");
	emit("  mov [contextbegin+i+4],edx\n");
	emit("%%assign i i+8\n");
	emit("%%endrep\n");
	emit("%%if ((contextend-contextbegin) %% 8)!=0\n");
	emit("  mov eax,[esi+i+0]\n");
	emit("  mov [contextbegin+i+0],eax\n");
	emit("%%endif\n");
	emit("pop esi\n");
	emit("pop edx\n");
	emit("xor eax,eax\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0fetch(address)
**
**  (Will need to be rewritten to handle function codes)
**
**  Reads a word from memory using the _supervisor_ fetch map.
**  Entry: Address in EAX
**  Exit:  The fetched word (or -1 if the address was invalid)
*/
	begin_source_proc("fetch");

	if(use_stack) emit("mov eax,[esp+4]\n");
	emit("push ebx\n");
	emit("push esi\n");
	emit("push edi\n");/* can be destroyed by rebase */
	emit("push ebp\n");
	emit("push dword[__fetch]\n");
	emit("mov ebx,[__s_fetch]\n");
	emit("mov [__fetch],ebx\n");
	/* Must save the fetch region cache as well */
	emit("push dword[__fetch_region_start]\n");
	emit("push dword[__fetch_region_end]\n");
	/* and __execinfo */
	emit("mov bl,[__execinfo]\n");
	emit("push ebx\n");

	emit("xor ebp,ebp\n");
	emit("mov esi,eax\n");
	emit("and byte[__execinfo],0FDh\n");
	/* Force re-base */
	emit("call basefunction\n");
	emit("test byte[__execinfo],2\n");
	emit("mov eax,-1\n");
	emit("jnz short .badfetch\n");
	emit("add esi,ebp\n");
	emit("inc eax\n"); /* make eax zero */
	emit("mov ax,[esi]\n");
	emit(".badfetch:\n");
	emit("pop ebx\n");
	emit("mov [__execinfo],bl\n");
	emit("pop dword[__fetch_region_end]\n");
	emit("pop dword[__fetch_region_start]\n");
	emit("pop dword[__fetch]\n");
	emit("pop ebp\n");
	emit("pop edi\n");
	emit("pop esi\n");
	emit("pop ebx\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0readOdometer()
**
**  Reads the odometer (works from anywhere)
**  Entry: Nothing
**  Exit:  Odometer in EAX
*/
	begin_source_proc("readOdometer");

	emit("mov eax,[__cycles_needed]\n");
	emit("sub eax,[__io_cycle_counter]\n");
	emit("dec eax\n");               /* eax = elapsed cycles */
	emit("sub eax,[__cycles_leftover]\n");
	emit("add eax,[__odometer]\n");  /* add to old __odometer */
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0tripOdometer()
**
**  Reads and then clears the odometer (works from anywhere)
**  Entry: Nothing
**  Exit:  Old odometer value in EAX
*/
	begin_source_proc("tripOdometer");
	/* Read */
	emit("mov eax,[__cycles_needed]\n");
	emit("sub eax,[__io_cycle_counter]\n");
	emit("dec eax\n");               /* eax = elapsed cycles */
	emit("sub eax,[__cycles_leftover]\n");
	emit("add [__odometer],eax\n");  /* add to old __odometer */
	/* Clear */
	emit("mov eax,[__io_cycle_counter]\n");
	emit("inc eax\n");
	emit("mov [__cycles_needed],eax\n");
	emit("mov eax,[__odometer]\n");
	emit("mov dword[__odometer],0\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0controlOdometer(n)
**
**  Reads the odometer, clears it only if n != 0
**  Entry: n in EAX
**  Exit:  Old odometer value in EAX
*/
	begin_source_proc("controlOdometer");
	if(use_stack) emit("mov eax,[esp+4]\n");
	emit("or eax,eax\n");
	emit("jnz short _%stripOdometer\n", sourcename);
	emit("jmp short _%sreadOdometer\n", sourcename);

/***************************************************************************/
/*
**  s680x0releaseTimeslice()
**
**  Ends the s680x0exec call prematurely.  The early exit is reflected in
**  __odometer.
**  Entry: Nothing
**  Exit:  Nothing
*/
	begin_source_proc("releaseTimeslice");

	emit("mov eax,[__cycles_needed]\n");
	emit("sub [__io_cycle_counter],eax\n");
	emit("xor eax,eax\n");
	emit("mov [__cycles_needed],eax\n");
	emit("ret\n");


/***************************************************************************/
/*
**  s680x0releaseCycles(eax : cycles)
**
**  add some cycle to the cyclecounter (util for DMA transfert)
**  __odometer.
**  Entry: Nothing
**  Exit:  Nothing
*/
	begin_source_proc("releaseCycles");

	emit("sub [__io_cycle_counter], eax\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0addCycles(int cycles)
**
**  add some cycle to the cyclecounter (util for DMA transfert)
**  __odometer.
**  Entry: Nothing
**  Exit:  Nothing
*/
	begin_source_proc("addCycles");

	if(use_stack) emit("mov eax,[esp+4]\n");
	emit("add [__odometer], eax\n");
	emit("ret\n");

/***************************************************************************/
/*
**  s680x0readPC()
**
**  Returns the current program counter.  Works anywhere, including I/O,
**  RESET, and BKPT handlers.
**
**  Note that the value returned won't necessarily coincide exactly with the
**  beginning of an instruction.
*/
	begin_source_proc("readPC");

	emit("test byte[__execinfo],1\n");
	emit("jnz short .live\n");
	emit("mov eax,[__pc]\n");
	emit("ret\n");
	emit(".live:\n");
	emit("mov eax,[__io_fetchbased_pc]\n");
	emit("sub eax,[__io_fetchbase]\n");
	emit("ret\n");

}

/***************************************************************************/

/*
** Routine that flushes pending interrupts (with correct priority).
** Assumes "live" registers, including EDI and EBP.
**
** Does not rebase the PC.  In fact, it will un-base the PC even if there
** aren't any pending interrupts.
**
** s680x0flushInterrupts() is actually a wrapper for this.
*/
static void gen_flush_interrupts(void) {
	int cycles = (cputype == 68010) ? 46 : 44;
	align(16);
	emit("flush_interrupts:\n");
	/* Unbase PC */
	emit("xor edx,edx\n");
	emit("sub esi,ebp\n");
	emit("xor ebp,ebp\n");

	emit("mov dl,[__interrupts]\n");
	emit("and edx, 0x7\n");
	emit("jz short .noint\n");

	emit("lea edx, [edx * 4 + (0x18 * 4)]\n");
	emit("call group_1_exception\n");
	emit("mov dl, [__interrupts]\n");
	emit("mov dh, [__sr+1]\n");
	emit("and dl, 0x07\n");
	emit("and dh, 0xF8\n");
	emit("sub edi, byte %d\n", cycles);
	emit("or dh, dl\n");
	emit("mov [__sr+1], dh\n");

	emit("push eax\n");
	emit("call _Int_Ack\n");					// in: dl = old IPL line, out: al = new IPL line
	emit("mov byte [__interrupts],al\n");	// set it
	emit("pop eax\n");

	emit(".noint\n");
	emit("ret\n");
}

/***************************************************************************/

static void ret_timing(int n) {
	if(n) {
		emit("sub edi,%s%d\n", (n < 128) ? "byte " : "", n);
	} else {
		emit("or edi,edi\n");
	}
	/* If hog mode is off, jump back to the main loop */
	if(!hog) {
		emit("jmp execend\n");
	/* If hog mode is on, fetch and execute the next instruction */
	} else {
		emit("js near execquit\n");
		emit("mov bx,[esi]\n");
		emit("add esi,byte 2\n");
		
// TODO: Port from Gens Rerecording
#if 0
		emit("pushad\n");
		emit("sub esi,ebp\n");
		emit("sub esi,byte 2\n");
		emit("mov [_hook_pc],esi\n");
		
		emit("shr ah,1\n");
		emit("adc ax,ax\n");
		emit("and ax,0C003h\n");
		emit("or ah,[__xflag]\n");
		emit("ror ah,4\n");
		emit("or al,ah\n");
		emit("mov [__sr],al\n");
		
		emit("call _GensTrace\n");
		emit("popad\n");
#endif
			
		emit("jmp dword[__jmptbl+ebx*4]\n");
	}
}

/*
** This version of ret_timing does a trace flag check.
**
** Note: this only needs to be used for instructions which can potentially
**  _set_ the trace flag.  Instructions which can't set the trace flag (even
**  if they can clear it) are OK to use ret_timing as usual.  Why?  Well, if
**  an instruction is run in trace mode, that instruction is doomed to be
**  traced, regardless if it clears the trace flag during its execution.
**  Furthermore, the trace exception (being a group 1 exception after all)
**  will clear the trace tricky bit as well as the trace flag.
*/
static void ret_timing_checkpoint(int n) {
	if(n) {
		emit("sub edi,%s%d\n", (n < 128) ? "byte " : "", n);
	} else {
		emit("or edi,edi\n");
	}
	emit("jmp exec_checkpoint\n");
}

/***************************************************************************/

/* This routine decodes an extension word into EDX. */
static void gen_decode_ext(void) {
	align(16);
	emit("decode_ext:\n");
	if(cputype <= 68010) {
		emit("push ecx\n");
		emit("movzx edx,word[esi]\n");
		emit("movsx ecx,dl\n");
		emit("add esi,byte 2\n");
		emit("shr edx,12\n");
		emit("mov edx,[__reg+edx*4]\n");
		emit("jc short .long\n");
		emit("movsx edx,dx\n");
		emit(".long:\n");
		emit("add edx,ecx\n");
		emit("pop ecx\n");
		emit("ret\n");
	} else {
		/* For future expansion... */
		/* need an extra jump table here */
	}
}

/***************************************************************************/

/*
** Perform a cached rebase
*/
void perform_cached_rebase(void) {
	int myline = linenum; linenum += 2;
	emit("cmp esi,[__fetch_region_start]\n");
	emit("jb short ln%d\n", myline);
	emit("cmp esi,[__fetch_region_end]\n");
	emit("jbe short ln%d\n", myline + 1);
	emit("ln%d:\n", myline);
	emit("call basefunction\n");
	emit("ln%d:\n", myline + 1);
	emit("add esi,ebp\n");
}

/***************************************************************************/

/*
** This is the function that generates a base for a given 68K PC.
**
**  Entry: 68K PC in ESI
**  Exit:  Newly calculated base in EBP
**
** Sounds like a simple lookup into the __fetch array, and in the case of
** 32-bit addresses, it is.  But for anything less, we'll need to compensate
** for potential garbage in the unused address bits, by subtracting the value
** of these unused bits from the base.  This way the full 32 bits of the PC
** are preserved, even if they're not used.
**
** The only registers which need to be "live" here are ESI and EBP.  The
** fetch region cache is updated, and bit 1 of __execinfo is set if the base
** couldn't be calculated.
*/
static void gen_basefunction(void) {
	align(16);
	emit("basefunction:\n");
	/*
	** Prepare ESI by masking off unused address bits (but save it
	** first).
	*/
	if(addressbits < 32) {
		emit("push esi\n");
		maskaddress("esi");
	}
	emit("mov ebp,[__fetch]\n");
	emit(".check:\n");
	//emit("db 3Eh\n");
	emit("cmp esi,[ebp]\n");           /* Are we smaller? */
	emit("jb short .next\n");          /* Yes, go to next address */
	//emit("db 3Eh\n");
	emit("cmp esi,[ebp+4]\n");         /* Are we bigger? */
	emit("jbe short .base\n");
	emit(".next:\n");
	//emit("db 3Eh\n");
	emit("cmp dword [ebp],byte -1\n"); /* End of list? */
	emit("je short .outofrange\n");
	emit("add ebp,byte 12\n");         /* To the next structure */
	emit("jmp short .check\n");
	/* Bad news... we jumped out into the weeds. */
	emit(".outofrange:\n");
	if(addressbits < 32) emit("pop esi\n");
	emit("xor ebp,ebp\n");
	emit("mov dword[__fetch_region_start],-1\n");
	emit("mov dword[__fetch_region_end],ebp\n");
	force_context_switch();
	emit("or byte[__execinfo],2\n");
	emit("ret\n");

	emit(".base:\n");
	/*
	** Dealing with addressbits < 32 again... if the unused PC bits are
	** anything but zero, then we'll need to adjust the base to
	** compensate.
	*/
	if(addressbits < 32) {
		emit("mov esi,[esp]\n");
		emit("and esi,%d\n", 0xFFFFFFFF << addressbits);
	}
	emit("push edx\n");
	emit("mov edx,ebp\n");
	/*
	** Update the fetch region cache, adding in the garbage bits where
	** applicable.
	*/
	emit("mov ebp,[edx]\n");
	if(addressbits < 32) emit("or ebp,esi\n");
	emit("mov [__fetch_region_start],ebp\n");
	emit("mov ebp,[edx+4]\n");
	if(addressbits < 32) emit("or ebp,esi\n");
	emit("mov [__fetch_region_end],ebp\n");
	emit("mov ebp,[edx+8]\n");
	emit("pop edx\n");
	if(addressbits < 32) {
		/*
		** Subtract garbage bits from the base, and restore the
		** original 32-bit PC value.
		*/
		emit("sub ebp,esi\n");
		emit("pop esi\n");
	}
	emit("ret\n");
}

/***************************************************************************/

/* Read flags from CL into our CCR.  CX is unmodified. */
static void cl2ccr(void){
	emit("mov al,cl\n");        /* read CCR -> AL */                 /* ???????????XNZVC */
	emit("mov ah,al\n");        /* copy to AH */                     /* ???XNZVC???XNZVC */
	emit("and ax,0C10h\n");     /* isolate NZ...X */                 /* 0000NZ00000X0000 */
	emit("shl ah,3\n");       /* put NZ almost where we want it */   /* 0NZ00000000X0000 */
	emit("shr al,4\n");         /* shift X flag into bit 0 */        /* 0NZ000000000000X */
	emit("mov [__xflag],al\n"); /* store X flag */                   /* 0NZ000000000000X al -> xflag */
	emit("mov al,cl\n");        /* read CCR -> AL again */           /* 0NZ00000000XNZVC */
	emit("and al,3\n");         /* isolate VC */                     /* 0NZ00000000000VC */
	emit("shr al,1\n");         /* just V */                         /* 0NZ000000000000V carry */
	emit("adc ah,ah\n");        /* append C to rest of flags */      /* NZ00000C0000000V */
}

/*
** Read flags from CX into our SR, performing a mode switch where applicable.
** CX is unmodified.  Uses 4 bytes of stack.
**
** This does not do any of the trace flag mojo, so be sure to check for it
** explicitly where applicable (hint: ret_timing_checkpoint).
*/
static void cx2sr(void){
	int myline = linenum; linenum += 2;

	emit("push ecx\n");
	/* Step 1: switch supervisor mode */
	/* Is the new mode different from the last? */
	emit("mov cl,[__sr+1]\n");
	emit("and cx,2020h\n");
	emit("xor ch,cl\n");
	emit("jz near ln%d\n", myline);
	/* If so, swap stack pointers */
	emit("mov ecx,[__a7]\n");
	emit("xchg ecx,[__asp]\n");
	emit("mov [__a7],ecx\n");
	/* and copy the appropriate memory map */
	emit("test byte[esp+1],20h\n");
	emit("jz short ln%d\n", myline + 1);
	copy_memory_map("s", "ecx");
	emit("jmp short ln%d\n", myline);
	emit("ln%d:\n", myline + 1);
	copy_memory_map("u", "ecx");
	emit("ln%d:\n", myline);
	emit("pop ecx\n");

	/* Step 2: set new PPL / supervisor mode / trace flag */
	emit("mov [__sr+1],ch\n");
	emit("and byte[__sr+1],0A7h\n");

	/* Step 3: Store CL into CCR */
	cl2ccr();
}

/* Read flags from our CCR into CL.  CH is zeroed. */
static void ccr2cl(void){
	/* Read XNZ */
	emit("mov ch,[__xflag]\n");  /* 0000000X???????? */
	emit("mov cl,ah\n");         /* 0000000XNZ?????C */
	emit("shr cx,6\n");          /* 0000000000000XNZ */
	/* Read V */
	emit("add cl,cl\n");         /* 000000000000XNZ0 */
	emit("or cl,al\n");          /* 000000000000XNZV */
	/* Read C */
	emit("mov ch,ah\n");         /* NZ?????C0000XNZV */
	emit("shl ch,8\n");          /* 000000000000XNZV carry */
	emit("adc cl,cl\n");         /* 00000000000XNZVC */
}

/* Read flags from our SR into CX. */
static void sr2cx(void){
	/* Condition codes */
	ccr2cl();
	/* PPL / supervisor mode / trace flag */
	emit("mov ch,[__sr+1]\n");
}

/* Switch to supervisor mode.  Can potentially destroy ECX. */
static void supervisor(void){
	int myline=linenum;linenum++;
	emit("test byte[__sr+1],20h\n");
	emit("jnz short ln%d\n",myline);
	emit("mov ecx,[__a7]\n");
	emit("xchg ecx,[__asp]\n");
	emit("mov [__a7],ecx\n");
	copy_memory_map("s", "ecx");
	emit("or byte[__sr+1],20h\n");
	emit("ln%d:\n",myline);
}

/***************************************************************************/

static void gen_readbw(int size)
{
	align(32);
	emit("readmemory%s:\n",sizename[size]);
	emit("readmemorydec%s:\n",sizename[size]);

	if (size == 1)
	{
		emit("\tmov [__access_address], edx\n");
		emit("\tand edx, 0xFFFFFF\n");
		emit("\txor ecx, ecx\n");

		emit("\tcmp edx, 0xE00000\n");
		emit("\tjb short .Not_In_Ram\n");
		emit("\tand edx, 0xFFFF\n");
		emit("\txor edx, byte 1\n");
		emit("\tmov cl, [Ram_68k + edx]\n");
		emit("\tmov edx, [__access_address]\n");
		
// TODO: Port from Gens Rerecording
#if 0
		emit("pushad\n");
		emit("sub esi,ebp\n");
		emit("sub esi,byte 2\n");
		emit("mov [_hook_pc],esi\n");
		emit("mov [_hook_address],edx\n");
		emit("mov [_hook_value],ecx\n");
		emit("call _hook_read_byte\n");
		emit("popad\n");
#endif	
		emit("\tret\n");

		emit("align 4\n");
		emit(".Not_In_Ram\n");
		emit("\tpush eax\n");
		emit("\tpush edx\n");
		emit("\tmov [__io_cycle_counter], edi\n");
		emit("\tmov [__io_fetchbase], ebp\n");
		emit("\tmov [__io_fetchbased_pc], esi\n");
		emit("\tcall M68K_RB\n");

//	emit("\tmov eax, [esp]\n");
//	emit("\tpush ebx\n");
//	emit("\tmov ebx, eax\n");
//	emit("\tand eax, 0xF80000\n");
//	emit("\tshr eax, 17\n");
//	emit("\tand ebx, 0xFFFFFF\n");
//	emit("\tcall [M68K_Read_Byte_Table + eax]\n");

		emit("\tmov ebp, [__io_fetchbase]\n");
		emit("\tmov cl, al\n");
		emit("\tmov edi, [__io_cycle_counter]\n");
		emit("\tadd esp, byte 4\n");
		emit("\tmov esi, [__io_fetchbased_pc]\n");
		emit("\tmov edx, [__access_address]\n");
		emit("\tpop eax\n");
		
// TODO: Port from Gens Rerecording
#if 0
		emit("pushad\n");
		emit("sub esi,ebp\n");
		emit("sub esi,byte 2\n");
		emit("mov [_hook_pc],esi\n");
		emit("mov [_hook_address],edx\n");
		emit("mov [_hook_value],ecx\n");
		emit("call _hook_read_byte\n");
		emit("popad\n");
#endif
		
		emit("\tret\n");
	}

	if (size == 2)
	{
		emit("\tmov [__access_address], edx\n");
		emit("\tand edx, 0xFFFFFF\n");
		emit("\txor ecx, ecx\n");

		emit("\tcmp edx, 0xE00000\n");
		emit("\tjb short .Not_In_Ram\n");
		emit("\tand edx, 0xFFFF\n");
		emit("\tmov cx, [Ram_68k + edx]\n");
		emit("\tmov edx,[__access_address]\n");
		
// TODO: Port from Gens Rerecording
#if 0
		emit("pushad\n");
		emit("sub esi,ebp\n");
		emit("sub esi,byte 2\n");
		emit("mov [_hook_pc],esi\n");
		emit("mov [_hook_address],edx\n");
		emit("mov [_hook_value],ecx\n");
		emit("call _hook_read_word\n");
		emit("popad\n");
#endif
		
		emit("\tret\n");

		emit("align 4\n");
		emit(".Not_In_Ram\n");
		emit("\tpush eax\n");
		emit("\tpush edx\n");
		emit("\tmov [__io_cycle_counter], edi\n");
		emit("\tmov [__io_fetchbase], ebp\n");
		emit("\tmov [__io_fetchbased_pc], esi\n");
		emit("\tcall M68K_RW\n");
		emit("\tmov edi, [__io_cycle_counter]\n");
		emit("\tadd esp, byte 4\n");
		emit("\tmov ebp, [__io_fetchbase]\n");
		emit("\tmov cx, ax\n");
		emit("\tmov esi, [__io_fetchbased_pc]\n");
		emit("\tmov edx, [__access_address]\n");
		emit("\tpop eax\n");
			
// TODO: Port from Gens Rerecording
#if 0		
		emit("pushad\n");
		emit("sub esi,ebp\n");
		emit("sub esi,byte 2\n");
		emit("mov [_hook_pc],esi\n");
		emit("mov [_hook_address],edx\n");
		emit("mov [_hook_value],ecx\n");
		emit("call _hook_read_word\n");
		emit("popad\n");
#endif
		
		emit("\tret\n");
	}
}

static void gen_readl(void)
{
	align(32);
	emit("readmemory%s:\n",sizename[4]);

	emit("\tmov [__access_address], edx\n");
	emit("\tand edx, 0xFFFFFF\n");

	emit("\tcmp edx, 0xE00000\n");
	emit("\tjb short .Not_In_Ram\n");
	emit("\tand edx, 0xFFFF\n");
	emit("\tmov ecx, [Ram_68k + edx]\n");
	emit("\trol ecx, 16\n");
	emit("\tmov edx, [__access_address]\n");
	
// TODO: Port from Gens Rerecording
#if 0
	emit("pushad\n");
	emit("sub esi,ebp\n");
	emit("sub esi,byte 2\n");
	emit("mov [_hook_pc],esi\n");
	emit("mov [_hook_address],edx\n");
	emit("mov [_hook_value],ecx\n");
	emit("call _hook_read_dword\n");
	emit("popad\n");
#endif
	
	emit("\tret\n");

	emit("align 4\n");
	emit(".Not_In_Ram\n");
	emit("\tpush eax\n");
	emit("\tpush edx\n");
	emit("\tmov [__io_cycle_counter], edi\n");
	emit("\tmov [__io_fetchbase], ebp\n");
	emit("\tmov [__io_fetchbased_pc], esi\n");
	emit("\tcall M68K_RW\n");
	emit("\tmov cx, ax\n");
	emit("\tadd dword [esp], byte 2\n");
	emit("\tshl ecx, 16\n");
	emit("\tcall M68K_RW\n");
	emit("\tmov edi, [__io_cycle_counter]\n");
	emit("\tadd esp, byte 4\n");
	emit("\tmov ebp, [__io_fetchbase]\n");
	emit("\tmov cx, ax\n");
	emit("\tmov esi, [__io_fetchbased_pc]\n");
	emit("\tpop eax\n");
	emit("\tmov edx, [__access_address]\n");
	
// TODO: Port from Gens Rerecording
#if 0
	emit("pushad\n");
	emit("sub esi,ebp\n");
	emit("sub esi,byte 2\n");
	emit("mov [_hook_pc],esi\n");
	emit("mov [_hook_address],edx\n");
	emit("mov [_hook_value],ecx\n");
	emit("call _hook_read_dword\n");
	emit("popad\n");
#endif
	
	emit("\tret\n");
}

static void gen_readdecl(void)
{
	align(32);
	emit("readmemorydec%s:\n",sizename[4]);

	emit("\tmov [__access_address], edx\n");
	emit("\tand edx, 0xFFFFFF\n");

	emit("\tcmp edx, 0xE00000\n");
	emit("\tjb short .Not_In_Ram\n");
	emit("\tand edx, 0xFFFF\n");
	emit("\tmov ecx, [Ram_68k + edx]\n");
	emit("\trol ecx, 16\n");
	emit("\tmov edx, [__access_address]\n");

// TODO: Port from Gens Rerecording
#if 0
	emit("pushad\n");
	emit("sub esi,ebp\n");
	emit("sub esi,byte 2\n");
	emit("mov [_hook_pc],esi\n");
	emit("mov [_hook_address],edx\n");
	emit("mov [_hook_value],ecx\n");
	emit("call _hook_read_dword\n");
	emit("popad\n");
#endif

	emit("\tret\n");

	emit("align 4\n");
	emit(".Not_In_Ram\n");
	emit("\tadd edx, byte 2\n");
	emit("\tpush eax\n");
	emit("\tpush edx\n");
	emit("\tmov [__io_cycle_counter], edi\n");
	emit("\tmov [__io_fetchbase], ebp\n");
	emit("\tmov [__io_fetchbased_pc], esi\n");
	emit("\tcall M68K_RW\n");
	emit("\tmov ecx, eax\n");
	emit("\tsub dword [esp], byte 2\n");
	emit("\tand ecx, 0xFFFF\n");
	emit("\tcall M68K_RW\n");
	emit("\tand eax, 0xFFFF\n");
	emit("\tmov edi, [__io_cycle_counter]\n");
	emit("\tshl eax, 16\n");
	emit("\tadd esp, byte 4\n");
	emit("\tmov ebp, [__io_fetchbase]\n");
	emit("\tor ecx, eax\n");
	emit("\tmov esi, [__io_fetchbased_pc]\n");
	emit("\tpop eax\n");
	emit("\tmov edx, [__access_address]\n");

// TODO: Port from Gens Rerecording
#if 0
	emit("pushad\n");
	emit("sub esi,ebp\n");
	emit("sub esi,byte 2\n");
	emit("mov [_hook_pc],esi\n");
	emit("mov [_hook_address],edx\n");
	emit("mov [_hook_value],ecx\n");
	emit("call _hook_read_dword\n");
	emit("popad\n");
#endif
	
	emit("\tret\n");
}

static void gen_writebw(int size)
{
	align(32);
	emit("writememory%s:\n",sizename[size]);
	emit("writememorydec%s:\n",sizename[size]);

	if (size == 1)
	{
		emit("\tmov [__access_address], edx\n");
		emit("\tand edx, 0xFFFFFF\n");

// TODO: Port from Gens Rerecording
#if 0
		emit("pushad\n");
		emit("sub esi,ebp\n");
		emit("sub esi,byte 2\n");
		emit("mov [_hook_pc],esi\n");
		emit("mov [_hook_address],edx\n");
		emit("mov [_hook_value],ecx\n");
		emit("call _hook_write_byte\n");
		emit("popad\n");
#endif
		
		emit("\tcmp edx, 0xE00000\n");
		emit("\tjb short .Not_In_Ram\n");
		emit("\txor edx, 1\n");
		emit("\tand edx, 0xFFFF\n");
		emit("\tmov [Ram_68k + edx], cl\n");
		emit("\tmov edx, [__access_address]\n");

// From Gens Rerecording, but was already commented out.
//		emit("pushad\n");
//		emit("\tpush dword 1\n");
//		emit("\tand edx,0xFFFFFF\n");
//		emit("\tpush edx\n");
//		emit("\tcall _Fix_Codes\n");
//		emit("\tadd esp, byte 8\n");
//		emit("popad\n");
				
		emit("\tret\n");

		emit("align 4\n");
		emit(".Not_In_Ram\n");
		emit("\tpush eax\n");
		emit("\tpush ecx\n");
		emit("\tpush edx\n");
		emit("\tmov [__io_cycle_counter], edi\n");
		emit("\tmov [__io_fetchbase], ebp\n");
		emit("\tmov [__io_fetchbased_pc], esi\n");
		emit("\tcall M68K_WB\n");
		emit("\tmov edi, [__io_cycle_counter]\n");
		emit("\tadd esp, byte 8\n");
		emit("\tmov ebp, [__io_fetchbase]\n");
		emit("\tmov esi, [__io_fetchbased_pc]\n");
		emit("\tpop eax\n");
		emit("\tmov edx, [__access_address]\n");
	
		emit("\tret\n");
	}

	if (size == 2)
	{
		emit("\tmov [__access_address], edx\n");
		emit("\tand edx, 0xFFFFFF\n");

// TODO: Port from Gens Rerecording
#if 0
		emit("pushad\n");
		emit("sub esi,ebp\n");
		emit("sub esi,byte 2\n");
		emit("mov [_hook_pc],esi\n");
		emit("mov [_hook_address],edx\n");
		emit("mov [_hook_value],ecx\n");
		emit("call _hook_write_word\n");
		emit("popad\n");
#endif

		emit("\tcmp edx, 0xE00000\n");
		emit("\tjb short .Not_In_Ram\n");
		emit("\tand edx, 0xFFFF\n");
		emit("\tmov [Ram_68k + edx], cx\n");
		emit("\tmov edx, [__access_address]\n");

// From Gens Rerecording, but was already commented out.
//		emit("pushad\n");
//		emit("\tpush dword 2\n");
//		emit("\tand edx,0xFFFFFF\n");
//		emit("\tpush edx\n");
//		emit("\tcall _Fix_Codes\n");
//		emit("\tadd esp, byte 8\n");
//		emit("popad\n");
			
		emit("\tret\n");

		emit("align 4\n");
		emit(".Not_In_Ram\n");
		emit("\tpush eax\n");
		emit("\tpush ecx\n");
		emit("\tpush edx\n");
		emit("\tmov [__io_cycle_counter], edi\n");
		emit("\tmov [__io_fetchbase], ebp\n");
		emit("\tmov [__io_fetchbased_pc], esi\n");
		emit("\tcall M68K_WW\n");
		emit("\tmov edi, [__io_cycle_counter]\n");
		emit("\tadd esp, byte 8\n");
		emit("\tmov ebp, [__io_fetchbase]\n");
		emit("\tmov esi, [__io_fetchbased_pc]\n");
		emit("\tpop eax\n");
		emit("\tmov edx, [__access_address]\n");
		
		emit("\tret\n");
	}
}

static void gen_writel(void)
{
	align(32);
	emit("writememory%s:\n",sizename[4]);

	emit("\tmov [__access_address], edx\n");
	emit("\tand edx, 0xFFFFFF\n");

// TODO: Port from Gens Rerecording
#if 0
	emit("pushad\n");
	emit("sub esi,ebp\n");
	emit("sub esi,byte 2\n");
	emit("mov [_hook_pc],esi\n");
	emit("mov [_hook_address],edx\n");
	emit("mov [_hook_value],ecx\n");
	emit("call _hook_write_dword\n");
	emit("popad\n");
#endif
	
	emit("\trol ecx, 16\n");
	emit("\tcmp edx, 0xE00000\n");
	emit("\tjb short .Not_In_Ram\n");
	emit("\tand edx, 0xFFFF\n");
	emit("\tmov [Ram_68k + edx], ecx\n");
	emit("\tmov edx, [__access_address]\n");
	emit("\trol ecx, 16\n");

// From Gens Rerecording, but was already commented out.
//	emit("pushad\n");
//	emit("\tpush dword 4\n");
//	emit("\tand edx,0xFFFFFF\n");
//	emit("\tpush edx\n");
//	emit("\tcall _Fix_Codes\n");
//	emit("\tadd esp, byte 8\n");
//	emit("popad\n");
		
	emit("\tret\n");

	emit("align 4\n");
	emit(".Not_In_Ram\n");
	emit("\tpush eax\n");
	emit("\tpush ecx\n");
	emit("\tpush edx\n");
	emit("\tmov [__io_cycle_counter], edi\n");
	emit("\tmov [__io_fetchbase], ebp\n");
	emit("\trol ecx, 16\n");
	emit("\tmov [__io_fetchbased_pc], esi\n");
	emit("\tcall M68K_WW\n");
	emit("\tmov [esp + 4], cx\n");
	emit("\tadd dword [esp], byte 2\n");
	emit("\tcall M68K_WW\n");
	emit("\tmov edi, [__io_cycle_counter]\n");
	emit("\tadd esp, byte 8\n");
	emit("\tmov ebp, [__io_fetchbase]\n");
	emit("\tmov esi, [__io_fetchbased_pc]\n");
	emit("\tpop eax\n");
	emit("\tmov edx, [__access_address]\n");
	
	emit("\tret\n");
}

static void gen_writedecl(void)
{
	align(32);
	emit("writememorydec%s:\n",sizename[4]);

	emit("\tmov [__access_address], edx\n");
	emit("\tand edx, 0xFFFFFF\n");

// TODO: Port from Gens Rerecording
#if 0
	emit("pushad\n");
	emit("sub esi,ebp\n");
	emit("sub esi,byte 2\n");
	emit("mov [_hook_pc],esi\n");
	emit("mov [_hook_address],edx\n");
	emit("mov [_hook_value],ecx\n");
	emit("call _hook_write_dword\n");
	emit("popad\n");
#endif
	
	emit("\tcmp edx, 0xE00000\n");
	emit("\tjb short .Not_In_Ram\n");
	emit("\trol ecx, 16\n");
	emit("\tand edx, 0xFFFF\n");
	emit("\tmov [Ram_68k + edx], ecx\n");
	emit("\tmov edx, [__access_address]\n");
	emit("\trol ecx, 16\n");

// From Gens Rerecording, but was already commented out.
//	emit("pushad\n");
//	emit("\tpush dword 4\n");
//	emit("\tand edx,0xFFFFFF\n");
//	emit("\tpush edx\n");
//	emit("\tcall _Fix_Codes\n");
//	emit("\tadd esp, byte 8\n");
//	emit("popad\n");
	
	emit("\tret\n");

	emit("align 4\n");
	emit(".Not_In_Ram\n");
	emit("\tadd edx, byte 2\n");
	emit("\tpush eax\n");
	emit("\tpush ecx\n");
	emit("\tpush edx\n");
	emit("\tmov [__io_cycle_counter], edi\n");
	emit("\tmov [__io_fetchbase], ebp\n");
	emit("\trol ecx, 16\n");
	emit("\tmov [__io_fetchbased_pc], esi\n");
	emit("\tcall M68K_WW\n");
	emit("\tmov [esp + 4], ecx\n");
	emit("\tsub dword [esp], byte 2\n");
	emit("\tcall M68K_WW\n");
	emit("\tmov edi, [__io_cycle_counter]\n");
	emit("\tadd esp, byte 8\n");
	emit("\tmov ebp, [__io_fetchbase]\n");
	emit("\tmov esi, [__io_fetchbased_pc]\n");
	emit("\tpop eax\n");
	emit("\tmov edx, [__access_address]\n");
	
	emit("\tret\n");
}



/***************************************************************************/
/*
** Group 1 and 2 exceptions
** Exception address is passed in EDX
**
** Does not fix the new PC!
*/
static void gen_group_12_exception(void) {
	align(16);
	emit("group_1_exception:\n");
	emit("group_2_exception:\n");
	emit("and byte[__interrupts],0EFh\n"); /* first thing's first */
	if(cputype == 68010) {
		emit("mov byte[__loopmode],0\n");
	}
	if(cputype >= 68010) {
		emit("push edx\n");
		emit("add edx,[__vbr]\n");
	}
	emit("call readmemorydword\n");
	if(cputype >= 68010) {
		emit("pop edx\n");
	}

	emit("push ecx\n");/* dest. PC */
	sr2cx();
	emit("push ecx\n");/* old SR */
	supervisor();
	/*
	** Exception handlers do not like being traced, so clear the SR trace
	** flag as well as the trace tricky bit.
	**
	** Leave the cycle leftover count alone, in case we still need to
	** call attention to other unrelated tricky bits.
	*/
	emit("and byte[__sr+1],27h\n");
	emit("mov byte[__trace_trickybit],0\n");

	emit("mov ecx,esi\n");
	emit("sub ecx,ebp\n");
	if(cputype >= 68010) {
		emit("push ecx\n");/* old PC */
		emit("mov ecx,edx\n");
	}
	emit("mov edx,[__a7]\n");
	if(cputype >= 68010) {
		emit("and ecx,0FFCh\n");/* Format code */
		emit("sub edx,byte 2\n");
		emit("call writememoryword\n");
		emit("pop ecx\n");
	}
	emit("sub edx,byte 4\n");
	emit("call writememorydword\n");
	emit("pop ecx\n");/* old SR */
	emit("sub edx,byte 2\n");
	emit("call writememoryword\n");
	emit("mov [__a7],edx\n");
	emit("pop esi\n");/* dest. PC */
	emit("ret\n");
}

/***************************************************************************/

/* Privilege violation */
static void gen_privilege_violation(void){
	align(16);
	emit("privilege_violation:\n");
	emit("sub esi,byte 2\n");
	emit("mov edx,20h\n");
	emit("call group_1_exception\n");
	perform_cached_rebase();
	ret_timing((cputype==68010)?38:34);
}

/***************************************************************************/

static void usereg(void) {
	emit("and ebx,byte 7\n");
}

/* usereg only where applicable */
static void selective_usereg(void) {
	switch(main_eamode) {
	case dreg: case areg:
	case aind: case ainc: case adec:
	case adsp: case axdp:
		usereg();
	default:;
	}
}

static void selftest(int size) {
	emit("test %s,%s\n", x86cx[size], x86cx[size]);
}

/***************************************************************************/

/* Get condition: Less Than (N^V)
** If true, the x86 sign flag will be set */
static void getcondition_l_s_ns(void) {
	emit("push eax\n");
	emit("neg al\n");
	emit("xor al,ah\n");
	emit("pop eax\n");
}

/* Get condition: Less Than or Equal ((N^V)|Z)
** If true, the x86 sign flag will be set */
static void getcondition_le_s_ns(void) {
	emit("push eax\n");
	emit("neg al\n");
	emit("xor al,ah\n");
	emit("add ah,ah\n");
	emit("or al,ah\n");
	emit("pop eax\n");
}

static char optcc[5];
static char optrc[5];

static void getcondition(int cc) {
	switch(cc) {
	case 0x0:
	case 0x1:
		break;
	case 0x2:/* a */
		emit("test ah,41h\n");
		sprintf(optcc, "z");
		sprintf(optrc, "nz");
		break;
	case 0x3:/* be */
		emit("test ah,41h\n");
		sprintf(optcc, "nz");
		sprintf(optrc, "z");
		break;
	case 0x4:/* nc */
		emit("test ah,1\n");
		sprintf(optcc, "z");
		sprintf(optrc, "nz");
		break;
	case 0x5:/* c */
		emit("test ah,1\n");
		sprintf(optcc, "nz");
		sprintf(optrc, "z");
		break;
	case 0x6:/* ne */
		emit("test ah,40h\n");
		sprintf(optcc, "z");
		sprintf(optrc, "nz");
		break;
	case 0x7:/* e */
		emit("test ah,40h\n");
		sprintf(optcc, "nz");
		sprintf(optrc, "z");
		break;
	case 0x8:/* no */
		emit("test al,1\n");
		sprintf(optcc, "z");
		sprintf(optrc, "nz");
		break;
	case 0x9:/* o */
		emit("test al,1\n");
		sprintf(optcc, "nz");
		sprintf(optrc, "z");
		break;
	case 0xA:/* ns */
		emit("or ah,ah\n");
		sprintf(optcc, "ns");
		sprintf(optrc, "s");
		break;
	case 0xB:/* s */
		emit("or ah,ah\n");
		sprintf(optcc, "s");
		sprintf(optrc, "ns");
		break;
	case 0xC:/* ge */
		getcondition_l_s_ns();
		sprintf(optcc, "ns");
		sprintf(optrc, "s");
		break;
	case 0xD:/* l */
		getcondition_l_s_ns();
		sprintf(optcc, "s");
		sprintf(optrc, "ns");
		break;
	case 0xE:/* g */
		getcondition_le_s_ns();
		sprintf(optcc, "ns");
		sprintf(optrc, "s");
		break;
	case 0xF:/* le */
		getcondition_le_s_ns();
		sprintf(optcc, "s");
		sprintf(optrc, "ns");
		break;
	default:break;
	}
}

static void flags(void) {
	emit("lahf\n");
	emit("seto al\n");
}

static void flags_v0(void) {
	emit("lahf\n");
	emit("mov al,0\n");
}

/* Put one of the x86 flags into the 68K zero flag. */
static void flag_to_z(char *f) {
	int myline = linenum; linenum += 2;
	emit("j%s short ln%d\n", f, myline);
	emit("and ah,0BFh\n");
	emit("jmp short ln%d\n", myline + 1);
	emit("ln%d:\n", myline);
	emit("or ah,40h\n");
	emit("ln%d:\n", myline + 1);
}

/* carry to X flag */
static void c2x(void) {
	emit("setc [__xflag]\n");
}

/* with previous flags in another register, adjust for non-changing zero */
static void adjzero(char *reg) {
	int myline = linenum; linenum++;
	emit("jnz short ln%d\n", myline);
	emit("or %s,0BFh\n", reg);
	emit("and ah,%s\n", reg);
	emit("ln%d:\n", myline);
}

/* Check for privilege violation */
static void privilegecheck(void) {
	emit("test byte[__sr+1],20h\n");
	emit("jz near privilege_violation\n");
}

/****************************************************************************
** EFFECTIVE ADDRESS GENERATION
****************************************************************************/

/*
** There are five types of EA activity:
**
** 1. Read:       precalc -> read -> postcalc
** 2. Write:      precalc -> write -> postcalc
** 3. R-M-W:      precalc -> read -> (modify) -> write -> postcalc
** 4. Move:       Read followed by Write
** 5. Control:    precalc
*/

/* Calculate address */
static void ea_step_precalc(int size, enum eamode mode, int reg) {
	char regs[100];
	if(reg == -1) sprintf(regs, "ebx*4");
	else sprintf(regs, "%d", reg * 4);
	switch(mode) {
	case dreg: case areg:
		break;
	case aind: case ainc: case adec:
		emit("mov edx,[__areg+%s]\n",regs);
		if(mode == adec) {
			/* Compensate for byte-sized stack ops */
			if(size == 1) {
				if(reg == -1) {
					emit("cmp bl,7\n");
					emit("adc edx,byte -2\n");
				} else if(reg == 7) {
					emit("sub edx,byte 2\n");
				} else {
					emit("dec edx\n");
				}
			} else {
				emit("sub edx,byte %d\n", size);
			}
		}
		break;
	case adsp:
		emit("movsx edx,word[esi]\n");
		emit("add esi,byte 2\n");
		emit("add edx,[__areg+%s]\n", regs);
		break;
	case axdp:
		emit("call decode_ext\n");
		emit("add edx,[__areg+%s]\n", regs);
		break;
	case absw:
		emit("movsx edx,word[esi]\n");
		emit("add esi,byte 2\n");
		break;
	case absl:
		emit("mov edx,dword[esi]\n");
		emit("add esi,byte 4\n");
		emit("rol edx,16\n");
		break;
	case pcdp:
		emit("movsx edx,word[esi]\n");
		emit("add edx,esi\n");
		emit("sub edx,ebp\n");
		emit("add esi,byte 2\n");
		break;
	case pcxd:
		emit("call decode_ext\n");
		emit("add edx,esi\n");
		emit("sub edx,ebp\n");
		emit("sub edx,byte 2\n");
		break;
	case immd:
		break;
	default:
		emit("#error ea_step_precalc\n");
		break;
	}
}

static void ea_step_read(int size, enum eamode mode, int reg) {
	char regs[100];
	if(reg == -1) sprintf(regs, "ebx*4");
	else sprintf(regs, "%d", reg * 4);
	switch(mode) {
	case dreg: emit("mov ecx,[__dreg+%s]\n", regs); break;
	case areg: emit("mov ecx,[__areg+%s]\n", regs); break;
	case adec:
		emit("call readmemorydec%s\n", sizename[size]);
		break;
	case aind: case ainc:
	case adsp: case axdp:
	case absw: case absl:
	case pcdp: case pcxd:
		emit("call readmemory%s\n", sizename[size]);
		break;
	case immd:
		switch(size) {
		case 1:
		case 2:
			emit("mov cx,[esi]\n");
			emit("add esi,byte 2\n");
			break;
		case 4:
			emit("mov ecx,[esi]\n");
			emit("add esi,byte 4\n");
			emit("rol ecx,16\n");
			break;
		default:
			emit("#error ea_step_read\n");
			break;
		}
		break;
	default:
		emit("#error ea_step_read\n");
		break;
	}
}

/*
** Special case for when you need to load a word and sign-extend it.
** This cuts some fat out of a few instructions (i.e. MOVEA).
*/
static void ea_step_read_signword(enum eamode mode, int reg) {
	char regs[100];
	if(reg == -1) sprintf(regs, "ebx*4");
	else sprintf(regs, "%d", reg * 4);
	switch(mode) {
	case dreg: emit("movsx ecx,word[__dreg+%s]\n", regs); break;
	case areg: emit("movsx ecx,word[__areg+%s]\n", regs); break;
	case aind: case ainc: case adec:
	case adsp: case axdp:
	case absw: case absl:
	case pcdp: case pcxd:
		emit("call readmemory%s\n", sizename[2]);
		emit("movsx ecx,cx\n");
		break;
	case immd:
		emit("movsx ecx,word[esi]\n");
		emit("add esi,byte 2\n");
		break;
	default:
		emit("#error ea_step_read_signword\n");
		break;
	}
}

static void ea_step_write(int size, enum eamode mode, int reg) {
	char regs[100];
	if(reg == -1) sprintf(regs, "ebx*4");
	else sprintf(regs, "%d", reg * 4);
	switch(mode) {
	case dreg:
		emit("mov [__dreg+%s],%s\n", regs, x86cx[size]);
		break;
	case adec:
		emit("call writememorydec%s\n", sizename[size]);
		break;
	case aind: case ainc:
	case adsp: case axdp:
	case absw: case absl:
		emit("call writememory%s\n", sizename[size]);
		break;
	default:
		emit("#error ea_step_write\n");
		break;
	}
}

static void ea_step_postcalc(int size, enum eamode mode, int reg) {
	char regs[100];
	if(reg == -1) sprintf(regs, "ebx*4");
	else sprintf(regs, "%d", reg * 4);
	switch(mode) {
	case ainc:
		/* Compensate for byte-sized stack ops */
		if(size == 1) {
			if(reg == -1) {
				emit("cmp bl,7\n");
				emit("sbb edx,byte -2\n");
			} else if(reg == 7) {
				emit("add edx,byte 2\n");
			} else {
				emit("inc edx\n");
			}
		} else {
			emit("add edx,byte %d\n", size);
		}
		/* Fall through */
	case adec:
		/* Store already-predecremented address */
		emit("mov [__areg+%s],edx\n", regs);
		break;
	case dreg: case areg:
	case aind: case adsp: case axdp:
	case absw: case absl:
	case pcdp: case pcxd:
	case immd:
		break;
	default:
		emit("#error ea_step_postcalc\n");
		break;
	}
}

/* Combined EA routines */

static void ea_load(int size, enum eamode mode, int reg) {
	ea_step_precalc (size, mode, reg);
	ea_step_read    (size, mode, reg);
	ea_step_postcalc(size, mode, reg);
}

static void ea_load_signword(enum eamode mode, int reg) {
	ea_step_precalc      (2, mode, reg);
	ea_step_read_signword(   mode, reg);
	ea_step_postcalc     (2, mode, reg);
}

static void ea_store(int size, enum eamode mode, int reg) {
	ea_step_precalc (size, mode, reg);
	ea_step_write   (size, mode, reg);
	ea_step_postcalc(size, mode, reg);
}

static void ea_rmw_load(int size, enum eamode mode, int reg) {
	ea_step_precalc (size, mode, reg);
	ea_step_read    (size, mode, reg);
}

static void ea_rmw_store(int size, enum eamode mode, int reg) {
	ea_step_write   (size, mode, reg);
	ea_step_postcalc(size, mode, reg);
}

static void ea_control(enum eamode mode, int reg) {
	ea_step_precalc (0, mode, reg);
}

static void main_ea_load(void) {
	ea_load(main_size, main_eamode, -1);
}

static void main_ea_load_signed(void) {
	if(main_size < 4) {
		ea_load_signword(main_eamode, -1);
	} else {
		ea_load(main_size, main_eamode, -1);
	}
}

static void main_ea_store(void) {
	ea_store(main_size, main_eamode, -1);
}

static void main_ea_rmw_load(void) {
	ea_rmw_load(main_size, main_eamode, -1);
}

static void main_ea_rmw_store(void) {
	ea_rmw_store(main_size, main_eamode, -1);
}

static void main_ea_control(void) {
	ea_control(main_eamode, -1);
}

/***************************************************************************/
/*
** Calculate cycles for main EA mode
** (68000, 68010)
*/
static int main_ea_cycles(void) {
	int l;
	if(main_size == 4) l = 4; else l = 0;
	switch(main_eamode) {
	case aind: return(l +  4);
	case ainc: return(l +  4);
	case adec: return(l +  6);
	case adsp: return(l +  8);
	case axdp: return(l + 10);
	case absw: return(l +  8);
	case absl: return(l + 12);
	case pcdp: return(l +  8);
	case pcxd: return(l + 10);
	case immd: return(l +  4);
	default:   break;
	}
	return 0;
}

/* Calculate cycles for main EA mode, without fetching (68010 only) */
static int main_ea_cycles_nofetch(void){
	switch(main_eamode) {
	case aind: return(2);
	case ainc: return(4);
	case adec: return(4);
	case adsp: return(4);
	case axdp: return(8);
	case absw: return(4);
	case absl: return(8);
	default:   break;
	}
	return 0;
}

/****************************************************************************
** PREFIXES / SUFFIXES
****************************************************************************/

/*
** Prefixes - stuff that appears before the instruction handling routines
*/
static void prefixes(void) {
	/* Basic stuff - banner, variable section, API */
	gen_banner();
	gen_variables();
	gen_interface();
	/* Internal functions - PC rebasing, I/O, etc. */
	gen_basefunction();
	gen_decode_ext();
	gen_readbw(1);
	gen_readbw(2);
	gen_readl();
	gen_readdecl();
	gen_writebw(1);
	gen_writebw(2);
	gen_writel();
	gen_writedecl();
	gen_group_12_exception();
	gen_privilege_violation();
	gen_flush_interrupts();
}

/*
** Suffixes - stuff that appears after the instruction handling routines and
** the jump table / loop info table
*/
static void suffixes(void) {
	emit("end\n");
}

/****************************************************************************
** INSTRUCTION HANDLING ROUTINES
****************************************************************************/

/* called 600 times (!) */
static void i_move(void) {
	int cycles;
	selective_usereg();
	main_ea_load();
	ea_store(main_size, main_destmode, main_reg);
	selftest(main_size);
	flags_v0();
	cycles = 4 + main_ea_cycles();
	switch(main_destmode) {
	case aind: if(main_size == 4) cycles += 4; cycles +=  4; break;
	case ainc: if(main_size == 4) cycles += 4; cycles +=  4; break;
	case adec: if(main_size == 4) cycles += 4; cycles +=  4; break;
	case adsp: if(main_size == 4) cycles += 4; cycles +=  8; break;
	case axdp: if(main_size == 4) cycles += 4; cycles += 10; break;
	case absw: if(main_size == 4) cycles += 4; cycles +=  8; break;
	case absl: if(main_size == 4) cycles += 4; cycles += 12; break;
	default:   break;
	}
	/* Calculate loop mode timings */
	if(cputype == 68010) {
		switch(main_eamode) {
		case dreg: case areg:
			switch(main_destmode) {
			case aind: case ainc:
				if(main_size <= 2) {
					loop_c_cycles = 2;
					loop_t_cycles = 10;
					loop_x_cycles = 8;
				}else{
					loop_c_cycles = 2;
					loop_t_cycles = 8;
					loop_x_cycles = 6;
				}
				break;
			default:break;
			}
			break;
		case aind: case ainc: case adec:
			switch(main_destmode) {
			case aind: case ainc:
				loop_c_cycles = 2;
				loop_t_cycles = 8;
				loop_x_cycles = 6;
				break;
			case adec:
				loop_c_cycles = 4;
				loop_t_cycles = 10;
				loop_x_cycles = 8;
				break;
			default:break;
			}
			break;
		default:break;
		}
	}
	ret_timing(cycles);
}

static void i_moveq(void) {
	emit("movsx ecx,bl\n");
	emit("mov [__dreg+%d],ecx\n", main_reg * 4);
	selftest(1);
	flags_v0();
	/* No loop mode */
	ret_timing(4);
}

static void op_to_areg(char *s) {
	selective_usereg();
	main_ea_load_signed(); /* sign extends to ecx where necessary */
	emit("%s [__areg+%d],ecx\n", s, main_reg * 4);
}

static void i_movea(void) {
	op_to_areg("mov");
	/* No loop mode */
	ret_timing(4 + main_ea_cycles());
}

/* ADDA or SUBA */
static void addsuba(char *op) {
	int base_cycles;
	op_to_areg(op);
	if(main_size==4){
		base_cycles=6;
		/* Register direct / immediate penalty (68000) */
		if(cputype==68000){
			switch(main_eamode){
			case areg:case dreg:case immd:
				base_cycles+=2;
				break;
			default:break;
			}
		}
	}else{
		base_cycles=8;
	}
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			if(main_size<=2){
				loop_c_cycles = 6;
				loop_t_cycles = 12;
				loop_x_cycles = 10;
			}else{
				loop_c_cycles = 8;
				loop_t_cycles = 14;
				loop_x_cycles = 12;
			}
			break;
		default:break;
		}
	}
	ret_timing(base_cycles+main_ea_cycles());
}

static void i_adda(void){addsuba("add");}
static void i_suba(void){addsuba("sub");}

static void i_cmpa(void){
	op_to_areg("cmp");
	flags();
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			if(main_size<=2){
				loop_c_cycles = 2;
				loop_t_cycles = 8;
				loop_x_cycles = 6;
			}else{
				loop_c_cycles = 4;
				loop_t_cycles = 10;
				loop_x_cycles = 6;
			}
			break;
		default:break;
		}
	}
	ret_timing(6+main_ea_cycles());
}

static void i_move_to_sr(void){
	main_size=2;
	privilegecheck();
	selective_usereg();
	main_ea_load();
	cx2sr();
	/* No loop mode; check PPL and trace flag */
	ret_timing_checkpoint(12+main_ea_cycles());
}

static void i_move_to_ccr(void){
	main_size=2;/* WEIRD! But it works! */
	selective_usereg();
	main_ea_load();
	cl2ccr();
	/* No loop mode */
	ret_timing(12+main_ea_cycles());
}

static void i_move_from_sr(void){
	int cycles;
	/* This is privileged on 68010 and up */
	if(cputype>=68010)privilegecheck();
	main_size=2;
	selective_usereg();
	sr2cx();
	main_ea_store();
	if(cputype==68010){
		cycles=8+main_ea_cycles_nofetch();
	}else{
		cycles=8+main_ea_cycles();
	}
	if((main_eamode==dreg)||(main_eamode==areg)){
		cycles-=2;
		/* Speed demon 68010 can do it in 2 fewer cycles... :p */
		if(cputype==68010)cycles-=2;
	}
	/* No loop mode */
	ret_timing(cycles);
}

/* 68000/68008 aren't supposed to have this */
static void i_move_from_ccr(void){
	int cycles;
	main_size=2;
	selective_usereg();
	ccr2cl();
	main_ea_store();
	if(cputype==68010){
		cycles=8+main_ea_cycles_nofetch();
	}else{
		cycles=8+main_ea_cycles();
	}
	if((main_eamode==dreg)||(main_eamode==areg)){
		cycles-=2;
		if(cputype==68010)cycles-=2;
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void op_to_ccr(char*op){
	ccr2cl();
	emit("%s cl,[esi]\n",op);
	emit("add esi,byte 2\n");
	cl2ccr();
	/* No loop mode */
	ret_timing((cputype==68010)?16:20);
}

static void op_to_sr(char*op){
	privilegecheck();
	sr2cx();
	emit("%s cx,[esi]\n",op);
	emit("add esi,byte 2\n");
	cx2sr();
	/* No loop mode */
	/* Check PPL and trace flag */
	ret_timing_checkpoint((cputype==68010)?16:20);
}

static void  i_ori_ccr(void){op_to_ccr("or" );}
static void i_andi_ccr(void){op_to_ccr("and");}
static void i_eori_ccr(void){op_to_ccr("xor");}
static void  i_ori_sr (void){op_to_sr ("or" );}
static void i_andi_sr (void){op_to_sr ("and");}
static void i_eori_sr (void){op_to_sr ("xor");}

static void i_clr(void){
	int cycles=0;
	selective_usereg();
	emit("xor ecx,ecx\n");
	main_ea_store();
	if(cputype==68000){
		cycles=main_ea_cycles();
		if((main_eamode==dreg)||(main_eamode==areg)){
			cycles+=4;if(main_size==4)cycles+=4;
		}else{
			cycles+=6;if(main_size==4)cycles+=6;
		}
	}else if(cputype==68010){
		switch(main_eamode){
		case dreg:cycles= 4;break;
		case aind:cycles= 8;break;
		case ainc:cycles= 8;break;
		case adec:cycles=10;break;
		case adsp:cycles=12;break;
		case axdp:cycles=16;break;
		case absw:cycles=12;break;
		case absl:cycles=16;break;
		default:break;
		}
		if(main_size==4){
			cycles+=4;
			if(main_eamode==dreg)cycles-=2;
		}
		/* Calculate loop mode timings */
		switch(main_eamode){
		case aind:case ainc:case adec:
			loop_c_cycles = 2;
			loop_t_cycles = 10;
			loop_x_cycles = 8;
			break;
		default:break;
		}
	}
	emit("mov ax,4000h\n");
	ret_timing(cycles);
}

static void i_tst(void){
	selective_usereg();
	main_ea_load();
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			if(main_size<=2){
				loop_c_cycles = 4;
				loop_t_cycles = 10;
				loop_x_cycles = 8;
			}else{
				loop_c_cycles = 6;
				loop_t_cycles = 12;
				loop_x_cycles = 8;
			}
			break;
		default:break;
		}
	}
	selftest(main_size);
	flags_v0();
	ret_timing(4+main_ea_cycles());
}

/* Always affects X flag
** (except for #,An which affects no flags whatsoever) */
static void op_quick(char*op){
	int cycles;
	selective_usereg();
	if(main_eamode==dreg){
		emit(
			"%s %s[__dreg+ebx*4],byte %d\n",
			op,sizename[main_size],quickvalue[main_qv]
		);
		flags();
		c2x();
		cycles=4;
	}else if(main_eamode==areg){
		emit(
			"%s dword[__areg+ebx*4],byte %d\n",
			op,quickvalue[main_qv]
		);
		cycles=4;
		/* SUBQ.W #,An incurs 4-cycle penalty (68000 only) */
		if(cputype==68000){
			if((main_size==2)&&(op[0]=='s'))cycles+=4;
		}
	}else{
		main_ea_rmw_load();
		emit(
			"%s %s,byte %d\n",op,x86cx[main_size],quickvalue[main_qv]
		);
		flags();
		c2x();
		main_ea_rmw_store();
		cycles=8+main_ea_cycles();
	}
	if(main_size==4)cycles+=4;
	/* No loop mode */
	ret_timing(cycles);
}

static void i_addq(void){op_quick("add");}
static void i_subq(void){op_quick("sub");}

static void op_to_dn(char*op,int affectx,int logical){
	int cycles;
	selective_usereg();
	main_ea_load();
	emit("%s [__dreg+%d],%s\n",
		op,main_reg*4,x86cx[main_size]
	);
	if(logical){
		flags_v0();
	}else{
		flags();
	}
	if(affectx)c2x();
	cycles=4+main_ea_cycles();
	if(main_size==4){
		cycles+=2;
		/* Register direct / immediate penalty (68000) */
		if((cputype==68000)&&(op[0]!='c')){
			switch(main_eamode){
			case areg:case dreg:case immd:
				cycles+=2;
				break;
			default:break;
			}
		}
	}
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			switch(op[0]){
			case 'a':/* ADD, AND */
			case 'o':/* OR */
				loop_c_cycles = 8;
				loop_t_cycles = 14;
				loop_x_cycles = 12;
				break;
			case 's':/* SUB */
				if(main_size<=2){
					loop_c_cycles = 8;
					loop_t_cycles = 14;
					loop_x_cycles = 12;
				}else{
					loop_c_cycles = 6;
					loop_t_cycles = 12;
					loop_x_cycles = 10;
				}
				break;
			case 'c':/* CMP */
				loop_c_cycles = 4;
				loop_t_cycles = 10;
				loop_x_cycles = 8;
				if(main_size==4)loop_x_cycles = 6;
				break;
			default:break;
			}
			break;
		default:break;
		}
	}
	ret_timing(cycles);
}

static void i_cmp_dn(void){op_to_dn("cmp",0,0);}
static void i_add_dn(void){op_to_dn("add",1,0);}
static void i_sub_dn(void){op_to_dn("sub",1,0);}
static void i_and_dn(void){op_to_dn("and",0,1);}
static void i_or_dn (void){op_to_dn("or" ,0,1);}

static void op_to_ea(char*op,int logical){
	int cycles;
	selective_usereg();
	main_ea_rmw_load();
	emit(
		"%s %s,[__dreg+%d]\n",
		op,x86cx[main_size],main_reg*4
	);
	if(logical){
		flags_v0();
	}else{
		flags();
	}
	/* Logical instructions don't affect X flag */
	if(!logical)c2x();
	main_ea_rmw_store();
	cycles=8+main_ea_cycles();
	if(main_size==4)cycles+=4;
	/* EOR Dn,Dn takes fewer cycles than we'd expect */
	if((op[0]=='x')&&(main_eamode==dreg)){
		cycles-=4;
		if(cputype==68010)cycles-=2;
	}
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			loop_c_cycles = 4;
			loop_t_cycles = 10;
			loop_x_cycles = 8;
			break;
		default:break;
		}
	}
	ret_timing(cycles);
}

static void i_eor_ea(void){op_to_ea("xor",1);}
static void i_add_ea(void){op_to_ea("add",0);}
static void i_sub_ea(void){op_to_ea("sub",0);}
static void i_and_ea(void){op_to_ea("and",1);}
static void i_or_ea (void){op_to_ea("or" ,1);}

/* called 144 times */
/*
** c1: Total cycles for #,Dn (byte/word)
** c2: Total cycles for #,Dn (long)
** c3: Basic cycles for #,M  (byte/word)
** c4: Basic cycles for #,M  (long)
*/
static void im_to_ea(char*op,int wback,int affectx,int logical,
	int c1,int c2,int c3,int c4){
	int cycles;
	selective_usereg();
	switch(main_size){
	case 1:
	case 2:
		emit("mov cx,[esi]\n");
		emit("add esi,byte 2\n");
		break;
	case 4:
		emit("mov ecx,[esi]\n");
		emit("rol ecx,16\n");
		emit("add esi,byte 4\n");
		break;
	default:break;
	}
	if(main_eamode==dreg){
		emit("%s [__dreg+ebx*4],%s\n",
			op,x86cx[main_size]
		);
		if(logical){
			flags_v0();
		}else{
			flags();
		}
		if(affectx)c2x();
		if(main_size<4)cycles=c1;else cycles=c2;
	}else{
		emit("push ecx\n");
		if(wback)main_ea_rmw_load();else main_ea_load();
		emit("%s %s,[esp]\n",op,x86cx[main_size]);
		if(logical){
			flags_v0();
		}else{
			flags();
		}
		if(affectx)c2x();
		emit("add esp,byte 4\n");
		if(wback)main_ea_rmw_store();
		if(main_size<4)cycles=c3;else cycles=c4;
		cycles+=main_ea_cycles();
	}
	/* No loop mode */
	ret_timing(cycles);
}

/* each called 24 times */
static void i_addi(void){im_to_ea("add",1,1,0,8,(cputype==68010)?14:16,12,20);}
static void i_subi(void){im_to_ea("sub",1,1,0,8,(cputype==68010)?14:16,12,20);}
static void i_cmpi(void){im_to_ea("cmp",0,0,0,8,(cputype==68010)?12:14, 8,12);}
static void i_andi(void){im_to_ea("and",1,0,1,8,(cputype==68010)?14:14,12,20);}
static void i_ori (void){im_to_ea("or" ,1,0,1,8,(cputype==68010)?14:16,12,20);}
static void i_eori(void){im_to_ea("xor",1,0,1,8,(cputype==68010)?14:16,12,20);}

static void flick_reg(char*op,int needxf,int affectx,int asl,int rotate){
	int cycles;
	char tmps[5];
	int i;
	usereg();
	cycles=6;
	if(main_size==4)cycles+=2;
	/* ASR doesn't need overflow checking */
	if(direction[main_dr]=='r')asl=0;
	if(main_ir==1){
		int myline=linenum;linenum++;
		emit("mov ecx,[__dreg+%d]\n",main_reg*4);
		emit("and ecx,byte 63\n");
		emit("jnz short ln%d\n",myline);
		/* The shift count was zero.  Strange things are about to
		** happen... */
		ea_load(main_size,dreg,-1);/* get data in eax */
		selftest(main_size);
		flags_v0();
		if(needxf){/* ROXL/ROXR: Set C flag equal to X */
			emit("and ah,0FEh\n");
			emit("or ah,[__xflag]\n");
		}
		ret_timing(cycles);
		/* Shift count non-zero */
		emit("ln%d:\n",myline);
		emit("sub edi,ecx\n");
		emit("sub edi,ecx\n");
		sprintf(tmps,"cl");
	}else{
		sprintf(tmps,"%d",quickvalue[main_reg]);
		cycles+=2*quickvalue[main_reg];
	}
	if(asl){
		switch(tmps[0]){
		case 'c':/* register shift count */
			emit("mov edx,[__dreg+ebx*4]\n");
			emit("mov al,0\n");/* overflow starts at 0 */
			emit("ln%d:\n",linenum);
			emit("add %s,%s\n",
				x86dx[main_size],x86dx[main_size]
			);
			emit("lahf\n");/* grab N,Z,C flags */
			emit("seto ch\n");
			emit("or al,ch\n");/* add overflow */
			emit("dec cl\n");
			emit("jnz short ln%d\n",linenum);linenum++;
			emit("mov [__dreg+ebx*4],%s\n",x86dx[main_size]);
			if(affectx){
				emit("mov cl,ah\n");
				emit("and cl,1\n");
				emit("mov [__xflag],cl\n");
			}
			break;
		case '1':/* immediate shift count ==1 */
			emit("sal %s[__dreg+ebx*4],1\n",
				sizename[main_size]
			);
			flags();
			if(affectx)c2x();
			break;
		default:/* immediate shift count >1 */
			emit("mov edx,[__dreg+ebx*4]\n");
			emit("mov al,0\n");/* overflow starts at 0 */
			for(i='1';i<=tmps[0];i++){
				emit("add %s,%s\n",
					x86dx[main_size],x86dx[main_size]
				);
				if(i==tmps[0]){
					/* grab N,Z,C flags */
					emit("lahf\n");
				}
				emit("seto ch\n");
				emit("or al,ch\n");/* add overflow */
			}
			emit("mov [__dreg+ebx*4],%s\n",x86dx[main_size]);
			if(affectx){
				emit("mov cl,ah\n");
				emit("and cl,1\n");
				emit("mov [__xflag],cl\n");
			}
			break;
		}
	}else{
		if(rotate){
			emit("mov edx,[__dreg+ebx*4]\n");
			if(needxf){
				emit("mov al,[__xflag]\n");
			}else{
				emit("mov al,0\n");
			}

/*****************/

	switch(tmps[0])
	{
		case 'c':/* register shift count */
			emit("cmp cl, 32\n");
			emit("jb short ln%d\n",linenum);
			emit("sub cl, 31\n");
			if(needxf){
				emit("shr al, 1\n");
			}
			emit("%s%c %s, 31\n", op,direction[main_dr],x86dx[main_size]);
			emit("%s%c %s,%s\n", op,direction[main_dr],x86dx[main_size],tmps);
			emit("jmp short ln%d\n",linenum + 1);
			emit("ln%d:\n",linenum); linenum++;
			if(needxf){
				emit("shr al, 1\n");
			}
			emit("%s%c %s,%s\n", op,direction[main_dr],x86dx[main_size],tmps);
			emit("ln%d:\n",linenum); linenum++;
			break;

		default:/* immediate shift count >1 */
			if(needxf){
				emit("shr al,1\n");
			}
			emit("%s%c %s,%s\n", op,direction[main_dr],x86dx[main_size],tmps);
			break;
	}

/*****************/

//			emit("%s%c %s,%s\n",
//				op,direction[main_dr],x86dx[main_size],tmps
//			);
			emit("adc al,al\n");
			emit("or %s,%s\n",
				x86dx[main_size],x86dx[main_size]
			);
			emit("lahf\n");
			emit("or ah,al\n");
			if(affectx){
				emit("mov [__xflag],al\n");
			}
			emit("mov al,0\n");
			emit("mov [__dreg+ebx*4],%s\n",x86dx[main_size]);
		}else{
			if(needxf){
				emit("mov al,[__xflag]\n");
			}else{
				emit("mov al,0\n");
			}

/*****************/

	switch(tmps[0])
	{
		case 'c':/* register shift count */
			emit("cmp cl, 32\n");
			emit("jb short ln%d\n",linenum);
			emit("sub cl, 31\n");
			if(needxf){
				emit("shr al, 1\n");
			}
			emit("%s%c %s[__dreg+ebx*4], 31\n", op,direction[main_dr],sizename[main_size]);
			emit("jmp short ln%d\n",linenum + 1);
			emit("ln%d:\n",linenum); linenum++;
			if(needxf){
				emit("shr al, 1\n");
			}
			emit("%s%c %s[__dreg+ebx*4],%s\n", op,direction[main_dr],sizename[main_size],tmps);
			emit("ln%d:\n",linenum); linenum++;
			break;

		default:/* immediate shift count >1 */
			if(needxf){
				emit("shr al, 1\n");
			}
			emit("%s%c %s[__dreg+ebx*4],%s\n", op,direction[main_dr],sizename[main_size],tmps);
			break;
	}

/*****************/

//			emit("%s%c %s[__dreg+ebx*4],%s\n",
//				op,direction[main_dr],sizename[main_size],tmps
//			);
			emit("lahf\n");
			if(affectx)c2x();
		}
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void i_lsx_reg(void){flick_reg("sh",0,1,0,0);}
static void i_asx_reg(void){flick_reg("sa",0,1,1,0);}
static void i_rox_reg(void){flick_reg("ro",0,0,0,1);}
static void i_rxx_reg(void){flick_reg("rc",1,1,0,1);}

static void flick_mem(char*op,int needxf,int affectx,int vf,int rotate){
	/* ASR doesn't need overflow checking */
	if(direction[main_dr]=='r')vf=0;
	main_size=2;
	selective_usereg();
	main_ea_rmw_load();
	if(needxf){
		emit("mov al,[__xflag]\n");
		emit("shr al,1\n");
	}else{
		if(rotate)emit("mov al,0\n");
	}
	emit("%s%c cx,1\n",op,direction[main_dr]);
	if(rotate){
		emit("adc al,al\n");
		emit("test cx,cx\n");
		emit("lahf\n");
		emit("or ah,al\n");
		if(affectx)emit("mov [__xflag],al\n");
		emit("mov al,0\n");
	}else{
		if(vf){
			flags();
		}else{
			flags_v0();
		}
		if(affectx)c2x();
	}
	main_ea_rmw_store();
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			loop_c_cycles = 6;
			loop_t_cycles = 12;
			loop_x_cycles = 10;
			break;
		default:break;
		}
	}
	ret_timing(8+main_ea_cycles());
}

static void i_lsx_mem(void){flick_mem("sh",0,1,0,0);}
static void i_asx_mem(void){flick_mem("sa",0,1,1,0);}
static void i_rox_mem(void){flick_mem("ro",0,0,0,1);}
static void i_rxx_mem(void){flick_mem("rc",1,1,0,1);}

static int created_bra_b=0;
static void i_bra_b(void){
	if(!created_bra_b){emit("r_bra_b:\n");created_bra_b=1;}
	emit("movsx ebx,bl\n");
	emit("add esi,ebx\n");
	emit("xor ebx,ebx\n");
	ret_timing(10);
}

static int created_bra_w=0;
static void i_bra_w(void){
	if(!created_bra_w){emit("r_bra_w:\n");created_bra_w=1;}
	emit("movsx ebx,word[esi]\n");
	emit("add esi,ebx\n");
	emit("xor ebx,ebx\n");
	ret_timing(10);
}

static void i_bsr_b(void){
	emit("movsx ebx,bl\n");
	emit("mov ecx,esi\n");
	emit("sub ecx,ebp\n");
	emit("add esi,ebx\n");
	emit("xor ebx,ebx\n");
	ea_store(4,adec,7);
	ret_timing(18);
}

static void i_bsr_w(void){
	emit("movsx ebx,word[esi]\n");
	emit("mov ecx,esi\n");
	emit("add ecx,byte 2\n");
	emit("sub ecx,ebp\n");
	emit("add esi,ebx\n");
	emit("xor ebx,ebx\n");
	ea_store(4,adec,7);
	ret_timing(18);
}

static void i_bcc_b(void){
	getcondition(main_cc);
	emit("j%s near r_bra_b\n",optcc);
	ret_timing((cputype==68010)?6:8);
}

static void i_bcc_w(void){
	getcondition(main_cc);
	emit("j%s near r_bra_w\n",optcc);
	emit("add esi,byte 2\n");/* skip relative offset */
	ret_timing(12);
}

/* called once */
static void i_dbra(void){
	emit("r_dbra:\n");
	usereg();
	if(cputype==68010){
		emit("movsx ecx,word[esi]\n");
		emit("cmp ecx,byte -4\n");
		emit("je short r_loopmode_dbra\n");

		/* Regular DBRA */
		emit("r_regular_dbra:\n");
		emit("sub word[__dreg+ebx*4],byte 1\n");
		emit("jc short r_dbra_expire\n");
		emit("add esi,ecx\n");
		ret_timing(10);
		emit("r_dbra_expire:\n");
		emit("add esi,byte 2\n");
		ret_timing(16);

		/* Loop mode DBRA */
		emit("r_loopmode_dbra:\n");
		emit("cmp byte[__loopmode],0\n");
		emit("jnz short r_loopmode_dbra_inloop\n");
		emit("mov byte[__loopmode],1\n");
		emit("jmp short r_regular_dbra\n");
		emit("r_loopmode_dbra_inloop:\n");
		emit("sub word[__dreg+ebx*4],byte 1\n");
		emit("jc short r_loopmode_dbra_expire\n");

		/* Continue */
		emit("mov bx,word[esi-4]\n");
		emit("add esi,ecx\n");
		/* Subtract continuation cycles */
		emit("mov cl,byte[__looptbl+ebx]\n");
		emit("and ecx,byte 0Eh\n");
		emit("sub edi,ecx\n");
		ret_timing(0);

		/* Expire */
		emit("r_loopmode_dbra_expire:\n");
		emit("mov bx,word[esi-4]\n");
		emit("mov byte[__loopmode],0\n");
		/* Subtract continuation and extra expiration cycles */
		emit("mov cl,byte[__looptbl+ebx]\n");
		emit("mov ebx,ecx\n");
		emit("rol cl,2\n");
		emit("add esi,byte 2\n");
		emit("and ebx,byte 0Eh\n");
		emit("and ecx,byte 06h\n");
		emit("sub edi,ebx\n");
		emit("sub edi,ecx\n");
		ret_timing(0);

		/* Terminate (visited externally by i_dbcc) */
		emit("r_dbra_terminate:\n");
		emit("cmp word[esi],byte -4\n");
		emit("jne short r_dbra_terminate_regular\n");
		emit("cmp byte[__loopmode],0\n");
		emit("je short r_dbra_terminate_regular\n");
		emit("mov bx,word[esi-4]\n");
		emit("mov byte[__loopmode],0\n");
		/* Subtract termination cycles */
		emit("mov cl,byte[__looptbl+ebx]\n");
		emit("shr cl,3\n");
		emit("add esi,byte 2\n");
		emit("and ecx,byte 0Eh\n");
		emit("sub edi,ecx\n");
		ret_timing(0);
		emit("r_dbra_terminate_regular:\n");
		ret_timing(10);
	}else{
		emit("sub word[__dreg+ebx*4],byte 1\n");
		emit("jnc near r_bra_w\n");
		emit("add esi,byte 2\n");
		ret_timing(14);
	}
}

static void i_dbtr(void){
	emit("add esi,byte 2\n");
	ret_timing(8);
}

static void i_dbcc(void){
	getcondition(main_cc);
	emit("j%s near r_dbra\n",optrc);
	/* Terminate */
	if(cputype==68010){
		emit("jmp r_dbra_terminate\n");
	}else{
		emit("add esi,byte 2\n");
		ret_timing(12);
	}
}

static void i_scc(void){
	int cycles;
	main_size=1;
	selective_usereg();
	if(main_cc>1){
		if((main_eamode==dreg)||(main_eamode==areg)){
			if(cputype==68000){
				emit("xor ecx,ecx\n");
			}
			getcondition(main_cc);
			emit("set%s cl\n",optcc);
			if(cputype==68000){
				/* 2 extra cycles if it's true */
				emit("sub edi,ecx\n");
				emit("sub edi,ecx\n");
			}
			emit("neg cl\n");
			cycles=4;
		}else{
			getcondition(main_cc);
			emit("set%s cl\n",optcc);
			emit("neg cl\n");
			cycles=8;
		}
	}else{
		emit("mov cl,%d\n",(main_cc^1)*0xFF);
		if((main_eamode==dreg)||(main_eamode==areg)){
			cycles=4;
			if(cputype==68000){
				/* 2 extra cycles if it's true */
				if(!main_cc)cycles+=2;
			}
		}else{
			cycles=8;
		}
	}
	main_ea_store();
	if(cputype==68010){
		cycles+=main_ea_cycles_nofetch();
	}else{
		cycles+=main_ea_cycles();
	}
	/* No loop mode */
	ret_timing(cycles);
}

/* bit operations */

/* called 315 times */
/* 0=btst,1=bchg,2=bclr,3=bset */
static void bitop(int static_cycles){
	int cycles;
	selective_usereg();
	emit("and ecx,byte %d\n",(main_eamode==dreg)?31:7);
	if(main_eamode==dreg){
		main_size=4;
		if(!main_cc){/* BTST */
			emit("mov edx,1\n");
			emit("shl edx,cl\n");
			emit("test [__dreg+ebx*4],edx\n");
			flag_to_z("z");
		}else{
			emit("mov edx,1\n");
			emit("shl edx,cl\n");
			emit("mov ecx,[__dreg+ebx*4]\n");
			emit("test ecx,edx\n");
			flag_to_z("z");
			switch(main_cc){
			case 1:emit("xor ecx,edx\n");break;
			case 2:emit("not edx\nand ecx,edx\n");break;
			case 3:emit("or ecx,edx\n");break;
			default:break;
			}
			emit("mov [__dreg+ebx*4],ecx\n");
		}
		cycles=6+static_cycles;
		if(main_cc)cycles+=2;
		if(main_cc==2)cycles+=2;
	}else{
		main_size=1;
		if(!main_cc){
			emit("push ecx\n");
			main_ea_load();
			emit("mov edx,ecx\n");
			emit("pop ecx\n");
			emit("inc cl\n");
			emit("shr dl,cl\n");
			flag_to_z("nc");
		}else{
			emit("mov dl,1\n");
			emit("shl dl,cl\n");
			emit("push edx\n");
			main_ea_rmw_load();
			emit("xchg edx,[esp]\n");
			emit("test cl,dl\n");
			flag_to_z("z");
			switch(main_cc){
			case 1:emit("xor cl,dl\n");break;
			case 2:emit("not dl\nand cl,dl\n");break;
			case 3:emit("or cl,dl\n");break;
			default:break;
			}
			emit("pop edx\n");
			main_ea_rmw_store();
		}
		cycles=4+static_cycles+main_ea_cycles();
		if(main_cc)cycles+=4;
		if((cputype==68010)&&(main_cc==2))cycles+=2;
	}
	ret_timing(cycles);
}

/* called 35 times */
static void i_bitop_imm(void){
	emit("mov cl,[esi]\n");
	emit("add esi,byte 2\n");
	bitop(4);
}

/* called 280 times */
static void i_bitop_reg(void){
	emit("mov cl,byte[__dreg+%d]\n",main_reg*4);
	bitop(0);
}

static void i_jmp(void){
	int cycles=0;
	selective_usereg();
	main_ea_control();
	emit("mov esi,edx\n");
	perform_cached_rebase();
	switch(main_eamode){
	case aind:cycles= 8;break;
	case adsp:cycles=10;break;
	case axdp:cycles=14;break;
	case absw:cycles=10;break;
	case absl:cycles=12;break;
	case pcdp:cycles=10;break;
	case pcxd:cycles=14;break;
	default:break;
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void i_jsr(void){
	int cycles=0;
	selective_usereg();
	main_ea_control();
	emit("mov ecx,esi\n");
	emit("sub ecx,ebp\n");
	emit("mov esi,edx\n");
	perform_cached_rebase();
	ea_store(4,adec,7);
	switch(main_eamode){
	case aind:cycles=16;break;
	case adsp:cycles=18;break;
	case axdp:cycles=22;break;
	case absw:cycles=18;break;
	case absl:cycles=20;break;
	case pcdp:cycles=18;break;
	case pcxd:cycles=22;break;
	default:break;
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void i_rts(void){
	ea_load(4,ainc,7);
	emit("mov esi,ecx\n");
	perform_cached_rebase();
	/* No loop mode */
	ret_timing(16);
}

/* 68010 and higher */
static void i_rtd(void){
	emit("mov edx,[__a7]\n");
	emit("call readmemorydword\n");
	emit("movsx ebx,word[esi]\n");
	emit("add edx,ebx\n");
	emit("mov esi,ecx\n");
	emit("add edx,byte 4\n");
	emit("xor ebx,ebx\n");
	emit("mov [__a7],edx\n");
	perform_cached_rebase();
	/* No loop mode */
	ret_timing(16);
}

static void i_rtr(void){
	ea_load(2,ainc,7);
	cl2ccr();
	ea_load(4,ainc,7);
	emit("mov esi,ecx\n");
	perform_cached_rebase();
	/* No loop mode */
	ret_timing(20);
}

static void i_rte(void){
	int myline=linenum;
	linenum++;
	privilegecheck();

	emit("mov edx,[__a7]\n");
	emit("call readmemory%s\n",sizename[2]);
	emit("add edx,byte 2\n");
	cx2sr();
	emit("test ch,20h\n");
	emit("jz short ln%d_nosupe\n",myline);
	emit("add dword [__a7],byte 6\n");
	emit("jmp short ln%d_finish\n",myline);
	emit("ln%d_nosupe:\n",myline);
	emit("add dword [__asp],byte 6\n");
	emit("ln%d_finish:\n",myline);
	emit("call readmemory%s\n",sizename[4]);
	emit("mov esi,ecx\n");
	perform_cached_rebase();
	ret_timing_checkpoint(20);
}

static void i_lea(void){
	int cycles=0;
	selective_usereg();
	main_ea_control();
	emit("mov [__areg+%d],edx\n",main_reg*4);
	switch(main_eamode){
	case aind:cycles= 4;break;
	case adsp:cycles= 8;break;
	case axdp:cycles=12;break;
	case absw:cycles= 8;break;
	case absl:cycles=12;break;
	case pcdp:cycles= 8;break;
	case pcxd:cycles=12;break;
	default:break;
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void i_pea(void){
	int cycles=0;
	selective_usereg();
	main_ea_control();
	emit("mov ecx,edx\n");
	ea_store(4,adec,7);
	switch(main_eamode){
	case aind:cycles=12;break;
	case adsp:cycles=16;break;
	case axdp:cycles=20;break;
	case absw:cycles=16;break;
	case absl:cycles=20;break;
	case pcdp:cycles=16;break;
	case pcxd:cycles=20;break;
	default:break;
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void i_nop(void){
	/* No loop mode */
	ret_timing(4);
}

static void i_movem_control(void){
	int cycles=0;
	int myline=linenum;linenum+=2;
	emit("push eax\n");
	selective_usereg();
	emit("mov ax,[esi]\n");
	emit("add esi,byte 2\n");
	main_ea_control();
	emit("xor ebx,ebx\n");
	emit("ln%d:\n",myline);
	emit("shr eax,1\n");
	emit("jnc short ln%d\n",myline+1);
	if(main_dr==0){/*register->memory*/
		switch(main_eamode){
		case aind:cycles= 8;break;
		case adsp:cycles=12;break;
		case axdp:cycles=14;break;
		case absw:cycles=12;break;
		case absl:cycles=16;break;
		default:break;
		}
		emit("mov ecx,[__reg+ebx]\n");
		emit("call writememory%s\n",sizename[main_size]);
	}else{/*memory->register*/
		switch(main_eamode){
		case aind:cycles=12;
			if((cputype==68010)&&(main_size==4))cycles=24;
			break;
		case adsp:cycles=16;break;
		case axdp:cycles=18;break;
		case absw:cycles=16;break;
		case absl:cycles=20;break;
		case pcdp:cycles=16;break;
		case pcxd:cycles=18;break;
		default:break;
		}
		emit("call readmemory%s\n",sizename[main_size]);
		if(main_size==2)emit("movsx ecx,cx\n");
		emit("mov [__reg+ebx],ecx\n");
	}
	emit("add edx,byte %d\n",main_size);
	emit("sub edi,byte %d\n",main_size*2);
	emit("ln%d:\n",myline+1);
	emit("add ebx,byte 4\n");
	emit("cmp ebx,byte 64\n");
	emit("jne short ln%d\n",myline);
	emit("pop eax\n");
	ret_timing(cycles);
}

static void i_movem_postinc(void){
	int myline=linenum;linenum+=2;
	usereg();
	emit("push eax\n");
	emit("mov ax,[esi]\n");
	emit("add esi,byte 2\n");
	emit("mov edx,[__areg+ebx*4]\n");
	emit("push ebx\n");
	emit("xor ebx,ebx\n");
	emit("ln%d:\n",myline);
	emit("shr eax,1\n");
	emit("jnc short ln%d\n",myline+1);
	emit("call readmemory%s\n",sizename[main_size]);
	if(main_size==2)emit("movsx ecx,cx\n");
	emit("mov [__reg+ebx],ecx\n");
	emit("add edx,byte %d\n",main_size);
	emit("sub edi,byte %d\n",main_size*2);
	emit("ln%d:\n",myline+1);
	emit("add ebx,byte 4\n");
	emit("cmp ebx,byte 64\n");
	emit("jne short ln%d\n",myline);
	emit("pop ebx\n");
	emit("pop eax\n");
	emit("mov [__areg+ebx*4],edx\n");
	ret_timing(12);
}

static void i_movem_predec(void){
	int myline=linenum;linenum+=2;
	usereg();
	emit("push eax\n");
	emit("mov ax,[esi]\n");
	emit("add esi,byte 2\n");
	emit("mov edx,[__areg+ebx*4]\n");
	emit("push ebx\n");
	emit("mov ebx,60\n");
	emit("ln%d:\n",myline);
	emit("shr eax,1\n");
	emit("jnc short ln%d\n",myline+1);
	emit("mov ecx,[__reg+ebx]\n");
	emit("sub edx,byte %d\n",main_size);
	emit("sub edi,byte %d\n",main_size*2);
	emit("call writememory%s\n",sizename[main_size]);
	emit("ln%d:\n",myline+1);
	emit("sub ebx,byte 4\n");
	emit("jns short ln%d\n",myline);
	emit("pop ebx\n");
	emit("pop eax\n");
	emit("mov [__areg+ebx*4],edx\n");
	ret_timing(8);
}

static void i_link(void){
	usereg();
	emit("mov ecx,[__areg+ebx*4]\n");
	ea_store(4,adec,7);
	emit("mov ecx,[__a7]\n");
	emit("mov [__areg+ebx*4],ecx\n");
	emit("movsx edx,word[esi]\n");
	emit("add ecx,edx\n");
	emit("mov [__a7],ecx\n");
	emit("add esi,byte 2\n");
	ret_timing(16);
}

static void i_unlk(void){
	usereg();
	emit("mov ecx,[__areg+ebx*4]\n");
	emit("mov [__a7],ecx\n");
	ea_load(4,ainc,7);
	emit("mov [__areg+ebx*4],ecx\n");
	ret_timing(12);
}

static void i_move_from_usp(void){
	privilegecheck(); /* makes our job much easier... */
	usereg();
	emit("mov ecx,[__asp]\n");
	emit("mov [__areg+ebx*4],ecx\n");
	ret_timing((cputype==68010)?6:4);
}

static void i_move_to_usp(void){
	privilegecheck();
	usereg();
	emit("mov ecx,[__areg+ebx*4]\n");
	emit("mov [__asp],ecx\n");
	ret_timing((cputype==68010)?6:4);
}

static void i_trap(void){
	emit("and ebx,byte 0Fh\n");
	emit("lea edx,[80h+ebx*4]\n");
	emit("call group_2_exception\n");
	perform_cached_rebase();
	ret_timing((cputype==68010)?38:34);
}

static void i_trapv(void){
	int myline=linenum;linenum++;
	emit("test al,1\n");
	emit("jnz short ln%d\n",myline);
	ret_timing(4);
	emit("ln%d:\n",myline);
	emit("mov edx,1Ch\n");
	emit("call group_2_exception\n");
	perform_cached_rebase();
	ret_timing(4+((cputype==68010)?38:34));
}

static void i_stop(void){
	int myline=linenum;linenum++;
	privilegecheck();
	emit("mov cx,[esi]\n");
	emit("add esi,2\n");
	cx2sr();
	emit("or byte[__interrupts],0x10\n");
	/* Forfeit all remaining cycles */
	emit("sub edi,byte 4\n");
	emit("js short ln%d\n",myline);
	emit("xor edi,edi\n");
	emit("dec edi\n");
	emit("ln%d:\n",myline);
	ret_timing(0);
}

static void i_extbw(void){
	usereg();
	emit("movsx cx,byte[__dreg+ebx*4]\n");
	emit("mov [__dreg+ebx*4],cx\n");
	selftest(2);
	flags_v0();
	ret_timing(4);
}

static void i_extwl(void){
	usereg();
	emit("movsx ecx,word[__dreg+ebx*4]\n");
	emit("mov [__dreg+ebx*4],ecx\n");
	selftest(4);
	flags_v0();
	ret_timing(4);
}

static void i_swap(void){
	usereg();
	emit("mov ecx,[__dreg+ebx*4]\n");
	emit("rol ecx,16\n");
	emit("mov [__dreg+ebx*4],ecx\n");
	selftest(4);
	flags_v0();
	ret_timing(4);
}

/* if main_cc==1 then it's signed */
static void i_mul(void){
	int base_cycles;
	selective_usereg();
	main_size=2;
	main_ea_load();
	emit("mov eax,ecx\n");
	/* Finally!  Real MULS/MULU timing! */
	if(cputype==68000){
		emit("mov dl,0\n");
		emit("mov bl,16\n");
		emit("ln%d:\n",linenum);
		emit("add cx,cx\n");
		if(main_cc==1){
			/* MULS: count the number of 10 or 01 pairs */
			emit("seto dh\n");
			emit("add dl,dh\n");
		}else{
			/* MULU: count the number of 1s */
			emit("adc dl,0\n");
		}
		emit("dec bl\n");
		emit("jnz ln%d\n",linenum);linenum++;
		emit("and edx,byte 127\n");
		emit("sub edi,edx\n");
		emit("sub edi,edx\n");
	}
	emit("%smul word[__dreg+%d]\n",
		(main_cc==1)?"i":"",main_reg*4
	);
	emit("shl edx,16\n");
	emit("and eax,0FFFFh\n");
	emit("mov ecx,edx\n");
	emit("or ecx,eax\n");
	flags_v0();
	emit("mov [__dreg+%d],ecx\n",main_reg*4);
	if(cputype==68010){
		/* Maximum is 42 signed, 40 unsigned */
		base_cycles=36;
		if(main_cc)base_cycles+=2;
	}else{
		/* 38+2n, signed or unsigned, maximum is 70 */
		base_cycles=38;
	}
	/* No loop mode */
	ret_timing(base_cycles+main_ea_cycles());
}

/* if main_cc=1 then it's signed */
static void i_div(void){
	int base_cycles;
	int myline=linenum;linenum+=2;
	selective_usereg();
	main_size=2;
	main_ea_load();
	emit("test cx,cx\n");
	emit("jnz short ln%d\n",myline);
	/* Forgot to put on our Division by Zero Suit... */
	emit("mov edx,14h\n");
	emit("call group_2_exception\n");
	perform_cached_rebase();
	base_cycles=38;
	if(cputype==68010)base_cycles+=4;
	ret_timing(base_cycles+main_ea_cycles());
	emit("ln%d:\n",myline);myline++;
	if(main_cc){
		emit("movsx ecx,cx\n");
	}else{
		emit("and ecx,0FFFFh\n");
	}
	emit("mov eax,[__dreg+%d]\n",main_reg*4);
	if(main_cc){
		emit("mov edx,eax\n");
		emit("sar edx,31\n");
	}else{
		emit("xor edx,edx\n");
	}
	emit("%sdiv ecx\n",main_cc?"i":"");
	if(main_cc){
		emit("mov ecx,eax\n");
		emit("sar cx,15\n");
		emit("or ecx,ecx\n");
		emit("je short ln%d\n",linenum);
		emit("inc ecx\n");
		emit("jne short ln%d\n",myline);
		emit("ln%d:\n",linenum);linenum++;
		emit("and eax,0FFFFh\n");
	}else{
		emit("test eax,0FFFF0000h\n");
		emit("jnz short ln%d\n",myline);
	}
	emit("shl edx,16\n");
	emit("mov dx,ax\n");
	emit("test dx,dx\n");
	flags_v0();
	emit("mov [__dreg+%d],edx\n",main_reg*4);
	if(cputype==68010){
		base_cycles=108;
		if(main_cc)base_cycles=122;
	}else{
		/* Varies from 142-158 signed, 126-140 unsigned */
		base_cycles=133;
		if(main_cc)base_cycles=150;
	}
	ret_timing(base_cycles+main_ea_cycles());
	/* Overflow */
	emit("ln%d:\n",myline);myline++;
	emit("mov ax,1\n");
	/* No loop mode */
	ret_timing(base_cycles+main_ea_cycles());
}

static void i_neg(void){
	int cycles;
	selective_usereg();
	cycles=4;
	if(main_size==4)cycles=6;
	if(main_eamode==dreg){
		emit("neg %s[__dreg+ebx*4]\n",
			sizename[main_size]
		);
		flags();
		c2x();
	}else{
		cycles*=2;
		main_ea_rmw_load();
		emit("neg %s\n",x86cx[main_size]);
		flags();
		c2x();
		main_ea_rmw_store();
		cycles+=main_ea_cycles();
	}
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			loop_c_cycles = 4;
			loop_t_cycles = 10;
			loop_x_cycles = 8;
			break;
		default:break;
		}
	}
	ret_timing(cycles);
}

static void i_negx(void){
	int cycles;
	selective_usereg();
	cycles=4;
	if(main_size==4)cycles=6;
	if(main_eamode==dreg){
		emit("mov cl,[__xflag]\n");
		emit("shr cl,1\n");
		if(main_size==1)emit("mov cl,0\n");
		else emit("mov ecx,0\n");
		emit("sbb %s,[__dreg+ebx*4]\n",x86cx[main_size]);
		emit("mov edx,eax\n");
		flags();c2x();
		adjzero("dh");
		emit("mov [__dreg+ebx*4],%s\n",x86cx[main_size]);
	}else{
		cycles*=2;
		main_ea_rmw_load();
		emit("push ebx\n");
		emit("mov bl,[__xflag]\n");
		emit("shr bl,1\n");
		if(main_size==1)emit("mov bl,0\n");
		else emit("mov ebx,0\n");
		emit("sbb %s,%s\n",x86bx[main_size],x86cx[main_size]);
		emit("mov ecx,ebx\n");
		emit("mov ebx,eax\n");
		flags();c2x();
		adjzero("bh");
		emit("pop ebx\n");
		main_ea_rmw_store();
		cycles+=main_ea_cycles();
	}
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			loop_c_cycles = 4;
			loop_t_cycles = 10;
			loop_x_cycles = 8;
			break;
		default:break;
		}
	}
	ret_timing(cycles);
}

static void i_nbcd(void){
	int cycles;
	main_size=1;
	selective_usereg();
	main_ea_rmw_load();
	/* Get the X flag into carry */
	emit("mov cl,[__xflag]\n");
	emit("shr cl,1\n");
	/* Save the previous Z flag in CH */
	emit("mov ch,ah\n");
	/* Perform the BCD subtraction */
	emit("mov al,0\n");
	emit("sbb al,cl\n");
	emit("das\n");
	/* Save result in CL */
	emit("mov cl,al\n");
	/* Set flags - V undefined */
	flags_v0();
	c2x();
	/* Adjust for non-changing Z (previous Z flag in CH) */
	adjzero("ch");
	main_ea_rmw_store();
	if(main_eamode==dreg){
		cycles=6;
	}else{
		cycles=8+main_ea_cycles();
	}
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			loop_c_cycles = 6;
			loop_t_cycles = 12;
			loop_x_cycles = 10;
			break;
		default:break;
		}
	}
	ret_timing(cycles);
}

static void i_tas(void){
	int cycles;
	main_size=1;
	selective_usereg();
	main_ea_rmw_load();
	selftest(1);
	flags_v0();

	/* Gens changes */
	
	if((main_eamode==dreg)||(main_eamode==areg)){
		emit("or cl,80h\n");
		main_ea_rmw_store();
		cycles=4;
	}else{
		cycles=14+main_ea_cycles();
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void i_not(void){
	int cycles;
	selective_usereg();
	cycles=4;
	if(main_size==4)cycles=6;
	if(main_eamode==dreg){
		emit("xor %s[__dreg+ebx*4],byte -1\n",
			sizename[main_size]
		);
		flags_v0();
	}else{
		cycles*=2;
		main_ea_rmw_load();
		emit("xor %s,byte -1\n",x86cx[main_size]);
		flags_v0();
		main_ea_rmw_store();
		cycles+=main_ea_cycles();
	}
	/* Calculate loop mode timings */
	if(cputype==68010){
		switch(main_eamode){
		case aind:case ainc:case adec:
			loop_c_cycles = 4;
			loop_t_cycles = 10;
			loop_x_cycles = 8;
			break;
		default:break;
		}
	}
	ret_timing(cycles);
}

/* main_reg=rx, main_dr is 0 or 32 (rx a/d), main_ir is 0 or 32 (ry a/d) */
static void i_exg(void){
	usereg();
	emit("mov ecx,[__reg+%d]\n",(main_reg*4)+main_dr);
	emit("mov edx,[__reg+%d+ebx*4]\n",main_ir);
	emit("mov [__reg+%d],edx\n",(main_reg*4)+main_dr);
	emit("mov [__reg+%d+ebx*4],ecx\n",main_ir);
	/* No loop mode */
	ret_timing(6);
}

static void i_cmpm(void){
	usereg();
	ea_load(main_size,ainc,-1);/* Keep this in order */
	emit("mov eax,ecx\n");
	ea_load(main_size,ainc,main_reg);
	emit("cmp %s,%s\n",x86cx[main_size],x86ax[main_size]);
	flags();
	/* Calculate loop mode timings */
	if(cputype==68010){
		if(main_size<=2){
			loop_c_cycles = 2;
			loop_t_cycles = 8;
			loop_x_cycles = 6;
		}else{
			loop_c_cycles = 4;
			loop_t_cycles = 10;
			loop_x_cycles = 6;
		}
	}
	ret_timing((main_size==4)?20:12);
}

static void opx_dreg(char*op,char*adjust){
	int cycles;
	usereg();
	emit("mov ch,ah\n");/* Save old Z flag in CH */
	emit("mov cl,[__xflag]\n");
	emit("shr cl,1\n");/* X -> x86 carry */
	emit("mov eax,[__dreg+%d]\n",main_reg*4);
	emit("%s %s,[__dreg+ebx*4]\n",op,x86ax[main_size]);
	if(adjust[0]){
		emit("%s\n",adjust);
	}
	emit("mov [__dreg+%d],%s\n",main_reg*4,x86ax[main_size]);
	emit("lahf\n");
	if(adjust[0]){
		emit("mov al,0\n");
	}else{
		emit("seto al\n");
	}
	c2x();
	adjzero("ch");
	if(main_size<=2){
		cycles=4;
		if(adjust[0])cycles=6;
	}else{
		cycles=8;
		if(cputype==68010)cycles=6;
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void opx_adec(char*op,char*adjust){
	int cycles;
	usereg();
	ea_load(main_size,adec,-1);/* Keep this in order */
	emit("mov ebx,ecx\n");
	ea_rmw_load(main_size,adec,main_reg);
	emit("xchg ecx,eax\n");/* flags -> ECX, dest -> EAX */
	emit("mov cl,[__xflag]\n");
	emit("shr cl,1\n");/* X -> x86 carry */
	emit("%s %s,%s\n",op,x86ax[main_size],x86bx[main_size]);
	if(adjust[0]){
		emit("%s\n",adjust);
	}
	emit("mov ebx,eax\n");
	emit("lahf\n");
	if(adjust[0]){
		emit("mov al,0\n");
	}else{
		emit("seto al\n");
	}
	c2x();
	adjzero("ch");
	emit("mov ecx,ebx\n");
	emit("xor ebx,ebx\n");
	ea_rmw_store(main_size,adec,main_reg);
	if(main_size<=2){
		cycles=18;
	}else{
		cycles=30;
	}
	/* Calculate loop mode timings */
	if(cputype==68010){
		if(adjust[0]){
			loop_c_cycles = 6;
			loop_t_cycles = 12;
			loop_x_cycles = 10;
		}else{
			if(main_size<=2){
				loop_c_cycles = 4;
				loop_t_cycles = 10;
				loop_x_cycles = 8;
			}else{
				loop_c_cycles = 2;
				loop_t_cycles = 8;
				loop_x_cycles = 6;
			}
		}
	}
	ret_timing(cycles);
}

static void i_addx_dreg(void){opx_dreg("adc","");}
static void i_addx_adec(void){opx_adec("adc","");}
static void i_subx_dreg(void){opx_dreg("sbb","");}
static void i_subx_adec(void){opx_adec("sbb","");}
static void i_abcd_dreg(void){main_size=1;opx_dreg("adc","daa");}
static void i_abcd_adec(void){main_size=1;opx_adec("adc","daa");}
static void i_sbcd_dreg(void){main_size=1;opx_dreg("sbb","das");}
static void i_sbcd_adec(void){main_size=1;opx_adec("sbb","das");}

static void i_movep_mem2reg(void){
	int cycles;
	usereg();
	emit("movsx edx,word[esi]\n");
	emit("add esi,byte 2\n");
	emit("add edx,[__areg+ebx*4]\n");
	emit("call readmemorybyte\n");
	emit("mov bh,cl\n");
	emit("add edx,byte 2\n");
	emit("call readmemorybyte\n");
	emit("mov bl,cl\n");
	if(main_size==2){
		emit("mov [__dreg+%d],bx\n",main_reg*4);
		cycles=16;
	}else{
		emit("add edx,byte 2\n");
		emit("shl ebx,16\n");
		emit("call readmemorybyte\n");
		emit("mov bh,cl\n");
		emit("add edx,byte 2\n");
		emit("call readmemorybyte\n");
		emit("mov bl,cl\n");
		emit("mov [__dreg+%d],ebx\n",main_reg*4);
		emit("xor ebx,ebx\n");
		cycles=24;
	}
	/* No loop mode */
	ret_timing(cycles);
}

static void i_movep_reg2mem(void){
	int cycles;
	usereg();
	emit("movsx edx,word[esi]\n");
	emit("add esi,byte 2\n");
	emit("add edx,[__areg+ebx*4]\n");
	emit("mov ebx,[__dreg+%d]\n",main_reg*4);
	if(main_size==4)emit("rol ebx,16\n");
	emit("mov cl,bh\n");
	emit("call writememorybyte\n");
	emit("add edx,byte 2\n");
	emit("mov cl,bl\n");
	emit("call writememorybyte\n");
	if(main_size==4){
		emit("add edx,byte 2\n");
		emit("rol ebx,16\n");
		emit("mov cl,bh\n");
		emit("call writememorybyte\n");
		emit("add edx,byte 2\n");
		emit("mov cl,bl\n");
		emit("call writememorybyte\n");
		cycles=24;
	}else{
		cycles=16;
	}
	emit("xor ebx,ebx\n");
	/* No loop mode */
	ret_timing(cycles);
}

static void i_chk(void){
	int cycles;
	int myline=linenum;linenum++;
	selective_usereg();
	main_ea_load();
	emit("cmp %s[__dreg+%d],byte 0\n",
		sizename[main_size],main_reg*4
	);
	emit("mov ax,8000h\n");
	emit("jl short ln%d\n",myline);
	emit("cmp [__dreg+%d],%s\n",
		main_reg*4,x86cx[main_size]
	);
	emit("mov ax,0\n");
	emit("jg short ln%d\n",myline);
	cycles=10;
	if(cputype==68010)cycles=8;
	ret_timing(cycles+main_ea_cycles());

	/* Out of bounds, so generate CHK exception */
	emit("ln%d:",myline);
	emit("mov edx,18h\n");
	emit("call group_2_exception\n");
	perform_cached_rebase();
	cycles=40;
	if(cputype==68010)cycles=44;
	/* No loop mode */
	ret_timing(cycles+main_ea_cycles());
}

static int created_illegal=0;
static void i_illegal(void){
	if(!created_illegal){emit("r_illegal:\n");created_illegal=1;}
	emit("sub esi,byte 2\n");
	emit("mov edx,10h\n");
	emit("call group_1_exception\n");
	perform_cached_rebase();
	ret_timing((cputype==68010)?38:34);
}

/* Breakpoint - notify hardware */
static void i_bkpt(void){
	int myline=linenum;
	linenum++;
	emit("mov ecx,[__bkpthandler]\n");
	emit("or ecx,ecx\n");
	emit("jz ln%d\n",myline);
	airlock_exit();
	emit("call ecx\n");
	airlock_enter();
	emit("ln%d:\n",myline);
	emit("mov edx,10h\n");
	emit("call group_1_exception\n");
	perform_cached_rebase();
	ret_timing((cputype==68010)?38:34);
}

static void i_aline(void){
	emit("sub esi,byte 2\n");
	emit("mov edx,28h\n");
	emit("call group_1_exception\n");
	perform_cached_rebase();
	/* This is just a guess */
	ret_timing((cputype==68010)?38:34);
}

static void i_fline(void){
	emit("sub esi,byte 2\n");
	emit("mov edx,2Ch\n");
	emit("call group_1_exception\n");
	perform_cached_rebase();
	/* This is just a guess */
	ret_timing((cputype==68010)?38:34);
}

static void i_reset(void){
	privilegecheck();
	emit("mov ecx,[__resethandler]\n");
	emit("or ecx,ecx\n");
	emit("jz near invalidins\n");
	airlock_exit();
	emit("call ecx\n");
	airlock_enter();
	ret_timing((cputype==68010)?130:132);
}

static void i_movec_c_to_r(void){
	int myline=linenum;linenum++;
	privilegecheck();
	emit("mov bx,word[esi]\n");
	emit("mov edx,ebx\n");
	emit("shr ebx,12\n");

	emit("and edx,0FFFh\n");
	emit("jnz short ln%d\n",linenum);
	emit("mov cl,[__sfc]\n");
	emit("and ecx,byte 7\n");
	emit("jmp short ln%d\n",myline);
	emit("ln%d:\n",linenum);linenum++;

	emit("cmp edx,byte 1\n");
	emit("jnz short ln%d\n",linenum);
	emit("mov cl,[__dfc]\n");
	emit("and ecx,byte 7\n");
	emit("jmp short ln%d\n",myline);
	emit("ln%d:\n",linenum);linenum++;

	emit("cmp edx,0800h\n");
	emit("jnz short ln%d\n",linenum);
	emit("mov ecx,[__asp]\n");
	emit("jmp short ln%d\n",myline);
	emit("ln%d:\n",linenum);linenum++;

	emit("cmp edx,0801h\n");
	emit("jnz short ln%d\n",linenum);
	emit("mov ecx,[__vbr]\n");
	emit("jmp short ln%d\n",myline);
	emit("ln%d:\n",linenum);linenum++;

	emit("jmp r_illegal\n");

	emit("ln%d:\n",myline);
	emit("add esi,byte 2\n");
	emit("mov dword[__reg+ebx*4],ecx\n");
	ret_timing(12);
}

static void i_movec_r_to_c(void){
	int myline=linenum;linenum++;
	privilegecheck();
	emit("mov bx,word[esi]\n");
	emit("mov edx,ebx\n");
	emit("shr ebx,12\n");
	emit("mov ecx,dword[__reg+ebx*4]\n");

	emit("and edx,0FFFh\n");
	emit("jnz short ln%d\n",linenum);
	emit("and cl,7\n");
	emit("mov [__sfc],cl\n");
	emit("jmp short ln%d\n",myline);
	emit("ln%d:\n",linenum);linenum++;

	emit("cmp edx,byte 1\n");
	emit("jnz short ln%d\n",linenum);
	emit("and cl,7\n");
	emit("mov [__dfc],cl\n");
	emit("jmp short ln%d\n",myline);
	emit("ln%d:\n",linenum);linenum++;

	emit("cmp edx,0800h\n");
	emit("jnz short ln%d\n",linenum);
	emit("mov [__asp],ecx\n");
	emit("jmp short ln%d\n",myline);
	emit("ln%d:\n",linenum);linenum++;

	emit("cmp edx,0801h\n");
	emit("jnz short ln%d\n",linenum);
	emit("mov [__vbr],ecx\n");
	emit("jmp short ln%d\n",myline);
	emit("ln%d:\n",linenum);linenum++;

	emit("jmp r_illegal\n");

	emit("ln%d:\n",myline);
	emit("add esi,byte 2\n");
	ret_timing(10);
}

#if 0
/* called not-quite-so-many times as i_move */
static void i_moves(void){
	int cycles;
	int unitsize = main_size == 4 ? 2 : main_size;
	int myline=linenum;
	linenum++;
	selective_usereg();
	emit("push esi\n"); /* save in case it's invalid */
	emit("movzx ecx,word[esi]\n");
	emit("add esi,byte 2\n");
	ea_step_precalc(main_size,main_eamode,-1); /* edx=address */
	emit("shr ecx,12\n");
	emit("jc short ln%d_write\n",myline);
	/* read */
	emit("cmp byte[__sfc],1\n");
	emit("je ln%d_read_userdata\n");
	emit("cmp byte[__sfc],2\n");
	emit("je ln%d_read_userprogram\n");
	emit("cmp byte[__sfc],5\n");
	emit("je ln%d_read_superdata\n");
	emit("cmp byte[__sfc],6\n");
	emit("je ln%d_read_superprogram\n");

	/*
	** Generic address space read routine; SFC in {0,3,4,7}
	*/
	emit("cmp dword[__fc_read%s],byte 0\n",sizename[unitsize]);
	emit("je short ln%d_inv\n",myline);
	emit("push ecx\n");
	emit("push edx\n");
	airlock_exit();
	emit("mov al,[__sfc]\n");
	emit("and eax,byte 7\n");
	if(use_stack) {
		emit("push edx\n");
		emit("push eax\n");
	}
	emit("call dword[__fc_read%s]\n");
	if(use_stack) {
		emit("add esp,byte 8\n");
	}
	emit("mov ecx,[esp+%d]\n",airlock_stacksize+4);
	switch(main_size) {
	case 1:emit("mov [__reg+ecx*4],al\n");break;
	case 2:emit("mov [__reg+ecx*4],ax\n");break;
	case 4:emit("mov [__reg+ecx*4+2],ax\n");
		emit("mov edx,[esp+%d]\n",airlock_stacksize);
		emit("add edx,byte 2\n");
		emit("mov al,[__sfc]\n");
		emit("and eax,byte 7\n");
		if(use_stack) {
			emit("push edx\n");
			emit("push eax\n");
		}
		emit("call dword[__fc_read%s]\n");
		if(use_stack) {
			emit("add esp,byte 8\n");
		}
		emit("mov ecx,[esp+%d]\n",airlock_stacksize+4);
		emit("mov [__reg+ecx*4],ax\n");
		break;
	}
	airlock_enter();
	emit("pop edx\n");
	emit("pop ecx\n");
	ea_step_postcalc(main_size,main_eamode,-1); /* edx=address */
	emit("jmp ln%d_end\n");

	ret_timing(cycles);
}
#endif

/****************************************************************************
** DECODE ROUTINES
****************************************************************************/

static int  rproc [0x10000];
static byte unique[0x10000];
static int  cease_decode;

static int test(int n, int m, int op) {
	int t;
	if((n & m) != op) return 0;
	for(t = op & 0xF000; t < n; t++) {
		if((!unique[t]) && ((t & m) == (n & m))) {
			rproc[n] = t;
			return 2;
		}
	}
	unique[n] = (m >> 16) & 1;
	rproc[n] = n;
	t = (m ^ 0xFFFF) & 0xFFF;
	if(!t) {
		emit("; Opcode %04X\n", n);
	} else {
		emit("; Opcodes %04X - %04X\n", n, op + t);
	}
/*	align(4);*/
	emit("%c%03X:\n", ((n >> 12) & 0xF) + 'K', n & 0xFFF);
	routine_counter++;
	return 1;
}

/* Instruction definition routine */
static void idef(
	int n, int mask, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	cease_decode = test(n, mask, op);
	if(cease_decode == 1) {
		if(cputype == 68010) {
			loop_c_cycles = 10;
			loop_t_cycles = 10;
			loop_x_cycles = 16;
		}
		proc();
		if(cputype == 68010) {
			if(loop_c_cycles > 14) {
				fprintf(stderr,
					"Bad news: instruction %04X:\n"
					"loop_c_cycles (%d) exceeds limit\n",
					n, loop_c_cycles
				);
				exit(1);
			}
			if(loop_t_cycles > 14) {
				fprintf(stderr,
					"Bad news: instruction %04X:\n"
					"loop_t_cycles (%d) exceeds limit\n",
					n, loop_t_cycles
				);
				exit(1);
			}
			if(loop_x_cycles > (loop_c_cycles + 6)) {
				fprintf(stderr,
					"Bad news: instruction %04X:\n"
					"loop_c_cycles (%d) and "
					"loop_x_cycles (%d) too far apart\n",
					n, loop_c_cycles, loop_x_cycles
				);
				exit(1);
			}
			loop_x_cycles -= loop_c_cycles;
			loopinfo[n] =
				(((loop_c_cycles     ) & 0x0E)  |
				 ((loop_t_cycles << 3) & 0x70)) |
				(((loop_x_cycles << 6) & 0x80)  |
				 ((loop_x_cycles >> 2) & 0x01));
		}
	}
}

/* Batch idef for all addressing modes */
static void eadef_all(
	int n, int m, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	main_eamode = dreg; idef(n, m | 0x38, op | 0x00, proc);
	main_eamode = areg; idef(n, m | 0x38, op | 0x08, proc);
	main_eamode = aind; idef(n, m | 0x38, op | 0x10, proc);
	main_eamode = ainc; idef(n, m | 0x38, op | 0x18, proc);
	main_eamode = adec; idef(n, m | 0x38, op | 0x20, proc);
	main_eamode = adsp; idef(n, m | 0x38, op | 0x28, proc);
	main_eamode = axdp; idef(n, m | 0x38, op | 0x30, proc);
	main_eamode = absw; idef(n, m | 0x3F, op | 0x38, proc);
	main_eamode = absl; idef(n, m | 0x3F, op | 0x39, proc);
	main_eamode = pcdp; idef(n, m | 0x3F, op | 0x3A, proc);
	main_eamode = pcxd; idef(n, m | 0x3F, op | 0x3B, proc);
	main_eamode = immd; idef(n, m | 0x3F, op | 0x3C, proc);
}

/* Batch idef for all addressing modes, excluding Address Register Direct
**  mode when the operand size is 1 */
static void eadef_all_nobyteaddress(
	int n, int m, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	main_eamode = dreg; idef(n, m | 0x38, op | 0x00, proc);
	if(main_size != 1) {
		main_eamode = areg; idef(n, m | 0x38, op | 0x08, proc);
	}
	main_eamode = aind; idef(n, m | 0x38, op | 0x10, proc);
	main_eamode = ainc; idef(n, m | 0x38, op | 0x18, proc);
	main_eamode = adec; idef(n, m | 0x38, op | 0x20, proc);
	main_eamode = adsp; idef(n, m | 0x38, op | 0x28, proc);
	main_eamode = axdp; idef(n, m | 0x38, op | 0x30, proc);
	main_eamode = absw; idef(n, m | 0x3F, op | 0x38, proc);
	main_eamode = absl; idef(n, m | 0x3F, op | 0x39, proc);
	main_eamode = pcdp; idef(n, m | 0x3F, op | 0x3A, proc);
	main_eamode = pcxd; idef(n, m | 0x3F, op | 0x3B, proc);
	main_eamode = immd; idef(n, m | 0x3F, op | 0x3C, proc);
}

/* Batch idef for all data addressing modes */
static void eadef_data(
	int n, int m, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	main_eamode = dreg; idef(n, m | 0x38, op | 0x00, proc);
	main_eamode = aind; idef(n, m | 0x38, op | 0x10, proc);
	main_eamode = ainc; idef(n, m | 0x38, op | 0x18, proc);
	main_eamode = adec; idef(n, m | 0x38, op | 0x20, proc);
	main_eamode = adsp; idef(n, m | 0x38, op | 0x28, proc);
	main_eamode = axdp; idef(n, m | 0x38, op | 0x30, proc);
	main_eamode = absw; idef(n, m | 0x3F, op | 0x38, proc);
	main_eamode = absl; idef(n, m | 0x3F, op | 0x39, proc);
	main_eamode = pcdp; idef(n, m | 0x3F, op | 0x3A, proc);
	main_eamode = pcxd; idef(n, m | 0x3F, op | 0x3B, proc);
	main_eamode = immd; idef(n, m | 0x3F, op | 0x3C, proc);
}

/* Batch idef for all alterable addressing modes, excluding Address Register
**  Direct mode when the operand size is 1 */
static void eadef_alterable_nobyteaddress(
	int n, int m, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	main_eamode = dreg; idef(n, m | 0x38, op | 0x00, proc);
	if(main_size != 1) {
		main_eamode = areg; idef(n, m | 0x38, op | 0x08, proc);
	}
	main_eamode = aind; idef(n, m | 0x38, op | 0x10, proc);
	main_eamode = ainc; idef(n, m | 0x38, op | 0x18, proc);
	main_eamode = adec; idef(n, m | 0x38, op | 0x20, proc);
	main_eamode = adsp; idef(n, m | 0x38, op | 0x28, proc);
	main_eamode = axdp; idef(n, m | 0x38, op | 0x30, proc);
	main_eamode = absw; idef(n, m | 0x3F, op | 0x38, proc);
	main_eamode = absl; idef(n, m | 0x3F, op | 0x39, proc);
}

/* Batch idef for all data alterable addressing modes */
static void eadef_data_alterable(
	int n, int m, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	main_eamode = dreg; idef(n, m | 0x38, op | 0x00, proc);
	main_eamode = aind; idef(n, m | 0x38, op | 0x10, proc);
	main_eamode = ainc; idef(n, m | 0x38, op | 0x18, proc);
	main_eamode = adec; idef(n, m | 0x38, op | 0x20, proc);
	main_eamode = adsp; idef(n, m | 0x38, op | 0x28, proc);
	main_eamode = axdp; idef(n, m | 0x38, op | 0x30, proc);
	main_eamode = absw; idef(n, m | 0x3F, op | 0x38, proc);
	main_eamode = absl; idef(n, m | 0x3F, op | 0x39, proc);
}

/* Batch idef for all memory alterable addressing modes */
static void eadef_memory_alterable(
	int n, int m, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	main_eamode = aind; idef(n, m | 0x38, op | 0x10, proc);
	main_eamode = ainc; idef(n, m | 0x38, op | 0x18, proc);
	main_eamode = adec; idef(n, m | 0x38, op | 0x20, proc);
	main_eamode = adsp; idef(n, m | 0x38, op | 0x28, proc);
	main_eamode = axdp; idef(n, m | 0x38, op | 0x30, proc);
	main_eamode = absw; idef(n, m | 0x3F, op | 0x38, proc);
	main_eamode = absl; idef(n, m | 0x3F, op | 0x39, proc);
}

/* Batch idef for all control addressing modes */
static void eadef_control(
	int n, int m, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	main_eamode = aind; idef(n, m | 0x38, op | 0x10, proc);
	main_eamode = adsp; idef(n, m | 0x38, op | 0x28, proc);
	main_eamode = axdp; idef(n, m | 0x38, op | 0x30, proc);
	main_eamode = absw; idef(n, m | 0x3F, op | 0x38, proc);
	main_eamode = absl; idef(n, m | 0x3F, op | 0x39, proc);
	main_eamode = pcdp; idef(n, m | 0x3F, op | 0x3A, proc);
	main_eamode = pcxd; idef(n, m | 0x3F, op | 0x3B, proc);
}

/* Batch idef for all control alterable addressing modes */
static void eadef_control_alterable(
	int n, int m, int op, void(*proc)(void)
) {
	if(cease_decode) return;
	main_eamode = aind; idef(n, m | 0x38, op | 0x10, proc);
	main_eamode = adsp; idef(n, m | 0x38, op | 0x28, proc);
	main_eamode = axdp; idef(n, m | 0x38, op | 0x30, proc);
	main_eamode = absw; idef(n, m | 0x3F, op | 0x38, proc);
	main_eamode = absl; idef(n, m | 0x3F, op | 0x39, proc);
}

/* Batch eadef for MOVE instructions */
static void defmove(int majorop, int n) {
	if(cease_decode) return;
	for(main_reg = 0; main_reg < 8; main_reg++) {
		word w = majorop | (main_reg << 9);
		main_destmode = dreg; eadef_all(n, 0xFFC0, 0x000 | w, i_move);
		main_destmode = aind; eadef_all(n, 0xFFC0, 0x080 | w, i_move);
		main_destmode = ainc; eadef_all(n, 0xFFC0, 0x0C0 | w, i_move);
		main_destmode = adec; eadef_all(n, 0xFFC0, 0x100 | w, i_move);
		main_destmode = adsp; eadef_all(n, 0xFFC0, 0x140 | w, i_move);
		main_destmode = axdp; eadef_all(n, 0xFFC0, 0x180 | w, i_move);
	}
	main_destmode = absw; eadef_all(n, 0xFFC0, 0x1C0 | majorop, i_move);
	main_destmode = absl; eadef_all(n, 0xFFC0, 0x3C0 | majorop, i_move);
}

/***************************************************************************/

static void decode0(int n) {
	cease_decode = 0;
	for(sizedef = 0; sizedef < 3; sizedef++) {
		main_size = 1 << sizedef;
		eadef_data_alterable(n, 0xFFC0, 0x0000 | (sizedef << 6), i_ori);
		eadef_data_alterable(n, 0xFFC0, 0x0200 | (sizedef << 6), i_andi);
		eadef_data_alterable(n, 0xFFC0, 0x0400 | (sizedef << 6), i_subi);
		eadef_data_alterable(n, 0xFFC0, 0x0600 | (sizedef << 6), i_addi);
		eadef_data_alterable(n, 0xFFC0, 0x0A00 | (sizedef << 6), i_eori);
		eadef_data_alterable(n, 0xFFC0, 0x0C00 | (sizedef << 6), i_cmpi);
		/* Not quite ready for prime time yet */
/*		if(cputype >= 68010) {
			eadef_memory_alterable(n, 0xFFC0, 0x0E00 | (sizedef << 6), i_moves);
		}*/
	}
	/*
	** Bit operations
	** BTST (main_cc 0) works with all data addressing modes; the others
	** require data alterable addressing modes
	*/
	for(main_cc = 0; main_cc < 4; main_cc++) {
		void (*eadef)(int n, int m, int op, void(*proc)(void)) =
			main_cc ? eadef_data_alterable : eadef_data;
		eadef(n, 0xFFC0, 0x0800 | (main_cc << 6), i_bitop_imm);
		for(main_reg = 0; main_reg < 8; main_reg++) {
			eadef(n, 0xFFC0,
				0x0100 | (main_cc << 6) | (main_reg << 9),
				i_bitop_reg
			);
		}
	}
	idef(n, 0xFFFF, 0x003C, i_ori_ccr );
	idef(n, 0xFFFF, 0x023C, i_andi_ccr);
	idef(n, 0xFFFF, 0x0A3C, i_eori_ccr);
	idef(n, 0xFFFF, 0x007C, i_ori_sr  );
	idef(n, 0xFFFF, 0x027C, i_andi_sr );
	idef(n, 0xFFFF, 0x0A7C, i_eori_sr );
	for(main_reg=0;main_reg<8;main_reg++)for(main_size=2;main_size<=4;main_size+=2){
		idef(n,0xFFF8,0x0108|((main_size&4)<<4)|(main_reg<<9),i_movep_mem2reg);
		idef(n,0xFFF8,0x0188|((main_size&4)<<4)|(main_reg<<9),i_movep_reg2mem);
	}
}

static void decode1(int n) {
	cease_decode = 0;
	main_size = 1;
	defmove(0x1000, n);
}

static void decode2(int n) {
	cease_decode = 0;
	main_size = 4;
	defmove(0x2000, n);
	for(main_reg = 0; main_reg < 8; main_reg++) {
		eadef_all(n, 0xFFC0, 0x2040 | (main_reg << 9), i_movea);
	}
}

static void decode3(int n) {
	cease_decode = 0;
	main_size = 2;
	defmove(0x3000, n);
	for(main_reg = 0; main_reg < 8; main_reg++) {
		eadef_all(n, 0xFFC0, 0x3040 | (main_reg << 9), i_movea);
	}
}

static void decode4(int n) {
	cease_decode = 0;
	eadef_data_alterable(n, 0xFFC0, 0x40C0, i_move_from_sr);
	if(cputype >= 68010) {
		eadef_data_alterable(n, 0xFFC0, 0x42C0, i_move_from_ccr);
	}
	eadef_data(n, 0xFFC0, 0x44C0, i_move_to_ccr);
	eadef_data(n, 0xFFC0, 0x46C0, i_move_to_sr );
	eadef_control(n, 0xFFC0, 0x4EC0, i_jmp);
	eadef_control(n, 0xFFC0, 0x4E80, i_jsr);
	for(main_reg=0;main_reg<8;main_reg++)eadef_control(n,0xFFC0,0x41C0|(main_reg<<9),i_lea);

//	main_size=2;for(main_reg=0;main_reg<8;main_reg++)eadef_data(n,0xFFC0,0x4100|(main_reg<<9),i_chk);

	/* Bug fixed in code generation for CHK instruction */
	
	main_size=2;for(main_reg=0;main_reg<8;main_reg++)eadef_data(n,0xFFC0,0x4180|(main_reg<<9),i_chk);

	eadef_control(n,0xFFC0,0x4840,i_pea);
	for(sizedef = 0; sizedef < 3; sizedef++) {
		main_size = 1 << sizedef;
		eadef_data_alterable(n, 0xFFC0, 0x4200 | (sizedef << 6), i_clr);
		eadef_data_alterable(n, 0xFFC0, 0x4A00 | (sizedef << 6), i_tst);
	}
	idef(n, 0xFFFF, 0x4E70, i_reset);
	idef(n, 0xFFFF, 0x4E71, i_nop);
	idef(n, 0xFFFF, 0x4E72, i_stop);
	idef(n, 0xFFFF, 0x4E73, i_rte);
	idef(n, 0xFFFF, 0x4E75, i_rts);
	idef(n, 0xFFFF, 0x4E76, i_trapv);
	idef(n, 0xFFFF, 0x4E77, i_rtr);
	if(cputype >= 68010) {
		idef(n, 0xFFFF, 0x4E74, i_rtd);
		idef(n, 0xFFFF, 0x4E7A, i_movec_c_to_r);
		idef(n, 0xFFFF, 0x4E7B, i_movec_r_to_c);
	}
	main_dr=0;for(sizedef=0;sizedef<2;sizedef++){main_size=1<<(sizedef+1);eadef_control_alterable(n,0xFFC0,0x4880|(main_dr<<10)|(sizedef<<6),i_movem_control);}
	main_dr=1;for(sizedef=0;sizedef<2;sizedef++){main_size=1<<(sizedef+1);eadef_control          (n,0xFFC0,0x4880|(main_dr<<10)|(sizedef<<6),i_movem_control);}
	for(sizedef = 0; sizedef < 2; sizedef++) {
		main_size = 1 << (sizedef + 1);
		idef(n, 0xFFF8, 0x4C98 | (sizedef << 6), i_movem_postinc);
		idef(n, 0xFFF8, 0x48A0 | (sizedef << 6), i_movem_predec );
	}
	idef(n, 0xFFF8, 0x4E50, i_link);
	idef(n, 0xFFF8, 0x4E58, i_unlk);
	idef(n, 0xFFF0, 0x4E40, i_trap);
	idef(n, 0xFFF8, 0x4E60, i_move_to_usp);
	idef(n, 0xFFF8, 0x4E68, i_move_from_usp);
	idef(n, 0xFFF8, 0x4840, i_swap);
	idef(n, 0xFFF8, 0x4880, i_extbw);
	idef(n, 0xFFF8, 0x48C0, i_extwl);
	for(sizedef = 0; sizedef < 3; sizedef++) {
		main_size = 1 << sizedef;
		eadef_data_alterable(n, 0xFFC0, 0x4000 | (sizedef << 6),
			i_negx);
		eadef_data_alterable(n, 0xFFC0, 0x4400 | (sizedef << 6),
			i_neg);
		eadef_data_alterable(n, 0xFFC0, 0x4600 | (sizedef << 6),
			i_not);
	}
	eadef_data_alterable(n, 0xFFC0, 0x4800, i_nbcd);
	eadef_data_alterable(n, 0xFFC0, 0x4AC0, i_tas);
	if(cputype == 68010) idef(n, 0xFFF8, 0x4848, i_bkpt);
	idef(n, 0xFFFF, 0x4AFA, i_illegal);
	idef(n, 0xFFFF, 0x4AFB, i_illegal);
	idef(n, 0xFFFF, 0x4AFC, i_illegal);
}

static void decode5(int n) {
	cease_decode = 0;
	for(sizedef = 0; sizedef < 3; sizedef++) {
		main_size = 1 << sizedef;
		for(main_qv = 0; main_qv < 8; main_qv++) {
			word w = (sizedef << 6) | (main_qv << 9);
			eadef_alterable_nobyteaddress(n, 0xFFC0, 0x5000 | w,
				i_addq);
			eadef_alterable_nobyteaddress(n, 0xFFC0, 0x5100 | w,
				i_subq);
		}
	}
	for(main_cc = 0x2; main_cc <= 0xF; main_cc++) {
		idef(n, 0xFFF8, 0x50C8 | (main_cc << 8), i_dbcc);
	}
	idef(n, 0xFFF8, 0x51C8, i_dbra);

	/* Bug fixed in code generation for DBTr instruction */

	idef(n, 0xFFF8, 0x50C8, i_dbtr);

	main_size = 1;
	for(main_cc = 0x0; main_cc <= 0xF; main_cc++) {
		eadef_data_alterable(n, 0xFFC0, 0x50C0 | (main_cc << 8),
			i_scc);
	}
}

static void decode6(int n){
	cease_decode=0;
	idef(n,0x1FFFF,0x6000,i_bra_w);
	idef(n,0x1FFFF,0x6100,i_bsr_w);
	for(main_cc=0x2;main_cc<=0xF;main_cc++){
		idef(n,0x1FFFF,0x6000|(main_cc<<8),i_bcc_w);
	}
	idef(n,0x0FF00,0x6000,i_bra_b);
	idef(n,0x0FF00,0x6100,i_bsr_b);
	for(main_cc=0x2;main_cc<=0xF;main_cc++){
		idef(n,0x0FF00,0x6000|(main_cc<<8),i_bcc_b);
	}
}

static void decode7(int n){
	cease_decode=0;
	for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFF00,0x7000|(main_reg<<9),i_moveq);
}

static void decode8(int n){
	cease_decode=0;
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_data             (n,0xFFC0,0x8000|(main_reg<<9)|(sizedef<<6),i_or_dn );}}
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_memory_alterable (n,0xFFC0,0x8100|(main_reg<<9)|(sizedef<<6),i_or_ea );}}
	for(main_cc=0;main_cc<2;main_cc++)for(main_reg=0;main_reg<8;main_reg++)eadef_data(n,0xFFC0,0x80C0|(main_reg<<9)|(main_cc<<8),i_div);
	main_size=1;
	for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0x8100|(main_reg<<9),i_sbcd_dreg);
	for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0x8108|(main_reg<<9),i_sbcd_adec);
}

static void decode9(int n){
	cease_decode=0;
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_all_nobyteaddress(n,0xFFC0,0x9000|(main_reg<<9)|(sizedef<<6),i_sub_dn);}}
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_memory_alterable (n,0xFFC0,0x9100|(main_reg<<9)|(sizedef<<6),i_sub_ea);}}
	main_size=2;for(main_reg=0;main_reg<8;main_reg++)eadef_all(n,0xFFC0,0x90C0|(main_reg<<9),i_suba);
	main_size=4;for(main_reg=0;main_reg<8;main_reg++)eadef_all(n,0xFFC0,0x91C0|(main_reg<<9),i_suba);
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0x9100|(main_reg<<9)|(sizedef<<6),i_subx_dreg);}
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0x9108|(main_reg<<9)|(sizedef<<6),i_subx_adec);}
}

static void decodeA(int n){
	cease_decode=0;
	idef(n,0xF000,0xA000,i_aline);
}

static void decodeB(int n){
	cease_decode=0;
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_all_nobyteaddress(n,0xFFC0,0xB000|(main_reg<<9)|(sizedef<<6),i_cmp_dn);}}
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_data_alterable   (n,0xFFC0,0xB100|(main_reg<<9)|(sizedef<<6),i_eor_ea);}}
	main_size=2;for(main_reg=0;main_reg<8;main_reg++)eadef_all(n,0xFFC0,0xB0C0|(main_reg<<9),i_cmpa);
	main_size=4;for(main_reg=0;main_reg<8;main_reg++)eadef_all(n,0xFFC0,0xB1C0|(main_reg<<9),i_cmpa);
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0xB108|(sizedef<<6)|(main_reg<<9),i_cmpm);}
}

static void decodeC(int n){
	cease_decode=0;
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_data             (n,0xFFC0,0xC000|(main_reg<<9)|(sizedef<<6),i_and_dn);}}
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_memory_alterable (n,0xFFC0,0xC100|(main_reg<<9)|(sizedef<<6),i_and_ea);}}
	for(main_cc=0;main_cc<2;main_cc++)for(main_reg=0;main_reg<8;main_reg++)eadef_data(n,0xFFC0,0xC0C0|(main_reg<<9)|(main_cc<<8),i_mul);
	main_dr=0 ;main_ir=0 ;for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0xC140|(main_reg<<9),i_exg);
	main_dr=32;main_ir=32;for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0xC148|(main_reg<<9),i_exg);
	main_dr=0 ;main_ir=32;for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0xC188|(main_reg<<9),i_exg);
	main_size=1;
	for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0xC100|(main_reg<<9),i_abcd_dreg);
	for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0xC108|(main_reg<<9),i_abcd_adec);
}

static void decodeD(int n){
	cease_decode=0;
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_all_nobyteaddress(n,0xFFC0,0xD000|(main_reg<<9)|(sizedef<<6),i_add_dn);}}
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++){eadef_memory_alterable (n,0xFFC0,0xD100|(main_reg<<9)|(sizedef<<6),i_add_ea);}}
	main_size=2;for(main_reg=0;main_reg<8;main_reg++)eadef_all(n,0xFFC0,0xD0C0|(main_reg<<9),i_adda);
	main_size=4;for(main_reg=0;main_reg<8;main_reg++)eadef_all(n,0xFFC0,0xD1C0|(main_reg<<9),i_adda);
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0xD100|(main_reg<<9)|(sizedef<<6),i_addx_dreg);}
	for(sizedef=0;sizedef<3;sizedef++){main_size=1<<sizedef;for(main_reg=0;main_reg<8;main_reg++)idef(n,0xFFF8,0xD108|(main_reg<<9)|(sizedef<<6),i_addx_adec);}
}

static void decodeE(int n){
	cease_decode=0;
	for(main_reg=0;main_reg<8;main_reg++)
	for(main_dr=0;main_dr<2;main_dr++)
	for(main_ir=0;main_ir<2;main_ir++)
	for(sizedef=0;sizedef<3;sizedef++){
		main_size=1<<sizedef;
		idef(n,0xFFF8,0xE000|(main_reg<<9)|(main_dr<<8)|(sizedef<<6)|(main_ir<<5),i_asx_reg);
		idef(n,0xFFF8,0xE008|(main_reg<<9)|(main_dr<<8)|(sizedef<<6)|(main_ir<<5),i_lsx_reg);
		idef(n,0xFFF8,0xE010|(main_reg<<9)|(main_dr<<8)|(sizedef<<6)|(main_ir<<5),i_rxx_reg);
		idef(n,0xFFF8,0xE018|(main_reg<<9)|(main_dr<<8)|(sizedef<<6)|(main_ir<<5),i_rox_reg);
	}
	for(main_dr=0;main_dr<2;main_dr++){
		eadef_memory_alterable(n,0xFFC0,0xE0C0|(main_dr<<8),i_asx_mem);
		eadef_memory_alterable(n,0xFFC0,0xE2C0|(main_dr<<8),i_lsx_mem);
		eadef_memory_alterable(n,0xFFC0,0xE4C0|(main_dr<<8),i_rxx_mem);
		eadef_memory_alterable(n,0xFFC0,0xE6C0|(main_dr<<8),i_rox_mem);
	}
}

static void decodeF(int n) {
	cease_decode = 0;
	idef(n, 0xF000, 0xF000, i_fline);
}

static void (*(decodetable[16]))(int n) =
{decode0, decode1, decode2, decode3, decode4, decode5, decode6, decode7,
 decode8, decode9, decodeA, decodeB, decodeC, decodeD, decodeE, decodeF};

/***************************************************************************/

/* Build a jump table entry (including loop info for 68010) */
static void tableentry(int last, int rl) {
	if(last == -1){
		emit("dd r_illegal-top");
	} else {
		emit("dd %c%03X-top",
			((last >> 12) & 0xF) + 'K', last & 0xFFF
		);
	}
	if(rl > 1) emit("+%u", ((dword)(rl - 1)) << 24);
	emit("\n");
	if(cputype == 68010) emit("db %d\n", loopinfo[last]);
}

/* Return the next parameter (or NULL if there isn't one */
static char *getparameter(int *ip, int argc, char **argv) {
	int i;
	(*ip)++;
	i = (*ip);
	if(i >= argc) {
		fprintf(stderr, "Invalid use of %s option\n", argv[i - 1]);
		return NULL;
	}
	return argv[i];
}

void printversion(void) {
	if(!quiet)
		fprintf(stderr, "STARSCREAM version " VERSION "\n");
}

int main(int argc, char **argv) {
	int i, j, last, rl, bank;
	char *codefilename = NULL;
	char default_sourcename[10];

	/* Read options from the command line */
	for(i = 1; i < argc; i++) {
		char *a = argv[i];
		if(*a == '-') {
			a++;
			       if(!strcmp("regcall"    , a)) { use_stack = 0;
			} else if(!strcmp("stackcall"  , a)) { use_stack = 1;
			} else if(!strcmp("nohog"      , a)) { hog = 0;
			} else if(!strcmp("hog"        , a)) { hog = 1;
			} else if(!strcmp("quiet"      , a)) { quiet = 1;
			} else if(!strcmp("addressbits", a)) {
				int n;
				char *s = getparameter(&i, argc, argv);
				if(!s) return 1;
				n = atol(s);
				if(n < 1 || n > 32) {
					printversion();
					fprintf(stderr,
						"Invalid number of address "
						"bits: \"%s\"\n", argv[i]
					);
					return 1;
				}
				addressbits = n;
			} else if(!strcmp("cputype"    , a)) {
				int n;
				char *s = getparameter(&i, argc, argv);
				if(!s) {
					printversion();
					fprintf(stderr, "Invalid (missing) cputype\n");
					return 1;
				}
				n = atol(s);
				switch(n) {
				case 68000:
				case 68010:
				case 68020:
					cputype = n;
					break;
				default:
					printversion();
					fprintf(stderr,
						"Invalid CPU type: \"%s\"\n",
						argv[i]
					);
					return 1;
				}
			} else if(!strcmp("name"       , a)) {
				sourcename = getparameter(&i, argc, argv);
				if(!sourcename) {
					printversion();
					fprintf(stderr, "Invalid (missing) name\n");
					return 1;
				}
			} else {
				printversion();
				fprintf(stderr,
					"\nUnrecognized option: \"%s\"\n",
					argv[i]
				);
				return 1;
			}
		} else {
			if(codefilename) {
				printversion();
				fprintf(stderr,
					"\n\"%s\": only one output filename "
					"is allowed\n",
					argv[i]
				);
				return 1;
			}
			codefilename = argv[i];
		}
	}

	printversion();

	if(!codefilename) {
		fprintf(stderr, "usage: %s outputfile [options]\n", argv[0]);
		fprintf(stderr, "see STARDOC.TXT for details\n");
		return 1;
	}

	/* Set default options where applicable */
	if(use_stack   < 0) use_stack = 1;
	if(hog         < 0) hog       = 0;
	if(cputype     < 0) cputype   = 68000;
	if(addressbits < 0) {
		if(cputype <= 68010) addressbits = 24;
		else                 addressbits = 32;
	}
	if(!sourcename) {
		sprintf(default_sourcename, "s%d", cputype);
		sourcename = default_sourcename;
	}

	/* Prepare to generate the code file */
	linenum = 0;
	fflush(stdout);
	fflush(stderr);
	codefile = fopen(codefilename, "w");
	if(!codefile) {
		perror(codefilename);
		return 1;
	}

	if(!quiet) {
		fprintf(stderr, "Generating \"%s\" with the following options:\n",
			codefilename
		);
		optiondump(stderr, " *  ");
	}
	prefixes();
	for(i = 0; i < 0x10000; i++) rproc[i] = -1;
	/* Clear loop timings for 68010 */
	if(cputype == 68010) {
		for(i = 0; i < 0x10000; i++) loopinfo[i] = 0xDB;
	}

	/*
	** Decode instructions
	** (this is where the vast majority of the code is emitted)
	*/
	if(!quiet)
		fprintf(stderr, "Decoding instructions: ");
	for(bank = 0; bank <= 0xF; bank++) {
		int bankend = (bank + 1) << 12;
		void (*decoderoutine)(int n) = decodetable[bank];
		if(!quiet) {
			fprintf(stderr, "%X", bank);
			fflush(stderr);
		}
		for(i = bank << 12; i < bankend; i++) decoderoutine(i);
	}
	if(!quiet)
		fprintf(stderr, " done\n");

	/*
	** Build the main jump table (all CPUs) / loop info table (68010)
	*/
	if(!quiet)
		fprintf(stderr, "Building table: ");
	emit("section .bss\n");
	emit("bits 32\n");
	align(4);
	emit("__jmptbl resb 262144\n");
	if(cputype == 68010) emit("__looptbl resb 65536\n");
	emit("section .data\n");
	emit("bits 32\n");
	align(4);
	emit("__jmptblcomp:\n");
	last = -2;
	rl = 0;
	for(i = 0; i < 0x10000; i++) {
		j = rproc[i];
		if(j == last){
			if(rl == 256) {
				tableentry(last, rl);
				rl = 1;
			} else {
				rl++;
			}
		} else {
			if(rl) tableentry(last, rl);
			rl = 1;
			last = j;
		}
	}
	tableentry(last, rl);
	align(4);

	/* Finish up */
	suffixes();
	if(!quiet) {
		fprintf(stderr, "done\n");
		fprintf(stderr, "routine_counter = %d\n", routine_counter);
	}
	fclose(codefile);
	return 0;
}
