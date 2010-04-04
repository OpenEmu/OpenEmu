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
#include "NstBoardMmc3.hpp"
#include "NstBoardBmcFk23c.hpp"
#include "../NstCrc32.hpp"
#include "../NstDipSwitches.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				class Fk23c::CartSwitches : public DipSwitches
				{
					enum Type
					{
						CRC_4_IN_1  = 0x38BA830E,
						CRC_6_IN_1  = 0xC16708E8,
						CRC_8_IN_1  = 0x63A87C95,
						CRC_15_IN_1 = 0x30FF6159,
						CRC_18_IN_1 = 0x83A38A2F,
						CRC_20_IN_1 = 0xFD9D1925
					};

					uint mode;
					const Type type;

					explicit CartSwitches(Type t)
					: mode(0), type(t) {}

					uint NumValues(uint) const
					{
						return type == CRC_4_IN_1 || type == CRC_6_IN_1 || type == CRC_8_IN_1 ? 2 : 8;
					}

				public:

					static CartSwitches* Create(const Context& c)
					{
						switch (const dword crc = Crc32::Compute(c.prg.Mem(),c.prg.Size()))
						{
							case CRC_4_IN_1:
							case CRC_6_IN_1:
							case CRC_8_IN_1:
							case CRC_15_IN_1:
							case CRC_18_IN_1:
							case CRC_20_IN_1:

								return new CartSwitches( static_cast<Type>(crc) );
						}

						return NULL;
					}

					void SetMode(uint value)
					{
						const uint num = CartSwitches::NumValues(0) - 1;
						mode = NST_MIN(value,num);
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
							case CRC_4_IN_1:
							{
								static const char names[2][8] =
								{
									"4-in-1",
									"7-in-1"
								};

								return names[i];
							}

							case CRC_6_IN_1:
							{
								static const char names[2][8] =
								{
									"6-in-1",
									"4-in-1"
								};

								return names[i];
							}

							case CRC_8_IN_1:
							{
								static const char names[2][8] =
								{
									"8-in-1",
									"4-in-1"
								};

								return names[i];
							}

							case CRC_15_IN_1:
							case CRC_20_IN_1:
							{
								static const char names[8][9] =
								{
									"15-in-1",
									"80-in-1",
									"160-in-1",
									"20-in-1",
									"99-in-1",
									"210-in-1",
									"25-in-1",
									"260-in-1"
								};

								return names[i];
							}

							case CRC_18_IN_1:
							{
								static const char names[8][9] =
								{
									"18-in-1",
									"58-in-1",
									"160-in-1",
									"15-in-1",
									"52-in-1",
									"180-in-1",
									"30-in-1",
									"288-in-1"
								};

								return names[i];
							}
						}

						return NULL;
					}
				};

				Fk23c::Fk23c(const Context& c)
				: Mmc3(c), cartSwitches(CartSwitches::Create(c)) {}

				Fk23c::~Fk23c()
				{
					delete cartSwitches;
				}

				void Fk23c::SubReset(const bool hard)
				{
					for (uint i=0; i < 8; ++i)
						exRegs[i] = 0xFF;

					if (prg.Source().Size() <= SIZE_512K)
					{
						for (uint i=0; i < 4; ++i)
							exRegs[i] = 0x00;
					}

					unromChr = 0x0;

					Mmc3::SubReset( hard );

					Map( 0x5000U, 0x5FFFU, &Fk23c::Poke_5000 );
					Map( 0x8000U, 0xFFFFU, &Fk23c::Poke_8000 );

					UpdatePrg();
					UpdateChr();
				}

				Fk23c::Device Fk23c::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void Fk23c::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','F','K'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<9> data( state );

								for (uint i=0; i < 8; ++i)
									exRegs[i] = data[i];

								unromChr = data[8] & 0x3;

								if (cartSwitches)
									cartSwitches->SetMode( data[8] >> 2 & 0x7 );
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Fk23c::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[] =
					{
						exRegs[0],
						exRegs[1],
						exRegs[2],
						exRegs[3],
						exRegs[4],
						exRegs[5],
						exRegs[6],
						exRegs[7],
						unromChr | (cartSwitches ? cartSwitches->GetMode() << 2 : 0)
					};

					state.Begin( AsciiId<'B','F','K'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL Fk23c::UpdatePrg(uint address,uint bank)
				{
					if ((exRegs[0] & 0x7U) - 3 > 1 && (!(exRegs[3] & 0x2U) || address < 0x4000))
					{
						if (exRegs[0] & 0x3U)
							bank = (bank & (0x3FU >> (exRegs[0] & 0x3U))) | (exRegs[1] << 1);

						prg.SwapBank<SIZE_8K>( address, bank );
					}
				}

				void NST_FASTCALL Fk23c::UpdateChr(uint address,uint bank) const
				{
					if (!(exRegs[0] & 0x40U) && (!(exRegs[3] & 0x2U) || (address != 0x400 && address != 0xC00)))
						chr.SwapBank<SIZE_1K>( address, (exRegs[2] & 0x7FU) << 3 | bank );
				}

				void Fk23c::UpdatePrg()
				{
					if ((exRegs[0] & 0x7U) == 4)
					{
						prg.SwapBank<SIZE_32K,0x0000>( exRegs[1] >> 1 );
					}
					else if ((exRegs[0] & 0x7U) == 3)
					{
						prg.SwapBanks<SIZE_16K,0x0000>( exRegs[1], exRegs[1] );
					}
					else
					{
						if (exRegs[3] & 0x2U)
							prg.SwapBanks<SIZE_8K,0x4000>( exRegs[4], exRegs[5] );

						Mmc3::UpdatePrg();
					}
				}

				void Fk23c::UpdateChr() const
				{
					ppu.Update();

					if (exRegs[0] & 0x40U)
					{
						chr.SwapBank<SIZE_8K,0x0000>( exRegs[2] | unromChr );
					}
					else
					{
						if (exRegs[3] & 0x2U)
						{
							const uint base = (exRegs[2] & 0x7FU) << 3;

							chr.SwapBank<SIZE_1K,0x0400>( base | exRegs[6] );
							chr.SwapBank<SIZE_1K,0x0C00>( base | exRegs[7] );
						}

						Mmc3::UpdateChr();
					}
				}

				NES_POKE_AD(Fk23c,5000)
				{
					if (address & (1U << ((cartSwitches ? cartSwitches->GetMode() : 0) + 4)))
					{
						exRegs[address & 0x3] = data;

						Fk23c::UpdatePrg();
						Fk23c::UpdateChr();
					}
				}

				NES_POKE_AD(Fk23c,8000)
				{
					if (exRegs[0] & 0x40U)
					{
						unromChr = (exRegs[0] & 0x30U) ? 0x0 : data & 0x3;

						Fk23c::UpdateChr();
					}
					else switch (address & 0xE001)
					{
						case 0x8000: Mmc3::NES_DO_POKE(8000,address,data); break;
						case 0x8001:

							if (exRegs[3] << 2 & (regs.ctrl0 & 0x8))
							{
								exRegs[4 | regs.ctrl0 & 0x3] = data;

								Fk23c::UpdatePrg();
								Fk23c::UpdateChr();
							}
							else
							{
								Mmc3::NES_DO_POKE(8001,address,data);
							}
							break;

						case 0xA000: SetMirroringHV(data); break;
						case 0xA001: Mmc3::NES_DO_POKE(A001,address,data); break;
						case 0xC000: Mmc3::NES_DO_POKE(C000,address,data); break;
						case 0xC001: Mmc3::NES_DO_POKE(C001,address,data); break;
						case 0xE000: Mmc3::NES_DO_POKE(E000,address,data); break;
						case 0xE001: Mmc3::NES_DO_POKE(E001,address,data); break;

						default: NST_UNREACHABLE();
					}
				}
			}
		}
	}
}
