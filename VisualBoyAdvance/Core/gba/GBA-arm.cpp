#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include <string.h>

#include "GBA.h"
#include "GBAcpu.h"
#include "GBAinline.h"
#include "Globals.h"
#include "EEprom.h"
#include "Flash.h"
#include "Sound.h"
#include "Sram.h"
#include "bios.h"
#include "Cheats.h"
#include "../NLS.h"
#include "elf.h"
#include "../Util.h"
#include "../System.h"
#include "agbprint.h"
#ifdef PROFILING
#include "prof/prof.h"
#endif

#ifdef _MSC_VER
 // Disable "empty statement" warnings
 #pragma warning(disable: 4390)
 // Visual C's inline assembler treats "offset" as a reserved word, so we
 // tell it otherwise.  If you want to use it, write "OFFSET" in capitals.
 #define offset offset_
#endif

///////////////////////////////////////////////////////////////////////////

static int clockTicks;

static INSN_REGPARM void armUnknownInsn(u32 opcode)
{
#ifdef GBA_LOGGING
    if (systemVerbose & VERBOSE_UNDEFINED) {
        log("Undefined ARM instruction %08x at %08x\n", opcode,
            armNextPC-4);
    }
#endif
    CPUUndefinedException();
}

#ifdef BKPT_SUPPORT
static INSN_REGPARM void armBreakpoint(u32 opcode)
{
    reg[15].I -= 4;
    armNextPC -= 4;
    dbgSignal(5, (opcode & 0x0f) | ((opcode>>4) & 0xfff0));
    clockTicks = -1;
}
#endif


// Subroutine to count instructions (for debugging/optimizing)
//#define INSN_COUNTER  // comment out if you don't want it
#ifdef INSN_COUNTER
static void count(u32 opcode, int cond_res)
{
    static int insncount = 0;    // number of insns seen
    static int executed = 0;     // number of insns executed
    static int mergewith[4096];  // map instructions to routines
    static int count[4096];      // count of each 12-bit code
    int index = ((opcode>>16)&0xFF0) | ((opcode>>4)&0x0F);
    static FILE *outfile = NULL;

    if (!insncount) {
        for (int i = 0; i < 4096; i++) {
            for (int j = 0; j < i; j++) {
                if (armInsnTable[i] == armInsnTable[j])
                    break;
            }
            mergewith[i] = j;
        }
        outfile = fopen("VBA-armcount.txt", "w");
    }
    if (cond_res) {
        count[mergewith[index]]++;
        executed++;
    }
    insncount++;
    if (outfile && insncount%1000000 == 0) {
        fprintf(outfile, "Total instructions: %d\n", insncount);
        fprintf(outfile, "Instructions executed: %d\n", executed);
        for (int i = 0; i < 4096; i++) {
            if (count[i])
                fprintf(outfile, "arm%03X: %d\n", i, count[i]);
        }
    }
}
#endif

// Common macros //////////////////////////////////////////////////////////

#ifdef BKPT_SUPPORT
#define CONSOLE_OUTPUT(a,b) do { \
    if ((opcode == 0xe0000000) && (reg[0].I == 0xC0DED00D)) {   \
        dbgOutput((a), (b));                                    \
} while (0)
#else
#define CONSOLE_OUTPUT(a,b)  /* nothing */
#endif

#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)

// The following macros are used for optimization; any not defined for a
// particular compiler/CPU combination default to the C core versions.
//
//    ALU_INIT_C:   Used at the beginning of ALU instructions (AND/EOR/...).
//    (ALU_INIT_NC) Can consist of variable declarations, like the C core,
//                  or the start of a continued assembly block, like the
//                  x86-optimized version.  The _C version is used when the
//                  carry flag from the shift operation is needed (logical
//                  operations that set condition codes, like ANDS); the
//                  _NC version is used when the carry result is ignored.
//    VALUE_XXX: Retrieve the second operand's value for an ALU instruction.
//               The _C and _NC versions are used the same way as ALU_INIT.
//    OP_XXX: ALU operations.  XXX is the instruction name.
//    ALU_FINISH: Appended to all ALU instructions.  Usually empty, but if
//                ALU_INIT started a block ALU_FINISH can be used to end it
//                (as with the asm(...) statement in the x86 core).
//    SETCOND_NONE: Used in multiply instructions in place of SETCOND_MUL
//                  when the condition codes are not set.  Usually empty.
//    SETCOND_MUL: Used in multiply instructions to set the condition codes.
//    ROR_IMM_MSR: Used to rotate the immediate operand for MSR.
//    ROR_OFFSET: Used to rotate the `offset' parameter for LDR and STR
//                instructions.
//    RRX_OFFSET: Used to rotate (RRX) the `offset' parameter for LDR and
//                STR instructions.

#ifndef C_CORE

#if 0  // definitions have changed
//#ifdef __POWERPC__
            #define OP_SUBS \
            {\
                register int Flags;                             \
                register int Result;                            \
                asm volatile("subco. %0, %2, %3\n"              \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (reg[base].I),                \
                              "r" (value)                       \
                            );                                  \
                reg[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_RSBS \
            {\
                register int Flags;                             \
                register int Result;                            \
                asm volatile("subfco. %0, %2, %3\n"             \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (reg[base].I),                \
                              "r" (value)                       \
                            );                                  \
                reg[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_ADDS \
            {\
                register int Flags;                             \
                register int Result;                            \
                asm volatile("addco. %0, %2, %3\n"              \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (reg[base].I),                \
                              "r" (value)                       \
                            );                                  \
                reg[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_ADCS \
            {\
                register int Flags;                             \
                register int Result;                            \
                asm volatile("mtspr xer, %4\n"                  \
                             "addeo. %0, %2, %3\n"              \
                             "mcrxr cr1\n"                      \
                             "mfcr      %1\n"                   \
                             : "=r" (Result),                   \
                               "=r" (Flags)                     \
                             : "r" (reg[base].I),               \
                               "r" (value),                     \
                               "r" (C_FLAG << 29)               \
                             );                                 \
                reg[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_SBCS \
            {\
                register int Flags;                             \
                register int Result;                            \
                asm volatile("mtspr xer, %4\n"                  \
                             "subfeo. %0, %3, %2\n"             \
                             "mcrxr cr1\n"                      \
                             "mfcr      %1\n"                   \
                             : "=r" (Result),                   \
                               "=r" (Flags)                     \
                             : "r" (reg[base].I),               \
                               "r" (value),                     \
                               "r" (C_FLAG << 29)               \
                             );                                 \
                reg[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_RSCS \
            {\
                register int Flags;                             \
                register int Result;                            \
                asm volatile("mtspr xer, %4\n"                  \
                             "subfeo. %0, %2, %3\n"             \
                             "mcrxr cr1\n"                      \
                             "mfcr      %1\n"                   \
                             : "=r" (Result),                   \
                               "=r" (Flags)                     \
                             : "r" (reg[base].I),               \
                               "r" (value),                     \
                               "r" (C_FLAG << 29)               \
                             );                                 \
                reg[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_CMP \
            {\
                register int Flags;                             \
                register int Result;                            \
                asm volatile("subco. %0, %2, %3\n"              \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (reg[base].I),                \
                              "r" (value)                       \
                            );                                  \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_CMN \
            {\
                register int Flags;                             \
                register int Result;                            \
                asm volatile("addco. %0, %2, %3\n"              \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (reg[base].I),                \
                              "r" (value)                       \
                            );                                  \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }

#else  // !__POWERPC__

// Macros to emit instructions in the format used by the particular compiler.
// We use GNU assembler syntax: "op src, dest" rather than "op dest, src"

#ifdef __GNUC__
 #define ALU_HEADER           asm("mov %%ecx, %%edi; "
 #define ALU_TRAILER          : "=D" (opcode) : "c" (opcode) : "eax", "edx", "esi")
 #define EMIT0(op)            #op"; "
 #define EMIT1(op,arg)        #op" "arg"; "
 #define EMIT2(op,src,dest)   #op" "src", "dest"; "
 #define CONST(val)           "$"#val
 #define ASMVAR(cvar)         ASMVAR2 (__USER_LABEL_PREFIX__, cvar)
 #define ASMVAR2(prefix,cvar) STRING (prefix) cvar
 #define STRING(x)            #x
 #define VAR(var)             ASMVAR(#var)
 #define VARL(var)            ASMVAR(#var)
 #define REGREF1(index)       ASMVAR("reg("index")")
 #define REGREF2(index,scale) ASMVAR("reg(,"index","#scale")")
 #define LABEL(n)             #n": "
 #define LABELREF(n,dir)      #n#dir
 #define al "%%al"
 #define ah "%%ah"
 #define eax "%%eax"
 #define bl "%%bl"
 #define bh "%%bh"
 #define ebx "%%ebx"
 #define cl "%%cl"
 #define ch "%%ch"
 #define ecx "%%ecx"
 #define dl "%%dl"
 #define dh "%%dh"
 #define edx "%%edx"
 #define esp "%%esp"
 #define ebp "%%ebp"
 #define esi "%%esi"
 #define edi "%%edi"
 #define movzx movzb
#else
 #define ALU_HEADER           __asm { __asm mov ecx, opcode
 #define ALU_TRAILER          }
 #define EMIT0(op)            __asm op
 #define EMIT1(op,arg)        __asm op arg
 #define EMIT2(op,src,dest)   __asm op dest, src
 #define CONST(val)           val
 #define VAR(var)             var
 #define VARL(var)            dword ptr var
 #define REGREF1(index)       reg[index]
 #define REGREF2(index,scale) reg[index*scale]
 #define LABEL(n)             __asm l##n:
 #define LABELREF(n,dir)      l##n
#endif

//X//#ifndef _MSC_VER
// ALU op register usage:
//    EAX -> 2nd operand value, result (RSB/RSC)
//    EBX -> C_OUT (carry flag from shift/rotate)
//    ECX -> opcode (input), shift/rotate count
//    EDX -> Rn (base) value, result (all except RSB/RSC)
//    ESI -> Rd (destination) index * 4

// Helper macros for loading value / shift count
#define VALUE_LOAD_IMM \
        EMIT2(and, CONST(0x0F), eax)            \
        EMIT2(mov, REGREF2(eax,4), eax)         \
        EMIT2(shr, CONST(7), ecx)               \
        EMIT2(and, CONST(0x1F), ecx)
#define VALUE_LOAD_REG \
        EMIT2(and, CONST(0x0F), eax)            \
        EMIT2(mov, REGREF2(eax,4), eax)         \
        EMIT2(movzx, ch, ecx)                   \
        EMIT2(and, CONST(0x0F), ecx)            \
        EMIT2(mov, REGREF2(ecx,4), ecx)

// Helper macros for setting flags
#define SETCOND_LOGICAL \
    EMIT1(sets, VAR(N_FLAG))            \
    EMIT1(setz, VAR(Z_FLAG))            \
    EMIT2(mov, bl, VAR(C_FLAG))
#define SETCOND_ADD \
    EMIT1(sets, VAR(N_FLAG))            \
    EMIT1(setz, VAR(Z_FLAG))            \
    EMIT1(seto, VAR(V_FLAG))            \
    EMIT1(setc, VAR(C_FLAG))
#define SETCOND_SUB \
    EMIT1(sets, VAR(N_FLAG))            \
    EMIT1(setz, VAR(Z_FLAG))            \
    EMIT1(seto, VAR(V_FLAG))            \
    EMIT1(setnc, VAR(C_FLAG))

// ALU initialization
#define ALU_INIT(LOAD_C_FLAG) \
    ALU_HEADER                          \
    LOAD_C_FLAG                         \
    EMIT2(mov, ecx, edx)                \
    EMIT2(shr, CONST(14), edx)          \
    EMIT2(mov, ecx, eax)                \
    EMIT2(mov, ecx, esi)                \
    EMIT2(shr, CONST(10), esi)          \
    EMIT2(and, CONST(0x3C), edx)        \
    EMIT2(mov, REGREF1(edx), edx)       \
    EMIT2(and, CONST(0x3C), esi)

#define LOAD_C_FLAG_YES EMIT2(mov, VAR(C_FLAG), bl)
#define LOAD_C_FLAG_NO  /*nothing*/
#define ALU_INIT_C ALU_INIT(LOAD_C_FLAG_YES)
#define ALU_INIT_NC ALU_INIT(LOAD_C_FLAG_NO)

// Macros to load the value operand for an ALU op; these all set N/Z
// according to the value

// OP Rd,Rb,Rm LSL #
#define VALUE_LSL_IMM_C \
    VALUE_LOAD_IMM                      \
    EMIT1(jnz, LABELREF(1,f))           \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(shl, cl, eax)                 \
    EMIT1(setc, bl)                     \
    LABEL(0)
#define VALUE_LSL_IMM_NC \
    VALUE_LOAD_IMM                      \
    EMIT2(shl, cl, eax)

// OP Rd,Rb,Rm LSL Rs
#define VALUE_LSL_REG_C \
    VALUE_LOAD_REG                      \
    EMIT2(test, cl, cl)                 \
    EMIT1(jz, LABELREF(0,f))            \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(je, LABELREF(1,f))            \
    EMIT1(ja, LABELREF(2,f))            \
    EMIT2(shl, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(test, CONST(1), al)           \
    EMIT1(setnz, bl)                    \
    EMIT2(xor, eax, eax)                \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(2)                            \
    EMIT2(xor, ebx, ebx)                \
    EMIT2(xor, eax, eax)                \
    LABEL(0)
#define VALUE_LSL_REG_NC \
    VALUE_LOAD_REG                      \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(jae, LABELREF(1,f))           \
    EMIT2(shl, cl, eax)                 \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(xor, eax, eax)                \
    LABEL(0)

// OP Rd,Rb,Rm LSR #
#define VALUE_LSR_IMM_C \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1,f))            \
    EMIT2(shr, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(test, eax, eax)               \
    EMIT1(sets, bl)                     \
    EMIT2(xor, eax, eax)                \
    LABEL(0)
#define VALUE_LSR_IMM_NC \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1,f))            \
    EMIT2(shr, cl, eax)                 \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(xor, eax, eax)                \
    LABEL(0)

// OP Rd,Rb,Rm LSR Rs
#define VALUE_LSR_REG_C \
    VALUE_LOAD_REG                      \
    EMIT2(test, cl, cl)                 \
    EMIT1(jz, LABELREF(0,f))            \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(je, LABELREF(1,f))            \
    EMIT1(ja, LABELREF(2,f))            \
    EMIT2(shr, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(test, eax, eax)               \
    EMIT1(sets, bl)                     \
    EMIT2(xor, eax, eax)                \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(2)                            \
    EMIT2(xor, ebx, ebx)                \
    EMIT2(xor, eax, eax)                \
    LABEL(0)
#define VALUE_LSR_REG_NC \
    VALUE_LOAD_REG                      \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(jae, LABELREF(1,f))           \
    EMIT2(shr, cl, eax)                 \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(xor, eax, eax)                \
    LABEL(0)

// OP Rd,Rb,Rm ASR #
#define VALUE_ASR_IMM_C \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1,f))            \
    EMIT2(sar, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(sar, CONST(31), eax)          \
    EMIT1(sets, bl)                     \
    LABEL(0)
#define VALUE_ASR_IMM_NC \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1,f))            \
    EMIT2(sar, cl, eax)                 \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(sar, CONST(31), eax)          \
    LABEL(0)

// OP Rd,Rb,Rm ASR Rs
#define VALUE_ASR_REG_C \
    VALUE_LOAD_REG                      \
    EMIT2(test, cl, cl)                 \
    EMIT1(jz, LABELREF(0,f))            \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(jae, LABELREF(1,f))           \
    EMIT2(sar, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(sar, CONST(31), eax)          \
    EMIT1(sets, bl)                     \
    LABEL(0)
#define VALUE_ASR_REG_NC \
    VALUE_LOAD_REG                      \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(jae, LABELREF(1,f))           \
    EMIT2(sar, cl, eax)                 \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(sar, CONST(31), eax)          \
    LABEL(0)

// OP Rd,Rb,Rm ROR #
#define VALUE_ROR_IMM_C \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1,f))            \
    EMIT2(ror, cl, eax)                 \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(bt, CONST(0), ebx)            \
    EMIT2(rcr, CONST(1), eax)           \
    LABEL(0)                            \
    EMIT1(setc, bl)
#define VALUE_ROR_IMM_NC \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1,f))            \
    EMIT2(ror, cl, eax)                 \
    EMIT1(jmp, LABELREF(0,f))           \
    LABEL(1)                            \
    EMIT2(bt, CONST(0), VARL(C_FLAG))   \
    EMIT2(rcr, CONST(1), eax)           \
    LABEL(0)

// OP Rd,Rb,Rm ROR Rs
#define VALUE_ROR_REG_C \
    VALUE_LOAD_REG                      \
    EMIT2(bt, CONST(0), ebx)            \
    EMIT2(ror, cl, eax)                 \
    EMIT1(setc, bl)
#define VALUE_ROR_REG_NC \
    VALUE_LOAD_REG                      \
    EMIT2(ror, cl, eax)

// OP Rd,Rb,# ROR #
#define VALUE_IMM_C \
    EMIT2(movzx, ch, ecx)               \
    EMIT2(add, ecx, ecx)                \
    EMIT2(movzx, al, eax)               \
    EMIT2(bt, CONST(0), ebx)            \
    EMIT2(ror, cl, eax)                 \
    EMIT1(setc, bl)
#define VALUE_IMM_NC \
    EMIT2(movzx, ch, ecx)               \
    EMIT2(add, ecx, ecx)                \
    EMIT2(movzx, al, eax)               \
    EMIT2(ror, cl, eax)

// Macros to perform ALU ops

// Set condition codes iff the destination register is not R15 (PC)
#define CHECK_PC(OP, SETCOND) \
    EMIT2(cmp, CONST(0x3C), esi)        \
    EMIT1(je, LABELREF(8,f))            \
    OP SETCOND                          \
    EMIT1(jmp, LABELREF(9,f))           \
    LABEL(8)                            \
    OP                                  \
    LABEL(9)

#define OP_AND \
    EMIT2(and, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_ANDS   CHECK_PC(OP_AND, SETCOND_LOGICAL)
#define OP_EOR \
    EMIT2(xor, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_EORS   CHECK_PC(OP_EOR, SETCOND_LOGICAL)
#define OP_SUB \
    EMIT2(sub, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_SUBS   CHECK_PC(OP_SUB, SETCOND_SUB)
#define OP_RSB \
    EMIT2(sub, edx, eax)                \
    EMIT2(mov, eax, REGREF1(esi))
#define OP_RSBS   CHECK_PC(OP_RSB, SETCOND_SUB)
#define OP_ADD \
    EMIT2(add, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_ADDS   CHECK_PC(OP_ADD, SETCOND_ADD)
#define OP_ADC \
    EMIT2(bt, CONST(0), VARL(C_FLAG))   \
    EMIT2(adc, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_ADCS   CHECK_PC(OP_ADC, SETCOND_ADD)
#define OP_SBC \
    EMIT2(bt, CONST(0), VARL(C_FLAG))   \
    EMIT0(cmc)                          \
    EMIT2(sbb, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_SBCS   CHECK_PC(OP_SBC, SETCOND_SUB)
#define OP_RSC \
    EMIT2(bt, CONST(0), VARL(C_FLAG))   \
    EMIT0(cmc)                          \
    EMIT2(sbb, edx, eax)                \
    EMIT2(mov, eax, REGREF1(esi))
#define OP_RSCS   CHECK_PC(OP_RSC, SETCOND_SUB)
#define OP_TST \
    EMIT2(and, eax, edx)                \
    SETCOND_LOGICAL
#define OP_TEQ \
    EMIT2(xor, eax, edx)                \
    SETCOND_LOGICAL
#define OP_CMP \
    EMIT2(sub, eax, edx)                \
    SETCOND_SUB
#define OP_CMN \
    EMIT2(add, eax, edx)                \
    SETCOND_ADD
#define OP_ORR \
    EMIT2(or, eax, edx)                 \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_ORRS   CHECK_PC(OP_ORR, SETCOND_LOGICAL)
#define OP_MOV \
    EMIT2(mov, eax, REGREF1(esi))
#define OP_MOVS   CHECK_PC(EMIT2(test,eax,eax) EMIT2(mov,eax,REGREF1(esi)), SETCOND_LOGICAL)
#define OP_BIC \
    EMIT1(not, eax)                     \
    EMIT2(and, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_BICS   CHECK_PC(OP_BIC, SETCOND_LOGICAL)
#define OP_MVN \
    EMIT1(not, eax)                     \
    EMIT2(mov, eax, REGREF1(esi))
#define OP_MVNS   CHECK_PC(OP_MVN, SETCOND_LOGICAL)

// ALU cleanup macro
#define ALU_FINISH  ALU_TRAILER

// End of ALU macros
//X//#endif //_MSC_VER

#ifdef __GNUC__

#define ROR_IMM_MSR \
    asm ("ror %%cl, %%eax;"             \
         : "=a" (value)                 \
         : "a" (opcode & 0xFF), "c" (shift));

#define ROR_OFFSET \
    asm("ror %%cl, %0"                  \
        : "=r" (offset)                 \
        : "0" (offset), "c" (shift));

#define RRX_OFFSET \
    asm(EMIT2(btl,CONST(0),VAR(C_FLAG)) \
        "rcr $1, %0"                    \
        : "=r" (offset)                 \
        : "0" (offset));

#else  // !__GNUC__, i.e. Visual C++

#define ROR_IMM_MSR \
    __asm {                             \
        __asm mov ecx, shift            \
        __asm ror value, cl             \
     }


#define ROR_OFFSET \
    __asm {                             \
        __asm mov ecx, shift            \
        __asm ror offset, cl            \
    }

#define RRX_OFFSET \
    __asm {                             \
        __asm bt dword ptr C_FLAG, 0    \
        __asm rcr offset, 1             \
    }

#endif  // !__GNUC__

#endif  // !__POWERPC__
#endif  // !C_CORE

// C core

#define C_SETCOND_LOGICAL \
    N_FLAG = ((s32)res < 0) ? true : false;             \
    Z_FLAG = (res == 0) ? true : false;                 \
    C_FLAG = C_OUT;
#define C_SETCOND_ADD \
    N_FLAG = ((s32)res < 0) ? true : false;             \
    Z_FLAG = (res == 0) ? true : false;                 \
    V_FLAG = ((NEG(lhs) & NEG(rhs) & POS(res)) |        \
              (POS(lhs) & POS(rhs) & NEG(res))) ? true : false;\
    C_FLAG = ((NEG(lhs) & NEG(rhs)) |                   \
              (NEG(lhs) & POS(res)) |                   \
              (NEG(rhs) & POS(res))) ? true : false;
#define C_SETCOND_SUB \
    N_FLAG = ((s32)res < 0) ? true : false;             \
    Z_FLAG = (res == 0) ? true : false;                 \
    V_FLAG = ((NEG(lhs) & POS(rhs) & POS(res)) |        \
              (POS(lhs) & NEG(rhs) & NEG(res))) ? true : false;\
    C_FLAG = ((NEG(lhs) & POS(rhs)) |                   \
              (NEG(lhs) & POS(res)) |                   \
              (POS(rhs) & POS(res))) ? true : false;

#ifndef ALU_INIT_C
 #define ALU_INIT_C \
    int dest __attribute__((unused)) = (opcode>>12) & 15; \
    bool C_OUT = C_FLAG;                                \
    u32 value;
#endif
// OP Rd,Rb,Rm LSL #
#ifndef VALUE_LSL_IMM_C
 #define VALUE_LSL_IMM_C \
    unsigned int shift = (opcode >> 7) & 0x1F;          \
    if (LIKELY(!shift)) {  /* LSL #0 most common? */    \
        value = reg[opcode & 0x0F].I;                   \
    } else {                                            \
        u32 v = reg[opcode & 0x0F].I;                   \
        C_OUT = (v >> (32 - shift)) & 1 ? true : false; \
        value = v << shift;                             \
    }
#endif
// OP Rd,Rb,Rm LSL Rs
#ifndef VALUE_LSL_REG_C
 #define VALUE_LSL_REG_C \
    unsigned int shift = reg[(opcode >> 8)&15].B.B0;    \
    if (LIKELY(shift)) {                                \
        if (shift == 32) {                              \
            value = 0;                                  \
            C_OUT = (reg[opcode & 0x0F].I & 1 ? true : false);\
        } else if (LIKELY(shift < 32)) {                \
            u32 v = reg[opcode & 0x0F].I;               \
            C_OUT = (v >> (32 - shift)) & 1 ? true : false;\
            value = v << shift;                         \
        } else {                                        \
            value = 0;                                  \
            C_OUT = false;                              \
        }                                               \
    } else {                                            \
        value = reg[opcode & 0x0F].I;                   \
    }
#endif
// OP Rd,Rb,Rm LSR #
#ifndef VALUE_LSR_IMM_C
 #define VALUE_LSR_IMM_C \
    unsigned int shift = (opcode >> 7) & 0x1F;          \
    if (LIKELY(shift)) {                                \
        u32 v = reg[opcode & 0x0F].I;                   \
        C_OUT = (v >> (shift - 1)) & 1 ? true : false;  \
        value = v >> shift;                             \
    } else {                                            \
        value = 0;                                      \
        C_OUT = (reg[opcode & 0x0F].I & 0x80000000) ? true : false;\
    }
#endif
// OP Rd,Rb,Rm LSR Rs
#ifndef VALUE_LSR_REG_C
 #define VALUE_LSR_REG_C \
    unsigned int shift = reg[(opcode >> 8)&15].B.B0;    \
    if (LIKELY(shift)) {                                \
        if (shift == 32) {                              \
            value = 0;                                  \
            C_OUT = (reg[opcode & 0x0F].I & 0x80000000 ? true : false);\
        } else if (LIKELY(shift < 32)) {                \
            u32 v = reg[opcode & 0x0F].I;               \
            C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
            value = v >> shift;                         \
        } else {                                        \
            value = 0;                                  \
            C_OUT = false;                              \
        }                                               \
    } else {                                            \
        value = reg[opcode & 0x0F].I;                   \
    }
#endif
// OP Rd,Rb,Rm ASR #
#ifndef VALUE_ASR_IMM_C
 #define VALUE_ASR_IMM_C \
    unsigned int shift = (opcode >> 7) & 0x1F;          \
    if (LIKELY(shift)) {                                \
        /* VC++ BUG: u32 v; (s32)v>>n is optimized to shr! */ \
        s32 v = reg[opcode & 0x0F].I;                   \
        C_OUT = (v >> (int)(shift - 1)) & 1 ? true : false;\
        value = v >> (int)shift;                        \
    } else {                                            \
        if (reg[opcode & 0x0F].I & 0x80000000) {        \
            value = 0xFFFFFFFF;                         \
            C_OUT = true;                               \
        } else {                                        \
            value = 0;                                  \
            C_OUT = false;                              \
        }                                               \
    }
#endif
// OP Rd,Rb,Rm ASR Rs
#ifndef VALUE_ASR_REG_C
 #define VALUE_ASR_REG_C \
    unsigned int shift = reg[(opcode >> 8)&15].B.B0;    \
    if (LIKELY(shift < 32)) {                           \
        if (LIKELY(shift)) {                            \
            s32 v = reg[opcode & 0x0F].I;               \
            C_OUT = (v >> (int)(shift - 1)) & 1 ? true : false;\
            value = v >> (int)shift;                    \
        } else {                                        \
            value = reg[opcode & 0x0F].I;               \
        }                                               \
    } else {                                            \
        if (reg[opcode & 0x0F].I & 0x80000000) {        \
            value = 0xFFFFFFFF;                         \
            C_OUT = true;                               \
        } else {                                        \
            value = 0;                                  \
            C_OUT = false;                              \
        }                                               \
    }
#endif
// OP Rd,Rb,Rm ROR #
#ifndef VALUE_ROR_IMM_C
 #define VALUE_ROR_IMM_C \
    unsigned int shift = (opcode >> 7) & 0x1F;          \
    if (LIKELY(shift)) {                                \
        u32 v = reg[opcode & 0x0F].I;                   \
        C_OUT = (v >> (shift - 1)) & 1 ? true : false;  \
        value = ((v << (32 - shift)) |                  \
                 (v >> shift));                         \
    } else {                                            \
        u32 v = reg[opcode & 0x0F].I;                   \
        C_OUT = (v & 1) ? true : false;                 \
        value = ((v >> 1) |                             \
                 (C_FLAG << 31));                       \
    }
#endif
// OP Rd,Rb,Rm ROR Rs
#ifndef VALUE_ROR_REG_C
 #define VALUE_ROR_REG_C \
    unsigned int shift = reg[(opcode >> 8)&15].B.B0;    \
    if (LIKELY(shift & 0x1F)) {                         \
        u32 v = reg[opcode & 0x0F].I;                   \
        C_OUT = (v >> (shift - 1)) & 1 ? true : false;  \
        value = ((v << (32 - shift)) |                  \
                 (v >> shift));                         \
    } else {                                            \
        value = reg[opcode & 0x0F].I;                   \
        if (shift)                                      \
            C_OUT = (value & 0x80000000 ? true : false);\
    }
#endif
// OP Rd,Rb,# ROR #
#ifndef VALUE_IMM_C
 #define VALUE_IMM_C \
    int shift = (opcode & 0xF00) >> 7;                  \
    if (UNLIKELY(shift)) {                              \
        u32 v = opcode & 0xFF;                          \
        C_OUT = (v >> (shift - 1)) & 1 ? true : false;  \
        value = ((v << (32 - shift)) |                  \
                 (v >> shift));                         \
    } else {                                            \
        value = opcode & 0xFF;                          \
    }
#endif

// Make the non-carry versions default to the carry versions
// (this is fine for C--the compiler will optimize the dead code out)
#ifndef ALU_INIT_NC
 #define ALU_INIT_NC ALU_INIT_C
#endif
#ifndef VALUE_LSL_IMM_NC
 #define VALUE_LSL_IMM_NC VALUE_LSL_IMM_C
#endif
#ifndef VALUE_LSL_REG_NC
 #define VALUE_LSL_REG_NC VALUE_LSL_REG_C
#endif
#ifndef VALUE_LSR_IMM_NC
 #define VALUE_LSR_IMM_NC VALUE_LSR_IMM_C
#endif
#ifndef VALUE_LSR_REG_NC
 #define VALUE_LSR_REG_NC VALUE_LSR_REG_C
#endif
#ifndef VALUE_ASR_IMM_NC
 #define VALUE_ASR_IMM_NC VALUE_ASR_IMM_C
#endif
#ifndef VALUE_ASR_REG_NC
 #define VALUE_ASR_REG_NC VALUE_ASR_REG_C
#endif
#ifndef VALUE_ROR_IMM_NC
 #define VALUE_ROR_IMM_NC VALUE_ROR_IMM_C
#endif
#ifndef VALUE_ROR_REG_NC
 #define VALUE_ROR_REG_NC VALUE_ROR_REG_C
#endif
#ifndef VALUE_IMM_NC
 #define VALUE_IMM_NC VALUE_IMM_C
#endif

#define C_CHECK_PC(SETCOND) if (LIKELY(dest != 15)) { SETCOND }
#ifndef OP_AND
 #define OP_AND \
    u32 res = reg[(opcode>>16)&15].I & value;           \
    reg[dest].I = res;
#endif
#ifndef OP_ANDS
 #define OP_ANDS   OP_AND C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_EOR
 #define OP_EOR \
    u32 res = reg[(opcode>>16)&15].I ^ value;           \
    reg[dest].I = res;
#endif
#ifndef OP_EORS
 #define OP_EORS   OP_EOR C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_SUB
 #define OP_SUB \
    u32 lhs = reg[(opcode>>16)&15].I;                   \
    u32 rhs = value;                                    \
    u32 res = lhs - rhs;                                \
    reg[dest].I = res;
#endif
#ifndef OP_SUBS
 #define OP_SUBS   OP_SUB C_CHECK_PC(C_SETCOND_SUB)
#endif
#ifndef OP_RSB
 #define OP_RSB \
    u32 lhs = reg[(opcode>>16)&15].I;                   \
    u32 rhs = value;                                    \
    u32 res = rhs - lhs;                                \
    reg[dest].I = res;
#endif
#ifndef OP_RSBS
 #define OP_RSBS   OP_RSB C_CHECK_PC(C_SETCOND_SUB)
#endif
#ifndef OP_ADD
 #define OP_ADD \
    u32 lhs = reg[(opcode>>16)&15].I;                   \
    u32 rhs = value;                                    \
    u32 res = lhs + rhs;                                \
    reg[dest].I = res;
#endif
#ifndef OP_ADDS
 #define OP_ADDS   OP_ADD C_CHECK_PC(C_SETCOND_ADD)
#endif
#ifndef OP_ADC
 #define OP_ADC \
    u32 lhs = reg[(opcode>>16)&15].I;                   \
    u32 rhs = value;                                    \
    u32 res = lhs + rhs + (u32)C_FLAG;                  \
    reg[dest].I = res;
#endif
#ifndef OP_ADCS
 #define OP_ADCS   OP_ADC C_CHECK_PC(C_SETCOND_ADD)
#endif
#ifndef OP_SBC
 #define OP_SBC \
    u32 lhs = reg[(opcode>>16)&15].I;                   \
    u32 rhs = value;                                    \
    u32 res = lhs - rhs - !((u32)C_FLAG);               \
    reg[dest].I = res;
#endif
#ifndef OP_SBCS
 #define OP_SBCS   OP_SBC C_CHECK_PC(C_SETCOND_SUB)
#endif
#ifndef OP_RSC
 #define OP_RSC \
    u32 lhs = reg[(opcode>>16)&15].I;                   \
    u32 rhs = value;                                    \
    u32 res = rhs - lhs - !((u32)C_FLAG);               \
    reg[dest].I = res;
#endif
#ifndef OP_RSCS
 #define OP_RSCS   OP_RSC C_CHECK_PC(C_SETCOND_SUB)
#endif
#ifndef OP_TST
 #define OP_TST \
    u32 res = reg[(opcode >> 16) & 0x0F].I & value;     \
    C_SETCOND_LOGICAL;
#endif
#ifndef OP_TEQ
 #define OP_TEQ \
    u32 res = reg[(opcode >> 16) & 0x0F].I ^ value;     \
    C_SETCOND_LOGICAL;
#endif
#ifndef OP_CMP
 #define OP_CMP \
    u32 lhs = reg[(opcode>>16)&15].I;                   \
    u32 rhs = value;                                    \
    u32 res = lhs - rhs;                                \
    C_SETCOND_SUB;
#endif
#ifndef OP_CMN
 #define OP_CMN \
    u32 lhs = reg[(opcode>>16)&15].I;                   \
    u32 rhs = value;                                    \
    u32 res = lhs + rhs;                                \
    C_SETCOND_ADD;
#endif
#ifndef OP_ORR
 #define OP_ORR \
    u32 res = reg[(opcode >> 16) & 0x0F].I | value;     \
    reg[dest].I = res;
#endif
#ifndef OP_ORRS
 #define OP_ORRS   OP_ORR C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_MOV
 #define OP_MOV \
    u32 res = value;                                    \
    reg[dest].I = res;
#endif
#ifndef OP_MOVS
 #define OP_MOVS   OP_MOV C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_BIC
 #define OP_BIC \
    u32 res = reg[(opcode >> 16) & 0x0F].I & (~value);  \
    reg[dest].I = res;
#endif
#ifndef OP_BICS
 #define OP_BICS   OP_BIC C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_MVN
 #define OP_MVN \
    u32 res = ~value;                                   \
    reg[dest].I = res;
#endif
#ifndef OP_MVNS
 #define OP_MVNS   OP_MVN C_CHECK_PC(C_SETCOND_LOGICAL)
#endif

#ifndef SETCOND_NONE
 #define SETCOND_NONE /*nothing*/
#endif
#ifndef SETCOND_MUL
 #define SETCOND_MUL \
     N_FLAG = ((s32)reg[dest].I < 0) ? true : false;    \
     Z_FLAG = reg[dest].I ? false : true;
#endif
#ifndef SETCOND_MULL
 #define SETCOND_MULL \
     N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
     Z_FLAG = reg[dest].I || reg[acc].I ? false : true;
#endif

#ifndef ALU_FINISH
 #define ALU_FINISH /*nothing*/
#endif

#ifndef ROR_IMM_MSR
 #define ROR_IMM_MSR \
    u32 v = opcode & 0xff;                              \
    value = ((v << (32 - shift)) | (v >> shift));
#endif
#ifndef ROR_OFFSET
 #define ROR_OFFSET \
    offset = ((offset << (32 - shift)) | (offset >> shift));
#endif
#ifndef RRX_OFFSET
 #define RRX_OFFSET \
    offset = ((offset >> 1) | ((int)C_FLAG << 31));
#endif

// ALU ops (except multiply) //////////////////////////////////////////////

// ALU_INIT: init code (ALU_INIT_C or ALU_INIT_NC)
// GETVALUE: load value and shift/rotate (VALUE_XXX)
// OP: ALU operation (OP_XXX)
// MODECHANGE: MODECHANGE_NO or MODECHANGE_YES
// ISREGSHIFT: 1 for insns of the form ...,Rn LSL/etc Rs; 0 otherwise
// ALU_INIT, GETVALUE, OP, and ALU_FINISH are concatenated in order.
#define ALU_INSN(ALU_INIT, GETVALUE, OP, MODECHANGE, ISREGSHIFT) \
    ALU_INIT GETVALUE OP ALU_FINISH;                            \
    if (LIKELY((opcode & 0x0000F000) != 0x0000F000)) {          \
        clockTicks = 1 + ISREGSHIFT                             \
                       + codeTicksAccessSeq32(armNextPC);       \
    } else {                                                    \
        MODECHANGE;                                             \
        if (armState) {                                         \
            reg[15].I &= 0xFFFFFFFC;                            \
            armNextPC = reg[15].I;                              \
            reg[15].I += 4;                                     \
            ARM_PREFETCH;                                       \
        } else {                                                \
            reg[15].I &= 0xFFFFFFFE;                            \
            armNextPC = reg[15].I;                              \
            reg[15].I += 2;                                     \
            THUMB_PREFETCH;                                     \
        }                                                       \
        clockTicks = 3 + ISREGSHIFT                             \
                       + codeTicksAccess32(armNextPC)           \
                       + codeTicksAccessSeq32(armNextPC)        \
                       + codeTicksAccessSeq32(armNextPC);       \
    }

#define MODECHANGE_NO  /*nothing*/
#define MODECHANGE_YES CPUSwitchMode(reg[17].I & 0x1f, false);

#define DEFINE_ALU_INSN_C(CODE1, CODE2, OP, MODECHANGE) \
  static INSN_REGPARM void arm##CODE1##0(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_LSL_IMM_C, OP_##OP, MODECHANGE_##MODECHANGE, 0); }\
  static INSN_REGPARM void arm##CODE1##1(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_LSL_REG_C, OP_##OP, MODECHANGE_##MODECHANGE, 1); }\
  static INSN_REGPARM void arm##CODE1##2(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_LSR_IMM_C, OP_##OP, MODECHANGE_##MODECHANGE, 0); }\
  static INSN_REGPARM void arm##CODE1##3(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_LSR_REG_C, OP_##OP, MODECHANGE_##MODECHANGE, 1); }\
  static INSN_REGPARM void arm##CODE1##4(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_ASR_IMM_C, OP_##OP, MODECHANGE_##MODECHANGE, 0); }\
  static INSN_REGPARM void arm##CODE1##5(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_ASR_REG_C, OP_##OP, MODECHANGE_##MODECHANGE, 1); }\
  static INSN_REGPARM void arm##CODE1##6(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_ROR_IMM_C, OP_##OP, MODECHANGE_##MODECHANGE, 0); }\
  static INSN_REGPARM void arm##CODE1##7(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_ROR_REG_C, OP_##OP, MODECHANGE_##MODECHANGE, 1); }\
  static INSN_REGPARM void arm##CODE2##0(u32 opcode) { ALU_INSN(ALU_INIT_C, VALUE_IMM_C,     OP_##OP, MODECHANGE_##MODECHANGE, 0); }
#define DEFINE_ALU_INSN_NC(CODE1, CODE2, OP, MODECHANGE) \
  static INSN_REGPARM void arm##CODE1##0(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_LSL_IMM_NC, OP_##OP, MODECHANGE_##MODECHANGE, 0); }\
  static INSN_REGPARM void arm##CODE1##1(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_LSL_REG_NC, OP_##OP, MODECHANGE_##MODECHANGE, 1); }\
  static INSN_REGPARM void arm##CODE1##2(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_LSR_IMM_NC, OP_##OP, MODECHANGE_##MODECHANGE, 0); }\
  static INSN_REGPARM void arm##CODE1##3(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_LSR_REG_NC, OP_##OP, MODECHANGE_##MODECHANGE, 1); }\
  static INSN_REGPARM void arm##CODE1##4(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_ASR_IMM_NC, OP_##OP, MODECHANGE_##MODECHANGE, 0); }\
  static INSN_REGPARM void arm##CODE1##5(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_ASR_REG_NC, OP_##OP, MODECHANGE_##MODECHANGE, 1); }\
  static INSN_REGPARM void arm##CODE1##6(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_ROR_IMM_NC, OP_##OP, MODECHANGE_##MODECHANGE, 0); }\
  static INSN_REGPARM void arm##CODE1##7(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_ROR_REG_NC, OP_##OP, MODECHANGE_##MODECHANGE, 1); }\
  static INSN_REGPARM void arm##CODE2##0(u32 opcode) { ALU_INSN(ALU_INIT_NC, VALUE_IMM_NC,     OP_##OP, MODECHANGE_##MODECHANGE, 0); }

// AND
DEFINE_ALU_INSN_NC(00, 20, AND,  NO)
// ANDS
DEFINE_ALU_INSN_C (01, 21, ANDS, YES)

// EOR
DEFINE_ALU_INSN_NC(02, 22, EOR,  NO)
// EORS
DEFINE_ALU_INSN_C (03, 23, EORS, YES)

// SUB
DEFINE_ALU_INSN_NC(04, 24, SUB,  NO)
// SUBS
DEFINE_ALU_INSN_NC(05, 25, SUBS, YES)

// RSB
DEFINE_ALU_INSN_NC(06, 26, RSB,  NO)
// RSBS
DEFINE_ALU_INSN_NC(07, 27, RSBS, YES)

// ADD
DEFINE_ALU_INSN_NC(08, 28, ADD,  NO)
// ADDS
DEFINE_ALU_INSN_NC(09, 29, ADDS, YES)

// ADC
DEFINE_ALU_INSN_NC(0A, 2A, ADC,  NO)
// ADCS
DEFINE_ALU_INSN_NC(0B, 2B, ADCS, YES)

// SBC
DEFINE_ALU_INSN_NC(0C, 2C, SBC,  NO)
// SBCS
DEFINE_ALU_INSN_NC(0D, 2D, SBCS, YES)

// RSC
DEFINE_ALU_INSN_NC(0E, 2E, RSC,  NO)
// RSCS
DEFINE_ALU_INSN_NC(0F, 2F, RSCS, YES)

// TST
DEFINE_ALU_INSN_C (11, 31, TST,  NO)

// TEQ
DEFINE_ALU_INSN_C (13, 33, TEQ,  NO)

// CMP
DEFINE_ALU_INSN_NC(15, 35, CMP,  NO)

// CMN
DEFINE_ALU_INSN_NC(17, 37, CMN,  NO)

// ORR
DEFINE_ALU_INSN_NC(18, 38, ORR,  NO)
// ORRS
DEFINE_ALU_INSN_C (19, 39, ORRS, YES)

// MOV
DEFINE_ALU_INSN_NC(1A, 3A, MOV,  NO)
// MOVS
DEFINE_ALU_INSN_C (1B, 3B, MOVS, YES)

// BIC
DEFINE_ALU_INSN_NC(1C, 3C, BIC,  NO)
// BICS
DEFINE_ALU_INSN_C (1D, 3D, BICS, YES)

// MVN
DEFINE_ALU_INSN_NC(1E, 3E, MVN,  NO)
// MVNS
DEFINE_ALU_INSN_C (1F, 3F, MVNS, YES)

// Multiply instructions //////////////////////////////////////////////////

// OP: OP_MUL, OP_MLA etc.
// SETCOND: SETCOND_NONE, SETCOND_MUL, or SETCOND_MULL
// CYCLES: base cycle count (1, 2, or 3)
#define MUL_INSN(OP, SETCOND, CYCLES) \
    int mult = (opcode & 0x0F);                         \
    u32 rs = reg[(opcode >> 8) & 0x0F].I;               \
    int acc  __attribute__((unused)) = (opcode >> 12) & 0x0F;  /* or destLo */  \
    int dest __attribute__((unused)) = (opcode >> 16) & 0x0F;  /* or destHi */  \
    OP;                                                 \
    SETCOND;                                            \
    if ((s32)rs < 0)                                    \
        rs = ~rs;                                       \
    if ((rs & 0xFFFFFF00) == 0)                         \
        clockTicks += 0;                                \
    else if ((rs & 0xFFFF0000) == 0)                    \
        clockTicks += 1;                                \
    else if ((rs & 0xFF000000) == 0)                    \
        clockTicks += 2;                                \
    else                                                \
        clockTicks += 3;                                \
    if (busPrefetchCount == 0)                          \
        busPrefetchCount = ((busPrefetchCount+1)<<clockTicks) - 1; \
    clockTicks += 1 + codeTicksAccess32(armNextPC);

#define OP_MUL \
    reg[dest].I = reg[mult].I * rs;
#define OP_MLA \
    reg[dest].I = reg[mult].I * rs + reg[acc].I;
#define OP_MULL(SIGN) \
    SIGN##64 res = (SIGN##64)(SIGN##32)reg[mult].I      \
                 * (SIGN##64)(SIGN##32)rs;              \
    reg[acc].I = (u32)res;                              \
    reg[dest].I = (u32)(res >> 32);
#define OP_MLAL(SIGN) \
    SIGN##64 res = ((SIGN##64)reg[dest].I<<32 | reg[acc].I)\
                 + ((SIGN##64)(SIGN##32)reg[mult].I     \
                    * (SIGN##64)(SIGN##32)rs);          \
    reg[acc].I = (u32)res;                              \
    reg[dest].I = (u32)(res >> 32);
#define OP_UMULL OP_MULL(u)
#define OP_UMLAL OP_MLAL(u)
#define OP_SMULL OP_MULL(s)
#define OP_SMLAL OP_MLAL(s)

// MUL Rd, Rm, Rs
static INSN_REGPARM void arm009(u32 opcode) { MUL_INSN(OP_MUL, SETCOND_NONE, 1); }
// MULS Rd, Rm, Rs
static INSN_REGPARM void arm019(u32 opcode) { MUL_INSN(OP_MUL, SETCOND_MUL, 1); }

// MLA Rd, Rm, Rs, Rn
static INSN_REGPARM void arm029(u32 opcode) { MUL_INSN(OP_MLA, SETCOND_NONE, 2); }
// MLAS Rd, Rm, Rs, Rn
static INSN_REGPARM void arm039(u32 opcode) { MUL_INSN(OP_MLA, SETCOND_MUL, 2); }

// UMULL RdLo, RdHi, Rn, Rs
static INSN_REGPARM void arm089(u32 opcode) { MUL_INSN(OP_UMULL, SETCOND_NONE, 2); }
// UMULLS RdLo, RdHi, Rn, Rs
static INSN_REGPARM void arm099(u32 opcode) { MUL_INSN(OP_UMULL, SETCOND_MULL, 2); }

// UMLAL RdLo, RdHi, Rn, Rs
static INSN_REGPARM void arm0A9(u32 opcode) { MUL_INSN(OP_UMLAL, SETCOND_NONE, 3); }
// UMLALS RdLo, RdHi, Rn, Rs
static INSN_REGPARM void arm0B9(u32 opcode) { MUL_INSN(OP_UMLAL, SETCOND_MULL, 3); }

// SMULL RdLo, RdHi, Rm, Rs
static INSN_REGPARM void arm0C9(u32 opcode) { MUL_INSN(OP_SMULL, SETCOND_NONE, 2); }
// SMULLS RdLo, RdHi, Rm, Rs
static INSN_REGPARM void arm0D9(u32 opcode) { MUL_INSN(OP_SMULL, SETCOND_MULL, 2); }

// SMLAL RdLo, RdHi, Rm, Rs
static INSN_REGPARM void arm0E9(u32 opcode) { MUL_INSN(OP_SMLAL, SETCOND_NONE, 3); }
// SMLALS RdLo, RdHi, Rm, Rs
static INSN_REGPARM void arm0F9(u32 opcode) { MUL_INSN(OP_SMLAL, SETCOND_MULL, 3); }

// Misc instructions //////////////////////////////////////////////////////

// SWP Rd, Rm, [Rn]
static INSN_REGPARM void arm109(u32 opcode)
{
    u32 address = reg[(opcode >> 16) & 15].I;
    u32 temp = CPUReadMemory(address);
    CPUWriteMemory(address, reg[opcode&15].I);
    reg[(opcode >> 12) & 15].I = temp;
    clockTicks = 4 + dataTicksAccess32(address) + dataTicksAccess32(address)
                   + codeTicksAccess32(armNextPC);
}

// SWPB Rd, Rm, [Rn]
static INSN_REGPARM void arm149(u32 opcode)
{
    u32 address = reg[(opcode >> 16) & 15].I;
    u32 temp = CPUReadByte(address);
    CPUWriteByte(address, reg[opcode&15].B.B0);
    reg[(opcode>>12)&15].I = temp;
    clockTicks = 4 + dataTicksAccess32(address) + dataTicksAccess32(address)
                   + codeTicksAccess32(armNextPC);
}

// MRS Rd, CPSR
static INSN_REGPARM void arm100(u32 opcode)
{
    if (LIKELY((opcode & 0x0FFF0FFF) == 0x010F0000)) {
        CPUUpdateCPSR();
        reg[(opcode >> 12) & 0x0F].I = reg[16].I;
    } else {
        armUnknownInsn(opcode);
    }
}

// MRS Rd, SPSR
static INSN_REGPARM void arm140(u32 opcode)
{
    if (LIKELY((opcode & 0x0FFF0FFF) == 0x014F0000)) {
        reg[(opcode >> 12) & 0x0F].I = reg[17].I;
    } else {
        armUnknownInsn(opcode);
    }
}

// MSR CPSR_fields, Rm
static INSN_REGPARM void arm120(u32 opcode)
{
    if (LIKELY((opcode & 0x0FF0FFF0) == 0x0120F000)) {
        CPUUpdateCPSR();
        u32 value = reg[opcode & 15].I;
        u32 newValue = reg[16].I;
        if (armMode > 0x10) {
            if (opcode & 0x00010000)
                newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
            if (opcode & 0x00020000)
                newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
            if (opcode & 0x00040000)
                newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
        }
        if (opcode & 0x00080000)
            newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);
        newValue |= 0x10;
        CPUSwitchMode(newValue & 0x1F, false);
        reg[16].I = newValue;
        CPUUpdateFlags();
        if (!armState) {  // this should not be allowed, but it seems to work
            THUMB_PREFETCH;
            reg[15].I = armNextPC + 2;
        }
    } else {
        armUnknownInsn(opcode);
    }
}

// MSR SPSR_fields, Rm
static INSN_REGPARM void arm160(u32 opcode)
{
    if (LIKELY((opcode & 0x0FF0FFF0) == 0x0160F000)) {
        u32 value = reg[opcode & 15].I;
        if (armMode > 0x10 && armMode < 0x1F) {
            if (opcode & 0x00010000)
                reg[17].I = (reg[17].I & 0xFFFFFF00) | (value & 0x000000FF);
            if (opcode & 0x00020000)
                reg[17].I = (reg[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
            if (opcode & 0x00040000)
                reg[17].I = (reg[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
            if (opcode & 0x00080000)
                reg[17].I = (reg[17].I & 0x00FFFFFF) | (value & 0xFF000000);
        }
    } else {
        armUnknownInsn(opcode);
    }
}

// MSR CPSR_fields, #
static INSN_REGPARM void arm320(u32 opcode)
{
    if (LIKELY((opcode & 0x0FF0F000) == 0x0320F000)) {
        CPUUpdateCPSR();
        u32 value = opcode & 0xFF;
        int shift = (opcode & 0xF00) >> 7;
        if (shift) {
            ROR_IMM_MSR;
        }
        u32 newValue = reg[16].I;
        if (armMode > 0x10) {
            if (opcode & 0x00010000)
                newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
            if (opcode & 0x00020000)
                newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
            if (opcode & 0x00040000)
                newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
        }
        if (opcode & 0x00080000)
            newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);

        newValue |= 0x10;

        CPUSwitchMode(newValue & 0x1F, false);
        reg[16].I = newValue;
        CPUUpdateFlags();
        if (!armState) {  // this should not be allowed, but it seems to work
            THUMB_PREFETCH;
            reg[15].I = armNextPC + 2;
        }
    } else {
        armUnknownInsn(opcode);
    }
}

// MSR SPSR_fields, #
static INSN_REGPARM void arm360(u32 opcode)
{
    if (LIKELY((opcode & 0x0FF0F000) == 0x0360F000)) {
        if (armMode > 0x10 && armMode < 0x1F) {
            u32 value = opcode & 0xFF;
            int shift = (opcode & 0xF00) >> 7;
            if (shift) {
                ROR_IMM_MSR;
            }
            if (opcode & 0x00010000)
                reg[17].I = (reg[17].I & 0xFFFFFF00) | (value & 0x000000FF);
            if (opcode & 0x00020000)
                reg[17].I = (reg[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
            if (opcode & 0x00040000)
                reg[17].I = (reg[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
            if (opcode & 0x00080000)
                reg[17].I = (reg[17].I & 0x00FFFFFF) | (value & 0xFF000000);
        }
    } else {
        armUnknownInsn(opcode);
    }
}

// BX Rm
static INSN_REGPARM void arm121(u32 opcode)
{
    if (LIKELY((opcode & 0x0FFFFFF0) == 0x012FFF10)) {
        int base = opcode & 0x0F;
        busPrefetchCount = 0;
        armState = reg[base].I & 1 ? false : true;
        if (armState) {
            reg[15].I = reg[base].I & 0xFFFFFFFC;
            armNextPC = reg[15].I;
            reg[15].I += 4;
            ARM_PREFETCH;
            clockTicks = 3 + codeTicksAccessSeq32(armNextPC)
                           + codeTicksAccessSeq32(armNextPC)
                           + codeTicksAccess32(armNextPC);
        } else {
            reg[15].I = reg[base].I & 0xFFFFFFFE;
            armNextPC = reg[15].I;
            reg[15].I += 2;
            THUMB_PREFETCH;
            clockTicks = 3 + codeTicksAccessSeq16(armNextPC)
                           + codeTicksAccessSeq16(armNextPC)
                           + codeTicksAccess16(armNextPC);
        }
    } else {
        armUnknownInsn(opcode);
    }
}

// Load/store /////////////////////////////////////////////////////////////

#define OFFSET_IMM \
    int offset = opcode & 0xFFF;
#define OFFSET_IMM8 \
    int offset = ((opcode & 0x0F) | ((opcode>>4) & 0xF0));
#define OFFSET_REG \
    int offset = reg[opcode & 15].I;
#define OFFSET_LSL \
    int offset = reg[opcode & 15].I << ((opcode>>7) & 31);
#define OFFSET_LSR \
    int shift = (opcode >> 7) & 31;                     \
    int offset = shift ? reg[opcode & 15].I >> shift : 0;
#define OFFSET_ASR \
    int shift = (opcode >> 7) & 31;                     \
    int offset;                                         \
    if (shift)                                          \
        offset = (int)((s32)reg[opcode & 15].I >> shift);\
    else if (reg[opcode & 15].I & 0x80000000)           \
        offset = 0xFFFFFFFF;                            \
    else                                                \
        offset = 0;
#define OFFSET_ROR \
    int shift = (opcode >> 7) & 31;                     \
    u32 offset = reg[opcode & 15].I;                    \
    if (shift) {                                        \
        ROR_OFFSET;                                     \
    } else {                                            \
        RRX_OFFSET;                                     \
    }

#define ADDRESS_POST (reg[base].I)
#define ADDRESS_PREDEC (reg[base].I - offset)
#define ADDRESS_PREINC (reg[base].I + offset)

#define OP_STR    CPUWriteMemory(address, reg[dest].I)
#define OP_STRH   CPUWriteHalfWord(address, reg[dest].W.W0)
#define OP_STRB   CPUWriteByte(address, reg[dest].B.B0)
#define OP_LDR    reg[dest].I = CPUReadMemory(address)
#define OP_LDRH   reg[dest].I = CPUReadHalfWord(address)
#define OP_LDRB   reg[dest].I = CPUReadByte(address)
#define OP_LDRSH  reg[dest].I = (s16)CPUReadHalfWordSigned(address)
#define OP_LDRSB  reg[dest].I = (s8)CPUReadByte(address)

#define WRITEBACK_NONE     /*nothing*/
#define WRITEBACK_PRE      reg[base].I = address
#define WRITEBACK_POSTDEC  reg[base].I = address - offset
#define WRITEBACK_POSTINC  reg[base].I = address + offset

#define LDRSTR_INIT(CALC_OFFSET, CALC_ADDRESS) \
    if (busPrefetchCount == 0)                          \
        busPrefetch = busPrefetchEnable;                \
    int dest __attribute__((unused)) = (opcode >> 12) & 15; \
    int base = (opcode >> 16) & 15;                     \
    CALC_OFFSET;                                        \
    u32 address = CALC_ADDRESS;

#define STR(CALC_OFFSET, CALC_ADDRESS, STORE_DATA, WRITEBACK1, WRITEBACK2, SIZE) \
    LDRSTR_INIT(CALC_OFFSET, CALC_ADDRESS);             \
    WRITEBACK1;                                         \
    STORE_DATA;                                         \
    WRITEBACK2;                                         \
    clockTicks = 2 + dataTicksAccess##SIZE(address)     \
                   + codeTicksAccess32(armNextPC);
#define LDR(CALC_OFFSET, CALC_ADDRESS, LOAD_DATA, WRITEBACK, SIZE) \
    LDRSTR_INIT(CALC_OFFSET, CALC_ADDRESS);             \
    LOAD_DATA;                                          \
    if (dest != base)                                   \
    {                                                   \
        WRITEBACK;                                      \
    }                                                   \
    clockTicks = 0;                                     \
    if (dest == 15) {                                   \
        reg[15].I &= 0xFFFFFFFC;                        \
        armNextPC = reg[15].I;                          \
        reg[15].I += 4;                                 \
        ARM_PREFETCH;                                   \
        clockTicks += 2 + dataTicksAccessSeq32(address) \
                        + dataTicksAccessSeq32(address);\
    }                                                   \
    clockTicks += 3 + dataTicksAccess##SIZE(address)    \
                    + codeTicksAccess32(armNextPC);
#define STR_POSTDEC(CALC_OFFSET, STORE_DATA, SIZE) \
  STR(CALC_OFFSET, ADDRESS_POST, STORE_DATA, WRITEBACK_NONE, WRITEBACK_POSTDEC, SIZE)
#define STR_POSTINC(CALC_OFFSET, STORE_DATA, SIZE) \
  STR(CALC_OFFSET, ADDRESS_POST, STORE_DATA, WRITEBACK_NONE, WRITEBACK_POSTINC, SIZE)
#define STR_PREDEC(CALC_OFFSET, STORE_DATA, SIZE) \
  STR(CALC_OFFSET, ADDRESS_PREDEC, STORE_DATA, WRITEBACK_NONE, WRITEBACK_NONE, SIZE)
#define STR_PREDEC_WB(CALC_OFFSET, STORE_DATA, SIZE) \
  STR(CALC_OFFSET, ADDRESS_PREDEC, STORE_DATA, WRITEBACK_PRE, WRITEBACK_NONE, SIZE)
#define STR_PREINC(CALC_OFFSET, STORE_DATA, SIZE) \
  STR(CALC_OFFSET, ADDRESS_PREINC, STORE_DATA, WRITEBACK_NONE, WRITEBACK_NONE, SIZE)
#define STR_PREINC_WB(CALC_OFFSET, STORE_DATA, SIZE) \
  STR(CALC_OFFSET, ADDRESS_PREINC, STORE_DATA, WRITEBACK_PRE, WRITEBACK_NONE, SIZE)
#define LDR_POSTDEC(CALC_OFFSET, LOAD_DATA, SIZE) \
  LDR(CALC_OFFSET, ADDRESS_POST, LOAD_DATA, WRITEBACK_POSTDEC, SIZE)
#define LDR_POSTINC(CALC_OFFSET, LOAD_DATA, SIZE) \
  LDR(CALC_OFFSET, ADDRESS_POST, LOAD_DATA, WRITEBACK_POSTINC, SIZE)
#define LDR_PREDEC(CALC_OFFSET, LOAD_DATA, SIZE) \
  LDR(CALC_OFFSET, ADDRESS_PREDEC, LOAD_DATA, WRITEBACK_NONE, SIZE)
#define LDR_PREDEC_WB(CALC_OFFSET, LOAD_DATA, SIZE) \
  LDR(CALC_OFFSET, ADDRESS_PREDEC, LOAD_DATA, WRITEBACK_PRE, SIZE)
#define LDR_PREINC(CALC_OFFSET, LOAD_DATA, SIZE) \
  LDR(CALC_OFFSET, ADDRESS_PREINC, LOAD_DATA, WRITEBACK_NONE, SIZE)
#define LDR_PREINC_WB(CALC_OFFSET, LOAD_DATA, SIZE) \
  LDR(CALC_OFFSET, ADDRESS_PREINC, LOAD_DATA, WRITEBACK_PRE, SIZE)

// STRH Rd, [Rn], -Rm
static INSN_REGPARM void arm00B(u32 opcode) { STR_POSTDEC(OFFSET_REG, OP_STRH, 16); }
// STRH Rd, [Rn], #-offset
static INSN_REGPARM void arm04B(u32 opcode) { STR_POSTDEC(OFFSET_IMM8, OP_STRH, 16); }
// STRH Rd, [Rn], Rm
static INSN_REGPARM void arm08B(u32 opcode) { STR_POSTINC(OFFSET_REG, OP_STRH, 16); }
// STRH Rd, [Rn], #offset
static INSN_REGPARM void arm0CB(u32 opcode) { STR_POSTINC(OFFSET_IMM8, OP_STRH, 16); }
// STRH Rd, [Rn, -Rm]
static INSN_REGPARM void arm10B(u32 opcode) { STR_PREDEC(OFFSET_REG, OP_STRH, 16); }
// STRH Rd, [Rn, -Rm]!
static INSN_REGPARM void arm12B(u32 opcode) { STR_PREDEC_WB(OFFSET_REG, OP_STRH, 16); }
// STRH Rd, [Rn, -#offset]
static INSN_REGPARM void arm14B(u32 opcode) { STR_PREDEC(OFFSET_IMM8, OP_STRH, 16); }
// STRH Rd, [Rn, -#offset]!
static INSN_REGPARM void arm16B(u32 opcode) { STR_PREDEC_WB(OFFSET_IMM8, OP_STRH, 16); }
// STRH Rd, [Rn, Rm]
static INSN_REGPARM void arm18B(u32 opcode) { STR_PREINC(OFFSET_REG, OP_STRH, 16); }
// STRH Rd, [Rn, Rm]!
static INSN_REGPARM void arm1AB(u32 opcode) { STR_PREINC_WB(OFFSET_REG, OP_STRH, 16); }
// STRH Rd, [Rn, #offset]
static INSN_REGPARM void arm1CB(u32 opcode) { STR_PREINC(OFFSET_IMM8, OP_STRH, 16); }
// STRH Rd, [Rn, #offset]!
static INSN_REGPARM void arm1EB(u32 opcode) { STR_PREINC_WB(OFFSET_IMM8, OP_STRH, 16); }

// LDRH Rd, [Rn], -Rm
static INSN_REGPARM void arm01B(u32 opcode) { LDR_POSTDEC(OFFSET_REG, OP_LDRH, 16); }
// LDRH Rd, [Rn], #-offset
static INSN_REGPARM void arm05B(u32 opcode) { LDR_POSTDEC(OFFSET_IMM8, OP_LDRH, 16); }
// LDRH Rd, [Rn], Rm
static INSN_REGPARM void arm09B(u32 opcode) { LDR_POSTINC(OFFSET_REG, OP_LDRH, 16); }
// LDRH Rd, [Rn], #offset
static INSN_REGPARM void arm0DB(u32 opcode) { LDR_POSTINC(OFFSET_IMM8, OP_LDRH, 16); }
// LDRH Rd, [Rn, -Rm]
static INSN_REGPARM void arm11B(u32 opcode) { LDR_PREDEC(OFFSET_REG, OP_LDRH, 16); }
// LDRH Rd, [Rn, -Rm]!
static INSN_REGPARM void arm13B(u32 opcode) { LDR_PREDEC_WB(OFFSET_REG, OP_LDRH, 16); }
// LDRH Rd, [Rn, -#offset]
static INSN_REGPARM void arm15B(u32 opcode) { LDR_PREDEC(OFFSET_IMM8, OP_LDRH, 16); }
// LDRH Rd, [Rn, -#offset]!
static INSN_REGPARM void arm17B(u32 opcode) { LDR_PREDEC_WB(OFFSET_IMM8, OP_LDRH, 16); }
// LDRH Rd, [Rn, Rm]
static INSN_REGPARM void arm19B(u32 opcode) { LDR_PREINC(OFFSET_REG, OP_LDRH, 16); }
// LDRH Rd, [Rn, Rm]!
static INSN_REGPARM void arm1BB(u32 opcode) { LDR_PREINC_WB(OFFSET_REG, OP_LDRH, 16); }
// LDRH Rd, [Rn, #offset]
static INSN_REGPARM void arm1DB(u32 opcode) { LDR_PREINC(OFFSET_IMM8, OP_LDRH, 16); }
// LDRH Rd, [Rn, #offset]!
static INSN_REGPARM void arm1FB(u32 opcode) { LDR_PREINC_WB(OFFSET_IMM8, OP_LDRH, 16); }

// LDRSB Rd, [Rn], -Rm
static INSN_REGPARM void arm01D(u32 opcode) { LDR_POSTDEC(OFFSET_REG, OP_LDRSB, 16); }
// LDRSB Rd, [Rn], #-offset
static INSN_REGPARM void arm05D(u32 opcode) { LDR_POSTDEC(OFFSET_IMM8, OP_LDRSB, 16); }
// LDRSB Rd, [Rn], Rm
static INSN_REGPARM void arm09D(u32 opcode) { LDR_POSTINC(OFFSET_REG, OP_LDRSB, 16); }
// LDRSB Rd, [Rn], #offset
static INSN_REGPARM void arm0DD(u32 opcode) { LDR_POSTINC(OFFSET_IMM8, OP_LDRSB, 16); }
// LDRSB Rd, [Rn, -Rm]
static INSN_REGPARM void arm11D(u32 opcode) { LDR_PREDEC(OFFSET_REG, OP_LDRSB, 16); }
// LDRSB Rd, [Rn, -Rm]!
static INSN_REGPARM void arm13D(u32 opcode) { LDR_PREDEC_WB(OFFSET_REG, OP_LDRSB, 16); }
// LDRSB Rd, [Rn, -#offset]
static INSN_REGPARM void arm15D(u32 opcode) { LDR_PREDEC(OFFSET_IMM8, OP_LDRSB, 16); }
// LDRSB Rd, [Rn, -#offset]!
static INSN_REGPARM void arm17D(u32 opcode) { LDR_PREDEC_WB(OFFSET_IMM8, OP_LDRSB, 16); }
// LDRSB Rd, [Rn, Rm]
static INSN_REGPARM void arm19D(u32 opcode) { LDR_PREINC(OFFSET_REG, OP_LDRSB, 16); }
// LDRSB Rd, [Rn, Rm]!
static INSN_REGPARM void arm1BD(u32 opcode) { LDR_PREINC_WB(OFFSET_REG, OP_LDRSB, 16); }
// LDRSB Rd, [Rn, #offset]
static INSN_REGPARM void arm1DD(u32 opcode) { LDR_PREINC(OFFSET_IMM8, OP_LDRSB, 16); }
// LDRSB Rd, [Rn, #offset]!
static INSN_REGPARM void arm1FD(u32 opcode) { LDR_PREINC_WB(OFFSET_IMM8, OP_LDRSB, 16); }

// LDRSH Rd, [Rn], -Rm
static INSN_REGPARM void arm01F(u32 opcode) { LDR_POSTDEC(OFFSET_REG, OP_LDRSH, 16); }
// LDRSH Rd, [Rn], #-offset
static INSN_REGPARM void arm05F(u32 opcode) { LDR_POSTDEC(OFFSET_IMM8, OP_LDRSH, 16); }
// LDRSH Rd, [Rn], Rm
static INSN_REGPARM void arm09F(u32 opcode) { LDR_POSTINC(OFFSET_REG, OP_LDRSH, 16); }
// LDRSH Rd, [Rn], #offset
static INSN_REGPARM void arm0DF(u32 opcode) { LDR_POSTINC(OFFSET_IMM8, OP_LDRSH, 16); }
// LDRSH Rd, [Rn, -Rm]
static INSN_REGPARM void arm11F(u32 opcode) { LDR_PREDEC(OFFSET_REG, OP_LDRSH, 16); }
// LDRSH Rd, [Rn, -Rm]!
static INSN_REGPARM void arm13F(u32 opcode) { LDR_PREDEC_WB(OFFSET_REG, OP_LDRSH, 16); }
// LDRSH Rd, [Rn, -#offset]
static INSN_REGPARM void arm15F(u32 opcode) { LDR_PREDEC(OFFSET_IMM8, OP_LDRSH, 16); }
// LDRSH Rd, [Rn, -#offset]!
static INSN_REGPARM void arm17F(u32 opcode) { LDR_PREDEC_WB(OFFSET_IMM8, OP_LDRSH, 16); }
// LDRSH Rd, [Rn, Rm]
static INSN_REGPARM void arm19F(u32 opcode) { LDR_PREINC(OFFSET_REG, OP_LDRSH, 16); }
// LDRSH Rd, [Rn, Rm]!
static INSN_REGPARM void arm1BF(u32 opcode) { LDR_PREINC_WB(OFFSET_REG, OP_LDRSH, 16); }
// LDRSH Rd, [Rn, #offset]
static INSN_REGPARM void arm1DF(u32 opcode) { LDR_PREINC(OFFSET_IMM8, OP_LDRSH, 16); }
// LDRSH Rd, [Rn, #offset]!
static INSN_REGPARM void arm1FF(u32 opcode) { LDR_PREINC_WB(OFFSET_IMM8, OP_LDRSH, 16); }

// STR[T] Rd, [Rn], -#
// Note: STR and STRT do the same thing on the GBA (likewise for LDR/LDRT etc)
static INSN_REGPARM void arm400(u32 opcode) { STR_POSTDEC(OFFSET_IMM, OP_STR, 32); }
// LDR[T] Rd, [Rn], -#
static INSN_REGPARM void arm410(u32 opcode) { LDR_POSTDEC(OFFSET_IMM, OP_LDR, 32); }
// STRB[T] Rd, [Rn], -#
static INSN_REGPARM void arm440(u32 opcode) { STR_POSTDEC(OFFSET_IMM, OP_STRB, 16); }
// LDRB[T] Rd, [Rn], -#
static INSN_REGPARM void arm450(u32 opcode) { LDR_POSTDEC(OFFSET_IMM, OP_LDRB, 16); }
// STR[T] Rd, [Rn], #
static INSN_REGPARM void arm480(u32 opcode) { STR_POSTINC(OFFSET_IMM, OP_STR, 32); }
// LDR Rd, [Rn], #
static INSN_REGPARM void arm490(u32 opcode) { LDR_POSTINC(OFFSET_IMM, OP_LDR, 32); }
// STRB[T] Rd, [Rn], #
static INSN_REGPARM void arm4C0(u32 opcode) { STR_POSTINC(OFFSET_IMM, OP_STRB, 16); }
// LDRB[T] Rd, [Rn], #
static INSN_REGPARM void arm4D0(u32 opcode) { LDR_POSTINC(OFFSET_IMM, OP_LDRB, 16); }
// STR Rd, [Rn, -#]
static INSN_REGPARM void arm500(u32 opcode) { STR_PREDEC(OFFSET_IMM, OP_STR, 32); }
// LDR Rd, [Rn, -#]
static INSN_REGPARM void arm510(u32 opcode) { LDR_PREDEC(OFFSET_IMM, OP_LDR, 32); }
// STR Rd, [Rn, -#]!
static INSN_REGPARM void arm520(u32 opcode) { STR_PREDEC_WB(OFFSET_IMM, OP_STR, 32); }
// LDR Rd, [Rn, -#]!
static INSN_REGPARM void arm530(u32 opcode) { LDR_PREDEC_WB(OFFSET_IMM, OP_LDR, 32); }
// STRB Rd, [Rn, -#]
static INSN_REGPARM void arm540(u32 opcode) { STR_PREDEC(OFFSET_IMM, OP_STRB, 16); }
// LDRB Rd, [Rn, -#]
static INSN_REGPARM void arm550(u32 opcode) { LDR_PREDEC(OFFSET_IMM, OP_LDRB, 16); }
// STRB Rd, [Rn, -#]!
static INSN_REGPARM void arm560(u32 opcode) { STR_PREDEC_WB(OFFSET_IMM, OP_STRB, 16); }
// LDRB Rd, [Rn, -#]!
static INSN_REGPARM void arm570(u32 opcode) { LDR_PREDEC_WB(OFFSET_IMM, OP_LDRB, 16); }
// STR Rd, [Rn, #]
static INSN_REGPARM void arm580(u32 opcode) { STR_PREINC(OFFSET_IMM, OP_STR, 32); }
// LDR Rd, [Rn, #]
static INSN_REGPARM void arm590(u32 opcode) { LDR_PREINC(OFFSET_IMM, OP_LDR, 32); }
// STR Rd, [Rn, #]!
static INSN_REGPARM void arm5A0(u32 opcode) { STR_PREINC_WB(OFFSET_IMM, OP_STR, 32); }
// LDR Rd, [Rn, #]!
static INSN_REGPARM void arm5B0(u32 opcode) { LDR_PREINC_WB(OFFSET_IMM, OP_LDR, 32); }
// STRB Rd, [Rn, #]
static INSN_REGPARM void arm5C0(u32 opcode) { STR_PREINC(OFFSET_IMM, OP_STRB, 16); }
// LDRB Rd, [Rn, #]
static INSN_REGPARM void arm5D0(u32 opcode) { LDR_PREINC(OFFSET_IMM, OP_LDRB, 16); }
// STRB Rd, [Rn, #]!
static INSN_REGPARM void arm5E0(u32 opcode) { STR_PREINC_WB(OFFSET_IMM, OP_STRB, 16); }
// LDRB Rd, [Rn, #]!
static INSN_REGPARM void arm5F0(u32 opcode) { LDR_PREINC_WB(OFFSET_IMM, OP_LDRB, 16); }

// STR[T] Rd, [Rn], -Rm, LSL #
static INSN_REGPARM void arm600(u32 opcode) { STR_POSTDEC(OFFSET_LSL, OP_STR, 32); }
// STR[T] Rd, [Rn], -Rm, LSR #
static INSN_REGPARM void arm602(u32 opcode) { STR_POSTDEC(OFFSET_LSR, OP_STR, 32); }
// STR[T] Rd, [Rn], -Rm, ASR #
static INSN_REGPARM void arm604(u32 opcode) { STR_POSTDEC(OFFSET_ASR, OP_STR, 32); }
// STR[T] Rd, [Rn], -Rm, ROR #
static INSN_REGPARM void arm606(u32 opcode) { STR_POSTDEC(OFFSET_ROR, OP_STR, 32); }
// LDR[T] Rd, [Rn], -Rm, LSL #
static INSN_REGPARM void arm610(u32 opcode) { LDR_POSTDEC(OFFSET_LSL, OP_LDR, 32); }
// LDR[T] Rd, [Rn], -Rm, LSR #
static INSN_REGPARM void arm612(u32 opcode) { LDR_POSTDEC(OFFSET_LSR, OP_LDR, 32); }
// LDR[T] Rd, [Rn], -Rm, ASR #
static INSN_REGPARM void arm614(u32 opcode) { LDR_POSTDEC(OFFSET_ASR, OP_LDR, 32); }
// LDR[T] Rd, [Rn], -Rm, ROR #
static INSN_REGPARM void arm616(u32 opcode) { LDR_POSTDEC(OFFSET_ROR, OP_LDR, 32); }
// STRB[T] Rd, [Rn], -Rm, LSL #
static INSN_REGPARM void arm640(u32 opcode) { STR_POSTDEC(OFFSET_LSL, OP_STRB, 16); }
// STRB[T] Rd, [Rn], -Rm, LSR #
static INSN_REGPARM void arm642(u32 opcode) { STR_POSTDEC(OFFSET_LSR, OP_STRB, 16); }
// STRB[T] Rd, [Rn], -Rm, ASR #
static INSN_REGPARM void arm644(u32 opcode) { STR_POSTDEC(OFFSET_ASR, OP_STRB, 16); }
// STRB[T] Rd, [Rn], -Rm, ROR #
static INSN_REGPARM void arm646(u32 opcode) { STR_POSTDEC(OFFSET_ROR, OP_STRB, 16); }
// LDRB[T] Rd, [Rn], -Rm, LSL #
static INSN_REGPARM void arm650(u32 opcode) { LDR_POSTDEC(OFFSET_LSL, OP_LDRB, 16); }
// LDRB[T] Rd, [Rn], -Rm, LSR #
static INSN_REGPARM void arm652(u32 opcode) { LDR_POSTDEC(OFFSET_LSR, OP_LDRB, 16); }
// LDRB[T] Rd, [Rn], -Rm, ASR #
static INSN_REGPARM void arm654(u32 opcode) { LDR_POSTDEC(OFFSET_ASR, OP_LDRB, 16); }
// LDRB Rd, [Rn], -Rm, ROR #
static INSN_REGPARM void arm656(u32 opcode) { LDR_POSTDEC(OFFSET_ROR, OP_LDRB, 16); }
// STR[T] Rd, [Rn], Rm, LSL #
static INSN_REGPARM void arm680(u32 opcode) { STR_POSTINC(OFFSET_LSL, OP_STR, 32); }
// STR[T] Rd, [Rn], Rm, LSR #
static INSN_REGPARM void arm682(u32 opcode) { STR_POSTINC(OFFSET_LSR, OP_STR, 32); }
// STR[T] Rd, [Rn], Rm, ASR #
static INSN_REGPARM void arm684(u32 opcode) { STR_POSTINC(OFFSET_ASR, OP_STR, 32); }
// STR[T] Rd, [Rn], Rm, ROR #
static INSN_REGPARM void arm686(u32 opcode) { STR_POSTINC(OFFSET_ROR, OP_STR, 32); }
// LDR[T] Rd, [Rn], Rm, LSL #
static INSN_REGPARM void arm690(u32 opcode) { LDR_POSTINC(OFFSET_LSL, OP_LDR, 32); }
// LDR[T] Rd, [Rn], Rm, LSR #
static INSN_REGPARM void arm692(u32 opcode) { LDR_POSTINC(OFFSET_LSR, OP_LDR, 32); }
// LDR[T] Rd, [Rn], Rm, ASR #
static INSN_REGPARM void arm694(u32 opcode) { LDR_POSTINC(OFFSET_ASR, OP_LDR, 32); }
// LDR[T] Rd, [Rn], Rm, ROR #
static INSN_REGPARM void arm696(u32 opcode) { LDR_POSTINC(OFFSET_ROR, OP_LDR, 32); }
// STRB[T] Rd, [Rn], Rm, LSL #
static INSN_REGPARM void arm6C0(u32 opcode) { STR_POSTINC(OFFSET_LSL, OP_STRB, 16); }
// STRB[T] Rd, [Rn], Rm, LSR #
static INSN_REGPARM void arm6C2(u32 opcode) { STR_POSTINC(OFFSET_LSR, OP_STRB, 16); }
// STRB[T] Rd, [Rn], Rm, ASR #
static INSN_REGPARM void arm6C4(u32 opcode) { STR_POSTINC(OFFSET_ASR, OP_STRB, 16); }
// STRB[T] Rd, [Rn], Rm, ROR #
static INSN_REGPARM void arm6C6(u32 opcode) { STR_POSTINC(OFFSET_ROR, OP_STRB, 16); }
// LDRB[T] Rd, [Rn], Rm, LSL #
static INSN_REGPARM void arm6D0(u32 opcode) { LDR_POSTINC(OFFSET_LSL, OP_LDRB, 16); }
// LDRB[T] Rd, [Rn], Rm, LSR #
static INSN_REGPARM void arm6D2(u32 opcode) { LDR_POSTINC(OFFSET_LSR, OP_LDRB, 16); }
// LDRB[T] Rd, [Rn], Rm, ASR #
static INSN_REGPARM void arm6D4(u32 opcode) { LDR_POSTINC(OFFSET_ASR, OP_LDRB, 16); }
// LDRB[T] Rd, [Rn], Rm, ROR #
static INSN_REGPARM void arm6D6(u32 opcode) { LDR_POSTINC(OFFSET_ROR, OP_LDRB, 16); }
// STR Rd, [Rn, -Rm, LSL #]
static INSN_REGPARM void arm700(u32 opcode) { STR_PREDEC(OFFSET_LSL, OP_STR, 32); }
// STR Rd, [Rn, -Rm, LSR #]
static INSN_REGPARM void arm702(u32 opcode) { STR_PREDEC(OFFSET_LSR, OP_STR, 32); }
// STR Rd, [Rn, -Rm, ASR #]
static INSN_REGPARM void arm704(u32 opcode) { STR_PREDEC(OFFSET_ASR, OP_STR, 32); }
// STR Rd, [Rn, -Rm, ROR #]
static INSN_REGPARM void arm706(u32 opcode) { STR_PREDEC(OFFSET_ROR, OP_STR, 32); }
// LDR Rd, [Rn, -Rm, LSL #]
static INSN_REGPARM void arm710(u32 opcode) { LDR_PREDEC(OFFSET_LSL, OP_LDR, 32); }
// LDR Rd, [Rn, -Rm, LSR #]
static INSN_REGPARM void arm712(u32 opcode) { LDR_PREDEC(OFFSET_LSR, OP_LDR, 32); }
// LDR Rd, [Rn, -Rm, ASR #]
static INSN_REGPARM void arm714(u32 opcode) { LDR_PREDEC(OFFSET_ASR, OP_LDR, 32); }
// LDR Rd, [Rn, -Rm, ROR #]
static INSN_REGPARM void arm716(u32 opcode) { LDR_PREDEC(OFFSET_ROR, OP_LDR, 32); }
// STR Rd, [Rn, -Rm, LSL #]!
static INSN_REGPARM void arm720(u32 opcode) { STR_PREDEC_WB(OFFSET_LSL, OP_STR, 32); }
// STR Rd, [Rn, -Rm, LSR #]!
static INSN_REGPARM void arm722(u32 opcode) { STR_PREDEC_WB(OFFSET_LSR, OP_STR, 32); }
// STR Rd, [Rn, -Rm, ASR #]!
static INSN_REGPARM void arm724(u32 opcode) { STR_PREDEC_WB(OFFSET_ASR, OP_STR, 32); }
// STR Rd, [Rn, -Rm, ROR #]!
static INSN_REGPARM void arm726(u32 opcode) { STR_PREDEC_WB(OFFSET_ROR, OP_STR, 32); }
// LDR Rd, [Rn, -Rm, LSL #]!
static INSN_REGPARM void arm730(u32 opcode) { LDR_PREDEC_WB(OFFSET_LSL, OP_LDR, 32); }
// LDR Rd, [Rn, -Rm, LSR #]!
static INSN_REGPARM void arm732(u32 opcode) { LDR_PREDEC_WB(OFFSET_LSR, OP_LDR, 32); }
// LDR Rd, [Rn, -Rm, ASR #]!
static INSN_REGPARM void arm734(u32 opcode) { LDR_PREDEC_WB(OFFSET_ASR, OP_LDR, 32); }
// LDR Rd, [Rn, -Rm, ROR #]!
static INSN_REGPARM void arm736(u32 opcode) { LDR_PREDEC_WB(OFFSET_ROR, OP_LDR, 32); }
// STRB Rd, [Rn, -Rm, LSL #]
static INSN_REGPARM void arm740(u32 opcode) { STR_PREDEC(OFFSET_LSL, OP_STRB, 16); }
// STRB Rd, [Rn, -Rm, LSR #]
static INSN_REGPARM void arm742(u32 opcode) { STR_PREDEC(OFFSET_LSR, OP_STRB, 16); }
// STRB Rd, [Rn, -Rm, ASR #]
static INSN_REGPARM void arm744(u32 opcode) { STR_PREDEC(OFFSET_ASR, OP_STRB, 16); }
// STRB Rd, [Rn, -Rm, ROR #]
static INSN_REGPARM void arm746(u32 opcode) { STR_PREDEC(OFFSET_ROR, OP_STRB, 16); }
// LDRB Rd, [Rn, -Rm, LSL #]
static INSN_REGPARM void arm750(u32 opcode) { LDR_PREDEC(OFFSET_LSL, OP_LDRB, 16); }
// LDRB Rd, [Rn, -Rm, LSR #]
static INSN_REGPARM void arm752(u32 opcode) { LDR_PREDEC(OFFSET_LSR, OP_LDRB, 16); }
// LDRB Rd, [Rn, -Rm, ASR #]
static INSN_REGPARM void arm754(u32 opcode) { LDR_PREDEC(OFFSET_ASR, OP_LDRB, 16); }
// LDRB Rd, [Rn, -Rm, ROR #]
static INSN_REGPARM void arm756(u32 opcode) { LDR_PREDEC(OFFSET_ROR, OP_LDRB, 16); }
// STRB Rd, [Rn, -Rm, LSL #]!
static INSN_REGPARM void arm760(u32 opcode) { STR_PREDEC_WB(OFFSET_LSL, OP_STRB, 16); }
// STRB Rd, [Rn, -Rm, LSR #]!
static INSN_REGPARM void arm762(u32 opcode) { STR_PREDEC_WB(OFFSET_LSR, OP_STRB, 16); }
// STRB Rd, [Rn, -Rm, ASR #]!
static INSN_REGPARM void arm764(u32 opcode) { STR_PREDEC_WB(OFFSET_ASR, OP_STRB, 16); }
// STRB Rd, [Rn, -Rm, ROR #]!
static INSN_REGPARM void arm766(u32 opcode) { STR_PREDEC_WB(OFFSET_ROR, OP_STRB, 16); }
// LDRB Rd, [Rn, -Rm, LSL #]!
static INSN_REGPARM void arm770(u32 opcode) { LDR_PREDEC_WB(OFFSET_LSL, OP_LDRB, 16); }
// LDRB Rd, [Rn, -Rm, LSR #]!
static INSN_REGPARM void arm772(u32 opcode) { LDR_PREDEC_WB(OFFSET_LSR, OP_LDRB, 16); }
// LDRB Rd, [Rn, -Rm, ASR #]!
static INSN_REGPARM void arm774(u32 opcode) { LDR_PREDEC_WB(OFFSET_ASR, OP_LDRB, 16); }
// LDRB Rd, [Rn, -Rm, ROR #]!
static INSN_REGPARM void arm776(u32 opcode) { LDR_PREDEC_WB(OFFSET_ROR, OP_LDRB, 16); }
// STR Rd, [Rn, Rm, LSL #]
static INSN_REGPARM void arm780(u32 opcode) { STR_PREINC(OFFSET_LSL, OP_STR, 32); }
// STR Rd, [Rn, Rm, LSR #]
static INSN_REGPARM void arm782(u32 opcode) { STR_PREINC(OFFSET_LSR, OP_STR, 32); }
// STR Rd, [Rn, Rm, ASR #]
static INSN_REGPARM void arm784(u32 opcode) { STR_PREINC(OFFSET_ASR, OP_STR, 32); }
// STR Rd, [Rn, Rm, ROR #]
static INSN_REGPARM void arm786(u32 opcode) { STR_PREINC(OFFSET_ROR, OP_STR, 32); }
// LDR Rd, [Rn, Rm, LSL #]
static INSN_REGPARM void arm790(u32 opcode) { LDR_PREINC(OFFSET_LSL, OP_LDR, 32); }
// LDR Rd, [Rn, Rm, LSR #]
static INSN_REGPARM void arm792(u32 opcode) { LDR_PREINC(OFFSET_LSR, OP_LDR, 32); }
// LDR Rd, [Rn, Rm, ASR #]
static INSN_REGPARM void arm794(u32 opcode) { LDR_PREINC(OFFSET_ASR, OP_LDR, 32); }
// LDR Rd, [Rn, Rm, ROR #]
static INSN_REGPARM void arm796(u32 opcode) { LDR_PREINC(OFFSET_ROR, OP_LDR, 32); }
// STR Rd, [Rn, Rm, LSL #]!
static INSN_REGPARM void arm7A0(u32 opcode) { STR_PREINC_WB(OFFSET_LSL, OP_STR, 32); }
// STR Rd, [Rn, Rm, LSR #]!
static INSN_REGPARM void arm7A2(u32 opcode) { STR_PREINC_WB(OFFSET_LSR, OP_STR, 32); }
// STR Rd, [Rn, Rm, ASR #]!
static INSN_REGPARM void arm7A4(u32 opcode) { STR_PREINC_WB(OFFSET_ASR, OP_STR, 32); }
// STR Rd, [Rn, Rm, ROR #]!
static INSN_REGPARM void arm7A6(u32 opcode) { STR_PREINC_WB(OFFSET_ROR, OP_STR, 32); }
// LDR Rd, [Rn, Rm, LSL #]!
static INSN_REGPARM void arm7B0(u32 opcode) { LDR_PREINC_WB(OFFSET_LSL, OP_LDR, 32); }
// LDR Rd, [Rn, Rm, LSR #]!
static INSN_REGPARM void arm7B2(u32 opcode) { LDR_PREINC_WB(OFFSET_LSR, OP_LDR, 32); }
// LDR Rd, [Rn, Rm, ASR #]!
static INSN_REGPARM void arm7B4(u32 opcode) { LDR_PREINC_WB(OFFSET_ASR, OP_LDR, 32); }
// LDR Rd, [Rn, Rm, ROR #]!
static INSN_REGPARM void arm7B6(u32 opcode) { LDR_PREINC_WB(OFFSET_ROR, OP_LDR, 32); }
// STRB Rd, [Rn, Rm, LSL #]
static INSN_REGPARM void arm7C0(u32 opcode) { STR_PREINC(OFFSET_LSL, OP_STRB, 16); }
// STRB Rd, [Rn, Rm, LSR #]
static INSN_REGPARM void arm7C2(u32 opcode) { STR_PREINC(OFFSET_LSR, OP_STRB, 16); }
// STRB Rd, [Rn, Rm, ASR #]
static INSN_REGPARM void arm7C4(u32 opcode) { STR_PREINC(OFFSET_ASR, OP_STRB, 16); }
// STRB Rd, [Rn, Rm, ROR #]
static INSN_REGPARM void arm7C6(u32 opcode) { STR_PREINC(OFFSET_ROR, OP_STRB, 16); }
// LDRB Rd, [Rn, Rm, LSL #]
static INSN_REGPARM void arm7D0(u32 opcode) { LDR_PREINC(OFFSET_LSL, OP_LDRB, 16); }
// LDRB Rd, [Rn, Rm, LSR #]
static INSN_REGPARM void arm7D2(u32 opcode) { LDR_PREINC(OFFSET_LSR, OP_LDRB, 16); }
// LDRB Rd, [Rn, Rm, ASR #]
static INSN_REGPARM void arm7D4(u32 opcode) { LDR_PREINC(OFFSET_ASR, OP_LDRB, 16); }
// LDRB Rd, [Rn, Rm, ROR #]
static INSN_REGPARM void arm7D6(u32 opcode) { LDR_PREINC(OFFSET_ROR, OP_LDRB, 16); }
// STRB Rd, [Rn, Rm, LSL #]!
static INSN_REGPARM void arm7E0(u32 opcode) { STR_PREINC_WB(OFFSET_LSL, OP_STRB, 16); }
// STRB Rd, [Rn, Rm, LSR #]!
static INSN_REGPARM void arm7E2(u32 opcode) { STR_PREINC_WB(OFFSET_LSR, OP_STRB, 16); }
// STRB Rd, [Rn, Rm, ASR #]!
static INSN_REGPARM void arm7E4(u32 opcode) { STR_PREINC_WB(OFFSET_ASR, OP_STRB, 16); }
// STRB Rd, [Rn, Rm, ROR #]!
static INSN_REGPARM void arm7E6(u32 opcode) { STR_PREINC_WB(OFFSET_ROR, OP_STRB, 16); }
// LDRB Rd, [Rn, Rm, LSL #]!
static INSN_REGPARM void arm7F0(u32 opcode) { LDR_PREINC_WB(OFFSET_LSL, OP_LDRB, 16); }
// LDRB Rd, [Rn, Rm, LSR #]!
static INSN_REGPARM void arm7F2(u32 opcode) { LDR_PREINC_WB(OFFSET_LSR, OP_LDRB, 16); }
// LDRB Rd, [Rn, Rm, ASR #]!
static INSN_REGPARM void arm7F4(u32 opcode) { LDR_PREINC_WB(OFFSET_ASR, OP_LDRB, 16); }
// LDRB Rd, [Rn, Rm, ROR #]!
static INSN_REGPARM void arm7F6(u32 opcode) { LDR_PREINC_WB(OFFSET_ROR, OP_LDRB, 16); }

// STM/LDM ////////////////////////////////////////////////////////////////

#define STM_REG(bit,num) \
    if (opcode & (1U<<(bit))) {                         \
        CPUWriteMemory(address, reg[(num)].I);          \
        if (!count) {                                   \
            clockTicks += 1 + dataTicksAccess32(address);\
        } else {                                        \
            clockTicks += 1 + dataTicksAccessSeq32(address);\
        }                                               \
        count++;                                        \
        address += 4;                                   \
    }
#define STMW_REG(bit,num) \
    if (opcode & (1U<<(bit))) {                         \
        CPUWriteMemory(address, reg[(num)].I);          \
        if (!count) {                                   \
            clockTicks += 1 + dataTicksAccess32(address);\
        } else {                                        \
            clockTicks += 1 + dataTicksAccessSeq32(address);\
        }                                               \
        reg[base].I = temp;                             \
        count++;                                        \
        address += 4;                                   \
    }
#define LDM_REG(bit,num) \
    if (opcode & (1U<<(bit))) {                         \
        reg[(num)].I = CPUReadMemory(address);          \
        if (!count) {                                   \
            clockTicks += 1 + dataTicksAccess32(address);\
        } else {                                        \
            clockTicks += 1 + dataTicksAccessSeq32(address);\
        }                                               \
        count++;                                        \
        address += 4;                                   \
    }
#define STM_LOW(STORE_REG) \
    STORE_REG(0, 0);                                    \
    STORE_REG(1, 1);                                    \
    STORE_REG(2, 2);                                    \
    STORE_REG(3, 3);                                    \
    STORE_REG(4, 4);                                    \
    STORE_REG(5, 5);                                    \
    STORE_REG(6, 6);                                    \
    STORE_REG(7, 7);
#define STM_HIGH(STORE_REG) \
    STORE_REG(8, 8);                                    \
    STORE_REG(9, 9);                                    \
    STORE_REG(10, 10);                                  \
    STORE_REG(11, 11);                                  \
    STORE_REG(12, 12);                                  \
    STORE_REG(13, 13);                                  \
    STORE_REG(14, 14);
#define STM_HIGH_2(STORE_REG) \
    if (armMode == 0x11) {                              \
        STORE_REG(8, R8_FIQ);                           \
        STORE_REG(9, R9_FIQ);                           \
        STORE_REG(10, R10_FIQ);                         \
        STORE_REG(11, R11_FIQ);                         \
        STORE_REG(12, R12_FIQ);                         \
    } else {                                            \
        STORE_REG(8, 8);                                \
        STORE_REG(9, 9);                                \
        STORE_REG(10, 10);                              \
        STORE_REG(11, 11);                              \
        STORE_REG(12, 12);                              \
    }                                                   \
    if (armMode != 0x10 && armMode != 0x1F) {           \
        STORE_REG(13, R13_USR);                         \
        STORE_REG(14, R14_USR);                         \
    } else {                                            \
        STORE_REG(13, 13);                              \
        STORE_REG(14, 14);                              \
    }
#define STM_PC \
    if (opcode & (1U<<15)) {                            \
        CPUWriteMemory(address, reg[15].I+4);           \
        if (!count) {                                   \
            clockTicks += 1 + dataTicksAccess32(address);\
        } else {                                        \
            clockTicks += 1 + dataTicksAccessSeq32(address);\
        }                                               \
        count++;                                        \
    }
#define STMW_PC \
    if (opcode & (1U<<15)) {                            \
        CPUWriteMemory(address, reg[15].I+4);           \
        if (!count) {                                   \
            clockTicks += 1 + dataTicksAccess32(address);\
        } else {                                        \
            clockTicks += 1 + dataTicksAccessSeq32(address);\
        }                                               \
        reg[base].I = temp;                             \
        count++;                                        \
    }
#define LDM_LOW \
    LDM_REG(0, 0);                                      \
    LDM_REG(1, 1);                                      \
    LDM_REG(2, 2);                                      \
    LDM_REG(3, 3);                                      \
    LDM_REG(4, 4);                                      \
    LDM_REG(5, 5);                                      \
    LDM_REG(6, 6);                                      \
    LDM_REG(7, 7);
#define LDM_HIGH \
    LDM_REG(8, 8);                                      \
    LDM_REG(9, 9);                                      \
    LDM_REG(10, 10);                                    \
    LDM_REG(11, 11);                                    \
    LDM_REG(12, 12);                                    \
    LDM_REG(13, 13);                                    \
    LDM_REG(14, 14);
#define LDM_HIGH_2 \
    if (armMode == 0x11) {                              \
        LDM_REG(8, R8_FIQ);                             \
        LDM_REG(9, R9_FIQ);                             \
        LDM_REG(10, R10_FIQ);                           \
        LDM_REG(11, R11_FIQ);                           \
        LDM_REG(12, R12_FIQ);                           \
    } else {                                            \
        LDM_REG(8, 8);                                  \
        LDM_REG(9, 9);                                  \
        LDM_REG(10, 10);                                \
        LDM_REG(11, 11);                                \
        LDM_REG(12, 12);                                \
    }                                                   \
    if (armMode != 0x10 && armMode != 0x1F) {           \
        LDM_REG(13, R13_USR);                           \
        LDM_REG(14, R14_USR);                           \
    } else {                                            \
        LDM_REG(13, 13);                                \
        LDM_REG(14, 14);                                \
    }
#define STM_ALL \
    STM_LOW(STM_REG);                                   \
    STM_HIGH(STM_REG);                                  \
    STM_PC;
#define STMW_ALL \
    STM_LOW(STMW_REG);                                  \
    STM_HIGH(STMW_REG);                                 \
    STMW_PC;
#define LDM_ALL \
    LDM_LOW;                                            \
    LDM_HIGH;                                           \
    if (opcode & (1U<<15)) {                            \
        reg[15].I = CPUReadMemory(address);             \
        if (!count) {                                   \
            clockTicks += 1 + dataTicksAccess32(address);\
        } else {                                        \
            clockTicks += 1 + dataTicksAccessSeq32(address);\
        }                                               \
        count++;                                        \
    }                                                   \
    if (opcode & (1U<<15)) {                            \
        armNextPC = reg[15].I;                          \
        reg[15].I += 4;                                 \
        ARM_PREFETCH;                                   \
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);\
    }
#define STM_ALL_2 \
    STM_LOW(STM_REG);                                   \
    STM_HIGH_2(STM_REG);                                \
    STM_PC;
#define STMW_ALL_2 \
    STM_LOW(STMW_REG);                                  \
    STM_HIGH_2(STMW_REG);                               \
    STMW_PC;
#define LDM_ALL_2 \
    LDM_LOW;                                            \
    if (opcode & (1U<<15)) {                            \
        LDM_HIGH;                                       \
        reg[15].I = CPUReadMemory(address);             \
        if (!count) {                                   \
            clockTicks += 1 + dataTicksAccess32(address); \
        } else {                                        \
            clockTicks += 1 + dataTicksAccessSeq32(address); \
        }                                               \
        count++;                                        \
    } else {                                            \
        LDM_HIGH_2;                                     \
    }
#define LDM_ALL_2B \
    if (opcode & (1U<<15)) {                            \
        CPUSwitchMode(reg[17].I & 0x1F, false);         \
        if (armState) {                                 \
            armNextPC = reg[15].I & 0xFFFFFFFC;         \
            reg[15].I = armNextPC + 4;                  \
            ARM_PREFETCH;                               \
        } else {                                        \
            armNextPC = reg[15].I & 0xFFFFFFFE;         \
            reg[15].I = armNextPC + 2;                  \
            THUMB_PREFETCH;                             \
        }                                               \
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);\
    }


// STMDA Rn, {Rlist}
static INSN_REGPARM void arm800(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (temp + 4) & 0xFFFFFFFC;
    int count = 0;
    STM_ALL;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMDA Rn, {Rlist}
static INSN_REGPARM void arm810(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (temp + 4) & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMDA Rn!, {Rlist}
static INSN_REGPARM void arm820(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (temp+4) & 0xFFFFFFFC;
    int count = 0;
    STMW_ALL;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMDA Rn!, {Rlist}
static INSN_REGPARM void arm830(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (temp + 4) & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
    if (!(opcode & (1U << base)))
        reg[base].I = temp;
}

// STMDA Rn, {Rlist}^
static INSN_REGPARM void arm840(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (temp+4) & 0xFFFFFFFC;
    int count = 0;
    STM_ALL_2;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMDA Rn, {Rlist}^
static INSN_REGPARM void arm850(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (temp + 4) & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL_2;
    LDM_ALL_2B;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMDA Rn!, {Rlist}^
static INSN_REGPARM void arm860(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (temp+4) & 0xFFFFFFFC;
    int count = 0;
    STMW_ALL_2;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMDA Rn!, {Rlist}^
static INSN_REGPARM void arm870(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (temp + 4) & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL_2;
    if (!(opcode & (1U << base)))
        reg[base].I = temp;
    LDM_ALL_2B;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMIA Rn, {Rlist}
static INSN_REGPARM void arm880(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = reg[base].I & 0xFFFFFFFC;
    int count = 0;
    STM_ALL;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMIA Rn, {Rlist}
static INSN_REGPARM void arm890(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = reg[base].I & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMIA Rn!, {Rlist}
static INSN_REGPARM void arm8A0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = reg[base].I & 0xFFFFFFFC;
    int count = 0;
    u32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 0xFF] + cpuBitsSet[(opcode >> 8) & 255]);
    STMW_ALL;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMIA Rn!, {Rlist}
static INSN_REGPARM void arm8B0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = reg[base].I & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
    if (!(opcode & (1U << base)))
        reg[base].I = temp;
}

// STMIA Rn, {Rlist}^
static INSN_REGPARM void arm8C0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = reg[base].I & 0xFFFFFFFC;
    int count = 0;
    STM_ALL_2;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMIA Rn, {Rlist}^
static INSN_REGPARM void arm8D0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = reg[base].I & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL_2;
    LDM_ALL_2B;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMIA Rn!, {Rlist}^
static INSN_REGPARM void arm8E0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = reg[base].I & 0xFFFFFFFC;
    int count = 0;
    u32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 0xFF] + cpuBitsSet[(opcode >> 8) & 255]);
    STMW_ALL_2;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMIA Rn!, {Rlist}^
static INSN_REGPARM void arm8F0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = reg[base].I & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL_2;
    if (!(opcode & (1U << base)))
        reg[base].I = temp;
    LDM_ALL_2B;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMDB Rn, {Rlist}
static INSN_REGPARM void arm900(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = temp & 0xFFFFFFFC;
    int count = 0;
    STM_ALL;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMDB Rn, {Rlist}
static INSN_REGPARM void arm910(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = temp & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMDB Rn!, {Rlist}
static INSN_REGPARM void arm920(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = temp & 0xFFFFFFFC;
    int count = 0;
    STMW_ALL;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMDB Rn!, {Rlist}
static INSN_REGPARM void arm930(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = temp & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
    if (!(opcode & (1U << base)))
        reg[base].I = temp;
}

// STMDB Rn, {Rlist}^
static INSN_REGPARM void arm940(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = temp & 0xFFFFFFFC;
    int count = 0;
    STM_ALL_2;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMDB Rn, {Rlist}^
static INSN_REGPARM void arm950(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = temp & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL_2;
    LDM_ALL_2B;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMDB Rn!, {Rlist}^
static INSN_REGPARM void arm960(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = temp & 0xFFFFFFFC;
    int count = 0;
    STMW_ALL_2;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMDB Rn!, {Rlist}^
static INSN_REGPARM void arm970(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I -
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = temp & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL_2;
    if (!(opcode & (1U << base)))
        reg[base].I = temp;
    LDM_ALL_2B;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMIB Rn, {Rlist}
static INSN_REGPARM void arm980(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = (reg[base].I+4) & 0xFFFFFFFC;
    int count = 0;
    STM_ALL;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMIB Rn, {Rlist}
static INSN_REGPARM void arm990(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = (reg[base].I+4) & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMIB Rn!, {Rlist}
static INSN_REGPARM void arm9A0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = (reg[base].I+4) & 0xFFFFFFFC;
    int count = 0;
    u32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 0xFF] + cpuBitsSet[(opcode >> 8) & 255]);
    STMW_ALL;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMIB Rn!, {Rlist}
static INSN_REGPARM void arm9B0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (reg[base].I+4) & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
    if (!(opcode & (1U << base)))
        reg[base].I = temp;
}

// STMIB Rn, {Rlist}^
static INSN_REGPARM void arm9C0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = (reg[base].I+4) & 0xFFFFFFFC;
    int count = 0;
    STM_ALL_2;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMIB Rn, {Rlist}^
static INSN_REGPARM void arm9D0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = (reg[base].I+4) & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL_2;
    LDM_ALL_2B;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// STMIB Rn!, {Rlist}^
static INSN_REGPARM void arm9E0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 address = (reg[base].I+4) & 0xFFFFFFFC;
    int count = 0;
    u32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 0xFF] + cpuBitsSet[(opcode >> 8) & 255]);
    STMW_ALL_2;
    clockTicks += 1 + codeTicksAccess32(armNextPC);
}

// LDMIB Rn!, {Rlist}^
static INSN_REGPARM void arm9F0(u32 opcode)
{
    if (busPrefetchCount == 0)
        busPrefetch = busPrefetchEnable;
    int base = (opcode & 0x000F0000) >> 16;
    u32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
    u32 address = (reg[base].I+4) & 0xFFFFFFFC;
    int count = 0;
    LDM_ALL_2;
    if (!(opcode & (1U << base)))
        reg[base].I = temp;
    LDM_ALL_2B;
    clockTicks += 2 + codeTicksAccess32(armNextPC);
}

// B/BL/SWI and (unimplemented) coproc support ////////////////////////////

// B <offset>
static INSN_REGPARM void armA00(u32 opcode)
{
    int offset = opcode & 0x00FFFFFF;
    if (offset & 0x00800000)
        offset |= 0xFF000000;  // negative offset
    reg[15].I += offset<<2;
    armNextPC = reg[15].I;
    reg[15].I += 4;
    ARM_PREFETCH;
    clockTicks = codeTicksAccessSeq32(armNextPC) + 1;
    clockTicks += 2 + codeTicksAccess32(armNextPC)
                    + codeTicksAccessSeq32(armNextPC);
    busPrefetchCount = 0;
}

// BL <offset>
static INSN_REGPARM void armB00(u32 opcode)
{
    int offset = opcode & 0x00FFFFFF;
    if (offset & 0x00800000)
        offset |= 0xFF000000;  // negative offset
    reg[14].I = reg[15].I - 4;
    reg[15].I += offset<<2;
    armNextPC = reg[15].I;
    reg[15].I += 4;
    ARM_PREFETCH;
    clockTicks = codeTicksAccessSeq32(armNextPC) + 1;
    clockTicks += 2 + codeTicksAccess32(armNextPC)
                    + codeTicksAccessSeq32(armNextPC);
    busPrefetchCount = 0;
}


#ifdef GP_SUPPORT
// MRC
static INSN_REGPARM void armE01(u32 opcode)
{
}
#else
 #define armE01 armUnknownInsn
#endif


// SWI <comment>
static INSN_REGPARM void armF00(u32 opcode)
{
    clockTicks = codeTicksAccessSeq32(armNextPC) + 1;
    clockTicks += 2 + codeTicksAccess32(armNextPC)
                    + codeTicksAccessSeq32(armNextPC);
    busPrefetchCount = 0;
    CPUSoftwareInterrupt(opcode & 0x00FFFFFF);
}

// Instruction table //////////////////////////////////////////////////////

typedef INSN_REGPARM void (*insnfunc_t)(u32 opcode);
#define REP16(insn) \
    insn,insn,insn,insn,insn,insn,insn,insn,\
    insn,insn,insn,insn,insn,insn,insn,insn
#define REP256(insn) \
    REP16(insn),REP16(insn),REP16(insn),REP16(insn),\
    REP16(insn),REP16(insn),REP16(insn),REP16(insn),\
    REP16(insn),REP16(insn),REP16(insn),REP16(insn),\
    REP16(insn),REP16(insn),REP16(insn),REP16(insn)
#define arm_UI armUnknownInsn
#ifdef BKPT_SUPPORT
 #define arm_BP armBreakpoint
#else
 #define arm_BP armUnknownInsn
#endif
static insnfunc_t armInsnTable[4096] = {
    arm000,arm001,arm002,arm003,arm004,arm005,arm006,arm007,  // 000
    arm000,arm009,arm002,arm00B,arm004,arm_UI,arm006,arm_UI,  // 008
    arm010,arm011,arm012,arm013,arm014,arm015,arm016,arm017,  // 010
    arm010,arm019,arm012,arm01B,arm014,arm01D,arm016,arm01F,  // 018
    arm020,arm021,arm022,arm023,arm024,arm025,arm026,arm027,  // 020
    arm020,arm029,arm022,arm_UI,arm024,arm_UI,arm026,arm_UI,  // 028
    arm030,arm031,arm032,arm033,arm034,arm035,arm036,arm037,  // 030
    arm030,arm039,arm032,arm_UI,arm034,arm01D,arm036,arm01F,  // 038
    arm040,arm041,arm042,arm043,arm044,arm045,arm046,arm047,  // 040
    arm040,arm_UI,arm042,arm04B,arm044,arm_UI,arm046,arm_UI,  // 048
    arm050,arm051,arm052,arm053,arm054,arm055,arm056,arm057,  // 050
    arm050,arm_UI,arm052,arm05B,arm054,arm05D,arm056,arm05F,  // 058
    arm060,arm061,arm062,arm063,arm064,arm065,arm066,arm067,  // 060
    arm060,arm_UI,arm062,arm_UI,arm064,arm_UI,arm066,arm_UI,  // 068
    arm070,arm071,arm072,arm073,arm074,arm075,arm076,arm077,  // 070
    arm070,arm_UI,arm072,arm_UI,arm074,arm05D,arm076,arm05F,  // 078
    arm080,arm081,arm082,arm083,arm084,arm085,arm086,arm087,  // 080
    arm080,arm089,arm082,arm08B,arm084,arm_UI,arm086,arm_UI,  // 088
    arm090,arm091,arm092,arm093,arm094,arm095,arm096,arm097,  // 090
    arm090,arm099,arm092,arm09B,arm094,arm09D,arm096,arm09F,  // 098
    arm0A0,arm0A1,arm0A2,arm0A3,arm0A4,arm0A5,arm0A6,arm0A7,  // 0A0
    arm0A0,arm0A9,arm0A2,arm_UI,arm0A4,arm_UI,arm0A6,arm_UI,  // 0A8
    arm0B0,arm0B1,arm0B2,arm0B3,arm0B4,arm0B5,arm0B6,arm0B7,  // 0B0
    arm0B0,arm0B9,arm0B2,arm_UI,arm0B4,arm09D,arm0B6,arm09F,  // 0B8
    arm0C0,arm0C1,arm0C2,arm0C3,arm0C4,arm0C5,arm0C6,arm0C7,  // 0C0
    arm0C0,arm0C9,arm0C2,arm0CB,arm0C4,arm_UI,arm0C6,arm_UI,  // 0C8
    arm0D0,arm0D1,arm0D2,arm0D3,arm0D4,arm0D5,arm0D6,arm0D7,  // 0D0
    arm0D0,arm0D9,arm0D2,arm0DB,arm0D4,arm0DD,arm0D6,arm0DF,  // 0D8
    arm0E0,arm0E1,arm0E2,arm0E3,arm0E4,arm0E5,arm0E6,arm0E7,  // 0E0
    arm0E0,arm0E9,arm0E2,arm_UI,arm0E4,arm_UI,arm0E6,arm_UI,  // 0E8
    arm0F0,arm0F1,arm0F2,arm0F3,arm0F4,arm0F5,arm0F6,arm0F7,  // 0F0
    arm0F0,arm0F9,arm0F2,arm_UI,arm0F4,arm0DD,arm0F6,arm0DF,  // 0F8

    arm100,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,  // 100
    arm_UI,arm109,arm_UI,arm10B,arm_UI,arm_UI,arm_UI,arm_UI,  // 108
    arm110,arm111,arm112,arm113,arm114,arm115,arm116,arm117,  // 110
    arm110,arm_UI,arm112,arm11B,arm114,arm11D,arm116,arm11F,  // 118
    arm120,arm121,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,arm_BP,  // 120
    arm_UI,arm_UI,arm_UI,arm12B,arm_UI,arm_UI,arm_UI,arm_UI,  // 128
    arm130,arm131,arm132,arm133,arm134,arm135,arm136,arm137,  // 130
    arm130,arm_UI,arm132,arm13B,arm134,arm13D,arm136,arm13F,  // 138
    arm140,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,  // 140
    arm_UI,arm149,arm_UI,arm14B,arm_UI,arm_UI,arm_UI,arm_UI,  // 148
    arm150,arm151,arm152,arm153,arm154,arm155,arm156,arm157,  // 150
    arm150,arm_UI,arm152,arm15B,arm154,arm15D,arm156,arm15F,  // 158
    arm160,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,arm_UI,  // 160
    arm_UI,arm_UI,arm_UI,arm16B,arm_UI,arm_UI,arm_UI,arm_UI,  // 168
    arm170,arm171,arm172,arm173,arm174,arm175,arm176,arm177,  // 170
    arm170,arm_UI,arm172,arm17B,arm174,arm17D,arm176,arm17F,  // 178
    arm180,arm181,arm182,arm183,arm184,arm185,arm186,arm187,  // 180
    arm180,arm_UI,arm182,arm18B,arm184,arm_UI,arm186,arm_UI,  // 188
    arm190,arm191,arm192,arm193,arm194,arm195,arm196,arm197,  // 190
    arm190,arm_UI,arm192,arm19B,arm194,arm19D,arm196,arm19F,  // 198
    arm1A0,arm1A1,arm1A2,arm1A3,arm1A4,arm1A5,arm1A6,arm1A7,  // 1A0
    arm1A0,arm_UI,arm1A2,arm1AB,arm1A4,arm_UI,arm1A6,arm_UI,  // 1A8
    arm1B0,arm1B1,arm1B2,arm1B3,arm1B4,arm1B5,arm1B6,arm1B7,  // 1B0
    arm1B0,arm_UI,arm1B2,arm1BB,arm1B4,arm1BD,arm1B6,arm1BF,  // 1B8
    arm1C0,arm1C1,arm1C2,arm1C3,arm1C4,arm1C5,arm1C6,arm1C7,  // 1C0
    arm1C0,arm_UI,arm1C2,arm1CB,arm1C4,arm_UI,arm1C6,arm_UI,  // 1C8
    arm1D0,arm1D1,arm1D2,arm1D3,arm1D4,arm1D5,arm1D6,arm1D7,  // 1D0
    arm1D0,arm_UI,arm1D2,arm1DB,arm1D4,arm1DD,arm1D6,arm1DF,  // 1D8
    arm1E0,arm1E1,arm1E2,arm1E3,arm1E4,arm1E5,arm1E6,arm1E7,  // 1E0
    arm1E0,arm_UI,arm1E2,arm1EB,arm1E4,arm_UI,arm1E6,arm_UI,  // 1E8
    arm1F0,arm1F1,arm1F2,arm1F3,arm1F4,arm1F5,arm1F6,arm1F7,  // 1F0
    arm1F0,arm_UI,arm1F2,arm1FB,arm1F4,arm1FD,arm1F6,arm1FF,  // 1F8

    REP16(arm200),REP16(arm210),REP16(arm220),REP16(arm230),  // 200
    REP16(arm240),REP16(arm250),REP16(arm260),REP16(arm270),  // 240
    REP16(arm280),REP16(arm290),REP16(arm2A0),REP16(arm2B0),  // 280
    REP16(arm2C0),REP16(arm2D0),REP16(arm2E0),REP16(arm2F0),  // 2C0
    REP16(arm_UI),REP16(arm310),REP16(arm320),REP16(arm330),  // 300
    REP16(arm_UI),REP16(arm350),REP16(arm360),REP16(arm370),  // 340
    REP16(arm380),REP16(arm390),REP16(arm3A0),REP16(arm3B0),  // 380
    REP16(arm3C0),REP16(arm3D0),REP16(arm3E0),REP16(arm3F0),  // 3C0

    REP16(arm400),REP16(arm410),REP16(arm400),REP16(arm410),  // 400
    REP16(arm440),REP16(arm450),REP16(arm440),REP16(arm450),  // 440
    REP16(arm480),REP16(arm490),REP16(arm480),REP16(arm490),  // 480
    REP16(arm4C0),REP16(arm4D0),REP16(arm4C0),REP16(arm4D0),  // 4C0
    REP16(arm500),REP16(arm510),REP16(arm520),REP16(arm530),  // 500
    REP16(arm540),REP16(arm550),REP16(arm560),REP16(arm570),  // 540
    REP16(arm580),REP16(arm590),REP16(arm5A0),REP16(arm5B0),  // 580
    REP16(arm5C0),REP16(arm5D0),REP16(arm5E0),REP16(arm5F0),  // 5C0

    arm600,arm_UI,arm602,arm_UI,arm604,arm_UI,arm606,arm_UI,  // 600
    arm600,arm_UI,arm602,arm_UI,arm604,arm_UI,arm606,arm_UI,  // 608
    arm610,arm_UI,arm612,arm_UI,arm614,arm_UI,arm616,arm_UI,  // 610
    arm610,arm_UI,arm612,arm_UI,arm614,arm_UI,arm616,arm_UI,  // 618
    arm600,arm_UI,arm602,arm_UI,arm604,arm_UI,arm606,arm_UI,  // 620
    arm600,arm_UI,arm602,arm_UI,arm604,arm_UI,arm606,arm_UI,  // 628
    arm610,arm_UI,arm612,arm_UI,arm614,arm_UI,arm616,arm_UI,  // 630
    arm610,arm_UI,arm612,arm_UI,arm614,arm_UI,arm616,arm_UI,  // 638
    arm640,arm_UI,arm642,arm_UI,arm644,arm_UI,arm646,arm_UI,  // 640
    arm640,arm_UI,arm642,arm_UI,arm644,arm_UI,arm646,arm_UI,  // 648
    arm650,arm_UI,arm652,arm_UI,arm654,arm_UI,arm656,arm_UI,  // 650
    arm650,arm_UI,arm652,arm_UI,arm654,arm_UI,arm656,arm_UI,  // 658
    arm640,arm_UI,arm642,arm_UI,arm644,arm_UI,arm646,arm_UI,  // 660
    arm640,arm_UI,arm642,arm_UI,arm644,arm_UI,arm646,arm_UI,  // 668
    arm650,arm_UI,arm652,arm_UI,arm654,arm_UI,arm656,arm_UI,  // 670
    arm650,arm_UI,arm652,arm_UI,arm654,arm_UI,arm656,arm_UI,  // 678
    arm680,arm_UI,arm682,arm_UI,arm684,arm_UI,arm686,arm_UI,  // 680
    arm680,arm_UI,arm682,arm_UI,arm684,arm_UI,arm686,arm_UI,  // 688
    arm690,arm_UI,arm692,arm_UI,arm694,arm_UI,arm696,arm_UI,  // 690
    arm690,arm_UI,arm692,arm_UI,arm694,arm_UI,arm696,arm_UI,  // 698
    arm680,arm_UI,arm682,arm_UI,arm684,arm_UI,arm686,arm_UI,  // 6A0
    arm680,arm_UI,arm682,arm_UI,arm684,arm_UI,arm686,arm_UI,  // 6A8
    arm690,arm_UI,arm692,arm_UI,arm694,arm_UI,arm696,arm_UI,  // 6B0
    arm690,arm_UI,arm692,arm_UI,arm694,arm_UI,arm696,arm_UI,  // 6B8
    arm6C0,arm_UI,arm6C2,arm_UI,arm6C4,arm_UI,arm6C6,arm_UI,  // 6C0
    arm6C0,arm_UI,arm6C2,arm_UI,arm6C4,arm_UI,arm6C6,arm_UI,  // 6C8
    arm6D0,arm_UI,arm6D2,arm_UI,arm6D4,arm_UI,arm6D6,arm_UI,  // 6D0
    arm6D0,arm_UI,arm6D2,arm_UI,arm6D4,arm_UI,arm6D6,arm_UI,  // 6D8
    arm6C0,arm_UI,arm6C2,arm_UI,arm6C4,arm_UI,arm6C6,arm_UI,  // 6E0
    arm6C0,arm_UI,arm6C2,arm_UI,arm6C4,arm_UI,arm6C6,arm_UI,  // 6E8
    arm6D0,arm_UI,arm6D2,arm_UI,arm6D4,arm_UI,arm6D6,arm_UI,  // 6F0
    arm6D0,arm_UI,arm6D2,arm_UI,arm6D4,arm_UI,arm6D6,arm_UI,  // 6F8

    arm700,arm_UI,arm702,arm_UI,arm704,arm_UI,arm706,arm_UI,  // 700
    arm700,arm_UI,arm702,arm_UI,arm704,arm_UI,arm706,arm_UI,  // 708
    arm710,arm_UI,arm712,arm_UI,arm714,arm_UI,arm716,arm_UI,  // 710
    arm710,arm_UI,arm712,arm_UI,arm714,arm_UI,arm716,arm_UI,  // 718
    arm720,arm_UI,arm722,arm_UI,arm724,arm_UI,arm726,arm_UI,  // 720
    arm720,arm_UI,arm722,arm_UI,arm724,arm_UI,arm726,arm_UI,  // 728
    arm730,arm_UI,arm732,arm_UI,arm734,arm_UI,arm736,arm_UI,  // 730
    arm730,arm_UI,arm732,arm_UI,arm734,arm_UI,arm736,arm_UI,  // 738
    arm740,arm_UI,arm742,arm_UI,arm744,arm_UI,arm746,arm_UI,  // 740
    arm740,arm_UI,arm742,arm_UI,arm744,arm_UI,arm746,arm_UI,  // 748
    arm750,arm_UI,arm752,arm_UI,arm754,arm_UI,arm756,arm_UI,  // 750
    arm750,arm_UI,arm752,arm_UI,arm754,arm_UI,arm756,arm_UI,  // 758
    arm760,arm_UI,arm762,arm_UI,arm764,arm_UI,arm766,arm_UI,  // 760
    arm760,arm_UI,arm762,arm_UI,arm764,arm_UI,arm766,arm_UI,  // 768
    arm770,arm_UI,arm772,arm_UI,arm774,arm_UI,arm776,arm_UI,  // 770
    arm770,arm_UI,arm772,arm_UI,arm774,arm_UI,arm776,arm_UI,  // 778
    arm780,arm_UI,arm782,arm_UI,arm784,arm_UI,arm786,arm_UI,  // 780
    arm780,arm_UI,arm782,arm_UI,arm784,arm_UI,arm786,arm_UI,  // 788
    arm790,arm_UI,arm792,arm_UI,arm794,arm_UI,arm796,arm_UI,  // 790
    arm790,arm_UI,arm792,arm_UI,arm794,arm_UI,arm796,arm_UI,  // 798
    arm7A0,arm_UI,arm7A2,arm_UI,arm7A4,arm_UI,arm7A6,arm_UI,  // 7A0
    arm7A0,arm_UI,arm7A2,arm_UI,arm7A4,arm_UI,arm7A6,arm_UI,  // 7A8
    arm7B0,arm_UI,arm7B2,arm_UI,arm7B4,arm_UI,arm7B6,arm_UI,  // 7B0
    arm7B0,arm_UI,arm7B2,arm_UI,arm7B4,arm_UI,arm7B6,arm_UI,  // 7B8
    arm7C0,arm_UI,arm7C2,arm_UI,arm7C4,arm_UI,arm7C6,arm_UI,  // 7C0
    arm7C0,arm_UI,arm7C2,arm_UI,arm7C4,arm_UI,arm7C6,arm_UI,  // 7C8
    arm7D0,arm_UI,arm7D2,arm_UI,arm7D4,arm_UI,arm7D6,arm_UI,  // 7D0
    arm7D0,arm_UI,arm7D2,arm_UI,arm7D4,arm_UI,arm7D6,arm_UI,  // 7D8
    arm7E0,arm_UI,arm7E2,arm_UI,arm7E4,arm_UI,arm7E6,arm_UI,  // 7E0
    arm7E0,arm_UI,arm7E2,arm_UI,arm7E4,arm_UI,arm7E6,arm_UI,  // 7E8
    arm7F0,arm_UI,arm7F2,arm_UI,arm7F4,arm_UI,arm7F6,arm_UI,  // 7F0
    arm7F0,arm_UI,arm7F2,arm_UI,arm7F4,arm_UI,arm7F6,arm_BP,  // 7F8

    REP16(arm800),REP16(arm810),REP16(arm820),REP16(arm830),  // 800
    REP16(arm840),REP16(arm850),REP16(arm860),REP16(arm870),  // 840
    REP16(arm880),REP16(arm890),REP16(arm8A0),REP16(arm8B0),  // 880
    REP16(arm8C0),REP16(arm8D0),REP16(arm8E0),REP16(arm8F0),  // 8C0
    REP16(arm900),REP16(arm910),REP16(arm920),REP16(arm930),  // 900
    REP16(arm940),REP16(arm950),REP16(arm960),REP16(arm970),  // 940
    REP16(arm980),REP16(arm990),REP16(arm9A0),REP16(arm9B0),  // 980
    REP16(arm9C0),REP16(arm9D0),REP16(arm9E0),REP16(arm9F0),  // 9C0

    REP256(armA00),                                           // A00
    REP256(armB00),                                           // B00
    REP256(arm_UI),                                           // C00
    REP256(arm_UI),                                           // D00

    arm_UI,armE01,arm_UI,armE01,arm_UI,armE01,arm_UI,armE01,  // E00
    arm_UI,armE01,arm_UI,armE01,arm_UI,armE01,arm_UI,armE01,  // E08
    arm_UI,armE01,arm_UI,armE01,arm_UI,armE01,arm_UI,armE01,  // E10
    arm_UI,armE01,arm_UI,armE01,arm_UI,armE01,arm_UI,armE01,  // E18
    REP16(arm_UI),                                            // E20
    REP16(arm_UI),                                            // E30
    REP16(arm_UI),REP16(arm_UI),REP16(arm_UI),REP16(arm_UI),  // E40
    REP16(arm_UI),REP16(arm_UI),REP16(arm_UI),REP16(arm_UI),  // E80
    REP16(arm_UI),REP16(arm_UI),REP16(arm_UI),REP16(arm_UI),  // EC0

    REP256(armF00),                                           // F00
};

// Wrapper routine (execution loop) ///////////////////////////////////////

#if 0
#include<time.h>
static void tester(void) {
  static int ran=0;if(ran)return;ran=1;
  FILE*f=fopen("p:\\timing.txt","w");if(!f)return;
  for (int op=/*0*/9; op</*0xF00*/10;op++){if(armInsnTable[op]==arm_UI)continue;
    int i;for(i=0;i<op;i++)if(armInsnTable[op]==armInsnTable[i])break;if(i<op)continue;
    for(i=0;i<16;i++)reg[i].I=0x3100000;
    clock_t s=clock();for(i=0;i<10000000;i++)armInsnTable[op](0);clock_t e=clock();
    fprintf(f,"arm%03X %6ld\n",op,e-s);fflush(f);
  }fclose(f);
}
#endif

int armExecute()
{
    do {
		if( cheatsEnabled ) {
			cpuMasterCodeCheck();
		}

        if ((armNextPC & 0x0803FFFF) == 0x08020000)
          busPrefetchCount = 0x100;

        u32 opcode = cpuPrefetch[0];
        cpuPrefetch[0] = cpuPrefetch[1];

        busPrefetch = false;
        if (busPrefetchCount & 0xFFFFFE00)
            busPrefetchCount = 0x100 | (busPrefetchCount & 0xFF);

        clockTicks = 0;
        int oldArmNextPC = armNextPC;

#ifndef FINAL_VERSION
        if (armNextPC == stop) {
            armNextPC++;
        }
#endif

        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH_NEXT;

        int cond = opcode >> 28;
        bool cond_res = true;
        if (UNLIKELY(cond != 0x0E)) {  // most opcodes are AL (always)
            switch(cond) {
              case 0x00: // EQ
                cond_res = Z_FLAG;
                break;
              case 0x01: // NE
                cond_res = !Z_FLAG;
                break;
              case 0x02: // CS
                cond_res = C_FLAG;
                break;
              case 0x03: // CC
                cond_res = !C_FLAG;
                break;
              case 0x04: // MI
                cond_res = N_FLAG;
                break;
              case 0x05: // PL
                cond_res = !N_FLAG;
                break;
              case 0x06: // VS
                cond_res = V_FLAG;
                break;
              case 0x07: // VC
                cond_res = !V_FLAG;
                break;
              case 0x08: // HI
                cond_res = C_FLAG && !Z_FLAG;
                break;
              case 0x09: // LS
                cond_res = !C_FLAG || Z_FLAG;
                break;
              case 0x0A: // GE
                cond_res = N_FLAG == V_FLAG;
                break;
              case 0x0B: // LT
                cond_res = N_FLAG != V_FLAG;
                break;
              case 0x0C: // GT
                cond_res = !Z_FLAG &&(N_FLAG == V_FLAG);
                break;
              case 0x0D: // LE
                cond_res = Z_FLAG || (N_FLAG != V_FLAG);
                break;
              case 0x0E: // AL (impossible, checked above)
                cond_res = true;
                break;
              case 0x0F:
              default:
                // ???
                cond_res = false;
                break;
            }
        }

        if (cond_res)
            (*armInsnTable[((opcode>>16)&0xFF0) | ((opcode>>4)&0x0F)])(opcode);
#ifdef INSN_COUNTER
        count(opcode, cond_res);
#endif
        if (clockTicks < 0)
            return 0;
        if (clockTicks == 0)
            clockTicks = 1 + codeTicksAccessSeq32(oldArmNextPC);
        cpuTotalTicks += clockTicks;

    } while (cpuTotalTicks<cpuNextEvent && armState && !holdState && !SWITicks);

    return 1;
}
