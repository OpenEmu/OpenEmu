/* V810 Emulator
 *
 * Copyright (C) 2006 David Tucker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//////////////////////////////////////////////////////////
// CPU Debug routines

//what do we realy need?
//#include <stdio.h>
//#include <stdlib.h> 
//#include <string.h>
//#include <ctype.h>

#include "mednafen/mednafen.h"
#include "v810_opt.h"
#include "v810_cpu.h"
#include "v810_cpuD.h"


////////////////////////////////////////////////////////////
// Defines

//Structure to store an element in our linked list
// used to dynamicaly dissasemble a rom
typedef struct dasms {
	int            offset;
	uint32 		   PC;
	uint32           jump;
      struct dasms * nextElement;
} dasmS;

typedef struct {
   int addr_mode;               // Addressing mode
   const char * opname;         // Opcode name (string)
} operation;

static const operation optable[80] = {
  { AM_I,       "mov  " },           // 0x00
  { AM_I,       "add  " },           // 0x01
  { AM_I,       "sub  " },           // 0x02
  { AM_I,       "cmp  " },           // 0x03
  { AM_I,       "shl  " },           // 0x04
  { AM_I,       "shr  " },           // 0x05
  { AM_I,       "jmp  " },           // 0x06
  { AM_I,       "sar  " },           // 0x07
  { AM_I,       "mul  " },           // 0x08
  { AM_I,       "div  " },           // 0x09
  { AM_I,       "mulu " },           // 0x0A
  { AM_I,       "divu " },           // 0x0B
  { AM_I,       "or   " },           // 0x0C
  { AM_I,       "and  " },           // 0x0D
  { AM_I,       "xor  " },           // 0x0E
  { AM_I,       "not  " },           // 0x0F

  { AM_II,      "mov  " },           // 0x10  // Imediate
  { AM_II,      "add  " },           // 0x11
  { AM_II,      "setf " },           // 0x12
  { AM_II,      "cmp  " },           // 0x13
  { AM_II,      "shl  " },           // 0x14
  { AM_II,      "shr  " },           // 0x15
  { AM_UDEF,    "???  " },           // 0x16  // Unknown
  { AM_II,      "sar  " },           // 0x17
  { AM_II,      "trap " },           // 0x18

  { AM_IX,      "reti " },           // 0x19  //BRKRETI
  { AM_IX,      "halt " },           // 0x1A  //STBY

  {AM_UDEF,     "???  " },           // 0x1B  // Unknown
  { AM_II,      "ldsr " },           // 0x1C
  { AM_II,      "stsr " },           // 0x1D
  { AM_UDEF,    "???  " },           // 0x1E  // Unknown
  {AM_BSTR,     "BSTR " },           // 0x1F  // Special Bit String Instructions

  {AM_UDEF,     "???  " },           // 0x20  // Unknown   // This is a fudg on our part
  {AM_UDEF,     "???  " },           // 0x21  // Unknown   // We have 6 and 7 bit instructions
  {AM_UDEF,     "???  " },           // 0x22  // Unknown   // this is filld in by the Conditional Branch Instructions
  {AM_UDEF,     "???  " },           // 0x23  // Unknown
  {AM_UDEF,     "???  " },           // 0x24  // Unknown
  {AM_UDEF,     "???  " },           // 0x25  // Unknown
  {AM_UDEF,     "???  " },           // 0x26  // Unknown
  {AM_UDEF,     "???  " },           // 0x27  // Unknown

  { AM_V,       "movea" },           // 0x28
  { AM_V,       "addi " },           // 0x29
  { AM_IV,      "jr   " },           // 0x2A
  { AM_IV,      "jal  " },           // 0x2B
  { AM_V,       "ori  " },           // 0x2C
  { AM_V,       "andi " },           // 0x2D
  { AM_V,       "xori " },           // 0x2E
  { AM_V,       "movhi" },           // 0x2F

  { AM_VIa,     "ld.b " },           // 0x30
  { AM_VIa,     "ld.h " },           // 0x31
  {AM_UDEF,     "???  " },           // 0x32  // Unknown
  { AM_VIa,     "ld.w " },           // 0x33
  { AM_VIb,     "st.b " },           // 0x34
  { AM_VIb,     "st.h " },           // 0x35
  {AM_UDEF,     "???  " },           // 0x36  // Unknown
  { AM_VIb,     "st.w " },           // 0x37
  { AM_VIa,     "in.b " },           // 0x38
  { AM_VIa,     "in.h " },           // 0x39
  { AM_VIa,     "caxi " },           // 0x3A
  { AM_VIa,     "in.w " },           // 0x3B
  { AM_VIb,     "out.b" },           // 0x3C
  { AM_VIb,     "out.h" },           // 0x3D
  { AM_FPP,     "FPP  " },           // 0x3E  //Floating Point Instruction, Special Case
  { AM_VIb,     "out.w" },           // 0x3F

  { AM_III,     "bv   " },           // 0x40
  { AM_III,     "bl   " },           // 0x41  //BC  0x41
  { AM_III,     "be   " },           // 0x42  //BZ  0x42
  { AM_III,     "bnh  " },           // 0x43
  { AM_III,     "bn   " },           // 0x44
  { AM_III,     "br   " },           // 0x45
  { AM_III,     "blt  " },           // 0x46
  { AM_III,     "ble  " },           // 0x47
  { AM_III,     "bnv  " },           // 0x48
  { AM_III,     "bnl  " },           // 0x49 //BNC 0x49
  { AM_III,     "bne  " },           // 0x4A //BNZ 0x4A
  { AM_III,     "bh   " },           // 0x4B
  { AM_III,     "bp   " },           // 0x4C
  { AM_III,     "nop  " },           // 0x4D
  { AM_III,     "bge  " },           // 0x4E
  { AM_III,     "bgt  " }            // 0x4F
};
// All instructions greater than 0x50 are undefined (this should not be posible of cource)


//Structure for holding the SubOpcodes, Same as above, without the InsType.
typedef struct {
   const char * opname;               // Opcode name (string)
} suboperation;


//  Bit String Subopcodes
static const suboperation bssuboptable[16] = {
  { "SCH0BSU" },           // 0x00
  { "SCH0BSD" },           // 0x01
  { "SCH1BSU" },           // 0x02
  { "SCH1BSD" },           // 0x03
  { "BError4" },           // 0x04  // Unknown
  { "BError5" },           // 0x05  // Unknown
  { "BError6" },           // 0x06  // Unknown
  { "BError7" },           // 0x07  // Unknown
  { "ORBSU  " },           // 0x08
  { "ANDBSU " },           // 0x09
  { "XORBSU " },           // 0x0A
  { "MOVBSU " },           // 0x0B
  { "ORNBSU " },           // 0x0C
  { "ANDNBSU" },           // 0x0D
  { "XORNBSU" },           // 0x0E
  { "NOTBSU " }            // 0x0F
};

//  Floating Point Subopcodes
static const suboperation fpsuboptable[16] = {
  { "cmpf.s " },           // 0x00
  { "FError1" },           // 0x01  // Unknown
  { "cvt.ws " },           // 0x02
  { "cvt.sw " },           // 0x03
  { "addf.s " },           // 0x04
  { "subf.s " },           // 0x05
  { "mulf.s " },           // 0x06
  { "divf.s " },           // 0x07
  { "XB" },                // 0x08  // undocumented
  { "XH" },                // 0x09  // undocumented //VFishing???
  { "REV" },               // 0x0A  // undocumented
  { "trnc.sw" },           // 0x0B
  { "MPYHW" },             // 0x0C  // undocumented
  { "FErrorD" },           // 0x0D  // Unknown
  { "FErrorE" },           // 0x0E  // Unknown
  { "FErrorF" }            // 0x0F  // Unknown
};

static const char *pretty_preg_names[32] =
{
 "r0", "r1", "hsp", "sp", "gp", "tp", "r6", "r7",
 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
 "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
 "r24", "r25", "r26", "r27", "r28", "r29", "r30", "lp",
};

static const char *pretty_sreg_names[32] =
{
 "sr0(eipc)", "sr1(eipsw)", "sr2(fepc)", "sr3(fepsw)", "sr4(ecr)", "sr5(psw)", "sr6(pir)", "sr7(tkcw)",
 "sr8(invalid)", "sr9(invalid)", "sr10(invalid)", "sr11(invalid)", "sr12(invalid)", "sr13(invalid)", 
 "sr14(invalid)", "sr15(invalid)", "sr16(invalid)", "sr17(invalid)", "sr18(invalid)", "sr19(invalid)", 
 "sr20(invalid)", "sr21(invalid)", "sr22(invalid)", "sr23(invalid)", 
 "sr24(chcw)", "sr25(adtre)", "sr26(invalid)", "sr27(invalid)", "sr28(invalid)", "sr29(invalid)", "sr30(invalid)", 
 "sr31(invalid)" 
};

void v810_dis(uint32 &tPC, int num, char *buf, uint16 (*rhword)(uint32))
{
    int lowB, highB, lowB2, highB2;             // up to 4 bytes for instruction (either 16 or 32 bits)
    int opcode, arg1, arg2, arg3;
    int i = 0;

    buf[0] = 0;

    for(i = 0; i< num; i++) 
    {
     uint16 tmp;

     tmp = rhword(tPC);
     //   lowB   = mem_rbyte(tPC);
     //   highB  = mem_rbyte(tPC+1);
     lowB = tmp & 0xFF;
     highB = tmp >> 8;

     tmp = rhword(tPC + 2);
     //   lowB2  = mem_rbyte(tPC+2);
     //   highB2 = mem_rbyte(tPC+3);
     lowB2 = tmp & 0xFF;
     highB2 = tmp >> 8;

        opcode = highB >> 2;
        if((highB & 0xE0) == 0x80)        // Special opcode format for          
            opcode = (highB >> 1);            // type III instructions.
    
        if((opcode > 0x4F) | (opcode < 0)) {
            //Error Invalid opcode!
            sprintf(&buf[strlen(buf)],"0x%2x 0x%2x  ;Invalid Opcode", lowB, highB);
            tPC += 2;                                               
        }
        
        switch(optable[opcode].addr_mode) {
        case AM_I:       // Do the same Ither way =)
            arg1 = (lowB >> 5) + ((highB & 0x3) << 3);
            arg2 = (lowB & 0x1F);
            if (opcode == JMP) {
                sprintf(&buf[strlen(buf)],"%s    [%s]", optable[opcode].opname, pretty_preg_names[arg2]);
            } else {
                sprintf(&buf[strlen(buf)],"%s    %s, %s", optable[opcode].opname, pretty_preg_names[arg2], pretty_preg_names[arg1]);
            }
            tPC += 2;   // 16 bit instruction
            break;
        case AM_II:
            arg1 = (lowB >> 5) + ((highB & 0x3) << 3);
            arg2 = (lowB & 0x1F);
			if(opcode == LDSR) {
				sprintf(&buf[strlen(buf)],"%s    %s, %s", optable[opcode].opname, pretty_preg_names[arg1], pretty_sreg_names[arg2]);
			} else if(opcode == STSR) {
				sprintf(&buf[strlen(buf)],"%s     %s, %s", optable[opcode].opname, pretty_sreg_names[arg2], pretty_preg_names[arg1]);
			} else if(opcode == ADD_I || opcode == CMP_I) {
				sprintf(&buf[strlen(buf)],"%s    %d, %s", optable[opcode].opname, sign_5(arg2), pretty_preg_names[arg1]);
			} else {
                                sprintf(&buf[strlen(buf)],"%s    %d, %s", optable[opcode].opname, arg2, pretty_preg_names[arg1]);
			}
            tPC += 2;   // 16 bit instruction
            break;
        case AM_III:
            arg1 = ((highB & 0x1) << 8) + (lowB & 0xFE);
	    if(opcode == NOP)
             sprintf(&buf[strlen(buf)],"%s", optable[opcode].opname);
	    else
             sprintf(&buf[strlen(buf)],"%s    %08x", optable[opcode].opname, tPC + sign_9(arg1));
            tPC += 2;   // 16 bit instruction
            break;
        case AM_IV:
            arg1 = ((highB & 0x3) << 24) + (lowB << 16) + (highB2 << 8) + lowB2;

            sprintf(&buf[strlen(buf)],"%s    %08x", optable[opcode].opname, tPC + sign_26(arg1));

            tPC += 4;                                               // 32 bit instruction
            break;
        case AM_V:       
            arg1 = (lowB >> 5) + ((highB & 0x3) << 3);
            arg2 = (lowB & 0x1F);
            arg3 = (highB2 << 8) + lowB2;

	    // TODO: What would be the best way to disassemble the MOVEA instruction?
	    //if(opcode == MOVEA)
            // sprintf(&buf[strlen(buf)],"%s    0x%X, %s, %s", optable[opcode].opname, (uint32)(int32)(int16)(uint16)arg3, pretty_preg_names[arg2], pretty_preg_names[arg1] );
	    //else
            sprintf(&buf[strlen(buf)],"%s    0x%X, %s, %s", optable[opcode].opname, arg3, pretty_preg_names[arg2], pretty_preg_names[arg1] );
            tPC += 4;   // 32 bit instruction
            break;
        case AM_VIa:  // Mode6 form1
            arg1 = (lowB >> 5) + ((highB & 0x3) << 3);
            arg2 = (lowB & 0x1F);
            arg3 = (highB2 << 8) + lowB2;

	    if(!arg3) // Don't bother printing offset if it's 0
             sprintf(&buf[strlen(buf)],"%s    [%s], %s", optable[opcode].opname, pretty_preg_names[arg2], pretty_preg_names[arg1]);
	    else if(sign_16(arg3) >= 0) // Make disassembly prettier if it's a positive offset
             sprintf(&buf[strlen(buf)],"%s    0x%04x[%s], %s", optable[opcode].opname, sign_16(arg3), pretty_preg_names[arg2], pretty_preg_names[arg1]);
	    else
             sprintf(&buf[strlen(buf)],"%s    %d[%s], %s", optable[opcode].opname, sign_16(arg3), pretty_preg_names[arg2], pretty_preg_names[arg1]);

            tPC += 4;   // 32 bit instruction
            break;
        case AM_VIb:  // Mode6 form2
            arg1 = (lowB >> 5) + ((highB & 0x3) << 3);
            arg2 = (lowB & 0x1F);
            arg3 = (highB2 << 8) + lowB2;                              //  whats the order??? 2,3,1 or 1,3,2

	    if(!arg3) // Don't bother printing offset if it's 0
             sprintf(&buf[strlen(buf)],"%s    %s, [%s]", optable[opcode].opname, pretty_preg_names[arg1], pretty_preg_names[arg2]);
            else if(sign_16(arg3) >= 0) // Make disassembly prettier if it's a positive offset
             sprintf(&buf[strlen(buf)],"%s    %s, 0x%04x[%s]", optable[opcode].opname, pretty_preg_names[arg1], sign_16(arg3), pretty_preg_names[arg2]);
	    else
             sprintf(&buf[strlen(buf)],"%s    %s, %d[%s]", optable[opcode].opname, pretty_preg_names[arg1], sign_16(arg3), pretty_preg_names[arg2]);
            tPC += 4;   // 32 bit instruction
            break;
        case AM_VII:   // Unhandled
            sprintf(&buf[strlen(buf)],"0x%2x 0x%2x 0x%2x 0x%2x", lowB, highB, lowB2, highB2);
            tPC +=4;        // 32 bit instruction
            break;
        case AM_VIII:  // Unhandled
            sprintf(&buf[strlen(buf)],"0x%2x 0x%2x 0x%2x 0x%2x", lowB, highB, lowB2, highB2);
            tPC += 4;   // 32 bit instruction
            break;
        case AM_IX:
            arg1 = (lowB & 0x1); // Mode ID, Ignore for now
            sprintf(&buf[strlen(buf)],"%s", optable[opcode].opname);   
            tPC += 2;   // 16 bit instruction
            break;
        case AM_BSTR:  // Bit String Subopcodes
            arg1 = (lowB >> 5) + ((highB & 0x3) << 3);
            arg2 = (lowB & 0x1F);
            if(arg2 > 15) {
                sprintf(&buf[strlen(buf)],"BError");
            } else {
                //sprintf(&buf[strlen(buf)],"%s, $%d", bssuboptable[arg2].opname,arg1);
		sprintf(&buf[strlen(buf)], "%s", bssuboptable[arg2].opname);
            }
            tPC += 2;   // 16 bit instruction
            break;
        case AM_FPP:   // Floating Point Subcode
            arg1 = (lowB >> 5) + ((highB & 0x3) << 3);
            arg2 = (lowB & 0x1F);
            arg3 = (highB2 >> 2);

            if(arg3 > 15) {
                sprintf(&buf[strlen(buf)],"(Invalid FPU: 0x%02x)", arg3);
            } else {
                sprintf(&buf[strlen(buf)],"%s  %s, %s", fpsuboptable[arg3].opname, pretty_preg_names[arg2], pretty_preg_names[arg1]);
            }
            tPC += 4;   // 32 bit instruction
            break;
        case AM_UDEF:  // Invalid opcode.
        default:       // Invalid opcode.
            sprintf(&buf[strlen(buf)],"0x%2x 0x%2x  ;Invalid Opcode", lowB, highB);
            tPC += 2;                                               
        }
    }
}
