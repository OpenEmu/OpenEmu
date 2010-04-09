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
#include "NstBoard.hpp"
#include "NstBoardWaixing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Waixing
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void TypeA::SubReset(const bool hard)
				{
					TypeI::SubReset( hard );

					for (uint i=0x0000; i < 0x2000; i += 0x2)
					{
						Map( 0xA000 + i, NMT_SWAP_VH01 );
						Map( 0xA001 + i, NOP_POKE      );
					}
				}

				void TypeF::SubReset(const bool hard)
				{
					exPrg[0] = 0x00;
					exPrg[1] = 0x01;
					exPrg[2] = 0x4E;
					exPrg[3] = 0x4F;

					TypeA::SubReset( hard );

					for (uint i=0x0000; i < 0x2000; i += 0x2)
						Map( 0x8001 + i, &TypeF::Poke_8001 );
				}

				void TypeG::SubReset(const bool hard)
				{
					exPrg[0] = 0x00;
					exPrg[1] = 0x01;
					exPrg[2] = 0x3E;
					exPrg[3] = 0x3F;

					for (uint i=0; i < 8; ++i)
						exChr[0] = i;

					TypeA::SubReset( hard );

					for (uint i=0x0000; i < 0x2000; i += 0x2)
						Map( 0x8001 + i, &TypeG::Poke_8001 );
				}

				void TypeH::SubReset(const bool hard)
				{
					exPrg = 0;

					Mmc3::SubReset( hard );

					wrk.Source().SetSecurity( true, true );

					for (uint i=0x0000; i < 0x2000; i += 0x2)
					{
						Map( 0x8001 + i, &TypeH::Poke_8001 );
						Map( 0xA001 + i, NOP_POKE          );
					}
				}

				void TypeI::SubReset(const bool hard)
				{
					Mmc3::SubReset( hard );

					wrk.Source().SetSecurity( true, true );

					if (board.GetWram() >= SIZE_8K+SIZE_1K)
						Map( 0x5000U, 0x5000U + NST_MIN(board.GetWram(),SIZE_4K) - 1, &TypeI::Peek_5000, &TypeI::Poke_5000 );
				}

				void TypeJ::SubReset(const bool hard)
				{
					if (hard)
					{
						exPrg[0] = 0x01;
						exPrg[1] = 0x02;
						exPrg[2] = 0x7E;
						exPrg[3] = 0x7F;
					}

					TypeI::SubReset( hard );

					for (uint i=0x8001; i < 0xA000; i += 0x2)
						Map( i, &TypeJ::Poke_8001 );
				}

				void TypeF::SubLoad(State::Loader& state,const dword baseChunk)
				{
					TypeA::SubLoad( state, baseChunk );

					for (uint i=0x0000; i < 0x8000; i += 0x2000)
						exPrg[GetPrgIndex(i)] = prg.GetBank<SIZE_8K>(i);
				}

				void TypeG::SubLoad(State::Loader& state,const dword baseChunk)
				{
					TypeA::SubLoad( state, baseChunk );

					for (uint i=0x0000; i < 0x8000; i += 0x2000)
						exPrg[GetPrgIndex(i)] = prg.GetBank<SIZE_8K>(i);

					for (uint i=0x0000; i < 0x2000; i += 0x0400)
						exChr[GetChrIndex(i)] = chr.GetBank<SIZE_1K>(i);
				}

				void TypeH::SubLoad(State::Loader& state,const dword baseChunk)
				{
					Mmc3::SubLoad( state, baseChunk );
					exPrg = prg.GetBank<SIZE_8K,0x0000>() | 0x40;
				}

				void TypeJ::SubLoad(State::Loader& state,const dword baseChunk)
				{
					TypeI::SubLoad( state, baseChunk );

					for (uint i=0x0000; i < 0x8000; i += 0x2000)
						exPrg[GetPrgIndex(i)] = prg.GetBank<SIZE_8K>(i);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(TypeI,5000)
				{
					return wrk.Source()[address-(0x5000-0x2000)];
				}

				NES_POKE_AD(TypeI,5000)
				{
					wrk.Source()[address-(0x5000-0x2000)] = data;
				}

				NES_POKE_AD(TypeF,8001)
				{
					if ((regs.ctrl0 & 7) >= 6)
						exPrg[(regs.ctrl0 & 7) - 6] = data & (data >= 0x40 ? 0x4F : 0x3F);

					Mmc3::NES_DO_POKE(8001,address,data);
				}

				NES_POKE_D(TypeG,8001)
				{
					switch (regs.ctrl0 & 0xF)
					{
						case 0x0: exChr[0] = data; Mmc3::UpdateChr(); break;
						case 0x1: exChr[2] = data; Mmc3::UpdateChr(); break;
						case 0x2: exChr[4] = data; Mmc3::UpdateChr(); break;
						case 0x3: exChr[5] = data; Mmc3::UpdateChr(); break;
						case 0x4: exChr[6] = data; Mmc3::UpdateChr(); break;
						case 0x5: exChr[7] = data; Mmc3::UpdateChr(); break;
						case 0x6: exPrg[0] = data; Mmc3::UpdatePrg(); break;
						case 0x7: exPrg[1] = data; Mmc3::UpdatePrg(); break;
						case 0x8: exPrg[2] = data; Mmc3::UpdatePrg(); break;
						case 0x9: exPrg[3] = data; Mmc3::UpdatePrg(); break;
						case 0xA: exChr[1] = data; Mmc3::UpdateChr(); break;
						case 0xB: exChr[3] = data; Mmc3::UpdateChr(); break;
					}
				}

				NES_POKE_AD(TypeH,8001)
				{
					if (!(regs.ctrl0 & 0x7))
					{
						uint reg = data << 5 & 0x40;

						if (exPrg != reg)
						{
							exPrg = reg;
							Mmc3::UpdatePrg();
						}
					}

					Mmc3::NES_DO_POKE(8001,address,data);
				}

				NES_POKE_AD(TypeJ,8001)
				{
					const uint index = (regs.ctrl0 & 0x7);

					if (index >= 6 && exPrg[index-6] != data)
					{
						exPrg[index-6] = data;
						Mmc3::UpdatePrg();
					}

					Mmc3::NES_DO_POKE(8001,address,data);
				}

				uint TypeI::GetPrgIndex(uint address) const
				{
					return address >> 13 ^ (regs.ctrl0 >> 5 & ~address >> 12 & 0x2);
				}

				uint TypeI::GetChrIndex(uint address) const
				{
					return address >> 10 ^ (regs.ctrl0 >> 5 & 0x4);
				}

				void NST_FASTCALL TypeF::UpdatePrg(uint address,uint)
				{
					prg.SwapBank<SIZE_8K>( address, exPrg[GetPrgIndex(address)] );
				}

				void NST_FASTCALL TypeG::UpdatePrg(uint address,uint)
				{
					prg.SwapBank<SIZE_8K>( address, exPrg[GetPrgIndex(address)] );
				}

				void NST_FASTCALL TypeH::UpdatePrg(uint address,uint bank)
				{
					prg.SwapBank<SIZE_8K>( address, exPrg | bank );
				}

				void NST_FASTCALL TypeA::UpdateChr(uint address,uint bank) const
				{
					chr.Source( GetChrSource(bank) ).SwapBank<SIZE_1K>( address, bank );
				}

				void NST_FASTCALL TypeG::UpdateChr(uint address,uint) const
				{
					uint bank = exChr[GetChrIndex(address)];
					chr.Source( bank <= 7 ).SwapBank<SIZE_1K>( address, bank );
				}

				void NST_FASTCALL TypeH::UpdateChr(uint address,uint bank) const
				{
					if (chr.Source().GetType() == Ram::ROM)
						chr.SwapBank<SIZE_1K>( address, bank );
				}

				void NST_FASTCALL TypeJ::UpdatePrg(uint address,uint bank)
				{
					prg.SwapBank<SIZE_8K>( address, exPrg[GetPrgIndex(address)] );
				}

				uint NST_FASTCALL TypeA::GetChrSource(uint bank) const
				{
					return bank - 8 <= 1;
				}

				uint NST_FASTCALL TypeB::GetChrSource(uint bank) const
				{
					return bank >> 7 & 0x1;
				}

				uint NST_FASTCALL TypeC::GetChrSource(uint bank) const
				{
					return bank - 8 <= 3;
				}

				uint NST_FASTCALL TypeD::GetChrSource(uint bank) const
				{
					return bank <= 1;
				}

				uint NST_FASTCALL TypeE::GetChrSource(uint bank) const
				{
					return bank <= 3;
				}

				uint NST_FASTCALL TypeF::GetChrSource(uint) const
				{
					return 0;
				}
			}
		}
	}
}
