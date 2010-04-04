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
#include "NstBoardSachenS8259.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sachen
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void S8259::SubReset(const bool hard)
				{
					for (uint i=0x4100; i < 0x8000; i += 0x200)
					{
						for (uint j=0; j < 0x100; j += 0x2)
						{
							Map( i + j + 0x0, &S8259::Poke_4100 );
							Map( i + j + 0x1, &S8259::Poke_4101 );
						}
					}

					if (hard)
					{
						ctrl = 0;

						for (uint i=0; i < 8; ++i)
							regs[i] = 0;

						prg.SwapBank<SIZE_32K,0x0000>(0);
					}

					if (board == Type::SACHEN_8259D && !chr.Source().Writable())
						chr.SwapBank<SIZE_4K,0x1000>( ~0U );
				}

				void S8259::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','8','2'>::V) );

					if (baseChunk == AsciiId<'S','8','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								ctrl = state.Read8();
								state.Read( regs );
							}

							state.End();
						}
					}
				}

				void S8259::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'S','8','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( ctrl ).Write( regs ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(S8259,4100)
				{
					ctrl = data;
				}

				NES_POKE_D(S8259,4101)
				{
					regs[ctrl & 0x7] = data;

					switch (ctrl & 0x7)
					{
						case 0x5:

							prg.SwapBank<SIZE_32K,0x0000>( data );
							break;

						case 0x7:
						{
							static const byte lut[4][4] =
							{
								{0,1,0,1},
								{0,0,1,1},
								{0,1,1,1},
								{0,0,0,0}
							};

							ppu.SetMirroring( lut[(data & 0x1) ? 0 : (data >> 1 & 0x3)] );
						}

						default:

							if (!chr.Source().Writable())
							{
								ppu.Update();

								if (board == Type::SACHEN_8259D)
								{
									chr.SwapBanks<SIZE_1K,0x0000>
									(
										(regs[0] & 0x07U),
										(regs[1] & 0x07U) | (regs[4] << 4 & 0x10U),
										(regs[2] & 0x07U) | (regs[4] << 3 & 0x10U),
										(regs[3] & 0x07U) | (regs[4] << 2 & 0x10U) | (regs[6] << 3 & 0x08U)
									);
								}
								else
								{
									const uint h = regs[4] << 3 & 0x38U;
									const uint s = (board == Type::SACHEN_8259A ? 1 : board == Type::SACHEN_8259C ? 2 : 0);

									chr.SwapBanks<SIZE_2K,0x0000>
									(
										(regs[(regs[7] & 0x1U) ? 0 : 0] & 0x07U | h) << s,
										(regs[(regs[7] & 0x1U) ? 0 : 1] & 0x07U | h) << s | (board != Type::SACHEN_8259B ? 1 : 0),
										(regs[(regs[7] & 0x1U) ? 0 : 2] & 0x07U | h) << s | (board == Type::SACHEN_8259C ? 2 : 0),
										(regs[(regs[7] & 0x1U) ? 0 : 3] & 0x07U | h) << s | (board == Type::SACHEN_8259A ? 1 : board == Type::SACHEN_8259C ? 3 : 0)
									);
								}
							}
							break;
					}
				}
			}
		}
	}
}
