/** Z80 Disassembler ******************************************/
/**                                                          **/
/**                         dasmz80.c                        **/
/**                                                          **/
/** This file contains the source of a portable disassembler **/
/** for the Z80 CPU.                                         **/
/**                                                          **/
/** Copyright (C) Marat Fayzullin 1995-1999                  **/
/**     You are not allowed to distribute this software      **/
/**     commercially. Please, notify me, if you make any     **/
/**     changes to this file.                                **/
/**************************************************************/

#include <stdio.h>
#include <string.h>

#ifdef ZLIB
#include <zlib.h>
#define fopen          gzopen
#define fclose         gzclose
#define fread(B,N,L,F) gzread(F,B,(L)*(N))
#endif

typedef unsigned char byte;   /* This type is exactly 1 byte  */
typedef unsigned short word;  /* This type is exactly 2 bytes */

static int PrintHex;          /* Print hexadecimal codes      */
static unsigned long Counter; /* Address counter              */

/*static - Removed by neopop_uk */ int DAsm(char *S,byte *A);
    /* This function will disassemble a single command and    */
    /* return the number of bytes disassembled.               */

static char *Mnemonics[256] =
{
  "NOP","LD BC,#h","LD (BC),A","INC BC","INC B","DEC B","LD B,*h","RLCA",
  "EX AF,AF'","ADD HL,BC","LD A,(BC)","DEC BC","INC C","DEC C","LD C,*h","RRCA",
  "DJNZ @h","LD DE,#h","LD (DE),A","INC DE","INC D","DEC D","LD D,*h","RLA",
  "JR @h","ADD HL,DE","LD A,(DE)","DEC DE","INC E","DEC E","LD E,*h","RRA",
  "JR NZ,@h","LD HL,#h","LD (#h),HL","INC HL","INC H","DEC H","LD H,*h","DAA",
  "JR Z,@h","ADD HL,HL","LD HL,(#h)","DEC HL","INC L","DEC L","LD L,*h","CPL",
  "JR NC,@h","LD SP,#h","LD (#h),A","INC SP","INC (HL)","DEC (HL)","LD (HL),*h","SCF",
  "JR C,@h","ADD HL,SP","LD A,(#h)","DEC SP","INC A","DEC A","LD A,*h","CCF",
  "LD B,B","LD B,C","LD B,D","LD B,E","LD B,H","LD B,L","LD B,(HL)","LD B,A",
  "LD C,B","LD C,C","LD C,D","LD C,E","LD C,H","LD C,L","LD C,(HL)","LD C,A",
  "LD D,B","LD D,C","LD D,D","LD D,E","LD D,H","LD D,L","LD D,(HL)","LD D,A",
  "LD E,B","LD E,C","LD E,D","LD E,E","LD E,H","LD E,L","LD E,(HL)","LD E,A",
  "LD H,B","LD H,C","LD H,D","LD H,E","LD H,H","LD H,L","LD H,(HL)","LD H,A",
  "LD L,B","LD L,C","LD L,D","LD L,E","LD L,H","LD L,L","LD L,(HL)","LD L,A",
  "LD (HL),B","LD (HL),C","LD (HL),D","LD (HL),E","LD (HL),H","LD (HL),L","HALT","LD (HL),A",
  "LD A,B","LD A,C","LD A,D","LD A,E","LD A,H","LD A,L","LD A,(HL)","LD A,A",
  "ADD B","ADD C","ADD D","ADD E","ADD H","ADD L","ADD (HL)","ADD A",
  "ADC B","ADC C","ADC D","ADC E","ADC H","ADC L","ADC (HL)","ADC A",
  "SUB B","SUB C","SUB D","SUB E","SUB H","SUB L","SUB (HL)","SUB A",
  "SBC B","SBC C","SBC D","SBC E","SBC H","SBC L","SBC (HL)","SBC A",
  "AND B","AND C","AND D","AND E","AND H","AND L","AND (HL)","AND A",
  "XOR B","XOR C","XOR D","XOR E","XOR H","XOR L","XOR (HL)","XOR A",
  "OR B","OR C","OR D","OR E","OR H","OR L","OR (HL)","OR A",
  "CP B","CP C","CP D","CP E","CP H","CP L","CP (HL)","CP A",
  "RET NZ","POP BC","JP NZ,#h","JP #h","CALL NZ,#h","PUSH BC","ADD *h","RST 00h",
  "RET Z","RET","JP Z,#h","PFX_CB","CALL Z,#h","CALL #h","ADC *h","RST 08h",
  "RET NC","POP DE","JP NC,#h","OUTA (*h)","CALL NC,#h","PUSH DE","SUB *h","RST 10h",
  "RET C","EXX","JP C,#h","INA (*h)","CALL C,#h","PFX_DD","SBC *h","RST 18h",
  "RET PO","POP HL","JP PO,#h","EX HL,(SP)","CALL PO,#h","PUSH HL","AND *h","RST 20h",
  "RET PE","LD PC,HL","JP PE,#h","EX DE,HL","CALL PE,#h","PFX_ED","XOR *h","RST 28h",
  "RET P","POP AF","JP P,#h","DI","CALL P,#h","PUSH AF","OR *h","RST 30h",
  "RET M","LD SP,HL","JP M,#h","EI","CALL M,#h","PFX_FD","CP *h","RST 38h"
};

static char *MnemonicsCB[256] =
{
  "RLC B","RLC C","RLC D","RLC E","RLC H","RLC L","RLC (HL)","RLC A",
  "RRC B","RRC C","RRC D","RRC E","RRC H","RRC L","RRC (HL)","RRC A",
  "RL B","RL C","RL D","RL E","RL H","RL L","RL (HL)","RL A",
  "RR B","RR C","RR D","RR E","RR H","RR L","RR (HL)","RR A",
  "SLA B","SLA C","SLA D","SLA E","SLA H","SLA L","SLA (HL)","SLA A",
  "SRA B","SRA C","SRA D","SRA E","SRA H","SRA L","SRA (HL)","SRA A",
  "SLL B","SLL C","SLL D","SLL E","SLL H","SLL L","SLL (HL)","SLL A",
  "SRL B","SRL C","SRL D","SRL E","SRL H","SRL L","SRL (HL)","SRL A",
  "BIT 0,B","BIT 0,C","BIT 0,D","BIT 0,E","BIT 0,H","BIT 0,L","BIT 0,(HL)","BIT 0,A",
  "BIT 1,B","BIT 1,C","BIT 1,D","BIT 1,E","BIT 1,H","BIT 1,L","BIT 1,(HL)","BIT 1,A",
  "BIT 2,B","BIT 2,C","BIT 2,D","BIT 2,E","BIT 2,H","BIT 2,L","BIT 2,(HL)","BIT 2,A",
  "BIT 3,B","BIT 3,C","BIT 3,D","BIT 3,E","BIT 3,H","BIT 3,L","BIT 3,(HL)","BIT 3,A",
  "BIT 4,B","BIT 4,C","BIT 4,D","BIT 4,E","BIT 4,H","BIT 4,L","BIT 4,(HL)","BIT 4,A",
  "BIT 5,B","BIT 5,C","BIT 5,D","BIT 5,E","BIT 5,H","BIT 5,L","BIT 5,(HL)","BIT 5,A",
  "BIT 6,B","BIT 6,C","BIT 6,D","BIT 6,E","BIT 6,H","BIT 6,L","BIT 6,(HL)","BIT 6,A",
  "BIT 7,B","BIT 7,C","BIT 7,D","BIT 7,E","BIT 7,H","BIT 7,L","BIT 7,(HL)","BIT 7,A",
  "RES 0,B","RES 0,C","RES 0,D","RES 0,E","RES 0,H","RES 0,L","RES 0,(HL)","RES 0,A",
  "RES 1,B","RES 1,C","RES 1,D","RES 1,E","RES 1,H","RES 1,L","RES 1,(HL)","RES 1,A",
  "RES 2,B","RES 2,C","RES 2,D","RES 2,E","RES 2,H","RES 2,L","RES 2,(HL)","RES 2,A",
  "RES 3,B","RES 3,C","RES 3,D","RES 3,E","RES 3,H","RES 3,L","RES 3,(HL)","RES 3,A",
  "RES 4,B","RES 4,C","RES 4,D","RES 4,E","RES 4,H","RES 4,L","RES 4,(HL)","RES 4,A",
  "RES 5,B","RES 5,C","RES 5,D","RES 5,E","RES 5,H","RES 5,L","RES 5,(HL)","RES 5,A",
  "RES 6,B","RES 6,C","RES 6,D","RES 6,E","RES 6,H","RES 6,L","RES 6,(HL)","RES 6,A",
  "RES 7,B","RES 7,C","RES 7,D","RES 7,E","RES 7,H","RES 7,L","RES 7,(HL)","RES 7,A",
  "SET 0,B","SET 0,C","SET 0,D","SET 0,E","SET 0,H","SET 0,L","SET 0,(HL)","SET 0,A",
  "SET 1,B","SET 1,C","SET 1,D","SET 1,E","SET 1,H","SET 1,L","SET 1,(HL)","SET 1,A",
  "SET 2,B","SET 2,C","SET 2,D","SET 2,E","SET 2,H","SET 2,L","SET 2,(HL)","SET 2,A",
  "SET 3,B","SET 3,C","SET 3,D","SET 3,E","SET 3,H","SET 3,L","SET 3,(HL)","SET 3,A",
  "SET 4,B","SET 4,C","SET 4,D","SET 4,E","SET 4,H","SET 4,L","SET 4,(HL)","SET 4,A",
  "SET 5,B","SET 5,C","SET 5,D","SET 5,E","SET 5,H","SET 5,L","SET 5,(HL)","SET 5,A",
  "SET 6,B","SET 6,C","SET 6,D","SET 6,E","SET 6,H","SET 6,L","SET 6,(HL)","SET 6,A",
  "SET 7,B","SET 7,C","SET 7,D","SET 7,E","SET 7,H","SET 7,L","SET 7,(HL)","SET 7,A"
};

static char *MnemonicsED[256] =
{
  "DB EDh,00h","DB EDh,01h","DB EDh,02h","DB EDh,03h",
  "DB EDh,04h","DB EDh,05h","DB EDh,06h","DB EDh,07h",
  "DB EDh,08h","DB EDh,09h","DB EDh,0Ah","DB EDh,0Bh",
  "DB EDh,0Ch","DB EDh,0Dh","DB EDh,0Eh","DB EDh,0Fh",
  "DB EDh,10h","DB EDh,11h","DB EDh,12h","DB EDh,13h",
  "DB EDh,14h","DB EDh,15h","DB EDh,16h","DB EDh,17h",
  "DB EDh,18h","DB EDh,19h","DB EDh,1Ah","DB EDh,1Bh",
  "DB EDh,1Ch","DB EDh,1Dh","DB EDh,1Eh","DB EDh,1Fh",
  "DB EDh,20h","DB EDh,21h","DB EDh,22h","DB EDh,23h",
  "DB EDh,24h","DB EDh,25h","DB EDh,26h","DB EDh,27h",
  "DB EDh,28h","DB EDh,29h","DB EDh,2Ah","DB EDh,2Bh",
  "DB EDh,2Ch","DB EDh,2Dh","DB EDh,2Eh","DB EDh,2Fh",
  "DB EDh,30h","DB EDh,31h","DB EDh,32h","DB EDh,33h",
  "DB EDh,34h","DB EDh,35h","DB EDh,36h","DB EDh,37h",
  "DB EDh,38h","DB EDh,39h","DB EDh,3Ah","DB EDh,3Bh",
  "DB EDh,3Ch","DB EDh,3Dh","DB EDh,3Eh","DB EDh,3Fh",
  "IN B,(C)","OUT (C),B","SBC HL,BC","LD (#h),BC",
  "NEG","RETN","IM 0","LD I,A",
  "IN C,(C)","OUT (C),C","ADC HL,BC","LD BC,(#h)",
  "DB EDh,4Ch","RETI","DB EDh,4Eh","LD R,A",
  "IN D,(C)","OUT (C),D","SBC HL,DE","LD (#h),DE",
  "DB EDh,54h","DB EDh,55h","IM 1","LD A,I",
  "IN E,(C)","OUT (C),E","ADC HL,DE","LD DE,(#h)",
  "DB EDh,5Ch","DB EDh,5Dh","IM 2","LD A,R",
  "IN H,(C)","OUT (C),H","SBC HL,HL","LD (#h),HL",
  "DB EDh,64h","DB EDh,65h","DB EDh,66h","RRD",
  "IN L,(C)","OUT (C),L","ADC HL,HL","LD HL,(#h)",
  "DB EDh,6Ch","DB EDh,6Dh","DB EDh,6Eh","RLD",
  "IN F,(C)","DB EDh,71h","SBC HL,SP","LD (#h),SP",
  "DB EDh,74h","DB EDh,75h","DB EDh,76h","DB EDh,77h",
  "IN A,(C)","OUT (C),A","ADC HL,SP","LD SP,(#h)",
  "DB EDh,7Ch","DB EDh,7Dh","DB EDh,7Eh","DB EDh,7Fh",
  "DB EDh,80h","DB EDh,81h","DB EDh,82h","DB EDh,83h",
  "DB EDh,84h","DB EDh,85h","DB EDh,86h","DB EDh,87h",
  "DB EDh,88h","DB EDh,89h","DB EDh,8Ah","DB EDh,8Bh",
  "DB EDh,8Ch","DB EDh,8Dh","DB EDh,8Eh","DB EDh,8Fh",
  "DB EDh,90h","DB EDh,91h","DB EDh,92h","DB EDh,93h",
  "DB EDh,94h","DB EDh,95h","DB EDh,96h","DB EDh,97h",
  "DB EDh,98h","DB EDh,99h","DB EDh,9Ah","DB EDh,9Bh",
  "DB EDh,9Ch","DB EDh,9Dh","DB EDh,9Eh","DB EDh,9Fh",
  "LDI","CPI","INI","OUTI",
  "DB EDh,A4h","DB EDh,A5h","DB EDh,A6h","DB EDh,A7h",
  "LDD","CPD","IND","OUTD",
  "DB EDh,ACh","DB EDh,ADh","DB EDh,AEh","DB EDh,AFh",
  "LDIR","CPIR","INIR","OTIR",
  "DB EDh,B4h","DB EDh,B5h","DB EDh,B6h","DB EDh,B7h",
  "LDDR","CPDR","INDR","OTDR",
  "DB EDh,BCh","DB EDh,BDh","DB EDh,BEh","DB EDh,BFh",
  "DB EDh,C0h","DB EDh,C1h","DB EDh,C2h","DB EDh,C3h",
  "DB EDh,C4h","DB EDh,C5h","DB EDh,C6h","DB EDh,C7h",
  "DB EDh,C8h","DB EDh,C9h","DB EDh,CAh","DB EDh,CBh",
  "DB EDh,CCh","DB EDh,CDh","DB EDh,CEh","DB EDh,CFh",
  "DB EDh,D0h","DB EDh,D1h","DB EDh,D2h","DB EDh,D3h",
  "DB EDh,D4h","DB EDh,D5h","DB EDh,D6h","DB EDh,D7h",
  "DB EDh,D8h","DB EDh,D9h","DB EDh,DAh","DB EDh,DBh",
  "DB EDh,DCh","DB EDh,DDh","DB EDh,DEh","DB EDh,DFh",
  "DB EDh,E0h","DB EDh,E1h","DB EDh,E2h","DB EDh,E3h",
  "DB EDh,E4h","DB EDh,E5h","DB EDh,E6h","DB EDh,E7h",
  "DB EDh,E8h","DB EDh,E9h","DB EDh,EAh","DB EDh,EBh",
  "DB EDh,ECh","DB EDh,EDh","DB EDh,EEh","DB EDh,EFh",
  "DB EDh,F0h","DB EDh,F1h","DB EDh,F2h","DB EDh,F3h",
  "DB EDh,F4h","DB EDh,F5h","DB EDh,F6h","DB EDh,F7h",
  "DB EDh,F8h","DB EDh,F9h","DB EDh,FAh","DB EDh,FBh",
  "DB EDh,FCh","DB EDh,FDh","DB EDh,FEh","DB EDh,FFh"
};

static char *MnemonicsXX[256] =
{
  "NOP","LD BC,#h","LD (BC),A","INC BC","INC B","DEC B","LD B,*h","RLCA",
  "EX AF,AF'","ADD I%,BC","LD A,(BC)","DEC BC","INC C","DEC C","LD C,*h","RRCA",
  "DJNZ @h","LD DE,#h","LD (DE),A","INC DE","INC D","DEC D","LD D,*h","RLA",
  "JR @h","ADD I%,DE","LD A,(DE)","DEC DE","INC E","DEC E","LD E,*h","RRA",
  "JR NZ,@h","LD I%,#h","LD (#h),I%","INC I%","INC I%h","DEC I%h","LD I%h,*h","DAA",
  "JR Z,@h","ADD I%,I%","LD I%,(#h)","DEC I%","INC I%l","DEC I%l","LD I%l,*h","CPL",
  "JR NC,@h","LD SP,#h","LD (#h),A","INC SP","INC (I%+^h)","DEC (I%+^h)","LD (I%+^h),*h","SCF",
  "JR C,@h","ADD I%,SP","LD A,(#h)","DEC SP","INC A","DEC A","LD A,*h","CCF",
  "LD B,B","LD B,C","LD B,D","LD B,E","LD B,I%h","LD B,I%l","LD B,(I%+^h)","LD B,A",
  "LD C,B","LD C,C","LD C,D","LD C,E","LD C,I%h","LD C,I%l","LD C,(I%+^h)","LD C,A",
  "LD D,B","LD D,C","LD D,D","LD D,E","LD D,I%h","LD D,I%l","LD D,(I%+^h)","LD D,A",
  "LD E,B","LD E,C","LD E,D","LD E,E","LD E,I%h","LD E,I%l","LD E,(I%+^h)","LD E,A",
  "LD I%h,B","LD I%h,C","LD I%h,D","LD I%h,E","LD I%h,I%h","LD I%h,I%l","LD H,(I%+^h)","LD I%h,A",
  "LD I%l,B","LD I%l,C","LD I%l,D","LD I%l,E","LD I%l,I%h","LD I%l,I%l","LD L,(I%+^h)","LD I%l,A",
  "LD (I%+^h),B","LD (I%+^h),C","LD (I%+^h),D","LD (I%+^h),E","LD (I%+^h),H","LD (I%+^h),L","HALT","LD (I%+^h),A",
  "LD A,B","LD A,C","LD A,D","LD A,E","LD A,I%h","LD A,I%l","LD A,(I%+^h)","LD A,A",
  "ADD B","ADD C","ADD D","ADD E","ADD I%h","ADD I%l","ADD (I%+^h)","ADD A",
  "ADC B","ADC C","ADC D","ADC E","ADC I%h","ADC I%l","ADC (I%+^h)","ADC,A",
  "SUB B","SUB C","SUB D","SUB E","SUB I%h","SUB I%l","SUB (I%+^h)","SUB A",
  "SBC B","SBC C","SBC D","SBC E","SBC I%h","SBC I%l","SBC (I%+^h)","SBC A",
  "AND B","AND C","AND D","AND E","AND I%h","AND I%l","AND (I%+^h)","AND A",
  "XOR B","XOR C","XOR D","XOR E","XOR I%h","XOR I%l","XOR (I%+^h)","XOR A",
  "OR B","OR C","OR D","OR E","OR I%h","OR I%l","OR (I%+^h)","OR A",
  "CP B","CP C","CP D","CP E","CP I%h","CP I%l","CP (I%+^h)","CP A",
  "RET NZ","POP BC","JP NZ,#h","JP #h","CALL NZ,#h","PUSH BC","ADD *h","RST 00h",
  "RET Z","RET","JP Z,#h","PFX_CB","CALL Z,#h","CALL #h","ADC *h","RST 08h",
  "RET NC","POP DE","JP NC,#h","OUTA (*h)","CALL NC,#h","PUSH DE","SUB *h","RST 10h",
  "RET C","EXX","JP C,#h","INA (*h)","CALL C,#h","PFX_DD","SBC *h","RST 18h",
  "RET PO","POP I%","JP PO,#h","EX I%,(SP)","CALL PO,#h","PUSH I%","AND *h","RST 20h",
  "RET PE","LD PC,I%","JP PE,#h","EX DE,I%","CALL PE,#h","PFX_ED","XOR *h","RST 28h",
  "RET P","POP AF","JP P,#h","DI","CALL P,#h","PUSH AF","OR *h","RST 30h",
  "RET M","LD SP,I%","JP M,#h","EI","CALL M,#h","PFX_FD","CP *h","RST 38h"
};

static char *MnemonicsXCB[256] =
{
  "RLC B","RLC C","RLC D","RLC E","RLC H","RLC L","RLC (I%@h)","RLC A",
  "RRC B","RRC C","RRC D","RRC E","RRC H","RRC L","RRC (I%@h)","RRC A",
  "RL B","RL C","RL D","RL E","RL H","RL L","RL (I%@h)","RL A",
  "RR B","RR C","RR D","RR E","RR H","RR L","RR (I%@h)","RR A",
  "SLA B","SLA C","SLA D","SLA E","SLA H","SLA L","SLA (I%@h)","SLA A",
  "SRA B","SRA C","SRA D","SRA E","SRA H","SRA L","SRA (I%@h)","SRA A",
  "SLL B","SLL C","SLL D","SLL E","SLL H","SLL L","SLL (I%@h)","SLL A",
  "SRL B","SRL C","SRL D","SRL E","SRL H","SRL L","SRL (I%@h)","SRL A",
  "BIT 0,B","BIT 0,C","BIT 0,D","BIT 0,E","BIT 0,H","BIT 0,L","BIT 0,(I%@h)","BIT 0,A",
  "BIT 1,B","BIT 1,C","BIT 1,D","BIT 1,E","BIT 1,H","BIT 1,L","BIT 1,(I%@h)","BIT 1,A",
  "BIT 2,B","BIT 2,C","BIT 2,D","BIT 2,E","BIT 2,H","BIT 2,L","BIT 2,(I%@h)","BIT 2,A",
  "BIT 3,B","BIT 3,C","BIT 3,D","BIT 3,E","BIT 3,H","BIT 3,L","BIT 3,(I%@h)","BIT 3,A",
  "BIT 4,B","BIT 4,C","BIT 4,D","BIT 4,E","BIT 4,H","BIT 4,L","BIT 4,(I%@h)","BIT 4,A",
  "BIT 5,B","BIT 5,C","BIT 5,D","BIT 5,E","BIT 5,H","BIT 5,L","BIT 5,(I%@h)","BIT 5,A",
  "BIT 6,B","BIT 6,C","BIT 6,D","BIT 6,E","BIT 6,H","BIT 6,L","BIT 6,(I%@h)","BIT 6,A",
  "BIT 7,B","BIT 7,C","BIT 7,D","BIT 7,E","BIT 7,H","BIT 7,L","BIT 7,(I%@h)","BIT 7,A",
  "RES 0,B","RES 0,C","RES 0,D","RES 0,E","RES 0,H","RES 0,L","RES 0,(I%@h)","RES 0,A",
  "RES 1,B","RES 1,C","RES 1,D","RES 1,E","RES 1,H","RES 1,L","RES 1,(I%@h)","RES 1,A",
  "RES 2,B","RES 2,C","RES 2,D","RES 2,E","RES 2,H","RES 2,L","RES 2,(I%@h)","RES 2,A",
  "RES 3,B","RES 3,C","RES 3,D","RES 3,E","RES 3,H","RES 3,L","RES 3,(I%@h)","RES 3,A",
  "RES 4,B","RES 4,C","RES 4,D","RES 4,E","RES 4,H","RES 4,L","RES 4,(I%@h)","RES 4,A",
  "RES 5,B","RES 5,C","RES 5,D","RES 5,E","RES 5,H","RES 5,L","RES 5,(I%@h)","RES 5,A",
  "RES 6,B","RES 6,C","RES 6,D","RES 6,E","RES 6,H","RES 6,L","RES 6,(I%@h)","RES 6,A",
  "RES 7,B","RES 7,C","RES 7,D","RES 7,E","RES 7,H","RES 7,L","RES 7,(I%@h)","RES 7,A",
  "SET 0,B","SET 0,C","SET 0,D","SET 0,E","SET 0,H","SET 0,L","SET 0,(I%@h)","SET 0,A",
  "SET 1,B","SET 1,C","SET 1,D","SET 1,E","SET 1,H","SET 1,L","SET 1,(I%@h)","SET 1,A",
  "SET 2,B","SET 2,C","SET 2,D","SET 2,E","SET 2,H","SET 2,L","SET 2,(I%@h)","SET 2,A",
  "SET 3,B","SET 3,C","SET 3,D","SET 3,E","SET 3,H","SET 3,L","SET 3,(I%@h)","SET 3,A",
  "SET 4,B","SET 4,C","SET 4,D","SET 4,E","SET 4,H","SET 4,L","SET 4,(I%@h)","SET 4,A",
  "SET 5,B","SET 5,C","SET 5,D","SET 5,E","SET 5,H","SET 5,L","SET 5,(I%@h)","SET 5,A",
  "SET 6,B","SET 6,C","SET 6,D","SET 6,E","SET 6,H","SET 6,L","SET 6,(I%@h)","SET 6,A",
  "SET 7,B","SET 7,C","SET 7,D","SET 7,E","SET 7,H","SET 7,L","SET 7,(I%@h)","SET 7,A"
};

/** DAsm() ****************************************************/
/** This function will disassemble a single command and      **/
/** return the number of bytes disassembled.                 **/
/**************************************************************/
int DAsm(char *S,byte *A)
{
  char R[128],H[10],C,*T,*P;
  byte *B,J,Offset;

  B=A;C='\0';J=0;

  switch(*B)
  {
    case 0xCB: B++;T=MnemonicsCB[*B++];break;
    case 0xED: B++;T=MnemonicsED[*B++];break;
    case 0xDD: B++;C='X';
               if(*B!=0xCB) T=MnemonicsXX[*B++];
               else { B++;Offset=*B++;J=1;T=MnemonicsXCB[*B++]; }
               break;
    case 0xFD: B++;C='Y';
               if(*B!=0xCB) T=MnemonicsXX[*B++];
               else { B++;Offset=*B++;J=1;T=MnemonicsXCB[*B++]; }
               break;
    default:   T=Mnemonics[*B++];
  }

  if(P=strchr(T,'^'))
  {
    strncpy(R,T,P-T);R[P-T]='\0';
    sprintf(H,"%02X",*B++);
    strcat(R,H);strcat(R,P+1);
  }
  else strcpy(R,T);
  if(P=strchr(R,'%')) *P=C;

  if(P=strchr(R,'*'))
  {
    strncpy(S,R,P-R);S[P-R]='\0';
    sprintf(H,"%02X",*B++);
    strcat(S,H);strcat(S,P+1);
  }
  else
    if(P=strchr(R,'@'))
    {
      strncpy(S,R,P-R);S[P-R]='\0';
      if(!J) Offset=*B++;
      strcat(S,Offset&0x80? "-":"+");
      J=Offset&0x80? 256-Offset:Offset;
      sprintf(H,"%02X",J);
      strcat(S,H);strcat(S,P+1);
    }
    else
      if(P=strchr(R,'#'))
      {
        strncpy(S,R,P-R);S[P-R]='\0';
        sprintf(H,"%04X",B[0]+256*B[1]);
        strcat(S,H);strcat(S,P+1);
        B+=2;
      }
      else strcpy(S,R);

  return(B-A);
}



/** main() ****************************************************/        
/** This is the main function from which execution starts.   **/
/**************************************************************/
/*		Function removed by neopop_uk
int main(int argc,char *argv[])
{
  FILE *F;
  int N,I,J;
  byte Buf[32];
  char S[128];

  Counter=0L;PrintHex=0;

  for(N=1;(N<argc)&&(*argv[N]=='-');N++)
    switch(argv[N][1])
    {
      case 'o': sscanf(argv[N],"-o%lx",&Counter);
                Counter&=0xFFFFFFFFL;break;
      default:
        for(J=1;argv[N][J];J++)
          switch(argv[N][J])
          {
            case 'h': PrintHex=1;break;
            default:
              fprintf(stderr,"%s: Unknown option -%c\n",argv[0],argv[N][J]);
          }
    }

  if(N==argc)
  {
    fprintf(stderr,"DASMZ80 Z80 Disassembler v.3.0 by Marat Fayzullin\n");
#ifdef ZLIB
    fprintf(stderr,"  This program will transparently uncompress singular\n");
    fprintf(stderr,"  GZIPped and PKZIPped files.\n");
#endif
    fprintf(stderr,"Usage: %s [-h] [-oOrigin] file\n",argv[0]);
    fprintf(stderr,"  -h - Print hexadecimal values\n");
    fprintf(stderr,"  -o - Count addresses from a given origin (hex)\n");
    return(1);
  }

  if(!(F=fopen(argv[N],"rb")))
  { printf("\n%s: Can't open file %s\n",argv[0],argv[N]);return(1); }

  for(N=0;N+=fread(Buf+N,1,16-N,F);)
  {
    memset(Buf+N,0,32-N);
    I=DAsm(S,Buf);
    printf("%08lX:",Counter);
    if(PrintHex)
    {
      for(J=0;J<I;J++) printf(" %02X",Buf[J]);
      if(I<3) printf("\t");
    }
    printf("\t%s\n",S);
    Counter+=I;
    N=N>I? N-I:0;
    for(J=0;J<N;J++) Buf[J]=Buf[J+I];
  }

  fclose(F);return(0);
}
*/