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

#include <cstring>
#include "../NstClock.hpp"
#include "NstBoard.hpp"
#include "NstBoardBandai.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Lz93d50Ex::Lz93d50Ex(const Context& c)
				:
				Lz93d50 (c),
				x24c01  (board == Type::BANDAI_DATACH || board == Type::BANDAI_LZ93D50_24C01 ? new X24C01 : NULL),
				x24c02  (board == Type::BANDAI_DATACH || board == Type::BANDAI_LZ93D50_24C02 ? new X24C02 : NULL)
				{
				}

				void Lz93d50Ex::SubReset(const bool hard)
				{
					Lz93d50::SubReset( hard );

					if (x24c01)
						x24c01->Reset();

					if (x24c02)
						x24c02->Reset();

					if (x24c01 && x24c02)
					{
						for (uint i=0x6000; i < 0x8000; i += 0x100)
							Map( i, &Lz93d50Ex::Peek_6000_24c01_24c02 );

						for (dword i=0x6000; i < 0x10000; i += 0x10)
						{
							Map( uint(i + 0x0), uint(i + 0x7), &Lz93d50Ex::Poke_8000_24c01_24c02 );
							Map( uint(i + 0xD),                &Lz93d50Ex::Poke_800D_24c01_24c02 );
						}
					}
					else if (x24c01)
					{
						for (uint i=0x6000; i < 0x8000; i += 0x100)
							Map( i, &Lz93d50Ex::Peek_6000_24c01 );

						for (dword i=0x600D; i < 0x10000; i += 0x10)
							Map( uint(i), &Lz93d50Ex::Poke_800D_24c01 );
					}
					else
					{
						for (uint i=0x6000; i < 0x8000; i += 0x100)
							Map( i, &Lz93d50Ex::Peek_6000_24c02 );

						for (dword i=0x600D; i < 0x10000; i += 0x10)
							Map( uint(i), &Lz93d50Ex::Poke_800D_24c02 );
					}
				}

				void Lz93d50Ex::Load(File& file)
				{
					const File::LoadBlock block[] =
					{
						{ x24c02 ? x24c02->GetData() : NULL, x24c02 ? X24C02::SIZE : 0 },
						{ x24c01 ? x24c01->GetData() : NULL, x24c01 ? X24C01::SIZE : 0 }
					};

					file.Load( File::EEPROM, block );
				}

				void Lz93d50Ex::Save(File& file) const
				{
					const File::SaveBlock block[] =
					{
						{ x24c02 ? x24c02->GetData() : NULL, x24c02 ? X24C02::SIZE : 0 },
						{ x24c01 ? x24c01->GetData() : NULL, x24c01 ? X24C01::SIZE : 0 }
					};

					file.Save( File::EEPROM, block );
				}

				void Lz93d50Ex::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','L','E'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'C','0','1'>::V:

									NST_VERIFY( x24c01 );

									if (x24c01)
										x24c01->LoadState( state );

									break;

								case AsciiId<'C','0','2'>::V:

									NST_VERIFY( x24c02 );

									if (x24c02)
										x24c02->LoadState( state );

									break;
							}

							state.End();
						}
					}
					else
					{
						Lz93d50::SubLoad( state, baseChunk );
					}
				}

				void Lz93d50Ex::SubSave(State::Saver& state) const
				{
					Lz93d50::SubSave( state );

					state.Begin( AsciiId<'B','L','E'>::V );

					if (x24c01)
						x24c01->SaveState( state, AsciiId<'C','0','1'>::V );

					if (x24c02)
						x24c02->SaveState( state, AsciiId<'C','0','2'>::V );

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK(Lz93d50Ex,6000_24c01)
				{
					return x24c01->Read();
				}

				NES_PEEK(Lz93d50Ex,6000_24c02)
				{
					return x24c02->Read();
				}

				NES_PEEK(Lz93d50Ex,6000_24c01_24c02)
				{
					return x24c01->Read() & x24c02->Read();
				}

				NES_POKE_D(Lz93d50Ex,8000_24c01_24c02)
				{
					x24c01->SetScl( data << 2 & 0x20 );
				}

				NES_POKE_D(Lz93d50Ex,800D_24c01)
				{
					x24c01->Set( data & 0x20, data & 0x40 );
				}

				NES_POKE_D(Lz93d50Ex,800D_24c02)
				{
					x24c02->Set( data & 0x20, data & 0x40 );
				}

				NES_POKE_D(Lz93d50Ex,800D_24c01_24c02)
				{
					x24c01->SetSda( data & 0x40 );
					x24c02->Set( data & 0x20, data & 0x40 );
				}
			}
		}
	}
}
