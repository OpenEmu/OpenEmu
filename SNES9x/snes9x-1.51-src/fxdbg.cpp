/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/




#include "fxemu.h"
#include "fxinst.h"
#include <stdio.h>
#include <string.h>

extern const char *fx_apvMnemonicTable[];
extern struct FxRegs_s GSU;


/*
  When printing a line from the pipe, it could look like this:

  01:8006 f4 fb 86 iwt r4,#$86fb

  The values are:
  program bank: 01
  adress: 8006
  values at memory address 8006: f4 fb 86
  instruction in the pipe: iwt r4,#$86fb

  Note! If the instruction has more than one byte (like in 'iwt')
  and the instruction is in a delay slot, the second and third
  byte displayed will not be the same as those used.
  Since the instrction is in a delay slot, the first byte
  of the instruction will be taken from the pipe at the address
  after the branch instruction, and the next one or two bytes
  will be taken from the address that the branch points to.
  This is a bit complicated, but I've taken this into account,
  in this debug function. (See the diffrence of how the values
  vPipe1 and vPipe2 are read, compared to the values vByte1 and
  vByte2)

  */
void FxPipeString(char * pvString)
{
    char *p;
    uint32 vOpcode = (GSU.vStatusReg & 0x300) | ((uint32)PIPE);
    const char *m = fx_apvMnemonicTable[vOpcode];
    uint8 vPipe1,vPipe2,vByte1,vByte2;
    uint8 vPipeBank = GSU.vPipeAdr >> 16;

    /* The next two bytes after the pipe's address */
    vPipe1 = GSU.apvRomBank[vPipeBank][USEX16(GSU.vPipeAdr+1)];
    vPipe2 = GSU.apvRomBank[vPipeBank][USEX16(GSU.vPipeAdr+2)];

    /* The actual next two bytes to be read */
    vByte1 = PRGBANK(USEX16(R15));
    vByte2 = PRGBANK(USEX16(R15+1));

    /* Print ROM address of the pipe */
    sprintf(pvString, "%02x:%04x %02x       ",
	    USEX8(vPipeBank), USEX16(GSU.vPipeAdr), USEX8(PIPE));
    p = &pvString[strlen(pvString)];

    /* Check if it's a branch instruction */
    if( PIPE >= 0x05 && PIPE <= 0x0f )
    {
	sprintf(&pvString[11], "%02x    ", USEX8(vPipe1));
#ifdef BRANCH_DELAY_RELATIVE
	sprintf(p, m, USEX16(R15 + SEX8(vByte1) + 1 ) );
#else
	sprintf(p, m, USEX16(R15 + SEX8(vByte1) - 1 ) );
#endif
    }
    /* Check for 'move' instruction */
    else if( PIPE >= 0x10 && PIPE <= 0x1f && TF(B) )
	sprintf(p, "move r%d,r%d", USEX8(PIPE & 0x0f), (uint32)(GSU.pvSreg - GSU.avReg));
    /* Check for 'ibt', 'lms' or 'sms' */
    else if( PIPE >= 0xa0 && PIPE <= 0xaf )
    {
	sprintf(&pvString[11], "%02x    ", USEX8(vPipe1));
	if( (GSU.vStatusReg & 0x300) == 0x100 || (GSU.vStatusReg & 0x300) == 0x200 )
	    sprintf(p, m, USEX16(vByte1) << 1 );
	else
	    sprintf(p, m, USEX16(vByte1) );
    }
    /* Check for 'moves' */
    else if( PIPE >= 0xb0 && PIPE <= 0xbf && TF(B) )
	sprintf(p, "moves r%d,r%d", (uint32)(GSU.pvDreg - GSU.avReg), USEX8(PIPE & 0x0f));
    /* Check for 'iwt', 'lm' or 'sm' */
    else if( PIPE >= 0xf0 )
    {
	sprintf(&pvString[11], "%02x %02x ", USEX8(vPipe1), USEX8(vPipe2));
	sprintf(p, m, USEX8(vByte1) | (USEX16(vByte2)<<8) );
    }
    /* Normal instruction */
    else
	strcpy(p, m);
}

const char *fx_apvMnemonicTable[] =
{
    /*
     * ALT0 Table
     */
    /* 00 - 0f */
    "stop",    "nop",     "cache",    "lsr",      "rol",      "bra $%04x","blt $%04x","bge $%04x",
    "bne $%04x","beq $%04x","bpl $%04x","bmi $%04x","bcc $%04x","bcs $%04x","bvc $%04x","bvs $%04x",
    /* 10 - 1f */
    "to r0",   "to r1",   "to r2",    "to r3",    "to r4",    "to r5",    "to r6",    "to r7",
    "to r8",   "to r9",   "to r10",   "to r11",   "to r12",   "to r13",   "to r14",   "to r15",
    /* 20 - 2f */
    "with r0", "with r1", "with r2",  "with r3",  "with r4",  "with r5",  "with r6",  "with r7",
    "with r8", "with r9", "with r10", "with r11", "with r12", "with r13", "with r14", "with r15",
    /* 30 - 3f */
    "stw (r0)","stw (r1)","stw (r2)", "stw (r3)", "stw (r4)", "stw (r5)", "stw (r6)", "stw (r7)",
    "stw (r8)","stw (r9)","stw (r10)","stw (r11)","loop",     "alt1",     "alt2",     "alt3",
    /* 40 - 4f */
    "ldw (r0)","ldw (r1)","ldw (r2)", "ldw (r3)", "ldw (r4)", "ldw (r5)", "ldw (r6)", "ldw (r7)",
    "ldw (r8)","ldw (r9)","ldw (r10)","ldw (r11)","plot",     "swap",     "color",    "not",
    /* 50 - 5f */
    "add r0",  "add r1",  "add r2",   "add r3",   "add r4",   "add r5",   "add r6",   "add r7",
    "add r8",  "add r9",  "add r10",  "add r11",  "add r12",  "add r13",  "add r14",  "add r15",
    /* 60 - 6f */
    "sub r0",  "sub r1",  "sub r2",   "sub r3",   "sub r4",   "sub r5",   "sub r6",   "sub r7",
    "sub r8",  "sub r9",  "sub r10",  "sub r11",  "sub r12",  "sub r13",  "sub r14",  "sub r15",
    /* 70 - 7f */
    "merge",   "and r1",  "and r2",   "and r3",   "and r4",   "and r5",   "and r6",   "and r7",
    "and r8",  "and r9",  "and r10",  "and r11",  "and r12",  "and r13",  "and r14",  "and r15",
    /* 80 - 8f */
    "mult r0", "mult r1", "mult r2",  "mult r3",  "mult r4",  "mult r5",  "mult r6",  "mult r7",
    "mult r8", "mult r9", "mult r10", "mult r11", "mult r12", "mult r13", "mult r14", "mult r15",
    /* 90 - 9f */
    "sbk",     "link #1", "link #2",  "link #3",  "link #4",  "sex",      "asr",      "ror",
    "jmp (r8)","jmp (r9)","jmp (r10)","jmp (r11)","jmp (r12)","jmp (r13)","lob",      "fmult",
    /* a0 - af */
    "ibt r0,#$%02x",  "ibt r1,#$%02x",  "ibt r2,#$%02x",   "ibt r3,#$%02x",
    "ibt r4,#$%02x",  "ibt r5,#$%02x",  "ibt r6,#$%02x",   "ibt r7,#$%02x",
    "ibt r8,#$%02x",  "ibt r9,#$%02x",  "ibt r10,#$%02x",  "ibt r11,#$%02x",
    "ibt r12,#$%02x", "ibt r13,#$%02x", "ibt r14,#$%02x",  "ibt r15,#$%02x",
    /* b0 - bf */
    "from r0", "from r1", "from r2",  "from r3",  "from r4",  "from r5",  "from r6",  "from r7",
    "from r8", "from r9", "from r10", "from r11", "from r12", "from r13", "from r14", "from r15",
    /* c0 - cf */
    "hib",     "or r1",   "or r2",    "or r3",    "or r4",    "or r5",    "or r6",    "or r7",
    "or r8",   "or r9",   "or r10",   "or r11",   "or r12",   "or r13",   "or r14",   "or r15",
    /* d0 - df */
    "inc r0",  "inc r1",  "inc r2",   "inc r3",   "inc r4",   "inc r5",   "inc r6",   "inc r7",
    "inc r8",  "inc r9",  "inc r10",  "inc r11",  "inc r12",  "inc r13",  "inc r14",  "getc",
    /* e0 - ef */
    "dec r0",  "dec r1",  "dec r2",   "dec r3",   "dec r4",   "dec r5",   "dec r6",   "dec r7",
    "dec r8",  "dec r9",  "dec r10",  "dec r11",  "dec r12",  "dec r13",  "dec r14",  "getb",
    /* f0 - ff */
    "iwt r0,#$%04x",  "iwt r1,#$%04x",  "iwt r2,#$%04x",   "iwt r3,#$%04x",
    "iwt r4,#$%04x",  "iwt r5,#$%04x",  "iwt r6,#$%04x",   "iwt r7,#$%04x",
    "iwt r8,#$%04x",  "iwt r9,#$%04x",  "iwt r10,#$%04x",  "iwt r11,#$%04x",
    "iwt r12,#$%04x", "iwt r13,#$%04x", "iwt r14,#$%04x",  "iwt r15,#$%04x",

    /*
     * ALT1 Table
     */

    /* 00 - 0f */
    "stop",    "nop",     "cache",    "lsr",      "rol",      "bra $%04x","blt $%04x","bge $%04x",
    "bne $%04x","beq $%04x","bpl $%04x","bmi $%04x","bcc $%04x","bcs $%04x","bvc $%04x","bvs $%04x",
    /* 10 - 1f */
    "to r0",   "to r1",   "to r2",    "to r3",    "to r4",    "to r5",    "to r6",    "to r7",
    "to r8",   "to r9",   "to r10",   "to r11",   "to r12",   "to r13",   "to r14",   "to r15",
    /* 20 - 2f */
    "with r0", "with r1", "with r2",  "with r3",  "with r4",  "with r5",  "with r6",  "with r7",
    "with r8", "with r9", "with r10", "with r11", "with r12", "with r13", "with r14", "with r15",
    /* 30 - 3f */
    "stb (r0)","stb (r1)","stb (r2)", "stb (r3)", "stb (r4)", "stb (r5)", "stb (r6)", "stb (r7)",
    "stb (r8)","stb (r9)","stb (r10)","stb (r11)","loop",     "alt1",     "alt2",     "alt3",
    /* 40 - 4f */
    "ldb (r0)","ldb (r1)","ldb (r2)", "ldb (r3)", "ldb (r4)", "ldb (r5)", "ldb (r6)", "ldb (r7)",
    "ldb (r8)","ldb (r9)","ldb (r10)","ldb (r11)","rpix",     "swap",     "cmode",    "not",
    /* 50 - 5f */
    "adc r0",  "adc r1",  "adc r2",   "adc r3",   "adc r4",   "adc r5",   "adc r6",   "adc r7",
    "adc r8",  "adc r9",  "adc r10",  "adc r11",  "adc r12",  "adc r13",  "adc r14",  "adc r15",
    /* 60 - 6f */
    "sbc r0",  "sbc r1",  "sbc r2",   "sbc r3",   "sbc r4",   "sbc r5",   "sbc r6",   "sbc r7",
    "sbc r8",  "sbc r9",  "sbc r10",  "sbc r11",  "sbc r12",  "sbc r13",  "sbc r14",  "sbc r15",
    /* 70 - 7f */
    "merge",   "bic r1",  "bic r2",   "bic r3",   "bic r4",   "bic r5",   "bic r6",   "bic r7",
    "bic r8",  "bic r9",  "bic r10",  "bic r11",  "bic r12",  "bic r13",  "bic r14",  "bic r15",
    /* 80 - 8f */
    "umult r0","umult r1","umult r2", "umult r3", "umult r4", "umult r5", "umult r6", "umult r7",
    "umult r8","umult r9","umult r10","umult r11","umult r12","umult r13","umult r14","umult r15",
    /* 90 - 9f */
    "sbk",     "link #1", "link #2",  "link #3",  "link #4",  "sex",      "div2",     "ror",
    "ljmp (r8)","ljmp (r9)","ljmp (r10)","ljmp (r11)", "ljmp (r12)", "ljmp (r13)", "lob", "lmult",
    /* a0 - af */
    "lms r0,($%04x)",  "lms r1,($%04x)",  "lms r2,($%04x)",   "lms r3,($%04x)",
    "lms r4,($%04x)",  "lms r5,($%04x)",  "lms r6,($%04x)",   "lms r7,($%04x)",
    "lms r8,($%04x)",  "lms r9,($%04x)",  "lms r10,($%04x)",  "lms r11,($%04x)",
    "lms r12,($%04x)", "lms r13,($%04x)", "lms r14,($%04x)",  "lms r15,($%04x)",
    /* b0 - bf */
    "from r0", "from r1", "from r2",  "from r3",  "from r4",  "from r5",  "from r6",  "from r7",
    "from r8", "from r9", "from r10", "from r11", "from r12", "from r13", "from r14", "from r15",
    /* c0 - cf */
    "hib",     "xor r1",  "xor r2",   "xor r3",   "xor r4",   "xor r5",   "xor r6",   "xor r7",
    "xor r8",  "xor r9",  "xor r10",  "xor r11",  "xor r12",  "xor r13",  "xor r14",  "xor r15",
    /* d0 - df */
    "inc r0",  "inc r1",  "inc r2",   "inc r3",   "inc r4",   "inc r5",   "inc r6",   "inc r7",
    "inc r8",  "inc r9",  "inc r10",  "inc r11",  "inc r12",  "inc r13",  "inc r14",  "getc",
    /* e0 - ef */
    "dec r0",  "dec r1",  "dec r2",   "dec r3",   "dec r4",   "dec r5",   "dec r6",   "dec r7",
    "dec r8",  "dec r9",  "dec r10",  "dec r11",  "dec r12",  "dec r13",  "dec r14",  "getbh",
    /* f0 - ff */
    "lm r0,($%04x)",   "lm r1,($%04x)",   "lm r2,($%04x)",    "lm r3,($%04x)",
    "lm r4,($%04x)",   "lm r5,($%04x)",   "lm r6,($%04x)",    "lm r7,($%04x)",
    "lm r8,($%04x)",   "lm r9,($%04x)",   "lm r10,($%04x)",   "lm r11,($%04x)",
    "lm r12,($%04x)",  "lm r13,($%04x)",  "lm r14,($%04x)",   "lm r15,($%04x)",

    /*
     * ALT2 Table
     */

    /* 00 - 0f */
    "stop",    "nop",     "cache",    "lsr",      "rol",      "bra $%04x","blt $%04x","bge $%04x",
    "bne $%04x","beq $%04x","bpl $%04x","bmi $%04x","bcc $%04x","bcs $%04x","bvc $%04x","bvs $%04x",
    /* 10 - 1f */
    "to r0",   "to r1",   "to r2",    "to r3",    "to r4",    "to r5",    "to r6",    "to r7",
    "to r8",   "to r9",   "to r10",   "to r11",   "to r12",   "to r13",   "to r14",   "to r15",
    /* 20 - 2f */
    "with r0", "with r1", "with r2",  "with r3",  "with r4",  "with r5",  "with r6",  "with r7",
    "with r8", "with r9", "with r10", "with r11", "with r12", "with r13", "with r14", "with r15",
    /* 30 - 3f */
    "stw (r0)","stw (r1)","stw (r2)", "stw (r3)", "stw (r4)", "stw (r5)", "stw (r6)", "stw (r7)",
    "stw (r8)","stw (r9)","stw (r10)","stw (r11)","loop",     "alt1",     "alt2",     "alt3",
    /* 40 - 4f */
    "ldw (r0)","ldw (r1)","ldw (r2)", "ldw (r3)", "ldw (r4)", "ldw (r5)", "ldw (r6)", "ldw (r7)",
    "ldw (r8)","ldw (r9)","ldw (r10)","ldw (r11)","plot",     "swap",     "color",    "not",
    /* 50 - 5f */
    "add #0",  "add #1",  "add #2",   "add #3",   "add #4",   "add #5",   "add #6",   "add #7",
    "add #8",  "add #9",  "add #10",  "add #11",  "add #12",  "add #13",  "add #14",  "add #15",
    /* 60 - 6f */
    "sub #0",  "sub #1",  "sub #2",   "sub #3",   "sub #4",   "sub #5",   "sub #6",   "sub #7",
    "sub #8",  "sub #9",  "sub #10",  "sub #11",  "sub #12",  "sub #13",  "sub #14",  "sub #15",
    /* 70 - 7f */
    "merge",   "and #1",  "and #2",   "and #3",   "and #4",   "and #5",   "and #6",   "and #7",
    "and #8",  "and #9",  "and #10",  "and #11",  "and #12",  "and #13",  "and #14",  "and #15",
    /* 80 - 8f */
    "mult #0", "mult #1", "mult #2",  "mult #3",  "mult #4",  "mult #5",  "mult #6",  "mult #7",
    "mult #8", "mult #9", "mult #10", "mult #11", "mult #12", "mult #13", "mult #14", "mult #15",
    /* 90 - 9f */
    "sbk",     "link #1", "link #2",  "link #3",  "link #4",  "sex",      "asr",      "ror",
    "jmp (r8)","jmp (r9)","jmp (r10)","jmp (r11)","jmp (r12)","jmp (r13)","lob",      "fmult",
    /* a0 - af */
    "sms ($%04x),r0",  "sms ($%04x),r1",  "sms ($%04x),r2",   "sms ($%04x),r3",
    "sms ($%04x),r4",  "sms ($%04x),r5",  "sms ($%04x),r6",   "sms ($%04x),r7",
    "sms ($%04x),r8",  "sms ($%04x),r9",  "sms ($%04x),r10",  "sms ($%04x),r11",
    "sms ($%04x),r12", "sms ($%04x),r13", "sms ($%04x),r14",  "sms ($%04x),r15",
    /* b0 - bf */
    "from r0", "from r1", "from r2",  "from r3",  "from r4",  "from r5",  "from r6",  "from r7",
    "from r8", "from r9", "from r10", "from r11", "from r12", "from r13", "from r14", "from r15",
    /* c0 - cf */
    "hib",     "or #1",   "or #2",    "or #3",    "or #4",    "or #5",    "or #6",    "or #7",
    "or #8",   "or #9",   "or #10",   "or #11",   "or #12",   "or #13",   "or #14",   "or #15",
    /* d0 - df */
    "inc r0",  "inc r1",  "inc r2",   "inc r3",   "inc r4",   "inc r5",   "inc r6",   "inc r7",
    "inc r8",  "inc r9",  "inc r10",  "inc r11",  "inc r12",  "inc r13",  "inc r14",  "ramb",
    /* e0 - ef */
    "dec r0",  "dec r1",  "dec r2",   "dec r3",   "dec r4",   "dec r5",   "dec r6",   "dec r7",
    "dec r8",  "dec r9",  "dec r10",  "dec r11",  "dec r12",  "dec r13",  "dec r14",  "getbl",
    /* f0 - ff */
    "sm ($%04x),r0",   "sm ($%04x),r1",   "sm ($%04x),r2",    "sm ($%04x),r3",
    "sm ($%04x),r4",   "sm ($%04x),r5",   "sm ($%04x),r6",    "sm ($%04x),r7",
    "sm ($%04x),r8",   "sm ($%04x),r9",   "sm ($%04x),r10",   "sm ($%04x),r11",
    "sm ($%04x),r12",  "sm ($%04x),r13",  "sm ($%04x),r14",   "sm ($%04x),r15",

    /*
     * ALT3 Table
     */

    /* 00 - 0f */
    "stop",    "nop",     "cache",    "lsr",      "rol",      "bra $%04x","blt $%04x","bge $%04x",
    "bne $%04x","beq $%04x","bpl $%04x","bmi $%04x","bcc $%04x","bcs $%04x","bvc $%04x","bvs $%04x",
    /* 10 - 1f */
    "to r0",   "to r1",   "to r2",    "to r3",    "to r4",    "to r5",    "to r6",    "to r7",
    "to r8",   "to r9",   "to r10",   "to r11",   "to r12",   "to r13",   "to r14",   "to r15",
    /* 20 - 2f */
    "with r0", "with r1", "with r2",  "with r3",  "with r4",  "with r5",  "with r6",  "with r7",
    "with r8", "with r9", "with r10", "with r11", "with r12", "with r13", "with r14", "with r15",
    /* 30 - 3f */
    "stb (r0)","stb (r1)","stb (r2)", "stb (r3)", "stb (r4)", "stb (r5)", "stb (r6)", "stb (r7)",
    "stb (r8)","stb (r9)","stb (r10)","stb (r11)","loop",     "alt1",     "alt2",     "alt3",
    /* 40 - 4f */
    "ldb (r0)","ldb (r1)","ldb (r2)", "ldb (r3)", "ldb (r4)", "ldb (r5)", "ldb (r6)", "ldb (r7)",
    "ldb (r8)","ldb (r9)","ldb (r10)","ldb (r11)","rpix",     "swap",     "cmode",    "not",
    /* 50 - 5f */
    "adc #0",  "adc #1",  "adc #2",   "adc #3",   "adc #4",   "adc #5",   "adc #6",   "adc #7",
    "adc #8",  "adc #9",  "adc #10",  "adc #11",  "adc #12",  "adc #13",  "adc #14",  "adc #15",
    /* 60 - 6f */
    "cmp r0",  "cmp r1",  "cmp r2",   "cmp r3",   "cmp r4",   "cmp r5",   "cmp r6",   "cmp r7",
    "cmp r8",  "cmp r9",  "cmp r10",  "cmp r11",  "cmp r12",  "cmp r13",  "cmp r14",  "cmp r15",
    /* 70 - 7f */
    "merge",   "bic #1",  "bic #2",   "bic #3",   "bic #4",   "bic #5",   "bic #6",   "bic #7",
    "bic #8",  "bic #9",  "bic #10",  "bic #11",  "bic #12",  "bic #13",  "bic #14",  "bic #15",
    /* 80 - 8f */
    "umult #0","umult #1","umult #2", "umult #3", "umult #4", "umult #5", "umult #6", "umult #7",
    "umult #8","umult #9","umult #10","umult #11","umult #12","umult #13","umult #14","umult #15",
    /* 90 - 9f */
    "sbk",     "link #1", "link #2",  "link #3",  "link #4",  "sex",      "div2",     "ror",
    "ljmp (r8)","ljmp (r9)","ljmp (r10)","ljmp (r11)", "ljmp (r12)", "ljmp (r13)", "lob", "lmult",
    /* a0 - af */
    "lms r0,($%04x)",  "lms r1,($%04x)",  "lms r2,($%04x)",   "lms r3,($%04x)",
    "lms r4,($%04x)",  "lms r5,($%04x)",  "lms r6,($%04x)",   "lms r7,($%04x)",
    "lms r8,($%04x)",  "lms r9,($%04x)",  "lms r10,($%04x)",  "lms r11,($%04x)",
    "lms r12,($%04x)", "lms r13,($%04x)", "lms r14,($%04x)",  "lms r15,($%04x)",
    /* b0 - bf */
    "from r0", "from r1", "from r2",  "from r3",  "from r4",  "from r5",  "from r6",  "from r7",
    "from r8", "from r9", "from r10", "from r11", "from r12", "from r13", "from r14", "from r15",
    /* c0 - cf */
    "hib",     "xor #1",  "xor #2",   "xor #3",   "xor #4",   "xor #5",   "xor #6",   "xor #7",
    "xor #8",  "xor #9",  "xor #10",  "xor #11",  "xor #12",  "xor #13",  "xor #14",  "xor #15",
    /* d0 - df */
    "inc r0",  "inc r1",  "inc r2",   "inc r3",   "inc r4",   "inc r5",   "inc r6",   "inc r7",
    "inc r8",  "inc r9",  "inc r10",  "inc r11",  "inc r12",  "inc r13",  "inc r14",  "romb",
    /* e0 - ef */
    "dec r0",  "dec r1",  "dec r2",   "dec r3",   "dec r4",   "dec r5",   "dec r6",   "dec r7",
    "dec r8",  "dec r9",  "dec r10",  "dec r11",  "dec r12",  "dec r13",  "dec r14",  "getbs",
    /* f0 - ff */
    "lm r0,($%04x)",   "lm r1,($%04x)",   "lm r2,($%04x)",    "lm r3,($%04x)",
    "lm r4,($%04x)",   "lm r5,($%04x)",   "lm r6,($%04x)",    "lm r7,($%04x)",
    "lm r8,($%04x)",   "lm r9,($%04x)",   "lm r10,($%04x)",   "lm r11,($%04x)",
    "lm r12,($%04x)",  "lm r13,($%04x)",  "lm r14,($%04x)",   "lm r15,($%04x)",
};

