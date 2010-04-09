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
#include "NstBoardBenshengBs5.hpp"
#include "../NstCrc32.hpp"
#include "../NstDipSwitches.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bensheng
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				class Bs5::CartSwitches : public DipSwitches
				{
					enum Type
					{
						CRC_4_IN_1_A = 0x01E54556,
						CRC_4_IN_1_B = 0x6DCE148C,
						CRC_4_IN_1_C = 0x13E55C4C
					};

					uint mode;
					const Type type;

					explicit CartSwitches(Type t)
					: mode(0), type(t) {}

				public:

					static CartSwitches* Create(const Context& c)
					{
						switch (const dword crc = Crc32::Compute(c.prg.Mem(),c.prg.Size()))
						{
							case CRC_4_IN_1_A:
							case CRC_4_IN_1_B:
							case CRC_4_IN_1_C:

								return new CartSwitches( static_cast<Type>(crc) );
						}

						return NULL;
					}

					void SetMode(uint value)
					{
						mode = value & 0x3;
					}

					uint GetMode() const
					{
						return mode;
					}

				private:

					uint GetValue(uint) const
					{
						return mode;
					}

					void SetValue(uint,uint value)
					{
						mode = value;
					}

					uint NumValues(uint) const
					{
						return 4;
					}

					uint NumDips() const
					{
						return 1;
					}

					cstring GetDipName(uint) const
					{
						return "Mode";
					}

					cstring GetValueName(uint,uint i) const
					{
						switch (type)
						{
							case CRC_4_IN_1_A:
							{
								static const char names[4][9] =
								{
									"4-in-1",
									"23-in-1",
									"53-in-1",
									"163-in-1"
								};

								return names[i];
							}

							case CRC_4_IN_1_B:
							{
								static const char names[4][9] =
								{
									"4-in-1",
									"32-in-1",
									"64-in-1",
									"128-in-1"
								};

								return names[i];
							}

							case CRC_4_IN_1_C:
							{
								static const char names[4][9] =
								{
									"4-in-1",
									"21-in-1",
									"81-in-1",
									"151-in-1"
								};

								return names[i];
							}
						}

						return NULL;
					}
				};

				Bs5::Bs5(const Context& c)
				: Board(c), cartSwitches(CartSwitches::Create(c)) {}

				Bs5::~Bs5()
				{
					delete cartSwitches;
				}

				void Bs5::SubReset(const bool hard)
				{
					if (hard)
						prg.SwapBanks<SIZE_8K,0x0000>( ~0U, ~0U, ~0U, ~0U );

					Map( 0x8000U, 0x8FFFU, &Bs5::Poke_8000 );
					Map( 0xA000U, 0xAFFFU, &Bs5::Poke_A000 );
				}

				Bs5::Device Bs5::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void Bs5::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'B','S','5'>::V) );

					if (baseChunk == AsciiId<'B','S','5'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'D','I','P'>::V)
							{
								NST_VERIFY( cartSwitches );

								if (cartSwitches)
									cartSwitches->SetMode( state.Read8() );
							}

							state.End();
						}
					}
				}

				void Bs5::SubSave(State::Saver& state) const
				{
					if (cartSwitches)
						state.Begin( AsciiId<'B','S','5'>::V ).Begin( AsciiId<'D','I','P'>::V ).Write8( cartSwitches->GetMode() ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_A(Bs5,8000)
				{
					ppu.Update();
					chr.SwapBank<SIZE_2K>( address << 1 & 0x1800, address & 0x1F );

				}

				NES_POKE_A(Bs5,A000)
				{
					if (address & (0x10U << (cartSwitches ? cartSwitches->GetMode() : 0)))
						prg.SwapBank<SIZE_8K>( address << 3 & 0x6000, address & 0xF );
				}
			}
		}
	}
}
