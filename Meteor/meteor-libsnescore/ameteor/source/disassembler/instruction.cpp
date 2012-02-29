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

#include "ameteor/disassembler/instruction.hpp"

#include "ameteor/disassembler/argregister.hpp"
#include "ameteor/disassembler/argrelative.hpp"
#include "ameteor/disassembler/argimmediate.hpp"
#include "ameteor/disassembler/arguimmediate.hpp"
#include "ameteor/disassembler/argshift.hpp"
#include "ameteor/disassembler/argpsr.hpp"
#include "ameteor/disassembler/argmulregisters.hpp"

#include "../globals.hpp" // for ROR

namespace AMeteor
{
	namespace Disassembler
	{
		void Instruction::Clear ()
		{
			m_operator.clear();
			m_args.Clear();
		}

#define Rn ((code >> 16) & 0xF)
#define Rd ((code >> 12) & 0xF)
#define Rs ((code >>  8) & 0xF)
#define Rm  (code        & 0xF)

		void Instruction::ParseArm (uint32_t offset, uint32_t code)
		{
			Clear();

			if ((code & 0x0FFFFFD0) == 0x12FFF10)
			{
				if (code & (0x1 << 5))
					m_operator = "BLX";
				else
					m_operator = "BX";
				m_args.AddArgument(ArgRegister(Rm));
				ParseArmCondition(code);
			}
			else if (((code >> 25) & 0x7) == 0x5)
			{
				if (((code >> 28) & 0xF) == 0xF)
				{
					m_operator = "BLX";
					m_args.AddArgument(ArgImmediate(offset + 8 +
								(((code & (0x1 << 23)) ? 0xFF000000 | (code & 0x00FFFFFF)
									: (code & 0x00FFFFFF)) << 2)
								+ ((code & (0x1 << 24)) ? 2 : 0)));
				}
				else
				{
					if (code & (0x1 << 24))
						m_operator = "BL";
					else
						m_operator = "B";
					m_args.AddArgument(ArgImmediate(offset + 8 +
								(((code & (0x1 << 23)) ? 0xFF000000 | (code & 0x00FFFFFF)
									: (code & 0x00FFFFFF)) << 2)));
					ParseArmCondition(code);
				}
			}
			else if (((code >> 25) & 0x7) == 0x1)
			{
				ParseArmDataProc(code);
			}
			else if (((code >> 26) & 0x3) == 0x1)
			{
				if ((code & 0xF0100000) != 0xF0100000) // not PLD
					m_args.AddArgument(ArgRegister(Rd));
				if (code & (0x1 << 25)) // register offset
				{
					switch ((code >> 5) & 0x3)
					{
						case 0: // Logical Shift Left
							m_args.AddArgument(ArgRelative(Rn, ArgShift(ArgRegister(Rm),
											ArgImmediate((code >> 7) & 0x1F), SHIFT_LSL, false),
										code & (0x1 << 24), code & (0x1 << 23),
										code & (0x1 << 21)));
							break;
						case 1: // Logical Shift Right
							if (code & (0x1F << 7))
								m_args.AddArgument(ArgRelative(Rn, ArgShift(ArgRegister(Rm),
												ArgImmediate((code >> 7) & 0x1F), SHIFT_LSR, false),
											code & (0x1 << 24), code & (0x1 << 23),
											code & (0x1 << 21)));
							else
								m_args.AddArgument(ArgRelative(Rn, ArgShift(ArgRegister(Rm),
												ArgImmediate(32), SHIFT_LSR, false),
											code & (0x1 << 24), code & (0x1 << 23),
											code & (0x1 << 21)));
							break;
						case 2: // Arithmetic Shift Right
							if (code & (0x1F << 7))
								m_args.AddArgument(ArgRelative(Rn, ArgShift(ArgRegister(Rm),
												ArgImmediate((code >> 7) & 0x1F), SHIFT_ASR, false),
											code & (0x1 << 24), code & (0x1 << 23),
											code & (0x1 << 21)));
							else
								m_args.AddArgument(ArgRelative(Rn, ArgShift(ArgRegister(Rm),
												ArgImmediate(32), SHIFT_ASR, false),
											code & (0x1 << 24), code & (0x1 << 23),
											code & (0x1 << 21)));
							break;
						case 3: // ROtate Right
							if (code & (0x1F << 7))
								m_args.AddArgument(ArgRelative(Rn, ArgShift(ArgRegister(Rm),
												ArgImmediate((code >> 7) & 0x1F), SHIFT_ROR, false),
											code & (0x1 << 24), code & (0x1 << 23),
											code & (0x1 << 21)));
							else
								m_args.AddArgument(ArgRelative(Rn, ArgShift(ArgRegister(Rm),
												ArgImmediate(1), SHIFT_RRX, false),
											code & (0x1 << 24), code & (0x1 << 23),
											code & (0x1 << 21)));
							break;
					}
				}
				else // immediate offset
				{
					m_args.AddArgument(ArgRelative(Rn, ArgImmediate(code & 0xFFF),
								code & (0x1 << 24), code & (0x1 << 23),
								code & (0x1 << 21)));
				}

				if ((code & 0xF0100000) == 0xF0100000)
					m_operator = "PLD";
				else
				{
					if (code & (0x1 << 20))
						m_operator = "LDR";
					else
						m_operator = "STR";
					if (code & (0x1 << 22))
						m_operator += "B";

					ParseArmCondition(code);
				}
			}
			else if (((code >> 25) & 0x7) == 0x4)
			{
				if (code & (0x1 << 20))
					m_operator = "LDM";
				else
					m_operator = "STM";

				if (code & (0x1 << 23))
					m_operator += 'I';
				else
					m_operator += 'D';

				if (code & (0x1 << 24))
					m_operator += 'B';
				else
					m_operator += 'A';

				m_args.AddArgument(ArgRegister(Rn, code & (0x1 << 21)));

				ArgMulRegisters argRegs(code & (0x1 << 22));
				for (register uint8_t n = 0; n < 16; ++n)
					if (code & (0x1 << n))
						argRegs.AddRegister(n);
				m_args.AddArgument(argRegs);

				ParseArmCondition(code);
			}
			else if (((code >> 25) & 0x7) == 0x0)
			{
				if ((code & 0x0FC000F0) == 0x00000090 ||
						(code & 0x0F8000F0) == 0x00800090 ||
						(code & 0x0F900090) == 0x01000080)
				{
					// NOTE : In this instruction Rn and Rd are inverted
					static const char* Instructions[] = {"MUL", "MLA", "Reserved",
						"Reserved", "UMULL", "UMLAL", "SMULL", "SMLAL", "SMLAxy",
						"", // This is for SMLAWy and SMULWy
						"SMLALxy", "SMULxy", "Reserved", "Reserved", "Reserved",
						"Reserved"};

					uint8_t opcode = (code >> 21) & 0xF;
					if (opcode == 0x9)
						m_operator = (code & (0x1 << 5)) ? "SMULWy" : "SMLAWy";
					else
						m_operator = Instructions[opcode];
					if (!(opcode & (0x1 << 4)) && (code & (0x1 << 20)))
						m_operator += 'S';

					ParseArmCondition(code);

					if ((opcode & 0xC) == 0x4 || opcode == 0xA)
						m_args.AddArgument(ArgRegister(Rd));
					m_args.AddArgument(ArgRegister(Rn));
					m_args.AddArgument(ArgRegister(Rm));
					m_args.AddArgument(ArgRegister(Rs));
					if ((opcode & 0xE) == 0x8 || opcode == 0x1)
						m_args.AddArgument(ArgRegister(Rd));
				}
				else if ((code & (0x1 << 7)) && (code & (0x1 << 4)))
				{
					if (((code >> 23) & 0x3) == 0x2 && ((code >> 20) & 0x3) == 0x0
							&& ((code >> 4) & 0xFF) == 0x09)
					{
						if (code & (0x1 << 22)) // SWPB
							m_operator = "SWPB";
						else // SWP
							m_operator = "SWP";

						ParseArmCondition(code);

						m_args.AddArgument(ArgRegister(Rd));
						m_args.AddArgument(ArgRegister(Rm));
						m_args.AddArgument(ArgRegister(Rn, false, false, true));
					}
					else
					{
						static const char* Instructions[] = {"Reserved", "STRH", "LDRD",
							"STRD", "Reserved", "LDRH", "LDRSB", "LDRSH"};

						m_operator = Instructions[((code >> 18) & 0x4)
							| ((code >> 5) & 0x3)];

						ParseArmCondition(code);

						m_args.AddArgument(ArgRegister(Rd));

						if (code & (0x1 << 22)) // immediate
						{
							m_args.AddArgument(ArgRelative(ArgRegister(Rn),
										ArgImmediate(((code >> 4) & 0xF0) | (code & 0xF)),
										code & (0x1 << 24), code & (0x1 << 23),
										code & (0x1 << 21)));
						}
						else
						{
							m_args.AddArgument(ArgRelative(ArgRegister(Rn),
										ArgRegister(code & 0xF),
										code & (0x1 << 24), code & (0x1 << 23),
										code & (0x1 << 21)));
						}
					}
				}
				else if (((code >> 23) & 0x3) == 0x2)
				{
					if (!((code >> 20) & 0x1))
					{
						if (code & (0x1 << 21))
						{
							m_operator = "MSR";

							m_args.AddArgument(ArgPsr(code & (0x1 << 22),
										(code >> 16) & 0xF));

							if (code & (0x1 << 25)) // immediate
							{
								m_args.AddArgument(ArgUImmediate(
											ROR(code & 0xFF, (code >> 8) & 0xF)));
							}
							else
							{
								m_args.AddArgument(ArgRegister(Rm));
							}
						}
						else
						{
							m_operator = "MRS";

							m_args.AddArgument(ArgRegister(Rd));
							m_args.AddArgument(ArgPsr(code & (0x1 << 22)));
						}

						ParseArmCondition(code);
					}
					else
					{
						ParseArmDataProc(code);
					}
				}
				else
				{
					ParseArmDataProc(code);
				}
			}
			else
			{
				m_operator = "Unknown";
			}
		}

		void Instruction::ParseArmDataProc (uint32_t code)
		{
			static const char* ops[] = {"AND", "EOR", "SUB", "RSB", "ADD", "ADC",
				"SBC", "RSC", "TST", "TEQ", "CMP", "CMN", "ORR", "MOV", "BIC", "MVN"};

			uint8_t opcode = (code >> 21) & 0xF;

			if (opcode < 0x8 || opcode > 0xB)
				m_args.AddArgument(ArgRegister(Rd));
			if (opcode != 0xD && opcode != 0xF)
				m_args.AddArgument(ArgRegister(Rn));

			if (code & (0x1 << 25)) // Immediate operand 2
			{
				/*if (code & (0xF << 8))
					m_args.AddArgument(ArgShift(ArgImmediate(code & 0xFF),
								ArgImmediate(((code >> 8) & 0xF) << 1), SHIFT_ROR, false));
				else
					m_args.AddArgument(ArgImmediate(code & 0xFF));*/
				m_args.AddArgument(ArgUImmediate(
							ROR(code & 0xFF, (code >> 7) & 0x1E)));
			}
			else
			{
				switch ((code >> 5) & 0x3)
				{
					case 0: // Logical Shift Left
						if (code & (0x1 << 4)) // Shift by register
						{
							m_args.AddArgument(ArgShift(ArgRegister(Rm),
										ArgRegister(Rs), SHIFT_LSL, false));
						}
						else // Shift by immediate
						{
							if (code & (0x1F << 7))
								m_args.AddArgument(ArgShift(ArgRegister(Rm),
											ArgImmediate((code >> 7) & 0x1F), SHIFT_LSL, false));
							else
								m_args.AddArgument(ArgRegister(Rm));
						}
						break;
					case 1: // Logical Shift Right
						if (code & (0x1 << 4)) // Shift by register
						{
							m_args.AddArgument(ArgShift(ArgRegister(Rm),
										ArgRegister(Rs), SHIFT_LSR, false));
						}
						else // Shift by immediate
						{
							if (code & (0x1F << 7))
								m_args.AddArgument(ArgShift(ArgRegister(Rm),
											ArgImmediate((code >> 7) & 0x1F), SHIFT_LSR, false));
							else
								m_args.AddArgument(ArgShift(ArgRegister(Rm),
											ArgImmediate(32), SHIFT_LSR, false));
						}
						break;
					case 2: // Arithmetic Shift Right
						if (code & (0x1 << 4)) // Shift by register
						{
							m_args.AddArgument(ArgShift(ArgRegister(Rm),
										ArgRegister(Rs), SHIFT_ASR, false));
						}
						else // Shift by immediate
						{
							if (code & (0x1F << 7))
								m_args.AddArgument(ArgShift(ArgRegister(Rm),
											ArgImmediate((code >> 7) & 0x1F), SHIFT_ASR, false));
							else
								m_args.AddArgument(ArgShift(ArgRegister(Rm),
											ArgImmediate(32), SHIFT_ASR, false));
						}
						break;
					case 3: // ROtate Right
						if (code & (0x1 << 4)) // Shift by register
						{
							m_args.AddArgument(ArgShift(ArgRegister(Rm),
										ArgRegister(Rs), SHIFT_ROR, false));
						}
						else // Shift by immediate
						{
							if (code & (0x1F << 7))
								m_args.AddArgument(ArgShift(ArgRegister(Rm),
											ArgImmediate((code >> 7) & 0x1F), SHIFT_ROR, false));
							else
								m_args.AddArgument(ArgShift(ArgRegister(Rm),
											ArgImmediate(1), SHIFT_RRX, false));
						}
						break;
				}
			}

			m_operator = ops[opcode];

			if (code & (0x1 << 20) && (opcode < 0x8 || opcode > 0xB))
			{
				m_operator += "S";
			}
			ParseArmCondition(code);
		}

		void Instruction::ParseArmCondition (uint32_t code)
		{
			static const char* Conditions[] = {"EQ", "NE", "CS", "CC", "MI", "PL",
				"VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "", "NV"};

			m_operator += Conditions[code >> 28];
		}

#undef Rn
#undef Rd
#undef Rs
#undef Rm

#define Rb ((code >> 8) & 0x7)
#define Ro ((code >> 6) & 0x7)
#define Rs ((code >> 3) & 0x7)
#define Rd ((code     ) & 0x7)
#define Imm (code & 0xFF)
#define Off ((code >> 6) & 0x1F)

#define HiRs ((code >> 3) & 0xF)
#define HiRd (((code & (0x1 << 7)) >> 4) | Rd)

		void Instruction::ParseThumb (uint32_t offset, uint16_t code)
		{
			Clear ();

			if ((code >> 12) == 0xB && ((code >> 9) & 0x3) == 0x2) // 1011x10
			{
				if (code & (0x1 << 11))
					m_operator = "POP";
				else
					m_operator = "PUSH";

				ArgMulRegisters argRegs(false);
				for (register uint8_t n = 0; n < 8; ++n)
					if (Imm & (0x1 << n))
						argRegs.AddRegister(n);

				if (code & (0x1 << 8))
				{
					if (code & (0x1 << 11))
						argRegs.AddLastRegister(SPREG_PC);
					else
						argRegs.AddLastRegister(SPREG_LR);
				}
				m_args.AddArgument(argRegs);
			}
			else if ((code >> 11) == 0x9) // 01001
			{
				m_operator = "LDR";

				m_args.AddArgument(ArgRegister(Rb));
				m_args.AddArgument(ArgRelative(15, ArgImmediate(Imm << 2),
							true, true, false));
			}
			else if ((code >> 12) == 0x8) // 1000
			{
				if (code & (0x1 << 11))
					m_operator = "LDRH";
				else
					m_operator = "STRH";

				m_args.AddArgument(ArgRegister(Rd));
				m_args.AddArgument(ArgRelative(Rs, ArgImmediate(Off << 1),
							true, true, false));
			}
			else if ((code >> 10) == 0x10) // 010000
			{
				static const char* Instructions[] = {"AND", "EOR", "LSL", "LSR", "ASR",
					"ADC", "SBC", "ROR", "TST", "NEG", "CMP", "CMN", "ORR", "MUL", "BIC",
					"MVN"};

				m_operator = Instructions[(code >> 6) & 0xF];

				m_args.AddArgument(ArgRegister(Rd));
				m_args.AddArgument(ArgRegister(Rs));
			}
			else if ((code >> 10) == 0x11) // 010001
			{
				switch ((code >> 8) & 0x3)
				{
					case 0x0: // ADD
						m_operator = "ADD";
						m_args.AddArgument(ArgRegister(HiRd));
						m_args.AddArgument(ArgRegister(HiRs));
						break;
					case 0x1: // CMP
						m_operator = "CMP";
						m_args.AddArgument(ArgRegister(HiRd));
						m_args.AddArgument(ArgRegister(HiRs));
						break;
					case 0x2:
						if (HiRd != 8 || HiRs != 8) // MOV
						{
							m_operator = "MOV";
							m_args.AddArgument(ArgRegister(HiRd));
							m_args.AddArgument(ArgRegister(HiRs));
						}
						else
							m_operator = "NOP";
						break;
					case 0x3:
						if (code & (0x1 << 7)) // BLX
						{
							m_operator = "BLX";
							m_args.AddArgument(ArgRegister(HiRs));
						}
						else // BX
						{
							m_operator = "BX";
							m_args.AddArgument(ArgRegister(HiRs));
						}
						break;
				}
			}
			else if ((code >> 13) == 0x1) // 001
			{
				static const char* Instructions[] = {"MOV", "CMP", "ADD", "SUB"};

				m_operator = Instructions[(code >> 11) & 0x3];

				m_args.AddArgument(ArgRegister(Rb));
				m_args.AddArgument(ArgImmediate(Imm));
			}
			else if ((code >> 13) == 0x3) // 011
			{
				static const char* Instructions[] = {"STR", "LDR", "STRB", "LDRB"};

				m_operator = Instructions[(code >> 11) & 0x3];

				m_args.AddArgument(ArgRegister(Rd));
				if (code & (0x1 << 12))
					m_args.AddArgument(ArgRelative(Rs, ArgImmediate(Off), true,
								true, false));
				else
					m_args.AddArgument(ArgRelative(Rs, ArgImmediate(Off << 2), true,
								true, false));
			}
			else if ((code >> 12) == 0xC) // 1100
			{
				if (code & (0x1 << 11))
					m_operator = "LDMIA";
				else
					m_operator = "STMIA";

				m_args.AddArgument(ArgRegister(Rb, true));

				ArgMulRegisters argRegs(false);
				for (register uint8_t n = 0; n < 8; ++n)
					if (Imm & (0x1 << n))
						argRegs.AddRegister(n);
				m_args.AddArgument(argRegs);
			}
			else if ((code >> 13) == 0x0) // 000
			{
				if ((code >> 11) == 0x3) // 00011
				{
					if ((code >> 9) & 0x1)
						m_operator = "SUB";
					else
						m_operator = "ADD";

					m_args.AddArgument(ArgRegister(Rd));
					m_args.AddArgument(ArgRegister(Rs));

					if ((code >> 10) & 0x1) // imm
						m_args.AddArgument(ArgImmediate(Ro));
					else // reg
						m_args.AddArgument(ArgRegister(Ro));
				}
				else // 000
				{
					static const char* Instructions[] = {"LSL", "LSR", "ASR", "Reserved"};

					m_operator = Instructions[(code >> 11) & 0x3];

					m_args.AddArgument(ArgRegister(Rd));
					m_args.AddArgument(ArgRegister(Rs));
					m_args.AddArgument(ArgImmediate(Off));
				}
			}
			else if ((code >> 11) == 0x1E) // 11110
			{
				m_operator = "BL.W1";

				m_args.AddArgument(ArgImmediate(offset + 4 + ((code & 0x7FF) << 12)));
			}
			else if ((code >> 13) == 0x7 && (code & (0x1 << 11))) // 111x1
			{
				m_operator = "BL.W2";

				m_args.AddArgument(ArgImmediate((code & 0x7FF) << 1));
			}
			else if ((code >> 11) == 0x1C) // 11100
			{
				m_operator = "B";

				if (code & (0x1 << 10))
					m_args.AddArgument(ArgUImmediate(offset + 4 +
								((int32_t)(((code & 0x3FF) << 1) | 0xFFFFF800))));
				else
					m_args.AddArgument(ArgUImmediate(offset + 4 + ((code & 0x3FF) << 1)));
			}
			else if ((code >> 12) == 0xD) // 1101
			{
				if (((code >> 8) & 0xF) == 0xF) // 11011111
				{
					m_operator = "SWI";

					m_args.AddArgument(ArgImmediate(code & 0xFF));
				}
				else // 1101
				{
					static const char* Conditions[] = {"EQ", "NE", "CS", "CC", "MI", "PL",
						"VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "__", "**"};

					m_operator = "B";
					m_operator += Conditions[(code >> 8) & 0xF];

					m_args.AddArgument(ArgUImmediate(offset + 4 +
								(((int32_t)(int8_t)Imm) << 1)));
				}
			}
			else if ((code >> 8) == 0xB0) // 10110000
			{
				m_operator = "ADD";

				m_args.AddArgument(ArgRegister(13, false, true));
				if (code & (0x1 << 7)) // substract
					m_args.AddArgument(ArgImmediate(-((code & 0x7F) << 2)));
				else // add
					m_args.AddArgument(ArgImmediate((code & 0x7F) << 2));
			}
			else if ((code >> 12) == 0x5) // 0101
			{
				if (code & (0x1 << 11))
					m_operator = "LDR";
				else
					m_operator = "STR";

				if (code & (0x1 << 10))
					m_operator += 'B';

				m_args.AddArgument(ArgRegister(Rd));
				m_args.AddArgument(ArgRelative(Rs, ArgRegister(Ro), true, true, false));
			}
			else if ((code >> 12) == 0x9) // 1001
			{
				if (code & (0x1 << 11))
					m_operator = "LDR";
				else
					m_operator = "STR";

				m_args.AddArgument(ArgRegister(Rb));
				m_args.AddArgument(ArgRelative(ArgRegister(13, false, true),
							ArgImmediate(Imm << 2), true, true, false));
			}
			else if ((code >> 12) == 0xA) // 1010
			{
				m_operator = "ADD";

				m_args.AddArgument(ArgRegister(Rb));
				if (code & (0x1 << 11)) // with SP
					m_args.AddArgument(ArgRegister(13, false, true));
				else // with PC
					m_args.AddArgument(ArgRegister(15, false, true));
				m_args.AddArgument(ArgImmediate(Imm << 2));
			}
			else
			{
				m_operator = "Unknown";
			}
		}

#undef Rb
#undef Ro
#undef Rs
#undef Rd
#undef Imm
#undef Off

#undef HiRs
#undef HiRd
	}
}
