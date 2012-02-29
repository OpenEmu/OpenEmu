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

#include "ameteor/bios.hpp"
#include "ameteor/cpu.hpp"
#include "ameteor/memory.hpp"

#include "globals.hpp"

#include "debug.hpp"

namespace AMeteor
{
	namespace Bios
	{
		static const int16_t sineTable[256] = {
			(int16_t)0x0000, (int16_t)0x0192, (int16_t)0x0323, (int16_t)0x04B5,
			(int16_t)0x0645, (int16_t)0x07D5, (int16_t)0x0964, (int16_t)0x0AF1,
			(int16_t)0x0C7C, (int16_t)0x0E05, (int16_t)0x0F8C, (int16_t)0x1111,
			(int16_t)0x1294, (int16_t)0x1413, (int16_t)0x158F, (int16_t)0x1708,
			(int16_t)0x187D, (int16_t)0x19EF, (int16_t)0x1B5D, (int16_t)0x1CC6,
			(int16_t)0x1E2B, (int16_t)0x1F8B, (int16_t)0x20E7, (int16_t)0x223D,
			(int16_t)0x238E, (int16_t)0x24DA, (int16_t)0x261F, (int16_t)0x275F,
			(int16_t)0x2899, (int16_t)0x29CD, (int16_t)0x2AFA, (int16_t)0x2C21,
			(int16_t)0x2D41, (int16_t)0x2E5A, (int16_t)0x2F6B, (int16_t)0x3076,
			(int16_t)0x3179, (int16_t)0x3274, (int16_t)0x3367, (int16_t)0x3453,
			(int16_t)0x3536, (int16_t)0x3612, (int16_t)0x36E5, (int16_t)0x37AF,
			(int16_t)0x3871, (int16_t)0x392A, (int16_t)0x39DA, (int16_t)0x3A82,
			(int16_t)0x3B20, (int16_t)0x3BB6, (int16_t)0x3C42, (int16_t)0x3CC5,
			(int16_t)0x3D3E, (int16_t)0x3DAE, (int16_t)0x3E14, (int16_t)0x3E71,
			(int16_t)0x3EC5, (int16_t)0x3F0E, (int16_t)0x3F4E, (int16_t)0x3F84,
			(int16_t)0x3FB1, (int16_t)0x3FD3, (int16_t)0x3FEC, (int16_t)0x3FFB,
			(int16_t)0x4000, (int16_t)0x3FFB, (int16_t)0x3FEC, (int16_t)0x3FD3,
			(int16_t)0x3FB1, (int16_t)0x3F84, (int16_t)0x3F4E, (int16_t)0x3F0E,
			(int16_t)0x3EC5, (int16_t)0x3E71, (int16_t)0x3E14, (int16_t)0x3DAE,
			(int16_t)0x3D3E, (int16_t)0x3CC5, (int16_t)0x3C42, (int16_t)0x3BB6,
			(int16_t)0x3B20, (int16_t)0x3A82, (int16_t)0x39DA, (int16_t)0x392A,
			(int16_t)0x3871, (int16_t)0x37AF, (int16_t)0x36E5, (int16_t)0x3612,
			(int16_t)0x3536, (int16_t)0x3453, (int16_t)0x3367, (int16_t)0x3274,
			(int16_t)0x3179, (int16_t)0x3076, (int16_t)0x2F6B, (int16_t)0x2E5A,
			(int16_t)0x2D41, (int16_t)0x2C21, (int16_t)0x2AFA, (int16_t)0x29CD,
			(int16_t)0x2899, (int16_t)0x275F, (int16_t)0x261F, (int16_t)0x24DA,
			(int16_t)0x238E, (int16_t)0x223D, (int16_t)0x20E7, (int16_t)0x1F8B,
			(int16_t)0x1E2B, (int16_t)0x1CC6, (int16_t)0x1B5D, (int16_t)0x19EF,
			(int16_t)0x187D, (int16_t)0x1708, (int16_t)0x158F, (int16_t)0x1413,
			(int16_t)0x1294, (int16_t)0x1111, (int16_t)0x0F8C, (int16_t)0x0E05,
			(int16_t)0x0C7C, (int16_t)0x0AF1, (int16_t)0x0964, (int16_t)0x07D5,
			(int16_t)0x0645, (int16_t)0x04B5, (int16_t)0x0323, (int16_t)0x0192,
			(int16_t)0x0000, (int16_t)0xFE6E, (int16_t)0xFCDD, (int16_t)0xFB4B,
			(int16_t)0xF9BB, (int16_t)0xF82B, (int16_t)0xF69C, (int16_t)0xF50F,
			(int16_t)0xF384, (int16_t)0xF1FB, (int16_t)0xF074, (int16_t)0xEEEF,
			(int16_t)0xED6C, (int16_t)0xEBED, (int16_t)0xEA71, (int16_t)0xE8F8,
			(int16_t)0xE783, (int16_t)0xE611, (int16_t)0xE4A3, (int16_t)0xE33A,
			(int16_t)0xE1D5, (int16_t)0xE075, (int16_t)0xDF19, (int16_t)0xDDC3,
			(int16_t)0xDC72, (int16_t)0xDB26, (int16_t)0xD9E1, (int16_t)0xD8A1,
			(int16_t)0xD767, (int16_t)0xD633, (int16_t)0xD506, (int16_t)0xD3DF,
			(int16_t)0xD2BF, (int16_t)0xD1A6, (int16_t)0xD095, (int16_t)0xCF8A,
			(int16_t)0xCE87, (int16_t)0xCD8C, (int16_t)0xCC99, (int16_t)0xCBAD,
			(int16_t)0xCACA, (int16_t)0xC9EE, (int16_t)0xC91B, (int16_t)0xC851,
			(int16_t)0xC78F, (int16_t)0xC6D6, (int16_t)0xC626, (int16_t)0xC57E,
			(int16_t)0xC4E0, (int16_t)0xC44A, (int16_t)0xC3BE, (int16_t)0xC33B,
			(int16_t)0xC2C2, (int16_t)0xC252, (int16_t)0xC1EC, (int16_t)0xC18F,
			(int16_t)0xC13B, (int16_t)0xC0F2, (int16_t)0xC0B2, (int16_t)0xC07C,
			(int16_t)0xC04F, (int16_t)0xC02D, (int16_t)0xC014, (int16_t)0xC005,
			(int16_t)0xC000, (int16_t)0xC005, (int16_t)0xC014, (int16_t)0xC02D,
			(int16_t)0xC04F, (int16_t)0xC07C, (int16_t)0xC0B2, (int16_t)0xC0F2,
			(int16_t)0xC13B, (int16_t)0xC18F, (int16_t)0xC1EC, (int16_t)0xC252,
			(int16_t)0xC2C2, (int16_t)0xC33B, (int16_t)0xC3BE, (int16_t)0xC44A,
			(int16_t)0xC4E0, (int16_t)0xC57E, (int16_t)0xC626, (int16_t)0xC6D6,
			(int16_t)0xC78F, (int16_t)0xC851, (int16_t)0xC91B, (int16_t)0xC9EE,
			(int16_t)0xCACA, (int16_t)0xCBAD, (int16_t)0xCC99, (int16_t)0xCD8C,
			(int16_t)0xCE87, (int16_t)0xCF8A, (int16_t)0xD095, (int16_t)0xD1A6,
			(int16_t)0xD2BF, (int16_t)0xD3DF, (int16_t)0xD506, (int16_t)0xD633,
			(int16_t)0xD767, (int16_t)0xD8A1, (int16_t)0xD9E1, (int16_t)0xDB26,
			(int16_t)0xDC72, (int16_t)0xDDC3, (int16_t)0xDF19, (int16_t)0xE075,
			(int16_t)0xE1D5, (int16_t)0xE33A, (int16_t)0xE4A3, (int16_t)0xE611,
			(int16_t)0xE783, (int16_t)0xE8F8, (int16_t)0xEA71, (int16_t)0xEBED,
			(int16_t)0xED6C, (int16_t)0xEEEF, (int16_t)0xF074, (int16_t)0xF1FB,
			(int16_t)0xF384, (int16_t)0xF50F, (int16_t)0xF69C, (int16_t)0xF82B,
			(int16_t)0xF9BB, (int16_t)0xFB4B, (int16_t)0xFCDD, (int16_t)0xFE6E
		};

		void Bios000h ()
		{
			debug("Bios entry point");
			R(13) = 0x03007FE0;
			R(15) = 0x08000004;
			CPU.SwitchToMode(Cpu::M_IRQ);
			R(13) = 0x03007FA0;
			CPU.SwitchToMode(Cpu::M_SYS);
			R(13) = 0x03007F00;
			ICPSR.irq_d = false;
			IO.Write8(Io::POSTFLG, 0x01);
		}

		void Bios008h ()
		{
			// if we are here, we should be in SVC mode (0x13)
			// store the spsr, r11, r12 and r14 on the stack
			uint32_t baseadd = R(13) - (4*4), add = (baseadd & 0xFFFFFFFC);
			MEM.Write32(add     , SPSR);
			MEM.Write32(add += 4, R(11));
			MEM.Write32(add += 4, R(12));
			MEM.Write32(add +  4, R(14));
			R(13) = baseadd;

			uint8_t swiComment = MEM.Read8(R(14) - 2);

			// put 0x1F in cpsr but don't touch to the irq disable bit
			CPU.SwitchToMode(0x1F);
			CPSR = 0x0000001F | (CPSR & (0x1 << 7));
			CPU.UpdateICpsr();

			// store r11 and r14 (of the user mode) on the stack
			baseadd = R(13) - (2*4); add = (baseadd & 0xFFFFFFFC);
			MEM.Write32(add     , R(11));
			MEM.Write32(add +  4, R(14));
			R(13) = baseadd;

			R(14) = 0x168;

			debug("Software IRQ start");
			switch (swiComment)
			{
				case 0x04:
					IntrWait();
					break;
				case 0x05:
					VBlankIntrWait();
					break;
				default:
					met_abort("not implemented : " << (int)swiComment);
					break;
			}
		}

		void Bios168h ()
		{
			uint32_t add = R(13) & 0xFFFFFFFC;
			R( 2) = MEM.Read32(add     );
			R(14) = MEM.Read32(add += 4);
			R(13) += 2*4;

			// SVC with fiq and irq disabled
			CPU.SwitchToMode(0x13); // SVC
			CPSR = 0x000000D3;
			CPU.UpdateICpsr();
			add = R(13) & 0xFFFFFFFC;

			SPSR = MEM.Read32(add);

			R(11) = MEM.Read32(add += 4);
			R(12) = MEM.Read32(add += 4);
			R(14) = MEM.Read32(add +  4);
			R(13) += 4*4;

			// FIXME this works (for thumb) ?
			if (CPU.Spsr().b.thumb)
				R(15) = R(14) + 2;
			else
				R(15) = R(14) + 4;

			debug("Software IRQ end");
			CPU.SwitchModeBack();
		}

		void Bios018h ()
		{
			debug("IRQ start");
			// stmfd r13!,r0-r3,r12,r14
			uint32_t baseadd = R(13) - (6*4), add = (baseadd & 0xFFFFFFFC);
			MEM.Write32(add     , R( 0));
			MEM.Write32(add += 4, R( 1));
			MEM.Write32(add += 4, R( 2));
			MEM.Write32(add += 4, R( 3));
			MEM.Write32(add += 4, R(12));
			MEM.Write32(add +  4, R(14));

			R(13) = baseadd;

			// add r14,r15,0h
			R(14) = 0x00000130;

			R(15) = MEM.Read32(0x03007FFC) + 4;
		}

		void Bios130h ()
		{
			debug("IRQ end");
			// ldmfd r13!,r0-r3,r12,r14
			uint32_t add = R(13) & 0xFFFFFFFC;
			R( 0) = MEM.Read32(add     );
			R( 1) = MEM.Read32(add += 4);
			R( 2) = MEM.Read32(add += 4);
			R( 3) = MEM.Read32(add += 4);
			R(12) = MEM.Read32(add += 4);
			R(14) = MEM.Read32(add +  4);

			R(13) += 6*4;

			// subs r15,r14,4h
			R(15) = R(14);
			if (CPU.Spsr().b.thumb)
				R(15) -= 2;

			CPU.SwitchModeBack();

			// XXX FIXME, usefull ? test on breath of fire !
			/*if (FLAG_T)
				R(15) &= 0xFFFFFFFE;
			else
				R(15) &= 0xFFFFFFFC;*/
		}

		void SoftReset ()
		{
			CPU.SoftReset ();
			if (MEM.Read8(0x03007FFA))
				R(15) = 0x02000004;
			else
				R(15) = 0x08000004;

			MEM.SoftReset ();
		}

		void RegisterRamReset ()
		{
			IO.Write16(Io::DISPCNT, 0x0080);
			uint8_t flagRes = R(0);
			if (flagRes & (0x1     ))
				MEM.ClearWbram();
			if (flagRes & (0x1 << 1))
				MEM.ClearWcram();
			if (flagRes & (0x1 << 2))
				MEM.ClearPalette();
			if (flagRes & (0x1 << 3))
				MEM.ClearVram();
			if (flagRes & (0x1 << 4))
				MEM.ClearOam();
			if (flagRes & (0x1 << 5))
				IO.ClearSio ();
			if (flagRes & (0x1 << 6))
				IO.ClearSound ();
			if (flagRes & (0x1 << 7))
				IO.ClearOthers ();
		}

		void Halt ()
		{
			IO.Write8(Io::HALTCNT, 0);
		}

		void IntrWait ()
		{
			// FIXME ugly
			R(13) -= 8;
			MEM.Write32(R(13) & 0xFFFFFFFC, R(4));
			MEM.Write32((R(13)+4) & 0xFFFFFFFC, R(14));

			uint16_t& intFlags = *(uint16_t*)MEM.GetRealAddress(0x03007FF8);

			if (R(0))
			{
				if (intFlags & R(1))
					intFlags = (intFlags & R(1)) ^ intFlags;
				else
					FLAG_Z = 1;
				IO.Write16(Io::IME, 1);
			}

			IO.Write8(Io::HALTCNT, 0);

			// return address (after IRQ)
			R(15) = 0x33C;

			debug("IntrWait start");
		}

		void Bios338h ()
		{
			uint16_t& intFlags = *(uint16_t*)MEM.GetRealAddress(0x03007FF8);

			if (!(intFlags & R(1)))
			{
				IO.Write16(Io::IME, 1);
				IO.Write8(Io::HALTCNT, 0);
			}
			else
			{
				intFlags = (intFlags & R(1)) ^ intFlags;
				IO.Write16(Io::IME, 1);

				// FIXME ugly
				R(4) = MEM.Read32(R(13) & 0xFFFFFFFC);
				R(14) = MEM.Read32((R(13)+4) & 0xFFFFFFFC);
				R(13) += 8;

				// should lead to 0x168
				R(15) = R(14)+4;
			}

			debug("IntWait end");
		}

		void VBlankIntrWait ()
		{
			R(0) = 1;
			R(1) = 1;
			IntrWait();
		}

		void Div ()
		{
			if (!R(1))
				met_abort("Div by 0");

			int32_t number = R(0), denom = R(1);

			int32_t div = number / denom;
			R(0) = div;
			R(1) = number % denom;
			R(3) = div < 0 ? -div : div;
		}

		void DivArm ()
		{
			uint32_t tmp = R(0);
			R(0) = R(1);
			R(1) = tmp;
			Div();
		}

		void ArcTan ()
		{
			int32_t a = -(((int32_t)R(0) * R(0)) >> 14);
			int32_t b = 0xA9;
			b = ((a * b) >> 14) + 0x0390;
			b = ((a * b) >> 14) + 0x091C;
			b = ((a * b) >> 14) + 0x0FB6;
			b = ((a * b) >> 14) + 0X16AA;
			b = ((a * b) >> 14) + 0X2081;
			b = ((a * b) >> 14) + 0X3651;
			b = ((a * b) >> 14) + 0XA2F9;

			R(0) = (R(0) * b) >> 16;
		}

		void ArcTan2 ()
		{
			int16_t x = R(0), y = R(1);
			if (y)
				if (x)
					if (abs(x) < abs(y))
					{
						R(0) <<= 14;
						Div();
						ArcTan();
						R(0) = 0x4000 - R(0);
						if (y < 0)
							R(0) += 0x8000;
					}
					else
					{
						uint32_t r1 = R(1);
						R(1) = R(0);
						R(0) = r1 << 14;
						Div();
						ArcTan();
						if (x < 0)
							R(0) += 0x8000;
						else if (y < 0)
							R(0) += 0x10000;
					}
				else
					if (y < 0)
						R(0) = 0xc000;
					else
						R(0) = 0x4000;
			else
				if (x < 0)
					R(0) = 0x8000;
				else
					R(0) = 0x0000;
		}

		void CpuSet ()
		{
			if (R(2) & (0x1 << 26)) // 32 bits
			{
				if (R(2) & (0x1 << 24)) // fixed source address
				{
					uint32_t source = MEM.Read32(R(0) & 0xFFFFFFFC);
					uint32_t address = R(1) & 0xFFFFFFFC;
					for (uint32_t count = (R(2) & 0x001FFFFF); count; --count)
					{
						MEM.Write32(address, source);
						address += 4;
					}
				}
				else // copy
				{
					uint32_t src = R(0) & 0xFFFFFFFC;
					uint32_t dest = R(1) & 0xFFFFFFFC;
					for (uint32_t count = (R(2) & 0x001FFFFF); count; --count)
					{
						MEM.Write32(dest, MEM.Read32(src));
						src += 4;
						dest += 4;
					}
				}
			}
			else // 16 bits
			{
				if (R(2) & (0x1 << 24)) // fixed source address
				{
					uint16_t source = MEM.Read16(R(0));
					uint32_t address = R(1);
					for (uint32_t count = (R(2) & 0x001FFFFF); count; --count)
					{
						MEM.Write16(address, source);
						address += 2;
					}
				}
				else // copy
				{
					uint32_t src = R(0);
					uint32_t dest = R(1);
					for (uint32_t count = (R(2) & 0x001FFFFF); count; --count)
					{
						MEM.Write16(dest, MEM.Read16(src));
						src += 2;
						dest += 2;
					}
				}
			}
		}

		void CpuFastSet ()
		{
			if (R(2) & (0x1 << 24)) // fixed source address
			{
				uint32_t source = MEM.Read32(R(0));
				uint32_t address = R(1);
				for (uint32_t count = (R(2) & 0x001FFFFF); count; --count)
				{
					MEM.Write32(address, source);
					address += 4;
				}
			}
			else // copy
			{
				uint32_t src = R(0);
				uint32_t dest = R(1);
				for (uint32_t count = (R(2) & 0x001FFFFF); count; --count)
				{
					MEM.Write32(dest, MEM.Read32(src));
					src += 4;
					dest += 4;
				}
			}
		}

		void BgAffineSet ()
		{
			uint32_t src = R(0);
			uint32_t dest = R(1);
			uint32_t num = R(2);

			int32_t cx, cy;
			int16_t dix, diy, rx, ry;
			uint16_t alpha;

			int32_t cos, sin;
			int16_t dx, dmx, dy, dmy;

			while (num--)
			{
				cx = MEM.Read32(src);
				src += 4;
				cy = MEM.Read32(src);
				src += 4;
				dix = MEM.Read16(src);
				src += 2;
				diy = MEM.Read16(src);
				src += 2;
				rx = MEM.Read16(src);
				src += 2;
				ry = MEM.Read16(src);
				src += 2;
				alpha = MEM.Read16(src) >> 8;
				src += 2;

				sin = sineTable[alpha];
				cos = sineTable[(alpha + 0x40) & 0xFF];

				dx = (rx * cos) >> 14;
				dmx = -((rx * sin) >> 14);
				dy = (ry * sin) >> 14;
				dmy = (ry * cos) >> 14;

				MEM.Write16(dest, dx);
				dest += 2;
				MEM.Write16(dest, dmx);
				dest += 2;
				MEM.Write16(dest, dy);
				dest += 2;
				MEM.Write16(dest, dmy);
				dest += 2;

				MEM.Write32(dest, cx - dx * dix - dmx * diy);
				dest += 4;
				MEM.Write32(dest, cy - dy * dix - dmy * diy);
				dest += 4;
			}
		}

		void ObjAffineSet ()
		{
			uint32_t src = R(0);
			uint32_t dest = R(1);
			uint32_t num = R(2);
			uint32_t off = R(3);

			int16_t rx, ry;
			uint16_t alpha;

			int32_t cos, sin;
			int16_t dx, dmx, dy, dmy;

			while (num--)
			{
				rx = MEM.Read16(src);
				src += 2;
				ry = MEM.Read16(src);
				src += 2;
				alpha = MEM.Read16(src) >> 8;
				src += 4;

				sin = sineTable[alpha];
				cos = sineTable[(alpha + 0x40) & 0xFF];

				dx = (rx * cos) >> 14;
				dmx = -((rx * sin) >> 14);
				dy = (ry * sin) >> 14;
				dmy = (ry * cos) >> 14;

				MEM.Write16(dest, dx);
				dest += off;
				MEM.Write16(dest, dmx);
				dest += off;
				MEM.Write16(dest, dy);
				dest += off;
				MEM.Write16(dest, dmy);
				dest += off;
			}
		}

		void LZ77UnCompWram ()
		{
			uint32_t src = R(0);
			uint32_t header = MEM.Read32(src);
			src += 4;
			if (((header >> 4) & 0xF) != 1)
				met_abort("This is not LZ77 data");
			uint32_t size = header >> 8;
			debug("LZ77UnCompWram from " << IOS_ADD << R(0) << " to " << IOS_ADD << R(1) << ", len : " << size);
			uint32_t dest = R(1);
			uint8_t flags;
			uint16_t block;
			uint8_t blocklen;
			uint32_t realaddr;

			// for each block of a flags byte + 8 blocks
			while (true)
			{
				flags = MEM.Read8(src++);

				for (uint8_t i = 0; i < 8; ++i)
				{
					// compressed block of 2 bytes
					if (flags & 0x80)
					{
						block = MEM.Read8(src) << 8 | MEM.Read8(src+1);
						src += 2;
						blocklen = (block >> 12) + 3;
						realaddr = dest - (block & 0x0FFF) - 1;
						for(uint16_t j = 0; j < blocklen; ++j)
						{
							MEM.Write8(dest++, MEM.Read8(realaddr++));

							--size;
							if(size == 0)
							{
								size = header >> 8;
								return;
							}
						}
					}
					// uncompressed block of 1 byte
					else
					{
						MEM.Write8(dest++, MEM.Read8(src++));

						--size;
						if (size == 0)
						{
							size = header >> 8;
							return;
						}
					}

					flags <<= 1;
				}
			}
		}

		void LZ77UnCompVram ()
		{
			uint32_t src = R(0);
			uint32_t header = MEM.Read32(src);
			src += 4;
			if (((header >> 4) & 0xF) != 1)
				met_abort("This is not LZ77 data");
			uint32_t size = header >> 8;
			debug("LZ77UnCompVram from " << IOS_ADD << R(0) << " to " << IOS_ADD << R(1) << ", len : " << size);
			uint32_t dest = R(1);
			uint8_t flags;
			uint16_t out = 0;
			uint8_t shift = 0;
			uint16_t block;
			uint8_t blocklen;
			uint32_t realaddr;

			// for each block of a flags byte + 8 blocks
			while (true)
			{
				flags = MEM.Read8(src++);

				for (uint8_t i = 0; i < 8; ++i)
				{
					// compressed block of 2 bytes
					if (flags & 0x80)
					{
						block = MEM.Read8(src) << 8 | MEM.Read8(src+1);
						src += 2;
						blocklen = (block >> 12) + 3;
						realaddr = dest + (shift/8) - (block & 0x0FFF) - 1;
						for(uint16_t j = 0; j < blocklen; ++j) {
							out |= MEM.Read8(realaddr++) << shift;
							shift += 8;

							if(shift == 16) {
								MEM.Write16(dest, out);
								dest += 2;
								out = 0;
								shift = 0;
							}

							--size;
							if(size == 0)
							{
								size = header >> 8;
								return;
							}
						}
					}
					// uncompressed block of 1 byte
					else
					{
						out |= MEM.Read8(src++) << shift;
						shift += 8;

						if (shift == 16)
						{
							MEM.Write16(dest, out);
							dest += 2;
							shift = 0;
							out = 0;
						}

						--size;
						if (size == 0)
						{
							size = header >> 8;
							return;
						}
					}

					flags <<= 1;
				}
			}
		}

		void HuffUnComp ()
		{
			uint32_t src = R(0) & 0xFFFFFFFC;
			uint32_t dest = R(1);
			uint32_t header = MEM.Read32(src);
			src += 4;
			if (((header >> 4) & 0xF) != 2)
				met_abort("This is not Huffman data");
			uint8_t blockLen = header & 0xF;
			uint32_t size = header >> 8;
			if (size % 4)
				met_abort("Size not multiple of 4 in HuffUnComp");
			uint32_t treeStart = src + 1;
			src += 2 + MEM.Read8(src) * 2;

			uint32_t cData = MEM.Read32(src);
			src += 4;
			uint32_t mask = 0x80000000;
			uint32_t treePos = treeStart;
			uint8_t node = MEM.Read8(treePos);
			bool endNode = false;
			uint32_t oData = 0;
			uint8_t oShift = 0;

			while (size)
			{
				treePos = (treePos & 0xFFFFFFFE) + (node & 0x3F) * 2 + 2;
				if (cData & mask)
				{
					++treePos;
					if (node & (0x1 << 6))
						endNode = true;
				}
				else
				{
					if (node & (0x1 << 7))
						endNode = true;
				}
				node = MEM.Read8(treePos);

				if (endNode)
				{
					oData |= ((uint32_t)node) << oShift;
					oShift += blockLen;

					if (oShift >= 32)
					{
						MEM.Write32(dest, oData);
						dest += 4;
						size -= 4;

						oShift -= 32;
						if (oShift)
							oData = node >> (8 - oShift);
						else
							oData = 0;
					}
					endNode = false;
					treePos = treeStart;
					node = MEM.Read8(treePos);
				}

				mask >>= 1;
				if (!mask)
				{
					cData = MEM.Read32(src);
					src += 4;
					mask = 0x80000000;
				}
			}
		}

		void RLUnCompWram ()
		{
			uint32_t src = R(0);
			uint32_t header = MEM.Read32(src);
			src += 4;
			if (((header >> 4) & 0xF) != 3)
				met_abort("This is not RL data");
			uint32_t size = header >> 8;
			debug("RLUnCompWram from " << IOS_ADD << R(0) << " to " << IOS_ADD << R(1) << ", len : " << size);
			uint32_t dest = R(1);
			uint8_t flags;
			uint8_t block;
			uint8_t blocklen;

			// for each block
			while (true)
			{
				flags = MEM.Read8(src++);
				blocklen = flags & 0x7F;

				// compressed block
				if (flags & 0x80)
				{
					blocklen += 3;
					block = MEM.Read8(src++);

					for(uint8_t i = 0; i < blocklen; ++i) {
						MEM.Write8(dest++, block);

						--size;
						if(size == 0)
						{
							size = header >> 8;
							return;
						}
					}
				}
				// uncompressed block
				else
				{
					blocklen += 1;

					for (uint8_t i = 0; i < blocklen; ++i)
					{
						MEM.Write8(dest++, MEM.Read8(src++));

						--size;
						if (size == 0)
						{
							size = header >> 8;
							return;
						}
					}
				}
			}
		}

		void RLUnCompVram ()
		{
			uint32_t src = R(0);
			uint32_t header = MEM.Read32(src);
			src += 4;
			if (((header >> 4) & 0xF) != 3)
				met_abort("This is not RL data");
			uint32_t size = header >> 8;
			debug("RLUnCompVram from " << IOS_ADD << R(0) << " to " << IOS_ADD << R(1) << ", len : " << size);
			uint32_t dest = R(1);
			uint8_t flags;
			uint16_t out = 0;
			uint8_t shift = 0;
			uint8_t block;
			uint8_t blocklen;

			// for each block
			while (true)
			{
				flags = MEM.Read8(src++);
				blocklen = flags & 0x7F;

				// compressed block
				if (flags & 0x80)
				{
					blocklen += 3;
					block = MEM.Read8(src++);

					for(uint8_t i = 0; i < blocklen; ++i) {
						out |= block << shift;
						shift += 8;

						if(shift == 16) {
							MEM.Write16(dest, out);
							dest += 2;
							out = 0;
							shift = 0;
						}

						--size;
						if(size == 0)
						{
							size = header >> 8;
							return;
						}
					}
				}
				// uncompressed block
				else
				{
					blocklen += 1;

					for (uint8_t i = 0; i < blocklen; ++i)
					{
						out |= MEM.Read8(src++) << shift;
						shift += 8;

						if (shift == 16)
						{
							MEM.Write16(dest, out);
							dest += 2;
							shift = 0;
							out = 0;
						}

						--size;
						if (size == 0)
						{
							size = header >> 8;
							return;
						}
					}
				}
			}
		}
	}
}
