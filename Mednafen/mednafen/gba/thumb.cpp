// -*- C++ -*-
// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "GBA.h"
#include "GBAinline.h"
#include "Globals.h"

unsigned int RunTHUMB(void)
{
#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)
#define ADDCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & NEG(b)) |\
            (NEG(a) & POS(c)) |\
            (NEG(b) & POS(c))) ? true : false;
#define ADDOVERFLOW(a, b, c) \
  V_FLAG = ((NEG(a) & NEG(b) & POS(c)) |\
            (POS(a) & POS(b) & NEG(c))) ? true : false;
#define SUBCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & POS(b)) |\
            (NEG(a) & POS(c)) |\
            (POS(b) & POS(c))) ? true : false;
#define SUBOVERFLOW(a, b, c)\
  V_FLAG = ((NEG(a) & POS(b) & POS(c)) |\
            (POS(a) & NEG(b) & NEG(c))) ? true : false;
#define ADD_RD_RS_RN \
   {\
     uint32 lhs = reg[source].I;\
     uint32 rhs = value;\
     uint32 res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADD_RD_RS_O3 \
   {\
     uint32 lhs = reg[source].I;\
     uint32 rhs = value;\
     uint32 res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADD_RN_O8(d) \
   {\
     uint32 lhs = reg[(d)].I;\
     uint32 rhs = (opcode & 255);\
     uint32 res = lhs + rhs;\
     reg[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define CMN_RD_RS \
   {\
     uint32 lhs = reg[dest].I;\
     uint32 rhs = value;\
     uint32 res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADC_RD_RS \
   {\
     uint32 lhs = reg[dest].I;\
     uint32 rhs = value;\
     uint32 res = lhs + rhs + (uint32)C_FLAG;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RD_RS_RN \
   {\
     uint32 lhs = reg[source].I;\
     uint32 rhs = value;\
     uint32 res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RD_RS_O3 \
   {\
     uint32 lhs = reg[source].I;\
     uint32 rhs = value;\
     uint32 res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RN_O8(d) \
   {\
     uint32 lhs = reg[(d)].I;\
     uint32 rhs = (opcode & 255);\
     uint32 res = lhs - rhs;\
     reg[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define CMP_RN_O8(d) \
   {\
     uint32 lhs = reg[(d)].I;\
     uint32 rhs = (opcode & 255);\
     uint32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SBC_RD_RS \
   {\
     uint32 lhs = reg[dest].I;\
     uint32 rhs = value;\
     uint32 res = lhs - rhs - !((uint32)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define LSL_RD_RM_I5 \
   {\
     C_FLAG = (reg[source].I >> (32 - shift)) & 1 ? true : false;\
     value = reg[source].I << shift;\
   }
#define LSL_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (32 - value)) & 1 ? true : false;\
     value = reg[dest].I << value;\
   }
#define LSR_RD_RM_I5 \
   {\
     C_FLAG = (reg[source].I >> (shift - 1)) & 1 ? true : false;\
     value = reg[source].I >> shift;\
   }
#define LSR_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false;\
     value = reg[dest].I >> value;\
   }
#define ASR_RD_RM_I5 \
   {\
     C_FLAG = ((int32)reg[source].I >> (int)(shift - 1)) & 1 ? true : false;\
     value = (int32)reg[source].I >> (int)shift;\
   }
#define ASR_RD_RS \
   {\
     C_FLAG = ((int32)reg[dest].I >> (int)(value - 1)) & 1 ? true : false;\
     value = (int32)reg[dest].I >> (int)value;\
   }
#define ROR_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false;\
     value = ((reg[dest].I << (32 - value)) |\
              (reg[dest].I >> value));\
   }
#define NEG_RD_RS \
   {\
     uint32 lhs = reg[source].I;\
     uint32 rhs = 0;\
     uint32 res = rhs - lhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define CMP_RD_RS \
   {\
     uint32 lhs = reg[dest].I;\
     uint32 rhs = value;\
     uint32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }

uint32 opcode = cpuPrefetch[0];
cpuPrefetch[0] = cpuPrefetch[1];

busPrefetch = false;
  if (busPrefetchCount<0)
      busPrefetchCount=0;
  if (busPrefetchCount>16)
      busPrefetchCount=8;

unsigned int clockTicks = codeTicksAccessSeq16(armNextPC)+1;

armNextPC = reg[15].I;
reg[15].I += 2;
THUMB_PREFETCH_NEXT;

switch(opcode >> 8) {
 case 0x00:
 case 0x01:
 case 0x02:
 case 0x03:
 case 0x04:
 case 0x05:
 case 0x06:
 case 0x07:
   {
     // LSL Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     uint32 value;
     
     if(shift) {
       LSL_RD_RM_I5;
     } else {
       value = reg[source].I;
     }
     reg[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false : true);
   }
   break;
 case 0x08:
 case 0x09:
 case 0x0a:
 case 0x0b:
 case 0x0c:
 case 0x0d:
 case 0x0e:
 case 0x0f:
   {
     // LSR Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     uint32 value;
     
     if(shift) {
       LSR_RD_RM_I5;
     } else {
       C_FLAG = reg[source].I & 0x80000000 ? true : false;
       value = 0;
     }
     reg[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false : true);
   }
   break;
 case 0x10:
 case 0x11:
 case 0x12:
 case 0x13:
 case 0x14:
 case 0x15:
 case 0x16:
 case 0x17:
   {     
     // ASR Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     uint32 value;
     
     if(shift) {
       ASR_RD_RM_I5;
     } else {
       if(reg[source].I & 0x80000000) {
         value = 0xFFFFFFFF;
         C_FLAG = true;
       } else {
         value = 0;
         C_FLAG = false;
       }
     }
     reg[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false :true);
   }
   break;
 case 0x18:
 case 0x19:
   {
     // ADD Rd, Rs, Rn
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     uint32 value = reg[(opcode>>6)& 0x07].I;
     ADD_RD_RS_RN;
   }
   break;
 case 0x1a:
 case 0x1b:
   {
     // SUB Rd, Rs, Rn
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     uint32 value = reg[(opcode>>6)& 0x07].I;
     SUB_RD_RS_RN;
   }
   break;
 case 0x1c:
 case 0x1d:
   {
     // ADD Rd, Rs, #Offset3
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     uint32 value = (opcode >> 6) & 7;
     ADD_RD_RS_O3;
   }
   break;
 case 0x1e:
 case 0x1f:
   {
     // SUB Rd, Rs, #Offset3
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     uint32 value = (opcode >> 6) & 7;
     SUB_RD_RS_O3;
   }
   break;
 case 0x20:
   // MOV R0, #Offset8
   reg[0].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[0].I ? false : true);
   break;
 case 0x21:
   // MOV R1, #Offset8
   reg[1].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[1].I ? false : true);
   break;   
 case 0x22:
   // MOV R2, #Offset8
   reg[2].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[2].I ? false : true);
   break;   
 case 0x23:
   // MOV R3, #Offset8
   reg[3].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[3].I ? false : true);
   break;   
 case 0x24:
   // MOV R4, #Offset8
   reg[4].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[4].I ? false : true);
   break;   
 case 0x25:
   // MOV R5, #Offset8
   reg[5].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[5].I ? false : true);
   break;   
 case 0x26:
   // MOV R6, #Offset8
   reg[6].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[6].I ? false : true);
   break;   
 case 0x27:
   // MOV R7, #Offset8
   reg[7].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[7].I ? false : true);
   break;
 case 0x28:
   // CMP R0, #Offset8
   CMP_RN_O8(0);
   break;
 case 0x29:
   // CMP R1, #Offset8
   CMP_RN_O8(1);
   break;
 case 0x2a:
   // CMP R2, #Offset8
   CMP_RN_O8(2);
   break;
 case 0x2b:
   // CMP R3, #Offset8
   CMP_RN_O8(3);
   break;
 case 0x2c:
   // CMP R4, #Offset8
   CMP_RN_O8(4);
   break;
 case 0x2d:
   // CMP R5, #Offset8
   CMP_RN_O8(5);
   break;
 case 0x2e:
   // CMP R6, #Offset8
   CMP_RN_O8(6);
   break;
 case 0x2f:
   // CMP R7, #Offset8
   CMP_RN_O8(7);
   break;
 case 0x30:
   // ADD R0,#Offset8
   ADD_RN_O8(0);
   break;   
 case 0x31:
   // ADD R1,#Offset8
   ADD_RN_O8(1);
   break;   
 case 0x32:
   // ADD R2,#Offset8
   ADD_RN_O8(2);
   break;   
 case 0x33:
   // ADD R3,#Offset8
   ADD_RN_O8(3);
   break;   
 case 0x34:
   // ADD R4,#Offset8
   ADD_RN_O8(4);
   break;   
 case 0x35:
   // ADD R5,#Offset8
   ADD_RN_O8(5);
   break;   
 case 0x36:
   // ADD R6,#Offset8
   ADD_RN_O8(6);
   break;   
 case 0x37:
   // ADD R7,#Offset8
   ADD_RN_O8(7);
   break;
 case 0x38:
   // SUB R0,#Offset8
   SUB_RN_O8(0);
   break;
 case 0x39:
   // SUB R1,#Offset8
   SUB_RN_O8(1);
   break;
 case 0x3a:
   // SUB R2,#Offset8
   SUB_RN_O8(2);
   break;
 case 0x3b:
   // SUB R3,#Offset8
   SUB_RN_O8(3);
   break;
 case 0x3c:
   // SUB R4,#Offset8
   SUB_RN_O8(4);
   break;
 case 0x3d:
   // SUB R5,#Offset8
   SUB_RN_O8(5);
   break;
 case 0x3e:
   // SUB R6,#Offset8
   SUB_RN_O8(6);
   break;
 case 0x3f:
   // SUB R7,#Offset8
   SUB_RN_O8(7);
   break;
 case 0x40:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // AND Rd, Rs
       int dest = opcode & 7;
       reg[dest].I &= reg[(opcode >> 3)&7].I;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
#ifdef BKPT_SUPPORT     
#define THUMB_CONSOLE_OUTPUT(a,b) \
  if((opcode == 0x4000) && (reg[0].I == 0xC0DED00D)) {\
    extern void (*dbgOutput)(char *, uint32);\
    dbgOutput((a), (b));\
  }
#else
#define THUMB_CONSOLE_OUTPUT(a,b)
#endif
       THUMB_CONSOLE_OUTPUT(NULL, reg[2].I);
     }
     break;
   case 0x01:
     // EOR Rd, Rs
     {
       int dest = opcode & 7;
       reg[dest].I ^= reg[(opcode >> 3)&7].I;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
     }
     break;
   case 0x02:
     // LSL Rd, Rs
     {
       int dest = opcode & 7;
       uint32 value = reg[(opcode >> 3)&7].B.B0;
       if(value) {
         if(value == 32) {
           value = 0;
           C_FLAG = (reg[dest].I & 1 ? true : false);
         } else if(value < 32) {
           LSL_RD_RS;
         } else {
           value = 0;
           C_FLAG = false;
         }
         reg[dest].I = value;        
       }
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
       clockTicks = codeTicksAccesint16(armNextPC)+2;
     }
     break;
   case 0x03:
     {
       // LSR Rd, Rs
       int dest = opcode & 7;
       uint32 value = reg[(opcode >> 3)&7].B.B0;
       if(value) {
         if(value == 32) {
           value = 0;
           C_FLAG = (reg[dest].I & 0x80000000 ? true : false);
         } else if(value < 32) {
           LSR_RD_RS;
         } else {
           value = 0;
           C_FLAG = false;
         }
         reg[dest].I = value;        
       }
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
       clockTicks = codeTicksAccesint16(armNextPC)+2;
     }
     break;
   }
   break;
 case 0x41:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // ASR Rd, Rs
       int dest = opcode & 7;
       uint32 value = reg[(opcode >> 3)&7].B.B0;
       // ASR
       if(value) {
         if(value < 32) {
           ASR_RD_RS;
           reg[dest].I = value;        
         } else {
           if(reg[dest].I & 0x80000000){
             reg[dest].I = 0xFFFFFFFF;
             C_FLAG = true;
           } else {
             reg[dest].I = 0x00000000;
             C_FLAG = false;
           }
         }
       }
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
       clockTicks = codeTicksAccesint16(armNextPC)+2;
     }
     break;
   case 0x01:
     {
       // ADC Rd, Rs
       int dest = opcode & 0x07;
       uint32 value = reg[(opcode >> 3)&7].I;
       // ADC
       ADC_RD_RS;
     }
     break;
   case 0x02:
     {
       // SBC Rd, Rs
       int dest = opcode & 0x07;
       uint32 value = reg[(opcode >> 3)&7].I;
       
       // SBC
       SBC_RD_RS;
     }
     break;
   case 0x03:
     // ROR Rd, Rs
     {
       int dest = opcode & 7;
       uint32 value = reg[(opcode >> 3)&7].B.B0;
       
       if(value) {
         value = value & 0x1f;
         if(value == 0) {
           C_FLAG = (reg[dest].I & 0x80000000 ? true : false);
         } else {
           ROR_RD_RS;
           reg[dest].I = value;
         }
       }
       clockTicks = codeTicksAccesint16(armNextPC)+2;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
     }
     break;
   }
   break;
 case 0x42:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // TST Rd, Rs
       uint32 value = reg[opcode & 7].I & reg[(opcode >> 3) & 7].I;
       N_FLAG = value & 0x80000000 ? true : false;
       Z_FLAG = value ? false : true;
     }
     break;
   case 0x01:
     {
       // NEG Rd, Rs
       int dest = opcode & 7;
       int source = (opcode >> 3) & 7;
       NEG_RD_RS;
     }
     break;
   case 0x02:
     {
       // CMP Rd, Rs
       int dest = opcode & 7;
       uint32 value = reg[(opcode >> 3)&7].I;
       CMP_RD_RS;
     }
     break;
   case 0x03:
     {
       // CMN Rd, Rs
       int dest = opcode & 7;
       uint32 value = reg[(opcode >> 3)&7].I;
       // CMN
       CMN_RD_RS;
     }
     break;
   }
   break;
 case 0x43:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // ORR Rd, Rs       
       int dest = opcode & 7;
       reg[dest].I |= reg[(opcode >> 3) & 7].I;
       Z_FLAG = reg[dest].I ? false : true;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x01:
     {
       // MUL Rd, Rs
       clockTicks = codeTicksAccesint16(armNextPC)+2;
       int dest = opcode & 7;
       uint32 rm = reg[dest].I;
       reg[dest].I = reg[(opcode >> 3) & 7].I * rm;
       if (((int32)rm) < 0)
         rm = ~rm;
       if ((rm & 0xFFFFFF00) == 0)
         clockTicks += 0;
       else if ((rm & 0xFFFF0000) == 0)
         clockTicks += 1;
       else if ((rm & 0xFF000000) == 0)
         clockTicks += 2;
       else
         clockTicks += 3;
       busPrefetchCount += clockTicks - codeTicksAccesint16(armNextPC) -1;
       Z_FLAG = reg[dest].I ? false : true;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x02:
     {
       // BIC Rd, Rs
       int dest = opcode & 7;
       reg[dest].I &= (~reg[(opcode >> 3) & 7].I);
       Z_FLAG = reg[dest].I ? false : true;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x03:
     {
       // MVN Rd, Rs
       int dest = opcode & 7;
       reg[dest].I = ~reg[(opcode >> 3) & 7].I;
       Z_FLAG = reg[dest].I ? false : true;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
     }
     break;
   }
   break;
 case 0x44:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     switch((opcode >> 6)& 3) {
     default:
       goto unknown_thumb;
     case 1:
       // ADD Rd, Hs
       reg[dest].I += reg[base+8].I;
       break;
     case 2:
       // ADD Hd, Rs
       reg[dest+8].I += reg[base].I;
       if(dest == 7) {
         reg[15].I &= 0xFFFFFFFE;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         THUMB_PREFETCH;
         clockTicks += clockTicks+codeTicksAccesint16(armNextPC)+1;
       }       
       break;
     case 3:
       // ADD Hd, Hs
       reg[dest+8].I += reg[base+8].I;
       if(dest == 7) {
         reg[15].I &= 0xFFFFFFFE;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         THUMB_PREFETCH;
         clockTicks += clockTicks+codeTicksAccesint16(armNextPC)+1;     
       }
       break;
     }
   }
   break;
 case 0x45:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     uint32 value;
     switch((opcode >> 6) & 3) {
     case 0:
       // CMP Rd, Hs
       value = reg[base].I;
       CMP_RD_RS;
       break;
     case 1:
       // CMP Rd, Hs
       value = reg[base+8].I;
       CMP_RD_RS;
       break;
     case 2:
       // CMP Hd, Rs
       value = reg[base].I;
       dest += 8;
       CMP_RD_RS;
       break;
     case 3:
       // CMP Hd, Hs
       value = reg[base+8].I;
       dest += 8;
       CMP_RD_RS;
       break;
     }
   }
   break;
 case 0x46:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     switch((opcode >> 6) & 3) {
     case 0:
       // this form should not be used...
       // MOV Rd, Rs
       reg[dest].I = reg[base].I;
       break;
     case 1:
       // MOV Rd, Hs
       reg[dest].I = reg[base+8].I;
       break;
     case 2:
       // MOV Hd, Rs
       reg[dest+8].I = reg[base].I;
       if(dest == 7) {
         reg[15].I &= 0xFFFFFFFE;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         THUMB_PREFETCH;
         clockTicks += clockTicks+codeTicksAccesint16(armNextPC)+1; 
       }
       break;
     case 3:
       // MOV Hd, Hs
       reg[dest+8].I = reg[base+8].I;
       if(dest == 7) {
         reg[15].I &= 0xFFFFFFFE;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         THUMB_PREFETCH;
         clockTicks += clockTicks+codeTicksAccesint16(armNextPC)+1; 
       }   
       break;
     }
   }
   break;
 case 0x47:
   {
     int base = (opcode >> 3) & 7;
     switch((opcode >>6) & 3) {
     case 0:
       // BX Rs
       reg[15].I = (reg[base].I) & 0xFFFFFFFE;
       if(reg[base].I & 1) {
         armState = false;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         THUMB_PREFETCH;
       } else {
         armState = true;
         reg[15].I &= 0xFFFFFFFC;
         armNextPC = reg[15].I;
         reg[15].I += 4;
         ARM_PREFETCH;
       }
       busPrefetchCount=0;
       clockTicks += clockTicks+codeTicksAccesint16(armNextPC)+1; 
       break;
     case 1:
       // BX Hs
       reg[15].I = (reg[8+base].I) & 0xFFFFFFFE;
       if(reg[8+base].I & 1) {
         armState = false;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         THUMB_PREFETCH;
       } else {
         armState = true;
         reg[15].I &= 0xFFFFFFFC;       
         armNextPC = reg[15].I;
         reg[15].I += 4;
         ARM_PREFETCH;
       }
       busPrefetchCount=0;
       clockTicks += clockTicks+codeTicksAccesint16(armNextPC)+1; 
       break;
     default:
       goto unknown_thumb;
     }
   }
   break;
 case 0x48:
   // LDR R0,[PC, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);
     reg[0].I = CPUReadMemoryQuick(address);
     busPrefetchCount=0;
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x49:
   // LDR R1,[PC, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);   
     reg[1].I = CPUReadMemoryQuick(address);
       busPrefetchCount=0;
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x4a:
   // LDR R2,[PC, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
     reg[2].I = CPUReadMemoryQuick(address);
     busPrefetchCount=0;
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x4b:
   // LDR R3,[PC, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
     reg[3].I = CPUReadMemoryQuick(address);
     busPrefetchCount=0;
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x4c:
   // LDR R4,[PC, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
     reg[4].I = CPUReadMemoryQuick(address);
     busPrefetchCount=0;
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x4d:
   // LDR R5,[PC, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
     reg[5].I = CPUReadMemoryQuick(address);
     busPrefetchCount=0;
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x4e:
   // LDR R6,[PC, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
     reg[6].I = CPUReadMemoryQuick(address);
     busPrefetchCount=0;
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x4f:
   // LDR R7,[PC, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);      
     reg[7].I = CPUReadMemoryQuick(address);
     busPrefetchCount=0;
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x50:
 case 0x51:
   // STR Rd, [Rs, Rn]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
     CPUWriteMemory(address,
                    reg[opcode & 7].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address) +2;
   }
   break;
 case 0x52:
 case 0x53:
   // STRH Rd, [Rs, Rn]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
     CPUWriteHalfWord(address,
                      reg[opcode&7].W.W0);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint16(address) +2;
   }
   break;
 case 0x54:
 case 0x55:
   // STRB Rd, [Rs, Rn]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + reg[(opcode >>6)&7].I;
     CPUWriteByte(address,
                  reg[opcode & 7].B.B0);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint16(address) +2;
   }
   break;
 case 0x56:
 case 0x57:
   // LDSB Rd, [Rs, Rn]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
     reg[opcode&7].I = (int8)CPUReadByte(address);
     clockTicks = 3 + dataTicksAccesint16(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x58:
 case 0x59:
   // LDR Rd, [Rs, Rn]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
     reg[opcode&7].I = CPUReadMemory(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x5a:
 case 0x5b:
   // LDRH Rd, [Rs, Rn]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
     reg[opcode&7].I = CPUReadHalfWord(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x5c:
 case 0x5d:
   // LDRB Rd, [Rs, Rn]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
     reg[opcode&7].I = CPUReadByte(address);
     clockTicks = 3 + dataTicksAccesint16(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x5e:
 case 0x5f:
   // LDSH Rd, [Rs, Rn]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I;
     reg[opcode&7].I = (int16)CPUReadHalfWordSigned(address);
     clockTicks = 3 + dataTicksAccesint16(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x60:
 case 0x61:
 case 0x62:
 case 0x63:
 case 0x64:
 case 0x65:
 case 0x66:
 case 0x67:
   // STR Rd, [Rs, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
     CPUWriteMemory(address,
                    reg[opcode&7].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2;
   }
   break;
 case 0x68:
 case 0x69:
 case 0x6a:
 case 0x6b:
 case 0x6c:
 case 0x6d:
 case 0x6e:
 case 0x6f:
   // LDR Rd, [Rs, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
     reg[opcode&7].I = CPUReadMemory(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x70:
 case 0x71:
 case 0x72:
 case 0x73:
 case 0x74:
 case 0x75:
 case 0x76:
 case 0x77:
   // STRB Rd, [Rs, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + (((opcode>>6)&31));
     CPUWriteByte(address,
                  reg[opcode&7].B.B0);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint16(address)+2;
   }
   break;
 case 0x78:
 case 0x79:
 case 0x7a:
 case 0x7b:
 case 0x7c:
 case 0x7d:
 case 0x7e:
 case 0x7f:
   // LDRB Rd, [Rs, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + (((opcode>>6)&31));
     reg[opcode&7].I = CPUReadByte(address);
     clockTicks = 3 + dataTicksAccesint16(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x80:
 case 0x81:
 case 0x82:
 case 0x83:
 case 0x84:
 case 0x85:
 case 0x86:
 case 0x87:
   // STRH Rd, [Rs, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
     CPUWriteHalfWord(address,
                      reg[opcode&7].W.W0);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint16(address)+2;
   }
   break;   
 case 0x88:
 case 0x89:
 case 0x8a:
 case 0x8b:
 case 0x8c:
 case 0x8d:
 case 0x8e:
 case 0x8f:
   // LDRH Rd, [Rs, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
     reg[opcode&7].I = CPUReadHalfWord(address);
     clockTicks = 3 + dataTicksAccesint16(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x90:
   // STR R0, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);
     CPUWriteMemory(address, reg[0].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2;
   }
   break;      
 case 0x91:
   // STR R1, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     CPUWriteMemory(address, reg[1].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2; 
   }
   break;      
 case 0x92:
   // STR R2, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     CPUWriteMemory(address, reg[2].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2;
   }
   break;      
 case 0x93:
   // STR R3, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     CPUWriteMemory(address, reg[3].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2;
   }
   break;      
 case 0x94:
   // STR R4, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     CPUWriteMemory(address, reg[4].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2;
   }
   break;      
 case 0x95:
   // STR R5, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     CPUWriteMemory(address, reg[5].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2;
   }
   break;      
 case 0x96:
   // STR R6, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     CPUWriteMemory(address, reg[6].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2;
   }
   break;      
 case 0x97:
   // STR R7, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     CPUWriteMemory(address, reg[7].I);
     clockTicks = codeTicksAccesint16(armNextPC) +
         dataTicksAccesint32(address)+2;
   }
   break;   
 case 0x98:
   // LDR R0, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     reg[0].I = CPUReadMemoryQuick(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x99:
   // LDR R1, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     reg[1].I = CPUReadMemoryQuick(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x9a:
   // LDR R2, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     reg[2].I = CPUReadMemoryQuick(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC); 
   }
   break;
 case 0x9b:
   // LDR R3, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     reg[3].I = CPUReadMemoryQuick(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x9c:
   // LDR R4, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     reg[4].I = CPUReadMemoryQuick(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x9d:
   // LDR R5, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     reg[5].I = CPUReadMemoryQuick(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x9e:
   // LDR R6, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     reg[6].I = CPUReadMemoryQuick(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0x9f:
   // LDR R7, [SP, #Imm]
   {
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[13].I + ((opcode&255)<<2);   
     reg[7].I = CPUReadMemoryQuick(address);
     clockTicks = 3 + dataTicksAccesint32(address) +
         codeTicksAccesint16(armNextPC);
   }
   break;
 case 0xa0:
   // ADD R0, PC, Imm
   reg[0].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa1:
   // ADD R1, PC, Imm
   reg[1].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa2:
   // ADD R2, PC, Imm
   reg[2].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa3:
   // ADD R3, PC, Imm
   reg[3].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa4:
   // ADD R4, PC, Imm
   reg[4].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa5:
   // ADD R5, PC, Imm
   reg[5].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa6:
   // ADD R6, PC, Imm
   reg[6].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa7:
   // ADD R7, PC, Imm
   reg[7].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa8:
   // ADD R0, SP, Imm
   reg[0].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xa9:
   // ADD R1, SP, Imm
   reg[1].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xaa:
   // ADD R2, SP, Imm
   reg[2].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xab:
   // ADD R3, SP, Imm
   reg[3].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xac:
   // ADD R4, SP, Imm
   reg[4].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xad:
   // ADD R5, SP, Imm
   reg[5].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xae:
   // ADD R6, SP, Imm
   reg[6].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xaf:
   // ADD R7, SP, Imm
   reg[7].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xb0:
   {
     // ADD SP, Imm
     int offset = (opcode & 127) << 2;
     if(opcode & 0x80)
       offset = -offset;
     reg[13].I += offset;
   }
   break;
#define PUSH_REG(val, r) \
  if(opcode & (val)) {\
    CPUWriteMemory(address, reg[(r)].I);\
    if(offset)\
      clockTicks += 1 + dataTicksAccessSeq32(address);\
    else\
      clockTicks += 1 + dataTicksAccesint32(address);\
    offset = 1;\
    address += 4;\
  }
 case 0xb4:
   // PUSH {Rlist}
   {
     busPrefetch = busPrefetchEnable;
     int offset = 0;
     uint32 temp = reg[13].I - 4 * cpuBitsSet[opcode & 0xff];
     uint32 address = temp & 0xFFFFFFFC;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     PUSH_REG(1, 0);
     PUSH_REG(2, 1);
     PUSH_REG(4, 2);
     PUSH_REG(8, 3);
     PUSH_REG(16, 4);
     PUSH_REG(32, 5);
     PUSH_REG(64, 6);
     PUSH_REG(128, 7);
     reg[13].I = temp;
   }
   break;
 case 0xb5:
   // PUSH {Rlist, LR}
   {
     busPrefetch = busPrefetchEnable;
     int offset = 0;
     uint32 temp = reg[13].I - 4 - 4 * cpuBitsSet[opcode & 0xff];
     uint32 address = temp & 0xFFFFFFFC;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     PUSH_REG(1, 0);
     PUSH_REG(2, 1);
     PUSH_REG(4, 2);
     PUSH_REG(8, 3);
     PUSH_REG(16, 4);
     PUSH_REG(32, 5);
     PUSH_REG(64, 6);
     PUSH_REG(128, 7);
     PUSH_REG(256, 14);
     reg[13].I = temp;
   }
   break;
#define POP_REG(val, r) \
  if(opcode & (val)) {\
    reg[(r)].I = CPUReadMemory(address);\
    if(offset)\
      clockTicks += 1 + dataTicksAccessSeq32(address);\
    else\
      clockTicks += 1 + dataTicksAccesint32(address);\
    offset = 1;\
    address += 4;\
  }
 case 0xbc:
   // POP {Rlist}
   {
     busPrefetch = busPrefetchEnable;
     int offset = 0;
     uint32 address = reg[13].I & 0xFFFFFFFC;
     uint32 temp = reg[13].I + 4*cpuBitsSet[opcode & 0xFF];
     clockTicks = 0;
     POP_REG(1, 0);
     POP_REG(2, 1);
     POP_REG(4, 2);
     POP_REG(8, 3);
     POP_REG(16, 4);
     POP_REG(32, 5);
     POP_REG(64, 6);
     POP_REG(128, 7);
     reg[13].I = temp;
     clockTicks += codeTicksAccesint16(armNextPC)+2;
   }
   break;   
 case 0xbd:
   // POP {Rlist, PC}
   {
     busPrefetch = busPrefetchEnable;
     int offset = 0;
     uint32 address = reg[13].I & 0xFFFFFFFC;
     uint32 temp = reg[13].I + 4 + 4*cpuBitsSet[opcode & 0xFF];
     clockTicks = 0;
     POP_REG(1, 0);
     POP_REG(2, 1);
     POP_REG(4, 2);
     POP_REG(8, 3);
     POP_REG(16, 4);
     POP_REG(32, 5);
     POP_REG(64, 6);
     POP_REG(128, 7);
     reg[15].I = (CPUReadMemory(address) & 0xFFFFFFFE);
     if(offset)
       clockTicks += 1 + dataTicksAccessSeq32(address);
     else
       clockTicks += 1 + dataTicksAccesint32(address);
     armNextPC = reg[15].I;
     reg[15].I += 2;
     reg[13].I = temp;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks += 2*(codeTicksAccesint16(armNextPC))+3;
   }
   break;      
#define THUMB_STM_REG(val,r,b) \
  if(opcode & (val)) {\
    CPUWriteMemory(address, reg[(r)].I);\
    if(!offset) {\
      reg[(b)].I = temp;\
      clockTicks += 1 + dataTicksAccesint32(address);\
    } else \
      clockTicks += 1 + dataTicksAccessSeq32(address);\
    offset = 1;\
    address += 4;\
  }
 case 0xc0:
   {
     // STM R0!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[0].I & 0xFFFFFFFC;
     uint32 temp = reg[0].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     // store
     THUMB_STM_REG(1, 0, 0);
     THUMB_STM_REG(2, 1, 0);
     THUMB_STM_REG(4, 2, 0);
     THUMB_STM_REG(8, 3, 0);
     THUMB_STM_REG(16, 4, 0);
     THUMB_STM_REG(32, 5, 0);
     THUMB_STM_REG(64, 6, 0);
     THUMB_STM_REG(128, 7, 0);
   }
   break;   
 case 0xc1:
   {
     // STM R1!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[1].I & 0xFFFFFFFC;
     uint32 temp = reg[1].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     // store
     THUMB_STM_REG(1, 0, 1);
     THUMB_STM_REG(2, 1, 1);
     THUMB_STM_REG(4, 2, 1);
     THUMB_STM_REG(8, 3, 1);
     THUMB_STM_REG(16, 4, 1);
     THUMB_STM_REG(32, 5, 1);
     THUMB_STM_REG(64, 6, 1);
     THUMB_STM_REG(128, 7, 1);
   }
   break;      
 case 0xc2:
   {
     // STM R2!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[2].I & 0xFFFFFFFC;
     uint32 temp = reg[2].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     // store
     THUMB_STM_REG(1, 0, 2);
     THUMB_STM_REG(2, 1, 2);
     THUMB_STM_REG(4, 2, 2);
     THUMB_STM_REG(8, 3, 2);
     THUMB_STM_REG(16, 4, 2);
     THUMB_STM_REG(32, 5, 2);
     THUMB_STM_REG(64, 6, 2);
     THUMB_STM_REG(128, 7, 2);
   }
   break;      
 case 0xc3:
   {
     // STM R3!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[3].I & 0xFFFFFFFC;
     uint32 temp = reg[3].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     // store
     THUMB_STM_REG(1, 0, 3);
     THUMB_STM_REG(2, 1, 3);
     THUMB_STM_REG(4, 2, 3);
     THUMB_STM_REG(8, 3, 3);
     THUMB_STM_REG(16, 4, 3);
     THUMB_STM_REG(32, 5, 3);
     THUMB_STM_REG(64, 6, 3);
     THUMB_STM_REG(128, 7, 3);
   }
   break;   
 case 0xc4:
   {
     // STM R4!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[4].I & 0xFFFFFFFC;
     uint32 temp = reg[4].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     // store
     THUMB_STM_REG(1, 0, 4);
     THUMB_STM_REG(2, 1, 4);
     THUMB_STM_REG(4, 2, 4);
     THUMB_STM_REG(8, 3, 4);
     THUMB_STM_REG(16, 4, 4);
     THUMB_STM_REG(32, 5, 4);
     THUMB_STM_REG(64, 6, 4);
     THUMB_STM_REG(128, 7, 4);
   }
   break;   
 case 0xc5:
   {
     // STM R5!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[5].I & 0xFFFFFFFC;
     uint32 temp = reg[5].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     // store
     THUMB_STM_REG(1, 0, 5);
     THUMB_STM_REG(2, 1, 5);
     THUMB_STM_REG(4, 2, 5);
     THUMB_STM_REG(8, 3, 5);
     THUMB_STM_REG(16, 4, 5);
     THUMB_STM_REG(32, 5, 5);
     THUMB_STM_REG(64, 6, 5);
     THUMB_STM_REG(128, 7, 5);
   }
   break;   
 case 0xc6:
   {
     // STM R6!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[6].I & 0xFFFFFFFC;
     uint32 temp = reg[6].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     // store
     THUMB_STM_REG(1, 0, 6);
     THUMB_STM_REG(2, 1, 6);
     THUMB_STM_REG(4, 2, 6);
     THUMB_STM_REG(8, 3, 6);
     THUMB_STM_REG(16, 4, 6);
     THUMB_STM_REG(32, 5, 6);
     THUMB_STM_REG(64, 6, 6);
     THUMB_STM_REG(128, 7, 6);
   }
   break;   
 case 0xc7:
   {
     // STM R7!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[7].I & 0xFFFFFFFC;
     uint32 temp = reg[7].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     clockTicks = codeTicksAccesint16(armNextPC)+1;
     // store
     THUMB_STM_REG(1, 0, 7);
     THUMB_STM_REG(2, 1, 7);
     THUMB_STM_REG(4, 2, 7);
     THUMB_STM_REG(8, 3, 7);
     THUMB_STM_REG(16, 4, 7);
     THUMB_STM_REG(32, 5, 7);
     THUMB_STM_REG(64, 6, 7);
     THUMB_STM_REG(128, 7, 7);
   }
   break;
#define THUMB_LDM_REG(val,r) \
  if(opcode & (val)) {\
    reg[(r)].I = CPUReadMemory(address);\
    if(offset)\
      clockTicks += 1 + dataTicksAccessSeq32(address);\
    else \
      clockTicks += 1 + dataTicksAccesint32(address);\
    offset = 1;\
    address += 4;\
  }
 case 0xc8:
   {
     // LDM R0!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[0].I & 0xFFFFFFFC;
     uint32 temp = reg[0].I + 4*cpuBitsSet[opcode & 0xFF];
     int offset = 0;
     clockTicks = 0;
     // load
     THUMB_LDM_REG(1, 0);
     THUMB_LDM_REG(2, 1);
     THUMB_LDM_REG(4, 2);
     THUMB_LDM_REG(8, 3);
     THUMB_LDM_REG(16, 4);
     THUMB_LDM_REG(32, 5);
     THUMB_LDM_REG(64, 6);
     THUMB_LDM_REG(128, 7);
     clockTicks += codeTicksAccesint16(armNextPC)+2;
     if(!(opcode & 1))
       reg[0].I = temp;
   }
   break;
 case 0xc9:
   {
     // LDM R1!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[1].I & 0xFFFFFFFC;
     uint32 temp = reg[1].I + 4*cpuBitsSet[opcode & 0xFF];
     int offset = 0;
     clockTicks = 0;
     // load
     THUMB_LDM_REG(1, 0);
     THUMB_LDM_REG(2, 1);
     THUMB_LDM_REG(4, 2);
     THUMB_LDM_REG(8, 3);
     THUMB_LDM_REG(16, 4);
     THUMB_LDM_REG(32, 5);
     THUMB_LDM_REG(64, 6);
     THUMB_LDM_REG(128, 7);
     clockTicks += codeTicksAccesint16(armNextPC)+2;
     if(!(opcode & 2))
       reg[1].I = temp;
   }
   break;
 case 0xca:
   {
     // LDM R2!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[2].I & 0xFFFFFFFC;
     uint32 temp = reg[2].I + 4*cpuBitsSet[opcode & 0xFF];
     int offset = 0;
     clockTicks = 0;
     // load
     THUMB_LDM_REG(1, 0);
     THUMB_LDM_REG(2, 1);
     THUMB_LDM_REG(4, 2);
     THUMB_LDM_REG(8, 3);
     THUMB_LDM_REG(16, 4);
     THUMB_LDM_REG(32, 5);
     THUMB_LDM_REG(64, 6);
     THUMB_LDM_REG(128, 7);
     clockTicks += codeTicksAccesint16(armNextPC)+2;
     if(!(opcode & 4))
       reg[2].I = temp;
   }
   break;
 case 0xcb:
   {
     // LDM R3!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[3].I & 0xFFFFFFFC;
     uint32 temp = reg[3].I + 4*cpuBitsSet[opcode & 0xFF];
     int offset = 0;
     clockTicks = 0;
     // load
     THUMB_LDM_REG(1, 0);
     THUMB_LDM_REG(2, 1);
     THUMB_LDM_REG(4, 2);
     THUMB_LDM_REG(8, 3);
     THUMB_LDM_REG(16, 4);
     THUMB_LDM_REG(32, 5);
     THUMB_LDM_REG(64, 6);
     THUMB_LDM_REG(128, 7);
     clockTicks += codeTicksAccesint16(armNextPC)+2;
     if(!(opcode & 8))
       reg[3].I = temp;
   }
   break;
 case 0xcc:
   {
     // LDM R4!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[4].I & 0xFFFFFFFC;
     uint32 temp = reg[4].I + 4*cpuBitsSet[opcode & 0xFF];
     int offset = 0;
     clockTicks = 0;
     // load
     THUMB_LDM_REG(1, 0);
     THUMB_LDM_REG(2, 1);
     THUMB_LDM_REG(4, 2);
     THUMB_LDM_REG(8, 3);
     THUMB_LDM_REG(16, 4);
     THUMB_LDM_REG(32, 5);
     THUMB_LDM_REG(64, 6);
     THUMB_LDM_REG(128, 7);   
     clockTicks += codeTicksAccesint16(armNextPC)+2;
     if(!(opcode & 16))
       reg[4].I = temp;
   }
   break;
 case 0xcd:
   {
     // LDM R5!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[5].I & 0xFFFFFFFC;
     uint32 temp = reg[5].I + 4*cpuBitsSet[opcode & 0xFF];
     int offset = 0;
     clockTicks = 0;
     // load
     THUMB_LDM_REG(1, 0);
     THUMB_LDM_REG(2, 1);
     THUMB_LDM_REG(4, 2);
     THUMB_LDM_REG(8, 3);
     THUMB_LDM_REG(16, 4);
     THUMB_LDM_REG(32, 5);
     THUMB_LDM_REG(64, 6);
     THUMB_LDM_REG(128, 7);   
     clockTicks += codeTicksAccesint16(armNextPC)+2;
     if(!(opcode & 32))
       reg[5].I = temp;
   }
   break;
 case 0xce:
   {
     // LDM R6!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[6].I & 0xFFFFFFFC;
     uint32 temp = reg[6].I + 4*cpuBitsSet[opcode & 0xFF];
     int offset = 0;
     clockTicks = 0;
     // load
     THUMB_LDM_REG(1, 0);
     THUMB_LDM_REG(2, 1);
     THUMB_LDM_REG(4, 2);
     THUMB_LDM_REG(8, 3);
     THUMB_LDM_REG(16, 4);
     THUMB_LDM_REG(32, 5);
     THUMB_LDM_REG(64, 6);
     THUMB_LDM_REG(128, 7); 
     clockTicks += codeTicksAccesint16(armNextPC)+2;
     if(!(opcode & 64))
       reg[6].I = temp;
   }
   break;
 case 0xcf:
   {
     // LDM R7!, {Rlist}
     busPrefetch = busPrefetchEnable;
     uint32 address = reg[7].I & 0xFFFFFFFC;
     uint32 temp = reg[7].I + 4*cpuBitsSet[opcode & 0xFF];
     int offset = 0;
     clockTicks = 0;
     // load
     THUMB_LDM_REG(1, 0);
     THUMB_LDM_REG(2, 1);
     THUMB_LDM_REG(4, 2);
     THUMB_LDM_REG(8, 3);
     THUMB_LDM_REG(16, 4);
     THUMB_LDM_REG(32, 5);
     THUMB_LDM_REG(64, 6);
     THUMB_LDM_REG(128, 7);  
     clockTicks += codeTicksAccesint16(armNextPC)+2;
     if(!(opcode & 128))
       reg[7].I = temp;
   }
   break;
 case 0xd0:
   // BEQ offset
   if(Z_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;      
 case 0xd1:
   // BNE offset
   if(!Z_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xd2:
   // BCS offset
   if(C_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xd3:
   // BCC offset
   if(!C_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xd4:
   // BMI offset
   if(N_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xd5:
   // BPL offset
   if(!N_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xd6:
   // BVS offset
   if(V_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xd7:
   // BVC offset
   if(!V_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xd8:
   // BHI offset
   if(C_FLAG && !Z_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xd9:
   // BLS offset
   if(!C_FLAG || Z_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xda:
   // BGE offset
   if(N_FLAG == V_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xdb:
   // BLT offset
   if(N_FLAG != V_FLAG) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xdc:
   // BGT offset
   if(!Z_FLAG && (N_FLAG == V_FLAG)) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xdd:
   // BLE offset
   if(Z_FLAG || (N_FLAG != V_FLAG)) {
     reg[15].I += ((int8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks = 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;   
 case 0xdf:
     {
   // SWI #comment
   uint32 address = 0;
   busPrefetchCount=0;
   clockTicks = 2*codeTicksAccessSeq16(address) + codeTicksAccesint16(address)+3;
   CPUSoftwareInterrupt(opcode & 0xFF);
   break;
     }
 case 0xe0:
 case 0xe1:
 case 0xe2:
 case 0xe3:
 case 0xe4:
 case 0xe5:
 case 0xe6:
 case 0xe7:
   {
     // B offset
     int offset = (opcode & 0x3FF) << 1;
     if(opcode & 0x0400)
       offset |= 0xFFFFF800;
     reg[15].I += offset;
     armNextPC = reg[15].I;
     reg[15].I += 2;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks += clockTicks + codeTicksAccesint16(armNextPC)+1;
   }
   break;
 case 0xf0:
 case 0xf1:
 case 0xf2:
 case 0xf3:
   {
     // BLL #offset
     int offset = (opcode & 0x7FF);
     reg[14].I = reg[15].I + (offset << 12);
   }
   break;      
 case 0xf4:
 case 0xf5:
 case 0xf6:
 case 0xf7:
   {
     // BLL #offset
     int offset = (opcode & 0x7FF);
     reg[14].I = reg[15].I + ((offset << 12) | 0xFF800000);
   }
   break;   
 case 0xf8:
 case 0xf9:
 case 0xfa:
 case 0xfb:
 case 0xfc:
 case 0xfd:
 case 0xfe:
 case 0xff:
   {
     // BLH #offset
     int offset = (opcode & 0x7FF);
     uint32 temp = reg[15].I-2;
     reg[15].I = (reg[14].I + (offset<<1))&0xFFFFFFFE;
     armNextPC = reg[15].I;
     reg[15].I += 2;
     reg[14].I = temp|1;
     THUMB_PREFETCH;
     busPrefetchCount=0;
     clockTicks += 2*codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC)+3;
   }
   break;
#ifdef BKPT_SUPPORT
 case 0xbe:
   // BKPT #comment
   extern void (*dbgSignal)(int,int);
   reg[15].I -= 2;
   armNextPC -= 2;   
   dbgSignal(5, opcode & 255);
   return;
#endif
 case 0xb1:
 case 0xb2:
 case 0xb3:
 case 0xb6:
 case 0xb7:
 case 0xb8:
 case 0xb9:
 case 0xba:
 case 0xbb:
#ifndef BKPT_SUPPORT
 case 0xbe:
#endif
 case 0xbf:
 case 0xde:
 default:
 unknown_thumb:
#ifdef DEV_VERSION
   if(systemVerbose & VERBOSE_UNDEFINED)
     log("Undefined THUMB instruction %04x at %08x\n", opcode, armNextPC-2);
#endif
   CPUUndefinedException();
   break;
}
 return(clockTicks);
}
