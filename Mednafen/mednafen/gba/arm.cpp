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

#define INCED_FROM_ARM_CPP

#include "GBA.h"
#include "GBAinline.h"
#include "Globals.h"

unsigned int RunARM(void)
{
 unsigned int clockTicks;

#define OP_AND \
      reg[dest].I = reg[(opcode>>16)&15].I & value;

#define OP_ANDS \
      reg[dest].I = reg[(opcode>>16)&15].I & value;\
      \
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
      Z_FLAG = (reg[dest].I) ? false : true;\
      C_FLAG = C_OUT;

#define OP_EOR \
      reg[dest].I = reg[(opcode>>16)&15].I ^ value;

#define OP_EORS \
      reg[dest].I = reg[(opcode>>16)&15].I ^ value;\
      \
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
      Z_FLAG = (reg[dest].I) ? false : true;\
      C_FLAG = C_OUT;

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
#define OP_SUB \
    {\
      reg[dest].I = reg[base].I - value;\
    }
#define OP_SUBS \
   {\
     uint32 lhs = reg[base].I;\
     uint32 rhs = value;\
     uint32 res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_RSB \
    {\
      reg[dest].I = value - reg[base].I;\
    }
#define OP_RSBS \
   {\
     uint32 lhs = reg[base].I;\
     uint32 rhs = value;\
     uint32 res = rhs - lhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define OP_ADD \
    {\
      reg[dest].I = reg[base].I + value;\
    }
#define OP_ADDS \
   {\
     uint32 lhs = reg[base].I;\
     uint32 rhs = value;\
     uint32 res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define OP_ADC \
    {\
      reg[dest].I = reg[base].I + value + (uint32)C_FLAG;\
    }
#define OP_ADCS \
   {\
     uint32 lhs = reg[base].I;\
     uint32 rhs = value;\
     uint32 res = lhs + rhs + (uint32)C_FLAG;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define OP_SBC \
    {\
      reg[dest].I = reg[base].I - value - !((uint32)C_FLAG);\
    }
#define OP_SBCS \
   {\
     uint32 lhs = reg[base].I;\
     uint32 rhs = value;\
     uint32 res = lhs - rhs - !((uint32)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_RSC \
    {\
      reg[dest].I = value - reg[base].I - !((uint32)C_FLAG);\
    }
#define OP_RSCS \
   {\
     uint32 lhs = reg[base].I;\
     uint32 rhs = value;\
     uint32 res = rhs - lhs - !((uint32)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define OP_CMP \
   {\
     uint32 lhs = reg[base].I;\
     uint32 rhs = value;\
     uint32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_CMN \
   {\
     uint32 lhs = reg[base].I;\
     uint32 rhs = value;\
     uint32 res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }

#define LOGICAL_LSL_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (32 - shift)) & 1 ? true : false;\
     value = v << shift;\
   }
#define LOGICAL_LSR_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = v >> shift;\
   }
#define LOGICAL_ASR_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     C_OUT = ((int32)v >> (int)(shift - 1)) & 1 ? true : false;\
     value = (int32)v >> (int)shift;\
   }
#define LOGICAL_ROR_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define LOGICAL_RRX_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     shift = (int)C_FLAG;\
     C_OUT = (v  & 1) ? true : false;\
     value = ((v >> 1) |\
              (shift << 31));\
   }
#define LOGICAL_ROR_IMM \
   {\
     uint32 v = opcode & 0xff;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ARITHMETIC_LSL_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     value = v << shift;\
   }
#define ARITHMETIC_LSR_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     value = v >> shift;\
   }
#define ARITHMETIC_ASR_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     value = (int32)v >> (int)shift;\
   }
#define ARITHMETIC_ROR_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ARITHMETIC_RRX_REG \
   {\
     uint32 v = reg[opcode & 0x0f].I;\
     shift = (int)C_FLAG;\
     value = ((v >> 1) |\
              (shift << 31));\
   }
#define ARITHMETIC_ROR_IMM \
   {\
     uint32 v = opcode & 0xff;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ROR_IMM_MSR \
   {\
     uint32 v = opcode & 0xff;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ROR_VALUE \
   {\
     value = ((value << (32 - shift)) |\
              (value >> shift));\
   }
#define RCR_VALUE \
   {\
     shift = (int)C_FLAG;\
     value = ((value >> 1) |\
              (shift << 31));\
   }

#define OP_TST \
      uint32 res = reg[base].I & value;\
      N_FLAG = (res & 0x80000000) ? true : false;\
      Z_FLAG = (res) ? false : true;\
      C_FLAG = C_OUT;

#define OP_TEQ \
      uint32 res = reg[base].I ^ value;\
      N_FLAG = (res & 0x80000000) ? true : false;\
      Z_FLAG = (res) ? false : true;\
      C_FLAG = C_OUT;

#define OP_ORR \
    reg[dest].I = reg[base].I | value;

#define OP_ORRS \
    reg[dest].I = reg[base].I | value;\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_MOV \
    reg[dest].I = value;

#define OP_MOVS \
    reg[dest].I = value;\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_BIC \
    reg[dest].I = reg[base].I & (~value);

#define OP_BICS \
    reg[dest].I = reg[base].I & (~value);\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_MVN \
    reg[dest].I = ~value;

#define OP_MVNS \
    reg[dest].I = ~value; \
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define CASE_16(BASE) \
  case BASE:\
  case BASE+1:\
  case BASE+2:\
  case BASE+3:\
  case BASE+4:\
  case BASE+5:\
  case BASE+6:\
  case BASE+7:\
  case BASE+8:\
  case BASE+9:\
  case BASE+10:\
  case BASE+11:\
  case BASE+12:\
  case BASE+13:\
  case BASE+14:\
  case BASE+15:

#define CASE_256(BASE) \
  CASE_16(BASE)\
  CASE_16(BASE+0x10)\
  CASE_16(BASE+0x20)\
  CASE_16(BASE+0x30)\
  CASE_16(BASE+0x40)\
  CASE_16(BASE+0x50)\
  CASE_16(BASE+0x60)\
  CASE_16(BASE+0x70)\
  CASE_16(BASE+0x80)\
  CASE_16(BASE+0x90)\
  CASE_16(BASE+0xa0)\
  CASE_16(BASE+0xb0)\
  CASE_16(BASE+0xc0)\
  CASE_16(BASE+0xd0)\
  CASE_16(BASE+0xe0)\
  CASE_16(BASE+0xf0)

#define LOGICAL_DATA_OPCODE(OPCODE, OPCODE2, BASE) \
  case BASE: \
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      \
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_LSL_REG\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
       /* OP Rd,Rb,Rm LSR # */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_LSR_REG\
      } else {\
        value = 0;\
        C_OUT = (reg[opcode & 0x0F].I & 0x80000000) ? true : false;\
      }\
      \
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
       /* OP Rd,Rb,Rm ASR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_ASR_REG\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }                   \
      }\
      \
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
       /* OP Rd,Rb,Rm ROR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_ROR_REG\
      } else {\
        LOGICAL_RRX_REG\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
       /* OP Rd,Rb,Rm LSL Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (reg[opcode & 0x0F].I & 1 ? true : false);\
        } else if(shift < 32) {\
           LOGICAL_LSL_REG\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
       /* OP Rd,Rb,Rm LSR Rs */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (reg[opcode & 0x0F].I & 0x80000000 ? true : false);\
        } else if(shift < 32) {\
            LOGICAL_LSR_REG\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
       /* OP Rd,Rb,Rm ASR Rs */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift < 32) {\
        if(shift) {\
          LOGICAL_ASR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
       /* OP Rd,Rb,Rm ROR Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
          LOGICAL_ROR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
          C_OUT = (value & 0x80000000 ? true : false);\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
        C_OUT = (value & 0x80000000 ? true : false);\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int base = (opcode >> 16) & 0x0F;\
      int dest = (opcode >> 12) & 0x0F;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_ROR_IMM\
      } else {\
        value = opcode & 0xff;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;

#define LOGICAL_DATA_OPCODE_WITHOUT_base(OPCODE, OPCODE2, BASE) \
  case BASE: \
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */ \
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      \
      if(shift) {\
        LOGICAL_LSL_REG\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
       /* OP Rd,Rb,Rm LSR # */ \
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_LSR_REG\
      } else {\
        value = 0;\
        C_OUT = (reg[opcode & 0x0F].I & 0x80000000) ? true : false;\
      }\
      \
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
       /* OP Rd,Rb,Rm ASR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_ASR_REG\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }                   \
      }\
      \
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
       /* OP Rd,Rb,Rm ROR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_ROR_REG\
      } else {\
        LOGICAL_RRX_REG\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
       /* OP Rd,Rb,Rm LSL Rs */\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (reg[opcode & 0x0F].I & 1 ? true : false);\
        } else if(shift < 32) {\
           LOGICAL_LSL_REG\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
       /* OP Rd,Rb,Rm LSR Rs */ \
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (reg[opcode & 0x0F].I & 0x80000000 ? true : false);\
        } else if(shift < 32) {\
            LOGICAL_LSR_REG\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
       /* OP Rd,Rb,Rm ASR Rs */ \
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift < 32) {\
        if(shift) {\
          LOGICAL_ASR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
       /* OP Rd,Rb,Rm ROR Rs */\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
          LOGICAL_ROR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
          C_OUT = (value & 0x80000000 ? true : false);\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
        C_OUT = (value & 0x80000000 ? true : false);\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int dest = (opcode >> 12) & 0x0F;\
      bool C_OUT = C_FLAG;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        LOGICAL_ROR_IMM\
      } else {\
        value = opcode & 0xff;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;

#define ARITHMETIC_DATA_OPCODE(OPCODE, OPCODE2, BASE) \
  case BASE:\
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        ARITHMETIC_LSL_REG\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
      /* OP Rd,Rb,Rm LSR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        ARITHMETIC_LSR_REG\
      } else {\
        value = 0;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
      /* OP Rd,Rb,Rm ASR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        ARITHMETIC_ASR_REG\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
        } else value = 0;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
      /* OP Rd,Rb,Rm ROR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
         ARITHMETIC_ROR_REG\
      } else {\
         ARITHMETIC_RRX_REG\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
      /* OP Rd,Rb,Rm LSL Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
        } else if(shift < 32) {\
           ARITHMETIC_LSL_REG\
        } else value = 0;\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
      /* OP Rd,Rb,Rm LSR Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
        } else if(shift < 32) {\
           ARITHMETIC_LSR_REG\
        } else value = 0;\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
      /* OP Rd,Rb,Rm ASR Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift < 32) {\
        if(shift) {\
           ARITHMETIC_ASR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
        } else value = 0;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
      /* OP Rd,Rb,Rm ROR Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
           ARITHMETIC_ROR_REG\
        } else {\
           value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int base = (opcode >> 16) & 0x0F;\
      int dest = (opcode >> 12) & 0x0F;\
      uint32 value;\
      if ((dest == 15)||((opcode & 0x02000010)==0x10))\
      {\
        clockTicks = 1+codeTicksAccesint32(armNextPC);\
        if ((opcode & 0x02000010)==0x10)\
          clockTicks++;\
      }\
      {\
        ARITHMETIC_ROR_IMM\
      }\
      if(dest == 15) {\
        clockTicks+=2+codeTicksAccessSeq32(armNextPC)+codeTicksAccessSeq32(armNextPC);\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
          ARM_PREFETCH;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
          THUMB_PREFETCH;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;

  uint32 opcode = cpuPrefetch[0];
  cpuPrefetch[0] = cpuPrefetch[1];

  busPrefetch = false;
  if (busPrefetchCount & 0xFFFFFE00)
    busPrefetchCount = 0x100 | (busPrefetchCount & 0xFF);


  clockTicks = 0;//codeTicksAccessSeq32(armNextPC)+1;
  int oldArmNextPC = armNextPC;

#ifndef FINAL_VERSION
  if(armNextPC == stop) {
    armNextPC++;
  }
#endif

  armNextPC = reg[15].I;
  reg[15].I += 4;
  ARM_PREFETCH_NEXT;

  int cond = opcode >> 28;
  // suggested optimization for frequent cases
  bool cond_res;
  if(cond == 0x0e) {
    cond_res = true;
  } else {
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
    case 0x0E: 
      cond_res = true; 
      break;
    case 0x0F:
    default:
      // ???
      cond_res = false;
      break;
    }
  }
  
if(cond_res) {
  switch(((opcode>>16)&0xFF0) | ((opcode>>4)&0x0F)) {
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_AND,  OP_AND, 0x000);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_ANDS, OP_AND, 0x010);
  case 0x009:
    {
      // MUL Rd, Rm, Rs
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      clockTicks = 1; 
      uint32 rs = reg[(opcode >> 8) & 0x0F].I;
      reg[dest].I = reg[mult].I * rs;
      if(((int32)rs)<0)
         rs = ~rs;
      if((rs & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if ((rs & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if ((rs & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;

    }
    break;
  case 0x019:
    {
      // MULS Rd, Rm, Rs
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      clockTicks = 1;
      uint32 rs = reg[(opcode >> 8) & 0x0F].I;
      reg[dest].I = reg[mult].I * rs;
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;
      Z_FLAG = (reg[dest].I) ? false : true;
      if(((int32)rs)<0)
        rs = ~rs;
      if((rs & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if ((rs & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if ((rs & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
  case 0x00b:
  case 0x02b:
    {
      // STRH Rd, [Rn], -Rm
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x04b:
  case 0x06b:
    {
      // STRH Rd, [Rn], #-offset
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 2 + dataTicksAccesint16(address)
          + codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x08b:
  case 0x0ab:
    {
      // STRH Rd, [Rn], Rm
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x0cb:
  case 0x0eb:
    {
      // STRH Rd, [Rn], #offset
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x10b:
    {
      // STRH Rd, [Rn, -Rm]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
    }
    break;
  case 0x12b:
    {
      // STRH Rd, [Rn, -Rm]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
      reg[base].I = address;
    }
    break;
  case 0x14b:
    {
      // STRH Rd, [Rn, -#offset]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
    }
    break;
  case 0x16b:
    {
      // STRH Rd, [Rn, -#offset]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
      reg[base].I = address;
    }
    break;
  case 0x18b:
    {
      // STRH Rd, [Rn, Rm]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
    }
    break;
  case 0x1ab:
    {
      // STRH Rd, [Rn, Rm]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
      reg[base].I = address;
    }
    break;
  case 0x1cb:
    {
      // STRH Rd, [Rn, #offset]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
    }
    break;
  case 0x1eb:
    {
      // STRH Rd, [Rn, #offset]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
      CPUWriteHalfWord(address, reg[dest].W.W0);
      reg[base].I = address;
    }
    break;
  case 0x01b:
  case 0x03b:
    {
      // LDRH Rd, [Rn], -Rm
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      reg[dest].I = CPUReadHalfWord(address);
      if(dest != base) {
        address -= offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x05b:
  case 0x07b:
    {
      // LDRH Rd, [Rn], #-offset
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      reg[dest].I = CPUReadHalfWord(address);
      if(dest != base) {
        address -= offset;
        reg[base].I = address;
      }
      clockTicks=0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x09b:
  case 0x0bb:
    {
      // LDRH Rd, [Rn], Rm
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      reg[dest].I = CPUReadHalfWord(address);
      if(dest != base) {
        address += offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x0db:
  case 0x0fb:
    {
      // LDRH Rd, [Rn], #offset
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      reg[dest].I = CPUReadHalfWord(address);
      if(dest != base) {
        address += offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x11b:
    {
      // LDRH Rd, [Rn, -Rm]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - reg[opcode & 0x0F].I;
      reg[dest].I = CPUReadHalfWord(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x13b:
    {
      // LDRH Rd, [Rn, -Rm]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - reg[opcode & 0x0F].I;
      reg[dest].I = CPUReadHalfWord(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x15b:
    {
      // LDRH Rd, [Rn, -#offset]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = CPUReadHalfWord(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x17b:
    {
      // LDRH Rd, [Rn, -#offset]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = CPUReadHalfWord(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x19b:
    {
      // LDRH Rd, [Rn, Rm]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + reg[opcode & 0x0F].I;
      reg[dest].I = CPUReadHalfWord(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1bb:
    {
      // LDRH Rd, [Rn, Rm]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + reg[opcode & 0x0F].I;
      reg[dest].I = CPUReadHalfWord(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1db:
    {
      // LDRH Rd, [Rn, #offset]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = CPUReadHalfWord(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1fb:
    {
      // LDRH Rd, [Rn, #offset]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = CPUReadHalfWord(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x01d:
  case 0x03d:
    {
      // LDRSB Rd, [Rn], -Rm
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      reg[dest].I = (int8)CPUReadByte(address);
      if(dest != base) {
        address -= offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x05d:
  case 0x07d:
    {
      // LDRSB Rd, [Rn], #-offset
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      reg[dest].I = (int8)CPUReadByte(address);
      if(dest != base) {
        address -= offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x09d:
  case 0x0bd:
    {
      // LDRSB Rd, [Rn], Rm
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      reg[dest].I = (int8)CPUReadByte(address);
      if(dest != base) {
        address += offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x0dd:
  case 0x0fd:
    {
      // LDRSB Rd, [Rn], #offset
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      reg[dest].I = (int8)CPUReadByte(address);
      if(dest != base) {
        address += offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x11d:
    {
      // LDRSB Rd, [Rn, -Rm]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - reg[opcode & 0x0F].I;
      reg[dest].I = (int8)CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x13d:
    {
      // LDRSB Rd, [Rn, -Rm]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - reg[opcode & 0x0F].I;
      reg[dest].I = (int8)CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x15d:
    {
      // LDRSB Rd, [Rn, -#offset]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = (int8)CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x17d:
    {
      // LDRSB Rd, [Rn, -#offset]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = (int8)CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x19d:
    {
      // LDRSB Rd, [Rn, Rm]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + reg[opcode & 0x0F].I;
      reg[dest].I = (int8)CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1bd:
    {
      // LDRSB Rd, [Rn, Rm]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + reg[opcode & 0x0F].I;
      reg[dest].I = (int8)CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1dd:
    {
      // LDRSB Rd, [Rn, #offset]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = (int8)CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1fd:
    {
      // LDRSB Rd, [Rn, #offset]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = (int8)CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x01f:
  case 0x03f:
    {
      // LDRSH Rd, [Rn], -Rm
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      if(dest != base) {
        address -= offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x05f:
  case 0x07f:
    {
      // LDRSH Rd, [Rn], #-offset
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      if(dest != base) {
        address -= offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x09f:
  case 0x0bf:
    {
      // LDRSH Rd, [Rn], Rm
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      if(dest != base) {
        address += offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x0df:
  case 0x0ff:
    {
      // LDRSH Rd, [Rn], #offset
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      if(dest != base) {
        address += offset;
        reg[base].I = address;
      }
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x11f:
    {
      // LDRSH Rd, [Rn, -Rm]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - reg[opcode & 0x0F].I;
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x13f:
    {
      // LDRSH Rd, [Rn, -Rm]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - reg[opcode & 0x0F].I;
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x15f:
    {
      // LDRSH Rd, [Rn, -#offset]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x17f:
    {
      // LDRSH Rd, [Rn, -#offset]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x19f:
    {
      // LDRSH Rd, [Rn, Rm]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + reg[opcode & 0x0F].I;
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1bf:
    {
      // LDRSH Rd, [Rn, Rm]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + reg[opcode & 0x0F].I;
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1df:
    {
      // LDRSH Rd, [Rn, #offset]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x1ff:
    {
      // LDRSH Rd, [Rn, #offset]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      uint32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      reg[dest].I = (int16)CPUReadHalfWordSigned(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_EOR,  OP_EOR, 0x020);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_EORS, OP_EOR, 0x030);
  case 0x029:
    {
      // MLA Rd, Rm, Rs, Rn
      clockTicks = 2;
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      uint32 rs = reg[(opcode >> 8) & 0x0F].I;
      reg[dest].I = reg[mult].I * rs + reg[(opcode>>12)&0x0f].I;
      if((rs & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if ((rs & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if ((rs & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
  case 0x039:
    {
      // MLAS Rd, Rm, Rs, Rn
      clockTicks = 2;
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      uint32 rs = reg[(opcode >> 8) & 0x0F].I;
      reg[dest].I = reg[mult].I * rs + reg[(opcode>>12)&0x0f].I;
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;
      Z_FLAG = (reg[dest].I) ? false : true;
      if(((int32)rs)<0)
        rs = ~rs;
      if((rs & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if ((rs & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if ((rs & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_SUB,  OP_SUB, 0x040);
    ARITHMETIC_DATA_OPCODE(OP_SUBS, OP_SUB, 0x050);
    ARITHMETIC_DATA_OPCODE(OP_RSB,  OP_RSB, 0x060);
    ARITHMETIC_DATA_OPCODE(OP_RSBS, OP_RSB, 0x070);
    ARITHMETIC_DATA_OPCODE(OP_ADD,  OP_ADD, 0x080);
    ARITHMETIC_DATA_OPCODE(OP_ADDS, OP_ADD, 0x090);
  case 0x089:
    {
      // UMULL RdLo, RdHi, Rn, Rs
      clockTicks = 2;
      uint32 umult = reg[(opcode & 0x0F)].I;
      uint32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      uint64 uTemp = ((uint64)umult)*((uint64)usource);
      reg[destLo].I = (uint32)uTemp;
      reg[destHi].I = (uint32)(uTemp >> 32);
      if ((usource & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if ((usource & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if ((usource & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
  case 0x099:
    {
      // UMULLS RdLo, RdHi, Rn, Rs
      clockTicks = 2;
      uint32 umult = reg[(opcode & 0x0F)].I;
      uint32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      uint64 uTemp = ((uint64)umult)*((uint64)usource);
      reg[destLo].I = (uint32)uTemp;
      reg[destHi].I = (uint32)(uTemp >> 32);
      Z_FLAG = (uTemp) ? false : true;
      N_FLAG = (reg[destHi].I & 0x80000000) ? true : false;
      if ((usource & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if ((usource & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if ((usource & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_ADC,  OP_ADC, 0x0a0);
    ARITHMETIC_DATA_OPCODE(OP_ADCS, OP_ADC, 0x0b0);
  case 0x0a9:
    {
      // UMLAL RdLo, RdHi, Rn, Rs
      clockTicks = 3;
      uint32 umult = reg[(opcode & 0x0F)].I;
      uint32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      uint64 uTemp = (uint64)reg[destHi].I;
      uTemp <<= 32;
      uTemp |= (uint64)reg[destLo].I;
      uTemp += ((uint64)umult)*((uint64)usource);
      reg[destLo].I = (uint32)uTemp;
      reg[destHi].I = (uint32)(uTemp >> 32);
      if ((usource & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if ((usource & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if ((usource & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
  case 0x0b9:
    {
      // UMLALS RdLo, RdHi, Rn, Rs
      clockTicks = 3;
      uint32 umult = reg[(opcode & 0x0F)].I;
      uint32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      uint64 uTemp = (uint64)reg[destHi].I;
      uTemp <<= 32;
      uTemp |= (uint64)reg[destLo].I;
      uTemp += ((uint64)umult)*((uint64)usource);
      reg[destLo].I = (uint32)uTemp;
      reg[destHi].I = (uint32)(uTemp >> 32);
      Z_FLAG = (uTemp) ? false : true;
      N_FLAG = (reg[destHi].I & 0x80000000) ? true : false;
      if ((usource & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if ((usource & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if ((usource & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_SBC,  OP_SBC, 0x0c0);
    ARITHMETIC_DATA_OPCODE(OP_SBCS, OP_SBC, 0x0d0);
  case 0x0c9:
    {
      // SMULL RdLo, RdHi, Rm, Rs
      clockTicks = 2;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      uint32 rs = reg[(opcode >> 8) & 0x0F].I;
      int64 m = (int32)reg[(opcode & 0x0F)].I;
      int64 s = (int32)rs;
      int64 sTemp = m*s;
      reg[destLo].I = (uint32)sTemp;
      reg[destHi].I = (uint32)(sTemp >> 32);
      if(((int32)rs) < 0)
        rs = ~rs;
      if((rs & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if((rs & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if((rs & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
  case 0x0d9:
    {
      // SMULLS RdLo, RdHi, Rm, Rs
      clockTicks = 2;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      uint32 rs = reg[(opcode >> 8) & 0x0F].I;
      int64 m = (int32)reg[(opcode & 0x0F)].I;
      int64 s = (int32)rs;
      int64 sTemp = m*s;
      reg[destLo].I = (uint32)sTemp;
      reg[destHi].I = (uint32)(sTemp >> 32);
      Z_FLAG = (sTemp) ? false : true;
      N_FLAG = (sTemp < 0) ? true : false;
      if(((int32)rs) < 0)
        rs = ~rs;
      if((rs & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if((rs & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if((rs & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
      clockTicks += codeTicksAccesint32(armNextPC) + 1;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_RSC,  OP_RSC, 0x0e0);
    ARITHMETIC_DATA_OPCODE(OP_RSCS, OP_RSC, 0x0f0);
  case 0x0e9:
    {
      // SMLAL RdLo, RdHi, Rm, Rs
      clockTicks = codeTicksAccesint32(armNextPC) + 4;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      uint32 rs = reg[(opcode >> 8) & 0x0F].I;
      int64 m = (int32)reg[(opcode & 0x0F)].I;
      int64 s = (int32)rs;
      int64 sTemp = (uint64)reg[destHi].I;
      sTemp <<= 32;
      sTemp |= (uint64)reg[destLo].I;
      sTemp += m*s;
      reg[destLo].I = (uint32)sTemp;
      reg[destHi].I = (uint32)(sTemp >> 32);
      if(((int32)rs) < 0)
        rs = ~rs;
      if((rs & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if((rs & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if((rs & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
    }
    break;
  case 0x0f9:
    {
      // SMLALS RdLo, RdHi, Rm, Rs
      clockTicks = codeTicksAccesint32(armNextPC) + 4;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      uint32 rs = reg[(opcode >> 8) & 0x0F].I;
      int64 m = (int32)reg[(opcode & 0x0F)].I;
      int64 s = (int32)rs;
      int64 sTemp = (uint64)reg[destHi].I;
      sTemp <<= 32;
      sTemp |= (uint64)reg[destLo].I;
      sTemp += m*s;
      reg[destLo].I = (uint32)sTemp;
      reg[destHi].I = (uint32)(sTemp >> 32);
      Z_FLAG = (sTemp) ? false : true;
      N_FLAG = (sTemp < 0) ? true : false;
      if(((int32)rs) < 0)
        rs = ~rs;
      if((rs & 0xFFFFFF00) == 0)
        clockTicks += 0;
      else if((rs & 0xFFFF0000) == 0)
        clockTicks += 1;
      else if((rs & 0xFF000000) == 0)
        clockTicks += 2;
      else
        clockTicks += 3;
      if (busPrefetchCount==0)
        busPrefetchCount = (busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
    }
    break;
    LOGICAL_DATA_OPCODE(OP_TST, OP_TST, 0x110);
  case 0x100:
    // MRS Rd, CPSR
    // TODO: check if right instruction....
    CPUUpdateCPSR();
    reg[(opcode >> 12) & 0x0F].I = reg[16].I;
    break;
  case 0x109:
    {
      // SWP Rd, Rm, [Rn]
      uint32 address = reg[(opcode >> 16) & 15].I;
      uint32 temp = CPUReadMemory(address);
      CPUWriteMemory(address, reg[opcode&15].I);
      reg[(opcode >> 12) & 15].I = temp;
      clockTicks = 4 + dataTicksAccesint32(address) + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
    LOGICAL_DATA_OPCODE(OP_TEQ, OP_TEQ, 0x130);
  case 0x120:
    {
      // MSR CPSR_fields, Rm
      CPUUpdateCPSR();
      uint32 value = reg[opcode & 15].I;
      uint32 newValue = reg[16].I;
      if(armMode > 0x10) {
        if(opcode & 0x00010000)
          newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
      }
      if(opcode & 0x00080000)
        newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);
      newValue |= 0x10;
      CPUSwitchMode(newValue & 0x1f, false);
      reg[16].I = newValue;
      CPUUpdateFlags();
      if(!armState) { // this should not be allowed, but it seems to work
        THUMB_PREFETCH;
        reg[15].I = armNextPC + 2;
      }
    }
    break;
  case 0x121:
    {
      // BX Rm
      // TODO: check if right instruction...
      int base = opcode & 0x0F;
      busPrefetchCount=0;
      armState = reg[base].I & 1 ? false : true;
      if(armState) {
        reg[15].I = reg[base].I & 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks = codeTicksAccessSeq32(armNextPC) +
            codeTicksAccessSeq32(armNextPC) + codeTicksAccesint32(armNextPC) + 3;
      } else {
        reg[15].I = reg[base].I & 0xFFFFFFFE;
        armNextPC = reg[15].I;
        reg[15].I += 2;
        THUMB_PREFETCH;
        clockTicks = codeTicksAccessSeq16(armNextPC) +
            codeTicksAccessSeq16(armNextPC) + codeTicksAccesint16(armNextPC) + 3;
      }
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_CMP, OP_CMP, 0x150);
  case 0x140:
    // MRS Rd, SPSR
    // TODO: check if right instruction...
    reg[(opcode >> 12) & 0x0F].I = reg[17].I;
    break;
  case 0x149:
    {
      // SWPB Rd, Rm, [Rn]
      uint32 address = reg[(opcode >> 16) & 15].I;
      uint32 temp = CPUReadByte(address);
      CPUWriteByte(address, reg[opcode&15].B.B0);
      reg[(opcode>>12)&15].I = temp;
      clockTicks = 4 + dataTicksAccesint32(address) + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_CMN, OP_CMN, 0x170);
  case 0x160:
    {
      // MSR SPSR_fields, Rm
      uint32 value = reg[opcode & 15].I;
      if(armMode > 0x10 && armMode < 0x1f) {
        if(opcode & 0x00010000)
          reg[17].I = (reg[17].I & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          reg[17].I = (reg[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          reg[17].I = (reg[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
        if(opcode & 0x00080000)
          reg[17].I = (reg[17].I & 0x00FFFFFF) | (value & 0xFF000000);
      }
    }
    break;
    LOGICAL_DATA_OPCODE             (OP_ORR,  OP_ORR, 0x180);
    LOGICAL_DATA_OPCODE             (OP_ORRS, OP_ORR, 0x190);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_MOV,  OP_MOV, 0x1a0);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_MOVS, OP_MOV, 0x1b0);
    LOGICAL_DATA_OPCODE             (OP_BIC,  OP_BIC, 0x1c0);
    LOGICAL_DATA_OPCODE             (OP_BICS, OP_BIC, 0x1d0);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_MVN,  OP_MVN, 0x1e0);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_MVNS, OP_MVN, 0x1f0);
#ifdef BKPT_SUPPORT
  case 0x127:
  case 0x7ff: // for GDB support
    extern void (*dbgSignal)(int,int);
    reg[15].I -= 4;
    armNextPC -= 4;
    dbgSignal(5, (opcode & 0x0f)|((opcode>>4) & 0xfff0));
    return;
#endif
  case 0x320:
  case 0x321:
  case 0x322:
  case 0x323:
  case 0x324:
  case 0x325:
  case 0x326:
  case 0x327:
  case 0x328:
  case 0x329:
  case 0x32a:
  case 0x32b:
  case 0x32c:
  case 0x32d:
  case 0x32e:
  case 0x32f:
    {
      // MSR CPSR_fields, #
      CPUUpdateCPSR();
      uint32 value = opcode & 0xFF;
      int shift = (opcode & 0xF00) >> 7;
      if(shift) {
        ROR_IMM_MSR;
      }
      uint32 newValue = reg[16].I;
      if(armMode > 0x10) {
        if(opcode & 0x00010000)
          newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
      }
      if(opcode & 0x00080000)
        newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);

      newValue |= 0x10;

      CPUSwitchMode(newValue & 0x1f, false);
      reg[16].I = newValue;
      CPUUpdateFlags();
      if(!armState) { // this should not be allowed, but it seems to work
        THUMB_PREFETCH;
        reg[15].I = armNextPC + 2;
      }
    }
    break;
  case 0x360:
  case 0x361:
  case 0x362:
  case 0x363:
  case 0x364:
  case 0x365:
  case 0x366:
  case 0x367:
  case 0x368:
  case 0x369:
  case 0x36a:
  case 0x36b:
  case 0x36c:
  case 0x36d:
  case 0x36e:
  case 0x36f:
    {
      // MSR SPSR_fields, #
      if(armMode > 0x10 && armMode < 0x1f) {
        uint32 value = opcode & 0xFF;
        int shift = (opcode & 0xF00) >> 7;
        if(shift) {
          ROR_IMM_MSR;
        }
        if(opcode & 0x00010000)
          reg[17].I = (reg[17].I & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          reg[17].I = (reg[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          reg[17].I = (reg[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
        if(opcode & 0x00080000)
          reg[17].I = (reg[17].I & 0x00FFFFFF) | (value & 0xFF000000);
      }
    }
  break;
  CASE_16(0x400)
  // T versions shouldn't be different on GBA      
  CASE_16(0x420)
    {
      // STR Rd, [Rn], -#
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - offset;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x480)
    // T versions shouldn't be different on GBA
  CASE_16(0x4a0)
    {
      // STR Rd, [Rn], #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + offset;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x500)
    {
      // STR Rd, [Rn, -#]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x520)
    {
      // STR Rd, [Rn, -#]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x580)
    {
      // STR Rd, [Rn, #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x5a0)
    {
      // STR Rd, [Rn, #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x410)
    {
      // LDR Rd, [Rn], -#
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I -= offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x430)
    {
      // LDRT Rd, [Rn], -#
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I -= offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x490)
    {
      // LDR Rd, [Rn], #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I += offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x4b0)
    {
      // LDRT Rd, [Rn], #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I += offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x510)
    {
      // LDR Rd, [Rn, -#]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x530)
    {
      // LDR Rd, [Rn, -#]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
  }
    break;
  CASE_16(0x590)
    {
      // LDR Rd, [Rn, #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x5b0)
    {
      // LDR Rd, [Rn, #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x440)
    // T versions shouldn't be different on GBA      
  CASE_16(0x460)
    {
      // STRB Rd, [Rn], -#
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - offset;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x4c0)
    // T versions shouldn't be different on GBA
  CASE_16(0x4e0)
    {
      // STRB Rd, [Rn], #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + offset;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x540)
    {
      // STRB Rd, [Rn, -#]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x560)
    {
      // STRB Rd, [Rn, -#]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x5c0)
    {
      // STRB Rd, [Rn, #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x5e0)
    {
      // STRB Rd, [Rn, #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x450)
    // T versions shouldn't be different
  CASE_16(0x470)
    {
      // LDRB Rd, [Rn], -#
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I -= offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x4d0)
  CASE_16(0x4f0) // T versions should not be different
    {
      // LDRB Rd, [Rn], #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I += offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x550)
    {
      // LDRB Rd, [Rn, -#]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x570)
    {
      // LDRB Rd, [Rn, -#]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x5d0)
    {
      // LDRB Rd, [Rn, #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x5f0)
    {
      // LDRB Rd, [Rn, #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x600:
  case 0x608:
    // T versions are the same
  case 0x620:
  case 0x628:
    {
      // STR Rd, [Rn], -Rm, LSL #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - offset;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x602:
  case 0x60a:
    // T versions are the same
  case 0x622:
  case 0x62a:
    {
      // STR Rd, [Rn], -Rm, LSR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - offset;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x604:
  case 0x60c:
    // T versions are the same
  case 0x624:
  case 0x62c:
    {
      // STR Rd, [Rn], -Rm, ASR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - offset;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x606:
  case 0x60e:
    // T versions are the same
  case 0x626:
  case 0x62e:
    {
      // STR Rd, [Rn], -Rm, ROR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - value;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x680:
  case 0x688:
    // T versions are the same
  case 0x6a0:
  case 0x6a8:
    {
      // STR Rd, [Rn], Rm, LSL #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + offset;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x682:
  case 0x68a:
    // T versions are the same
  case 0x6a2:
  case 0x6aa:
    {
      // STR Rd, [Rn], Rm, LSR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + offset;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x684:
  case 0x68c:
    // T versions are the same
  case 0x6a4:
  case 0x6ac:
    {
      // STR Rd, [Rn], Rm, ASR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + offset;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x686:
  case 0x68e:
    // T versions are the same
  case 0x6a6:
  case 0x6ae:
    {
      // STR Rd, [Rn], Rm, ROR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + value;
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x700:
  case 0x708:
    {
      // STR Rd, [Rn, -Rm, LSL #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x702:
  case 0x70a:
    {
      // STR Rd, [Rn, -Rm, LSR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x704:
  case 0x70c:
    {
      // STR Rd, [Rn, -Rm, ASR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x706:
  case 0x70e:
    {
      // STR Rd, [Rn, -Rm, ROR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - value;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x720:
  case 0x728:
    {
      // STR Rd, [Rn, -Rm, LSL #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x722:
  case 0x72a:
    {
      // STR Rd, [Rn, -Rm, LSR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[base].I = address;      
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x724:
  case 0x72c:
    {
      // STR Rd, [Rn, -Rm, ASR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x726:
  case 0x72e:
    {
      // STR Rd, [Rn, -Rm, ROR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - value;
      reg[base].I = address;      
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x780:
  case 0x788:
    {
      // STR Rd, [Rn, Rm, LSL #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x782:
  case 0x78a:
    {
      // STR Rd, [Rn, Rm, LSR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x784:
  case 0x78c:
    {
      // STR Rd, [Rn, Rm, ASR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x786:
  case 0x78e:
    {
      // STR Rd, [Rn, Rm, ROR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + value;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7a0:
  case 0x7a8:
    {
      // STR Rd, [Rn, Rm, LSL #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7a2:
  case 0x7aa:
    {
      // STR Rd, [Rn, Rm, LSR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[base].I = address;      
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7a4:
  case 0x7ac:
    {
      // STR Rd, [Rn, Rm, ASR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7a6:
  case 0x7ae:
    {
      // STR Rd, [Rn, Rm, ROR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + value;
      reg[base].I = address;      
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x610:
  case 0x618:
    // T versions are the same
  case 0x630:
  case 0x638:
    {
      // LDR Rd, [Rn], -Rm, LSL #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address - offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x612:
  case 0x61a:
    // T versions are the same
  case 0x632:
  case 0x63a:
    {
      // LDR Rd, [Rn], -Rm, LSR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address - offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x614:
  case 0x61c:
    // T versions are the same
  case 0x634:
  case 0x63c:
    {
      // LDR Rd, [Rn], -Rm, ASR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address - offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x616:
  case 0x61e:
    // T versions are the same
  case 0x636:
  case 0x63e:
    {
      // LDR Rd, [Rn], -Rm, ROR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address - value;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x690:
  case 0x698:
    // T versions are the same
  case 0x6b0:
  case 0x6b8:
    {
      // LDR Rd, [Rn], Rm, LSL #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address + offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x692:
  case 0x69a:
    // T versions are the same
  case 0x6b2:
  case 0x6ba:
    {
      // LDR Rd, [Rn], Rm, LSR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address + offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x694:
  case 0x69c:
    // T versions are the same
  case 0x6b4:
  case 0x6bc:
    {
      // LDR Rd, [Rn], Rm, ASR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address + offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x696:
  case 0x69e:
    // T versions are the same
  case 0x6b6:
  case 0x6be:
    {
      // LDR Rd, [Rn], Rm, ROR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address + value;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x710:
  case 0x718:
    {
      // LDR Rd, [Rn, -Rm, LSL #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x712:
  case 0x71a:
    {
      // LDR Rd, [Rn, -Rm, LSR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x714:
  case 0x71c:
    {
      // LDR Rd, [Rn, -Rm, ASR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x716:
  case 0x71e:
    {
      // LDR Rd, [Rn, -Rm, ROR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - value;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x730:
  case 0x738:
    {
      // LDR Rd, [Rn, -Rm, LSL #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x732:
  case 0x73a:
    {
      // LDR Rd, [Rn, -Rm, LSR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;      
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x734:
  case 0x73c:
    {
      // LDR Rd, [Rn, -Rm, ASR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x736:
  case 0x73e:
    {
      // LDR Rd, [Rn, -Rm, ROR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - value;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;      
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x790:
  case 0x798:
    {
      // LDR Rd, [Rn, Rm, LSL #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x792:
  case 0x79a:
    {
      // LDR Rd, [Rn, Rm, LSR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x794:
  case 0x79c:
    {
      // LDR Rd, [Rn, Rm, ASR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x796:
  case 0x79e:
    {
      // LDR Rd, [Rn, Rm, ROR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + value;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7b0:
  case 0x7b8:
    {
      // LDR Rd, [Rn, Rm, LSL #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7b2:
  case 0x7ba:
    {
      // LDR Rd, [Rn, Rm, LSR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;      
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7b4:
  case 0x7bc:
    {
      // LDR Rd, [Rn, Rm, ASR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7b6:
  case 0x7be:
    {
      // LDR Rd, [Rn, Rm, ROR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + value;
      reg[dest].I = CPUReadMemory(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint32(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x640:
  case 0x648:
    // T versions are the same
  case 0x660:
  case 0x668:
    {
      // STRB Rd, [Rn], -Rm, LSL #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - offset;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x642:
  case 0x64a:
    // T versions are the same
  case 0x662:
  case 0x66a:
    {
      // STRB Rd, [Rn], -Rm, LSR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - offset;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x644:
  case 0x64c:
    // T versions are the same
  case 0x664:
  case 0x66c:
    {
      // STRB Rd, [Rn], -Rm, ASR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - offset;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x646:
  case 0x64e:
    // T versions are the same
  case 0x666:
  case 0x66e:
    {
      // STRB Rd, [Rn], -Rm, ROR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - value;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x6c0:
  case 0x6c8:
    // T versions are the same
  case 0x6e0:
  case 0x6e8:
    {
      // STRB Rd, [Rn], Rm, LSL #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + offset;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x6c2:
  case 0x6ca:
    // T versions are the same
  case 0x6e2:
  case 0x6ea:
    {
      // STRB Rd, [Rn], Rm, LSR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + offset;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x6c4:
  case 0x6cc:
    // T versions are the same
  case 0x6e4:
  case 0x6ec:
    {
      // STRB Rd, [Rn], Rm, ASR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + offset;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x6c6:
  case 0x6ce:
    // T versions are the same
  case 0x6e6:
  case 0x6ee:
    {
      // STRB Rd, [Rn], Rm, ROR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + value;
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x740:
  case 0x748:
    {
      // STRB Rd, [Rn, -Rm, LSL #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x742:
  case 0x74a:
    {
      // STRB Rd, [Rn, -Rm, LSR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x744:
  case 0x74c:
    {
      // STRB Rd, [Rn, -Rm, ASR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x746:
  case 0x74e:
    {
      // STRB Rd, [Rn, -Rm, ROR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - value;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x760:
  case 0x768:
    {
      // STRB Rd, [Rn, -Rm, LSL #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x762:
  case 0x76a:
    {
      // STRB Rd, [Rn, -Rm, LSR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[base].I = address;      
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x764:
  case 0x76c:
    {
      // STRB Rd, [Rn, -Rm, ASR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x766:
  case 0x76e:
    {
      // STRB Rd, [Rn, -Rm, ROR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - value;
      reg[base].I = address;      
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7c0:
  case 0x7c8:
    {
      // STRB Rd, [Rn, Rm, LSL #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7c2:
  case 0x7ca:
    {
      // STRB Rd, [Rn, Rm, LSR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7c4:
  case 0x7cc:
    {
      // STRB Rd, [Rn, Rm, ASR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7c6:
  case 0x7ce:
    {
      // STRB Rd, [Rn, Rm, ROR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + value;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7e0:
  case 0x7e8:
    {
      // STRB Rd, [Rn, Rm, LSL #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7e2:
  case 0x7ea:
    {
      // STRB Rd, [Rn, Rm, LSR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[base].I = address;      
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7e4:
  case 0x7ec:
    {
      // STRB Rd, [Rn, Rm, ASR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7e6:
  case 0x7ee:
    {
      // STRB Rd, [Rn, Rm, ROR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + value;
      reg[base].I = address;      
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x650:
  case 0x658:
    // T versions are the same
  case 0x670:
  case 0x678:
    {
      // LDRB Rd, [Rn], -Rm, LSL #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address - offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x652:
  case 0x65a:
    // T versions are the same
  case 0x672:
  case 0x67a:
    {
      // LDRB Rd, [Rn], -Rm, LSR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address - offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x654:
  case 0x65c:
    // T versions are the same
  case 0x674:
  case 0x67c:
    {
      // LDRB Rd, [Rn], -Rm, ASR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address - offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x656:
  case 0x65e:
    // T versions are the same
  case 0x676:
  case 0x67e:
    {
      // LDRB Rd, [Rn], -Rm, ROR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address - value;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x6d0:
  case 0x6d8:
    // T versions are the same
  case 0x6f0:
  case 0x6f8:
    {
      // LDRB Rd, [Rn], Rm, LSL #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address + offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x6d2:
  case 0x6da:
    // T versions are the same
  case 0x6f2:
  case 0x6fa:
    {
      // LDRB Rd, [Rn], Rm, LSR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address + offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x6d4:
  case 0x6dc:
    // T versions are the same
  case 0x6f4:
  case 0x6fc:
    {
      // LDRB Rd, [Rn], Rm, ASR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address + offset;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x6d6:
  case 0x6de:
    // T versions are the same
  case 0x6f6:
  case 0x6fe:
    {
      // LDRB Rd, [Rn], Rm, ROR #
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address + value;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x750:
  case 0x758:
    {
      // LDRB Rd, [Rn, -Rm, LSL #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x752:
  case 0x75a:
    {
      // LDRB Rd, [Rn, -Rm, LSR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x754:
  case 0x75c:
    {
      // LDRB Rd, [Rn, -Rm, ASR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x756:
  case 0x75e:
    {
      // LDRB Rd, [Rn, -Rm, ROR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - value;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x770:
  case 0x778:
    {
      // LDRB Rd, [Rn, -Rm, LSL #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x772:
  case 0x77a:
    {
      // LDRB Rd, [Rn, -Rm, LSR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;      
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x774:
  case 0x77c:
    {
      // LDRB Rd, [Rn, -Rm, ASR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x776:
  case 0x77e:
    {
      // LDRB Rd, [Rn, -Rm, ROR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I - value;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;      
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7d0:
  case 0x7d8:
    {
      // LDRB Rd, [Rn, Rm, LSL #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7d2:
  case 0x7da:
    {
      // LDRB Rd, [Rn, Rm, LSR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7d4:
  case 0x7dc:
    {
      // LDRB Rd, [Rn, Rm, ASR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7d6:
  case 0x7de:
    {
      // LDRB Rd, [Rn, Rm, ROR #]
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + value;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7f0:
  case 0x7f8:
    {
      // LDRB Rd, [Rn, Rm, LSL #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7f2:
  case 0x7fa:
    {
      // LDRB Rd, [Rn, Rm, LSR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;      
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7f4:
  case 0x7fc:
    {
      // LDRB Rd, [Rn, Rm, ASR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((int32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
  case 0x7f6:
  case 0x7fe:
    {
      // LDRB Rd, [Rn, Rm, ROR #]!
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int shift = (opcode >> 7) & 31;
      uint32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      uint32 address = reg[base].I + value;
      reg[dest].I = CPUReadByte(address);
      if(dest != base)
        reg[base].I = address;      
      clockTicks = 0;
      if(dest == 15) {
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        clockTicks += 2 + dataTicksAccessSeq32(address) + dataTicksAccessSeq32(address);
      }
      clockTicks += 3 + dataTicksAccesint16(address) +
          codeTicksAccesint32(armNextPC);
    }
    break;
#define STMW_REG(val,num) \
  if(opcode & (val)) {\
    CPUWriteMemory(address, reg[(num)].I);\
    if(!offset) {\
      reg[base].I = temp;\
      clockTicks += 1 + dataTicksAccesint32(address);\
      offset = 1;\
    } else {\
      clockTicks += 1 + dataTicksAccessSeq32(address);\
    }\
    address += 4;\
  }
#define STM_REG(val,num) \
  if(opcode & (val)) {\
    CPUWriteMemory(address, reg[(num)].I);\
    if(!offset) {\
      clockTicks += 1 + dataTicksAccesint32(address);\
      offset = 1;\
    } else {\
      clockTicks += 1 + dataTicksAccessSeq32(address);\
    }\
    address += 4;\
  }
    
  CASE_16(0x800)
    {
      // STMDA Rn, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (temp + 4) & 0xFFFFFFFC;
      int offset = 0;
      STM_REG(1, 0);
      STM_REG(2, 1);
      STM_REG(4, 2);
      STM_REG(8, 3);
      STM_REG(16, 4);
      STM_REG(32, 5);
      STM_REG(64, 6);
      STM_REG(128, 7);
      STM_REG(256, 8);
      STM_REG(512, 9);
      STM_REG(1024, 10);
      STM_REG(2048, 11);
      STM_REG(4096, 12);
      STM_REG(8192, 13);
      STM_REG(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x820)
    {
      // STMDA Rn!, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (temp+4) & 0xFFFFFFFC;
      int offset = 0;

      STMW_REG(1, 0);
      STMW_REG(2, 1);
      STMW_REG(4, 2);
      STMW_REG(8, 3);
      STMW_REG(16, 4);
      STMW_REG(32, 5);
      STMW_REG(64, 6);
      STMW_REG(128, 7);
      STMW_REG(256, 8);
      STMW_REG(512, 9);
      STMW_REG(1024, 10);
      STMW_REG(2048, 11);
      STMW_REG(4096, 12);
      STMW_REG(8192, 13);
      STMW_REG(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        reg[base].I = temp;
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
  break;
  CASE_16(0x840)
    {
      // STMDA Rn, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (temp+4) & 0xFFFFFFFC;
      int offset = 0;

      STM_REG(1, 0);
      STM_REG(2, 1);
      STM_REG(4, 2);
      STM_REG(8, 3);
      STM_REG(16, 4);
      STM_REG(32, 5);
      STM_REG(64, 6);
      STM_REG(128, 7);

      if(armMode == 0x11) {
        STM_REG(256, R8_FIQ);
        STM_REG(512, R9_FIQ);
        STM_REG(1024, R10_FIQ);
        STM_REG(2048, R11_FIQ);
        STM_REG(4096, R12_FIQ);
      } else {
        STM_REG(256, 8);
        STM_REG(512, 9);
        STM_REG(1024, 10);
        STM_REG(2048, 11);
        STM_REG(4096, 12);
      }

      if(armMode != 0x10 && armMode != 0x1f) {
        STM_REG(8192, R13_USR);
        STM_REG(16384, R14_USR);
      } else {
        STM_REG(8192, 13);
        STM_REG(16384, 14);
      }

      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x860)
    {
      // STMDA Rn!, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (temp+4) & 0xFFFFFFFC;
      int offset = 0;

      STMW_REG(1, 0);
      STMW_REG(2, 1);
      STMW_REG(4, 2);
      STMW_REG(8, 3);
      STMW_REG(16, 4);
      STMW_REG(32, 5);
      STMW_REG(64, 6);
      STMW_REG(128, 7);

      if(armMode == 0x11) {
        STMW_REG(256, R8_FIQ);
        STMW_REG(512, R9_FIQ);
        STMW_REG(1024, R10_FIQ);
        STMW_REG(2048, R11_FIQ);
        STMW_REG(4096, R12_FIQ);
      } else {
        STMW_REG(256, 8);
        STMW_REG(512, 9);
        STMW_REG(1024, 10);
        STMW_REG(2048, 11);
        STMW_REG(4096, 12);
      }

      if(armMode != 0x10 && armMode != 0x1f) {
        STMW_REG(8192, R13_USR);
        STMW_REG(16384, R14_USR);
      } else {
        STMW_REG(8192, 13);
        STMW_REG(16384, 14);
      }

      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        reg[base].I = temp;
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  
  CASE_16(0x880)
    {
      // STMIA Rn, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = reg[base].I & 0xFFFFFFFC;
      int offset = 0;
      STM_REG(1, 0);
      STM_REG(2, 1);
      STM_REG(4, 2);
      STM_REG(8, 3);
      STM_REG(16, 4);
      STM_REG(32, 5);
      STM_REG(64, 6);
      STM_REG(128, 7);
      STM_REG(256, 8);
      STM_REG(512, 9);
      STM_REG(1024, 10);
      STM_REG(2048, 11);
      STM_REG(4096, 12);
      STM_REG(8192, 13);
      STM_REG(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
  break;
  CASE_16(0x8a0)
    {
      // STMIA Rn!, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = reg[base].I & 0xFFFFFFFC;
      int offset = 0;
      uint32 temp = reg[base].I + 4*(cpuBitsSet[opcode & 0xFF] +
                                  cpuBitsSet[(opcode >> 8) & 255]);
      STMW_REG(1, 0);
      STMW_REG(2, 1);
      STMW_REG(4, 2);
      STMW_REG(8, 3);
      STMW_REG(16, 4);
      STMW_REG(32, 5);
      STMW_REG(64, 6);
      STMW_REG(128, 7);
      STMW_REG(256, 8);
      STMW_REG(512, 9);
      STMW_REG(1024, 10);
      STMW_REG(2048, 11);
      STMW_REG(4096, 12);
      STMW_REG(8192, 13);
      STMW_REG(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset) {
          reg[base].I = temp;
          clockTicks += 1 + dataTicksAccesint32(address);
        } else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
  break;
  CASE_16(0x8c0)
    {
      // STMIA Rn, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = reg[base].I & 0xFFFFFFFC;
      int offset = 0;
      STM_REG(1, 0);
      STM_REG(2, 1);
      STM_REG(4, 2);
      STM_REG(8, 3);
      STM_REG(16, 4);
      STM_REG(32, 5);
      STM_REG(64, 6);
      STM_REG(128, 7);
      if(armMode == 0x11) {
        STM_REG(256, R8_FIQ);
        STM_REG(512, R9_FIQ);
        STM_REG(1024, R10_FIQ);
        STM_REG(2048, R11_FIQ);
        STM_REG(4096, R12_FIQ);
      } else {
        STM_REG(256, 8);
        STM_REG(512, 9);
        STM_REG(1024, 10);
        STM_REG(2048, 11);
        STM_REG(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STM_REG(8192, R13_USR);
        STM_REG(16384, R14_USR);
      } else {
        STM_REG(8192, 13);
        STM_REG(16384, 14);
      }
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x8e0)
    {
      // STMIA Rn!, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = reg[base].I & 0xFFFFFFFC;
      int offset = 0;
      uint32 temp = reg[base].I + 4*(cpuBitsSet[opcode & 0xFF] +
                                  cpuBitsSet[(opcode >> 8) & 255]);    
      STMW_REG(1, 0);
      STMW_REG(2, 1);
      STMW_REG(4, 2);
      STMW_REG(8, 3);
      STMW_REG(16, 4);
      STMW_REG(32, 5);
      STMW_REG(64, 6);
      STMW_REG(128, 7);
      if(armMode == 0x11) {
        STMW_REG(256, R8_FIQ);
        STMW_REG(512, R9_FIQ);
        STMW_REG(1024, R10_FIQ);
        STMW_REG(2048, R11_FIQ);
        STMW_REG(4096, R12_FIQ);
      } else {
        STMW_REG(256, 8);
        STMW_REG(512, 9);
        STMW_REG(1024, 10);
        STMW_REG(2048, 11);
        STMW_REG(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMW_REG(8192, R13_USR);
        STMW_REG(16384, R14_USR);
      } else {
        STMW_REG(8192, 13);
        STMW_REG(16384, 14);
      }
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset) {
          reg[base].I = temp;
          clockTicks += 1 + dataTicksAccesint32(address);
        } else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
    
  CASE_16(0x900)
    {
      // STMDB Rn, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = temp & 0xFFFFFFFC;
      int offset = 0;
      STM_REG(1, 0);
      STM_REG(2, 1);
      STM_REG(4, 2);
      STM_REG(8, 3);
      STM_REG(16, 4);
      STM_REG(32, 5);
      STM_REG(64, 6);
      STM_REG(128, 7);
      STM_REG(256, 8);
      STM_REG(512, 9);
      STM_REG(1024, 10);
      STM_REG(2048, 11);
      STM_REG(4096, 12);
      STM_REG(8192, 13);
      STM_REG(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x920)
    {
      // STMDB Rn!, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = temp & 0xFFFFFFFC;
      int offset = 0;

      STMW_REG(1, 0);
      STMW_REG(2, 1);
      STMW_REG(4, 2);
      STMW_REG(8, 3);
      STMW_REG(16, 4);
      STMW_REG(32, 5);
      STMW_REG(64, 6);
      STMW_REG(128, 7);
      STMW_REG(256, 8);
      STMW_REG(512, 9);
      STMW_REG(1024, 10);
      STMW_REG(2048, 11);
      STMW_REG(4096, 12);
      STMW_REG(8192, 13);
      STMW_REG(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        reg[base].I = temp;
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
  break;
  CASE_16(0x940)
    {
      // STMDB Rn, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = temp & 0xFFFFFFFC;
      int offset = 0;

      STM_REG(1, 0);
      STM_REG(2, 1);
      STM_REG(4, 2);
      STM_REG(8, 3);
      STM_REG(16, 4);
      STM_REG(32, 5);
      STM_REG(64, 6);
      STM_REG(128, 7);

      if(armMode == 0x11) {
        STM_REG(256, R8_FIQ);
        STM_REG(512, R9_FIQ);
        STM_REG(1024, R10_FIQ);
        STM_REG(2048, R11_FIQ);
        STM_REG(4096, R12_FIQ);
      } else {
        STM_REG(256, 8);
        STM_REG(512, 9);
        STM_REG(1024, 10);
        STM_REG(2048, 11);
        STM_REG(4096, 12);
      }

      if(armMode != 0x10 && armMode != 0x1f) {
        STM_REG(8192, R13_USR);
        STM_REG(16384, R14_USR);
      } else {
        STM_REG(8192, 13);
        STM_REG(16384, 14);
      }

      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x960)
    {
      // STMDB Rn!, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = temp & 0xFFFFFFFC;
      int offset = 0;

      STMW_REG(1, 0);
      STMW_REG(2, 1);
      STMW_REG(4, 2);
      STMW_REG(8, 3);
      STMW_REG(16, 4);
      STMW_REG(32, 5);
      STMW_REG(64, 6);
      STMW_REG(128, 7);

      if(armMode == 0x11) {
        STMW_REG(256, R8_FIQ);
        STMW_REG(512, R9_FIQ);
        STMW_REG(1024, R10_FIQ);
        STMW_REG(2048, R11_FIQ);
        STMW_REG(4096, R12_FIQ);
      } else {
        STMW_REG(256, 8);
        STMW_REG(512, 9);
        STMW_REG(1024, 10);
        STMW_REG(2048, 11);
        STMW_REG(4096, 12);
      }

      if(armMode != 0x10 && armMode != 0x1f) {
        STMW_REG(8192, R13_USR);
        STMW_REG(16384, R14_USR);
      } else {
        STMW_REG(8192, 13);
        STMW_REG(16384, 14);
      }

      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        reg[base].I = temp;
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;

  CASE_16(0x980)
    {
      // STMIB Rn, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = (reg[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      STM_REG(1, 0);
      STM_REG(2, 1);
      STM_REG(4, 2);
      STM_REG(8, 3);
      STM_REG(16, 4);
      STM_REG(32, 5);
      STM_REG(64, 6);
      STM_REG(128, 7);
      STM_REG(256, 8);
      STM_REG(512, 9);
      STM_REG(1024, 10);
      STM_REG(2048, 11);
      STM_REG(4096, 12);
      STM_REG(8192, 13);
      STM_REG(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x9a0)
    {
      // STMIB Rn!, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = (reg[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      uint32 temp = reg[base].I + 4*(cpuBitsSet[opcode & 0xFF] +
                                  cpuBitsSet[(opcode >> 8) & 255]);
      STMW_REG(1, 0);
      STMW_REG(2, 1);
      STMW_REG(4, 2);
      STMW_REG(8, 3);
      STMW_REG(16, 4);
      STMW_REG(32, 5);
      STMW_REG(64, 6);
      STMW_REG(128, 7);
      STMW_REG(256, 8);
      STMW_REG(512, 9);
      STMW_REG(1024, 10);
      STMW_REG(2048, 11);
      STMW_REG(4096, 12);
      STMW_REG(8192, 13);
      STMW_REG(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset) {
          reg[base].I = temp;
          clockTicks += 1 + dataTicksAccesint32(address);
        } else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x9c0)
    {
      // STMIB Rn, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = (reg[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      STM_REG(1, 0);
      STM_REG(2, 1);
      STM_REG(4, 2);
      STM_REG(8, 3);
      STM_REG(16, 4);
      STM_REG(32, 5);
      STM_REG(64, 6);
      STM_REG(128, 7);
      if(armMode == 0x11) {
        STM_REG(256, R8_FIQ);
        STM_REG(512, R9_FIQ);
        STM_REG(1024, R10_FIQ);
        STM_REG(2048, R11_FIQ);
        STM_REG(4096, R12_FIQ);
      } else {
        STM_REG(256, 8);
        STM_REG(512, 9);
        STM_REG(1024, 10);
        STM_REG(2048, 11);
        STM_REG(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STM_REG(8192, R13_USR);
        STM_REG(16384, R14_USR);
      } else {
        STM_REG(8192, 13);
        STM_REG(16384, 14);
      }
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x9e0)
    {
      // STMIB Rn!, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = (reg[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      uint32 temp = reg[base].I + 4*(cpuBitsSet[opcode & 0xFF] +
                                  cpuBitsSet[(opcode >> 8) & 255]);    
      STMW_REG(1, 0);
      STMW_REG(2, 1);
      STMW_REG(4, 2);
      STMW_REG(8, 3);
      STMW_REG(16, 4);
      STMW_REG(32, 5);
      STMW_REG(64, 6);
      STMW_REG(128, 7);
      if(armMode == 0x11) {
        STMW_REG(256, R8_FIQ);
        STMW_REG(512, R9_FIQ);
        STMW_REG(1024, R10_FIQ);
        STMW_REG(2048, R11_FIQ);
        STMW_REG(4096, R12_FIQ);
      } else {
        STMW_REG(256, 8);
        STMW_REG(512, 9);
        STMW_REG(1024, 10);
        STMW_REG(2048, 11);
        STMW_REG(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMW_REG(8192, R13_USR);
        STMW_REG(16384, R14_USR);
      } else {
        STMW_REG(8192, 13);
        STMW_REG(16384, 14);
      }
      if(opcode & 32768) {
        CPUWriteMemory(address, reg[15].I+4);
        if(!offset) {
          reg[base].I = temp;
          clockTicks += 1 + dataTicksAccesint32(address);
        } else
          clockTicks += 1 + dataTicksAccessSeq32(address);
      }
      clockTicks += 1 + codeTicksAccesint32(armNextPC);
    }
    break;
    
#define LDM_REG(val,num) \
  if(opcode & (val)) {\
    reg[(num)].I = CPUReadMemory(address);\
    if(offset)\
      clockTicks += 1 + dataTicksAccessSeq32(address);\
    else {\
      clockTicks += 1 + dataTicksAccesint32(address);\
      offset = 1;\
    }\
    address += 4;\
  }

  CASE_16(0x810)
    {
      // LDMDA Rn, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (temp + 4) & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      LDM_REG(1, 0);
      LDM_REG(2, 1);
      LDM_REG(4, 2);
      LDM_REG(8, 3);
      LDM_REG(16, 4);
      LDM_REG(32, 5);
      LDM_REG(64, 6);
      LDM_REG(128, 7);
      LDM_REG(256, 8);
      LDM_REG(512, 9);
      LDM_REG(1024, 10);
      LDM_REG(2048, 11);
      LDM_REG(4096, 12);
      LDM_REG(8192, 13);
      LDM_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address);
        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x830)
    {
      // LDMDA Rn!, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (temp + 4) & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      LDM_REG(1, 0);
      LDM_REG(2, 1);
      LDM_REG(4, 2);
      LDM_REG(8, 3);
      LDM_REG(16, 4);
      LDM_REG(32, 5);
      LDM_REG(64, 6);
      LDM_REG(128, 7);
      LDM_REG(256, 8);
      LDM_REG(512, 9);
      LDM_REG(1024, 10);
      LDM_REG(2048, 11);
      LDM_REG(4096, 12);
      LDM_REG(8192, 13);
      LDM_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address);

        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
      if(!(opcode & (1 << base)))
        reg[base].I = temp;
    }
    break;
  CASE_16(0x850)
    {
      // LDMDA Rn, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (temp + 4) & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);
        LDM_REG(256, 8);
        LDM_REG(512, 9);
        LDM_REG(1024, 10);
        LDM_REG(2048, 11);
        LDM_REG(4096, 12);
        LDM_REG(8192, 13);
        LDM_REG(16384, 14);

        reg[15].I = CPUReadMemory(address);

        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
          ARM_PREFETCH;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
          THUMB_PREFETCH;
        }
      } else {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);

        if(armMode == 0x11) {
          LDM_REG(256, R8_FIQ);
          LDM_REG(512, R9_FIQ);
          LDM_REG(1024, R10_FIQ);
          LDM_REG(2048, R11_FIQ);
          LDM_REG(4096, R12_FIQ);
        } else {
          LDM_REG(256, 8);
          LDM_REG(512, 9);
          LDM_REG(1024, 10);
          LDM_REG(2048, 11);
          LDM_REG(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_REG(8192, R13_USR);
          LDM_REG(16384, R14_USR);
        } else {
          LDM_REG(8192, 13);
          LDM_REG(16384, 14);
        }
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
        clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x870)
    {
      // LDMDA Rn!, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (temp + 4) & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);
        LDM_REG(256, 8);
        LDM_REG(512, 9);
        LDM_REG(1024, 10);
        LDM_REG(2048, 11);
        LDM_REG(4096, 12);
        LDM_REG(8192, 13);
        LDM_REG(16384, 14);

        reg[15].I = CPUReadMemory(address);

        if(!(opcode & (1 << base)))    
          reg[base].I = temp;

        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
          ARM_PREFETCH;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
          THUMB_PREFETCH;
        }
      } else {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);

        if(armMode == 0x11) {
          LDM_REG(256, R8_FIQ);
          LDM_REG(512, R9_FIQ);
          LDM_REG(1024, R10_FIQ);
          LDM_REG(2048, R11_FIQ);
          LDM_REG(4096, R12_FIQ);
        } else {
          LDM_REG(256, 8);
          LDM_REG(512, 9);
          LDM_REG(1024, 10);
          LDM_REG(2048, 11);
          LDM_REG(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_REG(8192, R13_USR);
          LDM_REG(16384, R14_USR);
        } else {
          LDM_REG(8192, 13);
          LDM_REG(16384, 14);
        }

        if(!(opcode & (1 << base)))    
          reg[base].I = temp;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
    
  CASE_16(0x890)
    {
      // LDMIA Rn, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = reg[base].I & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      LDM_REG(1, 0);
      LDM_REG(2, 1);
      LDM_REG(4, 2);
      LDM_REG(8, 3);
      LDM_REG(16, 4);
      LDM_REG(32, 5);
      LDM_REG(64, 6);
      LDM_REG(128, 7);
      LDM_REG(256, 8);
      LDM_REG(512, 9);
      LDM_REG(1024, 10);
      LDM_REG(2048, 11);
      LDM_REG(4096, 12);
      LDM_REG(8192, 13);
      LDM_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address);

        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
  break;
  CASE_16(0x8b0)
    {
      // LDMIA Rn!, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I + 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = reg[base].I & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      LDM_REG(1, 0);
      LDM_REG(2, 1);
      LDM_REG(4, 2);
      LDM_REG(8, 3);
      LDM_REG(16, 4);
      LDM_REG(32, 5);
      LDM_REG(64, 6);
      LDM_REG(128, 7);
      LDM_REG(256, 8);
      LDM_REG(512, 9);
      LDM_REG(1024, 10);
      LDM_REG(2048, 11);
      LDM_REG(4096, 12);
      LDM_REG(8192, 13);
      LDM_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address);

        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
      if(!(opcode & (1 << base)))
        reg[base].I = temp;
    }
    break;
  CASE_16(0x8d0)
    {
      // LDMIA Rn, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = reg[base].I & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);
        LDM_REG(256, 8);
        LDM_REG(512, 9);
        LDM_REG(1024, 10);
        LDM_REG(2048, 11);
        LDM_REG(4096, 12);
        LDM_REG(8192, 13);
        LDM_REG(16384, 14);

        reg[15].I = CPUReadMemory(address);

        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
          ARM_PREFETCH;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
          THUMB_PREFETCH;
        }
      } else {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);

        if(armMode == 0x11) {
          LDM_REG(256, R8_FIQ);
          LDM_REG(512, R9_FIQ);
          LDM_REG(1024, R10_FIQ);
          LDM_REG(2048, R11_FIQ);
          LDM_REG(4096, R12_FIQ);
        } else {
          LDM_REG(256, 8);
          LDM_REG(512, 9);
          LDM_REG(1024, 10);
          LDM_REG(2048, 11);
          LDM_REG(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_REG(8192, R13_USR);
          LDM_REG(16384, R14_USR);
        } else {
          LDM_REG(8192, 13);
          LDM_REG(16384, 14);
        }
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
  break;
  CASE_16(0x8f0)
    {
      // LDMIA Rn!, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = reg[base].I & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);
        LDM_REG(256, 8);
        LDM_REG(512, 9);
        LDM_REG(1024, 10);
        LDM_REG(2048, 11);
        LDM_REG(4096, 12);
        LDM_REG(8192, 13);
        LDM_REG(16384, 14);

        reg[15].I = CPUReadMemory(address);

        if(!(opcode & (1 << base)))    
          reg[base].I = temp;

        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
          ARM_PREFETCH;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
          THUMB_PREFETCH;
        }
      } else {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);

        if(armMode == 0x11) {
          LDM_REG(256, R8_FIQ);
          LDM_REG(512, R9_FIQ);
          LDM_REG(1024, R10_FIQ);
          LDM_REG(2048, R11_FIQ);
          LDM_REG(4096, R12_FIQ);
        } else {
          LDM_REG(256, 8);
          LDM_REG(512, 9);
          LDM_REG(1024, 10);
          LDM_REG(2048, 11);
          LDM_REG(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_REG(8192, R13_USR);
          LDM_REG(16384, R14_USR);
        } else {
          LDM_REG(8192, 13);
          LDM_REG(16384, 14);
        }

        if(!(opcode & (1 << base)))    
          reg[base].I = temp;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
    
  CASE_16(0x910)
    {
      // LDMDB Rn, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = temp & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      LDM_REG(1, 0);
      LDM_REG(2, 1);
      LDM_REG(4, 2);
      LDM_REG(8, 3);
      LDM_REG(16, 4);
      LDM_REG(32, 5);
      LDM_REG(64, 6);
      LDM_REG(128, 7);
      LDM_REG(256, 8);
      LDM_REG(512, 9);
      LDM_REG(1024, 10);
      LDM_REG(2048, 11);
      LDM_REG(4096, 12);
      LDM_REG(8192, 13);
      LDM_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address);

        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x930)
    {
      // LDMDB Rn!, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = temp & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      LDM_REG(1, 0);
      LDM_REG(2, 1);
      LDM_REG(4, 2);
      LDM_REG(8, 3);
      LDM_REG(16, 4);
      LDM_REG(32, 5);
      LDM_REG(64, 6);
      LDM_REG(128, 7);
      LDM_REG(256, 8);
      LDM_REG(512, 9);
      LDM_REG(1024, 10);
      LDM_REG(2048, 11);
      LDM_REG(4096, 12);
      LDM_REG(8192, 13);
      LDM_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address);

        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
      if(!(opcode & (1 << base)))
        reg[base].I = temp;
    }
    break;
  CASE_16(0x950)
    {
      // LDMDB Rn, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = temp & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);
        LDM_REG(256, 8);
        LDM_REG(512, 9);
        LDM_REG(1024, 10);
        LDM_REG(2048, 11);
        LDM_REG(4096, 12);
        LDM_REG(8192, 13);
        LDM_REG(16384, 14);

        reg[15].I = CPUReadMemory(address);

        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
          ARM_PREFETCH;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
          THUMB_PREFETCH;
        }
      } else {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);

        if(armMode == 0x11) {
          LDM_REG(256, R8_FIQ);
          LDM_REG(512, R9_FIQ);
          LDM_REG(1024, R10_FIQ);
          LDM_REG(2048, R11_FIQ);
          LDM_REG(4096, R12_FIQ);
        } else {
          LDM_REG(256, 8);
          LDM_REG(512, 9);
          LDM_REG(1024, 10);
          LDM_REG(2048, 11);
          LDM_REG(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_REG(8192, R13_USR);
          LDM_REG(16384, R14_USR);
        } else {
          LDM_REG(8192, 13);
          LDM_REG(16384, 14);
        }
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x970)
    {
      // LDMDB Rn!, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = temp & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);
        LDM_REG(256, 8);
        LDM_REG(512, 9);
        LDM_REG(1024, 10);
        LDM_REG(2048, 11);
        LDM_REG(4096, 12);
        LDM_REG(8192, 13);
        LDM_REG(16384, 14);

        reg[15].I = CPUReadMemory(address);

        if(!(opcode & (1 << base)))    
          reg[base].I = temp;

        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
          ARM_PREFETCH;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
          THUMB_PREFETCH;
        }
      } else {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);

        if(armMode == 0x11) {
          LDM_REG(256, R8_FIQ);
          LDM_REG(512, R9_FIQ);
          LDM_REG(1024, R10_FIQ);
          LDM_REG(2048, R11_FIQ);
          LDM_REG(4096, R12_FIQ);
        } else {
          LDM_REG(256, 8);
          LDM_REG(512, 9);
          LDM_REG(1024, 10);
          LDM_REG(2048, 11);
          LDM_REG(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_REG(8192, R13_USR);
          LDM_REG(16384, R14_USR);
        } else {
          LDM_REG(8192, 13);
          LDM_REG(16384, 14);
        }

        if(!(opcode & (1 << base)))    
          reg[base].I = temp;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
    
  CASE_16(0x990)
    {
      // LDMIB Rn, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = (reg[base].I+4) & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      LDM_REG(1, 0);
      LDM_REG(2, 1);
      LDM_REG(4, 2);
      LDM_REG(8, 3);
      LDM_REG(16, 4);
      LDM_REG(32, 5);
      LDM_REG(64, 6);
      LDM_REG(128, 7);
      LDM_REG(256, 8);
      LDM_REG(512, 9);
      LDM_REG(1024, 10);
      LDM_REG(2048, 11);
      LDM_REG(4096, 12);
      LDM_REG(8192, 13);
      LDM_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address);

        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x9b0)
    {
      // LDMIB Rn!, {Rlist}
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I + 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (reg[base].I+4) & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      LDM_REG(1, 0);
      LDM_REG(2, 1);
      LDM_REG(4, 2);
      LDM_REG(8, 3);
      LDM_REG(16, 4);
      LDM_REG(32, 5);
      LDM_REG(64, 6);
      LDM_REG(128, 7);
      LDM_REG(256, 8);
      LDM_REG(512, 9);
      LDM_REG(1024, 10);
      LDM_REG(2048, 11);
      LDM_REG(4096, 12);
      LDM_REG(8192, 13);
      LDM_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address);
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + dataTicksAccessSeq32(address);

        armNextPC = reg[15].I;
        reg[15].I += 4;
        ARM_PREFETCH;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
      if(!(opcode & (1 << base)))
        reg[base].I = temp;
    }
    break;    
  CASE_16(0x9d0)
    {
      // LDMIB Rn, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 address = (reg[base].I+4) & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);
        LDM_REG(256, 8);
        LDM_REG(512, 9);
        LDM_REG(1024, 10);
        LDM_REG(2048, 11);
        LDM_REG(4096, 12);
        LDM_REG(8192, 13);
        LDM_REG(16384, 14);

        reg[15].I = CPUReadMemory(address);

        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
          ARM_PREFETCH;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
          THUMB_PREFETCH;
        }
      } else {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);

        if(armMode == 0x11) {
          LDM_REG(256, R8_FIQ);
          LDM_REG(512, R9_FIQ);
          LDM_REG(1024, R10_FIQ);
          LDM_REG(2048, R11_FIQ);
          LDM_REG(4096, R12_FIQ);
        } else {
          LDM_REG(256, 8);
          LDM_REG(512, 9);
          LDM_REG(1024, 10);
          LDM_REG(2048, 11);
          LDM_REG(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_REG(8192, R13_USR);
          LDM_REG(16384, R14_USR);
        } else {
          LDM_REG(8192, 13);
          LDM_REG(16384, 14);
        }
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;
  CASE_16(0x9f0)
    {
      // LDMIB Rn!, {Rlist}^
      if (busPrefetchCount==0)
        busPrefetch = busPrefetchEnable;
      int base = (opcode & 0x000F0000) >> 16;
      uint32 temp = reg[base].I +
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      uint32 address = (reg[base].I+4) & 0xFFFFFFFC;
      clockTicks = 0;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);
        LDM_REG(256, 8);
        LDM_REG(512, 9);
        LDM_REG(1024, 10);
        LDM_REG(2048, 11);
        LDM_REG(4096, 12);
        LDM_REG(8192, 13);
        LDM_REG(16384, 14);

        reg[15].I = CPUReadMemory(address);

        if(!(opcode & (1 << base)))    
          reg[base].I = temp;

        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
          ARM_PREFETCH;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
          THUMB_PREFETCH;
        }
      } else {
        LDM_REG(1, 0);
        LDM_REG(2, 1);
        LDM_REG(4, 2);
        LDM_REG(8, 3);
        LDM_REG(16, 4);
        LDM_REG(32, 5);
        LDM_REG(64, 6);
        LDM_REG(128, 7);

        if(armMode == 0x11) {
          LDM_REG(256, R8_FIQ);
          LDM_REG(512, R9_FIQ);
          LDM_REG(1024, R10_FIQ);
          LDM_REG(2048, R11_FIQ);
          LDM_REG(4096, R12_FIQ);
        } else {
          LDM_REG(256, 8);
          LDM_REG(512, 9);
          LDM_REG(1024, 10);
          LDM_REG(2048, 11);
          LDM_REG(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_REG(8192, R13_USR);
          LDM_REG(16384, R14_USR);
        } else {
          LDM_REG(8192, 13);
          LDM_REG(16384, 14);
        }

        if(!(opcode & (1 << base)))    
          reg[base].I = temp;
        if (!offset)
          clockTicks += 1 + dataTicksAccesint32(address);
        else
          clockTicks += 1 + dataTicksAccessSeq32(address);
        clockTicks += 1 + codeTicksAccessSeq32(armNextPC);
      }
      clockTicks += 2 + codeTicksAccesint32(armNextPC);
    }
    break;    
  CASE_256(0xa00)
    {
      // B <offset>
      int offset = opcode & 0x00FFFFFF;
      if(offset & 0x00800000) {
        offset |= 0xFF000000;
      }
      offset <<= 2;
      reg[15].I += offset;
      armNextPC = reg[15].I;
      reg[15].I += 4;
      ARM_PREFETCH;
      clockTicks = codeTicksAccessSeq32(armNextPC) + 1;
      clockTicks += 2 + codeTicksAccesint32(armNextPC) +
          codeTicksAccessSeq32(armNextPC);
      busPrefetchCount=0;
    }
    break;
  CASE_256(0xb00)
    {
      // BL <offset>
      int offset = opcode & 0x00FFFFFF;
      if(offset & 0x00800000) {
        offset |= 0xFF000000;
      }
      offset <<= 2;
      reg[14].I = reg[15].I - 4;
      reg[15].I += offset;
      armNextPC = reg[15].I;
      reg[15].I += 4;
      ARM_PREFETCH;
      clockTicks = codeTicksAccessSeq32(armNextPC) + 1;
      clockTicks += 2 + codeTicksAccesint32(armNextPC) +
          codeTicksAccessSeq32(armNextPC);
      busPrefetchCount=0;
    }
    break;
  CASE_256(0xf00)
    // SWI <comment>
    clockTicks = codeTicksAccessSeq32(armNextPC) + 1;
    clockTicks += 2 + codeTicksAccesint32(armNextPC) +
        codeTicksAccessSeq32(armNextPC);
    busPrefetchCount=0;
    CPUSoftwareInterrupt(opcode & 0x00FFFFFF);  

    break;
#ifdef GP_SUPPORT
  case 0xe11:
  case 0xe13:
  case 0xe15:
  case 0xe17:
  case 0xe19:
  case 0xe1b:
  case 0xe1d:
  case 0xe1f:
    // MRC
    break;
  case 0xe01:
  case 0xe03:
  case 0xe05:
  case 0xe07:
  case 0xe09:
  case 0xe0b:
  case 0xe0d:
  case 0xe0f:
    // MRC
    break;    
#endif
  default:
    CPUUndefinedException();
    break;
    // END
  }
}

 if (clockTicks == 0)
  clockTicks = codeTicksAccessSeq32(oldArmNextPC) + 1;

 return(clockTicks);
}
