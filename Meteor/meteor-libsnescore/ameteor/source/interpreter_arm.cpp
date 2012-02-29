// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __INTERPRETER_ARM_H__
#define __INTERPRETER_ARM_H__

/*

- From GBATEK -

ARM Binary Opcode Format

|..3 ..................2 ..................1 ..................0|
|1_0_9_8_7_6_5_4_3_2_1_0_9_8_7_6_5_4_3_2_1_0_9_8_7_6_5_4_3_2_1_0|
|_Cond__|0_0_0|___Op__|S|__Rn___|__Rd___|__Shift__|Typ|0|__Rm___| DataProc
|_Cond__|0_0_0|___Op__|S|__Rn___|__Rd___|__Rs___|0|Typ|1|__Rm___| DataProc
|_Cond__|0_0_1|___Op__|S|__Rn___|__Rd___|_Shift_|___Immediate___| DataProc
|_Cond__|0_0_1_1_0|P|1|0|_Field_|__Rd___|_Shift_|___Immediate___| PSR Imm
|_Cond__|0_0_0_1_0|P|L|0|_Field_|__Rd___|0_0_0_0|0_0_0_0|__Rm___| PSR Reg
|_Cond__|0_0_0_1_0_0_1_0_1_1_1_1_1_1_1_1_1_1_1_1|0_0|L|1|__Rn___| BX,BLX
|1_1_1_0|0_0_0_1_0_0_1_0|_____immediate_________|0_1_1_1|_immed_| BKPT ARM9
|_Cond__|0_0_0_1_0_1_1_0_1_1_1_1|__Rd___|1_1_1_1|0_0_0_1|__Rm___| CLZ ARM9
|_Cond__|0_0_0_1_0|Op_|0|__Rn___|__Rd___|0_0_0_0|0_1_0_1|__Rm___| QALU ARM9
|_Cond__|0_0_0_0_0_0|A|S|__Rd___|__Rn___|__Rs___|1_0_0_1|__Rm___| Multiply
|_Cond__|0_0_0_0_1|U|A|S|_RdHi__|_RdLo__|__Rs___|1_0_0_1|__Rm___| MulLong
|_Cond__|0_0_0_1_0|Op_|0|Rd/RdHi|Rn/RdLo|__Rs___|1|y|x|0|__Rm___| MulHalf
|_Cond__|0_0_0_1_0|B|0_0|__Rn___|__Rd___|0_0_0_0|1_0_0_1|__Rm___| TransSwp12
|_Cond__|0_0_0|P|U|0|W|L|__Rn___|__Rd___|0_0_0_0|1|S|H|1|__Rm___| TransReg10
|_Cond__|0_0_0|P|U|1|W|L|__Rn___|__Rd___|OffsetH|1|S|H|1|OffsetL| TransImm10
|_Cond__|0_1_0|P|U|B|W|L|__Rn___|__Rd___|_________Offset________| TransImm9
|_Cond__|0_1_1|P|U|B|W|L|__Rn___|__Rd___|__Shift__|Typ|0|__Rm___| TransReg9
|_Cond__|0_1_1|________________xxx____________________|1|__xxx__| Undefined
|_Cond__|1_0_0|P|U|S|W|L|__Rn___|__________Register_List________| BlockTrans
|_Cond__|1_0_1|L|___________________Offset______________________| B,BL,BLX
|_Cond__|1_1_0|P|U|N|W|L|__Rn___|__CRd__|__CP#__|____Offset_____| CoDataTrans
|_Cond__|1_1_0_0_0_1_0|L|__Rn___|__Rd___|__CP#__|_CPopc_|__CRm__| CoRR ARM9
|_Cond__|1_1_1_0|_CPopc_|__CRn__|__CRd__|__CP#__|_CP__|0|__CRm__| CoDataOp
|_Cond__|1_1_1_0|CPopc|L|__CRn__|__Rd___|__CP#__|_CP__|1|__CRm__| CoRegTrans
|_Cond__|1_1_1_1|_____________Ignored_by_Processor______________| SWI

*/

#include "ameteor/interpreter.hpp"
#include "globals.hpp"
#include "cpu_globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

#define Rn ((code >> 16) & 0xF)
#define Rd ((code >> 12) & 0xF)
#define Rs ((code >>  8) & 0xF)
#define Rm  (code        & 0xF)

// Load/Store immediate offset
#define LSOff (code & 0xFFF)

#ifdef METDEBUG
#	define NOT_PC(reg) \
		if (reg == 15) \
			met_abort("Register is PC")
#	define NOT_PC_ALL() \
		NOT_PC(Rn); \
		NOT_PC(Rd); \
		NOT_PC(Rs); \
		NOT_PC(Rm)
#	define NOT_SAME2(reg1, reg2) \
		if (reg1 == reg2) \
			met_abort("Two same registers")
#	define NOT_SAME3(reg1, reg2, reg3) \
		NOT_SAME2(reg1, reg2); \
		NOT_SAME2(reg2, reg3); \
		NOT_SAME2(reg1, reg3)
#else
#	define NOT_PC(reg) {}
#	define NOT_PC_ALL() {}
#	define NOT_SAME2(reg1, reg2) {}
#	define NOT_SAME3(reg1, reg2, reg3) {}
#endif

#define ARM(name) \
	inline void Interpreter::a##name ()
#define NIARM(name) \
	void Interpreter::a##name ()

namespace AMeteor
{
	// Branch and Exchange
	ARM(BXBLX)
	{
		if ((code & 0x0FFFFF00) != 0x012FFF00)
			met_abort("Bits 8-27 must be 0001 00101111 11111111 for BX/BLX instructions");

		if (Rm == 15)
			met_abort("Branching on PC is undefined");

		if (code & (0x1 << 5)) // BLX
		{
			R(14) = R(15);
			met_abort("BLX not completly implemented");
		}
		// else BX

		if (R(Rm) & 0x1)
		{
			FLAG_T = 1;
			R(15) = R(Rm) + 1;
			CYCLES32NSeq(R(15), 3);
		}
		else
		{
			if (R(Rm) & 0x3)
				met_abort("BX with ARM on non 32 bit aligned address");
			R(15) = R(Rm)+4;
			CYCLES32NSeq(R(15), 3);
		}
	}

	// Branch and Branch with Link
	ARM(BBL)
	{
		if (((code >> 25) & 0x7) != 0x5)
			met_abort("Bits 25-27 must be 101 for B/BL/BLX instructions");

		if (((code >> 28) & 0xF) == 0xF)
			met_abort("BLX not implemented");

		int32_t off = code & 0x00FFFFFF;
		// Extend the sign bit
		off <<= 8;
		off >>= 8;
		// off is in steps of 4
		off *= 4;

		if (code & (0x1 << 24)) // BL
		{
			// R(15) points two instructions later, R(14) should point to next
			// instruction
			// LR = PC + 4 and R15 = PC + 8
			R(14) = R(15) - 4 ;
		}
		// else B
		R(15) += off + 4;
		CYCLES32NSeq(R(15), 3);
	}

	// Data Processing
	NIARM(_DataProcShiftImm)
	{
		if ((code >> 26) & 0x3)
			met_abort("Bits 26-27 must be 00 for DataProc instructions");

		uint8_t shift;
		uint32_t op2 = 0; // to avoid a warning
		bool shiftcarry = FLAG_C;
		uint8_t rd = Rd;

		uint8_t rm = Rm;
		if (rm == 15 && !(code & (0x1 << 20)) && (code & (0x1 << 4)))
			met_abort("Rm = 15, not implemented");

		shift = (code >> 7) & 0x1F;
		switch ((code >> 5) & 0x3)
		{
			case 0: // Logical Shift Left
				if (shift)
				{
					op2 = R(rm) << shift;
					shiftcarry = (R(rm) >> (32-shift)) & 0x1;
				}
				else // LSL#0
					op2 = R(rm);
				break;
			case 1: // Logical Shift Right
				if (shift)
				{
					op2 = R(rm) >> shift;
					shiftcarry = (R(rm) >> (shift-1)) & 0x1;
				}
				else // LSR#32
				{
					op2 = 0;
					shiftcarry = R(rm) >> 31;
				}
				break;
			case 2: // Arithmetic Shift Right
				if (shift)
				{
					op2 = ((int32_t)R(rm)) >> shift;
					shiftcarry = (((int32_t)R(rm)) >> (shift-1)) & 0x1;
				}
				else // ASR#32
				{
					op2 = ((int32_t)R(rm)) >> 31;
					shiftcarry = op2 & 0x1;
				}
				break;
			case 3: // ROtate Right
				if (shift)
				{
					op2 = ROR(R(rm), shift);
					shiftcarry = op2 >> 31;
				}
				else // RRX#1
				{
					shiftcarry = R(rm) & 0x1;
					op2 = (FLAG_C << 31) | (R(rm) >> 1);
				}
				break;
		}

		a_DataProcCore(rd, R(Rn), op2, shiftcarry);
	}

	NIARM(_DataProcShiftReg)
	{
		if ((code >> 26) & 0x3)
			met_abort("Bits 26-27 must be 00 for DataProc instructions");
		if (code & (0x1 << 7))
			met_abort("Bit 7 must be 0 for DataProc with shift by register instructions");

		uint8_t shift;
		uint32_t op1, op2 = 0; // to avoid a warning
		bool shiftcarry = FLAG_C;
		uint8_t rd = Rd;

		uint8_t rm = Rm;
		if (rm == 15 && !(code & (0x1 << 20)) && (code & (0x1 << 4)))
			met_abort("Rm = 15, not implemented");

		op1 = R(Rn);

		NOT_PC(Rs);
		ICYCLES(1);
		if (Rn == 15)
			op1 += 4;

		shift = R(Rs) & 0xFF; // only first byte used
		if (shift)
			switch ((code >> 5) & 0x3)
			{
				case 0: // Logical Shift Left
					if (shift == 32)
					{
						op2 = 0;
						shiftcarry = R(rm) & 0x1;
					}
					else if (shift < 32)
					{
						op2 = R(rm) << shift;
						shiftcarry = (R(rm) >> (32-shift)) & 0x1;
					}
					else
					{
						op2 = 0;
						shiftcarry = 0;
					}
					break;
				case 1: // Logical Shift Right
					if (shift == 32)
					{
						op2 = 0;
						shiftcarry = R(rm) >> 31;
					}
					else if (shift < 32)
					{
						op2 = R(rm) >> shift;
						shiftcarry = (R(rm) >> (shift-1)) & 0x1;
					}
					else
					{
						op2 = 0;
						shiftcarry = 0;
					}
					break;
				case 2: // Arithmetic Shift Right
					if (shift >= 32)
					{
						op2 = ((int32_t)R(rm)) >> 31;
						shiftcarry = op2 & 0x1;
					}
					else
					{
						op2 = ((int32_t)R(rm)) >> shift;
						shiftcarry = (((int32_t)R(rm)) >> (shift-1)) & 0x1;
					}
					break;
				case 3: // ROtate Right
					op2 = ROR(R(rm), shift % 32);
					shiftcarry = op2 >> 31;
					break;
			}
		else
			op2 = R(rm);

		a_DataProcCore(rd, op1, op2, shiftcarry);
	}

	NIARM(_DataProcImm)
	{
		if ((code >> 26) & 0x3)
			met_abort("Bits 26-27 must be 00 for DataProc instructions");

		uint32_t op2;
		bool shiftcarry = FLAG_C;
		uint8_t rd = Rd;

		uint8_t shift = ((code >> 8) & 0xF);
		if (shift)
		{
			op2 = ROR(code & 0xFF, shift * 2);
			shiftcarry = op2 >> 31;
		}
		else
			op2 = code & 0xFF;

		a_DataProcCore(rd, R(Rn), op2, shiftcarry);
	}

	inline void Interpreter::a_DataProcCore(uint8_t rd,
			uint32_t op1, uint32_t op2, bool shiftcarry)
	{
		uint8_t opcode = (code >> 21) & 0xF;

		if (opcode < 0x8 || opcode > 0xB)
		{
		}
		else if (!((code >> 20) & 0x1) || (rd != 0x0 && rd != 0xF))
			met_abort("Set condition bit not set for test operation or Rd not acceptable for a test");
		if ((opcode == 0xD || opcode == 0xF) && Rn)
			met_abort("Rn not null for MOV or MVN");

#ifndef X86_ASM
		uint32_t res;
#endif

		if (code & (0x1 << 20)) // if set condition
		{
			switch (opcode)
			{
				case 0x0 : // AND
#ifdef X86_ASM
					asm("andl %4, %3\n"
							"setzb %1\n"
							"setsb %2\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N)
							:"0"(op1), "r"(op2));
#else
					res = R(rd) = op1 & op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
#endif
					FLAG_C = shiftcarry;
					break;
				case 0x1 : // EOR
#ifdef X86_ASM
					asm("xorl %4, %3\n"
							"setzb %1\n"
							"setsb %2\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N)
							:"0"(op1), "r"(op2));
#else
					res = R(rd) = op1 ^ op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
#endif
					FLAG_C = shiftcarry;
					break;
				case 0x2 : // SUB
#ifdef X86_ASM
					asm("subl %6, %5\n"
							"setzb %1\n"
							"setsb %2\n"
							"setncb %3\n"
							"setob %4\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C),
							"=m"(FLAG_V)
							:"0"(op1), "r"(op2));
#else
					res = R(rd) = op1 - op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
#endif
					break;
				case 0x3 : // RSB
#ifdef X86_ASM
					asm("subl %6, %5\n"
							"setzb %1\n"
							"setsb %2\n"
							"setncb %3\n"
							"setob %4\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C),
							"=m"(FLAG_V)
							:"0"(op2), "r"(op1));
#else
					res = R(rd) = op2 - op1;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
#endif
					break;
				case 0x4 : // ADD
#ifdef X86_ASM
					asm("addl %6, %5\n"
							"setzb %1\n"
							"setsb %2\n"
							"setcb %3\n"
							"setob %4\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C),
							"=m"(FLAG_V)
							:"0"(op1), "r"(op2));
#else
					res = R(rd) = op1 + op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = ADDCARRY(op1, op2, res);
					FLAG_V = ADDOVERFLOW(op1, op2, res);
#endif
					break;
				case 0x5 : // ADC
					// TODO test on hardware how overflow and carry work for this
					// instruction
#ifdef X86_ASM
					asm("addl %6, %5\n"
							"setzb %1\n"
							"setsb %2\n"
							"setcb %3\n"
							"setob %4\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C),
							"=m"(FLAG_V)
							:"0"(op1+FLAG_C), "r"(op2));
#else
					res = R(rd) = op1 + op2 + FLAG_C;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = ADDCARRY(op1, op2, res);
					FLAG_V = ADDOVERFLOW(op1, op2, res);
#endif
					break;
				case 0x6 : // SBC
					// TODO test on hardware how overflow and carry work for this
					// instruction
#ifdef X86_ASM
					asm("subl %6, %5\n"
							"setzb %1\n"
							"setsb %2\n"
							"setncb %3\n"
							"setob %4\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C),
							"=m"(FLAG_V)
							:"0"(op1+FLAG_C-1), "r"(op2));
#else
					res = R(rd) = op1 - op2 + FLAG_C - 1;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
#endif
					break;
				case 0x7 : // RSC
#ifdef X86_ASM
					asm("subl %6, %5\n"
							"setzb %1\n"
							"setsb %2\n"
							"setncb %3\n"
							"setob %4\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C),
							"=m"(FLAG_V)
							:"0"(op2+FLAG_C-1), "r"(op1));
#else
					res = R(rd) = op2 - op1 + FLAG_C - 1;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
#endif
					break;
				case 0x8 : // TST
#ifdef X86_ASM
					asm("testl %3, %2\n"
							"setzb %0\n"
							"setsb %1\n"
							:"=m"(FLAG_Z), "=m"(FLAG_N)
							:"r"(op1), "r"(op2));
#else
					res = op1 & op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
#endif
					SETFB(C, shiftcarry);
					break;
				case 0x9 : // TEQ
#ifdef X86_ASM
					asm("xorl %3, %2\n"
							"setzb %0\n"
							"setsb %1\n"
							:"=m"(FLAG_Z), "=m"(FLAG_N)
							:"r"(op1), "r"(op2)
							:"2");
#else
					res = op1 ^ op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
#endif
					SETFB(C, shiftcarry);
					break;
				case 0xA : // CMP
#ifdef X86_ASM
					asm("cmpl %5, %4\n"
							"setzb %0\n"
							"setsb %1\n"
							"setncb %2\n"
							"setob %3\n"
							:"=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
							:"r"(op1), "r"(op2));
#else
					res = op1 - op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
#endif
					break;
				case 0xB : // CMN
#ifdef X86_ASM
					asm("addl %5, %4\n"
							"setzb %0\n"
							"setsb %1\n"
							"setcb %2\n"
							"setob %3\n"
							:"=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
							:"r"(op1), "r"(op2)
							:"4");
#else
					res = op1 + op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = ADDCARRY(op1, op2, res);
					FLAG_V = ADDOVERFLOW(op1, op2, res);
#endif
					break;
				case 0xC : // ORR
#ifdef X86_ASM
					asm("orl %4, %3\n"
							"setzb %1\n"
							"setsb %2\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N)
							:"0"(op1), "r"(op2));
#else
					res = R(rd) = op1 | op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
#endif
					FLAG_C = shiftcarry;
					break;
				case 0xD : // MOV
#ifdef X86_ASM
					R(rd) = op2;
					asm("testl %2, %2\n"
							"setzb %0\n"
							"setsb %1\n"
							:"=m"(FLAG_Z), "=m"(FLAG_N)
							:"r"(op2));
#else
					res = R(rd) = op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
#endif
					FLAG_C = shiftcarry;
					break;
				case 0xE : // BIC
#ifdef X86_ASM
					asm("not %3\n"
							"andl %4, %3\n"
							"setzb %1\n"
							"setsb %2\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N)
							:"0"(op2), "r"(op1));
#else
					res = R(rd) = op1 & ~op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
#endif
					FLAG_C = shiftcarry;
					break;
				case 0xF : // MVN
#ifdef X86_ASM
					asm("xorl $0xffffffff, %3\n"
							"setzb %1\n"
							"setsb %2\n"
							:"=r"(R(rd)), "=m"(FLAG_Z), "=m"(FLAG_N)
							:"0"(op2));
#else
					res = R(rd) = ~op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
#endif
					FLAG_C = shiftcarry;
					break;
			}

			if (rd == 15)
				CPU.SwitchModeBack();
		}
		else
			switch (opcode)
			{
				case 0x0 : // AND
					R(rd) = op1 & op2;
					break;
				case 0x1 : // EOR
					R(rd) = op1 ^ op2;
					break;
				case 0x2 : // SUB
					R(rd) = op1 - op2;
					break;
				case 0x3 : // RSB
					R(rd) = op2 - op1;
					break;
				case 0x4 : // ADD
					R(Rd) = op1 + op2;
					break;
				case 0x5 : // ADC
					R(rd) = op1 + op2 + FLAG_C;
					break;
				case 0x6 : // SBC
					R(rd) = op1 - op2 + FLAG_C - 1;
					break;
				case 0x7 : // RSC
					R(rd) = op2 - op1 + FLAG_C - 1;
					break;
				case 0x8 : // TST
				case 0x9 : // TEQ
				case 0xA : // CMP
				case 0xB : // CMN
					met_abort("Comparison or test without set flags bit");
					break;
				case 0xC : // ORR
					R(rd) = op1 | op2;
					break;
				case 0xD : // MOV
					R(rd) = op2;
					break;
				case 0xE : // BIC
					R(rd) = op1 & ~op2;
					break;
				case 0xF : // MVN
					R(rd) = ~op2;
					break;
			}

		if (rd == 15 && (opcode < 0x8 || opcode > 0xB))
		{
			if (FLAG_T)
			{
				CYCLES16NSeq(R(15), 3);
				R(15) += 2;
			}
			else
			{
				CYCLES32NSeq(R(15), 3);
				R(15) += 4;
			}
		}
		else
			CYCLES32Seq(R(15), 1);

		if (opcode >= 0x8 && opcode <= 0xB && rd == 0xF)
			met_abort("P test instruction (not implemented)");
	}

	// PSR Transfer (MRS, MSR)
	ARM(PSR)
	{
		if ((code >> 26) & 0x3)
			met_abort("Bits 26-27 must be 00 for PSR instructions");
		if (((code >> 23) & 0x3) != 0x2)
			met_abort("Bits 23-24 must be 10 for PSR instructions");
		if (code & (0x1 << 20))
			met_abort("Bit 20 must be 0 for PSR instructions");

		bool oncpsr = !(code & (0x1 << 22));
		if (oncpsr)
			CPU.UpdateCpsr();
		uint32_t& psr = oncpsr ? CPSR : SPSR;

		if (code & (0x1 << 21)) // MSR
		{
			if (((code >> 12) & 0xF) != 0xF)
				met_abort("Bits 12-15 must be 0xF for MSR instruction");

			uint32_t val;
			if (code & (0x1 << 25))
			{
				//val = ROR(code & 0xF, ((code >> 8) & 0x4) * 2);
				val = ROR(code & 0xF, (code >> 7) & 0x6);
			}
			else
			{
				if ((code >> 4) & 0xFF)
					met_abort("Bits 4-11 must be 0 for MSR instruction");
				val = R(Rm);
			}
			if (!(code & (0x1 << 19)))
				val = (val & 0x00FFFFFF) | (psr & 0xFF000000);
			if (!(code & (0x1 << 18)))
				val = (val & 0xFF00FFFF) | (psr & 0x00FF0000);
			if (!(code & (0x1 << 17)))
				val = (val & 0xFFFF00FF) | (psr & 0x0000FF00);
			if (!(code & (0x1 << 16)))
				val = (val & 0xFFFFFF00) | (psr & 0x000000FF);
			else if (oncpsr &&
					(psr & 0x1F) != (val & 0x1F)) // have we changed mode ?
				CPU.SwitchToMode(val & 0x1F);
			psr = val;
			if (oncpsr)
			{
				CPU.UpdateICpsr();
				CPU.CheckInterrupt();
			}
		}
		else // MRS
		{
			if ((code >> 25) & 0x1)
				met_abort("Bit 25 must be 0 for MRS instruction");
			if (((code >> 16) & 0xF) != 0xF)
				met_abort("Bits 16-19 must be 0xF for MRS instruction");
			if (code & 0xFFF)
				met_abort("Bits 0-11 must be 0 for MRS instruction");
			R(Rd) = psr;
		}

		CYCLES32Seq(R(15), 1);
	}

	// Multiply and Multiply-Accumulate (MUL,MLA)
	ARM(_Multiply)
	{
		// NOTE : In this instruction Rn and Rd are inverted
		if ((code >> 25) & 0x7)
			met_abort("Bits 25-27 must be 000 for Multiply instructions");
		if (code & (0x1 << 24))
		{
			if (!(code & (0x1 << 7)))
				met_abort("Bit 7 must be 1 for halfword multiply");
			if (code & (0x1 << 4))
				met_abort("Bit 7 must be 0 for halfword multiply");
		}
		else
		{
			if (((code >> 4) & 0xF) != 0x9)
				met_abort("Bits 4-7 must be 1001 for non halfword multiplies");
		}
		NOT_PC_ALL();

		switch ((code >> 21) & 0xF)
		{
			case 0x0 : // MUL
				if (Rd != 0)
					met_abort("Rd must be 0 for MUL instructions");
				NOT_SAME2(Rn, Rm);
				R(Rn) = R(Rm)*R(Rs);
				if (code & (0x1 << 20))
				{
					FZ(R(Rn));
					FN(R(Rn));
				}
				MULICYCLES(Rs);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x1 : // MLA
				NOT_SAME2(Rn, Rm);
				R(Rn) = R(Rm)*R(Rs)+R(Rd);
				if (code & (0x1 << 20))
				{
					FZ(R(Rn));
					FN(R(Rn));
				}
				MULICYCLES(Rs);
				ICYCLES(1);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x4 : // UMULL
				{
					NOT_SAME3(Rn, Rd, Rm);
					uint64_t out = ((uint64_t)R(Rm))*((uint64_t)R(Rs));
					R(Rn) = out >> 32;
					R(Rd) = out & 0xFFFFFFFF;
					if (code & (0x1 << 20))
					{
						FZ(out);
						FN(R(Rn));
					}
				}
				MULICYCLES(Rs);
				ICYCLES(1);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x5 : // UMLAL
				{
					NOT_SAME3(Rn, Rd, Rm);
					uint64_t out =
						((uint64_t)R(Rm)) * ((uint64_t)R(Rs)) +
						((((uint64_t)R(Rn)) << 32) | ((uint64_t)R(Rd)));
					R(Rn) = out >> 32;
					R(Rd) = out & 0xFFFFFFFF;
					if (code & (0x1 << 20))
					{
						FZ(out);
						FN(R(Rn));
					}
				}
				MULICYCLES(Rs);
				ICYCLES(2);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x6 : // SMULL
				{
					NOT_SAME3(Rn, Rd, Rm);
					int64_t out = ((int64_t)(int32_t)R(Rm)) * ((int64_t)(int32_t)R(Rs));
					R(Rn) = out >> 32;
					R(Rd) = out & 0xFFFFFFFF;
					if (code & (0x1 << 20))
					{
						FZ(out);
						FN(R(Rn));
					}
				}
				MULICYCLES(Rs);
				ICYCLES(1);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x7 : // SMLAL
				{
					NOT_SAME3(Rn, Rd, Rm);
					int64_t out = ((int64_t)(int32_t)R(Rm)) * ((int64_t)(int32_t)R(Rs))
						+ ((((int64_t)R(Rn)) << 32) | ((int64_t)R(Rd)));
					R(Rn) = out >> 32;
					R(Rd) = out & 0xFFFFFFFF;
					if (code & (0x1 << 20))
					{
						FZ(out);
						FN(R(Rn));
					}
				}
				MULICYCLES(Rs);
				ICYCLES(2);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x8 : // SMLAxy
			case 0x9 : // SMLAW/SMULW
			case 0xA : // SMLALxy
			case 0xB : // SMULxy
			default :
				met_abort("Not implemented multiply instruction or unknown");
		}
	}

	// Single Data Transfer (LDR, STR, PLD)

	// Load and store
	// FIXME : should this support Prepare Cache for Load instructions ?
	ARM(LDRSTR)
	{
		if (((code >> 28) & 0xF) == 0xF)
			met_abort("PLD instructions not implemented");
		if (((code >> 26) & 0x3) != 0x1)
			met_abort("Bits 26-27 must be 01 for LDR/STR instructions");

		uint32_t offset;

		if (code & (0x1 << 25)) // register offset
		{
			if (code & (0x1 << 4))
				met_abort("Bit 4 must be 0 for LDR or STR instruction with register offset");
			offset = (code >> 7) & 0x1F;
			switch ((code >> 5) & 0x3)
			{
				case 0: // Logical Shift Left
					if (offset)
						offset = R(Rm) << offset;
					else
						offset = R(Rm);
					break;
				case 1: // Logical Shift Right
					if (offset)
						offset = R(Rm) >> offset;
					else
						offset = 0;
					break;
				case 2: // Arithmetic Shift Right
					if (offset)
						offset = ((int32_t)R(Rm)) >> offset;
					else
					{
						if (R(Rm) >> 31)
							offset = 0xFFFFFFFF;
						else
							offset = 0;
					}
					break;
				case 3: // ROtate Right
					if (offset)
						offset = ROR(R(Rm), offset);
					else
						offset = (FLAG_C << 31) | (R(Rm) >> 1);
					break;
			}
		}
		else // immediate offset
		{
			offset = code & 0xFFF;
		}

		// bit 24 : 0 = add offset after and write-back, 1 = add offset before
		uint32_t add = R(Rn);
		if (code & (0x1 << 24))
		{
			if (code & (0x1 << 23))
				add += offset;
			else
				add -= offset;
		}

		/* bit 22 : 0 = write word, 1 = write byte
		 * bit 20 : 0 = store, 1 = load */
		if (code & (0x1 << 22))
		{
			if (code & (0x1 << 20)) // LDRB
			{
				R(Rd) = MEM.Read8(add);
				if (Rd == 15)
					met_abort("LDRB to R15 !");
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES32Seq(R(15), 1);
			}
			else // STRB
			{
				MEM.Write8(add, R(Rd));
				CYCLES16NSeq(add, 1);
				CYCLES32NSeq(R(15), 1);
			}
		}
		else
		{
			if (code & (0x1 << 20)) // LDR
			{
				R(Rd) = MEM.Read32(add);
				CYCLES32NSeq(add, 1);
				ICYCLES(1);
				if (Rd == 15)
				{
					CYCLES32NSeq(R(15), 3);
					R(Rd) += 4;
				}
				else
					CYCLES32Seq(R(15), 1);
			}
			else // STR
			{
				MEM.Write32(add, R(Rd));
				CYCLES32NSeq(add, 1);
				CYCLES32NSeq(R(15), 1);
			}
		}

		// bit 21 if write before : 0 = nothing, 1 = write-back
		if (!(code & (0x1 << 24))) // in post, writeback is always enabled
		{
			if (code & (0x1 << 23))
				R(Rn) = add + offset;
			else
				R(Rn) = add - offset;
		}
		else if (code & (0x1 << 21))
			R(Rn) = add;
	}

	// Halfword, Doubleword, and Signed Data Transfer
	ARM(STRLDR_HD)
	{
		if ((code >> 25) & 0x7)
			met_abort("Bits 25-27 must be 000 for halfword transfer instructions");
		if (!(code & (0x1 << 7)) || !(code & (0x1 << 4)))
			met_abort("Bits 4 and 7 must be 1 for halfword transfer instructions");
		if (Rd == 15)
			met_abort("operation on r15, not implemented");

		uint8_t rd = Rd;

		uint32_t off;
		if (code & (0x1 << 22)) // immediate offset
			off = ((code >> 4) & 0xF0) | (code & 0xF);
		else // register offset
		{
			if ((code >> 8) & 0xF)
				met_abort("Bits 8-11 must be 0 for halfword transfer with register offset instructions");
			NOT_PC(Rm);
			off = R(Rm);
		}

		uint32_t add = R(Rn);
		if (code & (0x1 << 24))
		{
			if (code & (0x1 << 23))
				add += off;
			else
				add -= off;
		}
		else if (code & (0x1 << 21))
			met_abort("Bit 21 must be 0 for post indexed halfword transfers instructions");

		switch (((code >> 18) & 0x4) | ((code >> 5) & 0x3))
		{
			case 0x0:
				met_abort("Reserved for SWP instruction !");
				break;
			case 0x1: // STRH
				MEM.Write16(add, rd == 15 ? R(15) + 4 : R(rd));
				CYCLES16NSeq(add, 1);
				CYCLES32NSeq(R(15), 1);
				break;
			case 0x2: // LDRD
				if (rd % 2)
					met_abort("Register number not even for double word transfer");
				if (add % 8)
					met_abort("Address not double word aligned");
				if (rd == 15)
					met_abort("Rd is 15 for double word transfer !");
				R(rd) = MEM.Read32(add);
				R(rd+1) = MEM.Read32(add+4);
				CYCLES32NSeq(add, 2);
				ICYCLES(1);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x3: // STRD
				if (rd % 2)
					met_abort("Register number not even for double word transfer");
				if (add % 8)
					met_abort("Address not double word aligned");
				if (rd == 15)
					met_abort("Rd is 15 for double word transfer !");
				MEM.Write32(add, R(rd));
				MEM.Write32(add + 4, rd == 14 ? R(15) + 4 : R(rd+1));
				CYCLES32NSeq(add, 2);
				CYCLES32NSeq(R(15), 1);
				break;
			case 0x4:
				met_abort("Reserved !");
				break;
			case 0x5: // LDRH
				R(rd) = MEM.Read16(add);
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x6: // LDRSB
				R(rd) = MEM.Read8(add);
				// sign-extend
				R(rd) <<= 24;
				R(rd) = ((int32_t)R(rd)) >> 24;
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES32Seq(R(15), 1);
				break;
			case 0x7: // LDRSH
				R(rd) = MEM.Read16(add);
				// sign-extend
				R(rd) <<= 16;
				R(rd) = ((int32_t)R(rd)) >> 16;
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES32Seq(R(15), 1);
				break;
		}

		if (!(code & (0x1 << 24))) // in post, writeback is always enabled
		{
			if (code & (0x1 << 23))
				R(Rn) = add + off;
			else
				R(Rn) = add - off;
		}
		else if (code & (0x1 << 21))
			R(Rn) = add;
	}

	// Block Data Transfer (LDM,STM)
	ARM(LDMSTM)
	{
		if (((code >> 25) & 0x7) != 0x4)
			met_abort("Bits 25-27 must be 100 for LDM/STM instructions");
		if (code & (0x1 << 22))
			met_abort("not implemented");

		static const uint8_t NumBits[] =
			{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

		// Works like LDR and STR, look at above comments
		uint8_t numregs =
			NumBits[(code >> 12) & 0xF] +
			NumBits[(code >>  8) & 0xF] +
			NumBits[(code >>  4) & 0xF] +
			NumBits[ code        & 0xF];
		uint16_t regs = code & 0xFFFF;

		uint32_t add, baseadd;
		baseadd = add = R(Rn);
		if (code & (0x1 << 24))
			if (code & (0x1 << 23))
				add += 4; // increment before
			else
				add -= numregs * 4; // decrement before
		else
			if (!(code & (0x1 << 23)))
				add -= (numregs-1) * 4; // decrement after
		add &= 0xFFFFFFFC;

		if (code & (0x1 << 20)) // LDM
		{
			CYCLES32NSeq(add, numregs);
			ICYCLES(1);
			for (register uint8_t n = 0; n < 16; ++n)
				if (regs & (0x1 << n))
				{
					R(n) = MEM.Read32 (add);
					if (n == 15)
						R(15) += 4;
					add += 4;
				}
			if (regs & (0x1 << 15))
				CYCLES32NSeq(R(15), 3);
			else
				CYCLES32Seq(R(15), 1);
		}
		else // STM
		{
			CYCLES32NSeq(add, numregs);
			CYCLES32NSeq(R(15), 1);

			for (register uint8_t n = 0; n < 16; ++n)
				if (regs & (0x1 << n))
				{
					if (n == 15)
						MEM.Write32 (add, R(15) + 4);
					else
						MEM.Write32 (add, R(n));
					add += 4;
				}
		}

		if (code & (0x1 << 21))
			if (code & (0x1 << 23))
				R(Rn) = baseadd + numregs * 4;
			else
				R(Rn) = baseadd - numregs * 4;
	}

	// Single Data Swap (SWP)
	ARM(SWP)
	{
		if (((code >> 23) & 0x1F) != 0x02)
			met_abort("Bits 23-27 must be 00010 for SWP instructions");
		if ((code >> 20) & 0x3)
			met_abort("Bits 20-21 must be 00 for SWP instructions");
		if (((code >> 4) & 0xFF) != 0x09)
			met_abort("Bits 4-11 must be 00001001 for SWP instructions");

		if (code & (0x1 << 22)) // SWPB
		{
			R(Rd) = MEM.Read8(R(Rn));
			MEM.Write8(R(Rn), R(Rm));
		}
		else // SWP
		{
			R(Rd) = MEM.Read32(R(Rn));
			MEM.Write32(R(Rn), R(Rm));
		}
		CYCLES32NSeq(R(Rn), 1);
		CYCLES32NSeq(R(Rn), 1);
		ICYCLES(1);
		CYCLES32Seq(R(15), 1);
	}

	// Software Interrupt (SWI,BKPT)
	ARM(SWI)
	{
		if (((code >> 24) & 0xF) != 0xF)
			met_abort("Bits 24-27 must be 1111 for SWI instructions");

		CPU.SoftwareInterrupt((code >> 16) & 0xFF);

		// FIXME seems wrong !
		CYCLES32NSeq(0, 3);
	}

	inline bool Interpreter::a_CheckCondition (uint8_t cond)
	{
		if (cond == 0xE)
			return true;

		switch (cond)
		{
			case 0x0 : // EQ
				if (!FLAG_Z)
					return false;
				break;
			case 0x1 : // NE
				if (FLAG_Z)
					return false;
				break;
			case 0x2 : // CS
				if (!FLAG_C)
					return false;
				break;
			case 0x3 : // CC
				if (FLAG_C)
					return false;
				break;
			case 0x4 : // MI
				if (!FLAG_N)
					return false;
				break;
			case 0x5 : // PL
				if (FLAG_N)
					return false;
				break;
			case 0x6 : // VS
				if (!FLAG_V)
					return false;
				break;
			case 0x7 : // VC
				if (FLAG_V)
					return false;
				break;
			case 0x8 : // HI
				if (!FLAG_C || FLAG_Z)
					return false;
				break;
			case 0x9 : // LS
				if (FLAG_C && !FLAG_Z)
					return false;
				break;
			case 0xA : // GE
				if (FLAG_N != FLAG_V)
					return false;
				break;
			case 0xB : // LT
				if (FLAG_N == FLAG_V)
					return false;
				break;
			case 0xC : // GT
				if (FLAG_Z || FLAG_N != FLAG_V)
					return false;
				break;
			case 0xD : // LE
				if (!FLAG_Z && FLAG_N == FLAG_V)
					return false;
				break;
			case 0xE : // AL
				break;
			case 0xF : // reserved
				break;
		}

		return true;
	}

	NIARM(_Code)
	{
		if (!a_CheckCondition(code >> 28)) // condition failed
			CYCLES32Seq(R(15), 1);
		else
			switch ((code >> 25) & 0x7)
			{
				case 0x0:
					switch ((code >> 18) & 0x60 | (code >> 16) & 0x10 |
							(code >> 4) & 0x0F)
					{
						case 0x40:
							aPSR();
							break;
						case 0x00:
						case 0x02:
						case 0x04:
						case 0x06:
						case 0x08:
						case 0x0A:
						case 0x0C:
						case 0x0E:
						case 0x10:
						case 0x12:
						case 0x14:
						case 0x16:
						case 0x18:
						case 0x1A:
						case 0x1C:
						case 0x1E:
						case 0x20:
						case 0x22:
						case 0x24:
						case 0x26:
						case 0x28:
						case 0x2A:
						case 0x2C:
						case 0x2E:
						case 0x30:
						case 0x32:
						case 0x34:
						case 0x36:
						case 0x38:
						case 0x3A:
						case 0x3C:
						case 0x3E:
						case 0x50:
						case 0x52:
						case 0x54:
						case 0x56:
						case 0x58:
						case 0x5A:
						case 0x5C:
						case 0x5E:
						case 0x60:
						case 0x62:
						case 0x64:
						case 0x66:
						case 0x68:
						case 0x6A:
						case 0x6C:
						case 0x6E:
						case 0x70:
						case 0x72:
						case 0x74:
						case 0x76:
						case 0x78:
						case 0x7A:
						case 0x7C:
						case 0x7E:
							a_DataProcShiftImm();
							break;
						case 0x01:
						case 0x03:
						case 0x05:
						case 0x07:
						case 0x11:
						case 0x13:
						case 0x15:
						case 0x17:
						case 0x21:
						case 0x23:
						case 0x25:
						case 0x27:
						case 0x31:
						case 0x33:
						case 0x35:
						case 0x37:
						case 0x51:
						case 0x53:
						case 0x55:
						case 0x57:
						case 0x61:
						case 0x63:
						case 0x65:
						case 0x67:
						case 0x71:
						case 0x73:
						case 0x75:
						case 0x77:
							a_DataProcShiftReg();
							break;
						case 0x09:
						case 0x19:
						case 0x29:
						case 0x39:
						case 0x48:
						case 0x4A:
						case 0x4C:
						case 0x4E:
							a_Multiply();
							break;
						case 0x0B:
						case 0x0D:
						case 0x0F:
						case 0x1B:
						case 0x1D:
						case 0x1F:
						case 0x2B:
						case 0x2D:
						case 0x2F:
						case 0x3B:
						case 0x3D:
						case 0x3F:
						case 0x4B:
						case 0x4D:
						case 0x4F:
						case 0x5B:
						case 0x5D:
						case 0x5F:
						case 0x6B:
						case 0x6D:
						case 0x6F:
						case 0x7B:
						case 0x7D:
						case 0x7F:
							aSTRLDR_HD();
							break;
						case 0x49:
							aSWP();
							break;
						case 0x41:
						case 0x43:
							aBXBLX();
							break;
						default:
							met_abort("unknown");
							break;
					}
					break;
				case 0x1:
					// TODO PSR
					a_DataProcImm();
					break;
				case 0x2:
				case 0x3:
					aLDRSTR();
					break;
				case 0x4:
					aLDMSTM();
					break;
				case 0x5:
					aBBL();
					break;
				case 0x7:
					if (code & (0x1 << 24))
						aSWI();
					else
						met_abort("unknown");
					break;
				default:
					{	std::cerr << IOS_ADD << R(15)-8 << " : " << IOS_ADD << code << " : "; debug_bits(code); met_abort("not implemented"); }
					break;
			}
	}
}

#undef Rn
#undef Rd
#undef Rs
#undef Rm

#undef LSOff

#undef NOT_PC
#undef NOT_PC_ALL
#undef NOT_SAME2
#undef NOT_SAME3

#undef ARM

#endif
