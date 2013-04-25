|  cpu_m68k.S - 6502 CPU emulation for Motorola 68000 based hosts
|               such are Atari ST/TT/Falcon and clones, or Amiga, or old Mac.
|
|  Copyright (C) 2001 Karel Rous (empty head)
|  Copyright (C) 2001-2003 Atari800 development team (see DOC/CREDITS)
|
|  This file is part of the Atari800 emulator project which emulates
|  the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
|
|  Atari800 is free software; you can redistribute it and/or modify
|  it under the terms of the GNU General Public License as published by
|  the Free Software Foundation; either version 2 of the License, or
|  (at your option) any later version.
|
|  Atari800 is distributed in the hope that it will be useful,
|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|  GNU General Public License for more details.
|
|  You should have received a copy of the GNU General Public License
|  along with Atari800; if not, write to the Free Software
|  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
|
| Last changes    :   30th March 2003, gerhard.janka
| Converted from Devpac to GCC syntax by Petr Stehlik, no functional changes

| P65C02 ; we emulate this version of processor (6502 has a bug in jump code,
         | you can emulate this bug by commenting out this line :)
| PROFILE  ; fills the 'instruction_count' array for instruction profiling
| MONITOR_BREAK   ; jump to monitor at break
| CRASH_MENU      ; enable crash menu output
| CYCLE_EXACT     ; !NO_CYCLE_EXACT :)
| NEW_CYCLE_EXACT ; !NO_NEW_CYCLE_EXACT :)

  .globl _CART_BountyBob2
  .globl _CART_BountyBob1
  .globl _GTIA_GetByte
  .globl _POKEY_GetByte
  .globl _PIA_GetByte
  .globl _ANTIC_GetByte
  .globl _CART_GetByte
  .globl _GTIA_PutByte
  .globl _POKEY_PutByte
  .globl _PIA_PutByte
  .globl _ANTIC_PutByte
  .globl _CART_PutByte
  .globl _Atari800_RunEsc
  .globl _Atari800_Exit
  .globl _exit
  .globl _wsync_halt |CPU is stopped
 #ifdef NEW_CYCLE_EXACT
  .globl _delayed_wsync
  .globl _antic2cpu_ptr
  .globl _cur_screen_pos
  #endif
  .globl _xpos
  .globl _xpos_limit
  .globl _regPC
  .globl _regA
  .globl _regP |/* Processor Status Byte (Partial) */
  .globl _regS
  .globl _regX
  .globl _regY
  .globl _memory
  .globl _attrib
 #ifdef PROFILE
  .globl _instruction_count
  #endif
 #ifdef MONITOR_BREAK
  .globl _remember_PC
  .globl _remember_PC_curpos
 #ifdef NEW_CYCLE_EXACT
  .globl _remember_xpos
  .globl _remember_xpos_curpos
  #endif
  .globl _remember_JMP
  .globl _remember_jmp_curpos
  .globl _ypos_break_addr
  .globl _ypos
  .globl _break_addr
  .globl _break_step
  .globl _break_ret
  .globl _break_cim
  .globl _break_here
  .globl _brkhere
  .globl _ret_nesting
  #endif
 #ifdef CRASH_MENU
  .globl _crash_code
  .globl _crash_address
  .globl _crash_afterCIM
  #endif
  .globl _IRQ
  .globl _NMI
  .globl _RTI
  .globl _GO
  .globl _CPUGET
  .globl _CPUPUT
  .globl _CPU_INIT
  .globl _cycles |temporarily needed outside :)
  .globl _cim_encountered
  .globl _rts_handler

 #ifdef MONITOR_BREAK

#define rem_pc_steps 64  // has to be equal to REMEMBER_PC_STEPS
#define rem_jmp_steps 16  // has to be equal to REMEMBER_JMP_STEPS

remember_PC:
_remember_PC:
  ds.w rem_pc_steps   |REMEMBER_PC_STEPS
remember_PC_curpos:
_remember_PC_curpos:
  ds.l 1
remember_xpos:
_remember_xpos:
  ds.l rem_pc_steps   |REMEMBER_PC_STEPS
remember_xpos_curpos:
_remember_xpos_curpos:
  ds.l 1

remember_JMP:
_remember_JMP:
  ds.w rem_jmp_steps  |REMEMBER_JMP_STEPS
remember_jmp_curpos:
_remember_jmp_curpos:
  ds.l 1

  #endif

  .even

  .align	4         | doubleword alignment

regP:
  ds.b 1        ;
_regP: ds.b 1   | CCR

regA:
  ds.b 1
_regA: ds.b 1   | A

regX:
  ds.b 1
_regX: ds.b 1   | X

regY:
  ds.b 1
_regY: ds.b 1   | Y

regPC:
_regPC: ds.w 1  | PC

regS:
  ds.b 1
_regS: ds.b 1   | stack

IRQ:
_IRQ: ds.b 1
      ds.b 1    | dummy

_cim_encountered:
	ds.b	1

_rts_handler:
	ds.l	1

  .even

#define memory_pointer a5
#define attrib_pointer a4
#define PC6502 a2

#define CD a6    /* cycles counter up */
#define ZFLAG d1 /* Bit 0..7 */
#define NFLAG d1 /* Bit 8..15 */
#define VFLAG d6 /* Bit 7 */
#define DFLAG d6 /* Bit 15 */
#define CFLAG d5 /* Bit 0..7, ( 1 = ff ) */
#define A d2
#define X d3
#define Y d4

|d0  contains usually address where we are working or temporary value
|d7  contains is a working register or address

|these are bit in MC68000 CCR register
#define NB68 3
#define EB68 4 // X
#define ZB68 2
#define OB68 1
#define CB68 0

#define WSYNC_C 106

#define N_FLAG 0x80
#define N_FLAGN 0x7f
#define N_FLAGB 7
#define V_FLAG 0x40
#define V_FLAGN 0xbf
#define V_FLAGB 6
#define G_FLAG 0x20
#define G_FLAGB 5
#define B_FLAG 0x10
#define B_FLAGN 0xef
#define B_FLAGB 4
#define D_FLAG 0x08
#define D_FLAGN 0xf7
#define D_FLAGB 3
#define I_FLAG 0x04
#define I_FLAGN 0xfb
#define I_FLAGB 2
#define Z_FLAG 0x02
#define Z_FLAGN 0xfd
#define Z_FLAGB 1
#define C_FLAG 0x01
#define C_FLAGN 0xfe
#define C_FLAGB 0
#define VCZN_FLAGS 0xc3
#define VCZN_FLAGSN 0x3c

/* cycles per instruction */
#define cy_CIM  2
#define cy_NOP  2
#define cy_NOP2  2
#define cy_NOP3  3
#define cy_NOP4  4
#define cy_SKW  4
#define cy_BRK  7
#define cy_Sub  6
#define cy_Bcc  2
#define cy_Bcc1  3
#define cy_Bcc2  4
#define cy_JmpAbs  3
#define cy_JmpInd  5
#define cy_IndX  6    // indirect X
#define cy_IndY  5    // indirect Y
#define cy_IndY2  6   // indirect Y (+)
#define cy_IndX_RW  8 // indirect X read/write ( all inofficial )
#define cy_IndY_RW  8 // indirect Y read/write ( all inofficial )
#define cy_Abs  4     // absolute
#define cy_Abs_RW  6  // absolute read/write
#define cy_AbsX  4    // absolute X
#define cy_AbsX2  5   // absolute X (+)
#define cy_AbsX_RW  7 // absolute X read/write
#define cy_AbsY  4    // absolute Y
#define cy_AbsY2  5   // absolute X (+)
#define cy_AbsY_RW  7 // absolute Y read/write ( all inofficial )
#define cy_ZP  3      // zero page
#define cy_ZP_RW  5   // zero page read/write
#define cy_ZPX  4     // zero page X
#define cy_ZPX_RW  6  // zero page X read/write
#define cy_ZPY  4     // zero page X
#define cy_Imm  2     // immediate
#define cy_FlagCS  2  // flag clear/set
#define cy_RegChg  2  // register only manipulation
#define cy_RegPH  3   // push register to stack
#define cy_RegPL  4   // pull register from stack

  .macro LoHi p1    |change order of lo and hi byte (address)
      ror.w #8,\p1
      .endm

|  ==========================================================
|  Emulated Registers and Flags are kept local to this module
|  ==========================================================

| regP=processor flags; regPC=PC; regA=A; regX=X; regY=Y
  .macro UPDATE_GLOBAL_REGS
  sub.l   memory_pointer,PC6502
  movem.w d0/d2-d4/a2,regP // d0->regP, d2-d4 (A,X,Y) a2 (regPC)
  .endm

| PC=regPC; A=regA; X=regX; Y=regY
  .macro UPDATE_LOCAL_REGS
  moveq   #0,d7
  move.w  regP,d0
  move.w  regA,d2
  move.w  regX,d3
  move.w  regY,d4
  move.w  regPC,d7
  move.l  memory_pointer,PC6502
  add.l   d7,PC6502
  lea     OPMODE_TABLE,a3
  btst    #D_FLAGB,_regP
  beq.s   PDATE_LOCAL_REGS_upd_end\@
  lea     OPMODE_TABLE_D,a3
PDATE_LOCAL_REGS_upd_end\@:
  .endm

_Local_GetByte:
  move.l d7,d1
  moveq  #0,d0
  move.b d1,d0
  lsr.w  #8,d1
  move.b (HIxTable,d1.l),d1
| jmp    ([GetTable,PC,d1.l*4])
  move.w (GetTable,PC,d1.l*2),d1
  jmp    (GetTable,d1.w)

GetTable:
  .word GetNone-GetTable,GetGTIA-GetTable
  .word GetPOKEY-GetTable,GetPIA-GetTable
  .word GetANTIC-GetTable,GetCART-GetTable
  .word ItsBob1-GetTable,ItsBob2-GetTable

GetNone:
  st     d0        | higher bytes are 0 from before
  rts
GetGTIA:
  move.l d0,-(a7)
  jsr    _GTIA_GetByte
  addq.l #4,a7
  rts
GetPOKEY:
  move.l d0,-(a7)
  jsr    _POKEY_GetByte
  addq.l #4,a7
  rts
GetPIA:
  move.l d0,-(a7)
  jsr    _PIA_GetByte
  addq.l #4,a7
  rts
GetANTIC:
  move.l d0,-(a7)
  jsr    _ANTIC_GetByte
  addq.l #4,a7
  rts
GetCART:
  move.l d0,-(a7)
  jsr    _CART_GetByte
  addq.l #4,a7
  rts
ItsBob2:
  move.w d7,-(a7)
  clr.w  -(a7)
  jsr    _CART_BountyBob2
  addq.l #4,a7
  moveq  #0,d0
  rts
ItsBob1:
  move.w d7,-(a7)
  clr.w  -(a7)
  jsr    _CART_BountyBob1
  addq.l #4,a7
  moveq  #0,d0
  rts

_Local_PutByte:
  moveq  #0,d1
  move.w d7,d1
  lsr.w  #8,d1
  move.b (HIxTable,d1.l),d1
  jmp    ([PutTable,PC,d1.l*4])

PutTable:
  .long PutNone,PutGTIA,PutPOKEY,PutPIA
  .long PutANTIC,PutCART,ItsBob1,ItsBob2

PutNone:
  moveq  #0,d0
  rts
PutGTIA:
  move.b d0,d1
  move.l d1,-(a7)
  move.b d7,d1
  move.l d1,-(a7)
 #ifdef CYCLE_EXACT
  move.l CD,_xpos
  #endif
  jsr    _GTIA_PutByte
  addq.l #8,a7
  rts
PutPOKEY:
  move.b d0,d1
  move.l d1,-(a7)
  move.b d7,d1
  move.l d1,-(a7)
 #ifdef CYCLE_EXACT
  move.l CD,_xpos
  #endif
  jsr    _POKEY_PutByte
  addq.l #8,a7
  rts
PutPIA:
  move.b d0,d1
  move.l d1,-(a7)
  move.b d7,d1
  move.l d1,-(a7)
  jsr    _PIA_PutByte
  addq.l #8,a7
  rts
PutANTIC:
  move.b d0,d1
  move.l d1,-(a7)
  move.b d7,d1
  move.l d1,-(a7)
  move.l CD,_xpos
  jsr    _ANTIC_PutByte
  move.l _xpos,CD
  addq.l #8,a7
  rts
PutCART:
  move.b d0,d1
  move.l d1,-(a7)
  move.b d7,d1
  move.l d1,-(a7)
  jsr    _CART_PutByte
  addq.l #8,a7
  rts

#define HIxNone 0
#define HIxGTIA8 1
#define HIxGTIA5 1
#define HIxPOKEY8 2
#define HIxPOKEY5 2
#define HIxPIA8 3
#define HIxANTIC8 4
#define HIxCART 5
#define HIxBob1 6
#define HIxBob2 7

HIxTable:
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 00..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 04..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 08..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 0c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 10..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 14..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 18..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 1c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 20..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 24..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 28..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 2c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 30..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 34..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 38..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 3c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 40..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 44..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 48..b
  .byte HIxNone,HIxNone,HIxNone,HIxBob1     | 4c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 50..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 54..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 58..b
  .byte HIxNone,HIxNone,HIxNone,HIxBob2     | 5c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 60..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 64..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 68..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 6c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 70..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 74..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 78..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 7c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 80..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 84..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 88..b
  .byte HIxNone,HIxNone,HIxNone,HIxBob1     | 8c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 90..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 94..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | 98..b
  .byte HIxNone,HIxNone,HIxNone,HIxBob2     | 9c..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | a0..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | a4..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | a8..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | ac..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | b0..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | b4..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | b8..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | bc..f
  .byte HIxGTIA5,HIxNone,HIxNone,HIxNone    | c0..3
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | c4..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | c8..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | cc..f
  .byte HIxGTIA8,HIxNone,HIxPOKEY8,HIxPIA8  | d0..3
  .byte HIxANTIC8,HIxCART,HIxNone,HIxNone   | d4..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | d8..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | dc..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | e0..3
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | e4..7
  .byte HIxPOKEY5,HIxNone,HIxNone,HIxPOKEY5 | e8..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | ec..f
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | f0..3
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | f4..7
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | f8..b
  .byte HIxNone,HIxNone,HIxNone,HIxNone     | fc..f

  .macro EXE_GETBYTE
| move.l d7,-(a7)
  jsr    _Local_GetByte
| addq.l #4,a7 ;put stack onto right place
  .endm

  .macro EXE_PUTBYTE p1
| clr.l  -(a7)
| move.b \p1,3(a7) ;byte
  jsr    _Local_PutByte
| addq.l #8,a7
  .endm

  .macro RMW_GETBYTE
 #ifdef CYCLE_EXACT
  move.w d7,d0
  and.w  #0xef1f,d0
  cmp.w  #0xc01a,d0
  bne.s  MW_GETBYTE_normal_get
  EXE_GETBYTE
  subq.l #1,CD
  move.l d0,-(a7)
  EXE_PUTBYTE d0
  move.l (a7)+,d0
  addq.l #1,CD
  bra.s  MW_GETBYTE_end_rmw_get
MW_GETBYTE_normal_get:
  EXE_GETBYTE
MW_GETBYTE_end_rmw_get:
 #else
  EXE_GETBYTE
  #endif
  .endm

_CPU_INIT:
 #ifdef MONITOR_BREAK
  moveq  #0,d0
  move.l d0,_remember_PC_curpos
  move.l d0,_remember_xpos_curpos
  move.l d0,_remember_jmp_curpos
  #endif
  moveq  #1,d0     | set regS to page 1
  move.b d0,regS
  rts

  .macro SetI
  ori.b  #I_FLAG,_regP
  .endm

  .macro ClrI
  andi.b #I_FLAGN,_regP
  .endm

  .macro SetB
  ori.b  #B_FLAG,_regP
  .endm

  .macro SetD
  ori.b  #D_FLAG,_regP
  lea    OPMODE_TABLE_D,a3
  .endm

  .macro ClrD
  andi.b #D_FLAGN,_regP
  lea    OPMODE_TABLE,a3
  .endm

|static UBYTE  N;  /* bit7 zero (0) or bit 7 non-zero (1) */
|static UBYTE  Z;  /* zero (0) or non-zero (1) */
|static UBYTE  V;
|static UBYTE  C;  /* zero (0) or one(1) */

#define isRAM 0
#define isROM 1
#define isHARDWARE 2

|/*
| * The following array is used for 6502 instruction profiling
| */

|int instruction_count[256];

|UBYTE memory[65536];
|UBYTE attrib[65536];

|/*
|  ===============================================================
|  Z flag: This actually contains the result of an operation which
|    would modify the Z flag. The value is tested for
|    equality by the BEQ and BNE instruction.
|  ===============================================================
|*/

| Bit    : 76543210
| 68000  : ***XNZVC
| _RegP  : NV*BDIZC

  .macro ConvertSTATUS_RegP p1
  move.b _regP,\p1 |put flag BDI into d0
  andi.b #VCZN_FLAGSN,\p1 | clear overflow, carry, zero & negative flag
  tst.b  CFLAG
  beq.s  onvertSTATUS_RegP_SETC\@
  addq.b #1,\p1
onvertSTATUS_RegP_SETC\@:
  tst.w  NFLAG
  bpl.s  onvertSTATUS_RegP_SETN\@
| ori.b  #N_FLAG,\p1
  tas    \p1
onvertSTATUS_RegP_SETN\@:
  tst.b  ZFLAG
  bne.s  onvertSTATUS_RegP_SETZ\@   | beware! reverse compare is ok
  addq.b #2,\p1
onvertSTATUS_RegP_SETZ\@:
  tst.b  VFLAG
  bpl.s  onvertSTATUS_RegP_SETV\@        | !!!
  ori.b  #V_FLAG,\p1
onvertSTATUS_RegP_SETV\@:
  .endm

  .macro ConvertSTATUS_RegP_destroy p1
  move.b _regP,\p1 |put flag BDI into d0
  andi.b  #VCZN_FLAGSN,\p1 | clear overflow, carry, zero & negative flag
  lsr.b  #7,CFLAG
  or.b   CFLAG,\p1
  tst.w  NFLAG
  bpl.s  onvertSTATUS_RegP_destroy_SETN\@
| ori.b  #N_FLAG,\p1
  tas    \p1
onvertSTATUS_RegP_destroy_SETN\@:
  tst.b  ZFLAG
  bne.s  onvertSTATUS_RegP_destroy_SETZ\@   | beware! reverse compare is ok
  addq.b #2,\p1
onvertSTATUS_RegP_destroy_SETZ\@:
  tst.b  VFLAG
  bpl.s  onvertSTATUS_RegP_destroy_SETV\@        | !!!
  ori.b  #V_FLAG,\p1
onvertSTATUS_RegP_destroy_SETV\@:
  .endm

  .macro ConvertRegP_STATUS p1
  btst   #V_FLAGB,\p1
  sne    VFLAG
  btst   #C_FLAGB,\p1
  sne    CFLAG
  move.b \p1,NFLAG
  lsl.w  #8,NFLAG  | sets NFLAG and clears ZFLAG
  btst   #Z_FLAGB,\p1
  seq    ZFLAG
  .endm

  .macro Call_Atari800_RunEsc
| move.l d7,-(a7)   !!!TEST!!!
  clr.l  -(a7)     |!!!TEST!!!
  move.b d7,(3,a7) |!!!TEST!!!
  ConvertSTATUS_RegP_destroy d0
  UPDATE_GLOBAL_REGS
  jsr _Atari800_RunEsc
  addq.l #4,a7
  UPDATE_LOCAL_REGS
  ConvertRegP_STATUS d0
  .endm

  .macro Call_Atari800_Exit_true
  pea    0x1.W
  jsr    _Atari800_Exit
  addq.l #4,a7
  tst.l  d0
  bne.s  all_Atari800_Exit_true_GOON\@
  clr.l  -(a7)
  jsr    _exit
all_Atari800_Exit_true_GOON\@:
  .endm

  .macro PLW p1 p2
  moveq  #0,\p2
  move.w regS,\p2
  addq.b #2,\p2     | wrong way around
  move.b (memory_pointer,\p2.l),\p1
  asl.w  #8,\p1
  subq.b #1,\p2
  or.b   (memory_pointer,\p2.l),\p1
  addq.b #1,\p2
  move.b \p2,_regS
  .endm

  .macro SetVFLAG
  st     VFLAG
  .endm

  .macro ClrVFLAG
  clr.b  VFLAG
  .endm

  .macro SetCFLAG
  st     CFLAG
  .endm

  .macro ClrCFLAG
  clr.b  CFLAG
  .endm

CPUGET:
_CPUGET:
  ConvertSTATUS_RegP d0
  move.b d0,_regP
  rts

CPUPUT:
_CPUPUT:
  move.b _regP,d0
  ConvertRegP_STATUS d0
  rts

NMI:
_NMI:
  lea    _memory,a0
  moveq  #0,d1
  move.w regS,d1
  move.b _regPC,(a0,d1.l)
  subq.b #1,d1
  move.b _regPC+1,(a0,d1.l)
  subq.b #1,d1
| move.b _regP,(a0,d1.l)  ;put P onto stack
  move.b _regP,d0       | Test
  andi.b #B_FLAGN,d0    | Test
  move.b d0,(a0,d1.l)   | Test
  subq.b #1,d1
  move.b d1,_regS
  SetI
  |put regPC & Stack pointer address on its place
  move.w (a0,0xfffa.l),d1
  LoHi d1
  move.w d1,_regPC
  addq.l #7,_xpos
 #ifdef MONITOR_BREAK
  addq.l #1,_ret_nesting
  #endif
  rts

_GO: |cycles (d0)

|  UWORD PC;
|  UBYTE S;
|  UBYTE A;
|  UBYTE X;
|  UBYTE Y;
;
|  UWORD  addr;
|  UBYTE  data;

|/*
|   This used to be in the main loop but has been removed to improve
|   execution speed. It does not seem to have any adverse effect on
|   the emulation for two reasons:-
;
|   1. NMI's can only be raised in atari_custom.c - there is
|      no way an NMI can be generated whilst in this routine.
;
|   2. The timing of the IRQs are not that critical.
|*/

  move.l  4(a7),d0
 #ifdef NEW_CYCLE_EXACT
  tst.b   _wsync_halt
  beq.s   NO_WS_HALT
  moveq.l #WSYNC_C-1,d1  | TEST : no -1 if bpl.s
  cmp.l   #-999,_cur_screen_pos
  beq.s   GO_now_cmp
  move.l  _antic2cpu_ptr,a0
  move.l  (a0,d1*4),d1
GO_now_cmp:
  add.l   _delayed_wsync,d1
  cmp.l   d0,d1
| bpl.s   TERM_GO        ; TEST
  bge     TERM_GO        | TEST
  addq.l  #1,d1          | TEST : not necessary if bpl.s
  move.l  d1,_xpos
  clr.b   _wsync_halt
  clr.l   _delayed_wsync
 #else
  tst.b   _wsync_halt
  beq.s   NO_WS_HALT
  moveq.l #WSYNC_C-1,d1  | TEST : no -1 if bpl.s
  cmp.l   d0,d1
| bpl.s   TERM_GO        ; TEST
  bge     TERM_GO        | TEST
  addq.l  #1,d1          | TEST : not necessary if bpl.s
  move.l  d1,_xpos
  clr.b   _wsync_halt
  #endif
NO_WS_HALT:
  move.l  d0,_xpos_limit |  needed for WSYNC store inside ANTIC
  movem.l d2-d7/a2-a6,-(a7)
  move.l  _xpos,CD
  lea     _memory,memory_pointer
  UPDATE_LOCAL_REGS
  ConvertRegP_STATUS d0
  lea     _attrib,attrib_pointer
  tst.b   _IRQ          | CPUCHECKIRQ
  beq     NEXTCHANGE_WITHOUT
  move.b  d0,d7
| and.b   #I_FLAG,d0 ;is interrupt active
  btst    #I_FLAG,d0
  bne     NEXTCHANGE_WITHOUT |yes, no other interrupt
  moveq   #0,d0
  move.w  regS,d0  | push PC and P to stack ( PHW + PHB ) start
  move.b  _regPC,(memory_pointer,d0.l)
  subq.b  #1,d0
  move.b  _regPC+1,(memory_pointer,d0.l)
  subq.b  #1,d0
| move.b  d7,(memory_pointer,d0.l) ;put P onto stack
  andi.b  #B_FLAGN,d7              | TEST
  move.b  d7,(memory_pointer,d0.l) | TEST
  subq.b  #1,d0
  move.b  d0,_regS      | push PC and P to stack ( PHW + PHB ) end
  SetI
  move.w  (memory_pointer,0xfffe.l),d0  | d0 already cleared from before
  LoHi d0
  move.l  d0,PC6502
  add.l   memory_pointer,PC6502
  addq.l  #7,CD
  clr.b   _IRQ |clear interrupt.....
 #ifdef MONITOR_BREAK
  addq.l  #1,_ret_nesting
  #endif
  bra     NEXTCHANGE_WITHOUT

|/*
|   =====================================
|   Extract Address if Required by Opcode
|   =====================================
|*/

|d0 contains final value for use in program

| addressing macros

  .macro NCYCLES_XY p1
  cmp.b  \p1,d7 | if ( (UBYTE) addr < X,Y ) ncycles++;
| bpl.s  CYCLES_XY_NCY_XY_NC\@
  bcc.s  CYCLES_XY_NCY_XY_NC\@
  addq.l #1,CD
CYCLES_XY_NCY_XY_NC\@:
  .endm

  .macro ABSOLUTE
  move.w (PC6502)+,d7
  LoHi d7 |d7 contains reversed value
  .endm

  .macro ABSOLUTE_X
  ABSOLUTE
  add.w  X,d7
  .endm

  .macro ABSOLUTE_X_NCY p1
  ABSOLUTE_X \p1
  NCYCLES_XY X
  .endm

  .macro ABSOLUTE_Y
  ABSOLUTE
  add.w  Y,d7
  .endm

  .macro ABSOLUTE_Y_NCY p1
  ABSOLUTE_Y \p1
  NCYCLES_XY Y
  .endm

  .macro IMMEDIATE p1
  move.b (PC6502)+,\p1
  .endm

  .macro INDIRECT_X
  move.b (PC6502)+,d7
  add.b  X,d7
  move.w (memory_pointer,d7.l),d7
  LoHi d7
  .endm

  .macro INDIRECT_Y
  move.b (PC6502)+,d7
  move.w (memory_pointer,d7.l),d7
  LoHi d7      |swap bytes
  add.w  Y,d7
  .endm

  .macro INDIRECT_Y_NCY
  INDIRECT_Y
  NCYCLES_XY Y
  .endm

  .macro ZPAGE
  move.b (PC6502)+,d7
  .endm

  .macro ZPAGE_X
  move.b (PC6502)+,d7
  add.b  X,d7
  .endm

  .macro ZPAGE_Y
  move.b (PC6502)+,d7
  add.b  Y,d7
  .endm

| miscellaneous macros

  .macro NEXTCHANGE_REG p1
  move.b \p1,ZFLAG
  bra.w  NEXTCHANGE_N
  .endm

| command macros

  .macro ROL_C p1
  add.b  CFLAG,CFLAG
  addx.b \p1,\p1 |left
  scs    CFLAG
  .endm

  .macro ROR_C p1
  add.b  CFLAG,CFLAG
  roxr.b #1,\p1
  scs    CFLAG
  .endm

  .macro ASL_C p1
  add.b  \p1,\p1 |left
  scs    CFLAG
  .endm

  .macro LSR_C p1
  lsr.b  #1,\p1
  scs    CFLAG
  .endm

| opcodes

| inofficial opcodes

|   unstable inofficial opcodes

opcode_93: |/* SHA (ab),y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] */
|  /* It seems previous memory value is important - also in 9f */;
  addq.l #cy_IndY2,CD
  move.b (PC6502)+,d7
  addq.b #1,d7
  move.b (memory_pointer,d7.l),d0
  addq.b #1,d0
  and.b  A,d0
  and.b  X,d0
  move.w (memory_pointer,d7.l),d7
  LoHi d7      |swap bytes
  add.w  Y,d7
  tst.b  (attrib_pointer,d7.l)    | PUTANYBYTE
  bne.w  A800PUTB
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

opcode_9f: |/* SHA abcd,y [unofficial, UNSTABLE - Store A AND X AND (H+1) ?] */
  addq.l #cy_IndY2,CD
  move.w (PC6502)+,d7
  move.b d7,d0
  LoHi d7 |d7 contains reversed value
  addq.b #1,d0
  and.b  A,d0
  and.b  X,d0
  add.w  Y,d7
  tst.b  (attrib_pointer,d7.l)    | PUTANYBYTE
  bne.w  A800PUTB
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

opcode_9e: |/* SHX abcd,y [unofficial - Store X and (H+1)] (Fox) */
|  /* Seems to be stable */
  addq.l #cy_IndY2,CD
  move.w (PC6502)+,d7
  move.b d7,d0
  LoHi d7 |d7 contains reversed value
  addq.b #1,d0
  and.b  X,d0
  add.w  Y,d7
  tst.b  (attrib_pointer,d7.l)    | PUTANYBYTE
  bne.w  A800PUTB
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

opcode_9c: |/* SHY abcd,x [unofficial - Store Y and (H+1)] (Fox) */
|  /* Seems to be stable */
  addq.l #cy_AbsX2,CD
  move.w (PC6502)+,d7
  move.b d7,d0
  LoHi d7 |d7 contains reversed value
  addq.b #1,d0
  and.b  A,d0
  and.b  Y,d0
  add.w  X,d7
  tst.b  (attrib_pointer,d7.l)    | PUTANYBYTE
  bne.w  A800PUTB
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

opcode_9b: |/* SHS abcd,y [unofficial, UNSTABLE] (Fox) */
|  /* Transfer A AND X to S, then store S AND (H+1)] */
|  /* S seems to be stable, only memory values vary */
  addq.l #cy_IndY2,CD
  move.w (PC6502)+,d7
  move.b d7,d0
  LoHi d7 |d7 contains reversed value
  move.b A,_regS
  and.b  X,_regS
  addq.b #1,d0
  and.b  _regS,d0
  add.w  Y,d7
  tst.b  (attrib_pointer,d7.l)    | PUTANYBYTE
  bne.w  A800PUTB
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

|   stable inofficial opcodes

opcode_6b: |/* ARR #ab [unofficial - Acc AND Data, ROR result] */
| not optimized because I think it will never be executed anyway
  addq.l #cy_Imm,CD
  IMMEDIATE ZFLAG
  and.b  A,ZFLAG
  btst   #D_FLAGB,_regP
  beq.s  pcode_6b_6b_noBCD
| 'BCD fixup'
  move.b ZFLAG,d7
  ROR_C ZFLAG
  move.b ZFLAG,A
  move.b d7,VFLAG  |VFLAG
  eor.b  ZFLAG,VFLAG
  and.b  #0x40,VFLAG
  sne    VFLAG
  move.b A,d7
  move.b A,d7
  move.b d7,d0
  andi.b #15,d0
  move.b d7,CFLAG
  andi.b #1,CFLAG
  add.b  CFLAG,d0
  cmpi.b #6,d0     | check for >5
  bmi.s  pcode_6b_6b_bcd1  | <=5
  move.b A,CFLAG
  and.b  #240,CFLAG
  move.b A,d0
  addq.b #6,d0
  and.b  #15,d0
  move.b CFLAG,A
  or.b   d0,A
pcode_6b_6b_bcd1:
  move.b d7,d0
  andi.b #0xf0,d0
  move.b d7,CFLAG
  andi.b #16,CFLAG
  cmpi.b #0x51,d0   | check for >0x50
  bmi.s  pcode_6b_6b_bcd2  | <=0x50
  move.b A,CFLAG
  and.b  #15,CFLAG
  move.b A,d0
  add.b  #0x60,d0
  and.b  #0xf0,d0
  move.b CFLAG,A
  or.w   d0,A
  SetCFLAG
  bra.w  NEXTCHANGE_N
pcode_6b_6b_bcd2:
  ClrCFLAG
  bra.w  NEXTCHANGE_N
| Binary
pcode_6b_6b_noBCD:
  ROR_C ZFLAG
  move.b ZFLAG,A
  move.b A,VFLAG   |VFLAG
  lsr.b  #6,VFLAG
  move.b A,CFLAG
  lsr.b  #5,CFLAG
  eor.b  CFLAG,VFLAG
  and.b  #1,VFLAG
  sne    VFLAG
  move.b A,CFLAG   |CFLAG
  and.b  #0x40,CFLAG
  sne    CFLAG
  bra.w  NEXTCHANGE_N

opcode_02: |/* CIM [unofficial - crash immediate] */
opcode_12:
opcode_22:
opcode_32:
opcode_42:
opcode_52:
opcode_62:
opcode_72:
opcode_92:
opcode_b2:
  addq.l #cy_CIM,CD
  subq.w #1,PC6502
  ConvertSTATUS_RegP_destroy d0
  UPDATE_GLOBAL_REGS
 #ifdef CRASH_MENU
  move.w PC6502,_crash_address
  addq.w #1,PC6502
  move.w PC6502,crash_afterCIM
  move.l d7,_crash_code
  move.l _atari_screen,-(sp)
  jsr    _ui
 #else
 #ifdef MONITOR_BREAK
  moveq  #1,d0
  move.l d0,_break_cim
 #else
  Call_Atari800_Exit_true
  #endif
  UPDATE_LOCAL_REGS
  ConvertRegP_STATUS d0
  #endif
  bra.w  NEXTCHANGE_WITHOUT

opcode_07: |/* ASO ab [unofficial - ASL then ORA with Acc] */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  ASL_C d0
  move.b d0,(memory_pointer,d7.l) | PUTZPBYTE
  or.b   d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_17: |/* ASO ab,x [unofficial - ASL then ORA with Acc] */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  ASL_C d0
  move.b d0,(memory_pointer,d7.l) | PUTZPBYTE
  or.b   d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

  .macro ASO_C_CONT   /* [unofficial - ASL Mem, then ORA with A] */
  move.b (attrib_pointer,d7.l),d0
  bne.s  ASO_Getbyte_ROMHW
  move.b (memory_pointer,d7.l),d0 |get byte
  ASL_C d0
  bra    ASO_STORE_MEM
  .endm

opcode_03: |/* ASO (ab,x) [unofficial - ASL then ORA with Acc] */
  addq.l #cy_IndX_RW,CD
  INDIRECT_X
  ASO_C_CONT

opcode_13: |/* ASO (ab),y [unofficial - ASL then ORA with Acc] */
  addq.l #cy_IndY_RW,CD
  INDIRECT_Y
  ASO_C_CONT

opcode_0f: |/* ASO abcd [unofficial - ASL then ORA with Acc] */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  ASO_C_CONT

opcode_1b: |/* ASO abcd,y [unofficial - ASL then ORA with Acc] */
  addq.l #cy_AbsY_RW,CD
  ABSOLUTE_Y
  ASO_C_CONT

opcode_1f: |/* ASO abcd,x [unofficial - ASL then ORA with Acc] */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  ASO_C_CONT

ASO_Getbyte_ROMHW:
  cmp.b  #isHARDWARE,d0
  beq.s  SO_Getbyte_ROMHW_Getbyte_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  ASL_C d0
  bra.s  ASO_NOW_ORA
SO_Getbyte_ROMHW_Getbyte_HW:
  RMW_GETBYTE
  ASL_C d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,d0
  bra.s  ASO_NOW_ORA
ASO_STORE_MEM:
  move.b d0,(memory_pointer,d7.l)
ASO_NOW_ORA:
  or.b   d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_27: /* RLA ab [unofficial - ROL Mem, then AND with A] */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  ROL_C d0
  move.b d0,(memory_pointer,d7.l) | PUTZPBYTE
  and.b  d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_37: /* RLA ab,x [unofficial - ROL Mem, then AND with A] */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  ROL_C d0
  move.b d0,(memory_pointer,d7.l) | PUTZPBYTE
  and.b  d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

  .macro RLA_C_CONT   /* [unofficial - ROL Mem, then AND with A] */
  move.b (attrib_pointer,d7.l),d0
  bne.w  RLA_Getbyte_ROMHW
  move.b (memory_pointer,d7.l),d0 |get byte
  ROL_C d0
  bra.w  RLA_STORE_MEM
  .endm

opcode_23: |/* RLA (ab,x) [unofficial - ROL Mem, then AND with A] */
  addq.l #cy_IndX_RW,CD
  INDIRECT_X
  RLA_C_CONT

opcode_33: |/* RLA (ab),y [unofficial - ROL Mem, then AND with A] */
  addq.l #cy_IndY_RW,CD
  INDIRECT_Y
  RLA_C_CONT

opcode_2f: |/* RLA abcd [unofficial - ROL Mem, then AND with A] */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  RLA_C_CONT

opcode_3b: |/* RLA abcd,y [unofficial - ROL Mem, then AND with A] */
  addq.l #cy_AbsY_RW,CD
  ABSOLUTE_Y
  RLA_C_CONT

opcode_3f: |/* RLA abcd,x [unofficial - ROL Mem, then AND with A] */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  RLA_C_CONT

RLA_Getbyte_ROMHW:
  cmp.b  #isHARDWARE,d0
  beq.s  LA_Getbyte_ROMHW_Getbyte_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  ROL_C d0
  bra.s  RLA_NOW_AND
LA_Getbyte_ROMHW_Getbyte_HW:
  RMW_GETBYTE
  ROL_C d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,d0
  bra.s  RLA_NOW_AND
RLA_STORE_MEM:
  move.b d0,(memory_pointer,d7.l)
RLA_NOW_AND:
  and.b  d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_47: |/* LSE ab [unofficial - LSR then EOR result with A] */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  LSR_C d0
  move.b d0,(memory_pointer,d7.l) | PUTZPBYTE
  eor.b  d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_57: |/* LSE ab,x [unofficial - LSR then EOR result with A] */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  LSR_C d0
  move.b d0,(memory_pointer,d7.l) | PUTZPBYTE
  eor.b  d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

  .macro LSE_C_CONT   /* [unofficial - LSR Mem then EOR with A] */
  move.b (attrib_pointer,d7.l),d0
  bne.s  LSE_Getbyte_ROMHW
  move.b (memory_pointer,d7.l),d0 |get byte
  LSR_C d0
  bra    LSE_STORE_MEM
  .endm

opcode_43: |/* LSE (ab,x) [unofficial] */
  addq.l #cy_IndX_RW,CD
  INDIRECT_X
  LSE_C_CONT

opcode_53: |/* LSE (ab),y [unofficial] */
  addq.l #cy_IndY_RW,CD
  INDIRECT_Y
  LSE_C_CONT

opcode_4f: |/* LSE abcd [unofficial] */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  LSE_C_CONT

opcode_5b: |/* LSE abcd,y [unofficial] */
  addq.l #cy_AbsY_RW,CD
  ABSOLUTE_Y
  LSE_C_CONT

opcode_5f: |/* LSE abcd,x [unofficial] */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  LSE_C_CONT

LSE_Getbyte_ROMHW:
  cmp.b  #isHARDWARE,d0
  beq.s  SE_Getbyte_ROMHW_Getbyte_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  LSR_C d0
  bra.s  LSE_NOW_EOR
SE_Getbyte_ROMHW_Getbyte_HW:
  RMW_GETBYTE
  LSR_C d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,d0
  bra.s  LSE_NOW_EOR
LSE_STORE_MEM:
  move.b d0,(memory_pointer,d7.l)
LSE_NOW_EOR:
  eor.b  d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_4b: |/* ALR #ab [unofficial - Acc AND Data, LSR result] */
  addq.l #cy_Imm,CD
  IMMEDIATE ZFLAG
  and.b  A,ZFLAG
  LSR_C ZFLAG
  bra.w  NEXTCHANGE_N

opcode_67: |/* RRA ab [unofficial - ROR Mem, then ADC to Acc] */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  ROR_C d0
  move.b d0,(memory_pointer,d7.l) | PUTZPBYTE
  bra.w  adc

opcode_77: |/* RRA ab,x [unofficial - ROR Mem, then ADC to Acc] */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  ROR_C d0
  move.b d0,(memory_pointer,d7.l) | PUTZPBYTE
  bra.w  adc

  .macro GETANYBYTE_RRA
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  bne    RRA_RAMROM
  RMW_GETBYTE
  bra    RRA_C_CONT
  .endm

opcode_63: |/* RRA (ab,x) [unofficial - ROR Mem, then ADC to Acc] */
  addq.l #cy_IndX_RW,CD
  INDIRECT_X
  GETANYBYTE_RRA

opcode_73: |/* RRA (ab),y [unofficial - ROR Mem, then ADC to Acc] */
  addq.l #cy_IndY_RW,CD
  INDIRECT_Y
  GETANYBYTE_RRA

opcode_6f: |/* RRA abcd [unofficial - ROR Mem, then ADC to Acc] */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  GETANYBYTE_RRA

opcode_7b: |/* RRA abcd,y [unofficial - ROR Mem, then ADC to Acc] */
  addq.l #cy_AbsY_RW,CD
  ABSOLUTE_Y
  GETANYBYTE_RRA

opcode_7f: |/* RRA abcd,x [unofficial - ROR Mem, then ADC to Acc] */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  GETANYBYTE_RRA

RRA_RAMROM:
  move.b (memory_pointer,d7.l),d0 |get byte
RRA_C_CONT:        |/* [unofficial - ROR Mem, then ADC to Acc] */
  ROR_C d0
  tst.b  (attrib_pointer,d7.l)
  bne.s  RA_C_CONT_ROM_OR_HW
  move.b d0,(memory_pointer,d7.l)
  bra.w  adc
RA_C_CONT_ROM_OR_HW:
  cmp.b  #isROM,(attrib_pointer,d7.l)
  beq.w  adc       |ROM ?
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,d0
  bra.w  adc

opcode_87: |/* SAX ab [unofficial - Store result A AND X] */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b A,d0
  and.b  X,d0
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

opcode_97: |/* SAX ab,y [unofficial - Store result A AND X] */
  addq.l #cy_ZPY,CD
  ZPAGE_Y
  move.b A,d0
  and.b  X,d0
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

opcode_83: |/* SAX (ab,x) [unofficial - Store result A AND X] */
  addq.l #cy_IndX,CD
  INDIRECT_X
  move.b A,d0
  and.b  X,d0
  tst.b  (attrib_pointer,d7.l)    | PUTANYBYTE
  bne.w  A800PUTB
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

opcode_8f: |/* SAX abcd [unofficial - Store result A AND X] */
  addq.l #cy_Abs,CD
  ABSOLUTE
  move.b A,d0
  and.b  X,d0
  tst.b  (attrib_pointer,d7.l)    | PUTANYBYTE
  bne.w  A800PUTB
  move.b d0,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT

opcode_a7: |/* LAX ab [unofficial] - LDA + LDX */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),A  | GETZPBYTE
  move.b A,X
  NEXTCHANGE_REG A

opcode_b7: |/* LAX ab,y [unofficial] - LDA + LDX */
  addq.l #cy_ZPY,CD
  ZPAGE_Y
  move.b (memory_pointer,d7.l),A  | GETZPBYTE
  move.b A,X
  NEXTCHANGE_REG A

  .macro GETANYBYTE_LAX
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  LAX_HW
  move.b (memory_pointer,d7.l),A  |get byte
  move.b A,X
  NEXTCHANGE_REG A
  .endm

opcode_a3: |/* LAX (ind,x) [unofficial] - LDA + LDX */
  addq.l #cy_IndX,CD
  INDIRECT_X
  GETANYBYTE_LAX

opcode_b3: |/* LAX (ind),y [unofficial] - LDA + LDX */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  GETANYBYTE_LAX

opcode_af: |/* LAX abcd [unofficial] - LDA + LDX */
  addq.l #cy_Abs,CD
  ABSOLUTE
  GETANYBYTE_LAX

opcode_bf: |/* LAX abcd,y [unofficial] - LDA + LDX */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  GETANYBYTE_LAX

LAX_HW:
  EXE_GETBYTE
  move.b d0,A
  move.b A,X
  NEXTCHANGE_REG A

opcode_bb: |/* LAS abcd,y [unofficial - AND S with Mem, transfer to A and X */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  bne.s  pcode_bb_Getbyte_RAMROM
  EXE_GETBYTE
  bra.s  pcode_bb_AFTER_READ
pcode_bb_Getbyte_RAMROM:
  move.b (memory_pointer,d7.l),d0 |get byte
pcode_bb_AFTER_READ:
  and.b  _regS,d0
  move.b d0,A
  move.b d0,X
  move.b d0,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_c7: |/* DCM ab [unofficial - DEC Mem then CMP with Acc] */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  subq.b #1,d0
  move.b d0,(memory_pointer,d7.l)
  bra.w  COMPARE_A

opcode_d7: |/* DCM ab,x [unofficial - DEC Mem then CMP with Acc] */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  subq.b #1,d0
  move.b d0,(memory_pointer,d7.l)
  bra.w  COMPARE_A

  .macro DCM_C_CONT   /* [unofficial - DEC Mem then CMP with Acc] */
  tst.b  (attrib_pointer,d7.l)
  bne.s  DCM_ROM_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  subq.b #1,d0
  move.b d0,(memory_pointer,d7.l)
  bra.w  COMPARE_A
  .endm

opcode_c3: |/* DCM (ab,x) [unofficial - DEC Mem then CMP with Acc] */
  addq.l #cy_IndX_RW,CD
  INDIRECT_X
  DCM_C_CONT

opcode_d3: |/* DCM (ab),y [unofficial - DEC Mem then CMP with Acc] */
  addq.l #cy_IndY_RW,CD
  INDIRECT_Y
  DCM_C_CONT

opcode_cf: |/* DCM abcd [unofficial] - DEC Mem then CMP with Acc] */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  DCM_C_CONT

opcode_db: |/* DCM abcd,y [unofficial - DEC Mem then CMP with Acc] */
  addq.l #cy_AbsY_RW,CD
  ABSOLUTE_Y
  DCM_C_CONT

DCM_ROM_HW:
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  CM_ROM_HW_Getbyte_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  subq.b #1,d0
  bra.w  COMPARE_A
CM_ROM_HW_Getbyte_HW:
  RMW_GETBYTE
  subq.b #1,d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,d0
  bra.w  COMPARE_A

opcode_df: |/* DCM abcd,x [unofficial - DEC Mem then CMP with Acc] */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  DCM_C_CONT

opcode_cb: |/* SBX #ab [unofficial - store (A AND X - Mem) in X] */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
  and.b  A,X
  subq.b #1,CFLAG
  subx.b d0,X
  scc    CFLAG
  NEXTCHANGE_REG X

opcode_e7: |/* INS ab [unofficial] - INC Mem then SBC with Acc] */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  addq.b #1,d0
  move.b d0,(memory_pointer,d7.l)
  bra sbc

opcode_f7: |/* INS ab,x [unofficial] - INC Mem then SBC with Acc] */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0 | GETZPBYTE
  addq.b #1,d0
  move.b d0,(memory_pointer,d7.l)
  bra sbc

  .macro INS_C_CONT   /* [unofficial - INC Mem then SBC with Acc] */
  tst.b  (attrib_pointer,d7.l)
  bne.s  INS_ROM_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  addq.b #1,d0
  move.b d0,(memory_pointer,d7.l)
  bra.w  sbc
  .endm

opcode_e3: |/* INS (ab,x) [unofficial] - INC Mem then SBC with Acc] */
  addq.l #cy_IndX_RW,CD
  INDIRECT_X
  INS_C_CONT

opcode_f3: |/* INS (ab),y [unofficial] - INC Mem then SBC with Acc] */
  addq.l #cy_IndY_RW,CD
  INDIRECT_Y
  INS_C_CONT

opcode_ef: |/* INS abcd [unofficial] - INC Mem then SBC with Acc] */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  INS_C_CONT

opcode_fb: |/* INS abcd,y [unofficial] - INC Mem then SBC with Acc] */
  addq.l #cy_AbsY_RW,CD
  ABSOLUTE_Y
  INS_C_CONT

INS_ROM_HW:
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  NS_ROM_HW_Getbyte_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  addq.b #1,d0
  bra.w  sbc
NS_ROM_HW_Getbyte_HW:
  RMW_GETBYTE
  addq.b #1,d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,d0
  bra.w  sbc

opcode_ff: |/* INS abcd,x [unofficial] - INC Mem then SBC with Acc] */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  INS_C_CONT

opcode_80: |/* NOP #ab [unofficial - skip byte] */
opcode_82:
opcode_89:
opcode_c2:
opcode_e2:
  addq.l #cy_NOP2,CD
  addq.l #1,PC6502
  bra.w  NEXTCHANGE_WITHOUT

opcode_04: |/* NOP ab [unofficial - skip byte] */
opcode_44:
opcode_64:
  addq.l #cy_NOP3,CD
  addq.l #1,PC6502
  bra.w  NEXTCHANGE_WITHOUT

opcode_14: |/* NOP ab,x [unofficial - skip byte] */
opcode_34:
opcode_54:
opcode_74:
opcode_d4:
opcode_f4:
  addq.l #cy_NOP4,CD
  addq.l #1,PC6502
  bra.w  NEXTCHANGE_WITHOUT

opcode_0b: |/* ANC #ab [unofficial - AND then copy N to C */
opcode_2b:
  addq.l #cy_Imm,CD
  and.b  (PC6502)+,A
  move.b A,ZFLAG
  smi    CFLAG
  bra.w  NEXTCHANGE_N

opcode_ab: |/* ANX #ab [unofficial - AND #ab, then TAX] */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
  and.b  d0,A
  move.b A,X
  NEXTCHANGE_REG A

opcode_8b: |/* ANE #ab [unofficial - A AND X AND (Mem OR 0xEF) to Acc] */
  addq.l #cy_Imm,CD
  move.b (PC6502)+,d0
  and.b  X,A
  move.b A,ZFLAG
  and.b  d0,ZFLAG
  or.b   #0xef,d0
  and.b  d0,A
  bra.w  NEXTCHANGE_N

opcode_0c: |/* NOP abcd [unofficial - skip word] */
  addq.l #cy_SKW,CD
  addq.l #2,PC6502
  bra.w  NEXTCHANGE_WITHOUT

opcode_1c: |/* NOP abcd,x [unofficial - skip word] */
opcode_3c:
opcode_5c:
opcode_7c:
opcode_dc:
opcode_fc:
  addq.l #cy_SKW,CD
  move.b (PC6502),d7
  add.l  X,d7
  bcs.s  pcode_fc_SOLVE_PB
  addq.l #cy_Bcc1,CD
  addq.l #2,PC6502
  bra.w  NEXTCHANGE_WITHOUT
pcode_fc_SOLVE_PB:
  addq.l #cy_Bcc2,CD
  addq.l #2,PC6502
  bra.w  NEXTCHANGE_WITHOUT

opcode_1a: |/* NOP [unofficial] */
opcode_3a:
opcode_5a:
opcode_7a:
opcode_da:
opcode_fa:
  addq.l #cy_NOP,CD
  bra.w  NEXTCHANGE_WITHOUT

| official opcodes

opcode_00: |/* BRK */
 #ifdef MONITOR_BREAK
  tst.l  _brkhere
  beq.s  pcode_00_oc_00_norm
  move.b #1,_break_here
  jsr    go_monitor
  bra.w  NEXTCHANGE_WITHOUT
pcode_00_oc_00_norm:
  #endif
  addq.l #cy_BRK,CD
| btst   #I_FLAGB,_regP
| bne.w  NEXTCHANGE_WITHOUT
  SetB
  move.l PC6502,d7
  sub.l  memory_pointer,d7
  addq.w #1,d7
  moveq  #0,d0                    | PHW + PHP
  move.w regS,d0
  subq.b #1,d0     | wrong way around
  move.b d7,(memory_pointer,d0.l)
  addq.b #1,d0
  LoHi d7
  move.b d7,(memory_pointer,d0.l)
  subq.b #2,d0
  ConvertSTATUS_RegP d7
| move.b d7,_regP       ;put result to _regP ! TEST !!!
  move.b d7,(memory_pointer,d0.l)
  subq.b #1,d0
  move.b d0,_regS
  SetI
  move.w (memory_pointer,0xfffe.l),d7
  LoHi d7
  move.l d7,PC6502
  add.l  memory_pointer,PC6502
 #ifdef MONITOR_BREAK
  addq.l #1,_ret_nesting
  #endif
  bra.w  NEXTCHANGE_WITHOUT

opcode_08: |/* PHP */
  addq.l #cy_RegPH,CD
  move.w regS,d7
  ConvertSTATUS_RegP d0
  move.b d0,(memory_pointer,d7.l)
  subq.b #1,d7
  move.b d7,_regS
  bra.w  NEXTCHANGE_WITHOUT

opcode_28: |/* PLP */
  addq.l #cy_RegPL,CD
  moveq  #0,d0          | PLP
  move.w regS,d0
  addq.b #1,d0
| move.b (memory_pointer,d0.l),_regP
  move.b (memory_pointer,d0.l),d7 | TEST
  ori.b  #0x30,d7                  | TEST
  move.b d7,_regP                 | TEST
  ConvertRegP_STATUS d7
  move.b d0,_regS
  tst.b  _IRQ           | CPUCHECKIRQ
  beq.w  NEXTCHANGE_WITHOUT
  btst   #I_FLAGB,d7
  bne.w  NEXTCHANGE_WITHOUT
| moveq  #0,d0
| move.w regS,d0        ; push PC and P to stack ( PHW + PHB ) start
  subq.b #2,d0          | but do it the wrong way around for optim.
  andi.b  #B_FLAGN,d7              ;
  move.b  d7,(memory_pointer,d0.l) | Push P
  move.l PC6502,d7
  sub.l  memory_pointer,d7
  addq.b #1,d0     | wrong way around
  move.b d7,(memory_pointer,d0.l)  | Push High
  addq.b #1,d0
  LoHi d7
  move.b d7,(memory_pointer,d0.l)  | Push Low
  subq.b #3,d0
  move.b d0,_regS       | push PC and P to stack ( PHW + PHB ) end
  SetI
  move.w (memory_pointer,0xfffe.l),d7
  LoHi d7
  move.l d7,PC6502
  add.l  memory_pointer,PC6502
  addq.l #7,CD
 #ifdef MONITOR_BREAK
  addq.l #1,_ret_nesting
  #endif
  bra.w  NEXTCHANGE_WITHOUT

opcode_48: |/* PHA */
  addq.l #cy_RegPH,CD
  move.w regS,d7
  move.b A,(memory_pointer,d7.l)
  subq.b #1,d7
  move.b d7,_regS
  bra.w  NEXTCHANGE_WITHOUT

opcode_68: |/* PLA */
  addq.l #cy_RegPL,CD
  move.w regS,d7
  addq.b #1,d7
  move.b (memory_pointer,d7.l),A
  move.b d7,_regS
  NEXTCHANGE_REG A

  .macro OR_ANYBYTE
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  OR_HW
  or.b   (memory_pointer,d7.l),A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N
  .endm

opcode_01: |/* ORA (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  OR_ANYBYTE

opcode_11: |/* ORA (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  OR_ANYBYTE

OR_HW:
  EXE_GETBYTE
  or.b   d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_0d: |/* ORA abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  OR_ANYBYTE

opcode_19: |/* ORA abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  OR_ANYBYTE

opcode_1d: |/* ORA abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  OR_ANYBYTE

opcode_05: |/* ORA ab */
  addq.l #cy_ZP,CD
  ZPAGE
  or.b   (memory_pointer,d7.l),A  | OR ZPBYTE
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_15: |/* ORA ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  or.b   (memory_pointer,d7.l),A  | OR ZPBYTE
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_09: |/* ORA #ab */
  addq.l #cy_Imm,CD
  or.b   (PC6502)+,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

  .macro AND_ANYBYTE
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  AND_HW
  and.b  (memory_pointer,d7.l),A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N
  .endm

opcode_21: |/* AND (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  AND_ANYBYTE

opcode_31: |/* AND (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  AND_ANYBYTE

AND_HW:
  EXE_GETBYTE
  and.b  d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_2d: |/* AND abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  AND_ANYBYTE

opcode_39: |/* AND abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  AND_ANYBYTE

opcode_3d: |/* AND abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  AND_ANYBYTE

opcode_25: |/* AND ab */
  addq.l #cy_ZP,CD
  ZPAGE
  and.b  (memory_pointer,d7.l),A  | AND ZPBYTE
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_35: |/* AND ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  and.b  (memory_pointer,d7.l),A  | AND ZPBYTE
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_29: |/* AND #ab */
  addq.l #cy_Imm,CD
  and.b  (PC6502)+,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

  .macro EOR_C_CONT
  eor.b  d0,A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N
  .endm

  .macro GETANYBYTE_EOR
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  EOR_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  EOR_C_CONT
  .endm

opcode_41: |/* EOR (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  GETANYBYTE_EOR

opcode_51: |/* EOR (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  GETANYBYTE_EOR

EOR_HW:
  EXE_GETBYTE
  EOR_C_CONT

opcode_4d: |/* EOR abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  GETANYBYTE_EOR

opcode_59: |/* EOR abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  GETANYBYTE_EOR

opcode_5d: |/* EOR abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  GETANYBYTE_EOR

opcode_45: |/* EOR ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0
  EOR_C_CONT

opcode_55: |/* EOR ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0
  EOR_C_CONT

opcode_49: |/* EOR #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE d0     | because eor only works with registers !
  EOR_C_CONT

opcode_0a: |/* ASLA */
  addq.l #cy_RegChg,CD
  ASL_C A
  NEXTCHANGE_REG A

opcode_06: |/* ASL ab */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  move.b (memory_pointer,d7.l),ZFLAG
  ASL_C ZFLAG
  move.b ZFLAG,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_N

opcode_16: |/* ASL ab,x */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),ZFLAG
  ASL_C ZFLAG
  move.b ZFLAG,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_N

  .macro RPW_ASL_C
  move.b  (attrib_pointer,d7.l),d0
  bne.s  RPW_HW_ASL
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  ASL_C ZFLAG
  move.b ZFLAG,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_N
  .endm

opcode_0e: |/* ASL abcd */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  RPW_ASL_C

opcode_1e: |/* ASL abcd,x */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  RPW_ASL_C

RPW_HW_ASL:
  cmp.b  #isROM,d0
  beq.s  RPW_ROM_ASL
  RMW_GETBYTE
  ASL_C d0
  ext.w  d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,ZFLAG
  bra.w  NEXTCHANGE_WITHOUT
RPW_ROM_ASL:
  move.b (memory_pointer,d7.l),ZFLAG | get byte
  ASL_C ZFLAG
  bra.w  NEXTCHANGE_N

opcode_2a: |/* ROLA */
  addq.l #cy_RegChg,CD
  ROL_C A
  NEXTCHANGE_REG A

opcode_26: |/* ROL ab */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  move.b (memory_pointer,d7.l),ZFLAG   | GETZPBYTE
  ROL_C ZFLAG
  move.b ZFLAG,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_N

opcode_36: |/* ROL ab,x */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),ZFLAG   | GETZPBYTE
  ROL_C ZFLAG
  move.b ZFLAG,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_N

  .macro RPW_ROL_C
  move.b (attrib_pointer,d7.l),d0
  bne.s  RPW_HW_ROL
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  ROL_C ZFLAG
  move.b ZFLAG,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_N
  .endm

opcode_2e: |/* ROL abcd */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  RPW_ROL_C

opcode_3e: |/* ROL abcd,x */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  RPW_ROL_C

RPW_HW_ROL:
  cmp.b  #isROM,d0
  beq.s  RPW_ROM_ROL
  RMW_GETBYTE
  ROL_C d0
  ext.w  d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,ZFLAG
  bra.w  NEXTCHANGE_WITHOUT
RPW_ROM_ROL:
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  ROL_C ZFLAG
  bra.w  NEXTCHANGE_N

opcode_4a: |/* LSRA */
  addq.l #cy_RegChg,CD
  clr.w  NFLAG
  lsr.b  #1,A
  scs    CFLAG
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_WITHOUT

opcode_46: |/* LSR ab */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  clr.w  NFLAG
  move.b (memory_pointer,d7.l),ZFLAG   | GETZPBYTE
  lsr.b  #1,ZFLAG
  scs    CFLAG
  move.b ZFLAG,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_WITHOUT

opcode_56: |/* LSR ab,x */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  clr.w  NFLAG
  move.b (memory_pointer,d7.l),ZFLAG   | GETZPBYTE
  lsr.b  #1,ZFLAG
  scs    CFLAG
  move.b ZFLAG,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_WITHOUT

  .macro RPW_LSR_C
  clr.w  NFLAG
  move.b (attrib_pointer,d7.l),d0
  bne.s  RPW_HW_LSR
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  LSR_C ZFLAG
  move.b ZFLAG,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT
  .endm

opcode_4e: |/* LSR abcd */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  RPW_LSR_C

opcode_5e: |/* LSR abcd,x */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  RPW_LSR_C

RPW_HW_LSR:
  cmp.b  #isROM,d0
  beq.s  RPW_ROM_LSR
  RMW_GETBYTE
  LSR_C d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,ZFLAG
  bra.w  NEXTCHANGE_WITHOUT
RPW_ROM_LSR:
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  LSR_C ZFLAG
  bra.w  NEXTCHANGE_WITHOUT

opcode_6a: |/* RORA */
  addq.l #cy_RegChg,CD
  ROR_C A
  NEXTCHANGE_REG A

opcode_66: |/* ROR ab */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  add.b  CFLAG,CFLAG
  move.b (memory_pointer,d7.l),ZFLAG   | GETZPBYTE
  roxr.b #1,ZFLAG
  scs    CFLAG
  move.b ZFLAG,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_N

opcode_76: |/* ROR ab,x */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  add.b  CFLAG,CFLAG
  move.b (memory_pointer,d7.l),ZFLAG   | GETZPBYTE
  roxr.b #1,ZFLAG
  scs    CFLAG
  move.b ZFLAG,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_N

  .macro RPW_ROR_C
  move.b (attrib_pointer,d7.l),d0
  bne.s  RPW_HW_ROR
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  ROR_C ZFLAG
  move.b ZFLAG,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_N
  .endm

opcode_6e: |/* ROR abcd */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  RPW_ROR_C

opcode_7e: |/* ROR abcd,x */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  RPW_ROR_C

RPW_HW_ROR:
  cmp.b  #isROM,d0
  beq.s  RPW_ROM_ROR
  RMW_GETBYTE
  ROR_C d0
  ext.w  d0
  move.l d0,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,ZFLAG
  bra.w  NEXTCHANGE_WITHOUT
RPW_ROM_ROR:
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  ROR_C ZFLAG
  bra.w  NEXTCHANGE_N

opcode_18: |/* CLC */
  addq.l #cy_FlagCS,CD
  ClrCFLAG
  bra.w  NEXTCHANGE_WITHOUT

opcode_38: |/* SEC */
  addq.l #cy_FlagCS,CD
  SetCFLAG
  bra.w  NEXTCHANGE_WITHOUT

opcode_58: |/* CLI */
  addq.l #cy_FlagCS,CD
  ClrI
  tst.b  _IRQ      | ~ CPUCHECKIRQ
  beq.w  NEXTCHANGE_WITHOUT
  move.l PC6502,d7
  sub.l  memory_pointer,d7
  moveq  #0,d0                    | PHW + PHP (B0)
  move.w regS,d0
  subq.b #1,d0     | wrong way around
  move.b d7,(memory_pointer,d0.l)
  addq.b #1,d0
  LoHi d7
  move.b d7,(memory_pointer,d0.l)
  subq.b #2,d0
  ConvertSTATUS_RegP d7
| move.b d7,_regP       ;put result to _regP ! TEST !!!
  andi.b #B_FLAGN,d7              | TEST
  move.b d7,(memory_pointer,d0.l)
  subq.b #1,d0
  move.b d0,_regS
  SetI
  move.w (memory_pointer,0xfffe.l),d7
  LoHi d7
  move.l d7,PC6502
  add.l  memory_pointer,PC6502
  clr.b  _IRQ
  addq.l #7,CD
 #ifdef MONITOR_BREAK
  addq.l #1,_ret_nesting
  #endif
  bra.w  NEXTCHANGE_WITHOUT

opcode_78: |/* SEI */
  addq.l #cy_FlagCS,CD
  SetI
  bra.w  NEXTCHANGE_WITHOUT

opcode_b8: |/* CLV */
  addq.l #cy_FlagCS,CD
  ClrVFLAG
  bra.w  NEXTCHANGE_WITHOUT

opcode_d8: |/* CLD */
  addq.l #cy_FlagCS,CD
  ClrD
  bra.w  NEXTCHANGE_WITHOUT

opcode_f8: |/* SED */
  addq.l #cy_FlagCS,CD
  SetD
  bra.w  NEXTCHANGE_WITHOUT

  .macro JMP_C
  move.w (PC6502)+,d7
  LoHi d7   |(in d7 address where we want to jump)
  lea (memory_pointer,d7.l),PC6502
  bra.w  NEXTCHANGE_WITHOUT
  .endm

opcode_4c: |/* JMP abcd */
 #ifdef MONITOR_BREAK
  move.l PC6502,d7 |current pointer
  sub.l  memory_pointer,d7
  subq.l #1,d7
  lea    _remember_JMP,a0
  move.l _remember_jmp_curpos,d0
  move.w d7,(a0,d0*2)
  addq.l #1,d0
  cmp.l  #rem_jmp_steps,d0
  bmi.s  pcode_4c_point_rem_jmp
  moveq  #0,d0
pcode_4c_point_rem_jmp:
  move.l d0,_remember_jmp_curpos
  #endif
  addq.l #cy_JmpAbs,CD
  JMP_C

opcode_6c: |/* JMP (abcd) */
 #ifdef MONITOR_BREAK
  move.l PC6502,d7 |current pointer
  sub.l  memory_pointer,d7
  subq.l #1,d7
  lea    _remember_JMP,a0
  move.l _remember_jmp_curpos,d0
  move.w d7,(a0,d0*2)
  addq.l #1,d0
  cmp.l  #rem_jmp_steps,d0
  bmi.s  pcode_6c_point_rem_jmp
  moveq  #0,d0
pcode_6c_point_rem_jmp:
  move.l d0,_remember_jmp_curpos
  #endif
  move.w (PC6502)+,d7
  LoHi d7
 #ifdef P65C02
  move.w (memory_pointer,d7.l),d7
  LoHi d7
  lea    (memory_pointer,d7.l),PC6502
 #else
  |/* original 6502 had a bug in jmp (addr) when addr crossed page boundary */
  cmp.b  #0xff,d7
  beq.s  pcode_6c_PROBLEM_FOUND |when problematic jump is found
  move.w (memory_pointer,d7.l),d7
  LoHi d7
  lea    (memory_pointer,d7.l),PC6502
  addq.l #cy_JmpInd,CD
  bra.w  NEXTCHANGE_WITHOUT
pcode_6c_PROBLEM_FOUND:
  move.l d7,d0 |we have to use both of them
  clr.b  d7 |instead of reading right this address,
            |we read address at this start of page
  move.b (memory_pointer,d7.l),d7
  LoHi d7
  move.b (memory_pointer,d0.l),d7
  lea    (memory_pointer,d7.l),PC6502
  #endif
  addq.l #cy_JmpInd,CD
  bra.w  NEXTCHANGE_WITHOUT

opcode_20: |/* JSR abcd */
  addq.l #cy_Sub,CD
  move.l PC6502,d7 |current pointer
  sub.l  memory_pointer,d7
 #ifdef MONITOR_BREAK
  subq.l #1,d7
  lea    _remember_JMP,a0
  move.l _remember_jmp_curpos,d0
  move.w d7,(a0,d0*2)
  addq.l #1,d7     | restore to PC
  addq.l #1,d0
  cmp.l  #rem_jmp_steps,d0
  bmi.s  pcode_20_point_rem_jmp
  moveq  #0,d0
pcode_20_point_rem_jmp:
  move.l d0,_remember_jmp_curpos
  addq.l #1,_ret_nesting
  #endif
  addq.l #1,d7 | return address
  moveq  #0,d0                    | PHW
  move.w regS,d0
  subq.b #1,d0     | wrong way around
  move.b d7,(memory_pointer,d0.l)
  addq.b #1,d0
  LoHi d7
  move.b d7,(memory_pointer,d0.l)
  subq.b #2,d0
  move.b d0,_regS
  JMP_C

opcode_60: |/* RTS */
  addq.l #cy_Sub,CD
  PLW d7,d0
  lea    1(memory_pointer,d7.l),PC6502
 #ifdef MONITOR_BREAK
  tst.b _break_ret
  beq.s pcode_60_mb_end
  tst.l _ret_nesting
  bmi.s pcode_60_mb_end
  move.b #1,_break_step
pcode_60_mb_end:
  subq.l #1,_ret_nesting
  #endif
  bra.w  NEXTCHANGE_WITHOUT

opcode_40: |/* RTI */
_RTI:
  addq.l #cy_Sub,CD
  moveq  #0,d0                    | PLP + PLW
  move.w regS,d0
  addq.b #1,d0
| move.b (memory_pointer,d0.l),_regP
  move.b (memory_pointer,d0.l),d7 | TEST
  ori.b  #0x30,d7                  | TEST
  move.b d7,_regP                 | TEST
  ConvertRegP_STATUS d7
  addq.b #2,d0     | wrong way around
  move.b (memory_pointer,d0.l),d7
  asl.w  #8,d7
  subq.b #1,d0
  or.b   (memory_pointer,d0.l),d7
  addq.b #1,d0
  move.b d0,_regS
  lea    (memory_pointer,d7.l),PC6502
 #ifdef MONITOR_BREAK
  tst.b _break_ret
  beq.s RTI_mb_end
  tst.l _ret_nesting
  bmi.s RTI_mb_end
  move.b #1,_break_step
RTI_mb_end:
  subq.l #1,_ret_nesting
  #endif
  tst.b  _IRQ           | CPUCHECKIRQ
  beq.w  NEXTCHANGE_WITHOUT
  move.b _regP,d7
| andi.b #I_FLAG,d7
  btst   #I_FLAGB,d7
  bne.w  NEXTCHANGE_WITHOUT
  moveq  #0,d0
  move.w regS,d0        | push PC and P to stack ( PHW + PHB ) start
  subq.b #2,d0
  andi.b #B_FLAGN,d7              | TEST
  move.b d7,(memory_pointer,d0.l) | Push P
  move.l PC6502,d7
  sub.l  memory_pointer,d7
  addq.b #1,d0          | wrong way around
  move.b d7,(memory_pointer,d0.l)
  addq.b #1,d0
  LoHi d7
  move.b d7,(memory_pointer,d0.l)
  subq.b #3,d0
  move.b d0,_regS       | push PC and P to stack ( PHW + PHB ) end
  SetI
  move.w (memory_pointer,0xfffe.l),d7
  LoHi d7
  move.l d7,PC6502
  add.l  memory_pointer,PC6502
  addq.l #7,CD
 #ifdef MONITOR_BREAK
  addq.l #1,_ret_nesting
  #endif
  bra.w  NEXTCHANGE_WITHOUT

  .macro BIT_C_CONT
  ext.w  NFLAG
  btst   #V_FLAGB,ZFLAG
  sne    VFLAG
  and.b  A,ZFLAG
  bra.w  NEXTCHANGE_WITHOUT
  .endm

opcode_24: |/* BIT ab */
  addq.l #cy_ZP,CD
  ZPAGE
BIT_RAMROM:
  move.b (memory_pointer,d7.l),ZFLAG   | GETZPBYTE
  BIT_C_CONT

opcode_2c: |/* BIT abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  bne.s  BIT_RAMROM
  EXE_GETBYTE
  move.b d0,ZFLAG
  BIT_C_CONT

  .macro STOREANYBYTE_A
  tst.b  (attrib_pointer,d7.l)
  bne.s  STA_HW
  move.b A,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT
  .endm

opcode_81: |/* STA (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  STOREANYBYTE_A

opcode_91: |/* STA (ab),y */
  addq.l #cy_IndY2,CD
  INDIRECT_Y
  STOREANYBYTE_A

opcode_8d: |/* STA abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  STOREANYBYTE_A

opcode_99: |/* STA abcd,y */
  addq.l #cy_IndY2,CD
  ABSOLUTE_Y
  STOREANYBYTE_A

opcode_9d: |/* STA abcd,x */
  addq.l #cy_AbsX2,CD
  ABSOLUTE_X
  STOREANYBYTE_A

STA_HW:
  move.b A,d0
  bra.w  A800PUTB

opcode_85: |/* STA ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b A,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_WITHOUT

opcode_95: |/* STA ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b A,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_WITHOUT

  .macro STOREANYBYTE p1
  tst.b  (attrib_pointer,d7.l)
  bne.s  TOREANYBYTE_GO_PUTBYTE\@
  move.b \p1,(memory_pointer,d7.l)
  bra.w  NEXTCHANGE_WITHOUT
TOREANYBYTE_GO_PUTBYTE\@:
  move.b \p1,d0
  bra.w  A800PUTB
  .endm

opcode_8e: |/* STX abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  STOREANYBYTE X

opcode_86: |/* STX ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b X,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_WITHOUT

opcode_96: |/* STX ab,y */
  addq.l #cy_ZPY,CD
  ZPAGE_Y
  move.b X,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_WITHOUT

opcode_8c: |/* STY abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  STOREANYBYTE Y

opcode_84: |/* STY ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b Y,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_WITHOUT

opcode_94: |/* STY ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b Y,(memory_pointer,d7.l)   | PUTZPBYTE
  bra.w  NEXTCHANGE_WITHOUT

  .macro LOADANYBYTE_A
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  LDA_HW
  move.b (memory_pointer,d7.l),A  |get byte
  NEXTCHANGE_REG A
  .endm

opcode_a1: |/* LDA (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  LOADANYBYTE_A

opcode_b1: |/* LDA (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  LOADANYBYTE_A

LDA_HW:
  EXE_GETBYTE
  move.b d0,A
  NEXTCHANGE_REG A

opcode_ad: |/* LDA abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  LOADANYBYTE_A

opcode_b9: |/* LDA abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  LOADANYBYTE_A

opcode_bd: |/* LDA abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  LOADANYBYTE_A

opcode_a5: |/* LDA ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),A  | GETZPBYTE
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_b5: |/* LDA ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),A  | GETZPBYTE
  NEXTCHANGE_REG A

opcode_a9: |/* LDA #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE A
  move.b A,ZFLAG
  bra.w  NEXTCHANGE_N

  .macro LOADANYBYTE_X
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  LDX_HW
  move.b (memory_pointer,d7.l),X  |get byte
  NEXTCHANGE_REG X
  .endm

opcode_ae: |/* LDX abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  LOADANYBYTE_X

opcode_be: |/* LDX abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  LOADANYBYTE_X

LDX_HW:
  EXE_GETBYTE
  move.b d0,X
  NEXTCHANGE_REG X

opcode_a6: |/* LDX ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),X  | GETZPBYTE
  move.b X,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_b6: |/* LDX ab,y */
  addq.l #cy_ZPY,CD
  ZPAGE_Y
  move.b (memory_pointer,d7.l),X  | GETZPBYTE
  NEXTCHANGE_REG X

opcode_a2: |/* LDX #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE X
  move.b X,ZFLAG
  bra.w  NEXTCHANGE_N

  .macro LOADANYBYTE_Y
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  LDY_HW
  move.b (memory_pointer,d7.l),Y  |get byte
  NEXTCHANGE_REG Y
  .endm

opcode_ac: |/* LDY abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  LOADANYBYTE_Y

opcode_bc: |/* LDY abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  LOADANYBYTE_Y

LDY_HW:
  EXE_GETBYTE
  move.b d0,Y
  NEXTCHANGE_REG Y

opcode_a4: |/* LDY ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),Y  | GETZPBYTE
  move.b Y,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_b4: |/* LDY ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),Y       | GETZPBYTE
  NEXTCHANGE_REG Y

opcode_a0: |/* LDY #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE Y
  move.b Y,ZFLAG
  bra.w  NEXTCHANGE_N

opcode_8a: |/* TXA */
  addq.l #cy_RegChg,CD
  move.b X,A
  NEXTCHANGE_REG A

opcode_aa: |/* TAX */
  addq.l #cy_RegChg,CD
  move.b A,X
  NEXTCHANGE_REG A

opcode_98: |/* TYA */
  addq.l #cy_RegChg,CD
  move.b Y,A
  NEXTCHANGE_REG A

opcode_a8: |/* TAY */
  addq.l #cy_RegChg,CD
  move.b A,Y
  NEXTCHANGE_REG A

opcode_9a: |/* TXS */
  addq.l #cy_RegChg,CD
  move.b X,_regS
  bra.w  NEXTCHANGE_WITHOUT

opcode_ba: |/* TSX */
  addq.l #cy_RegChg,CD
  move.b _regS,X
  NEXTCHANGE_REG X

opcode_d2: |/* ESCRTS #ab (JAM) - on Atari is here instruction CIM
           |[unofficial] !RS! */
  addq.l #cy_CIM,CD
  move.b (PC6502)+,d7
  Call_Atari800_RunEsc
  PLW d7,d0
  lea (memory_pointer,d7.l),PC6502
  addq.l #1,PC6502
 #ifdef MONITOR_BREAK
  tst.b _break_ret
  beq.s pcode_d2_mb_end
  tst.l _ret_nesting
  bmi.s pcode_d2_mb_end
  move.b #1,_break_step
pcode_d2_mb_end:
  subq.l #1,_ret_nesting
  #endif
  bra.w  NEXTCHANGE_WITHOUT

opcode_f2: |/* ESC #ab (JAM) - on Atari is here instruction CIM
           |[unofficial] !RS! */
  addq.l #cy_CIM,CD
  move.b (PC6502)+,d7
  Call_Atari800_RunEsc
  bra.w  NEXTCHANGE_WITHOUT

opcode_ea: |/* NOP */ ;official
  addq.l #cy_NOP,CD
  bra.w  NEXTCHANGE_WITHOUT

opcode_c6: |/* DEC ab */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  subq.b #1,(memory_pointer,d7.l)
  move.b (memory_pointer,d7.l),ZFLAG
  bra.w  NEXTCHANGE_N

opcode_d6: |/* DEC ab,x */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  subq.b #1,(memory_pointer,d7.l)
  move.b (memory_pointer,d7.l),ZFLAG
  bra.w  NEXTCHANGE_N

opcode_ce: |/* DEC abcd */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  move.b (attrib_pointer,d7.l),d0
  bne.s  DEC_Byte_ROMHW
  subq.b #1,(memory_pointer,d7.l)
  move.b (memory_pointer,d7.l),ZFLAG
  bra.w  NEXTCHANGE_N
DEC_Byte_ROMHW:
  cmp.b  #isHARDWARE,d0
  beq.s  EC_Byte_ROMHW_Getbyte_HW
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  subq.b #1,ZFLAG
  bra.w  NEXTCHANGE_N
EC_Byte_ROMHW_Getbyte_HW:
  RMW_GETBYTE
  move.b d0,ZFLAG
  subq.b #1,ZFLAG
| bra.w  A800PUTB_Ld0_N
A800PUTB_Ld0_N:
  ext.w  NFLAG
A800PUTB_Ld0:
  move.b ZFLAG,d0
A800PUTB:
  cmp.b  #isROM,(attrib_pointer,d7.l)
  beq.s  A800PUTBE
  move.l ZFLAG,-(a7)
  EXE_PUTBYTE d7
  move.l (a7)+,ZFLAG
A800PUTBE:
  bra.w  NEXTCHANGE_WITHOUT

opcode_de: |/* DEC abcd,x */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  move.b (attrib_pointer,d7.l),d0
  bne.s  DEC_Byte_ROMHW
  subq.b #1,(memory_pointer,d7.l)
  move.b (memory_pointer,d7.l),ZFLAG
  bra.w  NEXTCHANGE_N

opcode_ca: |/* DEX */
  addq.l #cy_RegChg,CD
  subq.b #1,X
  NEXTCHANGE_REG X

opcode_88: |/* DEY */
  addq.l #cy_RegChg,CD
  subq.b #1,Y
  NEXTCHANGE_REG Y

opcode_e6: |/* INC ab */
  addq.l #cy_ZP_RW,CD
  ZPAGE
  addq.b #1,(memory_pointer,d7.l)
  move.b (memory_pointer,d7.l),ZFLAG
  bra.w  NEXTCHANGE_N

opcode_f6: |/* INC ab,x */
  addq.l #cy_ZPX_RW,CD
  ZPAGE_X
  addq.b #1,(memory_pointer,d7.l)
  move.b (memory_pointer,d7.l),ZFLAG
  bra.w  NEXTCHANGE_N

opcode_ee: |/* INC abcd */
  addq.l #cy_Abs_RW,CD
  ABSOLUTE
  move.b (attrib_pointer,d7.l),d0
  bne.s  INC_Byte_ROMHW
  addq.b #1,(memory_pointer,d7.l)
  move.b (memory_pointer,d7.l),ZFLAG
  bra.w  NEXTCHANGE_N
INC_Byte_ROMHW:
  cmp.b  #isHARDWARE,d0
  beq.s  NC_Byte_ROMHW_Getbyte_HW
  move.b (memory_pointer,d7.l),ZFLAG |get byte
  addq.b #1,ZFLAG
  bra.w  NEXTCHANGE_N
NC_Byte_ROMHW_Getbyte_HW:
  RMW_GETBYTE
  move.b d0,ZFLAG
  addq.b #1,ZFLAG
  bra.w  A800PUTB_Ld0_N

opcode_fe: |/* INC abcd,x */
  addq.l #cy_AbsX_RW,CD
  ABSOLUTE_X
  move.b (attrib_pointer,d7.l),d0
  bne.s  INC_Byte_ROMHW
  addq.b #1,(memory_pointer,d7.l)
  move.b (memory_pointer,d7.l),ZFLAG
  bra.w  NEXTCHANGE_N

opcode_e8: |/* INX */
  addq.l #cy_RegChg,CD
  addq.b #1,X
  NEXTCHANGE_REG X

opcode_c8: |/* INY */
  addq.l #cy_RegChg,CD
  addq.b #1,Y
  NEXTCHANGE_REG Y

  .macro DONT_BRA
  addq.l #cy_Bcc,CD
  addq.l #1,PC6502
  bra.w  NEXTCHANGE_WITHOUT
  .endm

opcode_10: |/* BPL */
  tst.w  NFLAG
  bpl.s  SOLVE
  DONT_BRA

opcode_30: |/* BMI */
  tst.w  NFLAG
  bmi.s  SOLVE
  DONT_BRA

opcode_d0: |/* BNE */
  tst.b ZFLAG
  bne.s SOLVE
  DONT_BRA

opcode_f0: |/* BEQ */
  tst.b ZFLAG
  beq.s SOLVE
  DONT_BRA

SOLVE:
  move.b (PC6502)+,d7
  extb.l d7
  move.l PC6502,d0
  add.l  d7,PC6502
  sub.l  memory_pointer,d0
  and.w  #255,d0                  | !!!
  add.w  d7,d0
  and.w  #0xff00,d0
  bne.s  SOLVE_PB
  addq.l #cy_Bcc1,CD
  bra.w  NEXTCHANGE_WITHOUT
SOLVE_PB:
  addq.l #cy_Bcc2,CD
  bra.w  NEXTCHANGE_WITHOUT

opcode_90: |/* BCC */
  tst.b  CFLAG
  beq.s  SOLVE
  DONT_BRA

opcode_b0: |/* BCS */
  tst.b  CFLAG
  bne.s  SOLVE
  DONT_BRA

opcode_50: |/* BVC */
  tst.b  VFLAG
  beq.s  SOLVE
  DONT_BRA

opcode_70: |/* BVS */
  tst.b  VFLAG
  bne.s  SOLVE
  DONT_BRA

  .macro GETANYBYTE_ADC
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  ADC_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  bra.s  adcb
  .endm

adc:
  btst   #D_FLAGB,_regP
  bne.w  BCD_ADC
  bra.w  adcb

opcode_61: |/* ADC (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  GETANYBYTE_ADC

opcode_71: |/* ADC (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  GETANYBYTE_ADC

ADC_HW:
  EXE_GETBYTE
  bra.s  adcb

opcode_6d: |/* ADC abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  GETANYBYTE_ADC

opcode_79: |/* ADC abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  GETANYBYTE_ADC

opcode_7d: |/* ADC abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  GETANYBYTE_ADC

adcb:
  add.b  CFLAG,CFLAG
  addx.b d0,A
  svs    VFLAG
  scs    CFLAG
  NEXTCHANGE_REG A

opcode_65: |/* ADC ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  adcb

opcode_75: |/* ADC ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  adcb

opcode_69: |/* ADC #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
  bra.s  adcb

  .macro GETANYBYTE_ADC_D
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  ADC_HW_D
  move.b (memory_pointer,d7.l),d0 |get byte
  bra.s  BCD_ADC
  .endm

opcode_61_D: |/* ADC (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  GETANYBYTE_ADC_D

opcode_6d_D: |/* ADC abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  GETANYBYTE_ADC_D

opcode_71_D: |/* ADC (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  GETANYBYTE_ADC_D

ADC_HW_D:
  EXE_GETBYTE
  bra.s  BCD_ADC

opcode_79_D: |/* ADC abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  GETANYBYTE_ADC_D

opcode_7d_D: |/* ADC abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  GETANYBYTE_ADC_D

| Version 1 : exact like Thor
|   Z from binary calc.
|   N + V after lower nibble decimal correction
|   C from decimal calc.
| a lot of code necessary to replicate a 6502 bug
BCD_ADC:
  move.w d0,a0     | needed first
  moveq  #15,d7
  and.b  d7,d0     | low nibble Add
  move.b A,ZFLAG
  and.b  d7,ZFLAG  | low nibble A
  add.b  CFLAG,CFLAG
  abcd   d0,ZFLAG  | low nibble BCD add
  move.b A,d0
  move.b #0xf0,d7  /// PS! bylo moveq, ale pry operans mismatch
  and.b  d7,d0     | high nibble Add
  add.b  d0,ZFLAG
  move.w a0,d0
  and.b  d7,d0     | high nibble Add
  add.b  d0,ZFLAG
  ext.w  NFLAG     | NFLAG finished
  eor.b  A,d0      | A eor data
  eor.b  A,ZFLAG   | A eor temp
  not.b  ZFLAG
  or.b   d0,ZFLAG
  smi    VFLAG     | VFLAG finished
  move.w a0,d0     | restore data
  add.b  CFLAG,CFLAG
  move.b A,ZFLAG
  addx.b d0,ZFLAG  | ZFLAG finished
  add.b  CFLAG,CFLAG
  abcd   d0,A      | A finished
  scs    CFLAG
  bra.w  NEXTCHANGE_WITHOUT

opcode_65_D: |/* ADC ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  BCD_ADC

opcode_75_D: |/* ADC ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  BCD_ADC

opcode_69_D: |/* ADC #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
  bra.s  BCD_ADC

  .macro GETANYBYTE_SBC
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  SBC_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  bra.s  sbcb
  .endm

sbc:
  btst   #D_FLAGB,_regP
  bne.w  BCD_SBC
  bra.w  sbcb

opcode_e1: |/* SBC (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  GETANYBYTE_SBC

opcode_f1: |/* SBC (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  GETANYBYTE_SBC

SBC_HW:
  EXE_GETBYTE
  bra.s  sbcb

opcode_ed: |/* SBC abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  GETANYBYTE_SBC

opcode_f9: |/* SBC abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  GETANYBYTE_SBC

opcode_fd: |/* SBC abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  GETANYBYTE_SBC

sbcb:
  subq.b #1,CFLAG
  subx.b d0,A
  svs    VFLAG
  scc    CFLAG
  NEXTCHANGE_REG A

opcode_e5: |/* SBC ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  sbcb

opcode_f5: |/* SBC ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  sbcb

opcode_eb: |/* SBC #ab [unofficial] */
opcode_e9: |/* SBC #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
  bra.s  sbcb

  .macro GETANYBYTE_SBC_D
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  SBC_HW_D
  move.b (memory_pointer,d7.l),d0 |get byte
  bra.s  BCD_SBC
  .endm

opcode_e1_D: |/* SBC (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  GETANYBYTE_SBC_D

opcode_ed_D: |/* SBC abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  GETANYBYTE_SBC_D

opcode_f1_D: |/* SBC (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  GETANYBYTE_SBC_D

SBC_HW_D:
  EXE_GETBYTE
  bra.s  BCD_SBC

opcode_f9_D: |/* SBC abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  GETANYBYTE_SBC_D

opcode_fd_D: |/* SBC abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  GETANYBYTE_SBC_D

| Version exact like Thor
|   C, Z, N, V from binary calc.
|   A from decimal calc.
BCD_SBC:
  move.b A,ZFLAG
  not.b  CFLAG
  add.b  CFLAG,CFLAG
  sbcd   d0,A
  add.b  CFLAG,CFLAG
  subx.b d0,ZFLAG
  svs    VFLAG
  scc    CFLAG
  bra.w  NEXTCHANGE_N

opcode_e5_D: |/* SBC ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  BCD_SBC

opcode_f5_D: |/* SBC ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  BCD_SBC

opcode_eb_D: |/* SBC #ab [unofficial] */
opcode_e9_D: |/* SBC #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
  bra.s  BCD_SBC

opcode_cc: |/* CPY abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l) | GETANYBYTE
  beq.s  pcode_cc_Getbyte_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  move.b Y,ZFLAG
  bra COMPARE
pcode_cc_Getbyte_HW:
  EXE_GETBYTE
  move.b Y,ZFLAG
  bra COMPARE

opcode_c4: |/* CPY ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  move.b Y,ZFLAG
  bra COMPARE

opcode_c0: |/* CPY #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
  move.b Y,ZFLAG
  bra COMPARE

opcode_ec: |/* CPX abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l) | GETANYBYTE
  beq.s  pcode_ec_Getbyte_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  move.b X,ZFLAG
  bra COMPARE
pcode_ec_Getbyte_HW:
  EXE_GETBYTE
  move.b X,ZFLAG
  bra COMPARE

opcode_e4: |/* CPX ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  move.b X,ZFLAG
  bra COMPARE

opcode_e0: |/* CPX #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
  move.b X,ZFLAG
  bra COMPARE

  .macro GETANYBYTE_CMP
  cmp.b  #isHARDWARE,(attrib_pointer,d7.l)
  beq.s  CMP_HW
  move.b (memory_pointer,d7.l),d0 |get byte
  bra    COMPARE_A
  .endm

opcode_c1: |/* CMP (ab,x) */
  addq.l #cy_IndX,CD
  INDIRECT_X
  GETANYBYTE_CMP

opcode_d1: |/* CMP (ab),y */
  addq.l #cy_IndY,CD
  INDIRECT_Y_NCY
  GETANYBYTE_CMP

CMP_HW:
  EXE_GETBYTE
  bra.w  COMPARE_A

opcode_cd: |/* CMP abcd */
  addq.l #cy_Abs,CD
  ABSOLUTE
  GETANYBYTE_CMP

opcode_d9: |/* CMP abcd,y */
  addq.l #cy_AbsY,CD
  ABSOLUTE_Y_NCY
  GETANYBYTE_CMP

opcode_dd: |/* CMP abcd,x */
  addq.l #cy_AbsX,CD
  ABSOLUTE_X_NCY
  GETANYBYTE_CMP

opcode_d5: |/* CMP ab,x */
  addq.l #cy_ZPX,CD
  ZPAGE_X
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  COMPARE_A

opcode_c5: |/* CMP ab */
  addq.l #cy_ZP,CD
  ZPAGE
  move.b (memory_pointer,d7.l),d0      | GETZPBYTE
  bra.s  COMPARE_A

opcode_c9: |/* CMP #ab */
  addq.l #cy_Imm,CD
  IMMEDIATE d0
| bra.s  COMPARE_A

COMPARE_A:
  move.b A,ZFLAG
COMPARE:
  sub.b  d0,ZFLAG
  scc    CFLAG
| bra.w  NEXTCHANGE_N

|MAIN LOOP , where we are counting cycles and working with other STUFF

NEXTCHANGE_N:
  ext.w  NFLAG
NEXTCHANGE_WITHOUT:
  cmp.l _xpos_limit,CD
  bge.s END_OF_CYCLE
****************************************
 #ifdef MONITOR_BREAK
| following block of code allows you to enter a break address
  move.l _remember_PC_curpos,d0
  lea    _remember_PC,a0
  move.l PC6502,d7
  sub.l  memory_pointer,d7
  move.w d7,(a0,d0*2) | remember program counter
  addq.l #1,d0
  cmp.l  #rem_pc_steps,d0
  bmi.s  EXTCHANGE_WITHOUT_point_rem_pc
  moveq  #0,d0
EXTCHANGE_WITHOUT_point_rem_pc:
  move.l d0,_remember_PC_curpos
 #ifdef NEW_CYCLE_EXACT
  moveq   #0,d0
  move.b  _ypos,d0
  asl.w   #8,d0
  move.l  d0,a1
  move.l  CD,d0
  cmp.l   #-999,_cur_screen_pos
  beq.s   EXTCHANGE_WITHOUT_calc_all
  move.l  _cpu2antic_ptr,a0
  move.l  (a0,d0*4),d0
  bra.s   EXTCHANGE_WITHOUT_calc_all
EXTCHANGE_WITHOUT_calc_all:
  add.l   a1,d0
  move.l  d0,a1
  move.l  _remember_xpos,a0
  move.l  _remember_xpos_curpos,d0
  move.l  a1,(a0,d0*4)
  addq.l #1,d0
  cmp.l  #rem_pc_steps,d0
  bmi.s  EXTCHANGE_WITHOUT_point_rem_xpos
  moveq  #0,d0
EXTCHANGE_WITHOUT_point_rem_xpos:
  move.l d0,_remember_xpos_curpos
  #endif
  cmp.w  _break_addr,d7 | break address reached ?
  beq.s  EXTCHANGE_WITHOUT_go_monitor
  move.l _ypos,d0    | !!! or EXTCHANGE_WITHOUT_w ?
  cmp.l  _ypos_break_addr,d0 | break address reached ?
  beq.s  EXTCHANGE_WITHOUT_go_monitor
  tst.b  _break_step | step mode active ?
  beq.s  EXTCHANGE_WITHOUT_get_first
EXTCHANGE_WITHOUT_go_monitor:
  bsr    go_monitor  |on break monitor is invoked
EXTCHANGE_WITHOUT_get_first:
  #endif
****************************************
  moveq  #0,d7
  move.b (PC6502)+,d7
 #ifdef PROFILE
  lea    _instruction_count,a0
  addq.l #1,(a0,d7.l*4)
  #endif
| move.w (OPMODE_TABLE,PC,d7.l*2),d0
| jmp    (OPMODE_TABLE,d0.w)
  move.w (a3,d7.l*2),d0
  jmp    (a3,d0.w)

END_OF_CYCLE:
  ConvertSTATUS_RegP_destroy d0
  UPDATE_GLOBAL_REGS
  move.l CD,_xpos |returned value
  movem.l (a7)+,d2-d7/a2-a6
TERM_GO:
  rts

go_monitor:
  ConvertSTATUS_RegP_destroy d0
  UPDATE_GLOBAL_REGS
  Call_Atari800_Exit_true
  UPDATE_LOCAL_REGS
  ConvertRegP_STATUS d0
  rts

  .align	4         | doubleword alignment

OPMODE_TABLE:
OP_T:
  .word opcode_00-OP_T
  .word opcode_01-OP_T
  .word opcode_02-OP_T
  .word opcode_03-OP_T
  .word opcode_04-OP_T
  .word opcode_05-OP_T
  .word opcode_06-OP_T
  .word opcode_07-OP_T
  .word opcode_08-OP_T
  .word opcode_09-OP_T
  .word opcode_0a-OP_T
  .word opcode_0b-OP_T
  .word opcode_0c-OP_T
  .word opcode_0d-OP_T
  .word opcode_0e-OP_T
  .word opcode_0f-OP_T
  .word opcode_10-OP_T
  .word opcode_11-OP_T
  .word opcode_12-OP_T
  .word opcode_13-OP_T
  .word opcode_14-OP_T
  .word opcode_15-OP_T
  .word opcode_16-OP_T
  .word opcode_17-OP_T
  .word opcode_18-OP_T
  .word opcode_19-OP_T
  .word opcode_1a-OP_T
  .word opcode_1b-OP_T
  .word opcode_1c-OP_T
  .word opcode_1d-OP_T
  .word opcode_1e-OP_T
  .word opcode_1f-OP_T
  .word opcode_20-OP_T
  .word opcode_21-OP_T
  .word opcode_22-OP_T
  .word opcode_23-OP_T
  .word opcode_24-OP_T
  .word opcode_25-OP_T
  .word opcode_26-OP_T
  .word opcode_27-OP_T
  .word opcode_28-OP_T
  .word opcode_29-OP_T
  .word opcode_2a-OP_T
  .word opcode_2b-OP_T
  .word opcode_2c-OP_T
  .word opcode_2d-OP_T
  .word opcode_2e-OP_T
  .word opcode_2f-OP_T
  .word opcode_30-OP_T
  .word opcode_31-OP_T
  .word opcode_32-OP_T
  .word opcode_33-OP_T
  .word opcode_34-OP_T
  .word opcode_35-OP_T
  .word opcode_36-OP_T
  .word opcode_37-OP_T
  .word opcode_38-OP_T
  .word opcode_39-OP_T
  .word opcode_3a-OP_T
  .word opcode_3b-OP_T
  .word opcode_3c-OP_T
  .word opcode_3d-OP_T
  .word opcode_3e-OP_T
  .word opcode_3f-OP_T
  .word opcode_40-OP_T
  .word opcode_41-OP_T
  .word opcode_42-OP_T
  .word opcode_43-OP_T
  .word opcode_44-OP_T
  .word opcode_45-OP_T
  .word opcode_46-OP_T
  .word opcode_47-OP_T
  .word opcode_48-OP_T
  .word opcode_49-OP_T
  .word opcode_4a-OP_T
  .word opcode_4b-OP_T
  .word opcode_4c-OP_T
  .word opcode_4d-OP_T
  .word opcode_4e-OP_T
  .word opcode_4f-OP_T
  .word opcode_50-OP_T
  .word opcode_51-OP_T
  .word opcode_52-OP_T
  .word opcode_53-OP_T
  .word opcode_54-OP_T
  .word opcode_55-OP_T
  .word opcode_56-OP_T
  .word opcode_57-OP_T
  .word opcode_58-OP_T
  .word opcode_59-OP_T
  .word opcode_5a-OP_T
  .word opcode_5b-OP_T
  .word opcode_5c-OP_T
  .word opcode_5d-OP_T
  .word opcode_5e-OP_T
  .word opcode_5f-OP_T
  .word opcode_60-OP_T
  .word opcode_61-OP_T
  .word opcode_62-OP_T
  .word opcode_63-OP_T
  .word opcode_64-OP_T
  .word opcode_65-OP_T
  .word opcode_66-OP_T
  .word opcode_67-OP_T
  .word opcode_68-OP_T
  .word opcode_69-OP_T
  .word opcode_6a-OP_T
  .word opcode_6b-OP_T
  .word opcode_6c-OP_T
  .word opcode_6d-OP_T
  .word opcode_6e-OP_T
  .word opcode_6f-OP_T
  .word opcode_70-OP_T
  .word opcode_71-OP_T
  .word opcode_72-OP_T
  .word opcode_73-OP_T
  .word opcode_74-OP_T
  .word opcode_75-OP_T
  .word opcode_76-OP_T
  .word opcode_77-OP_T
  .word opcode_78-OP_T
  .word opcode_79-OP_T
  .word opcode_7a-OP_T
  .word opcode_7b-OP_T
  .word opcode_7c-OP_T
  .word opcode_7d-OP_T
  .word opcode_7e-OP_T
  .word opcode_7f-OP_T
  .word opcode_80-OP_T
  .word opcode_81-OP_T
  .word opcode_82-OP_T
  .word opcode_83-OP_T
  .word opcode_84-OP_T
  .word opcode_85-OP_T
  .word opcode_86-OP_T
  .word opcode_87-OP_T
  .word opcode_88-OP_T
  .word opcode_89-OP_T
  .word opcode_8a-OP_T
  .word opcode_8b-OP_T
  .word opcode_8c-OP_T
  .word opcode_8d-OP_T
  .word opcode_8e-OP_T
  .word opcode_8f-OP_T
  .word opcode_90-OP_T
  .word opcode_91-OP_T
  .word opcode_92-OP_T
  .word opcode_93-OP_T
  .word opcode_94-OP_T
  .word opcode_95-OP_T
  .word opcode_96-OP_T
  .word opcode_97-OP_T
  .word opcode_98-OP_T
  .word opcode_99-OP_T
  .word opcode_9a-OP_T
  .word opcode_9b-OP_T
  .word opcode_9c-OP_T
  .word opcode_9d-OP_T
  .word opcode_9e-OP_T
  .word opcode_9f-OP_T
  .word opcode_a0-OP_T
  .word opcode_a1-OP_T
  .word opcode_a2-OP_T
  .word opcode_a3-OP_T
  .word opcode_a4-OP_T
  .word opcode_a5-OP_T
  .word opcode_a6-OP_T
  .word opcode_a7-OP_T
  .word opcode_a8-OP_T
  .word opcode_a9-OP_T
  .word opcode_aa-OP_T
  .word opcode_ab-OP_T
  .word opcode_ac-OP_T
  .word opcode_ad-OP_T
  .word opcode_ae-OP_T
  .word opcode_af-OP_T
  .word opcode_b0-OP_T
  .word opcode_b1-OP_T
  .word opcode_b2-OP_T
  .word opcode_b3-OP_T
  .word opcode_b4-OP_T
  .word opcode_b5-OP_T
  .word opcode_b6-OP_T
  .word opcode_b7-OP_T
  .word opcode_b8-OP_T
  .word opcode_b9-OP_T
  .word opcode_ba-OP_T
  .word opcode_bb-OP_T
  .word opcode_bc-OP_T
  .word opcode_bd-OP_T
  .word opcode_be-OP_T
  .word opcode_bf-OP_T
  .word opcode_c0-OP_T
  .word opcode_c1-OP_T
  .word opcode_c2-OP_T
  .word opcode_c3-OP_T
  .word opcode_c4-OP_T
  .word opcode_c5-OP_T
  .word opcode_c6-OP_T
  .word opcode_c7-OP_T
  .word opcode_c8-OP_T
  .word opcode_c9-OP_T
  .word opcode_ca-OP_T
  .word opcode_cb-OP_T
  .word opcode_cc-OP_T
  .word opcode_cd-OP_T
  .word opcode_ce-OP_T
  .word opcode_cf-OP_T
  .word opcode_d0-OP_T
  .word opcode_d1-OP_T
  .word opcode_d2-OP_T
  .word opcode_d3-OP_T
  .word opcode_d4-OP_T
  .word opcode_d5-OP_T
  .word opcode_d6-OP_T
  .word opcode_d7-OP_T
  .word opcode_d8-OP_T
  .word opcode_d9-OP_T
  .word opcode_da-OP_T
  .word opcode_db-OP_T
  .word opcode_dc-OP_T
  .word opcode_dd-OP_T
  .word opcode_de-OP_T
  .word opcode_df-OP_T
  .word opcode_e0-OP_T
  .word opcode_e1-OP_T
  .word opcode_e2-OP_T
  .word opcode_e3-OP_T
  .word opcode_e4-OP_T
  .word opcode_e5-OP_T
  .word opcode_e6-OP_T
  .word opcode_e7-OP_T
  .word opcode_e8-OP_T
  .word opcode_e9-OP_T
  .word opcode_ea-OP_T
  .word opcode_eb-OP_T
  .word opcode_ec-OP_T
  .word opcode_ed-OP_T
  .word opcode_ee-OP_T
  .word opcode_ef-OP_T
  .word opcode_f0-OP_T
  .word opcode_f1-OP_T
  .word opcode_f2-OP_T
  .word opcode_f3-OP_T
  .word opcode_f4-OP_T
  .word opcode_f5-OP_T
  .word opcode_f6-OP_T
  .word opcode_f7-OP_T
  .word opcode_f8-OP_T
  .word opcode_f9-OP_T
  .word opcode_fa-OP_T
  .word opcode_fb-OP_T
  .word opcode_fc-OP_T
  .word opcode_fd-OP_T
  .word opcode_fe-OP_T
  .word opcode_ff-OP_T

OPMODE_TABLE_D:
OP_T_D:
  .word opcode_00-OP_T_D
  .word opcode_01-OP_T_D
  .word opcode_02-OP_T_D
  .word opcode_03-OP_T_D
  .word opcode_04-OP_T_D
  .word opcode_05-OP_T_D
  .word opcode_06-OP_T_D
  .word opcode_07-OP_T_D
  .word opcode_08-OP_T_D
  .word opcode_09-OP_T_D
  .word opcode_0a-OP_T_D
  .word opcode_0b-OP_T_D
  .word opcode_0c-OP_T_D
  .word opcode_0d-OP_T_D
  .word opcode_0e-OP_T_D
  .word opcode_0f-OP_T_D
  .word opcode_10-OP_T_D
  .word opcode_11-OP_T_D
  .word opcode_12-OP_T_D
  .word opcode_13-OP_T_D
  .word opcode_14-OP_T_D
  .word opcode_15-OP_T_D
  .word opcode_16-OP_T_D
  .word opcode_17-OP_T_D
  .word opcode_18-OP_T_D
  .word opcode_19-OP_T_D
  .word opcode_1a-OP_T_D
  .word opcode_1b-OP_T_D
  .word opcode_1c-OP_T_D
  .word opcode_1d-OP_T_D
  .word opcode_1e-OP_T_D
  .word opcode_1f-OP_T_D
  .word opcode_20-OP_T_D
  .word opcode_21-OP_T_D
  .word opcode_22-OP_T_D
  .word opcode_23-OP_T_D
  .word opcode_24-OP_T_D
  .word opcode_25-OP_T_D
  .word opcode_26-OP_T_D
  .word opcode_27-OP_T_D
  .word opcode_28-OP_T_D
  .word opcode_29-OP_T_D
  .word opcode_2a-OP_T_D
  .word opcode_2b-OP_T_D
  .word opcode_2c-OP_T_D
  .word opcode_2d-OP_T_D
  .word opcode_2e-OP_T_D
  .word opcode_2f-OP_T_D
  .word opcode_30-OP_T_D
  .word opcode_31-OP_T_D
  .word opcode_32-OP_T_D
  .word opcode_33-OP_T_D
  .word opcode_34-OP_T_D
  .word opcode_35-OP_T_D
  .word opcode_36-OP_T_D
  .word opcode_37-OP_T_D
  .word opcode_38-OP_T_D
  .word opcode_39-OP_T_D
  .word opcode_3a-OP_T_D
  .word opcode_3b-OP_T_D
  .word opcode_3c-OP_T_D
  .word opcode_3d-OP_T_D
  .word opcode_3e-OP_T_D
  .word opcode_3f-OP_T_D
  .word opcode_40-OP_T_D
  .word opcode_41-OP_T_D
  .word opcode_42-OP_T_D
  .word opcode_43-OP_T_D
  .word opcode_44-OP_T_D
  .word opcode_45-OP_T_D
  .word opcode_46-OP_T_D
  .word opcode_47-OP_T_D
  .word opcode_48-OP_T_D
  .word opcode_49-OP_T_D
  .word opcode_4a-OP_T_D
  .word opcode_4b-OP_T_D
  .word opcode_4c-OP_T_D
  .word opcode_4d-OP_T_D
  .word opcode_4e-OP_T_D
  .word opcode_4f-OP_T_D
  .word opcode_50-OP_T_D
  .word opcode_51-OP_T_D
  .word opcode_52-OP_T_D
  .word opcode_53-OP_T_D
  .word opcode_54-OP_T_D
  .word opcode_55-OP_T_D
  .word opcode_56-OP_T_D
  .word opcode_57-OP_T_D
  .word opcode_58-OP_T_D
  .word opcode_59-OP_T_D
  .word opcode_5a-OP_T_D
  .word opcode_5b-OP_T_D
  .word opcode_5c-OP_T_D
  .word opcode_5d-OP_T_D
  .word opcode_5e-OP_T_D
  .word opcode_5f-OP_T_D
  .word opcode_60-OP_T_D
  .word opcode_61_D-OP_T_D
  .word opcode_62-OP_T_D
  .word opcode_63-OP_T_D
  .word opcode_64-OP_T_D
  .word opcode_65_D-OP_T_D
  .word opcode_66-OP_T_D
  .word opcode_67-OP_T_D
  .word opcode_68-OP_T_D
  .word opcode_69_D-OP_T_D
  .word opcode_6a-OP_T_D
  .word opcode_6b-OP_T_D
  .word opcode_6c-OP_T_D
  .word opcode_6d_D-OP_T_D
  .word opcode_6e-OP_T_D
  .word opcode_6f-OP_T_D
  .word opcode_70-OP_T_D
  .word opcode_71_D-OP_T_D
  .word opcode_72-OP_T_D
  .word opcode_73-OP_T_D
  .word opcode_74-OP_T_D
  .word opcode_75_D-OP_T_D
  .word opcode_76-OP_T_D
  .word opcode_77-OP_T_D
  .word opcode_78-OP_T_D
  .word opcode_79_D-OP_T_D
  .word opcode_7a-OP_T_D
  .word opcode_7b-OP_T_D
  .word opcode_7c-OP_T_D
  .word opcode_7d_D-OP_T_D
  .word opcode_7e-OP_T_D
  .word opcode_7f-OP_T_D
  .word opcode_80-OP_T_D
  .word opcode_81-OP_T_D
  .word opcode_82-OP_T_D
  .word opcode_83-OP_T_D
  .word opcode_84-OP_T_D
  .word opcode_85-OP_T_D
  .word opcode_86-OP_T_D
  .word opcode_87-OP_T_D
  .word opcode_88-OP_T_D
  .word opcode_89-OP_T_D
  .word opcode_8a-OP_T_D
  .word opcode_8b-OP_T_D
  .word opcode_8c-OP_T_D
  .word opcode_8d-OP_T_D
  .word opcode_8e-OP_T_D
  .word opcode_8f-OP_T_D
  .word opcode_90-OP_T_D
  .word opcode_91-OP_T_D
  .word opcode_92-OP_T_D
  .word opcode_93-OP_T_D
  .word opcode_94-OP_T_D
  .word opcode_95-OP_T_D
  .word opcode_96-OP_T_D
  .word opcode_97-OP_T_D
  .word opcode_98-OP_T_D
  .word opcode_99-OP_T_D
  .word opcode_9a-OP_T_D
  .word opcode_9b-OP_T_D
  .word opcode_9c-OP_T_D
  .word opcode_9d-OP_T_D
  .word opcode_9e-OP_T_D
  .word opcode_9f-OP_T_D
  .word opcode_a0-OP_T_D
  .word opcode_a1-OP_T_D
  .word opcode_a2-OP_T_D
  .word opcode_a3-OP_T_D
  .word opcode_a4-OP_T_D
  .word opcode_a5-OP_T_D
  .word opcode_a6-OP_T_D
  .word opcode_a7-OP_T_D
  .word opcode_a8-OP_T_D
  .word opcode_a9-OP_T_D
  .word opcode_aa-OP_T_D
  .word opcode_ab-OP_T_D
  .word opcode_ac-OP_T_D
  .word opcode_ad-OP_T_D
  .word opcode_ae-OP_T_D
  .word opcode_af-OP_T_D
  .word opcode_b0-OP_T_D
  .word opcode_b1-OP_T_D
  .word opcode_b2-OP_T_D
  .word opcode_b3-OP_T_D
  .word opcode_b4-OP_T_D
  .word opcode_b5-OP_T_D
  .word opcode_b6-OP_T_D
  .word opcode_b7-OP_T_D
  .word opcode_b8-OP_T_D
  .word opcode_b9-OP_T_D
  .word opcode_ba-OP_T_D
  .word opcode_bb-OP_T_D
  .word opcode_bc-OP_T_D
  .word opcode_bd-OP_T_D
  .word opcode_be-OP_T_D
  .word opcode_bf-OP_T_D
  .word opcode_c0-OP_T_D
  .word opcode_c1-OP_T_D
  .word opcode_c2-OP_T_D
  .word opcode_c3-OP_T_D
  .word opcode_c4-OP_T_D
  .word opcode_c5-OP_T_D
  .word opcode_c6-OP_T_D
  .word opcode_c7-OP_T_D
  .word opcode_c8-OP_T_D
  .word opcode_c9-OP_T_D
  .word opcode_ca-OP_T_D
  .word opcode_cb-OP_T_D
  .word opcode_cc-OP_T_D
  .word opcode_cd-OP_T_D
  .word opcode_ce-OP_T_D
  .word opcode_cf-OP_T_D
  .word opcode_d0-OP_T_D
  .word opcode_d1-OP_T_D
  .word opcode_d2-OP_T_D
  .word opcode_d3-OP_T_D
  .word opcode_d4-OP_T_D
  .word opcode_d5-OP_T_D
  .word opcode_d6-OP_T_D
  .word opcode_d7-OP_T_D
  .word opcode_d8-OP_T_D
  .word opcode_d9-OP_T_D
  .word opcode_da-OP_T_D
  .word opcode_db-OP_T_D
  .word opcode_dc-OP_T_D
  .word opcode_dd-OP_T_D
  .word opcode_de-OP_T_D
  .word opcode_df-OP_T_D
  .word opcode_e0-OP_T_D
  .word opcode_e1_D-OP_T_D
  .word opcode_e2-OP_T_D
  .word opcode_e3-OP_T_D
  .word opcode_e4-OP_T_D
  .word opcode_e5_D-OP_T_D
  .word opcode_e6-OP_T_D
  .word opcode_e7-OP_T_D
  .word opcode_e8-OP_T_D
  .word opcode_e9_D-OP_T_D
  .word opcode_ea-OP_T_D
  .word opcode_eb_D-OP_T_D
  .word opcode_ec-OP_T_D
  .word opcode_ed_D-OP_T_D
  .word opcode_ee-OP_T_D
  .word opcode_ef-OP_T_D
  .word opcode_f0-OP_T_D
  .word opcode_f1_D-OP_T_D
  .word opcode_f2-OP_T_D
  .word opcode_f3-OP_T_D
  .word opcode_f4-OP_T_D
  .word opcode_f5_D-OP_T_D
  .word opcode_f6-OP_T_D
  .word opcode_f7-OP_T_D
  .word opcode_f8-OP_T_D
  .word opcode_f9_D-OP_T_D
  .word opcode_fa-OP_T_D
  .word opcode_fb-OP_T_D
  .word opcode_fc-OP_T_D
  .word opcode_fd_D-OP_T_D
  .word opcode_fe-OP_T_D
  .word opcode_ff-OP_T_D

cycles:
_cycles:   | dc.l for the world outside, equ for internal use
  .long cy_BRK,cy_IndX,cy_CIM,cy_IndX_RW,cy_NOP3,cy_ZP,cy_ZP_RW,cy_ZP_RW
  .long cy_RegPH,cy_Imm,cy_RegChg,cy_Imm,cy_SKW,cy_Abs,cy_Abs_RW,cy_Abs_RW
  .long cy_Bcc,cy_IndY,cy_CIM,cy_IndY_RW,cy_NOP4,cy_ZPX,cy_ZPX_RW,cy_ZPX_RW
  .long cy_FlagCS,cy_AbsY,cy_NOP,cy_AbsY_RW,cy_SKW,cy_AbsX,cy_AbsX_RW,cy_AbsX_RW
  .long cy_Sub,cy_IndX,cy_CIM,cy_IndX_RW,cy_ZP,cy_ZP,cy_ZP_RW,cy_ZP_RW
  .long cy_RegPL,cy_Imm,cy_RegChg,cy_Imm,cy_Abs,cy_Abs,cy_Abs_RW,cy_Abs_RW
  .long cy_Bcc,cy_IndY,cy_CIM,cy_IndY_RW,cy_NOP4,cy_ZPX,cy_ZPX_RW,cy_ZPX_RW
  .long cy_FlagCS,cy_AbsY,cy_NOP,cy_AbsY_RW,cy_SKW,cy_AbsX,cy_AbsX_RW,cy_AbsX_RW
  .long cy_Sub,cy_IndX,cy_CIM,cy_IndX_RW,cy_NOP3,cy_ZP,cy_ZP_RW,cy_ZP_RW
  .long cy_RegPH,cy_Imm,cy_RegChg,cy_Imm,cy_JmpAbs,cy_Abs,cy_Abs_RW,cy_Abs_RW
  .long cy_Bcc,cy_IndY,cy_CIM,cy_IndY_RW,cy_NOP4,cy_ZPX,cy_ZPX_RW,cy_ZPX_RW
  .long cy_FlagCS,cy_AbsY,cy_NOP,cy_AbsY_RW,cy_SKW,cy_AbsX,cy_AbsX_RW,cy_AbsX_RW
  .long cy_Sub,cy_IndX,cy_CIM,cy_IndX_RW,cy_NOP3,cy_ZP,cy_ZP_RW,cy_ZP_RW
  .long cy_RegPL,cy_Imm,cy_RegChg,cy_Imm,cy_JmpInd,cy_Abs,cy_Abs_RW,cy_Abs_RW
  .long cy_Bcc,cy_IndY,cy_CIM,cy_IndY_RW,cy_NOP4,cy_ZPX,cy_ZPX_RW,cy_ZPX_RW
  .long cy_FlagCS,cy_AbsY,cy_NOP,cy_AbsY_RW,cy_SKW,cy_AbsX,cy_AbsX_RW,cy_AbsX_RW
  .long cy_NOP2,cy_IndX,cy_NOP2,cy_IndX,cy_ZP,cy_ZP,cy_ZP,cy_ZP
  .long cy_RegChg,cy_NOP2,cy_RegChg,cy_Imm,cy_Abs,cy_Abs,cy_Abs,cy_Abs
  .long cy_Bcc,cy_IndY2,cy_CIM,cy_IndY2,cy_ZPX,cy_ZPX,cy_ZPY,cy_ZPY
  .long cy_RegChg,cy_IndY2,cy_RegChg,cy_IndY2,cy_AbsX2,cy_AbsX2,cy_IndY2,cy_IndY2
  .long cy_Imm,cy_IndX,cy_Imm,cy_IndX,cy_ZP,cy_ZP,cy_ZP,cy_ZP
  .long cy_RegChg,cy_Imm,cy_RegChg,cy_Imm,cy_Abs,cy_Abs,cy_Abs,cy_Abs
  .long cy_Bcc,cy_IndY,cy_CIM,cy_IndY,cy_ZPX,cy_ZPX,cy_ZPY,cy_ZPY
  .long cy_FlagCS,cy_AbsY,cy_RegChg,cy_AbsY,cy_AbsX,cy_AbsX,cy_AbsY,cy_AbsY
  .long cy_Imm,cy_IndX,cy_NOP2,cy_IndX_RW,cy_ZP,cy_ZP,cy_ZP_RW,cy_ZP_RW
  .long cy_RegChg,cy_Imm,cy_RegChg,cy_Imm,cy_Abs,cy_Abs,cy_Abs_RW,cy_Abs_RW
  .long cy_Bcc,cy_IndY,cy_CIM,cy_IndY_RW,cy_NOP4,cy_ZPX,cy_ZPX_RW,cy_ZPX_RW
  .long cy_FlagCS,cy_AbsY,cy_NOP,cy_AbsY_RW,cy_SKW,cy_AbsX,cy_AbsX_RW,cy_AbsX_RW
  .long cy_Imm,cy_IndX,cy_NOP2,cy_IndX_RW,cy_ZP,cy_ZP,cy_ZP_RW,cy_ZP_RW
  .long cy_RegChg,cy_Imm,cy_NOP,cy_Imm,cy_Abs,cy_Abs,cy_Abs_RW,cy_Abs_RW
  .long cy_Bcc,cy_IndY,cy_CIM,cy_IndY_RW,cy_NOP4,cy_ZPX,cy_ZPX_RW,cy_ZPX_RW
  .long cy_FlagCS,cy_AbsY,cy_NOP,cy_AbsY_RW,cy_SKW,cy_AbsX,cy_AbsX_RW,cy_AbsX_RW


