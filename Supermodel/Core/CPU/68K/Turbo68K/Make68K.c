/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * Turbo68K: Motorola 680X0 emulator
 * Copyright 2000-2002 Bart Trzynadlowski, see "README.TXT" for terms of use
 */

/*
 * Make68K: This program emits Turbo68K. Please see README.TXT for details.
 *
 * Note: If you get a link error involving pow(), remember to link in the math
 *       library. Try using "-lm".
 *
 * Note: I am not certain about dummy reads. The manual says they occur with
 *       some instructions on the MC68000 and MC68008 only. Does this include
 *       the MC68EC000? I'm assuming it does. Also, is a dummy read made for
 *       all Scc instructions, or just ST? I'm assuming all of them do it.
 *
 * Note: Do not use the profiling feature (#define PROFILE.) It will add
 *       useless junk to the emulator.
 *
 * Note: Future plans: I intend on removing as much of the Save/RestoreReg()
 *       junk from the API functions as possible. PUSH/POPs should work much
 *       better.
 *
 * Additional developers' notes pertaining to all aspects of Turbo68K can be
 * found at the very end of this file.
 */


#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "turbo68k.h"

#define VERSION "0.6"

#define SIZEOF_CONTEXT          "(context_end - context_start)"
#define SIZEOF_FETCHREGION      "12"
#define SIZEOF_DATAREGION       "16"
#define OFFSET_DATA_BASE        "0"
#define OFFSET_DATA_LIMIT       "4"
#define OFFSET_DATA_PTR         "8"
#define OFFSET_DATA_HANDLER     "12"
#define OFFSET_FETCH_BASE       "0"
#define OFFSET_FETCH_LIMIT      "4"
#define OFFSET_FETCH_PTR        "8"

#define WRITEPCTOMEM(dest)      {                               \
                                e("mov  dword ["dest"], esi\n");\
                                e("sub  dword ["dest"], ebp\n");\
                                }
#define WRITEPCTOREG(dest)      {                               \
                                e("mov  "dest", esi\n");        \
                                e("sub  "dest", ebp\n");        \
                                }
#define EMULATING()             e("or   byte [status], 1\n");
#define STOP_EMULATING()        e("and  byte [status], 0xfe\n");
#define STOP_CPU()              e("or   byte [status], 2\n");
#define UNSTOP_CPU()            e("and  byte [status], 0xfd\n");
#define INTERRUPT_PROCESSING()  e("or   byte [status], 4\n");
#define INTERRUPT_DONE()        e("and  byte [status], 0xfb\n");
#define STOP_RUNNING()          {                               \
                                e("mov [remaining], ecx\n");  \
                                WRITEPCTOMEM("pc");           \
                                }
#define A7                      "[a+7*4]"
#define SP                      "[__sp]"
#define SR                      "[sr]"

#define UNKNOWN_SIZE            0
#define BYTE_SIZE               1
#define WORD_SIZE               2
#define LONG_SIZE               3

unsigned short  decoded[65536];             /* compressed jump table data */
char            id[17] = { '\0' };          /* prepended to identifiers */
FILE            *fp;                        /* output file */
int             mpu = 68000;                /* 68000 by default */
unsigned        addr_bits = 24;             /* 24-bit by default */
unsigned        addr_mask = 0xffffff;
int             illegal = 1;                /* illegal instruction traps */
int             dummyread = 1;              /* dummy reads (68000 only) */
int             skip = 0;                   /* CPU (idle loop) skipping */
int             brafetch = 0;               /* branch fetch ptr updating */
int             pcfetch = 0;                /* special PC-relative fetching */
int             multiaddr = 1;              /* supervisor and user spaces */
int             memmap_type = 0;            /* memory map type */
int             call_convention = 0;        /* calling convention */
int             debug = 0;                  /* call debug function at every instruction */
int             mmx = 0;                    /* MMX support -- don't use! */
unsigned        num_handlers = 0;           /* # of inst. handlers emitted */

#ifdef PROFILE
char            *prof[512];
int             prof_i = 0;
#endif


/*****************************************************************************
* General Emitters                                                          */

void e(char *input, ...)
{
    va_list arg;
    char    text[256];
    va_start(arg, input);
    vsprintf(text, input, arg);
    va_end(arg);
    fprintf(fp, text);
}

void EmitLabel(char *label)
{
    e("%s:\n", label);
}

void Align(int i)
{
    e("times ($$-$) & %d nop\n", i - 1);
}

void CacheAlign()
{
    Align(32);
}

void EmitGlobalLabel(char *label)
{
    e("global %s%s, _%s%s, %s%s_\n", id, label, id, label, id, label);
    e("%s%s:\n_%s%s:\n%s%s_:\n", id, label, id, label, id, label);
}


/*****************************************************************************
* Utility Functions                                                         */

/*
 * SizeOfContext(): Returns the size of the context if needed by the emitter.
 * This should be used instead of C's sizeof().
 */

int SizeOfContext()
{
    switch (mpu)
    {
    case 68010:
        return sizeof(struct TURBO68K_CONTEXT_68010);
    }
    return sizeof(struct TURBO68K_CONTEXT_68000);
}

/*
 * NameOfContext(): Returns the name of the context based on the MPU type,
 * but NOT with the identifier in front of it.
 */

static char *NameOfContext()
{
    static char name[16 + 6 + 1];

    sprintf(name, "turbo68kcontext_%d", mpu);
    return name;
}

/*
 * NOTE: Only pass 32-bit or 8-bit (XL, not XH regs) to the SaveReg/SaveRegTo,
 * RestoreReg/RestoreRegTo functions. Don't ever pass 16-bit regs!
 */

/*
 * SaveReg/RestoreReg: Save/restore registers to/from a specific save slot
 * area.
 */

void SaveReg(char *area, char *reg)
{
    if (mmx)
    {
        if (!strcmp(area, "memhandler"))
        {   /* memhandler always saves full reg */
            if (!strcmp(reg, "eax"))    { e("movd   mm0, eax\n"); return; }
            if (!strcmp(reg, "ebx"))    { e("movd   mm1, ebx\n"); return; }
            if (!strcmp(reg, "ebp"))    { e("movd   mm2, ebp\n"); return; }
        }
        else if (!strcmp(area, "run"))
        {   /* we have to check if its a full or byte reg */
            if (!strcmp(reg, "edx") || !strcmp(reg, "dl"))
            {
                e("movd mm3, edx\n");
                return;
            }
            if (!strcmp(reg, "esi"))    { e("movd   mm4, esi\n"); return; }
            if (!strcmp(reg, "edi"))    { e("movd   mm5, edi\n"); return; }
        }          
    }        

    e("mov  [%s_", area);
    if (reg[1] == 'l')  /* ?l byte reg */
    {
        switch (reg[0])
        {
        case 'a':   e("eax"); break;
        case 'b':   e("ebx"); break;
        case 'c':   e("ecx"); break;
        case 'd':   e("edx"); break;
        }
    }
    else
        e("%s", reg);
    e("], %s\n", reg);
}

void RestoreReg(char *area, char *reg)
{
    if (mmx)
    {
        if (!strcmp(area, "memhandler"))
        {   /* memhandler always restores full reg */
            if (!strcmp(reg, "eax"))    { e("movd   eax, mm0\n"); return; }
            if (!strcmp(reg, "ebx"))    { e("movd   ebx, mm1\n"); return; }
            if (!strcmp(reg, "ebp"))    { e("movd   ebp, mm2\n"); return; }
        }
        else if (!strcmp(area, "run"))
        {   /* we have to check if its a full or byte reg */
            if (!strcmp(reg, "edx") || !strcmp(reg, "dl"))
            {
                e("movd edx, mm3\n");
                return;
            }
            if (!strcmp(reg, "esi"))    { e("movd   esi, mm4\n"); return; }
            if (!strcmp(reg, "edi"))    { e("movd   edi, mm5\n"); return; }
        }          
    }

    e("mov  %s, [%s_", reg, area);
    if (reg[1] == 'l')  /* ?l byte reg */
    {
        switch (reg[0])
        {
        case 'a':   e("eax"); break;
        case 'b':   e("ebx"); break;
        case 'c':   e("ecx"); break;
        case 'd':   e("edx"); break;
        }
    }
    else
        e("%s", reg);
    e("]\n");
}

/*
 * SaveRegTo/RestoreRegTo: Saves/restore a register to a different register
 * than indicated by the name of the save slot.
 */

void SaveRegTo(char *area, char *d_reg, char *s_reg)
{    
    if (mmx)
    {
        /* currently, memhandlers don't need this, only run_ does. */
        if (!strcmp(area, "run"))
        {
            /* test to make sure dest is MMX-able */
            if (!strcmp(d_reg, "edx") || !strcmp(d_reg, "esi") || !strcmp(d_reg, "edi"))
            {
                e("movd ");
                if (!strcmp(d_reg, "edx"))      e("mm3, ");
                else if (!strcmp(d_reg, "esi")) e("mm4, ");
                else if (!strcmp(d_reg, "edi")) e("mm5, ");
    
                if (s_reg[1] == 'l')    /* ?l byte reg */
                {
                    switch (s_reg[0])
                    {
                    case 'a':   e("eax\n"); break;
                    case 'b':   e("ebx\n"); break;
                    case 'c':   e("ecx\n"); break;
                    case 'd':   e("edx\n"); break;
                    }
                }
                else
                    e("%s\n", s_reg);
                return;
            }
        }
    }
    e("mov  [%s_%s], %s\n", area, d_reg, s_reg);
}

void RestoreRegTo(char *area, char *s_reg, char *d_reg)
{
    if (mmx)
    {
        /* currently, memhandlers don't need this, only run_ does. */
        if (!strcmp(area, "run"))
        {
            /* test to make sure src is MMX-able */
            if (!strcmp(s_reg, "edx") || !strcmp(s_reg, "esi") || !strcmp(s_reg, "edi"))
            {
                e("movd ");
                if (d_reg[1] == 'l')    /* ?l byte reg */
                {
                    switch (d_reg[0])
                    {
                    case 'a':   e("eax, "); break;
                    case 'b':   e("ebx, "); break;
                    case 'c':   e("ecx, "); break;
                    case 'd':   e("edx, "); break;
                    }
                }
                else
                    e("%s, ", d_reg);

                if (!strcmp(s_reg, "edx"))      e("mm3\n");
                else if (!strcmp(s_reg, "esi")) e("mm4\n");
                else if (!strcmp(s_reg, "edi")) e("mm5\n");
                return;
            }
        }
    }
    e("mov  %s, [%s_%s]\n", d_reg, area, s_reg);
}

void AddrClip(char *reg)
{   /* if it's 32-bit we don't need this */
    if (addr_mask != 0xffffffff)    e("and  %s, 0x%08X\n", reg, addr_mask);
}

void RideIntoTheDangerZone()    /* where EBP is modified and must be saved */
{
    e("push ebp\n");
}

void RideOutOfTheDangerZone()
{
    e("pop  ebp\n");
}

void GetArg(char *dest_reg, int num_arg, int stack_offs)
{
    char    *reg[2] = { "eax", "edx" }; /* currently, only 2 args supported */

    if (num_arg > 1)    /* error! */
    {
        fprintf(stderr, "Make68K: Internal Error: GetArg() called with num_arg = %d. Please contact Bart Trzynadlowski!\n", num_arg);
        if (call_convention)
            return;
    }

    if (!call_convention)
        e("mov  %s, [esp+%d]\n", dest_reg, stack_offs);
    else
    {
        if ((!strcmp(dest_reg, "eax") && !num_arg) || (!strcmp(dest_reg, "edx") && num_arg == 1))
            ;               /* EAX,EAX or EDX,EDX */
        else
            e("mov  %s, %s\n", dest_reg, reg[num_arg]);
    }
}

void GetArgFromTheDangerZone(char *dest_reg, int num)
{
    char    *reg[2] = { "eax", "edx" }; /* no more than 2 args usually */

    if (!call_convention)   /* stack-based */
        e("mov  %s, [esp+8+(%d*4)]\n", dest_reg, num);
    else                    /* register-based */
    {
        if ((!strcmp(dest_reg, "eax") && !num) || (!strcmp(dest_reg, "edx") && num == 1))
            ;               /* EAX,EAX or EDX,EDX */
        else
            e("mov  %s, %s\n", dest_reg, reg[num]);
    }
}

void GetArgNoStringsAttached(char *dest_reg, int num)
{
    char    *reg[2] = { "eax", "edx" }; /* no more than 2 args usually */

    if (!call_convention)
        e("mov  %s, [esp+4+(%d*4)]\n", dest_reg, num);
    else
    {
        if ((!strcmp(dest_reg, "eax") && !num) || (!strcmp(dest_reg, "edx") && num == 1))
            ;               /* EAX,EAX or EDX,EDX */
        else
            e("mov  %s, %s\n", dest_reg, reg[num]);
    }
}

void ReadLong()
{
    if (memmap_type == 0 || memmap_type == 1)   e("call ReadLong\n");
    else if (memmap_type == 2)                  e("call dword [read_long]\n");
}

void ReadByte()
{
    if (memmap_type == 0 || memmap_type == 1)   e("call ReadByte\n");
    else if (memmap_type == 2)                  e("call dword [read_byte]\n");
}

void ReadWord()
{
    if (memmap_type == 0 || memmap_type == 1)   e("call ReadWord\n");
    else if (memmap_type == 2)                  e("call dword [read_word]\n");
}

void WriteLong()
{
    if (memmap_type == 0 || memmap_type == 1)   e("call WriteLong\n");
    else if (memmap_type == 2)                  e("call dword [write_long]\n");
}

void WriteByte()
{
    if (memmap_type == 0 || memmap_type == 1)   e("call WriteByte\n");
    else if (memmap_type == 2)                  e("call dword [write_byte]\n");
}

void WriteWord()
{
    if (memmap_type == 0 || memmap_type == 1)   e("call WriteWord\n");
    else if (memmap_type == 2)                  e("call dword [write_word]\n");
}

void ReadByteSX()
{
    fprintf(stderr, "Make68K: Internal Error: Sign-extended byte read handler required. Please contact Bart Trzynadlowski!\n");
    exit(1);
}

void ReadWordSX()
{
    if (memmap_type == 0 || memmap_type == 1)   e("call ReadWordSX\n");
    else if (memmap_type == 2)
    {
        e("call     dword [read_word]\n");
        e("movsx    edx, dx\n");
    }
}

/*
 * LoadFromEA() and MOVEM() use the ReadXXXPC() functions.
 */

void ReadLongPC()
{
    e("call ReadLongPC\n");
}

void ReadBytePC()
{
    e("call ReadBytePC\n");
}

void ReadWordPC()
{
    e("call ReadWordPC\n");
}

void ReadByteSXPC()
{
    fprintf(stderr, "Make68K: Internal Error: Sign-extended byte read handler required. Please contact Bart Trzynadlowski!\n");
    exit(1);

    e("call     ReadBytePC\n");
    e("movsx    edx, dl\n");
}

void ReadWordSXPC()
{
    e("call     ReadWordPC\n");
    e("movsx    edx, dx\n");
}

void UpdateFetchPtr()   /* scan the fetch area */
{
    int     inst = 0;
    /*
     * In:  ESI = address
     * Out: ESI = PC + ptr
     *      EBP = ptr (base)
     *      EDX = trashed
     */

    /*
     * Note: If address bus is NOT 32-bits, the following applies.
     *
     * The unused bits of the PC MUST be preserved. Therefore, those unused
     * bits are subtracted from the base pointer so that when we add a PC
     * WITH unused bits, it won't go out of range, because it will be far
     * below range to begin with.
     *
     * The reasoning here is that if we subtract the unused bits from the base
     * pointer, the unused bits in the normalized PC pointer (ESI) will cancel
     * out.
     */
    if (addr_mask != 0xffffffff)
        SaveReg("fetch", "esi");
    AddrClip("esi");
    e("mov     edx, [fetch]\n");
    e(".find_fetch_loop%d:\n", inst);
    e("add     edx, byte "SIZEOF_FETCHREGION"\n");
    e("cmp     dword [edx], byte -1\n");        /* limit=-1? end. */
    e("je      near fetch_error\n");
    e("cmp     esi, [edx]\n");                  /* offset 0: base. above it? */
    e("jb      .find_fetch_loop%d\n", inst);    /* nope, not this region */
    e("cmp     esi, [edx+"OFFSET_FETCH_LIMIT"]\n"); /* limit. below it? */
    e("ja      .find_fetch_loop%d\n", inst);        /* nope, not this region */
    e("mov     ebp, [edx+"OFFSET_FETCH_PTR"]\n");   /* ebp=base ptr */
    if (addr_mask != 0xffffffff)    /* to handle those nasty unused bits! */
    {
        e("mov  edx, [fetch_esi]\n");
        e("mov  esi, edx\n");   /* this is the ACTUAL PC we saved earlier */
        e("and  edx, 0x%08X\n", ~addr_mask);/* we only want the unused bits */
        e("sub  ebp, edx\n");
    }
    e("add     esi, ebp\n");                        /* +pc=pc ptr.. */
}

void LoadCCR()  /* CCR->AX ... trashes AX and DX */
{
    e("mov      dh, [sr]\n");       /* DH:***XNZVC  */
    e("mov      ah, dh\n");         /* AH:***XNZVC  */
    e("shl      ah, byte 4\n");     /* AH:NZVC0000  */
    e("test     dh, byte 2\n");     /* V flag?      */
    e("setnz    al\n");             /* AL:0000000V  */
    e("test     dh, 0x10\n");       /* X flag?      */
    e("setnz    byte [x]\n");       /* X:0000000000000000000000000000000X */
    e("and      dh, byte 1\n");     /* DH:0000000C  */
    e("or       ah, dh\n");         /* AH:NZ**000C  */
}

void SaveCCR()  /* AX->CCR... trashes AX and DX */
{
    e("and      eax, 0xc101\n");    /* EAX:0000000000000000NZ00000C0000000V */
    e("mov      dh, ah\n");         /*  DH:NZ00000C */
    e("shr      dh, byte 4\n");     /*  DH:0000NZ00 */
    e("shl      al, byte 1\n");     /*  AL:000000V0 */
    e("or       dh, al\n");         /*  DH:0000NZV0 */
    e("shr      ah, byte 1\n");     /*  CF:C        */
    e("adc      dh, 0\n");          /*  DH:0000NZVC */
    e("mov      ah, [x]\n");        /*  AH:0000000X */
    e("shl      ah, byte 4\n");     /*  AH:000X0000 */
    e("or       dh, ah\n");         /*  DH:000XNZVC */
    e("mov      [sr], dh\n");         
}

void InstBegin(unsigned short op, char *mnem, unsigned num)
{
#ifdef PROFILE
    int i;
#endif

    Align(4);
    if (num == 1)
        e("I%04X: ; %s\t\t0x%04X\n", op, mnem, op);
    else
        e("I%04X: ; %s\t\t0x%04X-0x%04X\n", op, mnem, op, op+(num-1));
	if (debug)
	{
		// Debugging code 
		char noDebugLabel[16];
		char noPCChgLabel[16];
		sprintf(noDebugLabel, ".no_debug%04X", op);
		sprintf(noPCChgLabel, ".no_pcchg%04X", op);

		e("mov  ebx, [Debug]\n");            /* Get debug handler */
		e("test ebx, ebx\n");                /* If no handler set, skip debugging */
		e("jz   short %s\n", noDebugLabel);

		e("push eax\n");					 /* Note: __cdecl calling convention used to call handler */
		e("push edi\n");
		e("push ebp\n");
		WRITEPCTOMEM("pc");                  /* Save esi */
		e("mov  [remaining], ecx\n");
		e("push edi\n");                     /* Pass instruction opcode to handler */
		e("sub  esi, ebp\n");                /* Convert PC to base offset */
		e("sub  esi, byte 2\n");             /* Adjust back to beginning of instruction */
		e("push esi\n");				     /* Pass result to handler */
		e("call ebx\n");                     /* Call handler */
		e("add  esp, byte 8\n");
		e("mov  esi, [pc]\n");               /* Restore esi (this could have been changed) */
		e("mov  ecx, [remaining]\n");        /* This could have been changed */
		e("pop  ebp\n");
		e("pop  edi\n");
		e("add  esi, ebp\n");				 /* Convert esi to pointer (base+pc) */
		e("test eax, eax\n");			     /* Check if debug handler changed PC */
		e("jz   short %s\n", noPCChgLabel); 
		e("pop  eax\n");                     /* If so, move onto instruction at new PC */
		e("mov  di, [esi]\n");
		e("add  esi, byte 2\n");             /* Point to word after next opcode */
		e("jmp  dword [jmptab+edi*4]\n");

		EmitLabel(noPCChgLabel);
		e("pop  eax\n");

		EmitLabel(noDebugLabel);
	}
    decoded[op] = num;
    num_handlers++;

#ifdef PROFILE
    for (i = 0; i < strlen(mnem); i++)  /* remove spaces */
    {
        if (mnem[i] == ' ')
            mnem[i] = '_';
    }    
    e("inc  dword [prof%s]\n", mnem);
    for (i = 0; prof[i] != NULL && i < 512; i++)
    {
        if (!strcmp(prof[i], mnem)) /* already have one of these... */
            return;
    }
    prof[prof_i] = calloc(strlen(mnem) + 1, 1);
    strcpy(prof[prof_i], mnem);
    prof_i++;
#endif
}

void InstEnd()
{
    e("mov  di, [esi]\n");
    e("add  esi, byte 2\n");        /* point to word after next opcode */
    e("jmp  dword [jmptab+edi*4]\n");
}


/*****************************************************************************
* Address Space Management                                                  */

/*
 * Input:   Nothing
 * Notes:   Assumes that run_ecx, run_edi, and run_esi are available for use.
 */

void SetSupervisorAddressSpace()
{
    if (!multiaddr) return;
    e("push ecx\n");
    e("cld\n");                     /* direction: forward */
    e("push esi\n");
    e("push edi\n");
    e("mov  ecx, 8\n");             /* 8 dword-size pointers to copy */
    e("mov  edi, fetch\n");
    e("mov  esi, super_fetch\n");
    e("rep  movsd\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  ecx\n");
}

void SetUserAddressSpace()
{
    if (!multiaddr) return;
    e("push ecx\n");
    e("cld\n");                     /* direction: forward */
    e("push esi\n");
    e("push edi\n");
    e("mov  ecx, 8\n");             /* 8 dword-size pointers to copy */
    e("mov  edi, fetch\n");
    e("mov  esi, user_fetch\n");
    e("rep  movsd\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  ecx\n");
}

    
/*****************************************************************************
* Timing                                                                    */

/* TimingEA: calculates amount of clocks taken by a given EA mode */
unsigned TimingEA(unsigned ea, int size) 
{       /* EA format = MMMRRR (mode/register) */
    switch ((ea >> 3) & 7)  /* mode */
    {
    case 0: case 1: return 0; break;
    case 2: case 3:
        switch (size) { case BYTE_SIZE: case WORD_SIZE: return 4; break;
                        case LONG_SIZE: return 8; break; } break;
    case 4:
        switch (size) { case BYTE_SIZE: case WORD_SIZE: return 6; break;
                        case LONG_SIZE: return 10; break; } break;
    case 5:
        switch (size) { case BYTE_SIZE: case WORD_SIZE: return 8; break;
                        case LONG_SIZE: return 12; break; } break;
    case 6:
        switch (size) { case BYTE_SIZE: case WORD_SIZE: return 10; break;
                        case LONG_SIZE: return 14; break; } break;
    case 7: /* misc... */
        switch (ea & 0x7) { case 0: /* (xxx).W */
                                switch (size)
                                { case BYTE_SIZE: case WORD_SIZE: return 8; break;
                                  case LONG_SIZE: return 12; break; } break;
                            case 1: /* (xxx).L */
                                switch (size)
                                { case BYTE_SIZE: case WORD_SIZE: return 12; break;
                                  case LONG_SIZE: return 16; break; } break;
                            case 4: /* #<data> */
                                switch (size)
                                { case BYTE_SIZE: case WORD_SIZE: return 4; break;
                                  case LONG_SIZE: return 8; break; } break;
                            case 2: /* (d16,pc) */
                                switch (size)
                                { case BYTE_SIZE: case WORD_SIZE: return 8; break;
                                  case LONG_SIZE: return 12; break; } break;
                            case 3: /* (d8,pc,Xn) */
                            switch (size)
                            { case BYTE_SIZE: case WORD_SIZE: return 10; break;
                              case LONG_SIZE: return 14; break; } break;
                          }
    } return 0;
}

void EmitTiming(unsigned cycles)
{
    if (cycles)
    {
        if (cycles == 1)            e("dec  ecx\n");
        else if (!(cycles & 0x80))  e("sub  ecx, byte %d\n", cycles);
        else                        e("sub  ecx, %d\n", cycles);
        e("js   near Turbo68KRun_done\n"); /* below 0, finished */
    }
}


/*****************************************************************************
* Effective Address Code                                                    */

int CheckEA(unsigned ea, char *valid)   /* EA = MMMRRR (mode/reg) */
{
    /*
     * valid[] string is a mask for valid addressing modes as they appear
     * in the 68K manual, in order, 1 = okay, 0 = not allowed
     */
    
    if (((ea >> 3) & 7) != 7)   /* mode 7 is handled separately */
    {
        if (valid[(ea >> 3) & 7] != '1')    return 0;   /* not allowed! */
        else                                return 1;
    }
    else                        /* handle mode 7 here */
    {
        /* first check if the register is invalid */
        if ((ea & 7) >= 5)  return 0;                   /* no such thing! */
        else
        {
            switch (ea & 7)
            {
            case 0:
            case 1:
                if (valid[7+(ea & 7)] != '1')       return 0;
                else                    return 1;   break;
            case 4:
                if (valid[7+2] != '1')  return 0;
                else                    return 1;   break;
            case 2:
                if (valid[7+3] != '1')  return 0;
                else                    return 1;   break;
            case 3:
                if (valid[7+4] != '1')  return 0;
                else                    return 1;   break;
            }
        }
    }   return 0;
}

int NumDecodedEA(unsigned ea)   /* how many handlers can this feed? */
{
    /*
     * Turbo68K does not generate an instruction handler for every possible
     * opcode, instead, it generates one for every possible EA mode of a given
     * instruction. This means if reg==0, it returns the # to pass to InstEnd
     * (which indicates how many opcodes this will cover for) Otherwise, this
     * function returns 0 and the instruction handler should not be emitted.
     */

    if (((ea >> 3) & 7) <= 6 && (ea & 7) != 0)      return 0;
    else if (((ea >> 3) & 7) <= 6 && (ea & 7) == 0) return 8;   /* 8 regs */
    else if (((ea >> 3) & 7) == 7 && (ea & 7) <= 4) return 1;   /* 1 reg */
    return 0;
}

/*
 * Effective Address calculations
 *
 * Note: When loading words, usually the whole dword is loaded. When storing,
 * only the word part is stored
 */
   
/*
 * Returns: Data in EDX, address preserved in EBX
 * Used:    EBX, EDX, EDI (unless reading from register)
 * Notes:   ESI is assumed to point at word after opcode
 *          Assumes EDI contains opcode and that it has EA at the very end
 *          This function is only to be used when EA mode fields are located
 *          at bits 0-5. 
 */

void LoadFromEA(unsigned ea, int size, int sign_extend)
{ 
    switch ((ea >> 3) & 7)
    {
    case 0: /* Dn */
    case 1: /* An */
    {
        char    r = ((ea >> 3) & 7) ? 'a' : 'd';
        e("and  edi, byte 7\n");    /* get reg number, EDI contains opcode */
        if (size == LONG_SIZE)
            e("mov  edx, [%c+edi*4]\n", r);
        else
        {
            if (size == BYTE_SIZE)
            {
                if (!sign_extend)
                    e("mov  dl, [%c+edi*4]\n", r);
                else
                    e("movsx    edx, byte [%c+edi*4]\n", r);
            }
            else    /* word size */
            {
                if (!sign_extend)
                    e("mov  edx, [%c+edi*4]\n", r);
                else
                    e("movsx    edx, word [%c+edi*4]\n", r);
            }
        }        
    }
    break;
    case 2: /* (An) */
    case 3: /* (An)+ */
    case 4: /* -(An) */
    case 5: /* (d16,An) */
        e("and  edi, byte 7\n");    /* reg */
        if (((ea >> 3) & 7) == 4)   /* pre-dec */
        {
            if (size == BYTE_SIZE)
            {
                e("cmp  edi, byte 7\n");    /* A7 bytes */
                e("cmc\n");                 /* switch carry */
                e("sbb  dword [a+edi*4], byte 1\n");                
            }
            else if (size == WORD_SIZE)
                e("sub  dword [a+edi*4], byte 2\n");
            else if (size == LONG_SIZE)
                e("sub  dword [a+edi*4], byte 4\n");
        }
        if (((ea >> 3) & 7) == 5)   /* (d16,An) */
        {
            e("movsx    ebx, word [esi]\n");
            e("add      ebx, [a+edi*4]\n");
            e("add      esi, byte 2\n");    /* past d16, next instruction */
        }
        else
            e("mov  ebx, [a+edi*4]\n");
        if (((ea >> 3) & 7) == 3)   /* post-inc */
        {
            if (size == BYTE_SIZE)
            {
                e("cmp  edi, byte 7\n");    /* A7 bytes */
                e("cmc\n");                 /* switch carry */
                e("adc  dword [a+edi*4], byte 1\n");
            }
            else if (size == WORD_SIZE)
                e("add  dword [a+edi*4], byte 2\n");
            else if (size == LONG_SIZE)
                e("add  dword [a+edi*4], byte 4\n");
        }
        switch (size)
        {
        case BYTE_SIZE:
            if (sign_extend)
                ReadByteSX();
            else
                ReadByte();
            break;
        case WORD_SIZE:
            if (sign_extend)
                ReadWordSX();
            else
                ReadWord();
            break;
        case LONG_SIZE:
            ReadLong();
            break;
        }
        break;
    case 6: /* (d8,An,Xn) */
        e("and      edi, byte 7\n");    /* An reg # */
        e("xor      ebx, ebx\n");       /* so the high part is clear */
        e("mov      bx, [esi]\n");      /* fetch extension word */
        e("movsx    edx, bl\n");        /* sign-extended index */
        e("add      esi, byte 2\n");    /* point at next word */
        e("shr      ebx, byte 12\n");   /* regs must be: d, a for this to work */
                                        /* the shr sets CF if Long reg */
        e("mov      ebx, [d+ebx*4]\n");
        e("jc       short .ll\n");      /* CF=1? index reg is LONG */
        e("movsx    ebx, bx\n");        /* otherwise: sign-extended WORD */
        EmitLabel(".ll");
        e("add      ebx, edx\n");       /* disp+index_reg */
        e("add      ebx, [a+edi*4]\n");
        switch (size)
        {
        case BYTE_SIZE:
            if (sign_extend)
                ReadByteSX();
            else
                ReadByte();
            break;
        case WORD_SIZE:
            if (sign_extend)
                ReadWordSX();
            else
                ReadWord();
            break;
        case LONG_SIZE:
            ReadLong();
            break;
        }
        break;
    case 0x7:
        switch (ea & 7)
        {
        case 0: /* (xxx).W */
        case 1: /* (xxx).L */
            if ((ea & 7) == 0)  /* word */
            {
                e("movsx    ebx, word [esi]\n");
                e("add      esi, byte 2\n");
            }
            else                /* long */
            {
                e("mov      ebx, [esi]\n");               
                e("ror      ebx, byte 16\n");    /* word-swap */
                e("add      esi, byte 4\n");
            }
            switch (size)
            {
            case BYTE_SIZE:
                if (sign_extend)
                    ReadByteSX();
                else
                    ReadByte();
                break;
            case WORD_SIZE:
                if (sign_extend)
                    ReadWordSX();
                else
                    ReadWord();
                break;
            case LONG_SIZE:
                ReadLong();
                break;
            }
            break;
        case 4: /* #<data> */
            switch (size)
            {
            case BYTE_SIZE:
                if (!sign_extend)
                    e("mov  dl, [esi]\n");  /* get low byte of ext. word */
                else
                    e("movsx    edx, byte [esi]\n");
                e("add  esi, byte 2\n");
                break;
            case WORD_SIZE:
                if (!sign_extend)
                    e("mov  edx, [esi]\n"); /* bx = word, ebx=????:word */
                else
                    e("movsx    edx, word [esi]\n");
                e("add  esi, byte 2\n");
                break;
            case LONG_SIZE:
                e("mov  edx, [esi]\n");     /* get long-word */
                e("add  esi, byte 4\n");
                e("ror  edx, byte 16\n");   /* word swap to work... */
                break;
            }
        break;
        case 2: /* (d16,PC) */            
            /*
             * The PC is assumed to be pointing at the extension word, which
             * is exactly how the emulator works
             */
            e("movsx    ebx, word [esi]\n");
            e("add      ebx, esi\n");
            e("sub      ebx, ebp\n");
            e("add      esi, byte 2\n");
            switch (size)
            {
            case BYTE_SIZE:
                if (sign_extend)
                    pcfetch ? ReadByteSXPC() : ReadByteSX();
                else
                    pcfetch ? ReadBytePC() : ReadByte();
                break;
            case WORD_SIZE:
                if (sign_extend)
                    pcfetch ? ReadWordSXPC() : ReadWordSX();
                else
                    pcfetch ? ReadWordPC() : ReadWord();
                break;
            case LONG_SIZE:
                pcfetch ? ReadLongPC() : ReadLong();
                break;
            }
            break;
        case 3: /* (d8,PC,Xn) */
            e("xor      ebx, ebx\n");       /* so the high part is clear */
            e("mov      bx, [esi]\n");      /* fetch extension word */
            e("movsx    edx, bl\n");        /* sign-extended index */
            e("shr      ebx, byte 12\n");   /* regs must be: d, a for this to work */
                                            /* the shr sets CF if Long reg */
            e("mov      ebx, [d+ebx*4]\n");
            e("jc       short .ll\n");      /* CF=1? index reg is LONG */
            e("movsx    ebx, bx\n");        /* otherwise: sign-extended WORD */
            EmitLabel(".ll");
            e("add      ebx, edx\n");       /* disp+index_reg */
            e("add      ebx, esi\n");       /* add the PC */
            e("sub      ebx, ebp\n");
            e("add      esi, byte 2\n");    /* point at next word */
            switch (size)
            {
            case BYTE_SIZE:
                if (sign_extend)
                    pcfetch ? ReadByteSXPC() : ReadByteSX();
                else
                    pcfetch ? ReadBytePC() : ReadByte();
                break;
            case WORD_SIZE:
                if (sign_extend)
                    pcfetch ? ReadWordSXPC() : ReadWordSX();
                else
                    pcfetch ? ReadWordPC() : ReadWord();
                break;
            case LONG_SIZE:
                pcfetch ? ReadLongPC() : ReadLong();
                break;
            }
            break;
        }
        break;
    default:
        printf("Error!\a\n");
        break;
    }
}

/*
 * Expects: Data in EDX, address in EBX
 * Notes:   Destroys EBX, make sure to preserve the address if you need it.
 */

void StoreToEA(unsigned ea, int size, int same)
{   /* the "same" parameter is ignored. it no longer is needed */
    switch ((ea >> 3) & 7)
    {
    case 0: /* Dn */
    case 1: /* An */
    {
        char    r = ((ea >> 3) & 7) ? 'a' : 'd';
        if (size == LONG_SIZE)
            e("mov  [%c+%d*4], edx\n", r, ea & 7);
        else if (size == BYTE_SIZE)
            e("mov  [%c+%d*4], dl\n", r, ea & 7);
        else if (size == WORD_SIZE)
            e("mov  [%c+%d*4], dx\n", r, ea & 7);
    }
    break;
    case 2: /* (An) */
    case 3: /* (An)+ */
    case 4: /* -(An) */
    case 5: /* (d16,An) */
        if (((ea >> 3) & 7) == 4)   /* pre-dec */
        {
            if (size == BYTE_SIZE)
            {
                if ((ea & 7) == 7)  /* A7 kept word aligned */
                    e("sub  dword [a+%d*4], byte 2\n", ea & 7);
                else
                    e("dec  dword [a+%d*4]\n", ea & 7);
            }
            else if (size == WORD_SIZE)
                e("sub  dword [a+%d*4], byte 2\n", ea & 7);
            else if (size == LONG_SIZE)
                e("sub  dword [a+%d*4], byte 4\n", ea & 7);
        }
        if (((ea >> 3) & 7) == 5)   /* (d16,An) */
        {
            e("movsx    ebx, word [esi]\n");
            e("add      ebx, [a+%d*4]\n", ea & 7);
            e("add      esi, byte 2\n");    /* past d16, next instruction */
        }
        else
                e("mov  ebx, [a+%d*4]\n", ea & 7);
        if (((ea >> 3) & 7) == 3)   /* post-inc */
        {
            if (size == BYTE_SIZE)
            {
                if ((ea & 7) == 7)  /* A7 kept word aligned */
                    e("add  dword [a+%d*4], byte 2\n", ea & 7);
                else
                    e("inc  dword [a+%d*4]\n", ea & 7);
            }
            else if (size == WORD_SIZE)
                e("add  dword [a+%d*4], byte 2\n", ea & 7);
            else if (size == LONG_SIZE)
                e("add  dword [a+%d*4], byte 4\n", ea & 7);
        }
        switch (size)
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break;  }
        break;
    case 6: /* (d8,An,Xn) */
        e("xor      ebx, ebx\n");       /* so the high part is clear */
        e("mov      bx, [esi]\n");      /* fetch extension word */
        e("movsx    edi, bl\n");        /* sign-extended index */
        e("add      esi, byte 2\n");    /* point at next word */
        e("shr      ebx, byte 12\n");   /* regs must be: d, a for this to work */
                                        /* the shr sets CF if Long reg */
        e("mov      ebx, [d+ebx*4]\n");
        e("jc       short .ls\n");      /* CF=1? index reg is LONG */
        e("movsx    ebx, bx\n");        /* otherwise: sign-extended WORD */
        EmitLabel(".ls");
        e("add      ebx, edi\n");       /* disp+index_reg */
        e("add      ebx, [a+%d*4]\n", ea & 7);
        switch (size)   /* WriteXXX() clears EDI */
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break;  }
        break;
    case 0x7:
        switch (ea & 7)
        {
        case 0: /* (xxx).W */
        case 1: /* (xxx).L */
            if ((ea & 7) == 0)  /* word */
            {
                e("movsx    ebx, word [esi]\n");
                e("add      esi, byte 2\n");
            }
            else                /* long */
            {
                e("mov      ebx, [esi]\n");               
                e("ror      ebx, byte 16\n");   /* word-swap */
                e("add      esi, byte 4\n");
            }
            switch (size)
            {   case BYTE_SIZE: WriteByte(); break;
                case WORD_SIZE: WriteWord(); break;
                case LONG_SIZE: WriteLong(); break;  }
            break;
        case 4: /* #<data> */
            /* can't store to #data! */
            printf("Error!\a\n");
            break;
        case 2: /* (d16,PC) */            
            /*
             * The PC is assumed to be pointing at the extension word, which
             * is exactly how the emulator works
             */
            e("movsx    ebx, word [esi]\n");
            e("add      ebx, esi\n");
            e("sub      ebx, ebp\n");
            e("add      esi, byte 2\n");
            switch (size)
            {   case BYTE_SIZE: WriteByte(); break;
                case WORD_SIZE: WriteWord(); break;
                case LONG_SIZE: WriteLong(); break;  }
            break;
        case 3: /* (d8,PC,Xn) */
            e("xor      ebx, ebx\n");       /* so the high part is clear */
            e("mov      bx, [esi]\n");      /* fetch extension word */
            e("movsx    edi, bl\n");        /* sign-extended index */
            e("shr      ebx, byte 12\n");   /* regs must be: d, a for this to work */
                                            /* the shr sets CF if Long reg */
            e("mov      ebx, [d+ebx*4]\n");
            e("jc       short .ls\n");      /* CF=1? index reg is LONG */
            e("movsx    ebx, bx\n");        /* otherwise: sign-extended WORD */
            EmitLabel(".ls");
            e("add      ebx, edi\n");       /* disp+index_reg */
            e("add      ebx, esi\n");       /* add the PC */
            e("sub      ebx, ebp\n");       
            e("add      esi, byte 2\n");    /* point at next word */
            switch (size)   /* WriteXXX() clears EDI */
            {   case BYTE_SIZE: WriteByte(); break;
                case WORD_SIZE: WriteWord(); break;
                case LONG_SIZE: WriteLong(); break;  }       
            break;
        }
        break;
    default:
        break;
    }
}

/*
 * This is the same as StoreToEA() except that it uses EDI&7 to find the
 * register. It is assumed that EA is bits 0-5 of the opcode. This only works
 * in special situations like CLR and Scc.
 */

void StoreToEAUsingEDI(unsigned ea, int size, int same)
{   /* the "same" parameter is ignored. it no longer is needed */
    switch ((ea >> 3) & 7)
    {
    case 0: /* Dn */
    case 1: /* An */
    {
        char    r = ((ea >> 3) & 7) ? 'a' : 'd';
        e("and  edi, byte 7\n");
        if (size == LONG_SIZE)
            e("mov  [%c+edi*4], edx\n", r);
        else if (size == BYTE_SIZE)
            e("mov  [%c+edi*4], dl\n", r);
        else if (size == WORD_SIZE)
            e("mov  [%c+edi*4], dx\n", r);
    }
    break;
    case 2: /* (An) */
    case 3: /* (An)+ */
    case 4: /* -(An) */
    case 5: /* (d16,An) */
        e("and  edi, byte 7\n");
        if (((ea >> 3) & 7) == 4)   /* pre-dec */
        {          
            if (size == BYTE_SIZE)
            {
                e("cmp  edi, byte 7\n");    /* A7 bytes */
                e("cmc\n");                 /* switch carry */
                e("sbb  dword [a+edi*4], byte 1\n");                
            }
            else if (size == WORD_SIZE)
                e("sub  dword [a+edi*4], byte 2\n");
            else if (size == LONG_SIZE)
                e("sub  dword [a+edi*4], byte 4\n");
        }
        if (((ea >> 3) & 7) == 5)   /* (d16,An) */
        {
            e("movsx    ebx, word [esi]\n");
            e("add      ebx, [a+edi*4]\n");
            e("add      esi, byte 2\n");    /* past d16, next instruction */
        }
        else
                e("mov  ebx, [a+edi*4]\n");
        if (((ea >> 3) & 7) == 3)   /* post-inc */
        {
            if (size == BYTE_SIZE)
            {
                e("cmp  edi, byte 7\n");    /* A7 bytes */
                e("cmc\n");                 /* switch carry */
                e("adc  dword [a+edi*4], byte 1\n");
            }
            else if (size == WORD_SIZE)
                e("add  dword [a+edi*4], byte 2\n");
            else if (size == LONG_SIZE)
                e("add  dword [a+edi*4], byte 4\n");
        }
        switch (size)
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break;  }
        break;
    case 6: /* (d8,An,Xn) */
        e("and      edi, byte 7\n");    /* An reg # */
        SaveReg("memhandler", "edi");   /* no memhandlers using this now... */
        e("xor      ebx, ebx\n");       /* so the high part is clear */
        e("mov      bx, [esi]\n");      /* fetch extension word */
        e("movsx    edi, bl\n");        /* sign-extended index */
        e("add      esi, byte 2\n");    /* point at next word */
        e("shr      ebx, byte 12\n");   /* regs must be: d, a for this to work */
                                        /* the shr sets CF if Long reg */
        e("mov      ebx, [d+ebx*4]\n");
        e("jc       short .ll\n");      /* CF=1? index reg is LONG */
        e("movsx    ebx, bx\n");        /* otherwise: sign-extended WORD */
        EmitLabel(".ll");
        e("add      ebx, edi\n");       /* disp+index_reg */
        RestoreReg("memhandler", "edi");
        e("add      ebx, [a+edi*4]\n");
        switch (size)   /* WriteXXX() clears EDI */
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break;  }
        break;
    case 0x7:
        switch (ea & 7)
        {
        case 0: /* (xxx).W */
        case 1: /* (xxx).L */
            if ((ea & 7) == 0)  /* word */
            {
                e("movsx    ebx, word [esi]\n");
                e("add      esi, byte 2\n");
            }
            else                /* long */
            {
                e("mov      ebx, [esi]\n");               
                e("ror      ebx, byte 16\n");   /* word-swap */
                e("add      esi, byte 4\n");
            }
            switch (size)
            {   case BYTE_SIZE: WriteByte(); break;
                case WORD_SIZE: WriteWord(); break;
                case LONG_SIZE: WriteLong(); break;  }
            break;
        case 4: /* #<data> */
            /* can't store to #data! */
            break;
        case 2: /* (d16,PC) */            
            /*
             * The PC is assumed to be pointing at the extension word, which
             * is exactly how the emulator works
             */
            e("movsx    ebx, word [esi]\n");
            e("add      ebx, esi\n");
            e("sub      ebx, ebp\n");
            e("add      esi, byte 2\n");
            switch (size)
            {   case BYTE_SIZE: WriteByte(); break;
                case WORD_SIZE: WriteWord(); break;
                case LONG_SIZE: WriteLong(); break;  }
            break;
        case 3: /* (d8,PC,Xn) */
            e("xor      ebx, ebx\n");       /* so the high part is clear */
            e("mov      bx, [esi]\n");      /* fetch extension word */
            e("movsx    edi, bl\n");        /* sign-extended index */
            e("shr      ebx, byte 12\n");   /* regs must be: d, a for this to work */
                                            /* the shr sets CF if Long reg */
            e("mov      ebx, [d+ebx*4]\n");
            e("jc       short .ls\n");      /* CF=1? index reg is LONG */
            e("movsx    ebx, bx\n");        /* otherwise: sign-extended WORD */
            EmitLabel(".ls");
            e("add      ebx, edi\n");       /* disp+index_reg */
            e("add      ebx, esi\n");       /* add the PC */
            e("sub      ebx, ebp\n");       
            e("add      esi, byte 2\n");    /* point at next word */
            switch (size)   /* WriteXXX() clears EDI */
            {   case BYTE_SIZE: WriteByte(); break;
                case WORD_SIZE: WriteWord(); break;
                case LONG_SIZE: WriteLong(); break;  }       
            break;
        }
        break;
    default:
        printf("Error!\a\n");
        break;
    }
}


/*
 * Returns: Address in EBX
 * Used:    EBX, EDX
 * Notes:   ESI is assumed to point at word after opcode
 *          Assumes EDI contains opcode and that it has EA at the very end
 *          This function is only to be used when EA mode fields are located
 *          at bits 0-5. 
 */

void LoadControlEA(unsigned ea)
{ 
    switch ((ea >> 3) & 7)
    {
    case 2: /* (An) */
    case 3: /* (An)+ (just load what's in (An) -- MOVEM needs this) */
    case 4: /* -(An) (just load what's in (An) -- MOVEM needs this) */
    case 5: /* (d16,An) */
        e("and  edi, byte 7\n");    /* reg */
        if (((ea >> 3) & 7) == 5)   /* (d16,An) */
        {
            e("movsx    ebx, word [esi]\n");
            e("add      ebx, [a+edi*4]\n");
            e("add      esi, byte 2\n");    /* past d16, next instruction */
        }
        else
            e("mov  ebx, [a+edi*4]\n");
        break;
    case 6: /* (d8,An,Xn) */
        e("and      edi, byte 7\n");    /* edi=An # */
        e("xor      ebx, ebx\n");       /* so the high part is clear */
        e("mov      bx, [esi]\n");      /* fetch extension word */
        e("movsx    edx, bl\n");        /* sign-extended index */
        e("add      esi, byte 2\n");    /* point at next word */
        e("shr      ebx, byte 12\n");   /* regs must be: d, a for this to work */
                                        /* the shr sets CF if Long reg */
        e("mov      ebx, [d+ebx*4]\n");
        e("jc       short .ll\n");      /* CF=1? index reg is LONG */
        e("movsx    ebx, bx\n");        /* otherwise: sign-extended WORD */
        EmitLabel(".ll");
        e("add      ebx, edx\n");       /* disp+index_reg */
        e("add      ebx, [a+edi*4]\n");
        break;
    case 0x7:
        switch (ea & 7)
        {
        case 0: /* (xxx).W */
        case 1: /* (xxx).L */
            if ((ea & 7) == 0)  /* word */
            {
                e("movsx    ebx, word [esi]\n");
                e("add      esi, byte 2\n");
            }
            else                /* long */
            {
                e("mov      ebx, [esi]\n");
                e("ror      ebx, byte 16\n");   /* word-swap */
                e("add      esi, byte 4\n");
            }
            break;
        case 2: /* (d16,PC) */
            /*
             * The PC is assumed to be pointing at the extension word, which
             * is exactly how the emulator works
             */
            e("movsx    ebx, word [esi]\n");
            e("add      ebx, esi\n");
            e("sub      ebx, ebp\n");
            e("add      esi, byte 2\n");
            break;
        case 3: /* (d8,PC,Xn) */
            e("xor      ebx, ebx\n");       /* so the high part is clear */
            e("mov      bx, [esi]\n");      /* fetch extension word */
            e("movsx    edx, bl\n");        /* sign-extended index */
            e("shr      ebx, byte 12\n");   /* regs must be: d, a for this to work */
                                            /* the shr sets CF if Long reg */
            e("mov      ebx, [d+ebx*4]\n");
            e("jc       short .ll\n");      /* CF=1? index reg is LONG */
            e("movsx    ebx, bx\n");        /* otherwise: sign-extended WORD */
            EmitLabel(".ll");
            e("add      ebx, edx\n");       /* disp+index_reg */
            e("add      ebx, esi\n");       /* add the PC */
            e("sub      ebx, ebp\n");
            e("add      esi, byte 2\n");    /* point at next word */
            break;            
        }
        break;
    default:
        printf("Warning: LoadControlEA() defaulted\n");
        break;
    }
}


/*****************************************************************************
* Instruction Handlers                                                      */

int RTD(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("mov  ebx, [a+7*4]\n");
    ReadLong();
    e("xor  ebx, ebx\n");
    e("add  dword [a+7*4], byte 4\n");
    e("mov  bx, [esi]\n");
    e("add  dword [a+7*4], ebx\n"); /* SP+4+disp */
    e("mov  esi, edx\n");
    UpdateFetchPtr();
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}
    
int MOVEC(unsigned short op, char *mnem, unsigned base_timing)
{
    /*
     * Timing for an illegal control register is not handled, control is
     * simply passed to the illegal instruction exception handler, which uses
     * different timing.
     */

    InstBegin(op, mnem, 2);
    e("test byte [sr+1], 0x20\n");      /* in supervisor? */
    e("jz   near exception_privilege_violation\n");

    e("xor  ebx, ebx\n");
    e("xor  edx, edx\n");
    e("mov  dx, [esi]\n");               /* fetch data word */
    e("mov  ebx, edx\n");
    e("shr  ebx, byte 12\n");           /* EBX=general register */

    e("shr  edi, byte 1\n");            /* dr field */
    e("jc   short .to_control\n");

    /*
     * Test for control register. The control register value is placed in
     * EDX. At the end, EDX is moved to [d+ebx*4].
     */

    e("and  edx, 0xfff\n");
    e("jnz  short .from_dfc\n");
    e("mov  dl, [fc]\n");   /* SFC */
    e("jmp  short .end_from\n");

    EmitLabel(".from_dfc");
    e("cmp  edx, byte 1\n");
    e("jne  short .from_usp\n");
    e("mov  dl, [fc+1]\n"); /* DFC */
    e("jmp  short .end_from\n");

    EmitLabel(".from_usp");
    e("cmp  edx, 0x800\n");
    e("jne  short .from_vbr\n");
    e("mov  edx, [__sp]\n");    /* we're in supervisor, USP is in __sp */
    e("jmp  short .end_from\n");

    EmitLabel(".from_vbr");
    e("cmp  edx, 0x801\n");
    e("jne  near exception_illegal_instruction\n"); /* invalid control reg */
    e("mov  edx, [vbr]\n"); /* VBR */

    EmitLabel(".end_from");
    e("mov  [d+ebx*4], edx\n"); /* move to general reg */
    e("add  esi, byte 2\n");    /* remember, we fetched a word */
    EmitTiming(12);
    e("jmp  short .end\n");
                     
    /*
     * Test for control register. General reg is written to control reg.
     */

    EmitLabel(".to_control");
    e("mov  ebx, [d+ebx*4]\n"); /* general reg */

    e("and  edx, 0xfff\n");
    e("jnz  short .to_dfc\n");
    e("and  bl, 3\n");
    e("mov  [fc], bl\n");   /* SFC */
    e("jmp  short .end_to\n");

    EmitLabel(".to_dfc");
    e("cmp  edx, byte 1\n");
    e("jne  short .to_usp\n");
    e("and  bl, 3\n");
    e("mov  [fc+1], bl\n"); /* DFC */
    e("jmp  short .end_to\n");

    EmitLabel(".to_usp");
    e("cmp  edx, 0x800\n");
    e("jne  short .to_vbr\n");
    e("mov  [__sp], ebx\n");    /* we're in supervisor, USP is in __sp */
    e("jmp  short .end_to\n");

    EmitLabel(".to_vbr");
    e("cmp  edx, 0x801\n");
    e("jne  near exception_illegal_instruction\n"); /* invalid control reg */
    e("mov  [vbr], ebx\n"); /* VBR */

    EmitLabel(".end_to");
    e("add  esi, byte 2\n");    /* remember, we fetched a word */
    EmitTiming(10);

    /*
     * End
     */

    EmitLabel(".end");
    InstEnd();
    return 2;
}

int BKPT(unsigned short op, char *mnem, unsigned base_timing)
{
    /*
     * NOTE: Timing isn't done here, the exception processing code will do it.
     * The exception code doesn't use the proper timing for a BKPT, but it's
     * good enough ;)
     */

    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");    /* vector # */
    e("mov  ebx, [Bkpt]\n");    /* BKPT handler */
    e("test ebx, ebx\n");       /* if no handler, don't do anything */
    e("jz   short .no_handler\n");

    e("push eax\n");
    e("push edi\n");
    e("push ebp\n");
    WRITEPCTOMEM("pc");                 /* saves esi */
    e("mov     [remaining], ecx\n");
    e("push    edi\n");                 /* vector # */
    e("call    ebx\n");
    e("add     esp, byte 4\n");
    e("mov     esi, [pc]\n");
    e("mov     ecx, [remaining]\n");    /* this could have been changed */
    e("pop  ebp\n");
    e("pop  edi\n");
    e("pop  eax\n");
    e("add     esi, ebp\n");            /* base+pc=pc pointer */

    EmitLabel(".no_handler");
    e("jmp  near exception_illegal_instruction\n");
    return 8;
}

int MOVEfromCCR(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, t = 4;

    if (!CheckEA(ea, "101111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));
    e("xor  edx, edx\n");                       /* CCR->(sign extend)->word */
    e("mov  dl, [sr]\n");
    StoreToEA(ea, WORD_SIZE, 0);
    if ((ea >> 3) != 0) t += 4;                 /* mem */
    EmitTiming(t + TimingEA(ea, WORD_SIZE));    /* technically, word sized */
    InstEnd();
    return 1;
}

int TRAPV(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);

    e("test al, al\n"); /* do only if V */
    e("jz   near .no_trap\n");

    e("mov  edx, 0x2000\n");            /* what the SR will be */
    e("xor  edx, [sr]\n");              /* see if S bit is different */
    e("test edx, 0x2000\n");
    e("jz   .no_sp_magic\n");           /* nope, don't swap SPs */
    e("mov  edx, [__sp]\n");
    e("xchg [a+7*4], edx\n");
    e("mov  [__sp], edx\n");
    SetSupervisorAddressSpace();
    EmitLabel(".no_sp_magic");

    e("mov  ebx, [a+7*4]\n");           /* SP */

    /*
     * If 68010, push format info
     */

    if (mpu == 68010)
    {
        e("sub  ebx, byte 2\n");/* SP-2 */
        e("mov  edx, 7*4\n");   /* TRAPV vector | 0x00000000 */
        e("push ebx\n");
        WriteWord();
        e("pop  ebx\n");
    }

    e("mov  edx, esi\n");
    e("sub  edx, ebp\n");               /* PC->EDX */
    e("sub  ebx, byte 4\n");            /* SP-4 */
    e("push ebx\n");
    WriteLong();
    e("pop  ebx\n");
    e("mov  edx, [sr]\n");              /* get SR */
    e("sub  ebx, byte 2\n");            /* SP-2 */
    e("push ebx\n");
    WriteWord();                        /* save SR to stack */
    e("pop  ebx\n");
    e("or   dh, 0x20\n");               /* set supervisor for exception */
    e("and  edx, 0xa71f\n");            /* clear unwanted bits */
    e("xchg [sr], edx\n");              /* set new SR, get old SR->EDX */
    e("mov  [a+7*4], ebx\n");           /* write back SP */
    e("mov  ebx, 7*4\n");               /* TRAPV vector */
    if (mpu == 68010)
        e("add  ebx, [vbr]\n");
    ReadLong();                         /* get PC */
    e("mov  esi, edx\n");               /* set new PC */
    UpdateFetchPtr();
    EmitTiming(34+4);                   /* TRAPV=4, exception=34 */
    InstEnd();
    EmitLabel(".no_trap");
    e("xor  edi, edi\n");
    EmitTiming(4);                  
    InstEnd();

    return 1;
}
    
int STOP(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("test byte [sr+1], 0x20\n");      /* in supervisor? */
    e("jz   near exception_privilege_violation\n");
    e("mov  edx, [esi]\n");
    e("add  esi, byte 2\n");
    e("and  edx, 0xa71f\n");            /* mask out unimplemented bits */
    e("mov  [sr], dx\n");
    LoadCCR();
    STOP_CPU();
    EmitTiming(base_timing);
    e("mov  dword [remaining], 0\n");   /* stop executing */
    e("jmp  near Turbo68KRun_done\n");
    return 1;
}

int RESET(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("test byte [sr+1], 0x20\n");      /* in supervisor? */
    e("jz   near exception_privilege_violation\n");

    e("mov  ebx, [Reset]\n");   /* Reset handler */
    e("test ebx, ebx\n");       /* if no handler, don't do anything */
    e("jz   short .no_reset\n");

    e("push eax\n");
    e("push ebp\n");
    WRITEPCTOMEM("pc");                 /* saves esi */
    e("mov     [remaining], ecx\n");
    e("call    ebx\n");
    e("mov     esi, [pc]\n");
    e("mov     ecx, [remaining]\n");    /* this could have been changed */
    e("pop  ebp\n");
    e("pop  eax\n");
    e("add     esi, ebp\n");            /* base+pc=pc pointer */
    e("xor     edi, edi\n");            /* keep clear for fetch */

    EmitLabel(".no_reset");
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int TAS(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f;

    if (!CheckEA(ea, "101111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, BYTE_SIZE, 0);
    e("test dl, dl\n");     /* test... */
    e("lahf\n");
    e("xor  al, al\n");
    e("or   dl, 0x80\n");   /* ...and set bit 7 */
    if ((ea >> 3) == 0)
    {        
        e("mov  [d+edi*4], dl\n");
        EmitTiming(4);
    }
    else
    {

/*
 * NOTE:
 * -----
 *
 * You may wish to uncomment the WriteByte() below when emulating the Sega
 * Genesis if you wish to get Gargoyles running. The TAS instruction is used
 * to synchronize multiple 68Ks by grabbing the bus, but on the Genesis, the
 * VDP will not release the bus resulting in data not being written.
 */

        WriteByte();
        EmitTiming(14 + TimingEA(ea, BYTE_SIZE));
    }
    InstEnd();
    return 1;
}    

int NBCD(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, t;

    /*
     * Notes About Undefined Flags:
     * ----------------------------
     *
     * N flag = Set the same as the MSB of the result
     * V flag = If MSB changes from 1 to 0 when result is adjusted for BCD,
     *          the V flag is set, otherwise it is cleared.
     */

    if (!CheckEA(ea, "101111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));
    if ((ea >> 3) == 0) /* Dn */
    {
        e("and  edi, byte 7\n");

        e("xor  al, al\n");         /* we use AL so we can use DAS */

        e("shr  byte [x], 1\n");    /* X->CF */
        e("sbb  al, [d+edi*4]\n");
        e("mov  bh, al\n");         /* save unadjusted result in BH */
        e("das\n");                 /* BCD! */
        e("mov  bl, ah\n");         /* store old flags */
        e("lahf\n");
        e("setc byte [x]\n");
        e("jnz  short .clr\n");     /* Z clear */
        e("and  bl, byte 0x40\n");  /* isolate old Z */
        e("and  ah, byte 0x3f\n");  /* kill new Z */
        e("or   ah, bl\n");         /* in w/ old Z */
        EmitLabel(".clr");
        e("test al, al\n");         /* N flag */
        e("sets dh\n");
        e("ror  dh, 1\n");          /* into MSB of DH */
        e("or   ah, dh\n");         /* -> N flag */
        e("mov  [d+edi*4], al\n");

        /*
         * V flag calculation: Put unadjusted result's MSB into bit 1, and the
         * adjusted result's MSB into bit 0. If the result is 2, the V flag
         * is set.
         */

        e("shr  bh, byte 6\n");
        e("shr  al, byte 7\n");
        e("and  bh, 2\n");
        e("or   bh, al\n");
        e("xor  al, al\n");
        e("cmp  bh, 2\n");
        e("jne  short .no_v\n");
        e("mov  al, 1\n");  /* move 1 into V flag */
        EmitLabel(".no_v");

        t = 6;
    }
    else                /* memory */
    {
        LoadFromEA(ea, BYTE_SIZE, 0);

        e("xor  al, al\n");
        e("shr  byte [x], 1\n");    /* X->CF */
        e("sbb  al, dl\n");
        e("mov  dh, al\n");         /* DH now has unadjusted result */
        e("mov  dl, ah\n");
        e("das\n");                 /* BCD! */
        e("lahf\n");      
        e("setc byte [x]\n");
        e("jnz  short .clr\n");
        e("and  dl, byte 0x40\n");
        e("and  ah, byte 0x3f\n");
        e("or   ah, dl\n");
        EmitLabel(".clr");

        e("test al, al\n");         /* N flag */
        e("sets dl\n");
        e("ror  dl, 1\n");          /* into MSB of DL */
        e("or   ah, dl\n");         /* -> N flag */

        e("mov  dl, al\n");         /* move result into DL to write it */

        /*
         * V flag calculation: Put unadjusted result's MSB into bit 1, and the
         * adjusted result's MSB into bit 0. If the result is 2, the V flag
         * is set.
         */

        e("shr  dh, byte 6\n");
        e("shr  al, byte 7\n");
        e("and  dh, 2\n");
        e("or   dh, al\n");
        e("xor  al, al\n");
        e("cmp  dh, 2\n");
        e("jne  short .no_v\n");
        e("mov  al, 1\n");  /* move 1 into V flag */
        EmitLabel(".no_v");

        /* WriteXXX will clear EDI for us */
        WriteByte();
        t = 8 + TimingEA(ea, BYTE_SIZE);
    }
    EmitTiming(t);
    InstEnd();
    return 1;
}

int CHK(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    reg = (op >> 9) & 7, size, ea = op & 0x3f;

    /*
     * Notes About Undefined Flags:
     * ----------------------------
     *
     * Z flag = Set if register operand is 0 (second operand), cleared
     *          otherwise
     * V flag = Always cleared (?)
     * C flag = Always cleared (?)
     */

    if (!CheckEA(ea, "101111111111"))   return 0;
    if (!NumDecodedEA(ea))              return 0;
    switch ((op >> 7) & 3)
    {
    case 3: size = WORD_SIZE; break;
    default:    return 0;
    }

    InstBegin(op, mnem, NumDecodedEA(ea));
    /*
     * We have to emit the EA timing first because it must be accounted for
     * whether or not a trap occurs. DIVU and DIVS work like this too.
     */
    if (TimingEA(ea, size))
        e("sub     ecx, byte %d\n", TimingEA(ea, size));
    LoadFromEA(ea, size, 0);
    e("xor  al, al\n");                     /* V should be cleared */
    e("test word [d+%d*4], 0xffff\n", reg); /* If Dn == 0 Then Z=1 */
    e("setz ah\n");                         /* this also clears N and C */
    e("shl  ah, 6\n");                      /* put Z in proper position */
    e("test byte [d+%d*4+1], 0x80\n", reg); /* If Dn < 0 Then TRAP */
    e("jnz  near .trap_dn\n");
    e("cmp  [d+%d*4], dx\n", reg);          /* If Dn > Source Then TRAP */
    e("jg   near .trap_ea\n");
    EmitTiming(base_timing);
    InstEnd();
    EmitLabel(".trap_dn");  /* set N to 1 because Dn<0 test failed */
    e("or   ah, 0x80\n");
    e("jmp  short .trap\n");
    EmitLabel(".trap_ea");  /* clear N because Dn>Source test failed */
    e("and  ah, 0x7f\n");
    EmitLabel(".trap");
    e("jmp  near exception_chk\n");

    return 1;
}

int MOVEP(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    dx = (op >> 9) & 7, opmode = (op >> 6) & 7, t;

    if (opmode != 4 && opmode != 5 && opmode != 6 && opmode != 7)   return 0;

    InstBegin(op, mnem, 8);
    e("and      edi, byte 7\n");
    e("movsx    ebx, word [esi]\n");    /* (d16,An) */
    e("add      ebx, [a+edi*4]\n");
    e("add      esi, byte 2\n");
    switch (opmode & 2)
    {
    case 2: /* reg->mem */
        if (opmode & 1) /* long */
        {
            t = 24;
            e("push eax\n");        /* we will be using EAX to save EBX */
            e("mov      edx, [d+%d*4]\n", dx);
            e("mov      eax, ebx\n");   /* save EBX */
            e("rol      edx, byte 8\n");/* write high order */
            WriteByte();                
            e("add      eax, byte 2\n");
            e("rol      edx, byte 8\n");/* write mid-upper */
            e("mov      ebx, eax\n");
            WriteByte();
            e("add      eax, byte 2\n");
            e("rol      edx, byte 8\n");/* write mid-lower */
            e("mov      ebx, eax\n");
            WriteByte();
            e("add      eax, byte 2\n");
            e("rol      edx, byte 8\n");/* write lower order */
            e("mov      ebx, eax\n");
            WriteByte();
            e("pop  eax\n");
        }
        else            /* word */
        {
            t = 16;
            e("push eax\n");    /* we will be using EAX to save EBX */
            e("mov      dx, [d+%d*4]\n", dx);
            e("mov      eax, ebx\n");   /* save EBX */
            e("ror      edx, byte 8\n");/* write high order */
            WriteByte();                
            e("add      eax, byte 2\n");
            e("rol      edx, byte 8\n");/* write low order */
            e("mov      ebx, eax\n");
            WriteByte();
            e("pop  eax\n");
        }
        break;
    case 0: /* mem->reg */
        if (opmode & 1) /* long */
        {
            t = 24;
            ReadByte(); /* read high order */
            e("mov  [d+%d*4+3], dl\n", dx);     /* store high order in Dn */
            e("add  ebx, byte 2\n");
            ReadByte(); /* mid-upper order */
            e("mov  [d+%d*4+2], dl\n", dx);     /* store mid-upper order */
            e("add  ebx, byte 2\n");
            ReadByte(); /* mid-lower order */
            e("mov  [d+%d*4+1], dl\n", dx);     /* store mid-lower order */
            e("add  ebx, byte 2\n");
            ReadByte(); /* low order */
            e("mov  [d+%d*4+0], dl\n", dx);     /* store low order */
        }
        else            /* word */
        {
            t = 16;
            ReadByte(); /* read high order */
            e("mov  [d+%d*4+1], dl\n", dx);     /* store high order in Dn */
            e("add  ebx, byte 2\n");
            ReadByte(); /* read low order */
            e("mov  [d+%d*4+0], dl\n", dx);     /* store low order */
        }
        break;
    }
    EmitTiming(t);
    InstEnd();
    return 1;
}

int CMPM(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    size, ax = (op >> 9) & 7;

    switch ((op >> 6) & 3)
    {
    case 0: size = BYTE_SIZE; break;
    case 1: size = WORD_SIZE; break;
    case 2: size = LONG_SIZE; break;
    default:    return 0;
    }

    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");

    e("mov  ebx, [a+edi*4]\n");   /* get Ay */
    if (size == BYTE_SIZE)  /* Ay */
    {
        e("cmp  edi, byte 7\n");    /* A7 bytes */
        e("cmc\n");                 /* switch carry */
        e("adc  dword [a+edi*4], byte 1\n");
    }
    else if (size == WORD_SIZE)
        e("add  dword [a+edi*4], byte 2\n");
    else if (size == LONG_SIZE)
        e("add  dword [a+edi*4], byte 4\n");

    switch (size)
    {   case BYTE_SIZE: ReadByte(); break;
        case WORD_SIZE: ReadWord(); break;
        case LONG_SIZE: ReadLong(); break;  }
    SaveReg("run", "edx");      /* save Ay */

    e("mov  ebx, [a+%d*4]\n", ax);/* get Ax */
    if (size == BYTE_SIZE)  /* Ax */
    {
        if (ax == 7)
            e("add  dword [a+7*4], byte 2\n");
        else
            e("inc  dword [a+%d*4]\n", ax);
    }
    else if (size == WORD_SIZE)
        e("add  dword [a+%d*4], byte 2\n", ax);
    else    /* LONG */
        e("add  dword [a+%d*4], byte 4\n", ax);

    switch (size)
    {   case BYTE_SIZE: ReadByte(); break;
        case WORD_SIZE: ReadWord(); break;
        case LONG_SIZE: ReadLong(); break;  }
    switch (size)                   /* compare */
    {
    case BYTE_SIZE:
        RestoreRegTo("run", "edx", "bl");   /* [run_edx]->bl */
        e("cmp  dl, bl\n");
        break;
    case WORD_SIZE:
        RestoreRegTo("run", "edx", "ebx");  /* [run_edx]->ebx */
        e("cmp  dx, bx\n");
        break;
    case LONG_SIZE:
        RestoreRegTo("run", "edx", "ebx");  /* [run_edx]->ebx */
        e("cmp  edx, ebx\n");
        break;
    }
    e("lahf\n");
    e("seto al\n");

    if (size != LONG_SIZE)  /* byte, word */
        EmitTiming(12);
    else                    /* long */
        EmitTiming(20);
    InstEnd();
    return 1;
}

int TRAP(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 16);

    e("and  edi, byte 0xf\n");          /* lower 4 bits=vector */
    SaveReg("run", "edi");

    e("mov  edx, 0x2000\n");            /* what the SR will be */
    e("xor  edx, [sr]\n");              /* see if S bit is different */
    e("test edx, 0x2000\n");
    e("jz   .no_sp_magic\n");           /* nope, don't swap SPs */
    e("mov  edx, [__sp]\n");
    e("xchg [a+7*4], edx\n");
    e("mov  [__sp], edx\n");
    SetSupervisorAddressSpace();
    EmitLabel(".no_sp_magic");

    e("mov  ebx, [a+7*4]\n");           /* SP */

    /*
     * If 68010, push format info
     */

    if (mpu == 68010)
    {
        e("sub  ebx, byte 2\n");/* SP-2 */
        RestoreReg("run", "edi");   /* vector */
        e("mov  edx, edi\n");
        SaveReg("run", "edi");
        e("add  edx, byte 32\n");
        e("shl  edx, byte 2\n");    /* TRAP vector | 0x00000000 */
        SaveReg("run", "ebx");
        WriteWord();
        RestoreReg("run", "ebx");
    }        

    e("mov  edx, esi\n");
    e("sub  edx, ebp\n");               /* PC->EDX */
    e("sub  ebx, byte 4\n");            /* SP-4 */
    e("push ebx\n");
    WriteLong();
    e("pop  ebx\n");
    e("mov  edx, [sr]\n");              /* get SR */
    e("sub  ebx, byte 2\n");            /* SP-2 */
    e("push ebx\n");
    SaveReg("run", "ebx");
    WriteWord();                        /* save SR to stack */
    e("pop  ebx\n");
    e("or   dh, 0x20\n");               /* set supervisor for exception */
    e("and  edx, 0xa71f\n");            /* clear unwanted bits */
    e("xchg [sr], edx\n");              /* set new SR, get old SR->EDX */
    e("mov  [a+7*4], ebx\n");           /* write back SP */
    RestoreReg("run", "edi");
    e("mov  ebx, edi\n");
    e("add  ebx, byte 32\n");           /* vector+32=trap vector */
    e("shl  ebx, 2\n");
    if (mpu == 68010)
        e("add  ebx, [vbr]\n");

    ReadLong();                         /* get PC */
    e("mov  esi, edx\n");               /* set new PC */
    UpdateFetchPtr();
    e("xor  edi, edi\n");
    EmitTiming(base_timing);
    InstEnd();

    return 1;
}

int EORItoSR(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("test byte [sr+1], 0x20\n");      /* in supervisor? */
    e("jz   near exception_privilege_violation\n");
    SaveCCR();  /* save flags to SR */
    e("mov  edx, [esi]\n");
    e("and  edx, 0xa71f\n");            /* mask out unwanted bits */
    e("xor  [sr], dx\n");
    e("add  esi, byte 2\n");
    LoadCCR();  /* get flags back */
    e("test byte [sr+1], 0x20\n");      /* if we changed to User, swap SPs */
    e("jnz  short .in_s\n");
    e("mov  ebx, [a+7*4]\n");
    e("mov  edx, [__sp]\n");
    e("mov  [a+7*4], edx\n");
    e("mov  [__sp], ebx\n");
    SetUserAddressSpace();              /* map in user address space */
    EmitLabel(".in_s");    
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int EORItoCCR(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    SaveCCR();
    e("mov  dl, [esi]\n");
    e("and  dl, 0x1f\n");
    e("xor  [sr], dl\n");
    LoadCCR();
    e("add  esi, byte 2\n");
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int RTR(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("mov  ebx, [a+7*4]\n");             /* (SP)->CCR */
    ReadWord();
    e("and  dl, 0x1f\n");
    e("mov  [sr], dl\n");
    e("add  dword [a+7*4], byte 2\n");    /* SP+2->SP */
    LoadCCR();
    e("mov  ebx, [a+7*4]\n");             /* (SP)->PC */
    ReadLong();
    e("add  dword [a+7*4], byte 4\n");    /* SP+4->SP */
    e("mov  esi, edx\n");
    UpdateFetchPtr();
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int SUBX(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    rx = (op >> 9) & 7, rm = (op >> 3) & 1, size;

    switch ((op >> 6) & 3)
    {   case 0: size = BYTE_SIZE; break;
        case 1: size = WORD_SIZE; break;
        case 2: size = LONG_SIZE; break;
        default: return 0;  }

    InstBegin(op, mnem, 8);

    e("and  edi, byte 7\n");
    if (!rm)    /* Dy,Dx */
    {
        if (size == BYTE_SIZE)          /* EBX,BX,BL=destination */
            e("mov  bl, [d+%d*4]\n", rx);
        else
            e("mov  ebx, [d+%d*4]\n", rx);
        if (size == BYTE_SIZE)          /* EDX,DX,DL=source */
            e("mov  dl, [d+edi*4]\n");
        else
            e("mov  edx, [d+edi*4]\n");
        e("shr  byte [x], 1\n");      /* X->CF */
        switch (size)
        {   case BYTE_SIZE: e("sbb  bl, dl\n"); break;
            case WORD_SIZE: e("sbb  bx, dx\n"); break;
            case LONG_SIZE: e("sbb  ebx, edx\n"); break;
        }
        e("mov  dl, ah\n"); /* keep temporary copy of old flags in DL */
        e("lahf\n");
        e("setc byte [x]\n");
        e("seto al\n");
        e("jnz  short .z\n");           /* if non-zero, cleared */
        e("and  dl, 0x40\n");           /* otherwise, unchanged */
        e("and  ah, 0xbf\n");           /* (get rid of new unwanted Z) */
        e("or   ah, dl\n");             /* OR in the old, unchanged Z flag */
        EmitLabel(".z");        

        switch (size)   /* store results */
        {   case BYTE_SIZE: e("mov  [d+%d*4], bl\n", rx); break;
            case WORD_SIZE: e("mov  [d+%d*4], bx\n", rx); break;
            case LONG_SIZE: e("mov  [d+%d*4], ebx\n", rx); break;
        }
    }
    else        /* -(Ay),-(Ax) */
    {
        if (size == BYTE_SIZE)
        {
            e("cmp  edi, byte 7\n");    /* decrement Ay register (-2 for A7) */
            e("cmc\n");
            e("sbb  dword [a+edi*4], byte 1\n");
            if (rx != 7)
                e("dec  dword [a+%d*4]\n", rx);
            else
                e("sub  dword [a+7*4], byte 2\n");
        }
        else if (size == WORD_SIZE)
        {
            e("sub  dword [a+edi*4], byte 2\n");
            e("sub  dword [a+%d*4], byte 2\n", rx);
        }
        else if (size == LONG_SIZE)
        {
            e("sub  dword [a+edi*4], byte 4\n");
            e("sub  dword [a+%d*4], byte 4\n", rx);
        }

        e("mov  ebx, [a+edi*4]\n");   /* first, load up -(Ay) */
        switch (size)
        {   case BYTE_SIZE: ReadByte(); break;
            case WORD_SIZE: ReadWord(); break;
            case LONG_SIZE: ReadLong(); break;
        }
        SaveReg("run", "edx");          /* save data obtained */
        e("mov  ebx, [a+%d*4]\n", rx);/* next, load up -(Ax) */
        switch (size)
        {   case BYTE_SIZE: ReadByte(); break;
            case WORD_SIZE: ReadWord(); break;
            case LONG_SIZE: ReadLong(); break;
        }       
        SaveReg("run", "ebx");
        e("mov  ebx, edx\n");       /* EBX,BX,BL=destination */
        RestoreReg("run", "edx");   /* EDX,DX,DL=source */

        e("shr  byte [x], 1\n");      /* X->CF */
        switch (size)
        {   case BYTE_SIZE: e("sbb  bl, dl\n"); break;
            case WORD_SIZE: e("sbb  bx, dx\n"); break;
            case LONG_SIZE: e("sbb  ebx, edx\n"); break;
        }
        e("mov  dl, ah\n"); /* keep temporary copy of old flags in DL */
        e("lahf\n");
        e("setc byte [x]\n");
        e("seto al\n");
        e("jnz  short .z\n");           /* if non-zero, cleared */
        e("and  dl, 0x40\n");           /* otherwise, unchanged */
        e("and  ah, 0xbf\n");           /* (get rid of new unwanted Z) */
        e("or   ah, dl\n");             /* OR in the old, unchanged Z flag */
        EmitLabel(".z");       

        e("mov  edx, ebx\n");   /* store result */
        RestoreReg("run", "ebx");
        switch (size)
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break;
        }
    }
        
    if (!rm)
    {
        if (size == LONG_SIZE)
            EmitTiming(8);
        else
            EmitTiming(4);
    }
    else
    {
        if (size == LONG_SIZE)
            EmitTiming(30);
        else
            EmitTiming(18);
    }

    InstEnd();
    return 1;  
}

int ADDX(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    rx = (op >> 9) & 7, rm = (op >> 3) & 1, size;

    switch ((op >> 6) & 3)
    {   case 0: size = BYTE_SIZE; break;
        case 1: size = WORD_SIZE; break;
        case 2: size = LONG_SIZE; break;
        default: return 0;  }

    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");
    if (!rm)    /* Dy,Dx */
    {
        if (size == BYTE_SIZE)          /* EBX,BX,BL=destination */
            e("mov  bl, [d+%d*4]\n", rx);
        else
            e("mov  ebx, [d+%d*4]\n", rx);
        if (size == BYTE_SIZE)          /* EDX,DX,DL=source */
            e("mov  dl, [d+edi*4]\n");
        else
            e("mov  edx, [d+edi*4]\n");
        e("shr  byte [x], 1\n");      /* X->CF */
        switch (size)
        {   case BYTE_SIZE: e("adc  bl, dl\n"); break;
            case WORD_SIZE: e("adc  bx, dx\n"); break;
            case LONG_SIZE: e("adc  ebx, edx\n"); break;
        }
        e("mov  dl, ah\n"); /* keep temporary copy of old flags in DL */
        e("lahf\n");
        e("setc byte [x]\n");
        e("seto al\n");
        e("jnz  short .z\n");           /* if non-zero, cleared */
        e("and  dl, 0x40\n");           /* otherwise, unchanged */
        e("and  ah, 0xbf\n");           /* (get rid of new unwanted Z) */
        e("or   ah, dl\n");             /* OR in the old, unchanged Z flag */
        EmitLabel(".z");        

        switch (size)   /* store results */
        {   case BYTE_SIZE: e("mov  [d+%d*4], bl\n", rx); break;
            case WORD_SIZE: e("mov  [d+%d*4], bx\n", rx); break;
            case LONG_SIZE: e("mov  [d+%d*4], ebx\n", rx); break;
        }
    }
    else        /* -(Ay),-(Ax) */
    {
        if (size == BYTE_SIZE)
        {
            e("cmp  edi, byte 7\n");    /* decrement Ay register (-2 for A7) */
            e("cmc\n");
            e("sbb  dword [a+edi*4], byte 1\n");
            if (rx != 7)
                e("dec  dword [a+%d*4]\n", rx);
            else
                e("sub  dword [a+7*4], byte 2\n");
        }
        else if (size == WORD_SIZE)
        {
            e("sub  dword [a+edi*4], byte 2\n");
            e("sub  dword [a+%d*4], byte 2\n", rx);
        }
        else if (size == LONG_SIZE)
        {
            e("sub  dword [a+edi*4], byte 4\n");
            e("sub  dword [a+%d*4], byte 4\n", rx);
        }

        e("mov  ebx, [a+edi*4]\n");   /* first, load up -(Ay) */
        switch (size)
        {   case BYTE_SIZE: ReadByte(); break;
            case WORD_SIZE: ReadWord(); break;
            case LONG_SIZE: ReadLong(); break;
        }
        SaveReg("run", "edx");  /* save data obtained */
        e("mov  ebx, [a+%d*4]\n", rx);/* next, load up -(Ax) */
        switch (size)
        {   case BYTE_SIZE: ReadByte(); break;
            case WORD_SIZE: ReadWord(); break;
            case LONG_SIZE: ReadLong(); break;
        }
        SaveReg("run", "ebx");
        e("mov  ebx, edx\n");       /* EBX,BX,BL=destination */
        RestoreReg("run", "edx");   /* EDX,DX,DL=source */

        e("shr  byte [x], 1\n");      /* X->CF */
        switch (size)
        {   case BYTE_SIZE: e("adc  bl, dl\n"); break;
            case WORD_SIZE: e("adc  bx, dx\n"); break;
            case LONG_SIZE: e("adc  ebx, edx\n"); break;
        }
        e("mov  dl, ah\n"); /* keep temporary copy of old flags in DL */
        e("lahf\n");
        e("setc byte [x]\n");
        e("seto al\n");
        e("jnz  short .z\n");           /* if non-zero, cleared */
        e("and  dl, 0x40\n");           /* otherwise, unchanged */
        e("and  ah, 0xbf\n");           /* (get rid of new unwanted Z) */
        e("or   ah, dl\n");             /* OR in the old, unchanged Z flag */
        EmitLabel(".z");       

        e("mov  edx, ebx\n");   /* store result */
        RestoreReg("run", "ebx");
        switch (size)
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break;
        }
    }
        
    if (!rm)
    {
        if (size == LONG_SIZE)
            EmitTiming(8);
        else
            EmitTiming(4);
    }
    else
    {
        if (size == LONG_SIZE)
            EmitTiming(30);
        else
            EmitTiming(18);
    }

    InstEnd();
    return 1;  
}

int SBCD(unsigned short op, char *mnem, unsigned base_timing)
{
    /*
     * Notes About Undefined Flags:
     * ----------------------------
     *
     * N flag = Set the same as the MSB of the result
     * V flag = If MSB changes from 1 to 0 when result is adjusted for BCD,
     *          the V flag is set, otherwise it is cleared.
     */

    unsigned    rx = (op >> 9) & 7, rm = (op >> 3) & 1;

    InstBegin(op, mnem, 8);

    e("and  edi, byte 7\n");
    if (!rm)    /* Dy,Dx */
    {
        e("mov  al, [d+%d*4]\n", rx); /* AL=destination */
        e("mov  bl, [d+edi*4]\n");    /* BL=source */
        e("shr  byte [x], 1\n");  /* X->CF */
        e("sbb  al, bl\n"); /* subtract */
        e("mov  bh, al\n"); /* save the unadjusted result in BH */
        e("das\n");         /* adjust the packed BCD result */
        e("setc dl\n");
        e("setc byte [x]\n");
        e("jz   short .unchanged_z\n");
        e("and  ah, 0xbf\n");   /* cleared if nonzero, otherwise unchanged */
        EmitLabel(".unchanged_z");
        e("and  ah, 0x7e\n");
        e("or   ah, dl\n");     /* set C */
        e("test al, al\n");     /* N flag */
        e("sets dl\n");
        e("ror  dl, 1\n");      /* N flag -> MSB of DL */
        e("or   ah, dl\n");     /* into N flag */
        e("mov  [d+%d*4], al\n", rx); /* store result */

        /*
         * V flag calculation: Put unadjusted result's MSB into bit 1, and the
         * adjusted result's MSB into bit 0. If the result is 2, the V flag
         * is set.
         */

        e("shr  bh, byte 6\n");
        e("shr  al, byte 7\n");
        e("and  bh, 2\n");
        e("or   bh, al\n");
        e("cmp  bh, 2\n");
        e("sete al\n");     /* 1 -> V */
    }
    else        /* -(Ay),-(Ax) */
    {
        e("cmp  edi, byte 7\n");    /* decrement Ay register (-2 for A7) */
        e("cmc\n");
        e("sbb  dword [a+edi*4], byte 1\n");
        if (rx != 7)
            e("dec  dword [a+%d*4]\n", rx);
        else
            e("sub  dword [a+7*4], byte 2\n");

        e("mov  ebx, [a+edi*4]\n");   /* first, load up -(Ay) */
        ReadByte();
        SaveReg("run", "dl");       /* save byte obtained */
        e("mov  ebx, [a+%d*4]\n", rx);/* next, load up -(Ax) */
        ReadByte();
        e("mov  al, dl\n");         /* AL=destination */
        RestoreReg("run", "dl");    /* DL=source */

        e("shr  byte [x], 1\n");  /* X->CF */
        e("sbb  al, dl\n"); /* subtract */
        e("mov  dh, al\n"); /* save unadjusted result in DH */
        e("das\n");         /* adjust the packed BCD result */

        e("setc dl\n");
        e("setc byte [x]\n");
        e("jz   short .unchanged_z\n");
        e("and  ah, 0xbf\n");   /* cleared if nonzero, otherwise unchanged */
        EmitLabel(".unchanged_z");
        e("and  ah, 0x7e\n");
        e("or   ah, dl\n");     /* set C */
        e("test al, al\n");     /* N flag */
        e("sets dl\n");
        e("ror  dl, 1\n");      /* N flag -> MSB of DL */
        e("or   ah, dl\n");     /* into N flag */

        e("mov  dl, al\n");     /* prepare to write result */

        /*
         * V flag calculation: Put unadjusted result's MSB into bit 1, and the
         * adjusted result's MSB into bit 0. If the result is 2, the V flag
         * is set.
         */
    
        e("shr  dh, byte 6\n");
        e("shr  al, byte 7\n");
        e("and  dh, 2\n");
        e("or   dh, al\n");
        e("cmp  dh, 2\n");
        e("sete al\n"); /* 1->V */

        WriteByte();
    }
        
    if (!rm)
        EmitTiming(6);
    else
        EmitTiming(18);

    InstEnd();
    return 1;  
}

int ABCD(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    rx = (op >> 9) & 7, rm = (op >> 3) & 1;

    /*
     * Notes About Undefined Flags:
     * ----------------------------
     *
     * N flag = Set the same as the MSB of the result
     * V flag = If MSB changes from 0 to 1 when result is adjusted for BCD,
     *          the V flag is set, otherwise it is cleared.
     */

    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");
    if (!rm)    /* Dy,Dx */
    {
        e("mov  al, [d+%d*4]\n", rx); /* AL=destination */
        e("mov  bl, [d+edi*4]\n");    /* BL=source */
        e("shr  byte [x], 1\n");  /* X->CF */
        e("adc  al, bl\n"); /* add with extend */
        e("mov  bh, al\n"); /* save unadjusted result in BH */
        e("daa\n");         /* adjust the packed BCD result */
        e("setc dl\n");
        e("setc byte [x]\n");
        e("jz   short .unchanged_z\n");
        e("and  ah, 0xbf\n");   /* cleared if nonzero, otherwise unchanged */
        EmitLabel(".unchanged_z");        
        e("and  ah, 0x7e\n");
        e("or   ah, dl\n");     /* set C */
        e("test al, al\n");     /* N flag */
        e("sets dl\n");
        e("ror  dl, 1\n");      /* N flag -> MSB of DL */
        e("or   ah, dl\n");     /* into N flag */

        e("mov  [d+%d*4], al\n", rx); /* store result */

        /*
         * V flag calculation: Put unadjusted result's MSB into bit 1, and the
         * adjusted result's MSB into bit 0. If the result is 01, the V flag
         * is set.
         */

        e("shr  bh, byte 6\n");
        e("shr  al, byte 7\n");
        e("and  bh, 2\n");
        e("or   bh, al\n");
        e("cmp  bh, 1\n");
        e("sete al\n");     /* move 1 into V flag */
    
    }
    else        /* -(Ay),-(Ax) */
    {
        e("cmp  edi, byte 7\n");    /* decrement Ay register (-2 for A7) */
        e("cmc\n");
        e("sbb  dword [a+edi*4], byte 1\n");
        if (rx != 7)
            e("dec  dword [a+%d*4]\n", rx);
        else
            e("sub  dword [a+7*4], byte 2\n");

        e("mov  ebx, [a+edi*4]\n");   /* first, load up -(Ay) */
        ReadByte();
        SaveReg("run", "dl");       /* save byte obtained */
        e("mov  ebx, [a+%d*4]\n", rx);/* next, load up -(Ax) */
        ReadByte();
        e("mov  al, dl\n");         /* AL=destination */
        RestoreReg("run", "dl");    /* DL=source */

        e("shr  byte [x], 1\n");  /* X->CF */
        e("adc  al, dl\n"); /* add */
        e("mov  dh, al\n"); /* save unadjusted result in DH */
        e("daa\n");         /* adjust the packed BCD result */

        e("setc dl\n");
        e("setc byte [x]\n");
        e("jz   short .unchanged_z\n");
        e("and  ah, 0xbf\n");   /* cleared if nonzero, otherwise unchanged */
        EmitLabel(".unchanged_z");
        e("and  ah, 0x7e\n");
        e("or   ah, dl\n");     /* set C */
        e("test al, al\n");     /* N flag */
        e("sets dl\n");
        e("ror  dl, 1\n");      /* N flag -> MSB of DL */
        e("or   ah, dl\n");     /* into N flag */

        e("mov  dl, al\n"); /* store result */

        /*
         * V flag calculation: Put unadjusted result's MSB into bit 1, and the
         * adjusted result's MSB into bit 0. If the result is 1, the V flag
         * is set.
         */
    
        e("shr  dh, byte 6\n");
        e("shr  al, byte 7\n");
        e("and  dh, 2\n");
        e("or   dh, al\n");
        e("cmp  dh, 1\n");
        e("sete al\n");     /* 1 -> V */

        WriteByte();
    }
        
    if (!rm)
        EmitTiming(6);
    else
        EmitTiming(18);

    InstEnd();
    return 1;  
}

int ORItoCCR(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    SaveCCR();
    e("mov  dl, [esi]\n");      /* get data */
    e("and  dl, 0x1f\n");       /* mask out unwanted bits */
    e("or   [sr], dl\n");
    e("add  esi, byte 2\n");
    LoadCCR();
    EmitTiming(base_timing);
    InstEnd();
    e("db 66, 65, 82, 84, 33, 1\n");
    return 1;
}

int Scc(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, cond = (op >> 8) & 0xf;
    char        mnem[4];
    char        *mnem_t[] = { "ST",  "SF",  "SHI", "SLS",
                              "SCC", "SCS", "SNE", "SEQ",
                              "SVC", "SVS", "SPL", "SMI",
                              "SGE", "SLT", "SGT", "SLE" };

    if (!CheckEA(ea, "101111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    strcpy(mnem, mnem_t[cond]);

    InstBegin(op, mnem, NumDecodedEA(ea));
    switch (cond)
    {
    /* EAX:0000000000000000 NZ00 000C 0000 000V */
    case 0:     /* T: Always true */
        if ((ea >> 3) == 0) /* Dn */
        {
            e("and  edi, byte 7\n");
            e("mov  byte [d+edi*4], 0xff\n");
            EmitTiming(6);                              /* reg: byte, true */
        }
        else
        {
            if (mpu == 68000 && dummyread && (ea >> 3))
            {
                /*
                 * If the processor type is 68000, and the EA mode is not
                 * register, we can emulate dummy reads!
                 */
    
                LoadFromEA(ea, BYTE_SIZE, 0);
                e("mov  dl, 0xff\n");
                WriteByte();
            }
            else
            {
                e("mov  dl, 0xff\n");
                StoreToEAUsingEDI(ea, BYTE_SIZE, 0);
            }
            EmitTiming(8 + TimingEA(ea, BYTE_SIZE));    /* mem: byte, true */
        }    
        InstEnd();
        return 1;
        break;
    case 1:     /* F: Always false */
        if ((ea >> 3) == 0) /* Dn */
        {
            e("and  edi, byte 7\n");
            e("mov  byte [d+edi*4], 0\n");   
            EmitTiming(4);                              /* reg: byte, false */
        }
        else
        {
            if (mpu == 68000 && dummyread && (ea >> 3))
            {
                /*
                 * If the processor type is 68000, and the EA mode is not
                 * register, we can emulate dummy reads!
                 */
    
                LoadFromEA(ea, BYTE_SIZE, 0);
                e("xor  dl, dl\n");
                WriteByte();
            }
            else
            {
                e("xor  dl, dl\n");
                StoreToEAUsingEDI(ea, BYTE_SIZE, 0);
            }
            EmitTiming(8 + TimingEA(ea, BYTE_SIZE));    /* mem: byte, false */
        }
        InstEnd();
        return 1;
        break;
    case 2:     /* HI: !C & !Z */
        e("test ah, byte 0x41\n");
        e("jz   short do%d\n", op);
        break;
    case 3:     /* LS: C | Z */
        e("test ah, byte 0x41\n");
        e("jnz  short do%d\n", op);
        break;
    case 4:     /* CC: !C */
        e("test ah, byte 1\n");
        e("jz   short do%d\n", op);
        break;
    case 5:     /* CS: C */
        e("test ah, byte 1\n");
        e("jnz  short do%d\n", op);
        break;
    case 6:     /* NE: !Z */
        e("test ah, byte 0x40\n");
        e("jz   short do%d\n", op);
        break;
    case 7:     /* EQ: Z */
        e("test ah, byte 0x40\n");
        e("jnz  short do%d\n", op);
        break;
    case 8:     /* VC: !V */
        e("test al, byte 1\n");
        e("jz   short do%d\n", op);
        break;
    case 9:     /* VS: V */
        e("test al, byte 1\n");
        e("jnz  short do%d\n", op);
        break;
    case 0xa:   /* PL: !N */
        e("test ah, byte 0x80\n");
        e("jz   short do%d\n", op);
        break;
    case 0xb:   /* MI: N */
        e("test ah, byte 0x80\n");
        e("jnz  short do%d\n", op);
        break;
    case 0xc:   /* GE: N & V | !N & !V      (N&V || !N&!V) */
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpe  short do%d\n", op);        
        break;
    case 0xd:   /* LT: N & !V | !N & V */
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpo  short do%d\n", op);
        EmitLabel(".dont_do");
        break;
    case 0xe:   /* GT: N & V & !Z | !N & !V & !Z */
        /* if Z, then it is GE and we don't take this branch */
        e("test ah, byte 0x40\n");
        e("jnz  short .dont_do\n");
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpe  short do%d\n", op);        
        EmitLabel(".dont_do");
        break;
    case 0xf:   /* LE: Z | N & !V | !N & V */
        e("test ah, 0x40\n");
        e("jnz  short do%d\n", op);
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpo  short do%d\n", op);
        break;
    }
    if ((ea >> 3) == 0) /* Dn */
    {
        e("and  edi, byte 7\n");
        e("mov  byte [d+edi*4], 0\n");   
        EmitTiming(4);                              /* reg: byte, false */
    }
    else
    {
        e("false%d:\n", op);    /* this prevents redefinitions of labels in
                                   the EA code */
        if (mpu == 68000 && dummyread && (ea >> 3))
        {
            /*
             * If the processor type is 68000, and the EA mode is not
             * register, we can emulate dummy reads!
             */

            LoadFromEA(ea, BYTE_SIZE, 0);
            e("xor  dl, dl\n");
            WriteByte();
        }
        else
        {
            e("xor  dl, dl\n");
            StoreToEAUsingEDI(ea, BYTE_SIZE, 0);
        }
        EmitTiming(8 + TimingEA(ea, BYTE_SIZE));    /* mem: byte, false */
    }
    InstEnd();
    Align(4);
    e("do%d:\n", op);
    if ((ea >> 3) == 0) /* Dn */
    {
        e("and  edi, byte 7\n");
        e("mov  byte [d+edi*4], 0xff\n");
        EmitTiming(6);                              /* reg: byte, true */
    }
    else
    {
        e("true%d:\n", op);     /* this prevents redefinitions of labels in
                                   the EA code */
        if (mpu == 68000 && dummyread && (ea >> 3))
        {
            /*
             * If the processor type is 68000, and the EA mode is not
             * register, we can emulate dummy reads!
             */

            LoadFromEA(ea, BYTE_SIZE, 0);
            e("mov  dl, 0xff\n");
            WriteByte();
        }
        else
        {
            e("mov  dl, 0xff\n");
            StoreToEAUsingEDI(ea, BYTE_SIZE, 0);
        }
        EmitTiming(8 + TimingEA(ea, BYTE_SIZE));    /* mem: byte, true */
    }    
    InstEnd();
    return 1;
}

int EXG(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    rx = (op >> 9) & 7, opmode = (op >> 3) & 0x1f;

    if (opmode != 0x8 && opmode != 0x9 && opmode != 0x11)   return 0;

    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");
    switch (opmode)
    {
    case 8:     /* Dx,Dy */
        e("mov  ebx, [d+%d*4]\n", rx);
        e("mov  edx, [d+edi*4]\n");
        e("mov  [d+%d*4], edx\n", rx);
        e("mov  [d+edi*4], ebx\n");
        break;
    case 9:     /* Ax,Ay */
        e("mov  ebx, [a+%d*4]\n", rx);
        e("mov  edx, [a+edi*4]\n");
        e("mov  [a+%d*4], edx\n", rx);
        e("mov  [a+edi*4], ebx\n");
        break;
    case 0x11:  /* Dx,Ay */
        e("mov  ebx, [d+%d*4]\n", rx);
        e("mov  edx, [a+edi*4]\n");
        e("mov  [d+%d*4], edx\n", rx);
        e("mov  [a+edi*4], ebx\n");
        break;
    }
    EmitTiming(base_timing);
    InstEnd();

    return 1;
}

int MULS(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, dn = (op >> 9) & 7;

    if (!CheckEA(ea, "101111111111"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);
    e("mov  ax, [d+%d*4]\n", dn);
    e("imul dx\n");             /* ax*dx->dx:ax */
    e("shl  edx, byte 16\n");   /* EDX=32-bit result */
    e("mov  dx, ax\n");
    e("mov  [d+%d*4], edx\n", dn);
    e("test edx, edx\n");       /* C is always cleared */
    e("lahf\n");
    e("xor  al, al\n");         /* V=1 only occurs on 32*32 (not on 68000) */
    EmitTiming(base_timing + TimingEA(ea, WORD_SIZE));
    InstEnd();

    return 1;
}

int MULU(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, dn = (op >> 9) & 7;

    if (!CheckEA(ea, "101111111111"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);
    e("mov  ax, [d+%d*4]\n", dn);
    e("mul  dx\n"); /* ax*dx->dx:ax */
    e("shl  edx, byte 16\n");   /* EDX=32-bit result */
    e("mov  dx, ax\n");
    e("mov  [d+%d*4], edx\n", dn);
    e("test edx, edx\n");       /* C is always cleared */
    e("lahf\n");
    e("xor  al, al\n");         /* V=1 only occurs on 32*32 (not on 68000) */
    EmitTiming(base_timing + TimingEA(ea, WORD_SIZE));
    InstEnd();

    return 1;
}

int DIVS(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, dn = (op >> 9) & 7;

    if (!CheckEA(ea, "101111111111"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    /* See the note in DIVU() for info on how overflow is detected */

    InstBegin(op, mnem, NumDecodedEA(ea));

    if (TimingEA(ea, WORD_SIZE))
        e("sub     ecx, byte %d\n", TimingEA(ea, WORD_SIZE));   /* do 1st.. */
    LoadFromEA(ea, WORD_SIZE, 0);

    e("test dx, dx\n");
    e("jz   .divide_by_zero\n");
    e("movsx    ebx, dx\n");        /* divisor->EBX (sign extend to work) */
    SaveReg("run", "eax");
    e("mov  eax, [d+%d*4]\n", dn);    /* dividend */
    e("cdq\n");                         /* EAX->sign extend->EDX:EAX */
    e("idiv ebx\n");
    e("cmp  eax, -32768\n");
    e("jl   .overflow\n");
    e("cmp  eax, 32767\n");
    e("jg   .overflow\n");
    e("test ax, ax\n");                 /* test for flags */
    e("mov     [d+(%d*4)+2], dx\n", dn);  /* store back to Dn */
    e("mov     [d+(%d*4)+0], ax\n", dn);
    e("lahf\n");    /* the mov's did not affect the flags... */
    e("xor     al, al\n"); /* C cleared by TEST */
    EmitTiming(base_timing);
    InstEnd();

    EmitLabel(".overflow");
    RestoreReg("run", "eax");
    e("mov  eax, 1\n");             /* clear C and set overflow */
    EmitTiming(base_timing);
    InstEnd();

    EmitLabel(".divide_by_zero");
    e("xor  eax, eax\n");           /* clear C */
    e("jmp     near exception_divide_by_zero\n");

    return 1;
}

int DIVU(unsigned short op, char *mnem, unsigned base_timing)
{
    /*
     * Overflow Detection:
     *
     * Overflow detection is actually pretty simple. The 68000 DIVU and DIVS
     * instructions divide 32-bit operands by 16-bit operands. What I've done
     * is zero extend the operands to 64-bit and 32-bit and then divide. If
     * the result is larger than 0xFFFF (16 bits), the overflow condition is
     * set.
     *
     * See the end of this function for an alternate method.
     */

    unsigned    ea = op & 0x3f, dn = (op >> 9) & 7;

    if (!CheckEA(ea, "101111111111"))   return 0;
    if (!NumDecodedEA(ea))              return 0;
    
    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);

    if (TimingEA(ea, WORD_SIZE))
        e("sub     ecx, byte %d\n", TimingEA(ea, WORD_SIZE));   /* do 1st.. */

    e("test dx, dx\n");
    e("jz   .divide_by_zero\n");
    e("mov      ebx, edx\n");
    e("and      ebx, 0xffff\n");    /* divisor->EBX */
    SaveReg("run", "eax");
    e("mov  eax, [d+%d*4]\n", dn);  /* dividend */
    e("xor  edx, edx\n");           /* EDX:EAX=dividend */
    e("div  ebx\n");
    e("cmp  eax, 0xffff\n");
    e("jg   .overflow\n");
    e("test ax, ax\n");             /* test for flags */
    e("mov     [d+(%d*4)+2], dx\n", dn);    /* store back to Dn */
    e("mov     [d+(%d*4)+0], ax\n", dn);
    e("lahf\n");            /* the mov's did not affect the flags... */
    e("xor     al, al\n");  /* C cleared by TEST */
    EmitTiming(base_timing);
    InstEnd();

    EmitLabel(".overflow");
    RestoreReg("run", "eax");
    e("mov  eax, 1\n");             /* clear C and set overflow */
    EmitTiming(base_timing);
    InstEnd();

    EmitLabel(".divide_by_zero");
    e("xor  eax, eax\n");           /* clear C */
    e("jmp     near exception_divide_by_zero\n");

    return 1;

    /*
     *
     * An alternate method to detecting overflows is shown below. It doesn't
     * seem to be as acurrate as the method that is actually used, but perhaps
     * I implemented it wrong or misunderstood it. It broke Comix Zone, Hard
     * Drivin', and Race Drivin' (well, caused graphical glitches actually.)
     *
     * Kuwanger in #programmers (DALnet, of course) came up with a cool idea
     * on how to detect overflow before actually dividing. He used an example
     * of AH:AL/BL ... in the emulator, it is DX:AX/BX. The dividend, DX:AX
     * is stored entirely in EDX, and it is shifted right by the bit number of
     * the highest 1 bit in BX, the divisor. If the high 16-bits of EDX (which
     * would be DX in DX:AX) are not clear, then an overflow would occur.
     *
     * Here's Kuwanger's classic lecture:
     *
     * <Kuwanger> Maybe I should use a smaller example.
     * <Kuwanger> Lets use, ah:al/bl
     * ...
     * <Kuwanger> trzy: You could just convert one to a bit value.
     * <Kuwanger> trzy: Ie, hb(bl)..
     * <Kuwanger> And then shift the other one ah:al
     * ...
     * <Kuwanger> Though hb(bl) would have to return 0, that time.
     * <Kuwanger> then ah:all>>hb(bl)
     * <Kuwanger> err, ah:al, rather
     * <Kuwanger> Ie, you shift by the most prominent bit.
     * <Kuwanger> And if ah is non-zero..
     * <Kuwanger> Well, then the number won't fit.
     * ...
     * <Kuwanger> Lets try an extreme case.
     * <Kuwanger> 65535/255
     * <Kuwanger> 65535>>7
     * ...
     * <Kuwanger> That obviously doesn't fit. :)
     *
     * Hip-hip-HURRAY! 
     */
}    

int NOT(unsigned short op, char *mnem, unsigned base_timing)
{
    int         size;
    unsigned    ea = op & 0x3f, t;

    if (!CheckEA(ea, "101111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;
    switch ((op >> 6) & 3)
    {   case 0: size = BYTE_SIZE; break;
        case 1: size = WORD_SIZE; break;
        case 2: size = LONG_SIZE; break;
        default: return 0;  }

    InstBegin(op, mnem, NumDecodedEA(ea));
    if ((ea >> 3) == 0) /* Dn */
    {
        e("and  edi, byte 7\n");
        if (size == BYTE_SIZE)
            e("mov  dl, [d+edi*4]\n");
        else
            e("mov  edx, [d+edi*4]\n");
        switch (size)
        {   case BYTE_SIZE: e("not  dl\n"); e("test dl, dl\n"); break;
            case WORD_SIZE: e("not  dx\n"); e("test dx, dx\n"); break;
            case LONG_SIZE: e("not  edx\n"); e("test edx, edx\n"); break;
        }
        e("lahf\n");        /* C cleared */
        e("xor  al, al\n"); /* V cleared */
        switch (size)
        {   case BYTE_SIZE: e("mov  [d+edi*4], dl\n"); break;
            case WORD_SIZE: e("mov  [d+edi*4], dx\n"); break;
            case LONG_SIZE: e("mov  [d+edi*4], edx\n"); break;
        }
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE:
                t = 4; break;
            case LONG_SIZE:
                t = 6; break;   }
    }
    else                /* mem */
    {        
        LoadFromEA(ea, size, 0);
        switch (size)
        {   case BYTE_SIZE: e("not  dl\n"); e("test dl, dl\n"); break;
            case WORD_SIZE: e("not  dx\n"); e("test dx, dx\n"); break;
            case LONG_SIZE: e("not  edx\n"); e("test edx, edx\n"); break;
        }
        e("lahf\n");        /* C cleared */
        e("xor  al, al\n"); /* V cleared */
        switch (size)
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break;
        }    
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE:
                t = 8; break;
            case LONG_SIZE:
                t = 12; break;   }
        t += TimingEA(ea, size);
    }
    EmitTiming(t);
    InstEnd();
    return 1;
}

int RTE(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("test byte [sr+1], 0x20\n");      /* must be in supervisor mode */
    e("jz   near exception_privilege_violation\n");
    e("mov  ebx, [a+7*4]\n");
    SaveReg("run", "ebx");
    ReadWord();                         /* get SR */
    RestoreReg("run", "ebx");
    e("and  edx, 0xa71f\n");
    e("mov  [sr], edx\n");
    LoadCCR();
    e("add  ebx, byte 2\n");
    SaveReg("run", "ebx");
    ReadLong();                         /* get PC */
    RestoreReg("run", "ebx");
    e("add  ebx, byte 4\n");
    /*
     * If 68010, we have to fetch the format word and check it. If invalid,
     * we branch to stackframe_error. The PC points at the word AFTER the
     * faulty RTE.
     */
    if (mpu == 68010)
    {
        SaveReg("run", "edx");  /* EDX has the PC! */
        SaveReg("run", "ebx");
        ReadWord();
        RestoreReg("run", "ebx");
        e("add  ebx, byte 2\n");
        e("test dh, 0xf0\n");   /* format should only be 0x0000 */
        e("jnz  near stackframe_error\n");
        RestoreReg("run", "edx");
    }
        
    e("mov  [a+7*4], ebx\n");
    e("mov  esi, edx\n");
    UpdateFetchPtr();
    e("test byte [sr+1], 0x20\n");      /* if no longer in supervisor,
                                           SP->SSP, USP->SP */
    e("jnz  short .in_s\n");
    e("mov  ebx, [a+7*4]\n");
    e("mov  edx, [__sp]\n");
    e("mov  [a+7*4], edx\n");
    e("mov  [__sp], ebx\n");
    SetUserAddressSpace();              /* map in user address space */
    EmitLabel(".in_s");
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int ANDItoCCR(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    SaveCCR();
    e("mov  dl, [esi]\n");      /* get data */
    e("and  dl, 0x1f\n");       /* mask out unwanted bits */
    e("and  [sr], dl\n");
    e("add  esi, byte 2\n");
    LoadCCR();
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int CMPA(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    opmode = (op >> 6) & 7, t;

    if (!CheckEA(op & 0x3f, "111111111111"))    return 0;
    if (!NumDecodedEA(op & 0x3f))               return 0;
    if (opmode != 3 && opmode != 7)             return 0;

    InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
    if (opmode == 3)    LoadFromEA(op & 0x3f, WORD_SIZE, 1);
    else                LoadFromEA(op & 0x3f, LONG_SIZE, 0);
    e("cmp  [a+%d*4], edx\n", (op >> 9) & 7);
    e("lahf\n");
    e("seto al\n");
    if (opmode == 3)    /* word size */
        t = 6 + TimingEA(op & 0x3f, WORD_SIZE);
    else                /* long */
        t = 6 + TimingEA(op & 0x3f, LONG_SIZE);
    EmitTiming(t);
    InstEnd();
    return 1;
}

int PEA(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    t;

    if (!CheckEA(op & 0x3f, "001001111011"))    return 0;
    if (!NumDecodedEA(op & 0x3f))               return 0;
    InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
    LoadControlEA(op & 0x3f);
    e("sub  dword [a+7*4], byte 4\n");  /* SP-4->SP */
    e("mov  edx, ebx\n");
    e("mov  ebx, [a+7*4]\n");
    WriteLong();                        /* <ea>->(SP) */
    switch ((op >> 3) & 7)      /* timing based on control addressing mode */
    {   case 2: t = 12; break;  /* (An) */
        case 5: t = 16; break;  /* (d16,An) */
        case 6: t = 20; break;  /* (d8,An,Xn) */
        case 7:
            switch (op & 7)
            {   case 0: t = 16; break;  /* (xxx).W */
                case 1: t = 20; break;  /* (xxx).L */
                case 2: t = 16; break;  /* (d16,pc) */
                case 3: t = 20; break;  /* (d8,pc,Xn) */
            } break;
    }
    EmitTiming(t);
    InstEnd();
    return 1;
}

int SWAP(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");
    e("mov  edx, [d+edi*4]\n"); 
    e("rol  edx, byte 16\n");       /* swap words */
    e("test edx, edx\n");           /* clears CF, will set N and Z */
    e("lahf\n");                    
    e("xor  al, al\n");             /* clear V */
    e("mov  [d+edi*4], edx\n");
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int BSET(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    t;

    if (((op >> 6) & 7) != 7 && ((op >> 6) & 7) != 3)   return 0;
    if (op & 0x100)                 /* dynamic */
    {    
        if (!CheckEA(op & 0x3f, "101111111000"))    return 0;
        if (!NumDecodedEA(op & 0x3f))               return 0;
        InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
        if (((op >> 3) & 7) == 0)   /* Dn is long */
        {
            t = 8;
            e("and  edi, byte 7\n");
            e("mov  ebx, [d+%d*4]\n", (op >> 9) & 7); /* bit # */
            e("and  ebx, byte 31\n");           /* same as modulo 32 */
            e("bts  dword [d+edi*4], ebx\n"); /* bit->CF, set */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
        }
        else                        /* mem is byte */
        {
            t = 8;
            LoadFromEA(op & 0x3f, BYTE_SIZE, 0);
            e("mov  edi, [d+%d*4]\n", (op >> 9) & 7); /* bit # */
            e("and  edi, byte 7\n");             /* same as modulo 8 */
            e("bts  edx, edi\n");                /* bit->CF, set */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
            WriteByte();                         /* clears EDI for us */
        }
    }
    else if (!(op & 0x100))         /* static */
    {
        if (!CheckEA(op & 0x3f, "101111111000"))    return 0;
        if (!NumDecodedEA(op & 0x3f))               return 0;
        InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
        if (((op >> 3) & 7) == 0)   /* Dn is long */
        {
            t = 12;
            e("and  edi, byte 7\n");    /* isolate Dn reg # */
            e("mov  ebx, [esi]\n");     /* get bit # */
            e("and  ebx, byte 31\n");   /* same as modulo 32 */
            e("add  esi, byte 2\n");
            e("bts  dword [d+edi*4], ebx\n"); /* bit->CF, set */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
        }
        else                        /* mem is byte */
        {
            t = 12;
            SaveRegTo("run", "edi", "esi"); /* save to EDI spot */
            e("add  esi, byte 2\n");
            LoadFromEA(op & 0x3f, BYTE_SIZE, 0);
            RestoreReg("run", "edi");
            e("mov  edi, [edi]\n");
            e("and  edi, byte 7\n");    /* same as modulo 8 */
            e("bts  edx, edi\n");       /* bit->CF, set */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
            WriteByte();            /* write back, EDI is cleared */
        }        
    }
    if (((op & 0x3f) & 7) == 0) /* Dn is long */
        EmitTiming(t);
    else                        /* mem is byte */
        EmitTiming(t + TimingEA(op & 0x3f, BYTE_SIZE));        
    InstEnd();
    return 1;
}

int BCLR(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    t;

    if (((op >> 6) & 7) != 6 && ((op >> 6) & 7) != 2)   return 0;
    if (op & 0x100)                 /* dynamic */
    {    
        if (!CheckEA(op & 0x3f, "101111111000"))    return 0;
        if (!NumDecodedEA(op & 0x3f))               return 0;
        InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
        if (((op >> 3) & 7) == 0)   /* Dn is long */
        {
            t = 10;
            e("and  edi, byte 7\n");
            e("mov  ebx, [d+%d*4]\n", (op >> 9) & 7); /* bit # */
            e("and  ebx, byte 31\n");           /* same as modulo 32 */
            e("btr  dword [d+edi*4], ebx\n"); /* bit->CF, clear */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
        }
        else                        /* mem is byte */
        {
            t = 8;
            LoadFromEA(op & 0x3f, BYTE_SIZE, 0);
            e("mov  edi, [d+%d*4]\n", (op >> 9) & 7); /* bit # */
            e("and  edi, byte 7\n");             /* same as modulo 8 */
            e("btr  edx, edi\n");                /* bit->CF, clear */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
            WriteByte();                         /* clears EDI for us */
        }
    }
    else if (!(op & 0x100))         /* static */
    {
        if (!CheckEA(op & 0x3f, "101111111000"))    return 0;
        if (!NumDecodedEA(op & 0x3f))               return 0;
        InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
        if (((op >> 3) & 7) == 0)   /* Dn is long */
        {
            t = 14;
            e("and  edi, byte 7\n");    /* isolate Dn reg # */
            e("mov  ebx, [esi]\n");     /* get bit # */
            e("and  ebx, byte 31\n");   /* same as modulo 32 */
            e("add  esi, byte 2\n");
            e("btr  dword [d+edi*4], ebx\n"); /* bit->CF, clear */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
        }
        else                        /* mem is byte */
        {
            t = 12;
            SaveRegTo("run", "edi", "esi"); /* save to EDI spot */
            e("add  esi, byte 2\n");
            LoadFromEA(op & 0x3f, BYTE_SIZE, 0);
            RestoreReg("run", "edi");
            e("mov  edi, [edi]\n");
            e("and  edi, byte 7\n");    /* same as modulo 8 */
            e("btr  edx, edi\n");       /* bit->CF, clear */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
            WriteByte();                /* write back, EDI is cleared */
        }        
    }
    if (((op & 0x3f) & 7) == 0) /* Dn is long */
        EmitTiming(t);
    else                        /* mem is byte */
        EmitTiming(t + TimingEA(op & 0x3f, BYTE_SIZE));        
    InstEnd();
    return 1;
}

int BCHG(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    t;

    if (((op >> 6) & 7) != 5 && ((op >> 6) & 7) != 1)   return 0;
    if (op & 0x100)                 /* dynamic */
    {    
        if (!CheckEA(op & 0x3f, "101111111000"))    return 0;
        if (!NumDecodedEA(op & 0x3f))               return 0;
        InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
        if (((op >> 3) & 7) == 0)   /* Dn is long */
        {
            t = 8;
            e("and  edi, byte 7\n");
            e("mov  ebx, [d+%d*4]\n", (op >> 9) & 7); /* bit # */
            e("and  ebx, byte 31\n");           /* same as modulo 32 */
            e("btc  dword [d+edi*4], ebx\n"); /* bit->CF, change */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
        }
        else                        /* mem is byte */
        {
            t = 8;
            LoadFromEA(op & 0x3f, BYTE_SIZE, 0);
            e("mov  edi, [d+%d*4]\n", (op >> 9) & 7); /* bit # */
            e("and  edi, byte 7\n");             /* same as modulo 8 */
            e("btc  edx, edi\n");                /* bit->CF, change */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
            WriteByte();                         /* clears EDI for us */
        }
    }
    else if (!(op & 0x100))         /* static */
    {
        if (!CheckEA(op & 0x3f, "101111111000"))    return 0;
        if (!NumDecodedEA(op & 0x3f))               return 0;
        InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
        if (((op >> 3) & 7) == 0)   /* Dn is long */
        {
            t = 12;
            e("and  edi, byte 7\n");    /* isolate Dn reg # */
            e("mov  ebx, [esi]\n");     /* get bit # */
            e("and  ebx, byte 31\n");   /* same as modulo 32 */
            e("add  esi, byte 2\n");
            e("btc  dword [d+edi*4], ebx\n"); /* bit->CF, change */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
        }
        else                        /* mem is byte */
        {
            t = 12;
            SaveRegTo("run", "edi", "esi"); /* save to EDI spot */
            e("add  esi, byte 2\n");
            LoadFromEA(op & 0x3f, BYTE_SIZE, 0);
            RestoreReg("run", "edi");
            e("mov  edi, [edi]\n");
            e("and  edi, byte 7\n");    /* same as modulo 8 */
            e("btc  edx, edi\n");       /* bit->CF, change */
            e("setnc dh\n");        /* zero status of C->DH */
            e("and  ah, 0xbf\n");   /* get rid of old Z */
            e("shl  dh, 6\n");      /* put C in Z position */
            e("or   ah, dh\n");     /* set the new Z */
            WriteByte();                /* write back, EDI is cleared */
        }        
    }
    if (((op & 0x3f) & 7) == 0) /* Dn is long */
        EmitTiming(t);
    else                        /* mem is byte */
        EmitTiming(t + TimingEA(op & 0x3f, BYTE_SIZE));        
    InstEnd();
    return 1;
}

int ANDItoSR(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("test byte [sr+1], 0x20\n");    /* in supervisor? */
    e("jz   near exception_privilege_violation\n");
    SaveCCR();  /* save flags to SR */
    e("mov  edx, [esi]\n");
    e("and  edx, 0xa71f\n");    /* mask out unwanted bits */
    e("and  [sr], dx\n");
    e("add  esi, byte 2\n");
    LoadCCR();  /* get flags back */
    e("test byte [sr+1], 0x20\n");    /* if we changed to User, swap SPs */
    e("jnz  short .in_s\n");
    e("mov  ebx, [a+7*4]\n");
    e("mov  edx, [__sp]\n");
    e("mov  [a+7*4], edx\n");
    e("mov  [__sp], ebx\n");
    SetUserAddressSpace();              /* map in user address space */
    EmitLabel(".in_s");
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int UNLK(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");
    e("mov  ebx, [a+edi*4]\n");   /* An->SP */
    SaveReg("run", "ebx");          /* to avoid addrclip() */
    SaveReg("run", "edi");
    ReadLong();                     /* (SP)->An */
    RestoreReg("run", "edi");
    RestoreReg("run", "ebx");
    e("mov  [a+edi*4], edx\n");
    e("add  ebx, byte 4\n");
    e("mov  [a+7*4], ebx\n");     /* SP+4->SP */
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int LINK(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");
    e("mov  ebx, [a+7*4]\n"); /* EBX=SP */
    e("sub  ebx, byte 4\n");
    e("mov  edx, [a+edi*4]\n");
    SaveReg("run", "ebx");
    SaveReg("run", "edi");
    WriteLong();                    /* An->(SP) */
    RestoreReg("run", "edi");
    RestoreReg("run", "ebx");
    e("mov  [a+edi*4], ebx\n");   /* SP->An */
    e("movsx    edx, word [esi]\n");
    e("add      ebx, edx\n");       /* dn + SP->SP */
    e("mov      [a+7*4], ebx\n");
    e("add      esi, byte 2\n");
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int ROxMem(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, dr = (op >> 8) & 1;
    char        mnem[4];
    char        *mnem_t[] = { "ROR", "ROL" };

    if (!CheckEA(ea, "001111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    strcpy(mnem, mnem_t[(op >> 8) & 1]);

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);
    if (dr) /* left */
        e("rol  dx, byte 1\n");
    else
        e("ror  dx, byte 1\n");
    e("setc al\n");     /* AL=CF */
    e("test dx, dx\n"); /* CF is cleared */
    e("lahf\n");
    e("or   ah, al\n"); /* put in CF */
    e("xor  al, al\n"); /* V is always cleared */
    WriteWord();    /* write back to mem */
    EmitTiming(base_timing + TimingEA(ea, WORD_SIZE));
    InstEnd();
    return 1;
}

int ROxReg(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    size, t, opr_i, count;
    char        *opr[] =    { "ror", "rol" };
    char        mnem[4];
    char        *mnem_t[] = { "ROR", "ROL" };

    switch ((op >> 6) & 3)
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0;   }

    opr_i = (op >> 8) & 1;
    strcpy(mnem, mnem_t[opr_i]);

    count = (op >> 9) & 7;
    if (!count) count = 8;

    InstBegin(op, mnem, 8); /* 8 regs */
    e("and  edi, byte 7\n");
    if ((op >> 5) & 1)  /* reg contains count */
    {
        e("mov  ebx, ecx\n");   /* save ECX */
        e("mov  ecx, [d+%d*4]\n", (op >> 9) & 7);
        e("and  ecx, byte 0x3f\n"); /* modulo 64 */
        e("mov  edx, [d+edi*4]\n");
        switch (size)
        {   case BYTE_SIZE: e("%s   dl, cl\n", opr[opr_i]); e("setc al\n"); e("test dl, dl\n"); break;
            case WORD_SIZE: e("%s   dx, cl\n", opr[opr_i]); e("setc al\n"); e("test dx, dx\n"); break;
            case LONG_SIZE: e("%s   edx, cl\n", opr[opr_i]); e("setc al\n"); e("test edx, edx\n"); break;
        }        
        e("lahf\n");
        e("test cl, cl\n"); /* if shift count of 0, X is unaffected */
        e("jnz  short .not_zero\n");
        e("xor      al, al\n"); /* clear CF, count was 0 */
        EmitLabel(".not_zero");
        e("or   ah, al\n");     /* put CF in with rest of flags */
        e("xor  al, al\n");     /* V always cleared */
        switch (size)   /* write back results */
        {   case BYTE_SIZE: e("mov  [d+edi*4], dl\n", opr[opr_i]); break;
            case WORD_SIZE: e("mov  [d+edi*4], dx\n", opr[opr_i]); break;
            case LONG_SIZE: e("mov  [d+edi*4], edx\n", opr[opr_i]); break;
        }
        /* timing */
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE: t = 6; break;
            case LONG_SIZE: t = 8; break;   }
        e("shl  cl, byte 1\n"); /* 2n */
        e("add  cl, byte %d\n", t);
        e("neg  ecx\n");
        e("add  ecx, ebx\n");
        e("js   near Turbo68KRun_done\n");
    }
    else                /* immediate count */
    {
        e("mov  edx, [d+edi*4]\n");
        switch (size)
        {   case BYTE_SIZE: e("%s   dl, byte %d\n", opr[opr_i], count); e("setc al\n"); e("test dl, dl\n"); break;
            case WORD_SIZE: e("%s   dx, byte %d\n", opr[opr_i], count); e("setc al\n"); e("test dx, dx\n"); break;
            case LONG_SIZE: e("%s   edx, byte %d\n", opr[opr_i], count); e("setc al\n"); e("test edx, edx\n"); break;
        }        
        e("lahf\n");
        e("or       ah, al\n"); /* set CF */
        e("xor  al, al\n"); /* V always cleared */
        switch (size)   /* write back results */
        {   case BYTE_SIZE: e("mov  [d+edi*4], dl\n"); break;
            case WORD_SIZE: e("mov  [d+edi*4], dx\n"); break;
            case LONG_SIZE: e("mov  [d+edi*4], edx\n"); break;
        }
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE: t = 6; break;
            case LONG_SIZE: t = 8; break;   }
        EmitTiming(t + count * 2);
    }
    InstEnd();
    return 1;
}

int EXT(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    opmode = (op >> 6) & 7;

    if (opmode != 2 && opmode != 3) return 0;

    InstBegin(op, mnem, 8);
    e("and  edi, byte 7\n");
    e("mov  edx, [d+edi*4]\n");
    if (opmode == 2)    /* byte -> word */
    {
        e("movsx    dx, dl\n");
        e("mov      [d+edi*4], dx\n");
        e("test     dx, dx\n");
    }
    else if (opmode == 3)   /* word -> long */
    {
        e("movsx    edx, dx\n");
        e("mov      [d+edi*4], edx\n");
        e("test     edx, edx\n");
    }
    else if (opmode == 7)   /* byte -> long */
    {
        /*
         * Note: This code is never executed. EXTB is present on 68020 and
         * higher processors. Turbo68K was only intended to emulate the
         * 68000. This code was an accident, but it will be kept here in case
         * in the future it is needed.
         */

        e("movsx    edx, dl\n");
        e("mov      [d+edi*4], edx\n");
        e("test     edx, edx\n");
    }
    e("lahf\n");    /* CF is already cleared by TEST */
    e("xor  al, al\n"); /* V always cleared */
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int ROXxMem(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, dr = (op >> 8) & 1;
    char        mnem[5];
    char        *mnem_t[] = { "ROXR", "ROXL" };

    if (!CheckEA(ea, "001111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    strcpy(mnem, mnem_t[(op >> 8) & 1]);

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);
    e("shr  byte [x], 1\n");
    if (dr) /* left */
        e("rcl  dx, byte 1\n");
    else
        e("rcr  dx, byte 1\n");
    e("setc al\n");     /* AL=CF */
    e("setc byte [x]\n");
    e("test dx, dx\n"); /* CF is cleared */
    e("lahf\n");
    e("or   ah, al\n"); /* put in CF */
    e("xor  al, al\n");
    WriteWord();    /* write back to mem */
    EmitTiming(base_timing + TimingEA(ea, WORD_SIZE));
    InstEnd();
    return 1;
}

int ROXxReg(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    size, t, opr_i, count;
    char        *opr[] =    { "rcr", "rcl" };
    char        mnem[5];
    char        *mnem_t[] = { "ROXR", "ROXL" };

    switch ((op >> 6) & 3)
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0;   }

    opr_i = (op >> 8) & 1;
    strcpy(mnem, mnem_t[opr_i]);

    count = (op >> 9) & 7;
    if (!count) count = 8;

    InstBegin(op, mnem, 8); /* 8 regs */
    e("and  edi, byte 7\n");
    if ((op >> 5) & 1)  /* reg contains count */
    {
        e("mov  ebx, ecx\n");   /* save ECX */
        e("mov  ecx, [d+%d*4]\n", (op >> 9) & 7);
        e("and  ecx, byte 0x3f\n"); /* modulo 64 */
        e("mov  dl, [x]\n");  
        e("shr  dl, byte 1\n");     /* X->CF so we can use RCL/RCR */
        e("mov  edx, [d+edi*4]\n");
        switch (size)
        {   case BYTE_SIZE: e("%s   dl, cl\n", opr[opr_i]); e("setc al\n"); e("test dl, dl\n"); break;
            case WORD_SIZE: e("%s   dx, cl\n", opr[opr_i]); e("setc al\n"); e("test dx, dx\n"); break;
            case LONG_SIZE: e("%s   edx, cl\n", opr[opr_i]); e("setc al\n"); e("test edx, edx\n"); break;
        }        
        e("lahf\n");
        e("test cl, cl\n"); /* if shift count of 0, X is unaffected */
        e("jz   short .zero_count\n");
        e("or       ah, al\n"); /* set CF */
        e("test     ah, 1\n");  /* CF */
        e("setnz    byte [x]\n");
        e("jmp      short .skip\n");
        EmitLabel(".zero_count");
        e("mov  al, [x]\n");  /* X->C */
        e("and  al, byte 1\n");
        e("and  ah, 0xfe\n");   /* get rid of old C before putting new C in */
        e("or   ah, al\n");
        EmitLabel(".skip");
        e("xor  al, al\n"); /* V always cleared */
        switch (size)   /* write back results */
        {   case BYTE_SIZE: e("mov  [d+edi*4], dl\n", opr[opr_i]); break;
            case WORD_SIZE: e("mov  [d+edi*4], dx\n", opr[opr_i]); break;
            case LONG_SIZE: e("mov  [d+edi*4], edx\n", opr[opr_i]); break;
        }

        /* timing */
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE: t = 6; break;
            case LONG_SIZE: t = 8; break;   }
        e("shl  cl, byte 1\n"); /* 2n */
        e("add  cl, byte %d\n", t);
        e("neg  ecx\n");
        e("add  ecx, ebx\n");
        e("js   near Turbo68KRun_done\n");
    }
    else                /* immediate count */
    {
        e("mov  dl, [x]\n");  
        e("shr  dl, byte 1\n");     /* X->CF so we can use RCL/RCR */    
        e("mov  edx, [d+edi*4]\n");
        switch (size)
        {   case BYTE_SIZE: e("%s   dl, byte %d\n", opr[opr_i], count); e("setc al\n"); e("test dl, dl\n"); break;
            case WORD_SIZE: e("%s   dx, byte %d\n", opr[opr_i], count); e("setc al\n"); e("test dx, dx\n"); break;
            case LONG_SIZE: e("%s   edx, byte %d\n", opr[opr_i], count); e("setc al\n"); e("test edx, edx\n"); break;
        }        
        e("lahf\n");
        e("or       ah, al\n"); /* set CF */
        e("test     ah, 1\n");  /* CF */
        e("setnz    byte [x]\n");
        e("xor  al, al\n"); /* V always cleared */
        switch (size)   /* write back results */
        {   case BYTE_SIZE: e("mov  [d+edi*4], dl\n", opr[opr_i]); break;
            case WORD_SIZE: e("mov  [d+edi*4], dx\n", opr[opr_i]); break;
            case LONG_SIZE: e("mov  [d+edi*4], edx\n", opr[opr_i]); break;
        }
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE: t = 6; break;
            case LONG_SIZE: t = 8; break;   }
        EmitTiming(t + count * 2);
    }
    InstEnd();
    return 1;
}

int MOVEtoCCR(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f;

    if (!CheckEA(ea, "101111111111"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);
    e("mov  [sr], dl\n");
    LoadCCR();  /* update flags */
    EmitTiming(base_timing + TimingEA(ea, WORD_SIZE));  /* technically, word
                                                           sized */
    InstEnd();
    return 1;
}

int LSxMem(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, dr = (op >> 8) & 1;
    char        mnem[4];
    char        *mnem_t[] = { "LSR", "LSL" };

    if (!CheckEA(ea, "001111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    strcpy(mnem, mnem_t[(op >> 8) & 1]);

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);
    if (dr) /* left */
        e("shl  dx, byte 1\n");
    else
        e("shr  dx, byte 1\n");
    e("lahf\n");
    e("setc byte [x]\n");
    e("xor  al, al\n");
    WriteWord();    /* write back to mem */
    EmitTiming(base_timing + TimingEA(ea, WORD_SIZE));
    InstEnd();
    return 1;
}

int LSxReg(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    size, t, opr_i, count;
    char        *opr[] =    { "shr", "shl" };
    char        mnem[4];
    char        *mnem_t[] = { "LSR", "LSL" };

    switch ((op >> 6) & 3)
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0;   }

    opr_i = (op >> 8) & 1;
    strcpy(mnem, mnem_t[opr_i]);

    count = (op >> 9) & 7;
    if (!count) count = 8;

    InstBegin(op, mnem, 8); /* 8 regs */
    e("and  edi, byte 7\n");
    if ((op >> 5) & 1)  /* reg contains count */
    {
        e("mov  ebx, ecx\n");   /* save ECX */
        e("mov  ecx, [d+%d*4]\n", (op >> 9) & 7);
        e("and  ecx, byte 0x3f\n");     /* modulo 64 */
        e("mov  edx, [d+edi*4]\n");   /* get register... */
        switch (size)
        {   case BYTE_SIZE: e("%s   dl, cl\n", opr[opr_i]);
                            e("setc al\n"); /* CF -> AL */
                            e("test dl, dl\n");
                            e("mov  [d+edi*4], dl\n");    /* MOV does not
                                                               affect flags */
                            break;  
            case WORD_SIZE: e("%s   dx, cl\n", opr[opr_i]);
                            e("setc al\n");
                            e("test dx, dx\n");
                            e("mov  [d+edi*4], dx\n");
                            break;
            case LONG_SIZE: e("%s   edx, cl\n", opr[opr_i]);
                            e("setc al\n");
                            e("test edx, edx\n");
                            e("mov  [d+edi*4], edx\n");
                            break;
        }
        e("lahf\n");
        e("test cl, cl\n"); /* if zero count... */
        e("jz   short .zero\n");
        e("or   ah, al\n"); /* new CF... the first test cleared old C */
        e("test al, 1\n");  /* C->X */
        e("setnz    byte [x]\n");
        EmitLabel(".zero");
        e("xor  al, al\n"); /* V cleared */
        /* timing */
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE: t = 6; break;
            case LONG_SIZE: t = 8; break;   }
        e("shl  cl, byte 1\n"); /* 2n */
        e("add  cl, byte %d\n", t);
        e("neg  ecx\n");
        e("add  ecx, ebx\n");
        e("js   near Turbo68KRun_done\n");
    }
    else                /* immediate count */
    {
        e("%s   ", opr[opr_i]);
        switch (size)
        {   case BYTE_SIZE: e("byte "); break;
            case WORD_SIZE: e("word "); break;
            case LONG_SIZE: e("dword "); break; }
        e("[d+edi*4], %d\n", count);
        e("lahf\n");
        e("setc byte [x]\n");
        e("xor  al, al\n");
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE: t = 6; break;
            case LONG_SIZE: t = 8; break;   }
        EmitTiming(t + count * 2);
    }
    InstEnd();
    return 1;
}

int NOP(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int CLR(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, size, t;

    if (!CheckEA(ea, "101111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    switch ((op >> 6) & 3)
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0;   }

    InstBegin(op, mnem, NumDecodedEA(ea));
    e("mov  eax, 0x4000\n");    /* Z always set, everything else cleared */

    if (mpu == 68000 && dummyread && (ea >> 3))
    {
        /*
         * Emulate the dummy read for memory addressing modes, if necessary.
         * We only do it for memory modes, because it really can't make any
         * difference if the destination is a register!
         */

        LoadFromEA(ea, size, 0);    /* do the dummy read */
        e("xor  edx, edx\n");       /* now, write a 0 */
        switch (size)               /* write */
        {
        case BYTE_SIZE: WriteByte(); break;
        case WORD_SIZE: WriteWord(); break;
        case LONG_SIZE: WriteLong(); break;
        }
    }
    else
    {
        /*
         * This method is quicker, since dummy reads aren't needed
         */

        e("xor  edx, edx\n");
        StoreToEAUsingEDI(ea, size, 0);
    }

    switch (size)
    {   case BYTE_SIZE:
        case WORD_SIZE:
            if ((ea >> 3) == 0) t = 4;  /* reg */
            else                t = 8 + TimingEA(ea, size); /* mem */
            break;
        case LONG_SIZE:
            if ((ea >> 3) == 0) t = 6;  /* reg */
            else                t = 12 + TimingEA(ea, size); /* mem */
            break;
    }
    EmitTiming(t);
    InstEnd();
    return 1;
}

int ORItoSR(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("test byte [sr+1], 0x20\n");      /* in supervisor? */
    e("jz   near exception_privilege_violation\n");
    SaveCCR();  /* save flags to SR */
    e("mov  edx, [esi]\n");
    e("and  edx, 0xa71f\n");            /* mask out unwanted bits */
    e("or   [sr], dx\n");
    e("add  esi, byte 2\n");
    LoadCCR();  /* get flags back */
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int MOVEfromSR(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f;

    if (!CheckEA(ea, "101111111000"))   return 0;

    InstBegin(op, mnem, 1);
    /*
     * This instruction is not privileged for 68000 and 68008 processors.
     */
    if (mpu == 68010)
    {
        e("test byte [sr+1], 0x20\n");    /* in supervisor? */
        e("jz   near exception_privilege_violation\n");
    }
    SaveCCR();  /* save flags to SR */

    if (mpu == 68000 && dummyread && (ea >> 3))
    {
        /*
         * When dummy reads are enabled, and the EA mode is not register, we
         * perform a dummy read
         */

        LoadFromEA(ea, WORD_SIZE, 0);
        e("mov  edx, [sr]\n");
        WriteWord();
    }
    else
    {
        e("mov  edx, [sr]\n");    /* get SR */
        StoreToEAUsingEDI(ea, WORD_SIZE, 0);
    }

    LoadCCR();  /* get back SR */
    if ((ea >> 3) == 0)        
        EmitTiming(6);
    else
        EmitTiming(8 + TimingEA(ea, WORD_SIZE));
    InstEnd();
    return 1;
}

int ArithmeticA(unsigned short op, char *mnem, unsigned base_timing)
{    
    int     size = UNKNOWN_SIZE, t;

    if (!CheckEA(op & 0x3f, "111111111111"))    /* check EA mode */
        return 0;
    if (!NumDecodedEA(op & 0x3f))               /* check if decoded this EA */
        return 0;
    switch ((op >> 6) & 7)  /* set allowed sizes */
    {   case 3:     size = WORD_SIZE; break;
        case 7:     size = LONG_SIZE; break;
        default:    return 0; }
    if (((op >> 12) & 0xf) != 0xd && ((op >> 12) & 0xf) != 9)   return 0;
         
    InstBegin(op, mnem, NumDecodedEA(op & 0x3f));    
    LoadFromEA(op & 0x3f, size, 1);                 /* load */
    if (((op >> 12) & 0xf) == 0xd)  /* ADDA */
        e("add  [a+%d*4], edx\n", (op >> 9) & 7); /* store */
    else                            /* SUBA */
        e("sub  [a+%d*4], edx\n", (op >> 9) & 7);
    switch (size)
    {   case WORD_SIZE: t = 8; break;
        case LONG_SIZE: t = 6;
                        if (((op & 0x3f) >> 3) == 0) t += 2;    /* Dn */
                        else if (((op & 0x3f) >> 3) == 1) t += 2;   /* An */
                        else if (((op & 0x3f) >> 3) == 7 && (op & 7) == 4)
                            t += 2;                             /* #<data> */
                        break;
    }
    EmitTiming(TimingEA(op & 0x3f, size) + t);
    InstEnd();
    return 1;
}

int ASxMem(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, dr = (op >> 8) & 1;
    char        mnem[4];
    char        *mnem_t[] = { "ASR", "ASL" };

    if (!CheckEA(ea, "001111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    strcpy(mnem, mnem_t[(op >> 8) & 1]);

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);
    if (dr) /* left */
        e("shl  dx, byte 1\n");
    else
        e("sar  dx, byte 1\n");
    e("lahf\n");
    e("setc byte [x]\n");
    if (dr) /* only for left shifts, right shifts don't change the MSB */
    {
        e("sets al\n");         /* AL=current MSB */
        e("adc  al, byte 0\n"); /* CF contains old MSB */
        e("and  al, 1\n");      /* preserve only the flag bit */
    }
    WriteWord();    /* write back to mem */
    EmitTiming(base_timing + TimingEA(ea, WORD_SIZE));
    InstEnd();
    return 1;
}

int ASxReg(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    i, size, t = 0, opr_i, count;
    char        mnem[4];
    char        *mnem_t[] = { "ASR", "ASL" };

    switch ((op >> 6) & 3)
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0;   }

    opr_i = (op >> 8) & 1;
    strcpy(mnem, mnem_t[opr_i]);

    count = (op >> 9) & 7;
    if (!count) count = 8;

    InstBegin(op, mnem, 8); /* 8 regs */

    e("and  edi, byte 7\n");
    if ((op >> 5) & 1)  /* reg contains count */
    {
        if (opr_i)  /* ASL */
        {
            e("xor  al, al\n");
            SaveReg("run", "edi");      /* save EDI */
            e("mov  edx, [d+edi*4]\n"); /* get reg to shift */
            e("mov  edi, [d+%d*4]\n", (op >> 9) & 7);
            e("and  edi, byte 0x3f\n"); /* modulo 64 */
            e("jz   short .zero_shift\n");
            EmitLabel(".loop");
            e("shl  ");
            switch (size)
            {   case BYTE_SIZE: e("dl, "); break;
                case WORD_SIZE: e("dx, "); break;
                case LONG_SIZE: e("edx, "); break;  }
            e("1\n");
            e("setc bl\n");     /* old MSB */
            e("sets bh\n");     /* new MSB */
            e("xor  bh, bl\n"); /* detect change */
            e("or   al, bh\n"); /* reflect change in V */
            e("sub  ecx, byte 2\n");    /* each rotation=2 cycles */
            e("dec  edi\n");
            e("jnz  short .loop\n");
            RestoreReg("run", "edi");
            switch (size)       /* store reg, get flags */
            {   case BYTE_SIZE: e("mov  [d+edi*4], dl\n");
                                e("test dl, dl\n"); break;
                case WORD_SIZE: e("mov  [d+edi*4], dx\n");
                                e("test dx, dx\n"); break;
                case LONG_SIZE: e("mov  [d+edi*4], edx\n");
                                e("test edx, edx\n"); break;
            }
            e("lahf\n");
            e("or   ah, bl\n");     /* CF */
            e("test bl, bl\n");
            e("setnz byte [x]\n");  /* CF==X */
            e("jmp  short .end\n");

            EmitLabel(".zero_shift");
            switch (size)
            {   case BYTE_SIZE: e("test dl, dl\n"); break;
                case WORD_SIZE: e("test dx, dx\n"); break;
                case LONG_SIZE: e("test edx, edx\n"); break;
            }
            e("lahf\n");
            e("xor  al, al\n");

            EmitLabel(".end");            
            switch (size)
            {   case BYTE_SIZE:
                case WORD_SIZE: t = 6; break;
                case LONG_SIZE: t = 8; break;   }
            EmitTiming(t);
        }
        else        /* ASR */
        {
            e("mov  ebx, ecx\n");   /* save ECX */
            e("mov  ecx, [d+%d*4]\n", (op >> 9) & 7);
            e("and  ecx, byte 0x3f\n"); /* modulo 64 */
            switch (size)
            {   case BYTE_SIZE: e("mov  dl, [d+edi*4]\n");
                                e("sar  dl, cl\n");
                                e("setc bl\n");
                                e("test dl, dl\n");
                                break;
                case WORD_SIZE: e("mov  edx, [d+edi*4]\n");
                                e("sar  dx, cl\n");
                                e("setc bl\n");
                                e("test dx, dx\n");
                                break;
                case LONG_SIZE: e("mov  edx, [d+edi*4]\n");
                                e("sar  edx, cl\n");
                                e("setc bl\n");      /* get CF from SHL/SHR */
                                e("test edx, edx\n");/* this trashes the CF */
                                break;
            }
                
            e("lahf\n");
            e("test cl, cl\n"); /* if shift count of 0, X is unaffected, C=0 */
            e("jz   short .no_x\n");
            e("or   ah, bl\n");     /* CF! */
            e("test     ah, 1\n");  /* CF */
            e("setnz    byte [x]\n");
            EmitLabel(".no_x");
            switch (size)   /* write back and detect MSB change */
            {   case BYTE_SIZE: e("mov  [d+edi*4], dl\n");
                                break;
                case WORD_SIZE: e("mov  [d+edi*4], dx\n");
                                break;
                case LONG_SIZE: e("mov  [d+edi*4], edx\n");
                                break;
            }
            e("xor  al, al\n"); /* V=0 because MSB never changes w/ ASR */
            /* timing */
            switch (size)
            {   case BYTE_SIZE:
                case WORD_SIZE: t = 6; break;
                case LONG_SIZE: t = 8; break;   }
            e("shl  cl, byte 1\n"); /* 2n */
            e("add  cl, byte %d\n", t);
            e("neg  ecx\n");
            e("add  ecx, ebx\n");
            e("js   near Turbo68KRun_done\n");
        }
    }
    else                /* immediate count */
    {
        if (!opr_i) /* ASR */
        {
            e("sar  ");
            switch (size)
            {   case BYTE_SIZE: e("byte "); break;
                case WORD_SIZE: e("word "); break;
                case LONG_SIZE: e("dword "); break; }
            e("[d+edi*4], %d\n", count);
            e("lahf\n");
            e("setc byte [x]\n");
            e("xor  al, al\n");
            t = count * 2;  /* 2 cycles per shift */
        }
        else        /* ASL */
        {
            e("xor  al, al\n"); /* clear V */
            switch (size)
            {   case BYTE_SIZE: e("mov  dl, [d+edi*4]\n"); break;
                case WORD_SIZE: e("mov  dx, [d+edi*4]\n"); break;
                case LONG_SIZE: e("mov  edx, [d+edi*4]\n"); break;    }
            for (i = 1; i <= count; i++)
            {
                e("shl  ");
                switch (size)
                {   case BYTE_SIZE: e("dl, "); break;
                    case WORD_SIZE: e("dx, "); break;
                    case LONG_SIZE: e("edx, "); break;  }
                e("1\n");
                if (i == count)
                {
                    e("lahf\n");
                    e("setc byte [x]\n");
                }
                e("setc bl\n");     /* old MSB */
                e("sets bh\n");     /* new MSB */
                e("xor  bh, bl\n"); /* detect change */
                e("or   al, bh\n"); /* reflect change in V */
                e("sub  ecx, byte 2\n");    /* each rotation=2 cycles */
            }
            switch (size)   /* write results back to Dn */
            {   case BYTE_SIZE: e("mov  [d+edi*4], dl\n"); break;
                case WORD_SIZE: e("mov  [d+edi*4], dx\n"); break;
                case LONG_SIZE: e("mov  [d+edi*4], edx\n"); break;    }
        }
        switch (size)
        {   case BYTE_SIZE:
            case WORD_SIZE: t += 6; break;
            case LONG_SIZE: t += 8; break;  }
        EmitTiming(t);
    }
    InstEnd();
    return 1;
}

int ArithmeticQ(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, size, t, data, opr_i;
    char        mnem[5];
    char        *mnem_t[] = { "ADDQ", "SUBQ" };
    char        *opr[] =    { "add", "sub" };

    if (!CheckEA(ea, "111111111000"))   return 0;
    t = base_timing;
    switch ((op >> 6) & 3)
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; t += 4; break;
        default:    return 0;   }
    if ((ea >> 3) != 0 && (ea >> 3) != 1)   t += 4; /* mem */
    if (size == BYTE_SIZE && (ea >> 3) == 1)        /* byte An not allowed */
       return 0;
    if (!NumDecodedEA(ea))              return 0;
    data = (op >> 9) & 7;
    if (!data)  data = 8;

    strcpy(mnem, mnem_t[(op >> 8) & 1]);
    opr_i = (op >> 8) & 1;

    InstBegin(op, mnem, NumDecodedEA(ea));
    if ((ea >> 3) == 0)         /* Dn */
    {
        e("and  edi, byte 7\n");
        switch (size)
        {   case BYTE_SIZE:
                e("%s   byte [d+edi*4], %d\n", opr[opr_i], data);
                break;
            case WORD_SIZE:
                e("%s   word [d+edi*4], %d\n", opr[opr_i], data);
                break;
            case LONG_SIZE:
                e("%s   dword [d+edi*4], byte %d\n", opr[opr_i], data);
                break;
        }
        e("lahf\n");
        e("seto al\n");
        e("setc byte [x]\n");
    }
    else if ((ea >> 3) == 1)    /* An */
    {
        /*
         * Manual says the entire address register is used despite the size
         * of the operation.
         */
        e("and  edi, byte 7\n");
        e("%s   dword [a+edi*4], byte %d\n", opr[opr_i], data);
    }
    else                        /* memory */
    {
        LoadFromEA(ea, size, 0);
        switch (size)
        {   case BYTE_SIZE: e("%s   dl, byte %d\n", opr[opr_i], data); break;
            case WORD_SIZE: e("%s   dx, %d\n", opr[opr_i], data); break;
            case LONG_SIZE: e("%s   edx, byte %d\n", opr[opr_i], data); break;  }
        e("lahf\n");
        e("seto al\n");
        e("setc byte [x]\n");
        switch (size)
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break; }
    }
    EmitTiming(t + TimingEA(ea, size));
    InstEnd();
    return 1;
}              

int NEGx(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    ea = op & 0x3f, size, t;
    char        mnem[5];
    char        *mnem_t[] = { "NEGX", "NEG" };

    if (!CheckEA(ea, "101111111000"))   return 0;
    if (!NumDecodedEA(ea))              return 0;
    switch ((op >> 6) & 3)
    {   case 0: size = BYTE_SIZE; break;
        case 1: size = WORD_SIZE; break;
        case 2: size = LONG_SIZE; break;
        default: return 0;  }

    strcpy(mnem, mnem_t[(op >> 10) & 1]);

    InstBegin(op, mnem, NumDecodedEA(ea));
    if ((ea >> 3) == 0) /* Dn */
    {
        e("and  edi, byte 7\n");
        if (!(op & 0x0400)) /* NEGX */
        {
            switch (size)
            {   case BYTE_SIZE: e("xor  dl, dl\n"); break;
                case WORD_SIZE: case LONG_SIZE:
                                e("xor  edx, edx\n"); break;
            }
            e("shr  byte [x], 1\n");  /* X->CF */
            switch (size)
            {   case BYTE_SIZE: e("sbb  dl, [d+edi*4]\n");
                                e("mov  bl, ah\n"); /* store old flags */
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                e("jnz  short .clr\n"); /* Z clear */
                                e("and  bl, byte 0x40\n"); /* isolate old Z */
                                e("and  ah, byte 0xbf\n"); /* kill new Z */
                                e("or   ah, bl\n");        /* in w/ old Z */
                                EmitLabel(".clr");
                                e("mov  [d+edi*4], dl\n"); t = 4; break;
                case WORD_SIZE: e("sbb  dx, [d+edi*4]\n");
                                e("mov  bl, ah\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                e("jnz  short .clr\n");
                                e("and  bl, byte 0x40\n");
                                e("and  ah, byte 0xbf\n");
                                e("or   ah, bl\n");
                                EmitLabel(".clr");
                                e("mov  [d+edi*4], dx\n"); t = 4; break;
                case LONG_SIZE: e("sbb  edx, [d+edi*4]\n");
                                e("mov  bl, ah\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                e("jnz  short .clr\n");
                                e("and  bl, byte 0x40\n");
                                e("and  ah, byte 0xbf\n");
                                e("or   ah, bl\n");
                                EmitLabel(".clr");
                                e("mov  [d+edi*4], edx\n"); t = 6; break; }
        }
        else                /* NEG */
        {
            switch (size)
            {   case BYTE_SIZE: e("neg  byte [d+edi*4]\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                t = 4; break;
                case WORD_SIZE: e("neg  word [d+edi*4]\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                t = 4; break;
                case LONG_SIZE: e("neg  dword [d+edi*4]\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                t = 6; break;
            }
        }
    }
    else                /* memory */
    {
        if (!(op & 0x0400)) /* NEGX */
        {
            LoadFromEA(ea, size, 0);
            e("mov  edi, ebx\n");   /* save address where data came from */
            switch (size)
            {   case BYTE_SIZE: e("xor  bl, bl\n"); break;
                case WORD_SIZE: case LONG_SIZE:
                                e("xor  ebx, ebx\n"); break;
            }
            e("shr  byte [x], 1\n");  /* X->CF */
            switch (size)
            {   case BYTE_SIZE: e("sbb  bl, dl\n");
                                e("mov  dl, ah\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                e("jnz  short .clr\n");
                                e("and  dl, byte 0x40\n");
                                e("and  ah, byte 0xbf\n");
                                e("or   ah, dl\n");
                                EmitLabel(".clr");
                                break;
                case WORD_SIZE: e("sbb  bx, dx\n");
                                e("mov  dl, ah\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                e("jnz  short .clr\n");
                                e("and  dl, byte 0x40\n");
                                e("and  ah, byte 0xbf\n");
                                e("or   ah, dl\n");
                                EmitLabel(".clr");
                                break;
                case LONG_SIZE: e("sbb  ebx, edx\n");
                                e("mov  dl, ah\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                e("jnz  short .clr\n");
                                e("and  dl, byte 0x40\n");
                                e("and  ah, byte 0xbf\n");
                                e("or   ah, dl\n");
                                EmitLabel(".clr");
                                break;    }
            e("mov  edx, ebx\n");
            e("mov  ebx, edi\n");
            /* WriteXXX will clear EDI for us */
            switch (size)
            {   case BYTE_SIZE: WriteByte(); t = 8; break;
                case WORD_SIZE: WriteWord(); t = 8; break;
                case LONG_SIZE: WriteLong(); t = 12; break; }
            t += TimingEA(ea, size);
        }
        else                /* NEG */
        {
            LoadFromEA(ea, size, 0);
            switch (size)
            {   case BYTE_SIZE: e("neg  dl\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                WriteByte(); t = 8; break;
                case WORD_SIZE: e("neg  dx\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                WriteWord(); t = 8; break;
                case LONG_SIZE: e("neg  edx\n");
                                e("lahf\n");
                                e("seto al\n");
                                e("setc byte [x]\n");
                                WriteLong(); t = 12; break; }
            t += TimingEA(ea, size);
        }
    }
    EmitTiming(t);
    InstEnd();
    return 1;
}
   
int RTS(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 1);
    e("mov  ebx, [a+7*4]\n");
    ReadLong();
    e("add  dword [a+7*4], byte 4\n");
    e("mov  esi, edx\n");
    UpdateFetchPtr();
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int MOVEtoSR(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    ea = op & 0x3f;

    if (!CheckEA(ea, "101111111111"))   return 0;
    if (!NumDecodedEA(ea))              return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));
    LoadFromEA(ea, WORD_SIZE, 0);
    e("test byte [sr+1], 0x20\n");      /* in supervisor? */
    e("jz   near exception_privilege_violation\n");
    e("test dh, 0x20\n");               /* if still in supervisor... */
    e("jnz  .still_s\n");               /* ...no changes need to be made */
    e("mov  ebx, [__sp]\n");            /* swap SSP and USP */
    e("xchg [a+7*4], ebx\n");
    e("mov  [__sp], ebx\n");
    SetUserAddressSpace();              /* map in user address space */
    EmitLabel(".still_s");
    e("and  edx, 0xa71f\n");            /* mask out unwanted bits */
    e("mov  [sr], edx\n");              /* load new SR */
    LoadCCR();  /* reload flags we changed in CCR */
    EmitTiming(base_timing + TimingEA(ea, WORD_SIZE));
    InstEnd();
    return 1;
}

int DBcc(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    cond = (op >> 8) & 0xf;
    char        mnem[5];
    char        *mnem_t[] = {  "DBT",  "DBF",  "DBHI", "DBLS",
                               "DBCC", "DBCS", "DBNE", "DBEQ",
                               "DBVC", "DBVS", "DBPL", "DBMI",
                               "DBGE", "DBLT", "DBGT", "DBLE" };

    strcpy(mnem, mnem_t[cond]);             /* set mnemonic */

    InstBegin(op, mnem, 8);
    switch (cond)
    {
    /* EAX:0000000000000000 NZ00 000C 0000 000V */
    case 0:     /* always true */
        e("add  esi, byte 2\n");
        EmitTiming(10);
        InstEnd();
        return 1;
    case 1:     /* DBF/DBRA: no condition to test, just loop */
        break;
    case 2:     /* HI: !C & !Z */
        e("test ah, byte 0x41\n");
        e("jz   short .do\n");
        break;
    case 3:     /* LS: C | Z */
        e("test ah, byte 0x41\n");
        e("jnz  short .do\n");
        break;
    case 4:     /* CC: !C */
        e("test ah, byte 1\n");
        e("jz   short .do\n");
        break;
    case 5:     /* CS: C */
        e("test ah, byte 1\n");
        e("jnz  short .do\n");
        break;
    case 6:     /* NE: !Z */
        e("test ah, byte 0x40\n");
        e("jz   short .do\n");
        break;
    case 7:     /* EQ: Z */
        e("test ah, byte 0x40\n");
        e("jnz  short .do\n");
        break;
    case 8:     /* VC: !V */
        e("test al, byte 1\n");
        e("jz   short .do\n");
        break;
    case 9:     /* VS: V */
        e("test al, byte 1\n");
        e("jnz  short .do\n");
        break;
    case 0xa:   /* PL: !N */
        e("test ah, byte 0x80\n");
        e("jz   short .do\n");
        break;
    case 0xb:   /* MI: N */
        e("test ah, byte 0x80\n");
        e("jnz  short .do\n");
        break;
    case 0xc:   /* GE: N & V | !N & !V      (N&V || !N&!V) */
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpe  short .do\n");        
        break;
    case 0xd:   /* LT: N & !V | !N & V */
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpo  short .do\n");
        EmitLabel(".dont_do");
        break;
    case 0xe:   /* GT: N & V & !Z | !N & !V & !Z */
        /* if Z, then it is GE and we don't take this branch */
        e("test ah, byte 0x40\n");
        e("jnz  short .dont_do\n");
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpe  short .do\n");        
        EmitLabel(".dont_do");
        break;
    case 0xf:   /* LE: Z | N & !V | !N & V */
        e("test ah, 0x40\n");
        e("jnz  short .do\n");
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpo  short .do\n");
        break;
    }        
    e("and  edi, byte 7\n");/* condition not met, decrement and branch */
    if (cond == 1 && skip)  /* idle loop skipping w/ DBRA */
        e("mov  edx, esi\n");
    e("sub  word [d+edi*4], 1\n");
    e("jc   short .do_loop_expired\n"); /* loop expired... */
    e("movsx    ebx, word [esi]\n");
    e("add      esi, ebx\n");
    if (brafetch)
    {
        if (skip)               /* skip uses EDX, we must save it */
            SaveReg("run", "edx");
        e("sub  esi, ebp\n");   /* ESI=denormalized PC */
        UpdateFetchPtr();
        if (skip)
            RestoreReg("run", "edx");
    }
    if (cond == 1 && skip)
    {
        e("sub  edx, byte 2\n");        /* to get address of instruction */
        e("cmp  edx, esi\n");
        e("je   short .skip\n");        /* the same: this is an idle loop */
    }
    EmitTiming(10);
    InstEnd();
    Align(4);
    EmitLabel(".do");   /* condition was met, go on to next instruction */
    e("add  esi, byte 2\n");
    EmitTiming(12);
    InstEnd();
    Align(4);
    EmitLabel(".do_loop_expired");  /* condition false, but loop expired */
    e("add  esi, byte 2\n");
    EmitTiming(14);
    InstEnd();
    if (cond == 1 && skip)
    {
        /*
         * This code makes it look as if the loop was executed and expired.
         */
        EmitLabel(".skip");
        e("add  edx, byte 4\n");
        e("mov  esi, edx\n");       /* EDX contained PC for DBRA */
        e("mov  edx, [d+edi*4]\n"); /* get counter loop */
        e("and  edx, 0xffff\n");
        e("inc  edx\n");            /* we subtracted 1 a little ways above */
        e("mov  word [d+edi*4], 0xffff\n");
        e("mov  ebx, edx\n");
        e("shl  edx, byte 3\n");    /* (i << 3) + (i << 2) = i*10 */
        e("shl  ebx, byte 2\n");
        e("sub  ecx, edx\n");
        e("sub  ecx, ebx\n");
        e("js   near Turbo68KRun_done\n");
        InstEnd();
    }
    return 1;
}

int MOVEM(unsigned short op, char *mnem, unsigned base_timing)
{
    /*
     * If a non-post/pre-inc/decrement mode is used, the registers are
     * transferred to the address, and that address is inc/decremented but the
     * change is NOT reflected in the address register.
     *
     * If the mode is pre-dec or post-inc, it IS reflected. First, load up the
     * value of the register, and then write it back and the end, but don't
     * modify it while performing the operation, use a temp. X86 register
     * instead.
     *
     * NOTE: The reg->mem mode should be fine, I've tested it -- but there
     * might be some bugs.
     *
     * NOTE (April 8, 2001): Pete Dabbs just emailed me to report a problem
     * with MOVEM mem->reg. Basically, code like this wasn't working properly:
     *
     *      MOVEM.L (A7)+,A7
     *
     * What should happen there is the data from (A7) is read. Then, 4 is added
     * to A7, and finally, the data is written to A7. What Turbo68K was doing
     * wrong was it was writing the data from (A7) to A7 and THEN incrementing
     * A7. Not sure if the fix is correct, but I hope it is... I didn't change
     * anything in MOVEM reg->mem...
     */

    int         dr = (op >> 10) & 1, size = UNKNOWN_SIZE;
    unsigned    ea = op & 0x3f;

    switch ((op >> 6) & 1)
    {   case 0:     size = WORD_SIZE; break;
        case 1:     size = LONG_SIZE; break;
        default:    return 0;   }
    if (!NumDecodedEA(ea))              return 0;
    if (!dr)    /* reg->mem */
    {
        if (!CheckEA(ea, "001011111000"))   return 0;
    }
    else        /* mem->reg */
        if (!CheckEA(ea, "001101111011"))   return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));

    if (!dr)    /* reg->mem */
    {
        SaveReg("run", "eax");  /* save flags */
        e("mov  eax, [esi]\n"); /* AX=register list mask */
        e("add  esi, byte 2\n");
        LoadControlEA(ea);      /* EBX=store address */
        SaveReg("run", "esi");
        if ((ea >> 3) == 4)         /* write back if (An)+,-(An) */
            SaveReg("run", "edi");  /* EDI may contain reg # */
        if ((ea >> 3) == 4)         /* -(An) has reg mask reversed */
            e("mov  esi, d+15*4\n");  /* start from top and decrement */
        else
            e("mov  esi, d\n");
        EmitLabel(".loop");
        if (size == WORD_SIZE)
            e("sub  ecx, byte 4\n");
        else
            e("sub  ecx, byte 8\n");
        e("shr  eax, byte 1\n");    /* mask bit->CF */
        e("jnc  short .skip\n");    /* not 1? skip this reg */

        if ((ea >> 3) == 4) /* -(An) */
        {
            if (size == WORD_SIZE)  e("sub  ebx, byte 2\n");    /* predec */
            else                    e("sub  ebx, byte 4\n");
        }

        e("mov  edx, [esi]\n"); /* get reg to store... */
        SaveReg("run", "ebx");
        if (size == WORD_SIZE)  WriteWord(); else  WriteLong(); /* get mem */
        RestoreReg("run", "ebx");

        if (!((ea >> 3) == 4))      /* not predec, increment */
        {
            if (size == WORD_SIZE)  e("add  ebx, byte 2\n");
            else                    e("add  ebx, byte 4\n");
        }

        EmitLabel(".skip");
        if ((ea >> 3) == 4)         /* -(An) */
        {
            e("sub  esi, byte 4\n");    /* next reg down */
            e("cmp  esi, d-4\n");     /* done? */
            e("jne  short .loop\n");    /* not done, keep looping */       
        }
        else
        {
            e("add  esi, byte 4\n");    /* next reg up */
            e("cmp  esi, d+16*4\n");  /* done? */
            e("jne  short .loop\n");    /* not done, keep looping */
        }

        if ((ea >> 3) == 4)   /* write back if -(An) */
        {
            RestoreReg("run", "edi");           
            e("mov  [a+edi*4], ebx\n");
        }
        RestoreReg("run", "esi");
        RestoreReg("run", "eax");

        switch (ea >> 3)    /* timing for address mode */
        {   case 2: base_timing = 8; break;     /* (An) */
            case 4: base_timing = 8; break;     /* -(An) */
            case 5: base_timing = 12; break;    /* (d16,An) */
            case 6: base_timing = 14; break;    /* (d8,An,Xn) */
            case 7:
                switch (ea & 7)
                {   case 0: base_timing = 12; break;    /* (xxx).W */
                    case 1: base_timing = 16; break;    /* (xxx).L */
                } break;
        }
    }
    else        /* mem->reg */
    {
        SaveReg("run", "eax");  /* save flags */
        e("mov  eax, [esi]\n"); /* AX=register list mask */
        e("add  esi, byte 2\n");
        LoadControlEA(ea);      /* EBX=load address */
        SaveReg("run", "esi");
        if ((ea >> 3) == 3)         /* write back if (An)+,-(An) */
            SaveReg("run", "edi");  /* EDI may contain reg # */
        e("mov  esi, d\n");       /* start from bottom and increment */
        EmitLabel(".loop");
        if (size == WORD_SIZE)
            e("sub  ecx, byte 4\n");
        else
            e("sub  ecx, byte 8\n");
        e("shr  eax, byte 1\n");    /* mask bit->CF */
        e("jnc  short .skip\n");    /* not 1? skip this reg */

        /* if word, sign extend to long */
        if (size == WORD_SIZE)
        {
            if ((ea >> 3) == 7 && ((ea & 7) == 2 || (ea & 7) == 3))
                pcfetch ? ReadWordSXPC() : ReadWordSX();    /* PC-relative is special */
            else
                ReadWordSX();
        }
        else
        {
            if ((ea >> 3) == 7 && ((ea & 7) == 2 || (ea & 7) == 3))
                pcfetch ? ReadLongPC() : ReadLong();
            else
                ReadLong();
        }

        if (size == WORD_SIZE)  e("add  ebx, byte 2\n");
        else                    e("add  ebx, byte 4\n");
        if ((ea >> 3) == 3)
        {
            RestoreReg("run", "edi");
            e("mov  [a+edi*4], ebx\n");
            SaveReg("run", "edi");
        }            

        e("mov  [esi], edx\n"); /* store to reg */

        EmitLabel(".skip");
        e("add  esi, byte 4\n");    /* next reg up */
        e("cmp  esi, d+16*4\n");  /* done? */
        e("jne  short .loop\n");    /* not done, keep looping */       

        if ((ea >> 3) == 3)   /* just in case SaveReg()/RestoreReg() use stack */
            RestoreReg("run", "edi");

        RestoreReg("run", "esi");
        RestoreReg("run", "eax");

        switch (ea >> 3)    /* timing for address mode */
        {   case 2: base_timing = 12; break;    /* (An) */
            case 3: base_timing = 12; break;    /* (An)+ */
            case 5: base_timing = 16; break;    /* (d16,An) */
            case 6: base_timing = 18; break;    /* (d8,An,Xn) */
            case 7:
                switch (ea & 7)
                {   case 0: base_timing = 16; break;    /* (xxx).W */
                    case 1: base_timing = 20; break;    /* (xxx).L */
                    case 2: base_timing = 16; break;    /* (d16,PC) */
                    case 3: base_timing = 18; break;    /* (d16,PC,Xn */
                } break;
        }
    }
   
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

/* In case I screwed up MOVEM(), I have a back-up right here ;) */

int OldMOVEM(unsigned short op, char *mnem, unsigned base_timing)
{
    /*
     * If a non-post/pre-inc/decrement mode is used, the registers are
     * transferred to the address, and that address is inc/decremented but the
     * change is NOT reflected in the address register.
     *
     * If the mode is pre-dec or post-inc, it IS reflected. First, load up the
     * value of the register, and then write it back and the end, but don't
     * modify it while performing the operation, use a temp. X86 register
     * instead.
     *
     * NOTE: The reg->mem mode should be fine, I've tested it -- but there
     * might be some bugs.
     */

    int         dr = (op >> 10) & 1, size = UNKNOWN_SIZE;
    unsigned    ea = op & 0x3f;

    switch ((op >> 6) & 1)
    {   case 0:     size = WORD_SIZE; break;
        case 1:     size = LONG_SIZE; break;
        default:    return 0;   }
    if (!NumDecodedEA(ea))              return 0;
    if (!dr)    /* reg->mem */
    {
        if (!CheckEA(ea, "001011111000"))   return 0;
    }
    else        /* mem->reg */
        if (!CheckEA(ea, "001101111011"))   return 0;

    InstBegin(op, mnem, NumDecodedEA(ea));

    if (!dr)    /* reg->mem */
    {
        SaveReg("run", "eax");  /* save flags */
        e("mov  eax, [esi]\n"); /* AX=register list mask */
        e("add  esi, byte 2\n");
        LoadControlEA(ea);      /* EBX=store address */
        SaveReg("run", "esi");
        if ((ea >> 3) == 4)         /* write back if (An)+,-(An) */
            SaveReg("run", "edi");  /* EDI may contain reg # */
        if ((ea >> 3) == 4)         /* -(An) has reg mask reversed */
            e("mov  esi, d+15*4\n");  /* start from top and decrement */
        else
            e("mov  esi, d\n");
        EmitLabel(".loop");
        if (size == WORD_SIZE)
            e("sub  ecx, byte 4\n");
        else
            e("sub  ecx, byte 8\n");
        e("shr  eax, byte 1\n");    /* mask bit->CF */
        e("jnc  short .skip\n");    /* not 1? skip this reg */

        if ((ea >> 3) == 4) /* -(An) */
        {
            if (size == WORD_SIZE)  e("sub  ebx, byte 2\n");    /* predec */
            else                    e("sub  ebx, byte 4\n");
        }

        e("mov  edx, [esi]\n"); /* get reg to store... */
        SaveReg("run", "ebx");
        if (size == WORD_SIZE)  WriteWord(); else  WriteLong(); /* get mem */
        RestoreReg("run", "ebx");

        if (!((ea >> 3) == 4))      /* not predec, increment */
        {
            if (size == WORD_SIZE)  e("add  ebx, byte 2\n");
            else                    e("add  ebx, byte 4\n");
        }

        EmitLabel(".skip");
        if ((ea >> 3) == 4)         /* -(An) */
        {
            e("sub  esi, byte 4\n");    /* next reg down */
            e("cmp  esi, d-4\n");     /* done? */
            e("jne  short .loop\n");    /* not done, keep looping */       
        }
        else
        {
            e("add  esi, byte 4\n");    /* next reg up */
            e("cmp  esi, d+16*4\n");  /* done? */
            e("jne  short .loop\n");    /* not done, keep looping */
        }

        if ((ea >> 3) == 4)   /* write back if -(An) */
        {
            RestoreReg("run", "edi");           
            e("mov  [a+edi*4], ebx\n");
        }
        RestoreReg("run", "esi");
        RestoreReg("run", "eax");

        switch (ea >> 3)    /* timing for address mode */
        {   case 2: base_timing = 8; break;     /* (An) */
            case 4: base_timing = 8; break;     /* -(An) */
            case 5: base_timing = 12; break;    /* (d16,An) */
            case 6: base_timing = 14; break;    /* (d8,An,Xn) */
            case 7:
                switch (ea & 7)
                {   case 0: base_timing = 12; break;    /* (xxx).W */
                    case 1: base_timing = 16; break;    /* (xxx).L */
                } break;
        }
    }
    else        /* mem->reg */
    {
        SaveReg("run", "eax");  /* save flags */
        e("mov  eax, [esi]\n"); /* AX=register list mask */
        e("add  esi, byte 2\n");
        LoadControlEA(ea);      /* EBX=load address */
        SaveReg("run", "esi");
        if ((ea >> 3) == 3)         /* write back if (An)+,-(An) */
            SaveReg("run", "edi");  /* EDI may contain reg # */
        e("mov  esi, d\n");       /* start from bottom and increment */
        EmitLabel(".loop");
        if (size == WORD_SIZE)
            e("sub  ecx, byte 4\n");
        else
            e("sub  ecx, byte 8\n");
        e("shr  eax, byte 1\n");    /* mask bit->CF */
        e("jnc  short .skip\n");    /* not 1? skip this reg */

        /* if word, sign extend to long */
        if (size == WORD_SIZE)
        {
            if ((ea >> 3) == 7 && ((ea & 7) == 2 || (ea & 7) == 3))
                pcfetch ? ReadWordSXPC() : ReadWordSX();    /* PC-relative is special */
            else
                ReadWordSX();
        }
        else
        {
            if ((ea >> 3) == 7 && ((ea & 7) == 2 || (ea & 7) == 3))
                pcfetch ? ReadLongPC() : ReadLong();
            else
                ReadLong();
        }

        e("mov  [esi], edx\n"); /* store to reg */

        if (size == WORD_SIZE)  e("add  ebx, byte 2\n");
        else                    e("add  ebx, byte 4\n");

        EmitLabel(".skip");
        e("add  esi, byte 4\n");    /* next reg up */
        e("cmp  esi, d+16*4\n");  /* done? */
        e("jne  short .loop\n");    /* not done, keep looping */       

        if ((ea >> 3) == 3)   /* write back if (An)+ */
        {
            RestoreReg("run", "edi");           
            e("mov  [a+edi*4], ebx\n");
        }
        RestoreReg("run", "esi");
        RestoreReg("run", "eax");

        switch (ea >> 3)    /* timing for address mode */
        {   case 2: base_timing = 12; break;    /* (An) */
            case 3: base_timing = 12; break;    /* (An)+ */
            case 5: base_timing = 16; break;    /* (d16,An) */
            case 6: base_timing = 18; break;    /* (d8,An,Xn) */
            case 7:
                switch (ea & 7)
                {   case 0: base_timing = 16; break;    /* (xxx).W */
                    case 1: base_timing = 20; break;    /* (xxx).L */
                    case 2: base_timing = 16; break;    /* (d16,PC) */
                    case 3: base_timing = 18; break;    /* (d16,PC,Xn */
                } break;
        }
    }
   
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int MOVEUSP(unsigned short op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 8); /* handles all 8 possible An regs */
    e("and      edi, byte 7\n");
    e("test     byte [sr+1], 0x20\n");
    e("jz       near exception_privilege_violation\n");
    if (op & 8) /* USP->An */
    {
        e("mov  edx, [__sp]\n");
        e("mov  [a+edi*4], edx\n");
    }
    else        /* An->USP */
    {
        e("mov  edx, [a+edi*4]\n");
        e("mov  [__sp], edx\n");
    }
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int Jxx(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    t;
    if (!CheckEA(op & 0x3f, "001001111011"))    return 0;
    if (!NumDecodedEA(op & 0x3f))               return 0;
    InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
    LoadControlEA(op & 0x3f);

    if ((op & 0xffc0) == 0x4e80)    /* JSR */
    {
        e("mov  edx, esi\n");   /* EDX=PC to save to stack */
        e("sub  edx, ebp\n");
        SaveReg("run", "edx");
    }

    e("mov  esi, ebx\n");
    UpdateFetchPtr();
    if ((op & 0xffc0) == 0x4e80)    /* JSR stuff again... */
    {
        RestoreReg("run", "edx");   /* remember? EDX contains PC to save */
        e("sub  dword [a+7*4], byte 4\n");    /* SP - 4 -> SP */
        e("mov  ebx, [a+7*4]\n");
        WriteLong();                            /* PC -> (SP) */
    }        

    switch ((op >> 3) & 7)  /* timing based on control addressing mode */
    {   case 2: t = 8; break;   /* (An) */
        case 5: t = 10; break;  /* (d16,An) */
        case 6: t = 14; break;  /* (d8,An,Xn) */
        case 7:
            switch (op & 7)
            {   case 0: t = 10; break;  /* (xxx).W */
                case 1: t = 12; break;  /* (xxx).L */
                case 2: t = 10; break;  /* (d16,pc) */
                case 3: t = 14; break;  /* (d8,pc,Xn) */
            } break;
    }
    if ((op & 0xffc0) == 0x4e80)    t += 8; /* JSR */
    EmitTiming(t);
    InstEnd();
    return 1;
}

int LEA(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    t;
    if (!CheckEA(op & 0x3f, "001001111011"))    return 0;
    if (!NumDecodedEA(op & 0x3f))               return 0;
    InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
    LoadControlEA(op & 0x3f);
    e("mov  [a+%d*4], ebx\n", (op >> 9) & 7);
    switch ((op >> 3) & 7)  /* timing based on control addressing mode */
    {   case 2: t = 4; break;   /* (An) */
        case 5: t = 8; break;   /* (d16,An) */
        case 6: t = 12; break;  /* (d8,An,Xn) */
        case 7:
            switch (op & 7)
            {   case 0: t = 8; break;   /* (xxx).W */
                case 1: t = 12; break;  /* (xxx).L */
                case 2: t = 8; break;   /* (d16,pc) */
                case 3: t = 12; break;  /* (d8,pc,Xn) */
            } break;
    }
    EmitTiming(t);
    InstEnd();
    return 1;
}

int BTST(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    t;

    if (((op >> 6) & 7) != 4 && ((op >> 6) & 7) != 0)   return 0;
    if (op & 0x100)                 /* dynamic */
    {    
        if (!CheckEA(op & 0x3f, "101111111111"))    return 0;
        if (!NumDecodedEA(op & 0x3f))               return 0;
        InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
        if (((op >> 3) & 7) == 0)   /* Dn is long */
        {
            t = 6;
            e("and  edi, byte 7\n");
            e("mov  ebx, [d+%d*4]\n", (op >> 9) & 7); /* bit # */
            e("and  ebx, byte 31\n");           /* same as modulo 32 */
            e("bt   dword [d+edi*4], ebx\n"); /* bit->CF */
        }
        else                        /* mem is byte */
        {
            t = 4;
            LoadFromEA(op & 0x3f, BYTE_SIZE, 0);
            e("mov  ebx, [d+%d*4]\n", (op >> 9) & 7); /* bit # */
            e("and  ebx, byte 7\n");            /* same as modulo 8 */
            e("bt   edx, ebx\n");               /* bit->CF */
        }
    }
    else if (!(op & 0x100))         /* static */
    {
        if (!CheckEA(op & 0x3f, "101111111011"))    return 0;
        if (!NumDecodedEA(op & 0x3f))               return 0;
        InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
        if (((op >> 3) & 7) == 0)   /* Dn is long */
        {
            t = 10;
            e("and  edi, byte 7\n");    /* isolate Dn reg # */
            e("mov  ebx, [esi]\n");     /* get bit # */
            e("and  ebx, byte 31\n");   /* same as modulo 32 */
            e("add  esi, byte 2\n");
            e("bt   dword [d+edi*4], ebx\n"); /* bit->CF */
        }
        else                        /* mem is byte */
        {
            t = 8;
            e("mov  ebx, [esi]\n");     /* get bit # */
            e("and  ebx, byte 7\n");    /* same as modulo 8 */
            e("add  esi, byte 2\n");
            SaveReg("run", "ebx");
            LoadFromEA(op & 0x3f, BYTE_SIZE, 0);
            RestoreReg("run", "ebx");
            e("bt   edx, ebx\n");               /* bit->CF */
        }
    }
    e("setnc bl\n");        /* zero status of C->DH */
    e("and  ah, 0xbf\n");   /* get rid of old Z */
    e("shl  bl, 6\n");      /* put C in Z position */
    e("or   ah, bl\n");     /* set the new Z */

    if (((op & 0x3f) & 7) == 0) /* Dn is long */
        EmitTiming(t);
    else                        /* mem is byte */
        EmitTiming(t + TimingEA(op & 0x3f, BYTE_SIZE));        
    InstEnd();
    return 1;
}

int TST(unsigned short op, char *mnem, unsigned base_timing)
{
    int size;

    switch ((op >> 6) & 3)  /* set valid sizes */
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0;   }
    if (!NumDecodedEA(op & 0x3f))   /* check if decoded this EA */
        return 0;
    if (!CheckEA(op & 0x3f, "101111111000"))
        return 0;   /* no An,#dat,(d16,pc),(d8,pc,Xn) for 68000 */

    InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
    if (((op & 0x3f) >> 3) == 0)    /* Dn shortcut */
    {
        e("and  edi, byte 7\n");
        e("mov  edx, [d+edi*4]\n");
        if (size == BYTE_SIZE)
            e("test dl, dl\n");
        else if (size == WORD_SIZE)
            e("test dx, dx\n");
        else if (size == LONG_SIZE)
            e("test edx, edx\n");
    }
    else
    {
        LoadFromEA(op & 0x3f, size, 0);
        if (size == BYTE_SIZE)
            e("test dl, dl\n");
        else if (size == WORD_SIZE)
            e("test dx, dx\n");
        else if (size == LONG_SIZE)
            e("test edx, edx\n");
    }
    e("lahf\n");
    e("xor  al, al\n");
    EmitTiming(base_timing + TimingEA(op & 0x3f, size));
    InstEnd();
    return 1;
}

int Bxx(unsigned short op, char *mnem, unsigned base_timing)
{
    if ((op & 0xff) != 0 && (op & 0xff) != 1)
        return 0;
    switch (op & 0xff)
    {   case 0:     InstBegin(op, mnem, 1); break;
        case 1:     InstBegin(op, mnem, 255); break;
        break;  }
    switch (op & 0xff)
    {
    case 0:     /* 16-bit displacement */
        if ((op & 0xff00) == 0x6100)
        {
            e("mov  edx, 2\n");
            e("add  edx, esi\n");
        }
        e("movsx    ebx, word [esi]\n");
        e("add      esi, ebx\n");
        break;
    case 1:     /* 8-bit displacement -- XX01-XXFF */
        if ((op & 0xff00) == 0x6100)
            e("mov  edx, esi\n");
        e("and      edi, 0xff\n");
        e("mov      ebx, edi\n");
        e("movsx    ebx, bl\n");
        e("add      esi, ebx\n");
        break;
    }
    if (brafetch)
    {
        e("sub  esi, ebp\n");   /* ESI=denormalized PC */
        if ((op & 0xff00) == 0x6100)    /* if BSR, save EDX */
            SaveReg("run", "edx");
        UpdateFetchPtr();
        if ((op & 0xff00) == 0x6100)
            RestoreReg("run", "edx");
    }
    if ((op & 0xff00) == 0x6100)    /* BSR */
    {
        e("sub  dword [a+7*4], byte 4\n");    /* SP - 4 -> SP */
        e("sub  edx, ebp\n");   /* EDX=PC of next instruction */
        e("mov  ebx, [a+7*4]\n");
        WriteLong();                    /* PC -> (SP) */
    }        
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}
    
int Bcc(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    cond = (op >> 8) & 0xf;
    char        mnem[4];
    char        *mnem_t[] = {  NULL,  NULL, "BHI", "BLS",
                              "BCC", "BCS", "BNE", "BEQ",
                              "BVC", "BVS", "BPL", "BMI",
                              "BGE", "BLT", "BGT", "BLE" };

    if (!cond || cond == 1)     return 0;   /* these not available for Bcc */
    strcpy(mnem, mnem_t[cond]);             /* set mnemonic */

    switch (op & 0xff)
    {
    case 0:     /* 16-bit displacement */
        InstBegin(op, mnem, 1); break;
    case 1:     /* 8-bit displacement -- XX01-XXFE */
        InstBegin(op, mnem, 255); break;
    default:
        return 0;
    }

    switch (cond)
    {
    /* EAX:0000000000000000 NZ00 000C 0000 000V */
    case 2:     /* HI: !C & !Z */
        e("test ah, byte 0x41\n");
        e("jz   short .do\n");
        break;
    case 3:     /* LS: C | Z */
        e("test ah, byte 0x41\n");
        e("jnz  short .do\n");
        break;
    case 4:     /* CC: !C */
        e("test ah, byte 1\n");
        e("jz   short .do\n");
        break;
    case 5:     /* CS: C */
        e("test ah, byte 1\n");
        e("jnz  short .do\n");
        break;
    case 6:     /* NE: !Z */
        e("test ah, byte 0x40\n");
        e("jz   short .do\n");
        break;
    case 7:     /* EQ: Z */
        e("test ah, byte 0x40\n");
        e("jnz  short .do\n");
        break;
    case 8:     /* VC: !V */
        e("test al, byte 1\n");
        e("jz   short .do\n");
        break;
    case 9:     /* VS: V */
        e("test al, byte 1\n");
        e("jnz  short .do\n");
        break;
    case 0xa:   /* PL: !N */
        e("test ah, byte 0x80\n");
        e("jz   short .do\n");
        break;
    case 0xb:   /* MI: N */
        e("test ah, byte 0x80\n");
        e("jnz  short .do\n");
        break;
    case 0xc:   /* GE: N & V | !N & !V      (N&V || !N&!V) */
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpe  short .do\n");        
        break;
    case 0xd:   /* LT: N & !V | !N & V */
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpo  short .do\n");
        EmitLabel(".dont_do");
        break;
    case 0xe:   /* GT: N & V & !Z | !N & !V & !Z */
        /* if Z, then it is GE and we don't take this branch */
        e("test ah, byte 0x40\n");
        e("jnz  short .dont_do\n");
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpe  short .do\n");        
        EmitLabel(".dont_do");
        break;
    case 0xf:   /* LE: Z | N & !V | !N & V */
        e("test ah, 0x40\n");
        e("jnz  short .do\n");
        e("mov  bl, al\n");
        e("mov  bh, ah\n");
        e("and  bh, 0xfe\n");
        e("or   bl, bh\n");
        e("test bl, byte 0x81\n");
        e("jpo  short .do\n");
        break;
    }        

    /* branch not taken */
    switch (op & 0xff)  /* look at this to figure out timing and increment */
    {
    case 0:     /* 16-bit */
        e("add  esi, byte 2\n"); EmitTiming(12); break;       
    case 1:     /* 8-bit */
        EmitTiming(8); break;
    }
    InstEnd();
    Align(4);
    EmitLabel(".do");
    switch (op & 0xff)
    {
    case 0:     /* 16-bit */
        e("movsx    ebx, word [esi]\n");
        e("add      esi, ebx\n");
        break;
    case 1:     /* 8-bit */
        e("and      edi, 0xff\n");
        e("mov      ebx, edi\n");
        e("movsx    ebx, bl\n");
        e("add      esi, ebx\n");
        break;
    }
    if (brafetch)
    {
        e("sub  esi, ebp\n");   /* ESI=denormalized PC */
        UpdateFetchPtr();
    }
    EmitTiming(10);
    InstEnd();
    return 1;
}

int Arithmetic(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    t, ea = op & 0x3f, opr_code = (op >> 12),
                opmode = (op >> 6) & 7;
    int         size = UNKNOWN_SIZE;
    char        mnem[4];
    char        opr[4], *size_s[] = { "byte", "word", "dword" },
                *edx[] = { "dl", "dx", "edx" };

    if (opmode & 4) /* EA == destination */
    {
        switch (opr_code)
        {
        case 8:                                                     /* OR  */
        case 9:                                                     /* SUB */
        case 0xc:                                                   /* AND */
        case 0xd:   if (!CheckEA(ea, "001111111000"))   return 0;   /* ADD */
                    break;
        case 0xb:   if (!CheckEA(ea, "101111111000"))   return 0;   /* EOR */
                    break;
        default:    return 0;   }
    }
    else            /* EA == source */
    {
        switch (opr_code)
        {
        case 0xd:                                                   /* ADD */
        case 0xb:                                                   /* CMP */
        case 9:     if (!CheckEA(ea, "111111111111"))   return 0;   /* SUB */
                    if ((ea >> 3) == 1 && (opmode & 3) == 0)
                        return 0;   /* byte access to An not allowed! */
                    break;
        case 8:                                                     /* OR  */
        case 0xc:   if (!CheckEA(ea, "101111111111"))   return 0;   /* AND */
                    break;
        default:    return 0;   }
    }
    if (!NumDecodedEA(ea))  /* check if decoded this EA */
        return 0;
    switch (opmode & 3) /* set allowed sizes */
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0;   }
    switch (opr_code)       /* timing */
    {
    case 0xd:   /* ADD */
    case 9:     /* SUB */
        if (size == BYTE_SIZE || size == WORD_SIZE)
        {
            if ((ea >> 3) == 1)         t = 8;
            else if ((ea >> 3) == 0)    t = 4;
            else                        t = 8;
        }
        else
        {
            if ((ea >> 3) == 1)         t = 6;
            else if ((ea >> 3) == 0)    t = 6;
            else                        t = 12;
            if ((ea >> 3) == 0 || (ea >> 3) == 1 || ea == 0x3c)
                t += 2; /* if reg direct or immediate, timing of 6 is
                           increased to 8 */
        } break;
    case 0xb:
        if (!(opmode & 4))  /* CMP */
        {
            if (size == BYTE_SIZE || size == WORD_SIZE)
            {
                if ((ea >> 3) == 1)         t = 6;
                else if ((ea >> 3) == 0)    t = 4;
            }
            else
            {
                if ((ea >> 3) == 1)         t = 6;
                else if ((ea >> 3) == 0)    t = 6;
            }
        }
        else                /* EOR */
        {
            if (size == BYTE_SIZE || size == WORD_SIZE)
            {
                if ((ea >> 3) == 0)             t = 4;
                else                            t = 8;
            }
            else
            {
                if ((ea >> 3) == 0)             t = 8;
                else                            t = 12;
            } break;
        }
    case 0xc:   /* AND */
        if (size == BYTE_SIZE || size == WORD_SIZE)
        {
            if ((ea >> 3) == 0)             t = 4;
            else                            t = 8;
        }
        else
        {
            if ((ea >> 3) == 0)             t = 6;
            else                            t = 12;
            if ((ea >> 3) == 0 || (ea >> 3) == 1 || ea == 0x3c)
                t += 2; /* if reg direct or immediate, timing of 6 is
                           increased to 8 */
        } break;
    case 0x8:   /* OR */
        if (size == BYTE_SIZE || size == WORD_SIZE)
        {
            if ((ea >> 3) == 0)             t = 4;
            else                            t = 8;
        }
        else
        {
            if ((ea >> 3) == 0)             t = 6;
            else                            t = 12;
            if ((ea >> 3) == 0 || (ea >> 3) == 1 || ea == 0x3c)
                t += 2; /* if reg direct or immediate, timing of 6 is
                           increased to 8 */
        } break;
    }    
    /* set mnem and opr */
    switch (opr_code)
    {   case 0xd:   strcpy(mnem, "ADD");    strcpy(opr, "add"); break;
        case 0xc:   strcpy(mnem, "AND");    strcpy(opr, "and"); break;
        case 0xb:
                    if (opmode & 4) { strcpy(mnem, "EOR"); strcpy(opr, "xor"); }
                    else            { strcpy(mnem, "CMP"); strcpy(opr, "cmp"); }
                    break;
        case 8:     strcpy(mnem, "OR");     strcpy(opr, "or");  break;
        case 9:     strcpy(mnem, "SUB");    strcpy(opr, "sub"); break;
        default:    return 0;
    }


    InstBegin(op, mnem, NumDecodedEA(ea));
    if (opmode & 4) /* op Dn,ea */
    {
        if ((ea >> 3) == 0 || (ea >> 3) == 1)   /* Dn, An shortcut */
        {
            char    r[2] = { 'd', 'a' };
            int     i = ea >> 3;

            e("and  edi, byte 7\n");
            e("mov  edx, [d+(%d*4)]\n", (op >> 9) & 7);
            e("%s   %s [%c+(edi*4)], %s\n", opr, size_s[opmode & 3], r[i],
                                              edx[opmode & 3]);
            e("lahf\n");
            switch ((op >> 12) & 0xf)   /* flags calculations */
            {   case 0xc:   e("xor  al, al\n"); break;      /* AND */
                case 0xd:   e("setc byte [x]\n");
                            e("seto al\n"); break;          /* ADD */
                case 0xb:
                    if (opmode & 4) e("xor  al, al\n");     /* EOR */
                    else            e("seto al\n");         /* CMP */
                    break;
                case 8:     e("xor  al, al\n"); break;      /* OR  */
                case 9:     e("setc byte [x]\n");
                            e("seto al\n"); break;          /* SUB */ }
        }
        else
        {
            LoadFromEA(ea, size, 0);
            e("%s   %s, [d+(%d*4)]\n", opr, edx[opmode & 3], (op >> 9) & 7);
            e("lahf\n");
            switch ((op >> 12) & 0xf)   /* flags calculations */
            {   case 0xc:   e("xor  al, al\n"); break;      /* AND */
                case 0xd:   e("setc byte [x]\n");
                            e("seto al\n"); break;          /* ADD */
                case 0xb:
                    if (opmode & 4) e("xor  al, al\n");     /* EOR */
                    else            e("seto al\n");         /* CMP */
                    break;
                case 8:     e("xor  al, al\n"); break;      /* OR  */
                case 9:     e("setc byte [x]\n");
                            e("seto al\n"); break;          /* SUB */ }

            if(!(((op >> 12) & 0xf) == 0xb && !(opmode & 4))) // don't write if CMP
            {
                switch (size)
                {   case BYTE_SIZE: WriteByte(); break;
                    case WORD_SIZE: WriteWord(); break;
                    case LONG_SIZE: WriteLong(); break;
                }
            }
            else
            {
                e("xor edi, edi\n"); // the write functions normally handle this
            }
        }
    }
    else            /* op ea,Dn */
    {
        LoadFromEA(ea, size, 0);
        e("%s   [d+(%d*4)], %s\n", opr, (op >> 9) & 7, edx[opmode & 3]);
        e("lahf\n");
        switch ((op >> 12) & 0xf)   /* flags calculations */
        {   case 0xc:   e("xor  al, al\n"); break;      /* AND */
            case 0xd:   e("setc byte [x]\n");
                        e("seto al\n"); break;          /* ADD */
            case 0xb:
                if (opmode & 4) e("xor  al, al\n");     /* EOR */
                else            e("seto al\n");         /* CMP */
                break;
            case 8:     e("xor  al, al\n"); break;      /* OR  */
            case 9:     e("setc byte [x]\n");
                        e("seto al\n"); break;          /* SUB */ }        
    }

    EmitTiming(t);
    InstEnd();
    return 1;
}

int ArithmeticI(unsigned short op, char *mnem_unused, unsigned base_timing)
{
    unsigned    t = base_timing, ea = op & 0x3f;    /* base_timing = 8 */
    int         size = UNKNOWN_SIZE;
    char        mnem[5];
    char        opr[5], *size_s[] = { "byte", "word", "dword" },
                *edx[] = { "dl", "dx", "edx" };

    if (!CheckEA(ea, "101111111000"))
        return 0;
    if (!NumDecodedEA(ea))  /* check if decoded this EA */
        return 0;
    switch ((op >> 6) & 3)  /* set allowed sizes */
    {   case 0:     size = BYTE_SIZE; break;
        case 1:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0;   }    
    switch ((op >> 8) & 0xf)    /* set operation, mnem */
    {   case 2:     strcpy(mnem, "ANDI"); strcpy(opr, "and"); break;
        case 6:     strcpy(mnem, "ADDI"); strcpy(opr, "add"); break;
        case 0xc:   strcpy(mnem, "CMPI"); strcpy(opr, "cmp"); break;
        case 0xa:   strcpy(mnem, "EORI"); strcpy(opr, "xor"); break;
        case 0:     strcpy(mnem, "ORI");  strcpy(opr, "or");  break;
        case 4:     strcpy(mnem, "SUBI"); strcpy(opr, "sub"); break;
        default:    return 0;  }
    /* all but CMPI,ANDI take 8+8 cycles for long sizes */
    if (((op >> 8) & 0xf) != 0xc && ((op >> 8) & 0xf) != 2 && size == LONG_SIZE)
        t += 8;
    else if (((op >> 8) & 0xf) == 0xc)                  t += 6;

    InstBegin(op, mnem, NumDecodedEA(ea));
    if (((ea >> 3) & 7) == 0)   /* Dn is treated differently */
    {
        e("and  edi, byte 7\n");/* get reg # */
        e("mov  edx, [esi]\n"); /* get imm data */
        if (size == LONG_SIZE)
            e("ror  edx, byte 16\n");   /* prep. the long-word */
        e("%s   %s [d+edi*4], %s\n", opr, size_s[(op >> 6) & 3], edx[(op >> 6) & 3]);
        e("lahf\n");
        switch ((op >> 8) & 0xf)    /* flags calculations */
        {   case 2:     e("xor  al, al\n"); break;      /* ANDI */
            case 6:     e("setc byte [x]\n");
                        e("seto al\n"); break;          /* ADDI */
            case 0xc:   e("seto al\n"); break;          /* CMPI */
            case 0xa:   e("xor  al, al\n"); break;      /* EORI */
            case 0:     e("xor  al, al\n"); break;      /* ORI  */
            case 4:     e("setc byte [x]\n");
                        e("seto al\n"); break;          /* SUBI */ }
        if (size != LONG_SIZE)
            e("add  esi, byte 2\n");
        else
            e("add  esi, byte 4\n");
    }
    else
    {
        SaveReg("run", "esi");      /* address of immediate data */
        switch (size)   /* make ESI point to EA data */
        {   case BYTE_SIZE:
            case WORD_SIZE: e("add  esi, byte 2\n"); break;
            case LONG_SIZE: e("add  esi, byte 4\n"); break;
        }            
        LoadFromEA(ea, size, 0);
        RestoreRegTo("run", "esi", "edi"); /* EDI=address of immediate data */
        /* if byte or word, perform operation directly from memory */
        if (size == BYTE_SIZE || size == WORD_SIZE)
        {
            e("%s   %s, [edi]\n", opr, edx[(op >> 6) & 3]);
            e("lahf\n");
            switch ((op >> 8) & 0xf)    /* flags calculations */
            {   case 2:     e("xor  al, al\n"); break;      /* ANDI */
                case 6:     e("setc byte [x]\n");
                            e("seto al\n"); break;          /* ADDI */
                case 0xc:   e("seto al\n"); break;          /* CMPI */
                case 0xa:   e("xor  al, al\n"); break;      /* EORI */
                case 0:     e("xor  al, al\n"); break;      /* ORI  */
                case 4:     e("setc byte [x]\n");
                            e("seto al\n"); break;          /* SUBI */ }
        }
        else    /* long */
        {
            e("mov  edi, [edi]\n");
            e("ror  edi, byte 16\n");   /* word-swap long-words... */
            e("%s   %s, edi\n", opr, edx[(op >> 6) & 3]);
            e("lahf\n");
            switch ((op >> 8) & 0xf)    /* flags calculations */
            {   case 2:     e("xor  al, al\n"); break;      /* ANDI */
                case 6:     e("setc byte [x]\n");
                            e("seto al\n"); break;          /* ADDI */
                case 0xc:   e("seto al\n"); break;          /* CMPI */
                case 0xa:   e("xor  al, al\n"); break;      /* EORI */
                case 0:     e("xor  al, al\n"); break;      /* ORI  */
                case 4:     e("setc byte [x]\n");
                            e("seto al\n"); break;          /* SUBI */ }
        }

      if(((op >> 8) & 0x0F) != 0x0C)
      {
        switch (size)   /* write back to mem */
        {   case BYTE_SIZE: WriteByte(); break;
            case WORD_SIZE: WriteWord(); break;
            case LONG_SIZE: WriteLong(); break;
        }
      }
      else
      {
        e("xor edi, edi\n");
      }

    }
    EmitTiming(t + TimingEA(ea, size));
    InstEnd();
    return 1;
}


int MOVE(unsigned short op, char *mnem, unsigned base_timing)
{
    unsigned    src = op & 0x3f, dest = (op >> 6) & 0x3f;
    int         size = UNKNOWN_SIZE;

    dest = ((dest & 0x7) << 3) | ((dest >> 3) & 0x7);
    if (!CheckEA(src, "111111111111") || !CheckEA(dest, "101111111000"))
        return 0;
    if (!NumDecodedEA(src)) /* check if decoded this source EA */
        return 0;
    switch ((op >> 12) & 3) /* set allowed sizes */
    {   case 1:     size = BYTE_SIZE; break;
        case 3:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0; }
    if (size == BYTE_SIZE && ((src >> 3) & 7) == 1)
        return 0;   /* no An byte accesses allowed */
    InstBegin(op, mnem, NumDecodedEA(op & 0x3f));
    LoadFromEA(src, size, 0);
    switch (size)
    {   case BYTE_SIZE: e("test dl, dl\n"); break;
        case WORD_SIZE: e("test dx, dx\n"); break;
        case LONG_SIZE: e("test edx, edx\n"); break;    }
    e("lahf\n");
    e("xor  al, al\n"); /* V=always cleared */
    if (src == dest)
        StoreToEA(dest, size, 1);
    else
        StoreToEA(dest, size, 0);
    EmitTiming(base_timing + TimingEA(src, size) + TimingEA(dest, size));
    InstEnd();
    return 1;
}

int MOVEQ(unsigned op, char *mnem, unsigned base_timing)
{
    InstBegin(op, mnem, 256);   /* 256 possible immediate data combinations */
    e("mov      ebx, edi\n");   /* opcode is in edi */
    e("movsx    ebx, bl\n");    /* sign extend */
    e("mov      [d+%d*4], ebx\n", (op >> 9) & 7);
    e("test     bl, bl\n");     /* set N,Z,C appropriately */
    e("lahf\n");                /* get changes */
    e("xor      al, al\n");     /* V is cleared */   
    EmitTiming(base_timing);
    InstEnd();
    return 1;
}

int MOVEA(unsigned short op, char *mnem, unsigned base_timing)
{    
    int     size = UNKNOWN_SIZE;

    if (!CheckEA(op & 0x3f, "111111111111"))    /* check EA mode */
        return 0;
    if (!NumDecodedEA(op & 0x3f))               /* check if decoded this EA */
        return 0;
    switch ((op >> 12) & 3) /* set allowed sizes */
    {   case 3:     size = WORD_SIZE; break;
        case 2:     size = LONG_SIZE; break;
        default:    return 0; }
         
    InstBegin(op, mnem, NumDecodedEA(op & 0x3f));    
    LoadFromEA(op & 0x3f, size, 1);                 /* load */
    e("mov  [a+%d*4], edx\n", (op >> 9) & 7);     /* store */
    EmitTiming(TimingEA(op & 0x3f, size) + base_timing);
    InstEnd();
    return 1;
}


/*****************************************************************************
* Instruction Emission                                                      */

void EmitInstructions()
{
    unsigned    i, j, k, l, n;

    /* TST */
    n = 0;
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 0x3f; j++)
            n += TST((i << 6) + j + 0x4a00, "TST", 4);
    printf("TST:\t\t%d\n", n);

    /* Bcc */
    n = 0;
    for (i = 0; i <= 0xf; i++)
        for (j = 0; j < 0xff; j++)
            n += Bcc((i << 8) + j + 0x6000, "B??", 0);
    printf("Bcc:\t\t%d\n", n);

    /* MOVE */
    n = 0;
    for (i = 1; i <= 3; i++)            /* size */
        for (j = 0; j <= 0x3f; j++)     /* dest. EA */
            for (k = 0; k <= 0x3f; k++) /* source EA */
                n += MOVE((i << 12) + (j << 6) + k + 0x0000, "MOVE", 4);
    printf("MOVE:\t\t%d\n", n);

    /* ADD, AND, CMP, EOR, OR, SUB -- "Arithmetic" */
    n = 0;
    for (i = 8; i <= 0xd; i++)              /* operation */
        for (j = 0; j <= 7; j++)            /* Dn */
            for (k = 0; k <= 6; k++)        /* opmode */
                for (l = 0; l <= 0x3f; l++) /* EA */
                    n += Arithmetic((i << 12) + (j << 9) + (k << 6) + l, "????", 0);
    printf("ADD, AND, CMP, EOR, OR, SUB: %d\n", n);

    /* ADDI, ANDI, CMPI, EORI, ORI, SUBI -- "ArithmeticI" */
    n = 0;
    for (i = 0; i <= 0xc; i++)          /* operation */
        for (j = 0; j <= 2; j++)        /* size */
            for (k = 0; k <= 0x3f; k++) /* EA */
                n += ArithmeticI((i << 8) + (j << 6) + k + 0x0000, "????I", 8);
    printf("ADDI, ANDI, CMPI, EORI, ORI, SUBI: %d\n", n);

    /* DBcc */
    n = 0;
    for (i = 0; i <= 0xf; i++)          /* condition */
        n += DBcc((i << 8) + 0x50c8, "DB??", 0);
    printf("DBcc:\t\t%d\n", n);

    /* BTST */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 0; j <= 0x3f; j++)     /* EA */
            n += BTST((i << 9) + j + 0x0100, "BTST", 0);
    for (i = 0; i <= 0x3f; i++)         /* EA */
        n += BTST(i + 0x0800, "BTST", 0);
    printf("BTST:\t\t%d\n", n);

    /* ADDQ, SUBQ -- "ArithmeticQ" */
    n = 0;
    for (i = 0; i <= 7; i++)                /* data */
        for (j = 0; j <= 1; j++)            /* ADDQ/SUBQ */
            for (k = 0; k <= 2; k++)        /* size */
                for (l = 0; l <= 0x3f; l++) /* EA */
                    n += ArithmeticQ((i << 9) + (j << 8) + (k << 6) + l + 0x5000, "???Q", 4);
    printf("ADDQ, SUBQ:\t%d\n", n);

    /* BRA */
    n = 0;
    for (i = 0; i <= 0xff; i++)
        n += Bxx(i + 0x6000, "BRA", 10);
    printf("BRA:\t\t%d\n", n);

    /* LEA */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 0; j <= 0x3f; j++)     /* EA */
            n += LEA((i << 9) + 0x41c0 + j, "LEA", 0);
    printf("LEA:\t\t%d\n", n);

    /* RTS */
    RTS(0x4e75, "RTS", 16);
    printf("RTS:\t\t1\n");

    /* MOVEA */
    n = 0;
    for (i = 2; i <= 3; i++)            /* size */
        for (j = 0; j <= 7; j++)        /* dest. reg */
            for (k = 0; k <= 0x3f; k++) /* source EA */
                n += MOVEA((i << 12) + (j << 9) + k + 0x0040, "MOVEA", 4);
    printf("MOVEA:\t\t%d\n", n);

    /* ADDA */
    n = 0;
    for (i = 3; i <= 7; i++)            /* size */
        for (j = 0; j <= 7; j++)        /* dest. reg */
            for (k = 0; k <= 0x3f; k++) /* source EA */
                n += ArithmeticA((i << 6) + (j << 9) + k + 0xd000, "ADDA", 4);
    printf("ADDA:\t\t%d\n", n);

    /* MOVEQ */
    n = 0;
    for (i = 0; i < 8; i++)
        n += MOVEQ((i << 9) + 0x7000, "MOVEQ", 4);
    printf("MOVEQ:\t\t%d\n", n);

    /* JSR */
    n = 0;
    for (i = 0; i <= 0x3f; i++)         /* register */
        n += Jxx(i + 0x4e80, "JSR", 0);
    printf("JSR:\t\t%d\n", n);

    /* LSL, LSR */
    n = 0;
    for (i = 0; i <= 7; i++)                /* count/register */
        for (j = 0; j <= 1; j++)            /* direction */
            for (k = 0; k <= 2; k++)        /* size */
                for (l = 0; l <= 1; l++)    /* immediate/reg */
                    n += LSxReg((i << 9) + (j << 8) + (k << 6) + (l << 5) + 0xe008, "LS?", 0);
    for (i = 0; i <= 1; i++)                /* direction */
        for (j = 0; j <= 0x3f; j++)         /* EA */
            n += LSxMem((i << 8) + j + 0xe2c0, "LS?", 8);
    printf("LSL, LSR:\t%d\n", n);    

    /* SWAP */
    SWAP(0x4840, "SWAP", 4);
    printf("SWAP:\t\t1\n");

    /* BSR */
    n = 0;
    for (i = 0; i <= 0xff; i++)
        n += Bxx(i + 0x6100, "BSR", 18);
    printf("BSR:\t\t%d\n", n);

    /* EXT */
    n = 0;
    for (i = 2; i <= 7; i++)
        n += EXT((i << 6) + 0x4800, "EXT", 4);
    printf("EXT:\t\t%d\n", n);

    /* CLR */
    n = 0;
    for (i = 0; i <= 2; i++)            /* size */
        for (j = 0; j <= 0x3f; j++)     /* EA */
            n += CLR((i << 6) + j + 0x4200, "CLR", 0);
    printf("CLR:\t\t%d\n", n);

    /* MOVEM */
    n = 0;
    for (i = 0; i <= 1; i++)            /* direction */
        for (j = 0; j <= 1; j++)        /* size */
            for (k = 0; k <= 0x3f; k++) /* EA */
                n += MOVEM((i << 10) + (j << 6) + k + 0x4880, "MOVEM", 0);
    printf("MOVEM:\t\t%d\n", n);

    /* MOVE to SR */
    n = 0;
    for (i = 0; i <= 0x3f; i++)
        n += MOVEtoSR(i + 0x46c0, "MOVE to SR", 12);
    printf("MOVE to SR:\t%d\n", n);

    /* NOP */
    NOP(0x4e71, "NOP", 4);
    printf("NOP:\t\t1\n");

    /* JMP */
    n = 0;
    for (i = 0; i <= 0x3f; i++)         /* register */
        n += Jxx(i + 0x4ec0, "JMP", 0);
    printf("JMP:\t\t%d\n", n);

    /* CMPA */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 3; j <= 7; j++)        /* opmode */
            for (k = 0; k <= 0x3f; k++) /* EA */
                n += CMPA((i << 9) + (j << 6) + k + 0xb000, "CMPA", 0);
    printf("CMPA:\t\t%d\n", n);

    /* ASL, ASR */
    n = 0;
    for (i = 0; i <= 7; i++)                /* count/reg */
        for (j = 0; j <= 1; j++)            /* direction */
            for (k = 0; k <= 2; k++)        /* size */
                for (l = 0; l <= 1; l++)    /* immediate/reg */
                    n += ASxReg((i << 9) + (j << 8) + (k << 6) + (l << 5) + 0xe000, "AS?", 0);
    for (i = 0; i <= 1; i++)                /* direction */
        for (j = 0; j <= 0x3f; j++)         /* EA */
            n += ASxMem((i << 8) + j + 0xe0c0, "AS?", 8);
    printf("ASL, ASR:\t%d\n", n);

    /* MULS */
    n = 0;
    for (i = 0; i <= 7; i++)        /* register */
        for (j = 0; j <= 0x3f; j++) /* EA */
            n += MULS((i << 9) + j + 0xc1c0, "MULS", 70);
    printf("MULS:\t\t%d\n", n);

    /* RTE */
    RTE(0x4e73, "RTE", 20);
    printf("RTE:\t\t1\n");

    /* NEG, NEGX */
    n = 0;
    for (i = 0; i <= 1; i++)            /* NEG/NEGX */
        for (j = 0; j <= 2; j++)        /* size */
            for (k = 0; k <= 0x3f; k++) /* EA */
                n += NEGx((i << 10) + (j << 6) + k + 0x4000, "NEG?", 0);
    printf("NEG, NEGX:\t%d\n", n);

    /* ROL, ROR */
    n = 0;
    for (i = 0; i <= 7; i++)                /* count/register */
        for (j = 0; j <= 1; j++)            /* direction */
            for (k = 0; k <= 2; k++)        /* size */
                for (l = 0; l <= 1; l++)    /* immediate/reg */
                    n += ROxReg((i << 9) + (j << 8) + (k << 6) + (l << 5) + 0xe018, "RO?", 0);
    for (i = 0; i <= 1; i++)                /* direction */
        for (j = 0; j <= 0x3f; j++)         /* EA */
            n += ROxMem((i << 8) + j + 0xe6c0, "RO?", 8);
    printf("ROL, ROR:\t%d\n", n);

    /* BCLR */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 0; j <= 0x3f; j++)     /* EA */
            n += BCLR((i << 9) + j + 0x0180, "BCLR", 0);
    for (i = 0; i <= 0x3f; i++)         /* EA */
        n += BCLR(i + 0x0880, "BCLR", 0);
    printf("BCLR:\t\t%d\n", n);

    /* NOT */
    n = 0;
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 0x3f; j++)
            n += NOT((i << 6) + j + 0x4600, "NOT", 0);
    printf("NOT:\t\t%d\n", n);

    /* MOVE USP */
    n = 0;
    for (i = 0; i <= 1; i++)
        n += MOVEUSP((i << 3) + 0x4e60, "MOVE USP", 4);
    printf("MOVE USP:\t%d\n", n);

    /* ROXL, ROXR */
    n = 0;
    for (i = 0; i <= 7; i++)                /* count/register */
        for (j = 0; j <= 1; j++)            /* direction */
            for (k = 0; k <= 2; k++)        /* size */
                for (l = 0; l <= 1; l++)    /* immediate/reg */
                    n += ROXxReg((i << 9) + (j << 8) + (k << 6) + (l << 5) + 0xe010, "ROX?", 0);
    for (i = 0; i <= 1; i++)                /* direction */
        for (j = 0; j <= 0x3f; j++)         /* EA */
            n += ROXxMem((i << 8) + j + 0xe4c0, "ROX?", 8);
    printf("ROXL, ROXR:\t%d\n", n);

    /* BSET */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 0; j <= 0x3f; j++)     /* EA */
            n += BSET((i << 9) + j + 0x01c0, "BSET", 0);
    for (i = 0; i <= 0x3f; i++)         /* EA */
        n += BSET(i + 0x08c0, "BSET", 0);
    printf("BSET:\t\t%d\n", n);

    /* MOVE from SR */

    /*
     * The 68000 and 68008 "MOVE from SR" is not privileged. But for the 68010
     * and higher, it is.
     */

    n = 0;
    for (i = 0; i <= 0x3f; i++)
        n += MOVEfromSR(i + 0x40c0, "MOVE from SR", 0);
    printf("MOVE from SR:\t%d\n", n);

    /* SUBA */
    n = 0;
    for (i = 3; i <= 7; i++)            /* size */
        for (j = 0; j <= 7; j++)        /* dest. reg */
            for (k = 0; k <= 0x3f; k++) /* source EA */
                n += ArithmeticA((i << 6) + (j << 9) + k + 0x9000, "SUBA", 4);
    printf("SUBA:\t\t%d\n", n);

    /* DIVS */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 0; j <= 0x3f; j++)     /* EA */
            n += DIVS((i << 9) + j + 0x81c0, "DIVS", 158);
    printf("DIVS:\t\t%d\n", n);

    /* MULU */
    n = 0;
    for (i = 0; i <= 7; i++)        /* register */
        for (j = 0; j <= 0x3f; j++) /* EA */
            n += MULU((i << 9) + j + 0xc0c0, "MULU", 70);
    printf("MULU:\t\t%d\n", n);

    /* ADDX */
    n = 0;
    for (i = 0; i <= 7; i++)                /* Rx */
        for (j = 0; j <= 2; j++)            /* size */
            for (k = 0; k <= 1; k++)        /* R/M */
                n += ADDX((i << 9) + (j << 6) + (k << 3) + 0xd100, "ADDX", 0);
    printf("ADDX:\t\t%d\n", n);

    /* Scc */
    n = 0;
    for (i = 0; i <= 0xf; i++)      /* condition */
        for (j = 0; j <= 0x3f; j++) /* EA */
            n += Scc((i << 8) + j + 0x50c0, "S??", 0);
    printf("Scc:\t\t%d\n", n);

    /* ORI to SR */
    ORItoSR(0x007c, "ORI to SR", 20);
    printf("ORI to SR:\t1\n");

    /* MOVE to CCR */
    n = 0;
    for (i = 0; i <= 0x3f; i++)
        n += MOVEtoCCR(i + 0x44c0, "MOVE to CCR", 12);
    printf("MOVE to CCR:\t%d\n", n);

    /* EXG */
    n = 0;
    for (i = 0; i <= 7; i++)            /* Rx */
        for (j = 0; j <= 0x11; j++)     /* opmode */
            n += EXG((i << 9) + (j << 3) + 0xc100, "EXG", 6);
    printf("EXG:\t\t%d\n", n);

    /* UNLK */
    UNLK(0x4e58, "UNLK", 12);
    printf("UNLK:\t\t1\n");

    /* LINK */
    LINK(0x4e50, "LINK", 16);   /* word */
    printf("LINK:\t\t1\n");

    /* DIVU */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 0; j <= 0x3f; j++)     /* EA */
            n += DIVU((i << 9) + j + 0x80c0, "DIVU", 144);
    printf("DIVU:\t\t%d\n", n);

    /* ANDI to CCR */
    ANDItoCCR(0x023c, "ANDI to CCR", 20);
    printf("ANDI to CCR:\t1\n");

    /* PEA */
    n = 0;
    for (i = 0; i <= 0x3f; i++)
        n += PEA(i + 0x4840, "PEA", 0);
    printf("PEA:\t\t%d\n", n);

    /* ANDI to SR */
    ANDItoSR(0x027c, "ANDI to SR", 20);
    printf("ANDI to SR:\t%d\n", n);

    /* BCHG */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 0; j <= 0x3f; j++)     /* EA */
            n += BCHG((i << 9) + j + 0x0140, "BCHG", 0);
    for (i = 0; i <= 0x3f; i++)         /* EA */
        n += BCHG(i + 0x0840, "BCHG", 0);
    printf("BCHG:\t\t%d\n", n);

    /* ORI to CCR */
    ORItoCCR(0x003c, "ORI to CCR", 20);
    printf("ORI to CCR:\t1\n");

    /* RTR */
    RTR(0x4e77, "RTR", 20);
    printf("RTR:\t\t1\n");

    /* TRAP */
    TRAP(0x4e40, "TRAP", 34);
    printf("TRAP:\t\t1\n");

    /* SBCD */
    n = 0;
    for (i = 0; i <= 7; i++)            /* Rx */
        for (j = 0; j <= 1; j++)        /* R/M */
            n += SBCD((i << 9) + (j << 3) + 0x8100, "SBCD", 0);
    printf("SBCD:\t\t%d\n", n);

    /* SUBX */
    n = 0;
    for (i = 0; i <= 7; i++)                /* Rx */
        for (j = 0; j <= 2; j++)            /* size */
            for (k = 0; k <= 1; k++)        /* R/M */
                n += SUBX((i << 9) + (j << 6) + (k << 3) + 0x9100, "SUBX", 0);
    printf("SUBX:\t\t%d\n", n);

    /* ABCD */
    n = 0;
    for (i = 0; i <= 7; i++)            /* Rx */
        for (j = 0; j <= 1; j++)        /* R/M */
            n += ABCD((i << 9) + (j << 3) + 0xc100, "ABCD", 0);
    printf("ABCD:\t\t%d\n", n);

    /* CMPM */
    n = 0;
    for (i = 0; i <= 7; i++)            /* Ax */
        for (j = 0; j <= 2; j++)        /* size */
            n += CMPM((i << 9) + (j << 6) + 0xb108, "CMPM", 0);
    printf("CMPM:\t\t%d\n", n);

    /* MOVEP */
    n = 0;
    for (i = 0; i <= 7; i++)        /* Dx */
        for (j = 0; j <= 7; j++)    /* opmode */
            n += MOVEP((i << 9) + (j << 6) + 0x0008, "MOVEP", 0);
    printf("MOVEP:\t\t%d\n", n);

    /*
     * The following have no profile data associated with them, as far as my
     * tests have shown.
     */

    /* BKPT */
    if (mpu == 68010)
    {
        n = BKPT(0x4848, "BKPT", 45);
        printf("BKPT:\t\t%d\n", n);
    }
    
    /* CHK */
    n = 0;
    for (i = 0; i <= 7; i++)            /* register */
        for (j = 0; j <= 3; j++)        /* size */
            for (k = 0; k <= 0x3f; k++) /* EA */
                n += CHK((i << 9) + (j << 7) + k + 0x4000, "CHK", 10);
    printf("CHK:\t\t%d\n", n);

    /* EORI to CCR */
    EORItoCCR(0x0a3c, "EORI to CCR", 20);
    printf("EORI to CCR:\t1\n");

    /* EORI to SR */
    EORItoSR(0x0a7c, "EORI to SR", 20);
    printf("EORI to SR:\t1\n");

    /* ILLEGAL */
    InstBegin(0x4afc, "ILLEGAL", 1);
    e("jmp  near exception_illegal_instruction\n");
    printf("ILLEGAL:\t1\n");

    /* MOVEC */
    if (mpu == 68010)
    {
        n = MOVEC(0x4e7a, "MOVEC", 0);
        printf("MOVEC:\t\t%d\n", n);
    }
                
    /* MOVE from CCR -- 68010, 68020, 68030, 68040, CPU32 */
    if (mpu == 68010)
    {
        n = 0;
        for (i = 0; i <= 0x3f; i++)
            n += MOVEfromCCR(i + 0x42c0, "MOVE from CCR", 0);
        printf("MOVE from CCR:\t%d\n", n);
    }

    /* NBCD */
    n = 0;
    for (i = 0; i <= 0x3f; i++)     /* EA */
        n += NBCD(i + 0x4800, "NBCD", 0);
    printf("NBCD:\t\t%d\n", n);

    /* RESET */
    RESET(0x4e70, "RESET", 132);
    printf("RESET:\t\t1\n");

    /* RTD */
    if (mpu == 68010)
    {
        RTD(0x4e74, "RTD", 16);
        printf("RTD:\t\t1\n");
    }

    /* STOP */
    STOP(0x4e72, "STOP", 4);
    printf("STOP:\t\t1\n");

    /* TAS */
    n = 0;
    for (i = 0; i <= 0x3f; i++)
        n += TAS(i + 0x4ac0, "TAS", 0);
    printf("TAS:\t\t%d\n", n);

    /* TRAPV */
    TRAPV(0x4e76, "TRAPV", 0);
    printf("TRAPV:\t\t1\n");
              
    if (illegal)    /* emulate Line 1010 and Line 1111 exceptions */
    {
        /* Line 1010 */
        InstBegin(0xa000, "Line 1010 Emulator", 4096);
        e("jmp  near exception_line_1010_emulator\n");

        /* Line 1111 */
        InstBegin(0xf000, "Line 1111 Emulator", 4096);
        e("jmp  near exception_line_1111_emulator\n");

        num_handlers -= 2;  /* don't count these as instruction handlers */
    }

    /* I_Invalid: Invalid instruction */
    EmitLabel("I_Invalid");
    if (illegal)    /* emulate illegal instruction exceptions */
        e("jmp  near exception_illegal_instruction\n");
    else
    {
        e("sub  esi, byte 2\n");        /* back up to the bad instruction */
        e("jmp  invalid_error\n");      /* invalid instruction error */
    }
}

void EmitExceptions()
{

    /*
     * exception_chk:
     * exception_divide_by_zero:
     * Assume the PC points to the instruction AFTER the instruction which
     * caused the trap.
     */
    Align(4);
    EmitLabel("exception_chk");
    e("push dword 6*4\n");  /* CHK exception vector */
    e("jmp  short do_exception\n");
    Align(4);
    EmitLabel("exception_divide_by_zero");
    e("push dword 5*4\n");  /* division by zero exception vector */
    e("jmp  short do_exception\n");

    /*
     * exception_privilege_violation:
     * exception_illegal_instruction:
     * exception_line_1010_emulator:
     * exception_line_1111_emulator:
     * Assume the PC points to the word AFTER the instruction which caused the
     * trap. The value 2 will be subtracted from the PC in order to make them
     * point AT the instruction which caused the trap, as this is how the
     * exceptions work.
     */
    Align(4);
    EmitLabel("exception_privilege_violation");
    e("push dword 8*4\n");  /* privilege violation exception vector */
    e("jmp  short do_exception_fix_pc\n");
    Align(4);
    EmitLabel("exception_illegal_instruction");
    e("push dword 4*4\n");  /* illegal instruction exception vector */
    e("jmp  short do_exception_fix_pc\n");
    Align(4);
    EmitLabel("exception_line_1010_emulator");
    e("push dword 10*4\n"); /* Line 1010 emulator exception vector */
    e("jmp  short do_exception_fix_pc\n");
    Align(4);
    EmitLabel("exception_line_1111_emulator");
    e("push dword 11*4\n"); /* Line 1111 emulator exception vector */
    e("jmp  short do_exception_fix_pc\n");

    EmitLabel("do_exception_fix_pc");   /* points PC at trap instruction */
    e("sub  esi, byte 2\n");
    EmitLabel("do_exception");
    e("mov  edx, 0x2000\n");            /* what the SR will be */
    e("xor  edx, [sr]\n");              /* see if S bit is different */
    e("test edx, 0x2000\n");
    e("jz   .no_sp_magic\n");           /* nope, don't swap SPs */
    e("mov  edx, [__sp]\n");
    e("xchg [a+7*4], edx\n");
    e("mov  [__sp], edx\n");
    SetSupervisorAddressSpace();        /* map in supervisor address space */
    EmitLabel(".no_sp_magic");

    e("mov  ebx, [a+7*4]\n");           /* SP */
    /*
     * If 68010, we have to push the format word on the stack first.
     */
    if (mpu == 68010)
    {
        e("sub  ebx, byte 2\n");/* SP-2 */
        e("mov  edx, [esp]\n"); /* vector offset | 0x00000000 */
        SaveReg("run", "ebx");
        WriteWord();
        RestoreReg("run", "ebx");
    }        
    e("mov  edx, esi\n");
    e("sub  edx, ebp\n");               /* PC->EDX */
    e("sub  ebx, byte 4\n");            /* SP-4 */
    SaveReg("run", "ebx");
    WriteLong();
    RestoreReg("run", "ebx");
    e("mov  edx, [sr]\n");              /* get SR */
    e("sub  ebx, byte 2\n");            /* SP-2 */
    SaveReg("run", "ebx");
    WriteWord();                        /* save SR to stack */
    RestoreReg("run", "ebx");
    e("or   dh, 0x20\n");               /* set supervisor for exception */
    e("and  edx, 0xa71f\n");            /* clear unwanted bits */
    e("mov  [sr], edx\n");              /* set new SR, get old SR->EDX */
    e("mov  [a+7*4], ebx\n");           /* write back SP */

    e("pop  ebx\n");                    /* get exception vector address */
    if (mpu == 68010)
        e("add  ebx, [vbr]\n");

    ReadLong();                         /* get PC */
    e("mov  esi, edx\n");               /* set new PC */
    UpdateFetchPtr();
    e("xor  edi, edi\n");
    e("sub  ecx, byte 40\n");           /* timing... */
    e("mov  di, [esi]\n");
    e("add  esi, byte 2\n");
    e("jmp  [jmptab+edi*4]\n");         /* continue execution */
}

/*****************************************************************************
* Data                                                                      */

void EmitData()
{
    int     i;

    CacheAlign();
    EmitGlobalLabel(NameOfContext());
    e("context_start:\n");
    e("fetch      dd 0\n");             /* pointer to fetch array */
    e("pcfetch    dd 0\n");             /* PC-relative fetch */
    e("read_byte  dd 0\n");             /* pointer to read_byte array */
    e("read_word  dd 0\n");             /* pointer to read_word array */
    e("read_long  dd 0\n");             /* pointer to read_long array */
    e("write_byte dd 0\n");             /* pointer to write_byte array */
    e("write_word dd 0\n");             /* pointer to write_word array */
    e("write_long dd 0\n");             /* pointer to write_long array */
    e("super_fetch      dd 0\n");       /* supervisor memory map... */
    e("super_pcfetch    dd 0\n");
    e("super_read_byte  dd 0\n");
    e("super_read_word  dd 0\n");
    e("super_read_long  dd 0\n");
    e("super_write_byte dd 0\n");
    e("super_write_word dd 0\n");
    e("super_write_long dd 0\n");
    e("user_fetch       dd 0\n");       /* user memory map... */
    e("user_pcfetch     dd 0\n");
    e("user_read_byte   dd 0\n");
    e("user_read_word   dd 0\n");
    e("user_read_long   dd 0\n");
    e("user_write_byte  dd 0\n");
    e("user_write_word  dd 0\n");
    e("user_write_long  dd 0\n");

    e("intr       times 8 dd 0\n");     /* intr[0-6]=vectors for interrupt
                                           levels 1-7 (0=not pending.)
                                           intr[7]=# of interrupts pending */
    e("cycles     dd 0\n");             /* cycles total to emulate */
    e("remaining  dd 0\n");             /* cycles remaining */
    e("d          times 8 dd 0\n");     /* D0-D7 */
    e("a          times 8 dd 0\n");     /* A0-A7 */
    e("__sp       dd 0\n");             /* User mode: SSP, Supervisor: USP */
    e("sr         dd 0\n");             /* SR */
    e("pc         dd 0\n");             /* PC */
    if (mpu == 68010)
    {
        e("fc     dd 0\n");             /* FC (DFC, SFC) */
        e("vbr    dd 0\n");             /* VBR */
    }
    e("status     dd 0\n");             /* bit 0:1=emulating, 0=not
                                           bit 1:1=stopped, 0=not
                                           bit 2:1=processing ints, 0=not */
    e("InterruptAcknowledge dd 0\n");   /* interrupt acknowledge callback */
    e("Reset      dd 0\n");             /* pointer to RESET handler */
	if (mpu == 68010)
        e("Bkpt   dd 0\n");             /* pointer to BKPT handler */
    e("Debug      dd 0\n");			    /* pointer to debug handler */
    e("context_end:\n");
    e("x          dd 0\n");             /* X flag (maintained internally) */
    for (i = 0; i < 7; i++)
    {
        char    *reg[] = { "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp" };
        e("memhandler_%s    dd 0\n", reg[i]);
    }
    for (i = 0; i < 7; i++)
    {
        char    *reg[] = { "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp" };
        e("run_%s    dd 0\n", reg[i]);
    }
    e("fetch_esi dd 0\n");  /* for UpdateFetchPtr() */
}

/*****************************************************************************
* Code                                                                      */

void EmitCode()
{
    int     i;

    /* Turbo68KInit() */

    /*
     * Decompresses the jump table. See main() for the compression format
     */

    EmitGlobalLabel("Turbo68KInit");
    e("push ecx\n");
    e("push edx\n");
    e("push esi\n");
    e("push edi\n");
    e("mov  esi, compressed_jmptab\n");
    e("mov  edi, jmptab\n");
    e("xor  ecx, ecx\n");               /* we only need lower half (CX) */
    EmitLabel(".l");
    e("mov  cx, [esi]\n");              /* get repeat information */
    e("add  esi, byte 2\n");            /* point at data */
    e("mov  edx, ecx\n");
    e("and  edx, 0xc000\n");
    e("cmp  dx, 0xc000\n");             /* #handlers * 1 each? */
    e("je   .r1\n");
    e("cmp  dx, 0x8000\n");             /* #handlers * 8 each? */
    e("je   .r8\n");
    e("mov  eax, [esi]\n"); /* get data and repeat */
    e("add  esi, byte 4\n");
    e("cld\n");
    e("rep  stosd\n");

    EmitLabel(".c");
    e("cmp  dword [esi], byte -1\n");   /* end of compressed data? */
    e("jne  .l\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ecx\n");
    e("xor  eax, eax\n");
    e("ret\n");

    EmitLabel(".r1");
    e("and  ecx, 0x3ff\n");             /* get rid of 0xc000 marker */
    e("cld\n");
    e("rep  movsd\n");                  /* move address to decomp'd jmptab */
    e("jmp  .c\n");                     /* continue decompression */

    EmitLabel(".r8");
    e("and  ecx, 0x7ff\n");             /* get rid of 0x8000 marker */
    EmitLabel(".r8_l");
    e("mov  edx, ecx\n");               /* save counter temporarily */
    e("mov  ecx, 8\n");                 /* repeat handler 8 times */
    e("mov  eax, [esi]\n");             /* get handler to repeat */
    e("add  esi, byte 4\n");            /* point to next */
    e("cld\n");
    e("rep  stosd\n");                  /* store! */
    e("mov  ecx, edx\n");               /* restore loop counter and loop */
    e("dec  ecx\n");
    e("jnz  .r8_l\n");
    e("jmp  .c\n");

    /* Turbo68KReset() */

    Align(4);
    EmitGlobalLabel("Turbo68KReset");
    e("push ebx\n");
    e("push ecx\n");
    e("push edx\n");
    e("push esi\n");
    e("push edi\n");
    e("push ebp\n");
    SetSupervisorAddressSpace();
    EMULATING();
    UNSTOP_CPU();                       /* in case we're STOPped, unstop */
    if (mpu == 68010)                   /* clear 68010 VBR */
        e("mov  dword [vbr], 0\n");

    e("xor  ecx, ecx\n");   /* no cycles executed in case error */

    /*
     * Read the PC vector from the fetch memory map
     */
    if (!call_convention)   /* stack calling convention */
        e("push dword 4\n");
    else                    /* register calling convention */
        e("mov  eax, 4\n");
    e("call %sTurbo68KFetchPtr\n", id);
    if (!call_convention)
        e("add  esp, byte 4\n");
    e("test eax, eax\n");
    e("jz   near fetch_error\n");
    e("mov  eax, [eax]\n");
    e("rol  eax, byte 16\n");   /* memory is byte swapped */

    e("mov  dword "SR", 0x2700\n");     /* in supervisor mode at start */
    e("mov  dword [cycles], 0\n");      /* no cycles executed */
    e("mov  dword [remaining], 0\n");   /* ..ditto.. */
    e("mov  esi, eax\n");   /* from Turbo68KFetchPtr() */
    e("xor  eax, eax\n");   /* clear registers */
    e("mov  edi, d\n");
    e("mov  ecx, 16\n");
    e("cld\n");
    e("rep  stosd\n");
    e("mov  edi, intr\n");  /* clear interrupt queue */
    e("mov  ecx, 8\n");
    e("rep  stosd\n");
    e("xor  ecx, ecx\n");   /* no cycles executed in case error */
    UpdateFetchPtr();       /* this also makes sure it can be fetched */
    WRITEPCTOMEM("pc");        

    /*
     * Read the SP vector from the fetch memory map
     */
    if (!call_convention)   /* stack calling convention */
        e("push dword 0\n");
    else                    /* register calling convention */
        e("xor  eax, eax\n");
    e("call %sTurbo68KFetchPtr\n", id);
    if (!call_convention)
        e("add  esp, byte 4\n");
    e("test eax, eax\n");
    e("jz   near fetch_error\n");
    e("mov  eax, [eax]\n");
    e("rol  eax, byte 16\n");   /* memory is byte swapped */

    e("mov  "A7", eax\n");  /* supervisor */
    e("mov  "SP", eax\n");  /* user */
    STOP_EMULATING();       /* no longer running */
    e("pop  ebp\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ecx\n");
    e("pop  ebx\n");
    e("xor  eax, eax\n");
    if (mmx)    e("emms\n");
    e("ret\n");

    EmitLabel("invalid_error");     /* invalid instruction */
    SaveCCR();
    STOP_EMULATING();
    STOP_RUNNING();                 /* save PC and cycles remaining */
    e("pop  ebp\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ecx\n");
    e("pop  ebx\n");
    e("mov  eax, %d\n", TURBO68K_ERROR_INVINST);
    e("ret\n");

    /*
     * NOTE: Only UpdateFetchPtr() and Turbo68KReset() can call this
     */

    EmitLabel("fetch_error");       /* could not fetch instruction */
    e("mov  [remaining], ecx\n");   /* save cycles remaining */
    STOP_EMULATING();
    e("mov  [pc], esi\n");          /* UpdateFetchPtr() didn't yet change this */ 
    e("pop  ebp\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ecx\n");
    e("pop  ebx\n");
    e("mov  eax, %d\n", TURBO68K_ERROR_FETCH);
    e("ret\n");

    /* Turbo68KReadPC() */

    /*
     * Can be used anywhere, but usage while Turbo68K is emulating may
     * result in the value being offset a few bytes into the current
     * instruction, but it shouldn't
     */

    Align(4);
    EmitGlobalLabel("Turbo68KReadPC");
    e("mov  eax, [pc]\n");
    e("ret\n");

    /* Turbo68KSetFetch() */

    Align(4);
    EmitGlobalLabel("Turbo68KSetFetch");
    e("push eax\n");
    e("push edx\n");
    GetArg("eax", 0, 12);

    e("sub  eax, "SIZEOF_FETCHREGION"\n");
    if (multiaddr)
    {
        GetArg("edx", 1, 16);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  [super_fetch], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if supervisor, update fetch */
        e("jz   near .not_super\n");
        e("mov  [fetch], eax\n");
        EmitLabel(".not_super");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");

        EmitLabel(".user");
        e("mov  [user_fetch], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if user, update fetch */
        e("jnz  near .not_user\n");
        e("mov  [fetch], eax\n");
        EmitLabel(".not_user");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }
    else
    {
        e("mov  [fetch], eax\n");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }

    /* Turbo68KGetFetch() */

    Align(4);
    EmitGlobalLabel("Turbo68KGetFetch");
    e("push edx\n");
    if (multiaddr)
    {
        GetArg("edx", 0, 8);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  eax, [super_fetch]\n");
        e("jmp  near .finish\n");
        EmitLabel(".user");
        e("mov  eax, [user_fetch]\n");
        EmitLabel(".finish");
    }
    else
        e("mov  eax, [fetch]\n");
    e("pop  edx\n");
    e("add  eax, "SIZEOF_FETCHREGION"\n");
    e("ret\n");

    /* Turbo68KSetPCFetch() */

    if (pcfetch)
    {
        Align(4);
        EmitGlobalLabel("Turbo68KSetPCFetch");
        e("push eax\n");
        e("push edx\n");
        GetArg("eax", 0, 12);
        e("sub  eax, "SIZEOF_FETCHREGION"\n");
        if (multiaddr)
        {
            GetArg("edx", 1, 16);
            e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
            e("jne  short .user\n");
            e("mov  [super_pcfetch], eax\n");
            e("test byte [sr+1], 0x20\n");  /* if supervisor, update fetch */
            e("jz   near .not_super\n");
            e("mov  [pcfetch], eax\n");
            EmitLabel(".not_super");
            e("pop  edx\n");
            e("pop  eax\n");
            e("ret\n");
    
            EmitLabel(".user");
            e("mov  [user_pcfetch], eax\n");
            e("test byte [sr+1], 0x20\n");  /* if user, update fetch */
            e("jnz  near .not_user\n");
            e("mov  [pcfetch], eax\n");
            EmitLabel(".not_user");
            e("pop  edx\n");
            e("pop  eax\n");
            e("ret\n");
        }
        else
        {
            e("mov  [pcfetch], eax\n");
            e("pop  edx\n");
            e("pop  eax\n");
            e("ret\n");
        }

        /* Turbo68KGetPCFetch() */

        Align(4);
        EmitGlobalLabel("Turbo68KGetPCFetch");
        e("push edx\n");
        if (multiaddr)
        {
            GetArg("edx", 0, 8);
            e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
            e("jne  short .user\n");
            e("mov  eax, [super_pcfetch]\n");
            e("jmp  near .finish\n");
            EmitLabel(".user");
            e("mov  eax, [user_pcfetch]\n");
            EmitLabel(".finish");
        }
        else
            e("mov  eax, [pcfetch]\n");
        e("pop  edx\n");
        e("add  eax, "SIZEOF_FETCHREGION"\n");
        e("ret\n");
    }

    /*
     * If the memory map mode is NOT 0 (defmap), we do not add or subtract
     * from the pointers because they aren't arrays, but function pointers.
     */

    /* Turbo68KSetReadByte() */

    Align(4);
    EmitGlobalLabel("Turbo68KSetReadByte");
    e("push eax\n");
    e("push ebx\n");
    GetArg("eax", 0, 12);
    if (memmap_type == 0)
        e("sub  eax, "SIZEOF_DATAREGION"\n");
    if (multiaddr)
    {
        GetArg("edx", 1, 16);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  [super_read_byte], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if supervisor, update fetch */
        e("jz   near .not_super\n");
        e("mov  [read_byte], eax\n");
        EmitLabel(".not_super");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");

        EmitLabel(".user");
        e("mov  [user_read_byte], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if user, update fetch */
        e("jnz  near .not_user\n");
        e("mov  [read_byte], eax\n");
        EmitLabel(".not_user");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }
    else
    {
        e("mov  [read_byte], eax\n");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }

    /* Turbo68KGetReadByte() */

    Align(4);
    EmitGlobalLabel("Turbo68KGetReadByte");
    e("push edx\n");
    if (multiaddr)
    {
        GetArg("edx", 0, 8);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  eax, [super_read_byte]\n");
        e("jmp  near .finish\n");
        EmitLabel(".user");
        e("mov  eax, [user_read_byte]\n");
        EmitLabel(".finish");
    }
    else
        e("mov  eax, [read_byte]\n");
    if (memmap_type == 0)
        e("add  eax, "SIZEOF_DATAREGION"\n");
    e("pop  edx\n");
    e("ret\n");

    /* Turbo68KSetReadWord() */

    Align(4);
    EmitGlobalLabel("Turbo68KSetReadWord");
    e("push eax\n");
    e("push ebx\n");
    GetArg("eax", 0, 12);
    if (memmap_type == 0)
        e("sub  eax, "SIZEOF_DATAREGION"\n");
    if (multiaddr)
    {
        GetArg("edx", 1, 16);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  [super_read_word], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if supervisor, update fetch */
        e("jz   near .not_super\n");
        e("mov  [read_word], eax\n");
        EmitLabel(".not_super");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");

        EmitLabel(".user");
        e("mov  [user_read_word], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if user, update fetch */
        e("jnz  near .not_user\n");
        e("mov  [read_word], eax\n");
        EmitLabel(".not_user");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }
    else
    {
        e("mov  [read_word], eax\n");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }

    /* Turbo68KGetReadWord() */

    Align(4);
    EmitGlobalLabel("Turbo68KGetReadWord");
    e("push edx\n");
    if (multiaddr)
    {
        GetArg("edx", 0, 8);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  eax, [super_read_word]\n");
        e("jmp  near .finish\n");
        EmitLabel(".user");
        e("mov  eax, [user_read_word]\n");
        EmitLabel(".finish");
    }
    else
        e("mov  eax, [read_word]\n");
    if (memmap_type == 0)
        e("add  eax, "SIZEOF_DATAREGION"\n");
    e("pop  edx\n");
    e("ret\n");

    /* Turbo68KSetReadLong() */

    Align(4);
    EmitGlobalLabel("Turbo68KSetReadLong");
    e("push eax\n");
    e("push ebx\n");
    GetArg("eax", 0, 12);
    if (memmap_type == 0)
        e("sub  eax, "SIZEOF_DATAREGION"\n");
    if (multiaddr)
    {
        GetArg("edx", 1, 16);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  [super_read_long], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if supervisor, update fetch */
        e("jz   near .not_super\n");
        e("mov  [read_long], eax\n");
        EmitLabel(".not_super");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");

        EmitLabel(".user");
        e("mov  [user_read_long], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if user, update fetch */
        e("jnz  near .not_user\n");
        e("mov  [read_long], eax\n");
        EmitLabel(".not_user");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }
    else
    {
        e("mov  [read_long], eax\n");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }

    /* Turbo68KGetReadLong() */

    Align(4);
    EmitGlobalLabel("Turbo68KGetReadLong");
    e("push edx\n");
    if (multiaddr)
    {
        GetArg("edx", 0, 8);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  eax, [super_read_long]\n");
        e("jmp  near .finish\n");
        EmitLabel(".user");
        e("mov  eax, [user_read_long]\n");
        EmitLabel(".finish");
    }
    else
        e("mov  eax, [read_long]\n");
    if (memmap_type == 0)
        e("add  eax, "SIZEOF_DATAREGION"\n");
    e("pop  edx\n");
    e("ret\n");

    /* Turbo68KSetWriteByte() */

    Align(4);
    EmitGlobalLabel("Turbo68KSetWriteByte");
    e("push eax\n");
    e("push edx\n");
    GetArg("eax", 0, 12);
    if (memmap_type == 0)
        e("sub  eax, "SIZEOF_DATAREGION"\n");
    if (multiaddr)
    {
        GetArg("edx", 1, 16);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  [super_write_byte], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if supervisor, update fetch */
        e("jz   near .not_super\n");
        e("mov  [write_byte], eax\n");
        EmitLabel(".not_super");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");

        EmitLabel(".user");
        e("mov  [user_write_byte], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if user, update fetch */
        e("jnz  near .not_user\n");
        e("mov  [write_byte], eax\n");
        EmitLabel(".not_user");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }
    else
    {
        e("mov  [write_byte], eax\n");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }

    /* Turbo68KGetWriteByte() */

    Align(4);
    EmitGlobalLabel("Turbo68KGetWriteByte");
    e("push edx\n");
    if (multiaddr)
    {
        GetArg("edx", 0, 8);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  eax, [super_write_byte]\n");
        e("jmp  near .finish\n");
        EmitLabel(".user");
        e("mov  eax, [user_write_byte]\n");
        EmitLabel(".finish");
    }
    else
        e("mov  eax, [write_byte]\n");
    if (memmap_type == 0)
        e("add  eax, "SIZEOF_DATAREGION"\n");
    e("pop  edx\n");
    e("ret\n");

    /* Turbo68KSetWriteWord() */

    Align(4);
    EmitGlobalLabel("Turbo68KSetWriteWord");
    e("push eax\n");
    e("push edx\n");
    GetArg("eax", 0, 12);
    if (memmap_type == 0)
        e("sub  eax, "SIZEOF_DATAREGION"\n");
    if (multiaddr)
    {
        GetArg("edx", 1, 16);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  [super_write_word], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if supervisor, update fetch */
        e("jz   near .not_super\n");
        e("mov  [write_word], eax\n");
        EmitLabel(".not_super");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");

        EmitLabel(".user");
        e("mov  [user_write_word], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if user, update fetch */
        e("jnz  near .not_user\n");
        e("mov  [write_word], eax\n");
        EmitLabel(".not_user");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }
    else
    {
        e("mov  [write_word], eax\n");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }

    /* Turbo68KGetWriteWord() */

    Align(4);
    EmitGlobalLabel("Turbo68KGetWriteWord");
    e("push edx\n");
    if (multiaddr)
    {
        GetArg("edx", 0, 8);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  eax, [super_write_word]\n");
        e("jmp  near .finish\n");
        EmitLabel(".user");
        e("mov  eax, [user_write_word]\n");
        EmitLabel(".finish");
    }
    else
        e("mov  eax, [write_word]\n");
    if (memmap_type == 0)
        e("add  eax, "SIZEOF_DATAREGION"\n");
    e("pop  edx\n");
    e("ret\n");

    /* Turbo68KSetWriteLong() */

    Align(4);
    EmitGlobalLabel("Turbo68KSetWriteLong");
    e("push eax\n");
    e("push edx\n");
    GetArg("eax", 0, 12);
    if (memmap_type == 0)
        e("sub  eax, "SIZEOF_DATAREGION"\n");
    if (multiaddr)
    {
        GetArg("edx", 1, 16);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  [super_write_long], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if supervisor, update fetch */
        e("jz   near .not_super\n");
        e("mov  [write_long], eax\n");
        EmitLabel(".not_super");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");

        EmitLabel(".user");
        e("mov  [user_write_long], eax\n");
        e("test byte [sr+1], 0x20\n");  /* if user, update fetch */
        e("jnz  near .not_user\n");
        e("mov  [write_long], eax\n");
        EmitLabel(".not_user");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }
    else
    {
        e("mov  [write_long], eax\n");
        e("pop  edx\n");
        e("pop  eax\n");
        e("ret\n");
    }

    /* Turbo68KGetWriteLong() */

    Align(4);
    EmitGlobalLabel("Turbo68KGetWriteLong");
    e("push edx\n");
    if (multiaddr)
    {
        GetArg("edx", 0, 8);
        e("cmp  edx, byte %d\n", TURBO68K_SUPERVISOR);
        e("jne  short .user\n");
        e("mov  eax, [super_write_long]\n");
        e("jmp  near .finish\n");
        EmitLabel(".user");
        e("mov  eax, [user_write_long]\n");
        EmitLabel(".finish");
    }
    else
        e("mov  eax, [write_long]\n");
    if (memmap_type == 0)
        e("add  eax, "SIZEOF_DATAREGION"\n");
    e("pop  edx\n");
    e("ret\n");

    /* Turbo68KFetchPtr() */

    Align(4);
    EmitGlobalLabel("Turbo68KFetchPtr");

    e("push ebx\n");
    e("push edx\n");
    e("push esi\n");
    e("push ebp\n");

    GetArg("esi", 0, 20);                   /* address */
    if (multiaddr)
    {
        e("test byte [sr+1], 0x20\n");      /* set proper address space */
        e("jz   short .user\n");
        SetSupervisorAddressSpace();
        e("jmp  short .continue\n");
        EmitLabel(".user");
        SetUserAddressSpace();
        EmitLabel(".continue");
    }
    AddrClip("esi");
    e("mov     edx, [fetch]\n");
    e(".find_fetch_loop:\n");
    e("add     edx, byte "SIZEOF_FETCHREGION"\n");
    e("cmp     dword [edx], byte -1\n");    /* limit=-1? end. */
    e("je      short .not_found\n");
    e("cmp     esi, [edx]\n");              /* offset 0: base. above it? */
    e("jb      short .find_fetch_loop\n");  /* nope, not this region */
    e("cmp     esi, [edx+"OFFSET_FETCH_LIMIT"]\n"); /* limit. below it? */
    e("ja      short .find_fetch_loop\n");  /* nope, not this region */
    e("mov     ebp, [edx+"OFFSET_FETCH_PTR"]\n");   /* ebp=base ptr */
    e("add     esi, ebp\n");                        /* +pc=pc ptr.. */
    e("mov     eax, esi\n");

    e("pop  ebp\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ebx\n");

    e("ret\n");
    EmitLabel(".not_found");

    e("pop  ebp\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ebx\n");

    e("xor  eax, eax\n");
    e("ret\n");

    /* Turbo68KReadByte() */

    Align(4);
    EmitGlobalLabel("Turbo68KReadByte");
    e("push ebx\n");
    e("push ecx\n");
    e("push edx\n");
    e("push esi\n");
    e("push edi\n");
    e("push ebp\n");
    e("xor  ebp, ebp\n");                   /* so PC doesn't get trashed */
    e("mov  esi, [pc]\n");
    if (multiaddr)
    {
        e("test byte [sr+1], 0x20\n");      /* set proper address space */
        e("jz   short .user\n");
        SetSupervisorAddressSpace();
        e("jmp  short .continue\n");
        EmitLabel(".user");
        SetUserAddressSpace();
        EmitLabel(".continue");
    }
    GetArg("ebx", 0, 28);
    ReadByte();
    e("mov  eax, edx\n");
    if (mmx)    e("emms\n");
    e("pop  ebp\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ecx\n");
    e("pop  ebx\n");
    e("ret\n");
    
    /* Turbo68KReadWord() */

    Align(4);
    EmitGlobalLabel("Turbo68KReadWord");
    e("push ebx\n");
    e("push ecx\n");
    e("push edx\n");
    e("push esi\n");
    e("push edi\n");
    e("push ebp\n");
    e("xor  ebp, ebp\n");                   /* so PC doesn't get trashed */
    e("mov  esi, [pc]\n");
    if (multiaddr)
    {
        e("test byte [sr+1], 0x20\n");      /* set proper address space */
        e("jz   short .user\n");
        SetSupervisorAddressSpace();
        e("jmp  short .continue\n");
        EmitLabel(".user");
        SetUserAddressSpace();
        EmitLabel(".continue");
    }
    GetArg("ebx", 0, 28);
    ReadWord();
    e("mov  eax, edx\n");
    if (mmx)    e("emms\n");
    e("pop  ebp\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ecx\n");
    e("pop  ebx\n");
    e("ret\n");

    /* Turbo68KReadLong() */

    Align(4);
    EmitGlobalLabel("Turbo68KReadLong");
    e("push ebx\n");
    e("push ecx\n");
    e("push edx\n");
    e("push esi\n");
    e("push edi\n");
    e("push ebp\n");
    e("xor  ebp, ebp\n");                   /* so PC doesn't get trashed */
    e("mov  esi, [pc]\n");
    if (multiaddr)
    {
        e("test byte [sr+1], 0x20\n");      /* set proper address space */
        e("jz   short .user\n");
        SetSupervisorAddressSpace();
        e("jmp  short .continue\n");
        EmitLabel(".user");
        SetUserAddressSpace();
        EmitLabel(".continue");
    }
    GetArg("ebx", 0, 28);
    ReadLong();
    e("mov  eax, edx\n");
    if (mmx)    e("emms\n");
    e("pop  ebp\n");
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  edx\n");
    e("pop  ecx\n");
    e("pop  ebx\n");
    e("ret\n");

    /* Turbo68KWriteByte() */

    Align(4);
    EmitGlobalLabel("Turbo68KWriteByte");
    e("pusha\n");
    e("xor  ebp, ebp\n");                   /* so PC doesn't get trashed */
    e("mov  esi, [pc]\n");
    if (multiaddr)
    {
        e("test byte [sr+1], 0x20\n");      /* set proper address space */
        e("jz   short .user\n");
        SetSupervisorAddressSpace();
        e("jmp  short .continue\n");
        EmitLabel(".user");
        SetUserAddressSpace();
        EmitLabel(".continue");
    }
    GetArg("ebx", 0, 36);
    GetArg("edx", 1, 40);
    WriteByte();
    if (mmx)    e("emms\n");
    e("popa\n");
    e("ret\n");
                   
    /* Turbo68KWriteWord() */

    Align(4);
    EmitGlobalLabel("Turbo68KWriteWord");
    e("pusha\n");
    e("xor  ebp, ebp\n");                   /* so PC doesn't get trashed */
    e("mov  esi, [pc]\n");
    if (multiaddr)
    {
        e("test byte [sr+1], 0x20\n");      /* set proper address space */
        e("jz   short .user\n");
        SetSupervisorAddressSpace();
        e("jmp  short .continue\n");
        EmitLabel(".user");
        SetUserAddressSpace();
        EmitLabel(".continue");
    }
    GetArg("ebx", 0, 36);
    GetArg("edx", 1, 40);
    WriteWord();
    if (mmx)    e("emms\n");
    e("popa\n");
    e("ret\n");

    /* Turbo68KWriteLong() */

    Align(4);
    EmitGlobalLabel("Turbo68KWriteLong");
    e("pusha\n");
    e("xor  ebp, ebp\n");                   /* so PC doesn't get trashed */
    e("mov  esi, [pc]\n");
    if (multiaddr)
    {
        e("test byte [sr+1], 0x20\n");      /* set proper address space */
        e("jz   short .user\n");
        SetSupervisorAddressSpace();
        e("jmp  short .continue\n");
        EmitLabel(".user");
        SetUserAddressSpace();
        EmitLabel(".continue");
    }
    GetArg("ebx", 0, 36);
    GetArg("edx", 1, 40);
    WriteLong();
    if (mmx)    e("emms\n");
    e("popa\n");
    e("ret\n");

    /* Turbo68KSet/GetContext() */

    /*
     * Notice the trick here, all of the memory map pointers have 16
     * subtracted from them, this helps optimize the read/write handlers
     * since an "add edi, 16" is possible straight off. This was NB's
     * idea. The fetch area is also handled this way
     */

    Align(4);
    EmitGlobalLabel("Turbo68KSetContext");
    e("push ecx\n");
    e("push esi\n");
    e("push edi\n");
    GetArg("esi", 0, 16);
    e("mov  edi, %s%s\n", id, NameOfContext());
    if (mmx && !(SizeOfContext() & 7))
    {
        e("mov  ecx, " SIZEOF_CONTEXT " / 8\n");
        EmitLabel(".l");
        e("movq mm6, [esi]\n");
        e("movq [edi], mm6\n");
        e("add  esi, byte 8\n");
        e("add  edi, byte 8\n");
        e("dec  ecx\n");
        e("jnz  short .l\n");
        e("emms\n");
    }
    else
    {
        e("mov  ecx, " SIZEOF_CONTEXT " / 4\n");
        e("cld\n");
        e("rep  movsd\n");
    }

    e("sub  dword [fetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [pcfetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [super_fetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [super_pcfetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [user_fetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [user_pcfetch], "SIZEOF_FETCHREGION"\n");
    if (memmap_type == 0)
    {
        e("sub  dword [read_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [read_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [read_long], "SIZEOF_DATAREGION"\n");
        e("sub  dword [write_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [write_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [write_long], "SIZEOF_DATAREGION"\n");

        e("sub  dword [super_read_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_read_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_read_long], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_write_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_write_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_write_long], "SIZEOF_DATAREGION"\n");

        e("sub  dword [user_read_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_read_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_read_long], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_write_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_write_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_write_long], "SIZEOF_DATAREGION"\n");
    }
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  ecx\n");
    e("ret\n");

    Align(4);
    EmitGlobalLabel("Turbo68KGetContext");
    e("push ecx\n");
    e("push esi\n");
    e("push edi\n");

    e("add  dword [fetch], "SIZEOF_FETCHREGION"\n");
    e("add  dword [pcfetch], "SIZEOF_FETCHREGION"\n");
    e("add  dword [super_fetch], "SIZEOF_FETCHREGION"\n");
    e("add  dword [super_pcfetch], "SIZEOF_FETCHREGION"\n");
    e("add  dword [user_fetch], "SIZEOF_FETCHREGION"\n");
    e("add  dword [user_pcfetch], "SIZEOF_FETCHREGION"\n");
    if (memmap_type == 0)
    {
        e("add  dword [read_byte], "SIZEOF_DATAREGION"\n");
        e("add  dword [read_word], "SIZEOF_DATAREGION"\n");
        e("add  dword [read_long], "SIZEOF_DATAREGION"\n");
        e("add  dword [write_byte], "SIZEOF_DATAREGION"\n");
        e("add  dword [write_word], "SIZEOF_DATAREGION"\n");
        e("add  dword [write_long], "SIZEOF_DATAREGION"\n");

        e("add  dword [super_read_byte], "SIZEOF_DATAREGION"\n");
        e("add  dword [super_read_word], "SIZEOF_DATAREGION"\n");
        e("add  dword [super_read_long], "SIZEOF_DATAREGION"\n");
        e("add  dword [super_write_byte], "SIZEOF_DATAREGION"\n");
        e("add  dword [super_write_word], "SIZEOF_DATAREGION"\n");
        e("add  dword [super_write_long], "SIZEOF_DATAREGION"\n");

        e("add  dword [user_read_byte], "SIZEOF_DATAREGION"\n");
        e("add  dword [user_read_word], "SIZEOF_DATAREGION"\n");
        e("add  dword [user_read_long], "SIZEOF_DATAREGION"\n");
        e("add  dword [user_write_byte], "SIZEOF_DATAREGION"\n");
        e("add  dword [user_write_word], "SIZEOF_DATAREGION"\n");
        e("add  dword [user_write_long], "SIZEOF_DATAREGION"\n");
    }
    GetArg("edi", 0, 16);
    e("mov  esi, %s%s\n", id, NameOfContext());
    if (mmx && !(SizeOfContext() & 7))
    {
        e("mov  ecx, " SIZEOF_CONTEXT " / 8\n");
        EmitLabel(".l");
        e("movq mm6, [esi]\n");
        e("movq [edi], mm6\n");
        e("add  esi, byte 8\n");
        e("add  edi, byte 8\n");
        e("dec  ecx\n");
        e("jnz  short .l\n");
        e("emms\n");
    }
    else
    {
        e("mov  ecx, " SIZEOF_CONTEXT " / 4\n");
        e("cld\n");
        e("rep  movsd\n");
    }

    e("sub  dword [fetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [pcfetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [super_fetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [super_pcfetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [user_fetch], "SIZEOF_FETCHREGION"\n");
    e("sub  dword [user_pcfetch], "SIZEOF_FETCHREGION"\n");
    if (memmap_type == 0)
    {
        e("sub  dword [read_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [read_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [read_long], "SIZEOF_DATAREGION"\n");
        e("sub  dword [write_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [write_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [write_long], "SIZEOF_DATAREGION"\n");

        e("sub  dword [super_read_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_read_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_read_long], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_write_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_write_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [super_write_long], "SIZEOF_DATAREGION"\n");

        e("sub  dword [user_read_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_read_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_read_long], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_write_byte], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_write_word], "SIZEOF_DATAREGION"\n");
        e("sub  dword [user_write_long], "SIZEOF_DATAREGION"\n");
    }
    e("pop  edi\n");
    e("pop  esi\n");
    e("pop  ecx\n");
    e("ret\n");

    /* Turbo68KGetContextSize() */
    EmitGlobalLabel("Turbo68KGetContextSize");
    e("mov  eax, "SIZEOF_CONTEXT"\n");
    e("ret\n");

    /* Turbo68KClearCycles */
    EmitGlobalLabel("Turbo68KClearCycles");
    e("mov  dword [cycles], 0\n");
    e("mov  dword [remaining], 0\n");
    e("ret\n");

    /* Turbo68KFreeTimeSlice */
    EmitGlobalLabel("Turbo68KFreeTimeSlice");
    e("push eax\n");
    e("mov  eax, [cycles]\n");
    e("sub  eax, [remaining]\n");
    e("mov  [cycles], eax\n");
    e("mov  dword [remaining], 0\n");
    e("pop  eax\n");
    e("ret\n");

    /* Turbo68KGetElapsedCycles */
    Align(4);
    EmitGlobalLabel("Turbo68KGetElapsedCycles");
    e("mov  eax, [cycles]\n");
    e("sub  eax, [remaining]\n");
    e("ret\n");

    /* Turbo68KProcessInterrupts() */

    Align(4);
    EmitGlobalLabel("Turbo68KProcessInterrupts");
    e("pusha\n");
    e("test byte [status], 1\n");
    e("jnz  near .end\n");              /* does not work while running */
    e("mov  ecx, [remaining]\n");
    EMULATING();                        /* running=1 */
    e("mov  esi, [pc]\n");              /* fetch PC */
    UpdateFetchPtr();
    e("call ProcessInterrupts\n");
    STOP_RUNNING();                     /* save PC and cycles remaining */
    STOP_EMULATING();
    EmitLabel(".end");
    if (mmx)    e("emms\n");
    e("popa\n");
    e("xor  eax, eax\n");
    e("ret\n");

    /* Turbo68KInterrupt() */

    /*
     * Note: This function assumes that TURBO68K_AUTOVECTOR == 256, make
     * sure the definition in TURBO68K.H reflects this
     *
     * If 68K is halted (STOP instruction), if an interrupt with a priority
     * equal to or lower than the SR priority is requested, is the interrupt
     * made pending or discarded? (ask) Are there any situations when invalid-
     * priority interrupts are discarded completely?
     */

    Align(4);
    EmitGlobalLabel("Turbo68KInterrupt");
    e("push\tebx\n");
    e("push\tecx\n");
    e("push\tedx\n");
    e("push\tesi\n");
    e("push\tedi\n");

    GetArg("eax", 0, 24);

    e("test eax, eax\n");               /* 0 is not a valid level */
    e("jz   short .inv_level\n");
    e("cmp  eax, byte 7\n");            /* if greater than 7, invalid level */
    e("ja   short .inv_level\n");

    GetArg("ebx", 1, 28);               /* get vector */
    e("cmp  ebx, byte 2\n");
    e("jb   short .inv_vector\n");
    e("cmp  ebx, 256\n");
    e("ja   short .inv_vector\n");
    e("jne  short .not_auto\n");
    e("mov  ebx, eax\n");
    e("add  ebx, byte 24\n");
    EmitLabel(".not_auto");
    e("dec  eax\n");                    /* (level-1)*4=offset into intr[] */
    e("shl  eax, byte 2\n");
    e("cmp  [intr+eax], byte 0\n");
    e("jne  short .pending\n");         /* already pending */
    e("mov  [intr+eax], ebx\n");        /* store vector into intr[] */
    e("inc  dword [intr+7*4]\n");       /* one interrupt added */
    e("pop\tedi\n");
    e("pop\tesi\n");
    e("pop\tedx\n");
    e("pop\tecx\n");
    e("pop\tebx\n");
    e("xor  eax, eax\n");
    e("ret\n");

    EmitLabel(".inv_level");
    e("mov  eax, %d\n", TURBO68K_ERROR_INTLEVEL);
    e("pop\tedi\n");
    e("pop\tesi\n");
    e("pop\tedx\n");
    e("pop\tecx\n");
    e("pop\tebx\n");
    e("ret\n");
    EmitLabel(".inv_vector");
    e("mov  eax, %d\n", TURBO68K_ERROR_INTVECTOR);
    e("pop\tedi\n");
    e("pop\tesi\n");
    e("pop\tedx\n");
    e("pop\tecx\n");
    e("pop\tebx\n");
    e("ret\n");
    EmitLabel(".pending");
    e("mov  eax, %d\n", TURBO68K_ERROR_INTPENDING);
    e("pop\tedi\n");
    e("pop\tesi\n");
    e("pop\tedx\n");
    e("pop\tecx\n");
    e("pop\tebx\n");
    e("ret\n");

    /* Turbo68KCancelInterrupt */

    EmitGlobalLabel("Turbo68KCancelInterrupt");
    e("push\tebx\n");
    e("push\tecx\n");
    e("push\tedx\n");
    e("push\tesi\n");
    e("push\tedi\n");
    GetArg("eax", 0, 24);
    e("test eax, eax\n");        /* 0 is not a valid level */
    e("jz   short .inv_level\n");
    e("cmp  eax, byte 7\n");            /* if greater than 7, invalid level */
    e("ja   short .inv_level\n");
    e("test byte [status], 0x04\n");    /* processing interrupts? */
    e("jnz  short .busy\n");            /* yep... can't do this */
    e("dec  eax\n");                    /* make index into intr[] */
    e("cmp  dword [intr+eax*4], byte 0\n");
    e("je   .nothing_to_cancel\n");
    e("mov  dword [intr+eax*4], 0\n");
    e("dec  dword [intr+7*4]\n");       /* removed 1 interrupt */
    EmitLabel(".busy");
    EmitLabel(".nothing_to_cancel");

    e("pop\tedi\n");
    e("pop\tesi\n");
    e("pop\tedx\n");
    e("pop\tecx\n");
    e("pop\tebx\n");
    e("xor  eax, eax\n");
    e("ret\n");
    EmitLabel(".inv_level");
    e("pop\tedi\n");
    e("pop\tesi\n");
    e("pop\tedx\n");
    e("pop\tecx\n");
    e("pop\tebx\n");
    e("mov  eax, %d\n", TURBO68K_ERROR_INTLEVEL);
    e("ret\n");

    /*
     * ProcessInterrupts: Processes any pending interrupts if allowed.
     * Assumes all registers have been set up as per Turbo68KRun
     */

    Align(4);
    EmitLabel("ProcessInterrupts");
    INTERRUPT_PROCESSING();
    e("cmp  dword [intr+7*4], byte 0\n");
    e("je   near .no_int_end\n");       /* no interrupts pending */
    e("push eax\n");                    /* EAX=flags, save it! */
    e("mov  al, [sr+1]\n");
    e("and  eax, byte 7\n");            /* EAX=interrupt priority */
    e("cmp  al, 7\n");
    e("jne  short .no_int7\n");
    e("mov  edi, intr+6*4\n");
    e("jmp  short .l\n");
    EmitLabel(".no_int7");
    e("mov  edi, eax\n");
    e("shl  edi, byte 2\n");            /* *4, index into intr[] */
    e("add  edi, intr\n");              /* EDI=interrupt queue */
    e("inc  al\n");                     /* new SR priority mask */
    EmitLabel(".l");
    e("cmp  dword [edi], byte 0\n");
    e("je   near .skip\n");             /* no interrupt at this level */

    e("test byte [sr+1], 0x20\n");      /* see if in User mode */
    e("jnz  short .no_sp_magic\n");     /* nope, don't need to swap SPs */
    e("mov  edx, [__sp]\n");
    e("xchg [a+7*4], edx\n");
    e("mov  [__sp], edx\n");
    SetSupervisorAddressSpace();
    EmitLabel(".no_sp_magic");
    
    e("mov  ebx, [a+7*4]\n");           /* SP */
    /*
     * If 68010, we have to push the format word on the stack.
     */
    if (mpu == 68010)
    {
        e("mov  edx, [edi]\n"); /* vector number */
        e("sub  ebx, byte 2\n");/* SP-2 */
        e("shl  edx, byte 2\n");/* vector*4 for offset */
        e("push ebx\n");
        e("push edi\n");
        WriteWord();
        e("pop  edi\n");
        e("pop  ebx\n");
    }
    e("mov  edx, [pc]\n");              /* get old PC */
    e("sub  ebx, byte 4\n");            /* SP-4 */
    e("push ebx\n");
    e("push edi\n");
    WriteLong();
    e("pop  edi\n");
    e("pop  ebx\n");
    e("mov  edx, [sr]\n");              /* get SR */
    e("sub  ebx, byte 2\n");            /* SP-2 */
    e("push edi\n");
    e("push ebx\n");
    WriteWord();                        /* save SR to stack */
    e("pop  ebx\n");
    e("pop  edi\n");
    e("mov  dh, al\n");                 /* new interrupt priority bits */
    e("or   dh, 0x20\n");               /* set supervisor for exception */
    e("and  edx, 0xa71f\n");            /* clear unwanted bits */
    e("mov  [sr], edx\n");              /* set new SR */
    e("mov  [a+7*4], ebx\n");           /* write back SP */
    e("mov  ebx, [edi]\n");             /* vector of interrupt */

    e("test dword [InterruptAcknowledge], -1\n");   /* interrupt call-back */
    e("jz   short .no_interrupt_ack_handler\n");
    e("pushad\n");
    e("push ebx\n");
    e("call dword [InterruptAcknowledge]\n");
    e("add  esp, byte 4\n");
    e("popad\n");
    e(".no_interrupt_ack_handler:\n");

    e("shl  ebx, byte 2\n");            /* vector *= 4 */
    if (mpu == 68010)
        e("add  ebx, [vbr]\n");
    e("push edi\n");
    ReadLong();                         /* get PC */
    e("pop  edi\n");
    e("mov  [pc], edx\n");              /* set new PC */
    e("mov  dword [edi], 0\n");         /* done w/ this interrupt */
    e("dec  dword [intr+7*4]\n");       /* processed 1 interrupt... */
    e("sub  ecx, byte 44\n");           /* interrupts take 44 clocks */
    UNSTOP_CPU();
    EmitLabel(".skip");
    e("add  edi, byte 4\n");
    e("inc  al\n");                     /* next entry */
    e("cmp  edi, intr+7*4\n");
    e("jne  near .l\n");
    e("mov  esi, [pc]\n");
    e("pop  eax\n");                    /* get flags back */
    UpdateFetchPtr();
    EmitLabel(".no_int_end");
    INTERRUPT_DONE();
    e("ret\n");


    /* Turbo68KRun() */

    Align(4);
    EmitGlobalLabel("Turbo68KRun");
    e("pusha\n");
    GetArg("ecx", 0, 36);   /* ECX = cycles */
    e("mov  [cycles], ecx\n");
    e("mov  [remaining], ecx\n");
    e("test byte [sr+1], 0x20\n");
    e("jz   near .set_user_space\n");
    SetSupervisorAddressSpace();
    e("jmp  near .continue\n");
    EmitLabel(".set_user_space");
    SetUserAddressSpace();
    EmitLabel(".continue");
    EMULATING();                        /* running=1 */
    e("mov  esi, [pc]\n");              /* fetch PC */
    UpdateFetchPtr();
    LoadCCR();                          /* get flags */
    e("call ProcessInterrupts\n");
    e("test byte [status], 2\n");       /* stopped? */
    e("jnz  short .stopped\n");
    e("xor  edi, edi\n");
    e("mov  di, [esi]\n");              /* next instruction */
    e("add  esi, byte 2\n");    
    e("jmp  dword [jmptab+edi*4]\n");   /* jump... */
    EmitLabel(".stopped");
    e("xor  ecx, ecx\n");
    e("jmp  Turbo68KRun_done\n");

    Align(4);
    EmitLabel("Turbo68KRun_done");
    SaveCCR();                          /* save flags */
    STOP_RUNNING();                     /* save PC and cycles */
    STOP_EMULATING();
    if (mmx)    e("emms\n");
    e("popa\n");
    e("xor  eax, eax\n");               /* everything okay... */
    e("ret\n");

    /*
     * NOTE: Only the 68010 RTE handler can call this. PC is assumed to point
     * one word after the faulty RTE.
     */

    if (mpu == 68010)
    {
        EmitLabel("stackframe_error");
        e("sub  esi, byte 2\n");        /* point at faulty RTE */
        SaveCCR();
        STOP_EMULATING();
        STOP_RUNNING();                 /* save PC and cycles remaining */
        e("popa\n");
        e("mov  eax, %d\n", TURBO68K_ERROR_STACKFRAME);
        e("ret\n");
    }


    if (memmap_type == 2)   /* low level handling? don't generate the rest */
        return;

    /*
     * ReadXXXXPC: EBX = address
     * Out:        EDX = data; DL=byte, EDX=upper 32-bits trashed, DX=word,
     *             EDX=long-word
     *             EBX=address
     * Notes:      EDI is trashed in the process, but cleared at the end.
     *             These functions read from the *pcfetch areas. There are no
     *             SX functions, this is handled by the emitter. I did this to
     *             save space.
     */
    
    if (pcfetch)
    {
        for (i = 0; i < 3; i++) /* 0=byte, 1=word, 2=dword */
        {          
            CacheAlign();
            switch (i)
            {
            case 0:
                EmitLabel("ReadBytePC");
                break;
            case 1:
                EmitLabel("ReadWordPC");
                break;
            case 2:
                EmitLabel("ReadLongPC");
                break;
            }
            e("mov  edi, [pcfetch]\n");
            SaveReg("memhandler", "ebx");           /* save address */
            AddrClip("ebx");
            EmitLabel(".loop");
            e("add     edi, byte "SIZEOF_FETCHREGION"\n");
            e("cmp     dword [edi], byte -1\n");    /* base=-1? end. */
            e("je      short .not_found\n");
            e("cmp     ebx, [edi]\n");              /* offset 0: base. above it? */
            e("jb      short .loop\n");             /* nope, not this region */
            e("cmp     ebx, [edi+"OFFSET_FETCH_LIMIT"]\n");  /* limit. below it? */
            e("ja      short .loop\n");             /* nope, not this region */
            e("mov     edx, ebx\n");
            if (!i)     /* byte accesses need this because buffer is swapped */
                e("xor     dl, 1\n");
            e("add     edx, [edi+"OFFSET_FETCH_PTR"]\n");    /* into ptr.. */
            if (!i)         /* byte */
                e("mov     dl, [edx]\n");           /* fetch */
            else if (i == 1)    /* word */
                e("mov     edx, [edx]\n");
            else            /* long */
            {
                e("mov     edx, [edx]\n");
                e("rol     edx, byte 16\n");        /* swap words */
            }
            e("xor  edi, edi\n");
            RestoreReg("memhandler", "ebx");
            e("ret\n");
            Align(4);
            EmitLabel(".not_found");
            e("xor  edi, edi\n");
            e("xor  edx, edx\n");
            e("dec  edx\n");    /* not found, return all 1s */
            RestoreReg("memhandler", "ebx");
            e("ret\n");
        }
    }

    /*
     * ReadXXXX: EBX = address
     * Out:      EDX = data; DL=byte, EDX=upper 32-bits trashed, DX=word,
     *           EDX=long-word
     *           EBX=address
     * Notes:    EDI is trashed in the process, but cleared at the end.
     *           This applies to all ReadXXX and WriteXXX handlers, some
     *           instructions (NEGX) rely on this behavior.
     */
    
    if (memmap_type == 0)       /* default memory mapping system */
    {
        for (i = 0; i < 3; i++) /* 0=byte, 1=word, 2=dword */
        {          
            CacheAlign();
            switch (i)
            {
            case 0:
                EmitLabel("ReadByte");
                e("mov  edi, [read_byte]\n");
                break;
            case 1:
                EmitLabel("ReadWord");
                e("mov  edi, [read_word]\n");
                break;
            case 2:
                EmitLabel("ReadLong");
                e("mov  edi, [read_long]\n");
                break;
            }
            SaveReg("memhandler", "ebx");           /* save address */
            AddrClip("ebx");
            EmitLabel(".loop");
            e("add     edi, byte "SIZEOF_DATAREGION"\n");
            e("cmp     dword [edi], byte -1\n");    /* base=-1? end. */
            e("je      short .not_found\n");
            e("cmp     ebx, [edi]\n");              /* offset 0: base. above it? */
            e("jb      short .loop\n");             /* nope, not this region */
            e("cmp     ebx, [edi+"OFFSET_DATA_LIMIT"]\n");  /* limit. below it? */
            e("ja      short .loop\n");             /* nope, not this region */
            e("cmp     dword [edi+"OFFSET_DATA_PTR"], byte 0\n");
            e("je      short .read_from_handler\n");
            e("mov     edx, ebx\n");
            if (!i)     /* byte accesses need this because buffer is swapped */
                e("xor     dl, 1\n");
            e("add     edx, [edi+"OFFSET_DATA_PTR"]\n");    /* into ptr.. */
            if (!i)         /* byte */
                e("mov     dl, [edx]\n");           /* fetch */
            else if (i == 1)    /* word */
                e("mov     edx, [edx]\n");
            else            /* long */
            {
                e("mov     edx, [edx]\n");
                e("rol     edx, byte 16\n");        /* swap words */
            }
            e("xor  edi, edi\n");
            RestoreReg("memhandler", "ebx");
            e("ret\n");
            Align(4);
            EmitLabel(".not_found");
            e("xor  edi, edi\n");
            e("xor  edx, edx\n");
            e("dec  edx\n");    /* not found, return all 1s */
            RestoreReg("memhandler", "ebx");
            e("ret\n");
            Align(4);
            EmitLabel(".read_from_handler");
            SaveReg("memhandler", "eax");
            SaveReg("memhandler", "ebp");
            WRITEPCTOMEM("pc");                     /* saves esi */
            e("mov     [remaining], ecx\n");

            /* address */
            if (!call_convention)                   /* stack-based */
                e("push ebx\n"); 
            else                                    /* register-based */
                e("mov  eax, ebx\n");
            
            e("call    dword [edi+"OFFSET_DATA_HANDLER"]\n");      

            if (!call_convention)
                e("add     esp, byte 4\n");

            e("mov     edx, eax\n");                /* get the data */
            e("mov     esi, [pc]\n");       /* handler could have changed PC */
            e("mov     ecx, [remaining]\n");/* this could have been changed */
            RestoreReg("memhandler", "eax");
            RestoreReg("memhandler", "ebp");
            e("add     esi, ebp\n");        /* base+pc=pc pointer */
            e("xor     edi, edi\n");        /* keep clear for fetch */
            RestoreReg("memhandler", "ebx");
            e("ret\n");
        }
    }
    else if (memmap_type == 1)  /* high level handler */
    {
        for (i = 0; i < 3; i++)
        {
            CacheAlign();
            switch (i)
            {
            case 0: EmitLabel("ReadByte"); break;
            case 1: EmitLabel("ReadWord"); break;
            case 2: EmitLabel("ReadLong"); break;
            }            
            SaveReg("memhandler", "ebx");           /* save address */
            AddrClip("ebx");
            SaveReg("memhandler", "eax");
            SaveReg("memhandler", "ebp");
            WRITEPCTOMEM("pc");                     /* saves esi */
            e("mov     [remaining], ecx\n");

            /* address */
            if (!call_convention)                   /* stack-based */
                e("push ebx\n"); 
            else                                    /* register-based */
                e("mov  eax, ebx\n");

            switch (i)
            {
            case 0: e("call dword [read_byte]\n"); break;
            case 1: e("call dword [read_word]\n"); break;
            case 2: e("call dword [read_long]\n"); break;
            }

            if (!call_convention)
                e("add     esp, byte 4\n");

            e("mov     edx, eax\n");                /* get the data */
            e("mov     esi, [pc]\n");       /* handler could have changed PC */
            e("mov     ecx, [remaining]\n");/* this could have been changed */
            RestoreReg("memhandler", "eax");
            RestoreReg("memhandler", "ebp");
            e("add     esi, ebp\n");        /* base+pc=pc pointer */
            e("xor     edi, edi\n");        /* keep clear for fetch */
            RestoreReg("memhandler", "ebx");
            e("ret\n");
        }
    }

    if (memmap_type == 0)
    {
        /*
         * ReadWordSX is the same as ReadWord except that it sign extends the
         * word to 32 bits.
         */

        CacheAlign();
        EmitLabel("ReadWordSX");
        e("mov  edi, [read_word]\n");
        SaveReg("memhandler", "ebx");       /* save address */
        AddrClip("ebx");
        EmitLabel(".loop");        
        e("add     edi, byte "SIZEOF_DATAREGION"\n");
        e("cmp     dword [edi], byte -1\n");
        e("je      short .not_found\n");
        e("cmp     ebx, [edi]\n");
        e("jb      short .loop\n");
        e("cmp     ebx, [edi+"OFFSET_DATA_LIMIT"]\n");
        e("ja      short .loop\n");
        e("cmp     dword [edi+"OFFSET_DATA_PTR"], byte 0\n");
        e("je      short .read_from_handler\n");
        e("mov     edx, ebx\n");
        e("add     edx, [edi+"OFFSET_DATA_PTR"]\n");
        e("movsx   edx, word [edx]\n"); /* sign extend! */
        e("xor  edi, edi\n");
        RestoreReg("memhandler", "ebx");
        e("ret\n");
        Align(4);
        EmitLabel(".not_found");
        e("xor  edi, edi\n");
        e("xor  edx, edx\n");
        e("dec  edx\n");    /* not found, return all 1s */
        RestoreReg("memhandler", "ebx");
        e("ret\n");
        Align(4);
        EmitLabel(".read_from_handler");
        SaveReg("memhandler", "eax");
        SaveReg("memhandler", "ebp");
        WRITEPCTOMEM("pc");
        e("mov     [remaining], ecx\n");

        /* address */
        if (!call_convention)                   /* stack-based */
            e("push ebx\n"); 
        else                                    /* register-based */
            e("mov  eax, ebx\n");

        e("call    dword [edi+"OFFSET_DATA_HANDLER"]\n");      

        if (!call_convention)
            e("add     esp, byte 4\n");

        e("movsx   edx, ax\n");                 /* sign extend! */
        e("mov     esi, [pc]\n");
        e("mov     ecx, [remaining]\n");
        RestoreReg("memhandler", "eax");
        RestoreReg("memhandler", "ebp");
        e("add     esi, ebp\n");
        e("xor     edi, edi\n");
        RestoreReg("memhandler", "ebx");
        e("ret\n");
    }
    else if (memmap_type == 1)
    {
        CacheAlign();
        EmitLabel("ReadWordSX");
        SaveReg("memhandler", "ebx");           /* save address */
        AddrClip("ebx");
        SaveReg("memhandler", "eax");
        SaveReg("memhandler", "ebp");
        WRITEPCTOMEM("pc");                     /* saves esi */
        e("mov     [remaining], ecx\n");

        /* address */
        if (!call_convention)                   /* stack-based */
            e("push ebx\n"); 
        else                                    /* register-based */
            e("mov  eax, ebx\n");

        e("call dword [read_word]\n");

        if (!call_convention)
            e("add     esp, byte 4\n");

        e("movsx    edx, ax\n");                /* sign extend */

        e("mov     esi, [pc]\n");       /* handler could have changed PC */
        e("mov     ecx, [remaining]\n");/* this could have been changed */
        RestoreReg("memhandler", "eax");
        RestoreReg("memhandler", "ebp");
        e("add     esi, ebp\n");        /* base+pc=pc pointer */
        e("xor     edi, edi\n");        /* keep clear for fetch */
        RestoreReg("memhandler", "ebx");
        e("ret\n");
    }

    /*
     * WriteXXXX: EBX = address
     * In:        EDX = data; DL=byte, EDX{DX}=word,
     *            EDX=long-word
     * Out:       EBX=trashed, EDX=preserved
     * Notes:     EDI is trashed in the process, but cleared at the end
     */
    
    if (memmap_type == 0)
    {
        for (i = 0; i < 3; i++) /* 0=byte, 1=word, 2=dword */
        {          
            CacheAlign();
            switch (i)
            {
            case 0:
                EmitLabel("WriteByte");
                e("mov  edi, [write_byte]\n");
                break;
            case 1:
                EmitLabel("WriteWord");
                e("mov  edi, [write_word]\n");
                break;
            case 2:
                EmitLabel("WriteLong");
                e("mov  edi, [write_long]\n");
                break;
            }
            AddrClip("ebx");
            EmitLabel(".loop");
            e("add     edi, byte "SIZEOF_DATAREGION"\n");
            e("cmp     dword [edi], byte -1\n");    /* base=-1? end. */
            e("je      short .not_found\n");
            e("cmp     ebx, [edi]\n");              /* offset 0: base. above it? */
            e("jb      short .loop\n");             /* nope, not this region */
            e("cmp     ebx, [edi+"OFFSET_DATA_LIMIT"]\n");  /* limit. below it? */
            e("ja      short .loop\n");             /* nope, not this region */
            e("cmp     dword [edi+"OFFSET_DATA_PTR"], byte 0\n");
            e("je      short .write_with_handler\n");
            if (!i)         /* byte accesses need this because buffer is swapped */
                e("xor     bl, 1\n");
            e("add     ebx, [edi+"OFFSET_DATA_PTR"]\n");    /* into ptr.. */
            if (!i)         /* byte */
                e("mov     [ebx], dl\n");           /* write */          
            else if (i == 1)    /* word */
                e("mov     [ebx], dx\n");
            else            /* long */
            {
                e("ror     edx, byte 16\n");        /* swap words */
                e("mov     [ebx], edx\n");
                e("ror     edx, byte 16\n");        /* reswap back */
            }
            EmitLabel(".not_found");
            e("xor  edi, edi\n");
            RestoreReg("memhandler", "ebx");
            e("ret\n");
            Align(4);
            EmitLabel(".write_with_handler");
            SaveReg("memhandler", "eax");
            SaveReg("memhandler", "ebp");
            WRITEPCTOMEM("pc");                     /* saves esi */
            e("mov     [remaining], ecx\n");
            SaveReg("memhandler", "edx");           /* save params */

            /* data, address */
            if (!call_convention)                   /* stack-based */
            {
                e("push edx\n");
                e("push ebx\n");
            }
            else                                    /* register-based */
            {
                e("mov  eax, ebx\n");
                /* don't need: e("mov  edx, edx\n"); */
            }

            e("call    dword [edi+"OFFSET_DATA_HANDLER"]\n");      

            if (!call_convention)
                e("add     esp, byte 8\n");

            RestoreReg("memhandler", "edx");        /* restore params */
            e("mov     esi, [pc]\n");
            e("mov     ecx, [remaining]\n");    /* this could have been changed */
            RestoreReg("memhandler", "eax");
            RestoreReg("memhandler", "ebp");
            e("add     esi, ebp\n");            /* base+pc=pc pointer */
            e("xor     edi, edi\n");            /* keep clear for fetch */
            e("ret\n");
        }
    }
    else if (memmap_type == 1)
    {
        for (i = 0; i < 3; i++)
        {
            CacheAlign();
            switch (i)
            {
            case 0: EmitLabel("WriteByte"); break;
            case 1: EmitLabel("WriteWord"); break;
            case 2: EmitLabel("WriteLong"); break;
            }
            AddrClip("ebx");
            SaveReg("memhandler", "eax");
            SaveReg("memhandler", "ebp");
            WRITEPCTOMEM("pc");                     /* saves esi */
            e("mov     [remaining], ecx\n");
            SaveReg("memhandler", "edx");           /* save params */

            /* data, address */
            if (!call_convention)                   /* stack-based */
            {
                e("push edx\n");
                e("push ebx\n");
            }
            else                                    /* register-based */
            {
                e("mov  eax, ebx\n");
                /* don't need: e("mov  edx, edx\n"); */
            }

            switch (i)
            {
            case 0: e("call dword [write_byte]\n"); break;
            case 1: e("call dword [write_word]\n"); break;
            case 2: e("call dword [write_long]\n"); break;
            }

            if (!call_convention)
                e("add     esp, byte 8\n");

            RestoreReg("memhandler", "edx");        /* restore params */
            e("mov     esi, [pc]\n");
            e("mov     ecx, [remaining]\n");    /* this could have been changed */
            RestoreReg("memhandler", "eax");
            RestoreReg("memhandler", "ebp");
            e("add     esi, ebp\n");            /* base+pc=pc pointer */
            e("xor     edi, edi\n");            /* keep clear for fetch */
            e("ret\n");
        }
    }
}


/*****************************************************************************
* main() and Friends                                                        */

int FindV(char *option, int p, int h, int u, int argc, char **argv, int m)
{
    static int  t[128] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                         };
    int         i;
    char        *c;
          
    if (m)  /* mark as touched */
    {
        if (m < 128)
            t[m] = 1;
        return 0;
    }
    if (argc > 128)
        argc = 128;     /* maximum this function can handle is 128 */

    if (u)  /* find first untouched element */
    {
        for (i = 1; i < argc; i++)
        {
            if (!t[i])      /* 0 indicates untouched */
                return i;
        }
        return 0;
    }

    if (p)  /* find option and return integer value following it */
    {
        for (i = 1; i < argc; i++)
        {        
            if (strcmp(argv[i], option) == 0)       /* found */
            {
                if (i >= (argc - 1))        /* bounds! */
                    return 0;
                t[i + 1] = t[i] = 1;        /* touched */
                if (!h)
                    return atoi(argv[i + 1]);
                else
                    return strtoul(argv[i + 1], &c, 16);
            }
        }
        return 0;                       /* no match */
    }
    else    /* find option and return position  */    
    {
        for (i = 1; i < argc; i++)
        {        
            if (strcmp(argv[i], option) == 0)
            {
                t[i] = 1;
                return i;       /* found! return position */
            }
        }                              
        return 0;
    }
    return 0;
}

void ShowHelp()
{
    printf("Make68K Version "VERSION" by Bart Trzynadlowski: Turbo68K Source Emitter\n");
    printf("Usage:      make68k <outfile> [options]\n");
    printf("Options:    -?,-h        Show this help text\n");
    printf("            -mpu <type>  Processor to emulate [Default=68000]\n");
    printf("            -addr <bits> Address bus width [Default=24]\n");
    printf("            -illegal     Emulate illegal instruction exceptions [Default]\n");
    printf("            -noillegal   Treat illegal instructions as errors\n");
    printf("            -dummyread   Emulate 68000 dummy reads [Default]\n");
    printf("            -nodummyread Do not emulate dummy reads\n");
    printf("            -skip        Skip over idle loops\n");
    printf("            -noskip      Do not skip over idle loops [Default]\n");
    printf("            -brafetch    Update fetch pointer on Bcc, BRA, BSR, DBcc\n");
    printf("            -nobrafetch  Do not update fetch pointer for branches [Default]\n");
    printf("            -pcfetch     Special PC-relative read mode\n");
    printf("            -nopcfetch   Use data regions for PC-relative reading [Default]\n");
    printf("            -multiaddr   Address spaces for supervisor and user [Default]\n");
    printf("            -singleaddr  Single address space for supervisor and user\n");
    printf("            -defmap      Definable memory map arrays [Default]\n");
    printf("            -handler     Single handlers for memory access\n");
    printf("            -stackcall   Stack calling conventions [Default]\n");
    printf("            -regcall     Register calling conventions\n");
    printf("            -id <string> Add string to beginning of identifiers\n");
    exit(0);
}

void SetAddrMask(int num_bits)
{
    addr_bits = num_bits;
    if (!num_bits)
    {
        addr_mask = 0xffffff;   /* 24-bit is default address bus width */
        addr_bits = 24;
    }
    else
        addr_mask = (unsigned) pow(2, num_bits) - 1;
}

int main(int argc, char **argv)
{
    int     i, j, k, l, f;

#ifdef PROFILE
    for (i = 0; i < 512; i++)   prof[i] = NULL;
#endif

    if (argc <= 1 || FindV("-?", 0, 0, 0, argc, argv, 0) || FindV("-h", 0, 0, 0, argc, argv, 0))
        ShowHelp();

    mpu = FindV("-mpu", 1, 0, 0, argc, argv, 0);
    switch (mpu)    /* make sure a valid MPU type was specified */
    {
    case 0:
        mpu = 68000;
        break;
    case 68000:
    case 68010:
        break;
    default:
        fprintf(stderr, "Make68K: Unknown or unsupported processor type: %d\n", mpu);
        exit(1);
    }        
    SetAddrMask(FindV("-addr", 1, 0, 0, argc, argv, 0));
    if (FindV("-illegal", 0, 0, 0, argc, argv, 0))      illegal = 1;
    if (FindV("-noillegal", 0, 0, 0, argc, argv, 0))    illegal = 0;
    if (FindV("-dummyread", 0, 0, 0, argc, argv, 0))    dummyread = 1;
    if (FindV("-nodummyread", 0, 0, 0, argc, argv, 0))  dummyread = 0;
    if (FindV("-skip", 0, 0, 0, argc, argv, 0))         skip = 1;
    if (FindV("-noskip", 0, 0, 0, argc, argv, 0))       skip = 0;
    if (FindV("-brafetch", 0, 0, 0, argc, argv, 0))     brafetch = 1;
    if (FindV("-nobrafetch", 0, 0, 0, argc, argv, 0))   brafetch = 0;
    if (FindV("-pcfetch", 0, 0, 0, argc, argv, 0))      pcfetch = 1;
    if (FindV("-nopcfetch", 0, 0, 0, argc, argv, 0))    pcfetch = 0;
    if (FindV("-multiaddr", 0, 0, 0, argc, argv, 0))    multiaddr = 1;
    if (FindV("-singleaddr", 0, 0, 0, argc, argv, 0))   multiaddr = 0;
    if (FindV("-defmap", 0, 0, 0, argc, argv, 0))       memmap_type = 0;
    if (FindV("-handler", 0, 0, 0, argc, argv, 0))      memmap_type = 1;
    if (FindV("-stackcall", 0, 0, 0, argc, argv, 0))    call_convention = 0;
    if (FindV("-regcall", 0, 0, 0, argc, argv, 0))      call_convention = 1;
	if (FindV("-debug", 0, 0, 0, argc, argv, 0))        debug = 1;

    if ((i = FindV("-id", 0, 0, 0, argc, argv, 0)))
    {
        if ((i + 1) < argc) /* make sure we're within argv[] bounds */
        {
            if (strlen(argv[i + 1]) > 16)
            {
                fprintf(stderr, "Make68K: Identifier string is too long (maximum is 16 characters)\n");
                exit(1);
            }
            strncpy(id, argv[i + 1], 16);
            FindV(NULL, 0, 0, 0, argc, argv, i + 1);
        }
    }    
    
    if (!(f = FindV(NULL, 0, 0, 1, argc, argv, 0)))
    {
        fprintf(stderr, "Make68K: No output file specified\n");
        exit(1);
    }
    if ((fp = fopen(argv[f], "w")) == NULL)
    {
        fprintf(stderr, "Make68K: Failed to open file for writing: %s\n", argv[f]);
        exit(1);
    }

    for (i = 0; i < 65536; i++) decoded[i] = 0;
        
    printf("Make68K Version "VERSION" by Bart Trzynadlowski: Turbo68K Source Emitter\n");
    printf("\n");
    printf("Emitting file: %s\n", argv[f]);
    printf("Configuration:\n");

    e(";\n");
    e("; Turbo68K Version "VERSION": Motorola 680X0 emulator\n");
    e("; Copyright 2000-2002 Bart Trzynadlowski, see \"README.TXT\" for terms of use\n");
    e("; Assemble with NASM (http://www.web-sites.co.uk/nasm) only\n");
    e(";\n");
    e("; Configuration:\n");
    printf("- %d processor\n", mpu);
    e("; - %d processor\n", mpu);
    printf("- %d-bit addresses\n", addr_bits);
    e("; - %d-bit addresses\n", addr_bits);
    printf("- %s\n", illegal ? "Illegal instruction, Line 1010, and Line 1111 exceptions" : "Illegal instructions reported as errors");
    e("; - %s\n", illegal ? "Illegal instruction, Line 1010, and Line 1111 exceptions" : "Illegal instructions reported as errors");
    if (mpu == 68000)
    {
        printf("- Dummy reads %s\n", dummyread ? "emulated" : "not emulated");
        e("; - Dummy reads %s\n", dummyread ? "emulated" : "not emulated");
    }
    printf("- Idle loop skipping %s\n", skip ? "enabled" : "disabled");
    e("; - Idle loop skipping %s\n", skip ? "enabled" : "disabled");
    printf("- Fetch pointer%supdated for Bcc, BRA, BSR, and DBcc\n", brafetch ? " " : " not ");
    e("; - Fetch pointer%supdated for Bcc, BRA, BSR, and DBcc\n", brafetch ? " " : " not ");
    printf("- %s regions used for PC-relative reading\n", pcfetch ? "Special PC fetch" : "Data");
    e("; - %s regions used for PC-relative reading\n", pcfetch ? "Special PC fetch" : "Data");
    printf("- %s for supervisor and user\n", multiaddr ? "Separate address spaces" : "Single address space");
    e("; - %s for supervisor and user\n", multiaddr ? "Separate address spaces" : "Single address space");
    switch (memmap_type)
    {
    case 0: printf("- Definable memory map arrays\n");
            e("; - Definable memory map arrays\n");            
            break;
    case 1: printf("- Single memory handlers\n");
            e("; - Single memory handlers\n");
            break;
    case 2: printf("- Low-level memory handlers\n");
            e("; - Low-level memory handlers\n");
            break;
    }
    printf("- %s calling conventions\n", call_convention ? "Register" : "Stack");
    e("; - %s calling conventions\n", call_convention ? "Register" : "Stack");
    if (id[0] != '\0')
    {
        printf("- Identifers start with: %s\n", id);
        e("; - Identifiers start with: %s\n", id);
    }
	if (debug)
	{
		printf("- Debug function called at every instruction\n");
		e("; - Debug function called at every instruction\n");
	}
    e(";\n");
    printf("\n");
    e("bits 32\n");
    e("section .data\n");
    EmitData();
    e("section .text\n");
    EmitCode();
    printf("Generating instruction handlers:\n");
    EmitInstructions();
    EmitExceptions();
    e("section .bss\n");
    CacheAlign();
    e("jmptab       resd 65536\n");


    /*
     * Emit the compressed jump table
     *
     * Format:
     * dw 0x8000 + #    ; # of handler addresses follow, each repeated 8 times
     * ... dd # ...     ; handler addresses...
     * dw 0xc000 + #    ; # of handler addresses follow, each repeated 1 time
     * ... dd # ...     ; handler addresses...
     * dw 0x0000 + #    ; repeat following address handler # times
     * dd #             ; handler address to repeat
     * dd -1            ; terminator
     */

    e("section .data\n");
    Align(4);
    e("compressed_jmptab:\n");

    i = 0;
    j = 0;
    while (i < 65536)
    {
        if (!decoded[i])        /* invalid */
        {
            for (i = i; decoded[i] == 0 && i < 65536; i++)  j++;
            e("dw 0x0000 + %d\n", j);
            e("dd I_Invalid\n");
            j = 0;
        }
        else
        {
            if (decoded[i] == 8)    /* if more handlers*8, make a big block */
            {
                k = 0;
                l = i;  /* save opcode # */
                while (decoded[i] == 8)
                {
                    i += decoded[i];
                    k++;
                }
                e("dw 0x8000 + %d\n", k);   /* # of handlers w/ 8 reps */
                while (k != 0)              /* list handlers */
                {
                    e("dd I%04X\n", l);
                    l += decoded[l];
                    k--;
                }
            }
            if (decoded[i] == 1)    /* if more handlers*1, make a big block */
            {
                k = 0;
                l = i;  /* save opcode # */
                while (decoded[i] == 1)
                {
                    i += decoded[i];
                    k++;
                }
                e("dw 0xc000 + %d\n", k);   /* # of handlers w/ 1 reps */
                while (k != 0)              /* list handlers */
                {
                    e("dd I%04X\n", l);
                    l += decoded[l];
                    k--;
                }
            }   
            else if (decoded[i] == 0)       /* invalid instruction */
            {
                e("dw 1\n");
                e("dd I_Invalid\n");
                i++;
            }
            else                            /* misc. */
            {
                e("dw %d\n", decoded[i]);   /* # of valids */
                e("dd I%04X\n", i);         /* opcode */
                i += decoded[i];
            }
        }
    }            
    e("dd -1\n");

    /*
     * Emit profile data
     *
     * Step 1: Output the names of the instructions.
     * Step 2: Output the usage count variables and pointers to the strings.
     */

#ifdef PROFILE
    for (i = 0; prof[i] != NULL && i < 512; i++)
        e("str%s db \"%s\",0\n", prof[i], prof[i]);
    e("global _t68k_prof\n");
    e("_t68k_prof: dd begin_t68k_prof\n");
    e("begin_t68k_prof:\n");
    for (i = 0; prof[i] != NULL && i < 512; i++)
    {
        e("prof%s dd 0, str%s\n", prof[i], prof[i]);
        free(prof[i]);
    }
    e("dd -1, -1\n");
#endif

    fclose(fp);

    printf("Total:\t%d\n", num_handlers);
    printf("\nSee \"README.TXT\" for terms of use and documentation!\n");
    return 0;
}

/*
 *
 * Developers' Notes: A Guide to the Guts of Turbo68K ;)
 * -----------------------------------------------------
 *
 * Register Usage:
 *
 * EAX: ****************NZ*****C*******V
 *      AH: NZ*****C    AL: *******V
 * EBX: Address
 * ECX: Cycle counter
 * EDX: Data
 * ESI: Current fetch (PC) pointer
 * EDI: Instruction fetch/decoding (keep upper 16 bits clear!). At the
 *      beginning of any instruction handler, contains opcode in lower 16 bits
 * EBP: Pointer to base of PC region. May be below the base if the unused PC
 *      bits are non-zero
 * ---
 * MMX optimizations are unsupported -- DO NOT USE THEM! They happen to slow
 * things down
 *
 * MMX Register Usage:
 *
 * MM0:     Saved EAX (memhandler_eax)
 * MM1:     Saved EBX (memhandler_ebx)
 * MM2:     Saved EBP (memhandler_ebp)
 * MM3:     Saved EDX (run_edx)
 * MM4:     Saved ESI (run_esi)
 * MM5:     Saved EDI (run_edi)
 * MM6,MM7: Misc. usage
 * ---
 * Format of context.intr[]:
 *
 * intr[0-6] = Interrupt levels 1-7, elements contain the vector #, if 0 then
 *             no interrupt is pending at this level
 * intr[7] = Number of interrupts pending (set to 0 at reset)
 * ---
 * Format of context.status:
 *
 * 0000 0000 0000 0000 0000 0000 0000 0ISR
 *
 * I=Interrupt; 1=interrupts being processed, 0=not
 * S=Stop; 1=68K is stopped (STOP instruction), 0=not
 * R=Running; 1=68K is running, 0=not
 * ---
 * Format of decoded[]:
 *
 * [0] = 1: 2 opcode 0 handlers
 * [1] = 0: see above.. this is unused
 * [2] = next handler... (0 + 2)
 * ---
 * SR values obtained from instructions are ANDed with 0xa71f to mask out
 * unused bits. CCR values are ANDed with 0x1f to mask out the unused bits.
 * This is how a real 68K behaves.
 * ---
 * Memory mapping types: The low-level handler mode code is present, but not
 * accessible. I decided to forbid using it because of performance problems.
 * ---
 * The unused address bits of the PC are preserved. They are only lost if the
 * PC goes out of bounds.
 */
