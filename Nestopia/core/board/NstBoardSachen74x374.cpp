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
#include "NstBoardSachen74x374.hpp"
#include "../NstDipSwitches.hpp"
#include "../NstCrc32.hpp"

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

				class S74x374b::CartSwitches : public DipSwitches
				{
					enum
					{
						CA = 0x858130BF
					};

					uint copyright;

					CartSwitches()
					: copyright(1) {}

				public:

					static CartSwitches* Create(const Context& c)
					{
						return Crc32::Compute(c.prg.Mem(),c.prg.Size()) == CA ? new CartSwitches : NULL;
					}

					void SetCopyright(uint value)
					{
						copyright = value;
					}

					uint GetCopyright() const
					{
						return copyright;
					}

				private:

					uint GetValue(uint) const
					{
						return copyright ^ 1;
					}

					void SetValue(uint,uint value)
					{
						copyright = value ^ 1;
					}

					uint NumDips() const
					{
						return 1;
					}

					uint NumValues(uint) const
					{
						return 2;
					}

					cstring GetDipName(uint) const
					{
						return "Copyright";
					}

					cstring GetValueName(uint,uint i) const
					{
						return i ? "Sachen & Hacker" : "Sachen";
					}
				};

				S74x374b::S74x374b(const Context& c)
				: S74x374a(c), cartSwitches(CartSwitches::Create(c)) {}

				S74x374b::~S74x374b()
				{
					delete cartSwitches;
				}

				S74x374b::Device S74x374b::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void S74x374a::SubReset(const bool hard)
				{
					for (uint i=0x4100; i < 0x6000; i += 0x200)
					{
						for (uint j=0x00; j < 0x100; j += 0x2)
						{
							Map( i + j + 0x0, &S74x374a::Poke_4100 );
							Map( i + j + 0x1, &S74x374a::Poke_4101 );
						}
					}

					if (hard)
					{
						ctrl = 0;
						prg.SwapBank<SIZE_32K,0x0000>(0);
					}
				}

				void S74x374b::SubReset(const bool hard)
				{
					S74x374a::SubReset( hard );

					for (uint i=0x4100; i < 0x6000; i += 0x200)
					{
						for (uint j=0x00; j < 0x100; j += 0x2)
						{
							Map( i + j + 0x0, &S74x374b::Peek_4100 );
							Map( i + j + 0x1, &S74x374b::Peek_4100, &S74x374b::Poke_4101 );
						}
					}
				}

				void S74x374a::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','7','A'>::V) );

					if (baseChunk == AsciiId<'S','7','A'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								ctrl = state.Read8();

							state.End();
						}
					}
				}

				void S74x374b::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'S','7','B'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'D','I','P'>::V)
							{
								NST_VERIFY( cartSwitches );

								if (cartSwitches)
									cartSwitches->SetCopyright( state.Read8() & 0x1 );
							}

							state.End();
						}
					}
					else
					{
						S74x374a::SubLoad( state, baseChunk );
					}
				}

				void S74x374a::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'S','7','A'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( ctrl ).End().End();
				}

				void S74x374b::SubSave(State::Saver& state) const
				{
					S74x374a::SubSave( state );

					if (cartSwitches)
						state.Begin( AsciiId<'S','7','B'>::V ).Begin( AsciiId<'D','I','P'>::V ).Write8( cartSwitches->GetCopyright() ? 0x1 : 0x0 ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void S74x374a::UpdatePrg(uint bank)
				{
					prg.SwapBank<SIZE_32K,0x0000>( bank );
				}

				void S74x374a::UpdateChr(uint bank) const
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000>( bank );
				}

				void S74x374a::UpdateNmt(uint mode) const
				{
					NST_ASSERT( mode <= 0x3 );

					static const byte lut[4][4] =
					{
						{0,1,0,1},
						{0,0,1,1},
						{0,1,1,1},
						{0,0,0,0}
					};

					ppu.SetMirroring( lut[mode] );
				}

				NES_PEEK(S74x374b,4100)
				{
					return (~(ctrl & 0x7) & 0x3F) ^ (cartSwitches ? cartSwitches->GetCopyright() : 0x1);
				}

				NES_POKE_D(S74x374a,4100)
				{
					ctrl = data;
				}

				NES_POKE_D(S74x374a,4101)
				{
					switch (ctrl & 0x7)
					{
						case 0x0:

							UpdatePrg( 0x0 );
							UpdateChr( 0x3 );
							break;

						case 0x2:

							UpdateChr( (chr.GetBank<SIZE_8K,0x0000>() & ~0x8U) | (data << 3 & 0x8) );
							break;

						case 0x4:

							UpdateChr( (chr.GetBank<SIZE_8K,0x0000>() & ~0x1U) | (data << 0 & 0x1) );
							break;

						case 0x5:

							UpdatePrg( data & 0x1 );
							break;

						case 0x6:

							UpdateChr( (chr.GetBank<SIZE_8K,0x0000>() & ~0x6U) | (data << 1 & 0x6) );
							break;

						case 0x7:

							UpdateNmt( data & 0x1 );
							break;
					}
				}

				NES_POKE_D(S74x374b,4101)
				{
					switch (ctrl & 0x7)
					{
						case 0x2:

							UpdatePrg( data & 0x1 );
							UpdateChr( (chr.GetBank<SIZE_8K,0x0000>() & ~0x8U) | (data << 3 & 0x8) );
							break;

						case 0x4:

							UpdateChr( (chr.GetBank<SIZE_8K,0x0000>() & ~0x4U) | (data << 2 & 0x4) );
							break;

						case 0x5:

							UpdatePrg( data & 0x7 );
							break;

						case 0x6:

							UpdateChr( (chr.GetBank<SIZE_8K,0x0000>() & ~0x3U) | (data << 0 & 0x3) );
							break;

						case 0x7:

							UpdateNmt( data >> 1 & 0x3 );
							break;
					}
				}
			}
		}
	}
}
