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

#ifndef __INTERPRETER_THUMB_H__
#define __INTERPRETER_THUMB_H__

/*

- From GBATEK -

THUMB Binary Opcode Format

Fo|_15|_14|_13|_12|_11|_10|_9_|_8_|_7_|_6_|_5_|_4_|_3_|_2_|_1_|_0_|
_1|_0___0___0_|__Op___|_______Offset______|____Rs_____|____Rd_____|Shifted
_2|_0___0___0___1___1_|_I,_Op_|___Rn/nn___|____Rs_____|____Rd_____|ADD/SUB
_3|_0___0___1_|__Op___|____Rd_____|_____________Offset____________|Immedi.
_4|_0___1___0___0___0___0_|______Op_______|____Rs_____|____Rd_____|AluOp
_5|_0___1___0___0___0___1_|__Op___|Hd_|Hs_|____Rs_____|____Rd_____|HiReg/BX
_6|_0___1___0___0___1_|____Rd_____|_____________Word______________|LDR PC
_7|_0___1___0___1_|__Op___|_0_|___Ro______|____Rb_____|____Rd_____|LDR/STR
_8|_0___1___0___1_|__Op___|_1_|___Ro______|____Rb_____|____Rd_____|""H/SB/SH
_9|_0___1___1_|__Op___|_______Offset______|____Rb_____|____Rd_____|""{B}
10|_1___0___0___0_|Op_|_______Offset______|____Rb_____|____Rd_____|""H
11|_1___0___0___1_|Op_|____Rd_____|_____________Word______________|"" SP
12|_1___0___1___0_|Op_|____Rd_____|_____________Word______________|ADD PC/SP
13|_1___0___1___1___0___0___0___0_|_S_|___________Word____________|ADD SP,nn
14|_1___0___1___1_|Op_|_1___0_|_R_|____________Rlist______________|PUSH/POP
17|_1___0___1___1___1___1___1___0_|___________User_Data___________|BKPT ARM9
15|_1___1___0___0_|Op_|____Rb_____|____________Rlist______________|STM/LDM
16|_1___1___0___1_|_____Cond______|_________Signed_Offset_________|B{cond}
U_|_1___1___0___1___1___1___1___0_|_____________var_______________|UNDEF ARM9
17|_1___1___0___1___1___1___1___1_|___________User_Data___________|SWI
18|_1___1___1___0___0_|________________Offset_____________________|B
19|_1___1___1___0___1_|_________________________var___________|_0_|BLXsuf ARM9
U_|_1___1___1___0___1_|_________________________var___________|_1_|UNDEF ARM9
19|_1___1___1___1_|_H_|______________Offset_Low/High______________|BL (BLX ARM9)

*/

#include "ameteor/interpreter.hpp"
#include "globals.hpp"
#include "cpu_globals.hpp"
#include "ameteor/memory.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

#define Rb ((code >> 8) & 0x7)
#define Ro ((code >> 6) & 0x7)
#define Rs ((code >> 3) & 0x7)
#define Rd ((code     ) & 0x7)
#define Imm (code & 0xFF)
#define Off ((code >> 6) & 0x1F)

//#define HiRs (((code & (0x1 << 6)) >> 3) | Rs)
#define HiRs ((code >> 3) & 0xF)
#define HiRd (((code & (0x1 << 7)) >> 4) | Rd)

#ifdef METDEBUG
#	define NOT_PC(reg) \
		if (reg == 15) \
			met_abort("Register is PC")
#else
#	define NOT_PC(reg) {}
#endif

#define THUMB(name) \
	inline void Interpreter::t##name ()
#define NITHUMB(name) \
	void Interpreter::t##name ()

namespace AMeteor
{
	// move shifted register
	THUMB(_Shift)
	{
		if ((code >> 13) & 0x7)
		{
			met_abort("Bits 13-15 must be 000 for shift instructions");
		}

		uint8_t off = Off;
		switch ((code >> 11) & 0x3)
		{
			case 0x0: // LSL
				if (off)
				{
					SETF(C, R(Rs) & (0x1 << (32-off)));
					R(Rd) = R(Rs) << off;
					FZ(R(Rd));
					FN(R(Rd));
				}
				else
				{
					R(Rd) = R(Rs);
					FZ(R(Rd));
					FN(R(Rd));
				}
				break;
			case 0x1: // LSR
				if (off)
				{
					SETF(C, R(Rs) & (0x1 << (off-1)));
					R(Rd) = R(Rs) >> off;
					FZ(R(Rd));
					FN(R(Rd));
				}
				else
				{
					SETF(C, R(Rs) & (0x1 << 31));
					R(Rd) = 0;
					FLAG_Z = 1;
					FLAG_N = 0;
				}
				break;
			case 0x2: // ASR
				if (off)
				{
					SETF(C, (((int32_t)R(Rs)) >> (off - 1)) & 0x1);
					R(Rd) = ((int32_t)R(Rs)) >> off;
					FZ(R(Rd));
					FN(R(Rd));
				}
				else
				{
					if (R(Rd) & (0x1 << 31))
					{
						R(Rd) = 0xFFFFFFFF;
						FLAG_Z = 0;
						FLAG_N = 1;
						FLAG_C = 1;
					}
					else
					{
						R(Rd) = 0;
						FLAG_Z = 1;
						FLAG_N = 0;
						FLAG_C = 0;
					}
				}
				break;
			case 0x3: // reserved
				met_abort("Bits 11-12 must not be 11 for shift instructions");
				break;
		}

		CYCLES16Seq(R(15), 1);
	}

	// add/substract
	THUMB(ADDSUB)
	{
		if (((code >> 11) & 0x1F) != 0x3)
		{
			met_abort("Bits 11-15 should be 00011 for add/substract");
		}

		uint32_t op2;
		if ((code >> 10) & 0x1) // imm
			op2 = Ro;
		else // reg
			op2 = R(Ro);

		if ((code >> 9) & 0x1) // SUB
		{
#ifdef X86_ASM
			asm("subl %6, %5\n"
					"setzb %1\n"
					"setsb %2\n"
					"setncb %3\n"
					"setob %4\n"
					:"=r"(R(Rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
					:"0"(R(Rs)), "r"(op2));
#else
			uint32_t op1 = R(Rs), res = R(Rd) = op1 - op2;
			FLAG_Z = !res;
			FLAG_N = res >> 31;
			FLAG_C = SUBCARRY(op1, op2, res);
			FLAG_V = SUBOVERFLOW(op1, op2, res);
#endif
		}
		else // ADD
		{
#ifdef X86_ASM
			asm("addl %6, %5\n"
					"setzb %1\n"
					"setsb %2\n"
					"setcb %3\n"
					"setob %4\n"
					:"=r"(R(Rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
					:"0"(R(Rs)), "r"(op2));
#else
			uint32_t op1 = R(Rs), res = R(Rd) = op1 + op2;
			FLAG_Z = !res;
			FLAG_N = res >> 31;
			FLAG_C = ADDCARRY(op1, op2, res);
			FLAG_V = ADDOVERFLOW(op1, op2, res);
#endif
		}

		CYCLES16Seq(R(15), 1);
	}

	// move/compare/add/substact immediate
	THUMB(_Imm)
	{
		if (((code >> 13) & 0x7) != 0x1)
		{
			met_abort("Bits 13-15 must be 001 for immediate instructions");
		}

		switch ((code >> 11) & 0x3)
		{
			case 0x0: // MOV
				R(Rb) = Imm;
				FLAG_Z = !Imm;
				FLAG_N = 0;
				break;
			case 0x1: // CMP
#ifdef X86_ASM
				asm("cmpl %5, %4\n"
						"setzb %0\n"
						"setsb %1\n"
						"setncb %2\n"
						"setob %3\n"
						:"=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
						:"r"(R(Rb)), "r"(Imm));
#else
				{
					uint32_t op1 = R(Rb), op2 = Imm, res = op1 - op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
				}
#endif
				break;
			case 0x2: // ADD
#ifdef X86_ASM
				asm("addl %6, %5\n"
						"setzb %1\n"
						"setsb %2\n"
						"setcb %3\n"
						"setob %4\n"
						:"=r"(R(Rb)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
						:"0"(R(Rb)), "r"(Imm));
#else
				{
					uint32_t op1 = R(Rb), op2 = Imm, res = R(Rb) = op1 + op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = ADDCARRY(op1, op2, res);
					FLAG_V = ADDOVERFLOW(op1, op2, res);
				}
#endif
				break;
			case 0x3: // SUB
#ifdef X86_ASM
				asm("subl %6, %5\n"
						"setzb %1\n"
						"setsb %2\n"
						"setncb %3\n"
						"setob %4\n"
						:"=r"(R(Rb)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
						:"0"(R(Rb)), "r"(Imm));
#else
				{
					uint32_t op1 = R(Rb), op2 = Imm, res = R(Rb) = op1 - op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
				}
#endif
				break;
		}

		CYCLES16Seq(R(15), 1);
	}

	// ALU operations
	THUMB(_ALU)
	{
		if (((code >> 10) & 0x3F) != 0x10)
		{
			met_abort("Bits 10-15 must be 010000 for ALU instructions");
		}

		uint8_t opcode = (code >> 6) & 0xF;
		// TODO put a ifndef X86_ASM here around this declaration
		uint32_t res = 0; // to avoid a warning
		uint8_t shift;

		switch (opcode)
		{
			case 0x0: // AND
#ifdef X86_ASM
				asm("andl %4, %3\n"
						"setzb %1\n"
						"setsb %2\n"
						:"=r"(R(Rd)), "=m"(FLAG_Z), "=m"(FLAG_N)
						:"0"(R(Rd)), "r"(R(Rs)));
#else
				res = R(Rd) &= R(Rs);
				FLAG_Z = !res;
				FLAG_N = res >> 31;
#endif
				break;
			case 0x1: // EOR
#ifdef X86_ASM
				asm("xorl %4, %3\n"
						"setzb %1\n"
						"setsb %2\n"
						:"=r"(R(Rd)), "=m"(FLAG_Z), "=m"(FLAG_N)
						:"0"(R(Rd)), "r"(R(Rs)));
#else
				res = R(Rd) ^= R(Rs);
				FLAG_Z = !res;
				FLAG_N = res >> 31;
#endif
				break;
			case 0x2: // LSL
				ICYCLES(1);
				shift = R(Rs) & 0xFF;
				if (shift)
				{
					if (shift == 32)
					{
						SETFB(C, R(Rd) & 0x1);
						R(Rd) = 0;
						FLAG_Z = FLAG_N = 0;
					}
					else if (shift < 32)
					{
						SETFB(C, (R(Rd) >> (32-shift)) & 0x1);
						res = R(Rd) <<= shift;
						FLAG_Z = !res;
						FLAG_N = res >> 31;
					}
					else
					{
						R(Rd) = 0;
						FLAG_C = FLAG_Z = FLAG_N = 0;
					}
				}
				else
				{
					res = R(Rd);
					FLAG_Z = !res;
					FLAG_N = res >> 31;
				}
				break;
			case 0x3: // LSR
				ICYCLES(1);
				shift = R(Rs) & 0xFF;
				if (shift)
				{
					if (shift == 32)
					{
						SETFB(C, R(Rd) >> 31);
						R(Rd) = 0;
						FLAG_Z = FLAG_N = 0;
					}
					else if (shift < 32)
					{
						SETFB(C, (R(Rd) >> (shift-1)) & 0x1);
						res = R(Rd) >>= shift;
						FLAG_Z = !res;
						FLAG_N = res >> 31;
					}
					else
					{
						R(Rd) = 0;
						FLAG_C = FLAG_Z = FLAG_N = 0;
					}
				}
				else
				{
					res = R(Rd);
					FLAG_Z = !res;
					FLAG_N = res >> 31;
				}
				break;
			case 0x4: // ASR
				ICYCLES(1);
				shift = R(Rs) & 0xFF;
				if (shift)
				{
					if (shift >= 32)
					{
						R(Rd) = ((int32_t)R(Rs)) >> 31;
						FLAG_C = FLAG_Z = FLAG_N = R(Rd) & 0x1;
					}
					else
					{
						SETFB(C, (((int32_t)R(Rd)) >> (shift-1)) & 0x1);
						res = R(Rd) = ((int32_t)R(Rd)) >> shift;
						FLAG_Z = !res;
						FLAG_N = res >> 31;
					}
				}
				else
				{
					res = R(Rd);
					FLAG_Z = !res;
					FLAG_N = res >> 31;
				}
				break;
			case 0x5: // ADC
				res = R(Rd) + R(Rs) + FLAG_C;
				FLAG_C = ADDCARRY(R(Rd), R(Rs), res);
				FLAG_V = ADDOVERFLOW(R(Rd), R(Rs), res);
				R(Rd) = res;
				break;
			case 0x6: // SBC
				res = R(Rd) - R(Rs) + FLAG_C - 1;
				FLAG_C = SUBCARRY(R(Rd), R(Rs), res);
				FLAG_V = SUBOVERFLOW(R(Rd), R(Rs), res);
				R(Rd) = res;
				break;
			case 0x7: // ROR
				ICYCLES(1);
				shift = R(Rs) & 0xFF;
				if (shift)
				{
					shift %= 32;
					res = R(Rd);
					SETFB(C, (res >> (shift - 1)) & 0x1);
					res = R(Rd) = ROR(res, shift);
					FLAG_Z = !res;
					FLAG_N = res >> 31;
				}
				else
				{
					res = R(Rd);
					FLAG_Z = !res;
					FLAG_N = res >> 31;
				}
				break;
			case 0x8: // TST
#ifdef X86_ASM
				asm("testl %3, %2\n"
						"setzb %0\n"
						"setsb %1\n"
						:"=m"(FLAG_Z), "=m"(FLAG_N)
						:"r"(R(Rd)), "r"(R(Rs)));
#else
				res = R(Rd) & R(Rs);
				FLAG_Z = !res;
				FLAG_N = res >> 31;
#endif
				break;
			case 0x9: // NEG
#ifdef X86_ASM
				asm("negl %5\n"
						"setzb %1\n"
						"setsb %2\n"
						"setncb %3\n"
						"setob %4\n"
						:"=r"(R(Rd)), "=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
						:"0"(R(Rs)));
#else
				{
					uint32_t op2;
					op2 = R(Rs);
					res = R(Rd) = -op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					// we overflow only if op2 == INT_MIN
					FLAG_V = SUBOVERFLOW(0, op2, res);
					// we have a carry only if op2 == 0
					FLAG_C = SUBCARRY(0, op2, res);
				}
#endif
				break;
			case 0xA: // CMP
#ifdef X86_ASM
				asm("cmpl %5, %4\n"
						"setzb %0\n"
						"setsb %1\n"
						"setncb %2\n"
						"setob %3\n"
						:"=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
						:"r"(R(Rd)), "r"(R(Rs)));
#else
				{
					uint32_t op1, op2;
					op1 = R(Rd);
					op2 = R(Rs);
					res = op1 - op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
				}
#endif
				break;
			case 0xB: // CMN
#ifdef X86_ASM
				asm("addl %5, %4\n"
						"setzb %0\n"
						"setsb %1\n"
						"setcb %2\n"
						"setob %3\n"
						:"=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
						:"r"(R(Rd)), "r"(R(Rs))
						:"4");
#else
				{
					uint32_t op1, op2;
					op1 = R(Rd);
					op2 = R(Rs);
					res = op1 + op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = ADDCARRY(op1, op2, res);
					FLAG_V = ADDOVERFLOW(op1, op2, res);
				}
#endif
				break;
			case 0xC: // ORR
#ifdef X86_ASM
				asm("orl %4, %3\n"
						"setzb %1\n"
						"setsb %2\n"
						:"=r"(R(Rd)), "=m"(FLAG_Z), "=m"(FLAG_N)
						:"0"(R(Rd)), "r"(R(Rs)));
#else
				res = R(Rd) |= R(Rs);
				FLAG_Z = !res;
				FLAG_N = res >> 31;
#endif
				break;
			case 0xD: // MUL
				MULICYCLES(Rs);
				res = R(Rd) *= R(Rs);
				FLAG_Z = !res;
				FLAG_N = res >> 31;
				break;
			case 0xE: // BIC
				res = R(Rd) &= ~R(Rs);
				FLAG_Z = !res;
				FLAG_N = res >> 31;
				break;
			case 0xF: // MVN
				res = R(Rd) = ~R(Rs);
				FLAG_Z = !res;
				FLAG_N = res >> 31;
				break;
		}

		CYCLES16Seq(R(15), 1);
	}

	// Hi register operations/branch exchange
	THUMB(_HiRegOp)
	{
		if (((code >> 10) & 0x3F) != 0x11)
		{
			met_abort("Bits 10-15 must be 010001 for HiReg instructions");
		}

		uint8_t rd = HiRd, rs = HiRs;
		switch ((code >> 8) & 0x3)
		{
			case 0x0: // ADD
				R(rd) += R(rs);
				if (rd == 15)
				{
					R(15) &= 0xFFFFFFFE;
					CYCLES16NSeq(R(15), 3);
					R(15) += 2;
				}
				else
					CYCLES16Seq(R(15), 1);
				break;
			case 0x1: // CMP
#ifdef X86_ASM
				asm("cmpl %5, %4\n"
						"setzb %0\n"
						"setsb %1\n"
						"setncb %2\n"
						"setob %3\n"
						:"=m"(FLAG_Z), "=m"(FLAG_N), "=m"(FLAG_C), "=m"(FLAG_V)
						:"r"(R(rd)), "r"(R(rs)));
#else
				{
					uint32_t op1 = R(rd), op2 = R(rs), res = op1 - op2;
					FLAG_Z = !res;
					FLAG_N = res >> 31;
					FLAG_C = SUBCARRY(op1, op2, res);
					FLAG_V = SUBOVERFLOW(op1, op2, res);
				}
#endif
				CYCLES16Seq(R(15), 1);
				break;
			case 0x2:
				if (rd != 8 || rs != 8) // MOV
				{
					R(rd) = R(rs);
					if (rd == 15)
					{
						R(15) &= 0xFFFFFFFE;
						CYCLES16NSeq(R(15), 3);
						R(15) += 2;
					}
					else
						CYCLES16Seq(R(15), 1);
				}
				else // NOP
					CYCLES16Seq(R(15), 1);
				break;
			case 0x3:
				if (Rd)
					met_abort("Rd must be 0 for BX/BLX instructions");
				if (code & (0x1 << 7)) // BLX
				{
					NOT_PC(rs);
					met_abort("BLX not implemented");
				}
				else // BX
				{
					if (R(rs) & 0x1)
					{
						R(15) = (R(rs) & 0xFFFFFFFE) + 2;
						CYCLES16NSeq(R(15), 3);
					}
					else
					{
						// switch to arm
						FLAG_T = 0;
						R(15) = (R(rs) & 0xFFFFFFFC) + 4;
						CYCLES32NSeq(R(15), 3);
					}
				}
				break;
		}
	}

	// load PC-relative
	THUMB(LDRimm)
	{
		if (((code >> 11) & 0x1F) != 0x9)
		{
			met_abort("Bits 11-15 must be 01001 for LDR with imm instructions");
		}

		uint32_t add = (R(15) & 0xFFFFFFFC) + (Imm << 2);
		R(Rb) = MEM.Read32(add);

		CYCLES32NSeq(add, 1);
		ICYCLES(1);
		CYCLES16Seq(R(15), 1);
	}

	// load/store with register offset
	// and load/store sign-extended byte/halfword
	THUMB(STRLDRreg)
	{
		if (((code >> 12) & 0xF) != 0x5)
		{
			met_abort("Bits 12-15 must be 0101 for LDR/STR with reg instructions");
		}

		uint32_t add = R(Ro) + R(Rs);
		switch ((code >> 9) & 0x7)
		{
			// load/store with register offset
			case 0x0: // STR
				MEM.Write32(add, R(Rd));
				CYCLES32NSeq(add, 1);
				CYCLES16NSeq(R(15), 1);
				break;
			case 0x2: // STRB
				MEM.Write8(add, R(Rd));
				CYCLES16NSeq(add, 1);
				CYCLES16NSeq(R(15), 1);
				break;
			case 0x4: // LDR
				R(Rd) = MEM.Read32(add);
				CYCLES32NSeq(add, 1);
				ICYCLES(1);
				CYCLES16Seq(R(15), 1);
				break;
			case 0x6: // LDRB
				R(Rd) = MEM.Read8(add);
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES16Seq(R(15), 1);
				break;

			// load/store sign-extended byte/halfword
			case 0x1: // STRH
				MEM.Write16(add, R(Rd));
				CYCLES16NSeq(add, 1);
				CYCLES16NSeq(R(15), 1);
				break;
			case 0x3: // LDSB
				R(Rd) = (int8_t)MEM.Read8(add);
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES16Seq(R(15), 1);
				break;
			case 0x5: // LDRH
				R(Rd) = MEM.Read16(add);
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES16Seq(R(15), 1);
				break;
			case 0x7: // LDSH
				R(Rd) = (int16_t)MEM.Read16(add);
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES16Seq(R(15), 1);
				break;
		}
	}

	// load/store with immediate offset
	THUMB(STRLDRoff)
	{
		if (((code >> 13) & 0x7) != 0x3)
		{
			met_abort("Bits 13-15 must be 011 for LDR/STR with off instructions");
		}

		uint32_t add;
		switch ((code >> 11) & 0x3)
		{
			case 0x0: // STR
				add = R(Rs) + (Off << 2);
				MEM.Write32(add, R(Rd));
				CYCLES32NSeq(add, 1);
				CYCLES16NSeq(R(15), 1);
				break;
			case 0x1: // LDR
				add = R(Rs) + (Off << 2);
				R(Rd) = MEM.Read32(add);
				CYCLES32NSeq(add, 1);
				ICYCLES(1);
				CYCLES16Seq(R(15), 1);
				break;
			case 0x2: // STRB
				add = R(Rs) + Off;
				MEM.Write8(add, R(Rd));
				CYCLES16NSeq(add, 1);
				CYCLES16NSeq(R(15), 1);
				break;
			case 0x3: // LDRB
				add = R(Rs) + Off;
				R(Rd) = MEM.Read8(add);
				CYCLES16NSeq(add, 1);
				ICYCLES(1);
				CYCLES16Seq(R(15), 1);
				break;
		}
	}

	// load/store halfword
	THUMB(LDRHSTRHoff)
	{
		if (((code >> 12) & 0xF) != 0x8)
		{
			met_abort("Bits 12-15 must be 1000 for LDRH/STRH with off instructions");
		}

		uint32_t add = R(Rs) + (Off * 2);
		if (code & (0x1 << 11)) // LDRH
		{
			R(Rd) = MEM.Read16(add);
			CYCLES16NSeq(add, 1);
			ICYCLES(1);
			CYCLES16Seq(R(15), 1);
		}
		else // STRH
		{
			MEM.Write16(add, R(Rd));
			CYCLES16NSeq(add, 1);
			CYCLES16NSeq(R(15), 1);
		}
	}

	// load/store SP-relative
	THUMB(STRLDRsp)
	{
		if (((code >> 12) & 0xF) != 0x9)
		{
			met_abort("Bits 12-15 must be 1001 for LDR/STR SP-relative instructions");
		}

		uint32_t add = R(13) + (Imm << 2);
		if (code & (0x1 << 11)) // LDR
		{
			R(Rb) = MEM.Read32(add);
			CYCLES32NSeq(add, 1);
			ICYCLES(1);
			CYCLES16Seq(R(15), 1);
		}
		else // STR
		{
			MEM.Write32(add, R(Rb));
			CYCLES32NSeq(add, 1);
			CYCLES16NSeq(R(15), 1);
		}
	}

	// get relative address
	THUMB(ADDpcsp)
	{
		if (((code >> 12) & 0xF) != 0xA)
		{
			met_abort("Bits 12-15 must be 1010 for ADD relative instructions");
		}

		if (code & (0x1 << 11)) // with SP
		{
			R(Rb) = R(13) + (Imm << 2);
		}
		else // with PC
		{
			R(Rb) = (R(15) & 0xFFFFFFFC) + (Imm << 2);
		}

		CYCLES16Seq(R(15), 1);
	}

	// add offset to stack pointer
	THUMB(ADDsp)
	{
		if (((code >> 8) & 0xFF) != 0xB0)
		{
			met_abort("Bits 8-15 must be 10110000 for ADD to SP instructions");
		}

		if (code & (0x1 << 7)) // substract
		{
			R(13) -= ((code & 0x7F) << 2);
		}
		else // add
		{
			R(13) += ((code & 0x7F) << 2);
		}

		CYCLES16Seq(R(15), 1);
	}

	// push/pop registers
	THUMB(PUSHPOP)
	{
		if (((code >> 12) & 0xF) != 0xB)
		{
			met_abort("Bits 12-15 must be 1011 for PUSH/POP instructions");
		}
		if (((code >> 9) & 0x3) != 0x2)
		{
			met_abort("Bits 9-10 must be 10 for PUSH/POP instructions");
		}

		static const uint8_t NumBits[] =
			{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

		uint8_t numregs =
			NumBits[(code >> 4) & 0xF] +
			NumBits[ code       & 0xF];
		if (code & (0x1 << 8))
			++numregs;

		uint32_t add, newsp;
		if (code & (0x1 << 11)) // POP
		{
			newsp = R(13) + 4 * numregs;
			add = R(13) & 0xFFFFFFFC;

			CYCLES32NSeq(add, numregs);
			ICYCLES(1);

			for (register uint8_t n = 0; n < 8; ++n)
				if (code & (0x1 << n))
				{
					R(n) = MEM.Read32 (add);
					add += 4;
				}
			if (code & (0x1 << 8)) // POP PC
			{
				R(15) = (MEM.Read32(add) + 2) & 0xFFFFFFFE;
				CYCLES16NSeq(R(15), 3);
			}
			else
				CYCLES16Seq(R(15), 1);
		}
		else // PUSH
		{
			newsp = R(13) - 4 * numregs;
			add = newsp & 0xFFFFFFFC;

			CYCLES32NSeq(add, numregs);
			CYCLES16NSeq(R(15), 1);

			for (register uint8_t n = 0; n < 8; ++n)
				if (code & (0x1 << n))
				{
					MEM.Write32 (add, R(n));
					add += 4;
				}
			if (code & (0x1 << 8)) // PUSH LR
			{
				MEM.Write32 (add, R(14));
			}
		}

		R(13) = newsp;
	}

	// multiple load/store
	THUMB(STMLDM)
	{
		if (((code >> 12) & 0xF) != 0xC)
		{
			met_abort("Bits 12-15 must be 1100 for STM/LDM instructions");
		}

		static const uint8_t NumBits[] =
			{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

		uint8_t numregs =
			NumBits[(code >> 4) & 0xF] +
			NumBits[ code       & 0xF];

		uint32_t add = R(Rb) & 0xFFFFFFFC;

		if (code & (0x1 << 11)) // LDMIA
		{
			CYCLES32NSeq(add, numregs);
			ICYCLES(1);

			for (register uint8_t n = 0; n < 8; ++n)
				if (code & (0x1 << n))
				{
					R(n) = MEM.Read32 (add);
					add += 4;
				}

			CYCLES16Seq(R(15), 1);
		}
		else // STMIA
		{
			CYCLES32NSeq(add, numregs);
			CYCLES16NSeq(R(15), 1);

			for (register uint8_t n = 0; n < 8; ++n)
				if (code & (0x1 << n))
				{
					MEM.Write32 (add, R(n));
					add += 4;
				}
		}

		if (!(code & (0x1 << Rb)))
			R(Rb) = R(Rb) + 4 * numregs;
	}

	// conditional branch
	THUMB(_CondBranch)
	{
		if (((code >> 12) & 0xF) != 0xD)
		{
			met_abort("Bits 12-15 must be 1101 for branch on condition instructions");
		}

		bool branch = false;
		switch ((code >> 8) & 0xF)
		{
			case 0x0: // BEQ
				if (FLAG_Z)
					branch = true;
				break;
			case 0x1: // BNE
				if (!FLAG_Z)
					branch = true;
				break;
			case 0x2: // BCS
				if (FLAG_C)
					branch = true;
				break;
			case 0x3: // BCC
				if (!FLAG_C)
					branch = true;
				break;
			case 0x4: // BMI
				if (FLAG_N)
					branch = true;
				break;
			case 0x5: // BPL
				if (!FLAG_N)
					branch = true;
				break;
			case 0x6: // BVS
				if (FLAG_V)
					branch = true;
				break;
			case 0x7: // BVC
				if (!FLAG_V)
					branch = true;
				break;
			case 0x8: // BHI
				if (FLAG_C && !FLAG_Z)
					branch = true;
				break;
			case 0x9: // BLS
				if (!FLAG_C || FLAG_Z)
					branch = true;
				break;
			case 0xA: // BGE
				if (FLAG_N == FLAG_V)
					branch = true;
				break;
			case 0xB: // BLT
				if (FLAG_N != FLAG_V)
					branch = true;
				break;
			case 0xC: // BGT
				if (!FLAG_Z && FLAG_N == FLAG_V)
					branch = true;
				break;
			case 0xD: // BLE
				if (FLAG_Z || FLAG_N != FLAG_V)
					branch = true;
				break;
			case 0xE: // undefined
				met_abort("Undefined branch on condition");
				break;
			case 0xF: // reserved for SWI instruction
				met_abort("Bits 8-11 must not be 1111 for branch instruction");
				break;
		}

		if (branch)
		{
			R(15) += (((int32_t)(int8_t)Imm) << 1) + 2;

			CYCLES16NSeq(R(15), 3);
		}
		else
			CYCLES16Seq(R(15), 1);
	}

	// software interrupt and breakpoint
	THUMB(SWI)
	{
		CPU.SoftwareInterrupt(code & 0xFF);

		// FIXME seems wrong !
		CYCLES32NSeq(0, 3);
	}

	// unconditional branch
	THUMB(B)
	{
		if (((code >> 11) & 0x1F) != 0x1C)
		{
			met_abort("Bits 11-15 must be 11100 for branch instructions");
		}

		int32_t off = (code & 0x7FF) << 1;
		if (off & 0x800)
			off |= 0xFFFFF000; // extends sign bit
		R(15) += off + 2;

		CYCLES16NSeq(R(15), 3);
	}

	// long branch with link
	THUMB(_BL1)
	{
		if (((code >> 11) & 0x1F) != 0x1E)
		{
			met_abort("Bits 11-15 must be 11110 for long branch instructions");
		}

		if (code & (0x1 << 10)) // negative offset
			R(14) = R(15) + (((int32_t)((code & 0x7FF) << 12)) | 0xFF800000);
		else
			R(14) = R(15) + ((code & 0x7FF) << 12);

		CYCLES16Seq(R(15), 1);
	}

	THUMB(_BL2)
	{
		// XXX we dont do check here to be sure this instruction is called after
		// BL1
		uint32_t add = R(14) + ((code & 0x7FF) << 1);

		if (code & (0x1 << 11)) // BL
		{
			R(14) = (R(15)-2) | 0x1;
			R(15) = (add & 0xFFFFFFFE) + 2;
		}
		else // BLX
		{
			met_abort("BLX not implemented");
			if (code & 0x1)
				met_abort("BLX with odd address");
			FLAG_T = 0;
		}

		CYCLES16NSeq(R(15), 3);
	}

	NITHUMB(_Code)
	{
		switch (code >> 13)
		{
			case 0: // 000
				if ((code & 0x1800) == 0x1800) // 00011
					tADDSUB();
				else // 000
					t_Shift();
				break;
			case 1: // 001
				t_Imm();
				break;
			case 2: // 010
				switch ((code >> 10) & 0x7)
				{
					case 0: // 010000
						t_ALU();
						break;
					case 1: // 010001
						t_HiRegOp();
						break;
					case 2:
					case 3: // 01001x
						tLDRimm();
						break;
					default:
						tSTRLDRreg();
						break;
				}
				break;
			case 3: // 011
				tSTRLDRoff();
				break;
			case 4: // 100
				if (code & (0x1 << 12)) // 1001
					tSTRLDRsp();
				else // 100
					tLDRHSTRHoff();
				break;
			case 5: // 101
				if (code & (0x1 << 12)) // 1011
				{
					switch (code & 0x0600)
					{
						case 0x0000: // 1011x00
							tADDsp();
							break;
						case 0x0400: // 1011x10
							tPUSHPOP();
							break;
						default:
							met_abort("not implemented or unknown");
							break;
					}
				}
				else // 101
					tADDpcsp();
				break;
			case 6: // 110
				if (code & (0x1 << 12)) // 1101
				{
					if ((code & 0x0F00) == 0x0F00) // 11011111
						tSWI();
					else // 1101xxxx
						t_CondBranch();
				}
				else // 1100
					tSTMLDM();
				break;
			case 7: // 111
				switch ((code >> 11) & 0x3)
				{
					case 0: // 11100
						tB();
						break;
					case 2: // 11110
						t_BL1();
						break;
					case 3: // 11111
						t_BL2();
						break;
					default:
						met_abort("not implemented or unknown");
						break;
				}
				break;
		}
	}
}

#undef Rb
#undef Ro
#undef Rs
#undef Rd
#undef Imm

#undef THUMB

#endif
