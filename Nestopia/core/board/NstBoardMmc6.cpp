////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include "NstBoard.hpp"
#include "NstBoardMmc6.hpp"
#include "../NstFile.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Mmc6::Mmc6(const Context& c)
			: Mmc3(c,REV_B) {}

			void Mmc6::SubReset(const bool hard)
			{
				Mmc3::SubReset( hard );

				reg = 0;

				Map( 0x6000U, 0x6FFFU, NOP_POKE );
				Map( 0x7000U, 0x7FFFU, &Mmc6::Peek_7000, &Mmc6::Poke_7000 );

				for (uint i=0xA001; i < 0xC000; i += 0x2)
					Map( i, &Mmc6::Poke_A001 );
			}

			void Mmc6::Load(File& file)
			{
				if (board.HasBattery())
					file.Load( File::BATTERY, ram, sizeof(ram) );
			}

			void Mmc6::Save(File& file) const
			{
				if (board.HasBattery())
					file.Save( File::BATTERY, ram, sizeof(ram) );
			}

			void Mmc6::SubLoad(State::Loader& state,const dword baseChunk)
			{
				if (baseChunk == AsciiId<'M','M','6'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:

								reg = state.Read8();
								break;

							case AsciiId<'R','A','M'>::V:

								state.Uncompress( ram );
								break;
						}

						state.End();
					}
				}
				else
				{
					Mmc3::SubLoad( state, baseChunk );
				}
			}

			void Mmc6::SubSave(State::Saver& state) const
			{
				Mmc3::SubSave( state );

				state.Begin( AsciiId<'M','M','6'>::V );
				state.Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End();
				state.Begin( AsciiId<'R','A','M'>::V ).Compress( ram ).End();
				state.End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			inline bool Mmc6::IsRamEnabled() const
			{
				return reg & (RAM_LO_BANK_ENABLED|RAM_HI_BANK_ENABLED);
			}

			inline bool Mmc6::IsRamReadable(uint address) const
			{
				return (reg >> (address >> 8 & 0x2)) & 0x20;
			}

			inline bool Mmc6::IsRamWritable(uint address) const
			{
				return ((reg >> (address >> 8 & 0x2)) & 0x30) == 0x30;
			}

			NES_POKE_AD(Mmc6,7000)
			{
				NST_VERIFY( IsRamWritable(address) );

				if (IsRamWritable( address ))
					ram[address & 0x3FF] = data;
			}

			NES_PEEK_A(Mmc6,7000)
			{
				NST_VERIFY( IsRamEnabled() && IsRamReadable(address) );

				if (IsRamEnabled())
					return IsRamReadable(address) ? ram[address & 0x3FF] : 0x00;
				else
					return address >> 8;
			}

			NES_POKE_D(Mmc6,A001)
			{
				if ((reg & 0x1) | (regs.ctrl0 & RAM_ENABLE))
					reg = data | 0x1;
			}
		}
	}
}
