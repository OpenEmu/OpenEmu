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

#include <cstdio>
#include <algorithm>
#include "../NstLog.hpp"
#include "../NstClock.hpp"
#include "NstBoard.hpp"
#include "NstBoardNRom.hpp"
#include "NstBoardAxRom.hpp"
#include "NstBoardBxRom.hpp"
#include "NstBoardCxRom.hpp"
#include "NstBoardDxRom.hpp"
#include "NstBoardExRom.hpp"
#include "NstBoardFxRom.hpp"
#include "NstBoardGxRom.hpp"
#include "NstBoardHxRom.hpp"
#include "NstBoardJxRom.hpp"
#include "NstBoardMxRom.hpp"
#include "NstBoardNxRom.hpp"
#include "NstBoardPxRom.hpp"
#include "NstBoardSxRom.hpp"
#include "NstBoardTxRom.hpp"
#include "NstBoardUxRom.hpp"
#include "NstBoardDiscrete.hpp"
#include "NstBoardEvent.hpp"
#include "NstBoardFb.hpp"
#include "NstBoardQj.hpp"
#include "NstBoardZz.hpp"
#include "NstBoardAe.hpp"
#include "NstBoardAgci.hpp"
#include "NstBoardAve.hpp"
#include "NstBoardBandai.hpp"
#include "NstBoardCaltron.hpp"
#include "NstBoardCamerica.hpp"
#include "NstBoardCne.hpp"
#include "NstBoardCony.hpp"
#include "NstBoardDreamtech.hpp"
#include "NstBoardFfe.hpp"
#include "NstBoardFujiya.hpp"
#include "NstBoardFukutake.hpp"
#include "NstBoardFutureMedia.hpp"
#include "NstBoardGouder.hpp"
#include "NstBoardHes.hpp"
#include "NstBoardHosenkan.hpp"
#include "NstBoardIrem.hpp"
#include "NstBoardJaleco.hpp"
#include "NstBoardJyCompany.hpp"
#include "NstBoardKaiser.hpp"
#include "NstBoardKasing.hpp"
#include "NstBoardKay.hpp"
#include "NstBoardKonami.hpp"
#include "NstBoardMagicSeries.hpp"
#include "NstBoardNanjing.hpp"
#include "NstBoardNihon.hpp"
#include "NstBoardNitra.hpp"
#include "NstBoardNtdec.hpp"
#include "NstBoardOpenCorp.hpp"
#include "NstBoardRcm.hpp"
#include "NstBoardRexSoft.hpp"
#include "NstBoardRumbleStation.hpp"
#include "NstBoardSachen.hpp"
#include "NstBoardSomeriTeam.hpp"
#include "NstBoardSubor.hpp"
#include "NstBoardSunsoft.hpp"
#include "NstBoardSuperGame.hpp"
#include "NstBoardTaito.hpp"
#include "NstBoardTengen.hpp"
#include "NstBoardTxc.hpp"
#include "NstBoardVsSystem.hpp"
#include "NstBoardWaixing.hpp"
#include "NstBoardWhirlwind.hpp"
#include "NstBoardBenshengBs5.hpp"
#include "NstBoardUnlCc21.hpp"
#include "NstBoardUnlEdu2000.hpp"
#include "NstBoardUnlKingOfFighters96.hpp"
#include "NstBoardUnlKingOfFighters97.hpp"
#include "NstBoardUnlMortalKombat2.hpp"
#include "NstBoardUnlSuperFighter3.hpp"
#include "NstBoardUnlTf1201.hpp"
#include "NstBoardUnlWorldHero.hpp"
#include "NstBoardUnlXzy.hpp"
#include "NstBoardBtl2708.hpp"
#include "NstBoardBtlAx5705.hpp"
#include "NstBoardBtl6035052.hpp"
#include "NstBoardBtlDragonNinja.hpp"
#include "NstBoardBtlGeniusMerioBros.hpp"
#include "NstBoardBtlMarioBaby.hpp"
#include "NstBoardBtlPikachuY2k.hpp"
#include "NstBoardBtlShuiGuanPipe.hpp"
#include "NstBoardBtlSmb2a.hpp"
#include "NstBoardBtlSmb2b.hpp"
#include "NstBoardBtlSmb2c.hpp"
#include "NstBoardBtlSmb3.hpp"
#include "NstBoardBtlSuperBros11.hpp"
#include "NstBoardBtlT230.hpp"
#include "NstBoardBtlTobidaseDaisakusen.hpp"
#include "NstBoardBmc110in1.hpp"
#include "NstBoardBmc1200in1.hpp"
#include "NstBoardBmc150in1.hpp"
#include "NstBoardBmc15in1.hpp"
#include "NstBoardBmc20in1.hpp"
#include "NstBoardBmc21in1.hpp"
#include "NstBoardBmc22Games.hpp"
#include "NstBoardBmc31in1.hpp"
#include "NstBoardBmc35in1.hpp"
#include "NstBoardBmc36in1.hpp"
#include "NstBoardBmc64in1.hpp"
#include "NstBoardBmc72in1.hpp"
#include "NstBoardBmc76in1.hpp"
#include "NstBoardBmc800in1.hpp"
#include "NstBoardBmc8157.hpp"
#include "NstBoardBmc9999999in1.hpp"
#include "NstBoardBmcA65as.hpp"
#include "NstBoardBmcBallgames11in1.hpp"
#include "NstBoardBmcCtc65.hpp"
#include "NstBoardBmcFamily4646B.hpp"
#include "NstBoardBmcFk23c.hpp"
#include "NstBoardBmcGamestarA.hpp"
#include "NstBoardBmcGamestarB.hpp"
#include "NstBoardBmcGolden190in1.hpp"
#include "NstBoardBmcGoldenCard6in1.hpp"
#include "NstBoardBmcGoldenGame260in1.hpp"
#include "NstBoardBmcHero.hpp"
#include "NstBoardBmcMarioParty7in1.hpp"
#include "NstBoardBmcNovelDiamond.hpp"
#include "NstBoardBmcCh001.hpp"
#include "NstBoardBmcPowerjoy84in1.hpp"
#include "NstBoardBmcResetBased4in1.hpp"
#include "NstBoardBmcSuper22Games.hpp"
#include "NstBoardBmcSuper24in1.hpp"
#include "NstBoardBmcSuper40in1.hpp"
#include "NstBoardBmcSuper700in1.hpp"
#include "NstBoardBmcSuperBig7in1.hpp"
#include "NstBoardBmcSuperGun20in1.hpp"
#include "NstBoardBmcSuperHiK4in1.hpp"
#include "NstBoardBmcSuperHiK300in1.hpp"
#include "NstBoardBmcSuperVision16in1.hpp"
#include "NstBoardBmcT262.hpp"
#include "NstBoardBmcVrc4.hpp"
#include "NstBoardBmcVt5201.hpp"
#include "NstBoardBmcY2k64in1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Board::Type::Type()
			:
			id       (UNKNOWN),
			nmt      (NMT_VERTICAL),
			chrRam   (0),
			battery  (false),
			wramAuto (false)
			{
			}

			Board::Type::Type(Id i,Ram& prgRom,Ram& chrRom,Nmt n,bool b,bool a)
			: id(i), battery(b)
			{
				wramAuto = (a && GetWram() >= SIZE_8K);

				const dword oldPrg = prgRom.Size();

				prgRom.Set( Ram::ROM, true, false, NST_MIN(oldPrg,GetMaxPrg()) );
				prgRom.Mirror( SIZE_16K );

				if (prgRom.Size() != oldPrg)
				{
					NST_DEBUG_MSG("PRG-ROM truncated!");
					Log::Flush( "Board: warning, PRG-ROM truncated" NST_LINEBREAK );
				}

				switch (dword(id) >> 7 & 0x7)
				{
					default:     chrRam =  0; break;
					case CRM_1:  chrRam =  1; break;
					case CRM_2:  chrRam =  2; break;
					case CRM_4:  chrRam =  4; break;
					case CRM_6:  chrRam =  6; break;
					case CRM_8:  chrRam =  8; break;
					case CRM_16: chrRam = 16; break;
					case CRM_32: chrRam = 32; break;
				}

				if (chrRam < 8 && chrRom.Empty())
					chrRam = 8;

				const dword oldChr = chrRom.Size();

				chrRom.Set( Ram::ROM, true, false, NST_MIN(oldChr,GetMaxChr()) );

				if (chrRom.Size())
					chrRom.Mirror( SIZE_8K );

				if (chrRom.Size() != oldChr)
				{
					NST_DEBUG_MSG("CHR-ROM truncated!");
					Log::Flush( "Board: warning, CHR-ROM truncated" NST_LINEBREAK );
				}

				switch (dword(i) >> 4 & 0x7)
				{
					case NMT_H:
					case NMT_V:
					case NMT_Z: nmt = NMT_CONTROLLED; break;
					case NMT_1: nmt = NMT_SINGLESCREEN; break;
					case NMT_2:
					case NMT_4: nmt = NMT_FOURSCREEN; break;
					default:    nmt = (n == NMT_CONTROLLED ? NMT_VERTICAL : n); break;
				}
			}

			dword Board::Type::GetMaxPrg() const
			{
				return 0x2000UL << (dword(id) >> 20 & 0xF);
			}

			dword Board::Type::GetMaxChr() const
			{
				uint v = dword(id) >> 16 & 0xF;
				return v ? 0x1000UL << v : 0UL;
			}

			uint Board::Type::GetSavableWram() const
			{
				uint v = dword(id) >> 13 & 0x7;
				return v ? 0x200U << v : 0UL;
			}

			uint Board::Type::GetNonSavableWram() const
			{
				uint v = dword(id) >> 10 & 0x7;
				return v ? 0x200U << v : 0UL;
			}

			uint Board::Type::GetWram() const
			{
				return GetSavableWram() + GetNonSavableWram();
			}

			uint Board::Type::GetChrRam() const
			{
				return chrRam * SIZE_1K;
			}

			Board::Type::Nmt Board::Type::GetStartupNmt() const
			{
				switch (static_cast<NmtInit>(dword(id) >> 4 & 0x7))
				{
					case NMT_H: return NMT_HORIZONTAL;
					case NMT_V: return NMT_VERTICAL;
					case NMT_Z:
					case NMT_1: return NMT_SINGLESCREEN;
					case NMT_2:
					case NMT_4: return NMT_FOURSCREEN;
					default:    return GetNmt();
				}
			}

			uint Board::Type::GetNmtRam() const
			{
				if (nmt == NMT_FOURSCREEN)
					return (dword(id) >> 4 & 0x7) == NMT_2 ? SIZE_2K : SIZE_4K;
				else
					return 0;
			}

			uint Board::Type::GetSavableVram() const
			{
				return 0;
			}

			uint Board::Type::GetNonSavableVram() const
			{
				return GetChrRam() + GetNmtRam();
			}

			uint Board::Type::GetVram() const
			{
				return GetSavableVram() + GetNonSavableVram();
			}

			Board::Board(const Context& context)
			:
			cpu   (*context.cpu),
			ppu   (*context.ppu),
			chr   (context.ppu->GetChrMem()),
			nmt   (context.ppu->GetNmtMem()),
			vram  (Ram::RAM,true,true,context.type.GetVram()),
			board (context.type)
			{
				prg.Source(0).Set( context.prg );

				if (const uint size = board.GetWram())
				{
					wrk.Source(0).Set( board.GetSavableWram() ? Ram::NVRAM : Ram::RAM, true, true, size );
					wrk.Source(0).Fill( 0x00 );
				}
				else
				{
					wrk.Source(0).Set( context.prg );
				}

				prg.Source(1).Set( wrk.Source(0).Reference() );
				wrk.Source(1).Set( prg.Source(0).Reference() );

				if (const uint size = board.GetChrRam())
					chr.Source(1).Set( Ram::RAM, true, true, size, vram.Mem() );
				else
					chr.Source(1).Set( context.chr );

				if (context.chr.Size())
					chr.Source(0).Set( context.chr );
				else
					chr.Source(0).Set( chr.Source(1).Reference() );

				if (const uint size = board.GetNmtRam())
					nmt.Source(1).Set( Ram::RAM, true, true, size, vram.Mem() + board.GetChrRam() );
				else
					nmt.Source(1).Set( chr.Source().Reference() );

				vram.Fill( 0x00 );

				if (Log::Available())
				{
					Log log;

					log << "Board: " << context.name << NST_LINEBREAK;
					log << "Board: " << (context.prg.Size() / SIZE_1K) << "k PRG-ROM" NST_LINEBREAK;

					if (context.chr.Size())
						log << "Board: " << (context.chr.Size() / SIZE_1K) << "k CHR-ROM" NST_LINEBREAK;

					if (const uint size = board.GetWram())
						log << "Board: " << (size / SIZE_1K) << (board.IsAutoWram() ? "k auto W-RAM" NST_LINEBREAK : "k W-RAM" NST_LINEBREAK);

					if (const uint size = board.GetVram())
						log << "Board: " << (size / SIZE_1K) << "k V-RAM" NST_LINEBREAK;
				}
			}

			void Board::Reset(const bool hard)
			{
				cpu.Map( 0x4018, 0x5FFF ).Set( this, &Board::Peek_Nop, &Board::Poke_Nop );

				if (board.GetWram() >= SIZE_8K)
					cpu.Map( 0x6000, 0x7FFF ).Set( this, &Board::Peek_Wram_6, &Board::Poke_Wram_6 );
				else
					cpu.Map( 0x6000, 0x7FFF ).Set( this, &Board::Peek_Nop, &Board::Poke_Nop );

				cpu.Map( 0x8000, 0x9FFF ).Set( this, &Board::Peek_Prg_8, &Board::Poke_Nop );
				cpu.Map( 0xA000, 0xBFFF ).Set( this, &Board::Peek_Prg_A, &Board::Poke_Nop );
				cpu.Map( 0xC000, 0xDFFF ).Set( this, &Board::Peek_Prg_C, &Board::Poke_Nop );
				cpu.Map( 0xE000, 0xFFFF ).Set( this, &Board::Peek_Prg_E, &Board::Poke_Nop );

				if (hard)
				{
					wrk.Source().SetSecurity( true, board.GetWram() > 0 );

					for (uint i=board.GetSavableWram(), n=board.GetWram(); i < n; ++i)
						*wrk.Source().Mem(i) = (board.IsAutoWram() && i < SIZE_8K) ? (0x6000 + i) >> 8 : 0x00;

					vram.Fill( 0x00 );

					prg.SwapBanks<SIZE_16K,0x0000>(0U,~0U);
					chr.SwapBank<SIZE_8K,0x0000>(0);
					wrk.SwapBank<SIZE_8K,0x0000>(0);

					switch (board.GetStartupNmt())
					{
						case Type::NMT_HORIZONTAL:

							ppu.SetMirroring( Ppu::NMT_H );
							break;

						case Type::NMT_VERTICAL:

							ppu.SetMirroring( Ppu::NMT_V );
							break;

						case Type::NMT_SINGLESCREEN:

							ppu.SetMirroring( Ppu::NMT_0 );
							break;

						case Type::NMT_FOURSCREEN:

							if (board.GetNmtRam() == SIZE_2K)
							{
								nmt.Source(0).SwapBank<SIZE_2K,0x0000>(0);
								nmt.Source(1).SwapBank<SIZE_2K,0x0800>(0);
							}
							else
							{
								nmt.Source(1).SwapBank<SIZE_4K,0x0000>(0);
							}
							break;
					}
				}

				SubReset( hard );
			}

			void Board::Save(File& file) const
			{
				if (board.HasBattery() && board.GetSavableWram())
					file.Save( File::BATTERY, wrk.Source().Mem(), board.GetSavableWram() );
			}

			void Board::Load(File& file)
			{
				if (board.HasBattery() && board.GetSavableWram())
					file.Load( File::BATTERY, wrk.Source().Mem(), board.GetSavableWram() );
			}

			void Board::SaveState(State::Saver& state,const dword baseChunk) const
			{
				state.Begin( baseChunk );

				if (const uint size = board.GetWram())
					state.Begin( AsciiId<'W','R','M'>::V ).Compress( wrk.Source().Mem(), size ).End();

				if (const uint size = board.GetVram())
					state.Begin( AsciiId<'V','R','M'>::V ).Compress( vram.Mem(), size ).End();

				prg.SaveState( state, AsciiId<'P','R','G'>::V );
				chr.SaveState( state, AsciiId<'C','H','R'>::V );
				nmt.SaveState( state, AsciiId<'N','M','T'>::V );
				wrk.SaveState( state, AsciiId<'W','R','K'>::V );

				SubSave( state );

				state.End();
			}

			void Board::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'W','R','M'>::V:

							NST_VERIFY( board.GetWram() );

							if (const uint size = board.GetWram())
								state.Uncompress( wrk.Source().Mem(), size );

							break;

						case AsciiId<'V','R','M'>::V:

							NST_VERIFY( board.GetVram() );

							if (const uint size = board.GetVram())
								state.Uncompress( vram.Mem(), size );

							break;

						case AsciiId<'P','R','G'>::V:

							prg.LoadState( state );
							break;

						case AsciiId<'C','H','R'>::V:

							chr.LoadState( state );
							break;

						case AsciiId<'N','M','T'>::V:

							nmt.LoadState( state );
							break;

						case AsciiId<'W','R','K'>::V:

							wrk.LoadState( state );
							break;

						default:

							SubLoad( state, chunk );
							break;
					}

					state.End();
				}
			}

			void Board::Map( uint a,uint b,PrgSwap8k0  ) const { cpu.Map(a,b).Set( &Board::Poke_Prg_8k_0  ); }
			void Board::Map( uint a,uint b,PrgSwap8k1  ) const { cpu.Map(a,b).Set( &Board::Poke_Prg_8k_1  ); }
			void Board::Map( uint a,uint b,PrgSwap8k2  ) const { cpu.Map(a,b).Set( &Board::Poke_Prg_8k_2  ); }
			void Board::Map( uint a,uint b,PrgSwap8k3  ) const { cpu.Map(a,b).Set( &Board::Poke_Prg_8k_3  ); }
			void Board::Map( uint a,uint b,PrgSwap16k0 ) const { cpu.Map(a,b).Set( &Board::Poke_Prg_16k_0 ); }
			void Board::Map( uint a,uint b,PrgSwap16k1 ) const { cpu.Map(a,b).Set( &Board::Poke_Prg_16k_1 ); }
			void Board::Map( uint a,uint b,PrgSwap32k  ) const { cpu.Map(a,b).Set( &Board::Poke_Prg_32k   ); }
			void Board::Map( uint a,uint b,ChrSwap1k0  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_1k_0  ); }
			void Board::Map( uint a,uint b,ChrSwap1k1  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_1k_1  ); }
			void Board::Map( uint a,uint b,ChrSwap1k2  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_1k_2  ); }
			void Board::Map( uint a,uint b,ChrSwap1k3  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_1k_3  ); }
			void Board::Map( uint a,uint b,ChrSwap1k4  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_1k_4  ); }
			void Board::Map( uint a,uint b,ChrSwap1k5  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_1k_5  ); }
			void Board::Map( uint a,uint b,ChrSwap1k6  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_1k_6  ); }
			void Board::Map( uint a,uint b,ChrSwap1k7  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_1k_7  ); }
			void Board::Map( uint a,uint b,ChrSwap2k0  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_2k_0  ); }
			void Board::Map( uint a,uint b,ChrSwap2k1  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_2k_1  ); }
			void Board::Map( uint a,uint b,ChrSwap2k2  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_2k_2  ); }
			void Board::Map( uint a,uint b,ChrSwap2k3  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_2k_3  ); }
			void Board::Map( uint a,uint b,ChrSwap4k0  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_4k_0  ); }
			void Board::Map( uint a,uint b,ChrSwap4k1  ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_4k_1  ); }
			void Board::Map( uint a,uint b,ChrSwap8k   ) const { cpu.Map(a,b).Set( &Board::Poke_Chr_8k    ); }
			void Board::Map( uint a,uint b,NmtSwapHv   ) const { cpu.Map(a,b).Set( &Board::Poke_Nmt_Hv    ); }
			void Board::Map( uint a,uint b,NmtSwapVh   ) const { cpu.Map(a,b).Set( &Board::Poke_Nmt_Vh    ); }
			void Board::Map( uint a,uint b,NmtSwapVh01 ) const { cpu.Map(a,b).Set( &Board::Poke_Nmt_Vh01  ); }
			void Board::Map( uint a,uint b,NmtSwapHv01 ) const { cpu.Map(a,b).Set( &Board::Poke_Nmt_Hv01  ); }
			void Board::Map( uint a,uint b,NopPeek     ) const { cpu.Map(a,b).Set( &Board::Peek_Nop       ); }
			void Board::Map( uint a,uint b,NopPoke     ) const { cpu.Map(a,b).Set( &Board::Poke_Nop       ); }

			void Board::Map( PrgSwap8k0Bc  ) const { cpu.Map(0x8000,0xFFFF).Set( &Board::Poke_Prg_8k_0_bc  ); }
			void Board::Map( PrgSwap16k0Bc ) const { cpu.Map(0x8000,0xFFFF).Set( &Board::Poke_Prg_16k_0_bc ); }
			void Board::Map( PrgSwap32kBc  ) const { cpu.Map(0x8000,0xFFFF).Set( &Board::Poke_Prg_32k_bc   ); }
			void Board::Map( ChrSwap4k1Bc  ) const { cpu.Map(0x8000,0xFFFF).Set( &Board::Poke_Chr_4k_1_bc  ); }
			void Board::Map( ChrSwap8kBc   ) const { cpu.Map(0x8000,0xFFFF).Set( &Board::Poke_Chr_8k_bc    ); }

			void Board::Map(uint a,uint b,NopPeekPoke) const
			{
				cpu.Map(a,b).Set( &Board::Peek_Nop, &Board::Poke_Nop );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_PEEK_A(Board,Prg_8) { return prg[0][address - 0x8000]; }
			NES_PEEK_A(Board,Prg_A) { return prg[1][address - 0xA000]; }
			NES_PEEK_A(Board,Prg_C) { return prg[2][address - 0xC000]; }
			NES_PEEK_A(Board,Prg_E) { return prg[3][address - 0xE000]; }

			NES_POKE_D(Board,Prg_8k_0)  { prg.SwapBank<SIZE_8K,0x0000>( data ); }
			NES_POKE_D(Board,Prg_8k_1)  { prg.SwapBank<SIZE_8K,0x2000>( data ); }
			NES_POKE_D(Board,Prg_8k_2)  { prg.SwapBank<SIZE_8K,0x4000>( data ); }
			NES_POKE_D(Board,Prg_8k_3)  { prg.SwapBank<SIZE_8K,0x6000>( data ); }
			NES_POKE_D(Board,Prg_16k_0) { prg.SwapBank<SIZE_16K,0x0000>( data ); }
			NES_POKE_D(Board,Prg_16k_1) { prg.SwapBank<SIZE_16K,0x4000>( data ); }
			NES_POKE_D(Board,Prg_32k)   { prg.SwapBank<SIZE_32K,0x0000>( data ); }

			NES_POKE_D(Board,Chr_1k_0) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0000>( data ); }
			NES_POKE_D(Board,Chr_1k_1) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0400>( data ); }
			NES_POKE_D(Board,Chr_1k_2) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0800>( data ); }
			NES_POKE_D(Board,Chr_1k_3) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0C00>( data ); }
			NES_POKE_D(Board,Chr_1k_4) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1000>( data ); }
			NES_POKE_D(Board,Chr_1k_5) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1400>( data ); }
			NES_POKE_D(Board,Chr_1k_6) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1800>( data ); }
			NES_POKE_D(Board,Chr_1k_7) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1C00>( data ); }
			NES_POKE_D(Board,Chr_2k_0) { ppu.Update(); chr.SwapBank<SIZE_2K,0x0000>( data ); }
			NES_POKE_D(Board,Chr_2k_1) { ppu.Update(); chr.SwapBank<SIZE_2K,0x0800>( data ); }
			NES_POKE_D(Board,Chr_2k_2) { ppu.Update(); chr.SwapBank<SIZE_2K,0x1000>( data ); }
			NES_POKE_D(Board,Chr_2k_3) { ppu.Update(); chr.SwapBank<SIZE_2K,0x1800>( data ); }
			NES_POKE_D(Board,Chr_4k_0) { ppu.Update(); chr.SwapBank<SIZE_4K,0x0000>( data ); }
			NES_POKE_D(Board,Chr_4k_1) { ppu.Update(); chr.SwapBank<SIZE_4K,0x1000>( data ); }
			NES_POKE_D(Board,Chr_8k)   { ppu.Update(); chr.SwapBank<SIZE_8K,0x0000>( data ); }

			uint Board::GetBusData(uint address,uint data) const
			{
				NST_VERIFY( data == prg.Peek(address & 0x7FFF) );
				return data & prg.Peek(address & 0x7FFF);
			}

			NES_POKE_AD(Board,Prg_8k_0_bc)
			{
				prg.SwapBank<SIZE_8K,0x0000>( GetBusData(address,data) );
			}

			NES_POKE_AD(Board,Prg_16k_0_bc)
			{
				prg.SwapBank<SIZE_16K,0x0000>( GetBusData(address,data) );
			}

			NES_POKE_AD(Board,Prg_32k_bc)
			{
				prg.SwapBank<SIZE_32K,0x0000>( GetBusData(address,data) );
			}

			NES_POKE_AD(Board,Chr_4k_1_bc)
			{
				ppu.Update();
				chr.SwapBank<SIZE_4K,0x1000>( GetBusData(address,data) );
			}

			NES_POKE_AD(Board,Chr_8k_bc)
			{
				ppu.Update();
				chr.SwapBank<SIZE_8K,0x0000>( GetBusData(address,data) );
			}

			NES_POKE_AD(Board,Wram_6)
			{
				NST_VERIFY( wrk.Writable(0) );

				if (wrk.Writable(0))
					wrk[0][address - 0x6000] = data;
			}

			NES_PEEK_A(Board,Wram_6)
			{
				NST_VERIFY( wrk.Readable(0) );
				return wrk.Readable(0) ? wrk[0][address - 0x6000] : (address >> 8);
			}

			NES_POKE_D(Board,Nmt_Hv)
			{
				NST_VERIFY( data <= 0x1 );
				ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_H : Ppu::NMT_V );
			}

			NES_POKE_D(Board,Nmt_Vh)
			{
				NST_VERIFY( data <= 0x1 );
				ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_V : Ppu::NMT_H );
			}

			NES_POKE_D(Board,Nmt_Vh01)
			{
				NST_VERIFY( data <= 0x3 );

				static const byte lut[4][4] =
				{
					{0,1,0,1},
					{0,0,1,1},
					{0,0,0,0},
					{1,1,1,1}
				};

				ppu.SetMirroring( lut[data & 0x3] );
			}

			NES_POKE_D(Board,Nmt_Hv01)
			{
				NST_VERIFY( data <= 0x3 );

				static const byte lut[4][4] =
				{
					{0,0,1,1},
					{0,1,0,1},
					{0,0,0,0},
					{1,1,1,1}
				};

				ppu.SetMirroring( lut[data & 0x3] );
			}

			NES_POKE(Board,Nop)
			{
				NST_DEBUG_MSG("write ignored!");
			}

			NES_PEEK_A(Board,Nop)
			{
				NST_DEBUG_MSG("read ignored!");
				return address >> 8;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Board::Context::Context
			(
				Cpu* c,
				Apu* a,
				Ppu* p,
				Ram& pr,
				Ram& cr,
				const Ram& t,
				Type::Nmt n,
				bool wb,
				bool mb,
				Chips& h
			)
			:
			name        (""),
			cpu         (c),
			apu         (a),
			ppu         (p),
			prg         (pr),
			chr         (cr),
			trainer     (t),
			nmt         (n),
			chips       (h),
			wramBattery (wb),
			mmcBattery  (mb)
			{
			}

			struct Board::Context::Element
			{
				cstring name;
				dword id;

				struct Less
				{
					bool operator () (const Element& a,const Element& b) const
					{
						return Core::StringCompare( a.name, b.name ) < 0;
					}

					bool operator () (const Element& a,wcstring b) const
					{
						return Core::StringCompare( a.name, b ) < 0;
					}

					bool operator () (wcstring a,const Element& b) const
					{
						return Core::StringCompare( b.name, a ) >= 0;
					}
				};
			};

			bool Board::Context::DetectBoard(wcstring string,const dword wram)
			{
				static const Element lut[] =
				{
					{ "ACCLAIM-AOROM",               Type::STD_AOROM                },
					{ "ACCLAIM-MC-ACC",              Type::STD_TLROM                },
					{ "ACCLAIM-TLROM",               Type::STD_TLROM                },
					{ "AGCI-47516",                  Type::DISCRETE_74_377          },
					{ "AGCI-50282",                  Type::AGCI_50282               },
					{ "AVE-74*161",                  Type::UNL_CXROM                },
					{ "AVE-MB-91",                   Type::AVE_MB_91                },
					{ "AVE-NINA-01",                 Type::AVE_NINA001              },
					{ "AVE-NINA-02",                 Type::AVE_NINA002              },
					{ "AVE-NINA-03",                 Type::AVE_NINA03               },
					{ "AVE-NINA-06",                 Type::AVE_NINA06               },
					{ "AVE-NINA-07",                 Type::AVE_NINA07               },
					{ "BANDAI-74*161/161/32",        Type::DISCRETE_74_161_161_32_A },
					{ "BANDAI-CNROM",                Type::STD_CNROM                },
					{ "BANDAI-FCG-1",                Type::BANDAI_FCG1              },
					{ "BANDAI-FCG-2",                Type::BANDAI_FCG2              },
					{ "BANDAI-GNROM",                Type::STD_GNROM                },
					{ "BANDAI-JUMP2",                Type::BANDAI_BAJUMP2           },
					{ "BANDAI-LZ93D50+24C01",        Type::BANDAI_LZ93D50_24C01     },
					{ "BANDAI-LZ93D50+24C02",        Type::BANDAI_LZ93D50_24C02     },
					{ "BANDAI-NROM-128",             Type::STD_NROM                 },
					{ "BANDAI-NROM-256",             Type::STD_NROM                 },
					{ "BANDAI-PT-554",               Type::BANDAI_AEROBICSSTUDIO    },
					{ "BMC-190IN1",                  Type::BMC_GOLDEN_190IN1        },
					{ "BMC-42IN1RESETSWITCH",        Type::BMC_SUPER_22GAMES        },
					{ "BMC-64IN1NOREPEAT",           Type::BMC_Y2K_64IN1            },
					{ "BMC-70IN1",                   Type::BMC_GAME_800IN1          },
					{ "BMC-70IN1B",                  Type::BMC_GAME_800IN1          },
					{ "BMC-8157",                    Type::BMC_8157                 },
					{ "BMC-A65AS",                   Type::BMC_A65AS                },
					{ "BMC-BS-5",                    Type::BENSHENG_BS5             },
					{ "BMC-D1038",                   Type::BMC_VT5201               },
					{ "BMC-FK23C",                   Type::BMC_FKC23C               },
					{ "BMC-GHOSTBUSTERS63IN1",       Type::BMC_CTC65                },
					{ "BMC-GS-2004",                 Type::RCM_GS2004               },
					{ "BMC-GS-2013",                 Type::RCM_GS2013               },
					{ "BMC-NOVELDIAMOND9999999IN1",  Type::BMC_NOVELDIAMOND         },
					{ "BMC-SUPER24IN1SC03",          Type::BMC_SUPER_24IN1          },
					{ "BMC-SUPERHIK8IN1",            Type::BMC_HERO                 },
					{ "BMC-SUPERVISION16IN1",        Type::BMC_SUPERVISION_16IN1    },
					{ "BMC-T-262",                   Type::BMC_T262                 },
					{ "BMC-WS",                      Type::BMC_SUPER_40IN1          },
					{ "BTL-MARIO1-MALEE2",           Type::BTL_GENIUSMERIOBROS      },
					{ "CAMERICA-ALGN",               Type::CAMERICA_ALGNV11         },
					{ "CAMERICA-ALGQ",               Type::CAMERICA_ALGQV11         },
					{ "CAMERICA-BF9093",             Type::CAMERICA_BF9093          },
					{ "CAMERICA-BF9096",             Type::CAMERICA_BF9096          },
					{ "CAMERICA-BF9097",             Type::CAMERICA_BF9097          },
					{ "CAMERICA-GAMEGENIE",          Type::STD_NROM                 },
					{ "COLORDREAMS-74*377",          Type::DISCRETE_74_377          },
					{ "DREAMTECH01",                 Type::DREAMTECH_01             },
					{ "HVC-AMROM",                   Type::STD_AMROM                },
					{ "HVC-AN1ROM",                  Type::STD_AN1ROM               },
					{ "HVC-ANROM",                   Type::STD_ANROM                },
					{ "HVC-AOROM",                   Type::STD_AOROM                },
					{ "HVC-BNROM",                   Type::STD_BNROM                },
					{ "HVC-CNROM",                   Type::STD_CNROM                },
					{ "HVC-CPROM",                   Type::STD_CPROM                },
					{ "HVC-DE1ROM",                  Type::STD_DE1ROM               },
					{ "HVC-DEROM",                   Type::STD_DEROM                },
					{ "HVC-DRROM",                   Type::STD_DRROM                },
					{ "HVC-EKROM",                   Type::STD_EKROM                },
					{ "HVC-ELROM",                   Type::STD_ELROM                },
					{ "HVC-ETROM",                   Type::STD_ETROM                },
					{ "HVC-EWROM",                   Type::STD_EWROM                },
					{ "HVC-FAMILYBASIC",             Type::CUSTOM_FB02              },
					{ "HVC-FJROM",                   Type::STD_FJROM                },
					{ "HVC-FKROM",                   Type::STD_FKROM                },
					{ "HVC-GNROM",                   Type::STD_GNROM                },
					{ "HVC-HKROM",                   Type::STD_HKROM                },
					{ "HVC-HROM",                    Type::STD_NROM                 },
					{ "HVC-JLROM",                   Type::STD_JLROM                },
					{ "HVC-JSROM",                   Type::STD_JSROM                },
					{ "HVC-MHROM",                   Type::STD_MHROM                },
					{ "HVC-NROM",                    Type::STD_NROM                 },
					{ "HVC-NROM-128",                Type::STD_NROM                 },
					{ "HVC-NROM-256",                Type::STD_NROM                 },
					{ "HVC-NTBROM",                  Type::STD_NTBROM               },
					{ "HVC-PEEOROM",                 Type::STD_PEEOROM              },
					{ "HVC-PNROM",                   Type::STD_PNROM                },
					{ "HVC-RROM",                    Type::STD_NROM                 },
					{ "HVC-RROM-128",                Type::STD_NROM                 },
					{ "HVC-SAROM",                   Type::STD_SAROM                },
					{ "HVC-SBROM",                   Type::STD_SBROM                },
					{ "HVC-SC1ROM",                  Type::STD_SCROM                },
					{ "HVC-SCROM",                   Type::STD_SCROM                },
					{ "HVC-SEROM",                   Type::STD_SEROM                },
					{ "HVC-SF1ROM",                  Type::STD_SFROM                },
					{ "HVC-SFROM",                   Type::STD_SFROM                },
					{ "HVC-SGROM",                   Type::STD_SGROM                },
					{ "HVC-SH1ROM",                  Type::STD_SHROM                },
					{ "HVC-SHROM",                   Type::STD_SHROM                },
					{ "HVC-SJROM",                   Type::STD_SJROM                },
					{ "HVC-SKROM",                   Type::STD_SKROM                },
					{ "HVC-SL1ROM",                  Type::STD_SLROM                },
					{ "HVC-SL2ROM",                  Type::STD_SLROM                },
					{ "HVC-SL3ROM",                  Type::STD_SLROM                },
					{ "HVC-SLROM",                   Type::STD_SLROM                },
					{ "HVC-SLRROM",                  Type::STD_SLROM                },
					{ "HVC-SNROM",                   Type::STD_SNROM                },
					{ "HVC-SOROM",                   Type::STD_SOROM                },
					{ "HVC-SROM",                    Type::STD_NROM                 },
					{ "HVC-STROM",                   Type::STD_NROM                 },
					{ "HVC-SUROM",                   Type::STD_SUROM                },
					{ "HVC-SXROM",                   Type::STD_SXROM                },
					{ "HVC-TBROM",                   Type::STD_TBROM                },
					{ "HVC-TEROM",                   Type::STD_TEROM                },
					{ "HVC-TFROM",                   Type::STD_TFROM                },
					{ "HVC-TGROM",                   Type::STD_TGROM                },
					{ "HVC-TKROM",                   Type::STD_TKROM                },
					{ "HVC-TKSROM",                  Type::STD_TKSROM               },
					{ "HVC-TL1ROM",                  Type::STD_TLROM                },
					{ "HVC-TL2ROM",                  Type::STD_TLROM                },
					{ "HVC-TLROM",                   Type::STD_TLROM                },
					{ "HVC-TLSROM",                  Type::STD_TLSROM               },
					{ "HVC-TNROM",                   Type::STD_TNROM                },
					{ "HVC-TQROM",                   Type::STD_TQROM                },
					{ "HVC-TR1ROM",                  Type::STD_TR1ROM               },
					{ "HVC-TSROM",                   Type::STD_TSROM                },
					{ "HVC-TVROM",                   Type::STD_TVROM                },
					{ "HVC-UN1ROM",                  Type::STD_UN1ROM               },
					{ "HVC-UNROM",                   Type::STD_UNROM                },
					{ "HVC-UOROM",                   Type::STD_UOROM                },
					{ "IREM-74*161/161/21/138",      Type::IREM_LROG017             },
					{ "IREM-BNROM",                  Type::STD_BNROM                },
					{ "IREM-G101",                   Type::IREM_G101A_0             },
					{ "IREM-G101-A",                 Type::IREM_G101A_0             },
					{ "IREM-G101-B",                 Type::IREM_G101B_0             },
					{ "IREM-HOLYDIVER",              Type::IREM_HOLYDIVER           },
					{ "IREM-NROM-128",               Type::STD_NROM                 },
					{ "IREM-NROM-256",               Type::STD_NROM                 },
					{ "IREM-UNROM",                  Type::STD_UNROM                },
					{ "JALECO-JF-01",                Type::JALECO_JF01              },
					{ "JALECO-JF-02",                Type::JALECO_JF02              },
					{ "JALECO-JF-03",                Type::JALECO_JF03              },
					{ "JALECO-JF-04",                Type::JALECO_JF04              },
					{ "JALECO-JF-05",                Type::JALECO_JF05              },
					{ "JALECO-JF-06",                Type::JALECO_JF06              },
					{ "JALECO-JF-07",                Type::JALECO_JF07              },
					{ "JALECO-JF-08",                Type::JALECO_JF08              },
					{ "JALECO-JF-09",                Type::JALECO_JF09              },
					{ "JALECO-JF-10",                Type::JALECO_JF10              },
					{ "JALECO-JF-11",                Type::JALECO_JF11              },
					{ "JALECO-JF-12",                Type::JALECO_JF12              },
					{ "JALECO-JF-13",                Type::JALECO_JF13              },
					{ "JALECO-JF-14",                Type::JALECO_JF14              },
					{ "JALECO-JF-15",                Type::JALECO_JF15              },
					{ "JALECO-JF-16",                Type::JALECO_JF16              },
					{ "JALECO-JF-17",                Type::JALECO_JF17              },
					{ "JALECO-JF-18",                Type::JALECO_JF18              },
					{ "JALECO-JF-19",                Type::JALECO_JF19              },
					{ "JALECO-JF-20",                Type::JALECO_JF20              },
					{ "JALECO-JF-21",                Type::JALECO_JF21              },
					{ "JALECO-JF-22",                Type::JALECO_JF22              },
					{ "JALECO-JF-23",                Type::JALECO_JF23              },
					{ "JALECO-JF-24",                Type::JALECO_JF24              },
					{ "JALECO-JF-25",                Type::JALECO_JF25              },
					{ "JALECO-JF-26",                Type::JALECO_JF26              },
					{ "JALECO-JF-27",                Type::JALECO_JF27              },
					{ "JALECO-JF-28",                Type::JALECO_JF28              },
					{ "JALECO-JF-29",                Type::JALECO_JF29              },
					{ "JALECO-JF-30",                Type::JALECO_JF30              },
					{ "JALECO-JF-31",                Type::JALECO_JF31              },
					{ "JALECO-JF-32",                Type::JALECO_JF32              },
					{ "JALECO-JF-33",                Type::JALECO_JF33              },
					{ "JALECO-JF-34",                Type::JALECO_JF34              },
					{ "JALECO-JF-35",                Type::JALECO_JF35              },
					{ "JALECO-JF-36",                Type::JALECO_JF36              },
					{ "JALECO-JF-37",                Type::JALECO_JF37              },
					{ "JALECO-JF-38",                Type::JALECO_JF38              },
					{ "JALECO-JF-39",                Type::JALECO_JF39              },
					{ "JALECO-JF-40",                Type::JALECO_JF40              },
					{ "JALECO-JF-41",                Type::JALECO_JF41              },
					{ "KONAMI-74*139/74",            Type::DISCRETE_74_139_74       },
					{ "KONAMI-CNROM",                Type::STD_CNROM                },
					{ "KONAMI-NROM-128",             Type::STD_NROM                 },
					{ "KONAMI-SLROM",                Type::STD_SLROM                },
					{ "KONAMI-TLROM",                Type::STD_TLROM                },
					{ "KONAMI-UNROM",                Type::STD_UNROM                },
					{ "KONAMI-VRC-1",                Type::KONAMI_VRC1              },
					{ "KONAMI-VRC-2",                Type::KONAMI_VRC2              },
					{ "KONAMI-VRC-3",                Type::KONAMI_VRC3              },
					{ "KONAMI-VRC-4",                Type::KONAMI_VRC4_0            },
					{ "KONAMI-VRC-6",                Type::KONAMI_VRC6_0            },
					{ "KONAMI-VRC-7",                Type::KONAMI_VRC7_0            },
					{ "MLT-ACTION52",                Type::AE_STD                   },
					{ "MLT-CALTRON6IN1",             Type::CALTRON_6IN1             },
					{ "MLT-MAXI15",                  Type::AVE_D1012                },
					{ "NAMCOT-163",                  Type::NAMCOT_163_0             },
					{ "NAMCOT-3301",                 Type::STD_NROM                 },
					{ "NAMCOT-3302",                 Type::STD_NROM                 },
					{ "NAMCOT-3303",                 Type::STD_NROM                 },
					{ "NAMCOT-3305",                 Type::STD_NROM                 },
					{ "NAMCOT-3311",                 Type::STD_NROM                 },
					{ "NAMCOT-3401",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3405",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3406",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3407",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3411",                 Type::STD_NROM                 },
					{ "NAMCOT-3413",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3414",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3415",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3416",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3417",                 Type::STD_DE1ROM               },
					{ "NAMCOT-3425",                 Type::NAMCOT_3425              },
					{ "NAMCOT-3433",                 Type::NAMCOT_3433              },
					{ "NAMCOT-3443",                 Type::NAMCOT_3443              },
					{ "NAMCOT-3446",                 Type::NAMCOT_3446              },
					{ "NAMCOT-3451",                 Type::STD_DE1ROM               },
					{ "NES-AMROM",                   Type::STD_AMROM                },
					{ "NES-AN1ROM",                  Type::STD_AN1ROM               },
					{ "NES-ANROM",                   Type::STD_ANROM                },
					{ "NES-AOROM",                   Type::STD_AOROM                },
					{ "NES-B4",                      Type::STD_TLROM                },
					{ "NES-BNROM",                   Type::STD_BNROM                },
					{ "NES-BTR",                     Type::CUSTOM_BTR               },
					{ "NES-CNROM",                   Type::STD_CNROM                },
					{ "NES-CPROM",                   Type::STD_CPROM                },
					{ "NES-DE1ROM",                  Type::STD_DE1ROM               },
					{ "NES-DEROM",                   Type::STD_DEROM                },
					{ "NES-DRROM",                   Type::STD_DRROM                },
					{ "NES-EKROM",                   Type::STD_EKROM                },
					{ "NES-ELROM",                   Type::STD_ELROM                },
					{ "NES-ETROM",                   Type::STD_ETROM                },
					{ "NES-EVENT",                   Type::CUSTOM_EVENT             },
					{ "NES-EWROM",                   Type::STD_EWROM                },
					{ "NES-FJROM",                   Type::STD_FJROM                },
					{ "NES-FKROM",                   Type::STD_FKROM                },
					{ "NES-GNROM",                   Type::STD_GNROM                },
					{ "NES-HKROM",                   Type::STD_HKROM                },
					{ "NES-HROM",                    Type::STD_NROM                 },
					{ "NES-JLROM",                   Type::STD_JLROM                },
					{ "NES-JSROM",                   Type::STD_JSROM                },
					{ "NES-MHROM",                   Type::STD_MHROM                },
					{ "NES-NROM",                    Type::STD_NROM                 },
					{ "NES-NROM-128",                Type::STD_NROM                 },
					{ "NES-NROM-256",                Type::STD_NROM                 },
					{ "NES-NTBROM",                  Type::STD_NTBROM               },
					{ "NES-PEEOROM",                 Type::STD_PEEOROM              },
					{ "NES-PNROM",                   Type::STD_PNROM                },
					{ "NES-QJ",                      Type::CUSTOM_QJ                },
					{ "NES-RROM",                    Type::STD_NROM                 },
					{ "NES-RROM-128",                Type::STD_NROM                 },
					{ "NES-SAROM",                   Type::STD_SAROM                },
					{ "NES-SBROM",                   Type::STD_SBROM                },
					{ "NES-SC1ROM",                  Type::STD_SCROM                },
					{ "NES-SCROM",                   Type::STD_SCROM                },
					{ "NES-SEROM",                   Type::STD_SEROM                },
					{ "NES-SF1ROM",                  Type::STD_SFROM                },
					{ "NES-SFROM",                   Type::STD_SFROM                },
					{ "NES-SGROM",                   Type::STD_SGROM                },
					{ "NES-SH1ROM",                  Type::STD_SHROM                },
					{ "NES-SHROM",                   Type::STD_SHROM                },
					{ "NES-SJROM",                   Type::STD_SJROM                },
					{ "NES-SKROM",                   Type::STD_SKROM                },
					{ "NES-SL1ROM",                  Type::STD_SLROM                },
					{ "NES-SL2ROM",                  Type::STD_SLROM                },
					{ "NES-SL3ROM",                  Type::STD_SLROM                },
					{ "NES-SLROM",                   Type::STD_SLROM                },
					{ "NES-SLRROM",                  Type::STD_SLROM                },
					{ "NES-SNROM",                   Type::STD_SNROM                },
					{ "NES-SOROM",                   Type::STD_SOROM                },
					{ "NES-SROM",                    Type::STD_NROM                 },
					{ "NES-STROM",                   Type::STD_NROM                 },
					{ "NES-SUROM",                   Type::STD_SUROM                },
					{ "NES-SXROM",                   Type::STD_SXROM                },
					{ "NES-TBROM",                   Type::STD_TBROM                },
					{ "NES-TEROM",                   Type::STD_TEROM                },
					{ "NES-TFROM",                   Type::STD_TFROM                },
					{ "NES-TGROM",                   Type::STD_TGROM                },
					{ "NES-TKROM",                   Type::STD_TKROM                },
					{ "NES-TKSROM",                  Type::STD_TKSROM               },
					{ "NES-TL1ROM",                  Type::STD_TLROM                },
					{ "NES-TL2ROM",                  Type::STD_TLROM                },
					{ "NES-TLROM",                   Type::STD_TLROM                },
					{ "NES-TLSROM",                  Type::STD_TLSROM               },
					{ "NES-TNROM",                   Type::STD_TNROM                },
					{ "NES-TQROM",                   Type::STD_TQROM                },
					{ "NES-TR1ROM",                  Type::STD_TR1ROM               },
					{ "NES-TSROM",                   Type::STD_TSROM                },
					{ "NES-TVROM",                   Type::STD_TVROM                },
					{ "NES-UN1ROM",                  Type::STD_UN1ROM               },
					{ "NES-UNROM",                   Type::STD_UNROM                },
					{ "NES-UOROM",                   Type::STD_UOROM                },
					{ "NES-WH",                      Type::CUSTOM_WH                },
					{ "NTDEC-N715062",               Type::NTDEC_N715062            },
					{ "PAL-MH",                      Type::STD_MHROM                },
					{ "PAL-ZZ",                      Type::CUSTOM_ZZ                },
					{ "SACHEN-8259A",                Type::SACHEN_8259A             },
					{ "SACHEN-8259B",                Type::SACHEN_8259B             },
					{ "SACHEN-8259C",                Type::SACHEN_8259C             },
					{ "SACHEN-8259D",                Type::SACHEN_8259D             },
					{ "SACHEN-CNROM",                Type::STD_CXROM                },
					{ "SETA-NROM-128",               Type::STD_NROM                 },
					{ "SUNSOFT-1",                   Type::SUNSOFT_1                },
					{ "SUNSOFT-2",                   Type::SUNSOFT_2B               },
					{ "SUNSOFT-3",                   Type::SUNSOFT_3                },
					{ "SUNSOFT-4",                   Type::SUNSOFT_4_0              },
					{ "SUNSOFT-5B",                  Type::SUNSOFT_5B_0             },
					{ "SUNSOFT-FME-7",               Type::SUNSOFT_FME7_0           },
					{ "SUNSOFT-NROM-256",            Type::STD_NROM                 },
					{ "TAITO-74*139/74",             Type::DISCRETE_74_139_74       },
					{ "TAITO-74*161/161/32",         Type::DISCRETE_74_161_161_32_A },
					{ "TAITO-CNROM",                 Type::STD_CNROM                },
					{ "TAITO-NROM-128",              Type::STD_NROM                 },
					{ "TAITO-NROM-256",              Type::STD_NROM                 },
					{ "TAITO-TC0190FMC",             Type::TAITO_TC0190FMC          },
					{ "TAITO-TC0190FMC+PAL16R4",     Type::TAITO_TC0190FMC_PAL16R4  },
					{ "TAITO-UNROM",                 Type::STD_UNROM                },
					{ "TAITO-X1-005",                Type::TAITO_X1005              },
					{ "TAITO-X1-017",                Type::TAITO_X1017              },
					{ "TENGEN-800002",               Type::TENGEN_800002            },
					{ "TENGEN-800003",               Type::STD_NROM                 },
					{ "TENGEN-800004",               Type::TENGEN_800004            },
					{ "TENGEN-800008",               Type::TENGEN_800008            },
					{ "TENGEN-800030",               Type::TENGEN_800030            },
					{ "TENGEN-800032",               Type::TENGEN_800032            },
					{ "TENGEN-800037",               Type::TENGEN_800037            },
					{ "TENGEN-800042",               Type::TENGEN_800042            },
					{ "UNL-22211",                   Type::TXC_22211A               },
					{ "UNL-603-5052",                Type::BTL_6035052              },
					{ "UNL-8237",                    Type::SUPERGAME_POCAHONTAS2    },
					{ "UNL-AX5705",                  Type::BTL_AX5705               },
					{ "UNL-CC-21",                   Type::UNL_CC21                 },
					{ "UNL-EDU2000",                 Type::UNL_EDU2000              },
					{ "UNL-H2288",                   Type::KAY_H2288                },
					{ "UNL-KOF97",                   Type::UNL_KINGOFFIGHTERS97     },
					{ "UNL-KS7032",                  Type::KAISER_KS7032            },
					{ "UNL-SA-0036",                 Type::SACHEN_SA0036            },
					{ "UNL-SA-0037",                 Type::SACHEN_SA0037            },
					{ "UNL-SA-016-1M",               Type::SACHEN_SA0161M           },
					{ "UNL-SA-72007",                Type::SACHEN_SA72007           },
					{ "UNL-SA-72008",                Type::SACHEN_SA72008           },
					{ "UNL-SA-NROM",                 Type::SACHEN_TCA01             },
					{ "UNL-SACHEN-74LS374N",         Type::SACHEN_74_374B           },
					{ "UNL-SACHEN-8259A",            Type::SACHEN_8259A             },
					{ "UNL-SACHEN-8259B",            Type::SACHEN_8259B             },
					{ "UNL-SACHEN-8259C",            Type::SACHEN_8259C             },
					{ "UNL-SACHEN-8259D",            Type::SACHEN_8259D             },
					{ "UNL-SHERO",                   Type::SACHEN_STREETHEROES      },
					{ "UNL-SL1632",                  Type::REXSOFT_SL1632           },
					{ "UNL-SMB2J",                   Type::BTL_SMB2_C               },
					{ "UNL-T-230",                   Type::BTL_T230                 },
					{ "UNL-TC-U01-1.5M",             Type::SACHEN_TCU01             },
					{ "UNL-TEK90",                   Type::JYCOMPANY_TYPE_A         },
					{ "UNL-TF1201",                  Type::UNL_TF1201               },
					{ "VIRGIN-SNROM",                Type::STD_SNROM                }
				};

			#ifdef NST_DEBUG
				for (uint i=1; i < sizeof(array(lut)); ++i)
					NST_ASSERT( Element::Less()( lut[i-1], lut[i] ) );
			#endif

				const Element* const result = std::lower_bound
				(
					lut,
					lut + sizeof(array(lut)),
					string,
					Element::Less()
				);

				if (result == lut+sizeof(array(lut)) || Core::StringCompare( result->name, string ) != 0)
					return false;

				name = result->name;
				Type::Id id = static_cast<Type::Id>(result->id);

				switch (id)
				{
					case Type::STD_NTBROM:

						if (prg.Size() > SIZE_128K)
							id = Type::SUNSOFT_DCS;

						break;

					case Type::CUSTOM_FB02:

						if (wram == 0)
						{
							id = Type::STD_NROM;
						}
						else if (wram > SIZE_2K)
						{
							id = Type::CUSTOM_FB04;
						}
						break;

					case Type::DISCRETE_74_161_161_32_A:

						if (nmt != Type::NMT_VERTICAL && nmt != Type::NMT_HORIZONTAL)
							id = Type::DISCRETE_74_161_161_32_B;

						break;

					case Type::SUNSOFT_2B:

						if (nmt == Type::NMT_VERTICAL || nmt == Type::NMT_HORIZONTAL)
							id = Type::SUNSOFT_2A;

						break;

					case Type::SUNSOFT_4_0:

						if (prg.Size() > SIZE_128K)
						{
							id = Type::SUNSOFT_DCS;
						}
						else if (wram)
						{
							id = Type::SUNSOFT_4_1;
						}
						break;

					case Type::NAMCOT_163_0:

						if (mmcBattery)
						{
							id = (wram ? Type::NAMCOT_163_S_1 : Type::NAMCOT_163_S_0);
						}
						else if (wram)
						{
							id = Type::NAMCOT_163_1;
						}
						break;

					default:

						if (wram)
						{
							switch (id)
							{
								case Type::KONAMI_VRC4_0:  id = (wram > SIZE_2K ? Type::KONAMI_VRC4_2 : Type::KONAMI_VRC4_1); break;
								case Type::KONAMI_VRC6_0:  id = Type::KONAMI_VRC6_1; break;
								case Type::KONAMI_VRC7_0:  id = Type::KONAMI_VRC7_1; break;
								case Type::IREM_G101A_0:   id = Type::IREM_G101A_1; break;
								case Type::IREM_G101B_0:   id = Type::IREM_G101B_1; break;
								case Type::SUNSOFT_FME7_0: id = Type::SUNSOFT_FME7_1; break;
								case Type::SUNSOFT_5B_0:   id = Type::SUNSOFT_5B_1; break;
							}
						}
						break;
				}

				type = Type( id, prg, chr, nmt, wramBattery || mmcBattery, false );

				return true;
			}

			bool Board::Context::DetectBoard(const byte mapper,const dword wram,bool wramAuto)
			{
				Type::Id id;

				const dword prg = this->prg.Size();
				const dword chr = this->chr.Size();

				bool useWramAuto = false;

				if (wram)
				{
					wramAuto = false;
				}
				else if (wramAuto)
				{
					wramAuto = false;
					useWramAuto = true;
				}

				switch (mapper)
				{
					case 0:

						if ((prg < SIZE_32K && prg != SIZE_16K) || !chr || wram >= SIZE_8K || (nmt != Type::NMT_HORIZONTAL && nmt != Type::NMT_VERTICAL))
						{
							name = "NROM (non-standard)";
							id = Type::UNL_NROM;
						}
						else if (wram && wramBattery)
						{
							if (wram > SIZE_2K)
							{
								name = "FB-04";
								id = Type::CUSTOM_FB04;
							}
							else
							{
								name = "FB-02";
								id = Type::CUSTOM_FB02;
							}
						}
						else
						{
							if (prg == SIZE_16K)
								name = "NROM-128";
							else
								name = "NROM-256";

							id = Type::STD_NROM;
						}
						break;

					case 155:

						chips.Add(L"MMC1A");

					case 1:

						if (prg == SIZE_64K && (chr == SIZE_16K || chr == SIZE_32K || chr == SIZE_64K) && wram <= SIZE_8K)
						{
							if (wram || useWramAuto)
							{
								wramAuto = useWramAuto;
								name = "SAROM";
								id = Type::STD_SAROM;
							}
							else
							{
								name = "SBROM";
								id = Type::STD_SBROM;
							}
						}
						else if (prg == SIZE_64K && chr >= SIZE_128K && !wram)
						{
							name = "SCROM";
							id = Type::STD_SCROM;
						}
						else if (prg == SIZE_32K && (chr == SIZE_16K || chr == SIZE_32K || chr == SIZE_64K) && !wram)
						{
							name = "SEROM";
							id = Type::STD_SEROM;
						}
						else if ((prg == SIZE_128K || prg == SIZE_256K) && (chr == SIZE_16K || chr == SIZE_32K || chr == SIZE_64K) && wram <= SIZE_8K)
						{
							if (wram || useWramAuto)
							{
								wramAuto = useWramAuto;
								name = "SJROM";
								id = Type::STD_SJROM;
							}
							else
							{
								name = "SFROM";
								id = Type::STD_SFROM;
							}
						}
						else if ((prg == SIZE_128K || prg == SIZE_256K) && !chr && wram <= SIZE_16K)
						{
							if (wram > SIZE_8K)
							{
								name = "SOROM";
								id = Type::STD_SOROM;
							}
							else if (wram || useWramAuto)
							{
								wramAuto = useWramAuto;
								name = "SNROM";
								id = Type::STD_SNROM;
							}
							else
							{
								name = "SGROM";
								id = Type::STD_SGROM;
							}
						}
						else if (prg == SIZE_32K && chr == SIZE_128K && !wram)
						{
							name = "SHROM";
							id = Type::STD_SHROM;
						}
						else if ((prg == SIZE_128K || prg == SIZE_256K) && chr == SIZE_128K && wram <= SIZE_8K)
						{
							if (wram || useWramAuto)
							{
								wramAuto = useWramAuto;
								name = "SKROM";
								id = Type::STD_SKROM;
							}
							else
							{
								name = "SLROM";
								id = Type::STD_SLROM;
							}
						}
						else if ((prg == SIZE_64K || prg == SIZE_128K || prg == SIZE_256K) && chr == SIZE_128K && !wram)
						{
							name = "SL1ROM";
							id = Type::STD_SLROM;
						}
						else if (prg >= SIZE_512K && !chr && wram < SIZE_32K)
						{
							name = "SUROM";
							id = Type::STD_SUROM;
						}
						else if ((prg == SIZE_128K || prg == SIZE_256K || prg >= SIZE_512K) && !chr && wram >= SIZE_32K)
						{
							name = "SXROM";
							id = Type::STD_SXROM;
						}
						else
						{
							name = "SxROM (non-standard)";

							if (wram)
								id = Type::STD_SKROM;
							else
								id = Type::STD_SLROM;
						}
						break;

					case 2:

						if (!chr && !wram && (nmt == Type::NMT_HORIZONTAL || nmt == Type::NMT_VERTICAL))
						{
							if (prg == SIZE_128K)
							{
								name = "UNROM";
								id = Type::STD_UXROM;
								break;
							}
							else if (prg == SIZE_256K)
							{
								name = "UOROM";
								id = Type::STD_UXROM;
								break;
							}
						}

						name = "UxROM (non-standard)";
						id = Type::UNL_UXROM;
						break;

					case 185:

						if (!this->prg.PinsDefined())
							return false;

					case 3:

						if ((prg != SIZE_16K && prg != SIZE_32K) || (chr != SIZE_8K && chr != SIZE_16K && chr != SIZE_32K) || (nmt != Type::NMT_HORIZONTAL && nmt != Type::NMT_VERTICAL))
						{
							name = "CxROM (non-standard)";
							id = Type::UNL_CXROM;
						}
						else if (wram)
						{
							name = "X79B";
							id = Type::CUSTOM_X79B;
						}
						else
						{
							name = "CNROM";
							id = Type::STD_CXROM;
						}
						break;

					case 4:

						if (nmt == Type::NMT_FOURSCREEN)
						{
							if (prg == SIZE_64K && (chr == SIZE_32K || chr == SIZE_64K))
							{
								name = "TVROM";
								id = Type::STD_TVROM;
							}
							else if ((prg == SIZE_128K || prg == SIZE_256K || prg >= SIZE_512K) && chr == SIZE_64K)
							{
								name = "TR1ROM";
								id = Type::STD_TR1ROM;
							}
							else
							{
								name = "TxROM (non-standard)";
								id = Type::UNL_TRXROM;
							}
						}
						else
						{
							if (prg == SIZE_32K && (chr == SIZE_32K || chr == SIZE_64K) && !wram)
							{
								name = "TEROM";
								id = Type::STD_TEROM;
							}
							else if (prg == SIZE_64K && (chr == SIZE_32K || chr == SIZE_64K))
							{
								name = "TBROM";
								id = Type::STD_TBROM;
							}
							else if ((prg == SIZE_128K || prg == SIZE_256K || prg >= SIZE_512K) && (chr == SIZE_32K || chr == SIZE_64K) && !wram)
							{
								name = "TFROM";
								id = Type::STD_TFROM;
							}
							else if ((prg == SIZE_128K || prg == SIZE_256K || prg >= SIZE_512K) && !chr)
							{
								if (wram || useWramAuto)
								{
									wramAuto = useWramAuto;
									name = "TNROM";
									id = Type::STD_TNROM;
								}
								else
								{
									name = "TGROM";
									id = Type::STD_TGROM;
								}
							}
							else if ((prg == SIZE_128K || prg == SIZE_256K || prg >= SIZE_512K) && (chr == SIZE_128K || chr >= SIZE_256K))
							{
								if (wram || useWramAuto)
								{
									wramAuto = useWramAuto;

									if (wramBattery)
									{
										name = "TKROM";
										id = Type::STD_TKROM;
									}
									else
									{
										name = "TSROM";
										id = Type::STD_TSROM;
									}
								}
								else
								{
									name = "TLROM";
									id = Type::STD_TLROM;
								}
							}
							else
							{
								name = "TxROM (non-standard)";

								if (wram)
									id = Type::STD_TKROM;
								else
									id = Type::STD_TLROM;
							}
						}
						break;

					case 5:

						if
						(
							(prg == SIZE_128K || prg == SIZE_256K || prg == SIZE_512K) &&
							(chr == SIZE_128K || chr == SIZE_256K || chr == SIZE_512K) &&
							(wram <= SIZE_32K)
						)
						{
							if (wram > SIZE_16K)
							{
								name = "EWROM";
								id = Type::STD_EWROM;
							}
							else if (wram > SIZE_8K)
							{
								name = "ETROM";
								id = Type::STD_ETROM;
							}
							else if (wram || useWramAuto)
							{
								wramAuto = useWramAuto;
								name = "EKROM";
								id = Type::STD_EKROM;
							}
							else
							{
								name = "ELROM";
								id = Type::STD_ELROM;
							}
						}
						else
						{
							name = "ExROM (non-standard)";

							if (wram > SIZE_40K)
							{
								id = Type::STD_EXROM_5;
							}
							else if (wram > SIZE_32K)
							{
								id = Type::STD_EXROM_4;
							}
							else if (wram > SIZE_16K)
							{
								id = Type::STD_EXROM_3;
							}
							else if (wram > SIZE_8K)
							{
								id = Type::STD_EXROM_2;
							}
							else if (wram || useWramAuto)
							{
								wramAuto = useWramAuto;
								id = Type::STD_EXROM_1;
							}
							else
							{
								id = Type::STD_EXROM_0;
							}
						}
						break;

					case 6:

						name = "FFE F4xxx / F3xxx (a)";
						id = Type::CUSTOM_FFE4;
						break;

					case 7:

						if (!chr && !wram)
						{
							if (prg == SIZE_256K)
							{
								name = "AOROM";
								id = Type::STD_AOROM;
								break;
							}
							else if (prg == SIZE_128K)
							{
								name = "ANROM";
								id = Type::STD_ANROM;
								break;
							}
							else if (prg == SIZE_64K)
							{
								name = "AN1ROM";
								id = Type::STD_AN1ROM;
								break;
							}
						}

						name = "AxROM (non-standard)";
						id = Type::UNL_AXROM;
						break;

					case 8:

						name = "FFE F3xxx (b)";
						id = Type::CUSTOM_FFE3;
						break;

					case 9:

						if (prg < SIZE_128K || (chr < SIZE_128K && chr != SIZE_8K && chr != SIZE_16K && chr != SIZE_32K && chr != SIZE_64K))
						{
							name = "PxROM (non-standard)";
							id = Type::STD_PNROM;
						}
						else if (wram && wramBattery)
						{
							name = "PNROM PC10";
							id = Type::STD_PNROM_PC10;
						}
						else
						{
							name = "PNROM / PEEOROM";
							id = Type::STD_PNROM;
						}
						break;

					case 10:

						if (chr == SIZE_8K || chr == SIZE_16K || chr == SIZE_32K || chr == SIZE_64K || chr >= SIZE_128K)
						{
							if (prg == SIZE_128K)
							{
								name = "FJROM";
								id = Type::STD_FJROM;
								break;
							}
							else if (prg >= SIZE_256K)
							{
								name = "FKROM";
								id = Type::STD_FKROM;
								break;
							}
						}

						name = "FxROM (non-standard)";
						id = Type::STD_FKROM;
						break;

					case 11:

						name = "COLORDREAMS 74*377";
						id = Type::DISCRETE_74_377;
						break;

					case 12:

						name = "REX DBZ5";
						id = Type::REXSOFT_DBZ5;
						break;

					case 13:

						name = "CPROM";
						id = Type::STD_CPROM;
						break;

					case 14:

						name = "SOMERITEAM SL-1632";
						id = Type::REXSOFT_SL1632;
						break;

					case 15:

						name = "WAIXING / BMC CONTRA 100-IN-1";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::WAIXING_PS2_1;
						}
						else
						{
							id = Type::WAIXING_PS2_0;
						}
						break;

					case 16:

						if (!wram && prg <= SIZE_256K)
						{
							name = "BANDAI LZ93D50 +24C02";
							id = Type::BANDAI_LZ93D50_24C02;
							break;
						}

					case 153:

						if (prg == SIZE_512K)
						{
							name = "BANDAI BA-JUMP2";
							id = Type::BANDAI_BAJUMP2;
						}
						else
						{
							name = "BANDAI FCG-1/FCG-2";
							id = Type::BANDAI_FCG1;
						}
						break;

					case 17:

						name = "FFE F8xxx / SMxxxx";
						id = Type::CUSTOM_FFE8;
						break;

					case 18:

						name = "JALECO SS88006";
						id = Type::JALECO_SS88006;
						break;

					case 19:

						name = "NAMCOT 163";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;

							if (mmcBattery)
								id = Type::NAMCOT_163_S_1;
							else
								id = Type::NAMCOT_163_1;
						}
						else
						{
							if (mmcBattery)
								id = Type::NAMCOT_163_S_0;
							else
								id = Type::NAMCOT_163_0;
						}
						break;

					case 21:
					case 25:

						if (!this->chips.Has(L"Konami VRC IV"))
							return false;

						name = "KONAMI VRC4";
						id = Type::KONAMI_VRC4_2;
						break;

					case 22:

						if (!this->chips.Has(L"Konami VRC II"))
						{
							Chips::Type& chip = chips.Add(L"Konami VRC II");

							chip.Pin(3)  = L"PRG A0";
							chip.Pin(4)  = L"PRG A1";
							chip.Pin(21) = (chr >= SIZE_256K ? L"CHR A17" : L"NC");
							chip.Pin(22) = L"CHR A15";
							chip.Pin(23) = L"CHR A10";
							chip.Pin(24) = L"CHR A12";
							chip.Pin(25) = L"CHR A13";
							chip.Pin(26) = L"CHR A11";
							chip.Pin(27) = L"CHR A14";
							chip.Pin(28) = L"CHR A16";
						}

						name = "KONAMI VRC2";
						id = Type::KONAMI_VRC2;
						break;

					case 23:

						if (prg >= SIZE_512K)
						{
							if (!this->chips.Has(L"Konami VRC IV"))
							{
								Chips::Type& chip = chips.Add(L"Konami VRC IV");

								chip.Pin(3) = L"PRG A3";
								chip.Pin(4) = L"PRG A2";
							}

							name = "BMC VRC4";
							id = Type::BMC_VRC4;
						}
						else if (this->chips.Has(L"Konami VRC II"))
						{
							name = "KONAMI VRC2";
							id = Type::KONAMI_VRC2;
						}
						else if (this->chips.Has(L"Konami VRC IV"))
						{
							name = "KONAMI VRC4";
							id = Type::KONAMI_VRC4_2;
						}
						else
						{
							return false;
						}
						break;

					case 24:
					case 26:

						if (!this->chips.Has(L"Konami VRC VI"))
						{
							Chips::Type& chip = chips.Add(L"Konami VRC VI");

							if (mapper == 24)
							{
								chip.Pin(9)  = L"PRG A1";
								chip.Pin(10) = L"PRG A0";

								name = "KONAMI VRC6 (a)";
							}
							else
							{
								chip.Pin(9)  = L"PRG A0";
								chip.Pin(10) = L"PRG A1";

								name = "KONAMI VRC6 (b)";
							}
						}

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::KONAMI_VRC6_1;
						}
						else
						{
							id = Type::KONAMI_VRC6_0;
						}
						break;

					case 27:

						name = "UNL WORLDHERO";
						id = Type::UNL_WORLDHERO;
						break;

					case 32:

						name = "IREM G-101";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::IREM_G101A_1;
						}
						else
						{
							id = Type::IREM_G101A_0;
						}
						break;

					case 33:

						name = "TAITO TC0190FMC";
						id = Type::TAITO_TC0190FMC;
						break;

					case 34:

						if (chr)
						{
							name = "AVE NINA-001";
							id = Type::AVE_NINA001;
						}
						else if (prg == SIZE_128K && !wram && (nmt == Type::NMT_HORIZONTAL || nmt == Type::NMT_VERTICAL))
						{
							name = "BNROM";
							id = Type::STD_BNROM;
						}
						else
						{
							name = "BxROM (non-standard)";
							id = Type::UNL_BXROM;
						}
						break;

					case 36:

						name = "TXC 01-22000-400";
						id = Type::TXC_POLICEMAN;
						break;

					case 37:

						name = "ZZ";
						id = Type::CUSTOM_ZZ;
						break;

					case 38:

						name = "BIT CORP 74*161/138";
						id = Type::DISCRETE_74_161_138;
						break;

					case 39:

						name = "STUDY & GAME 32-IN-1";
						id = Type::SUBOR_STUDYNGAME;
						break;

					case 40:

						name = "BTL SMB2 (a)";
						id = Type::BTL_SMB2_A;
						break;

					case 41:

						name = "CALTRON 6-IN-1";
						id = Type::CALTRON_6IN1;
						break;

					case 42:

						if (chr)
						{
							name = "BTL UPA";
							id = Type::BTL_MARIOBABY;
						}
						else
						{
							name = "BTL AI SENSHI NICOL";
							id = Type::BTL_AISENSHINICOL;
						}
						break;

					case 43:

						name = "BTL SMB2 (c)";
						id = Type::BTL_SMB2_C;
						break;

					case 44:

						name = "BMC SUPERBIG 7-IN-1";
						id = Type::BMC_SUPERBIG_7IN1;
						break;

					case 45:

						name = "BMC SUPER/HERO X-IN-1";
						id = Type::BMC_HERO;
						break;

					case 46:

						name = "RUMBLESTATION 15-IN-1";
						id = Type::CUSTOM_RUMBLESTATION;
						break;

					case 47:

						name = "QJ";
						id = Type::CUSTOM_QJ;
						break;

					case 48:

						name = "TAITO TC0190FMC +PAL16R4";
						id = Type::TAITO_TC0190FMC_PAL16R4;
						break;

					case 49:

						name = "BMC SUPERHIK 4-IN-1";
						id = Type::BMC_SUPERHIK_4IN1;
						break;

					case 50:

						name = "BTL SMB2 (b)";
						id = Type::BTL_SMB2_B;
						break;

					case 51:

						name = "BMC 11-IN-1 BALLGAMES";
						id = Type::BMC_BALLGAMES_11IN1;
						break;

					case 52:

						name = "BMC MARIOPARTY 7-IN-1";
						id = Type::BMC_MARIOPARTY_7IN1;
						break;

					case 53:

						name = "BMC SUPERVISION 16-IN-1";
						id = Type::BMC_SUPERVISION_16IN1;
						break;

					case 54:

						name = "BMC NOVELDIAMOND 9999999-IN-1";
						id = Type::BMC_NOVELDIAMOND;
						break;

					case 55:

						name = "BTL GENIUS MERIO BROS";
						id = Type::BTL_GENIUSMERIOBROS;
						break;

					case 56:

						name = "KAISER KS-202";
						id = Type::KAISER_KS202;
						break;

					case 57:

						name = "BMC GAMESTAR (a)";
						id = Type::BMC_GKA;
						break;

					case 58:

						name = "BMC GAMESTAR (b)";
						id = Type::BMC_GKB;
						break;

					case 60:

						if (prg == SIZE_64K && chr == SIZE_32K)
						{
							name = "BMC RESETBASED 4-IN-1";
							id = Type::BMC_RESETBASED_4IN1;
						}
						else
						{
							name = "BMC VT5201";
							id = Type::BMC_VT5201;
						}
						break;

					case 61:

						name = "TXC / BMC 20-IN-1";
						id = Type::RCM_TETRISFAMILY;
						break;

					case 62:

						name = "BMC SUPER 700-IN-1";
						id = Type::BMC_SUPER_700IN1;
						break;

					case 63:

						name = "BMC CH-001";
						id = Type::BMC_CH001;
						break;

					case 64:

						name = "TENGEN 800032";
						id = Type::TENGEN_800032;
						break;

					case 65:

						name = "IREM H-3001";
						id = Type::IREM_H3001;
						break;

					case 66:

						if ((chr == SIZE_8K || chr == SIZE_16K || chr == SIZE_32K) && !wram && (nmt == Type::NMT_HORIZONTAL || nmt == Type::NMT_VERTICAL))
						{
							if (prg == SIZE_64K)
							{
								name = "GNROM / MHROM";
								id = Type::STD_GNROM;
								break;
							}
							else if (prg == SIZE_32K || prg == SIZE_128K)
							{
								name = "GNROM";
								id = Type::STD_GNROM;
								break;
							}
						}

						name = "GxROM (non-standard)";
						id = Type::UNL_GXROM;
						break;

					case 67:

						name = "SUNSOFT 3";
						id = Type::SUNSOFT_3;
						break;

					case 68:

						if (prg > SIZE_128K)
						{
							name = "SUNSOFT DOUBLE CASETTE SYSTEM";
							id = Type::SUNSOFT_DCS;
						}
						else
						{
							name = "SUNSOFT 4";

							if (wram || useWramAuto)
							{
								wramAuto = useWramAuto;
								id = Type::SUNSOFT_4_1;
							}
							else
							{
								id = Type::SUNSOFT_4_0;
							}
						}
						break;

					case 69:

						name = "SUNSOFT 5B/FME7";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::SUNSOFT_5B_1;
						}
						else
						{
							id = Type::SUNSOFT_5B_0;
						}
						break;

					case 70:

						name = "74*161/161/32 (a)";
						id = Type::DISCRETE_74_161_161_32_A;
						break;

					case 71:

						if (prg >= SIZE_256K)
						{
							name = "CAMERICA BF9093";
							id = Type::CAMERICA_BF9093;
						}
						else
						{
							name = "CAMERICA BF9093/BF9097";
							id = Type::CAMERICA_BF909X;
						}
						break;

					case 72:

						name = "JALECO JF-26/28";
						id = Type::JALECO_JF26;
						break;

					case 73:

						name = "KONAMI VRC3";
						id = Type::KONAMI_VRC3;
						break;

					case 74:

						name = "WAIXING (a)";
						id = Type::WAIXING_TYPE_A;
						break;

					case 75:

						name = "KONAMI VRC1 / JALECO JF-22";
						id = Type::KONAMI_VRC1;
						break;

					case 76:

						name = "NAMCOT 3446";
						id = Type::NAMCOT_3446;
						break;

					case 77:

						name = "IREM 74*161/161/21/138";
						id = Type::IREM_LROG017;
						break;

					case 78:

						name = "JALECO JF-16";
						id = Type::JALECO_JF16;
						break;

					case 79:

						name = "AVE NINA-03 / NINA-06 / MB-91";
						id = Type::AVE_NINA06;
						break;

					case 80:

						name = "TAITO X1-005 (a)";
						id = Type::TAITO_X1005;
						break;

					case 82:

						name = "TAITO X1-017";
						id = Type::TAITO_X1017;
						break;

					case 83:

						if (prg >= SIZE_1024K || chr >= SIZE_1024K)
						{
							name = "BMC DRAGONBALLPARTY 4-IN-1";
							id = Type::BMC_DRAGONBOLLPARTY;
						}
						else
						{
							name = "CONY";
							id = Type::CONY_STD;
						}
						break;

					case 85:

						name = "KONAMI VRC7";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::KONAMI_VRC7_1;
						}
						else
						{
							id = Type::KONAMI_VRC7_0;
						}
						break;

					case 86:

						name = "JALECO JF-13";
						id = Type::JALECO_JF13;
						break;

					case 87:

						name = "74*139/74";
						id = Type::DISCRETE_74_139_74;
						break;

					case 88:

						if (nmt == Type::NMT_HORIZONTAL)
						{
							name = "NAMCOT 3433";
							id = Type::NAMCOT_3433;
						}
						else
						{
							name = "NAMCOT 3443";
							id = Type::NAMCOT_3443;
						}
						break;

					case 89:

						name = "SUNSOFT 2 (b)";
						id = Type::SUNSOFT_2B;
						break;

					case 90:

						name = "J.Y.COMPANY (a)";
						id = Type::JYCOMPANY_TYPE_A;
						break;

					case 91:

						name = "UNL MK2/SF3/SMKR";
						id = Type::UNL_MORTALKOMBAT2;
						break;

					case 92:

						name = "JALECO JF-21";
						id = Type::JALECO_JF21;
						break;

					case 93:

						name = "SUNSOFT 2 (a)";
						id = Type::SUNSOFT_2A;
						break;

					case 94:

						name = "UN1ROM";
						id = Type::STD_UN1ROM;
						break;

					case 95:

						name = "NAMCOT 3425";
						id = Type::NAMCOT_3425;
						break;

					case 96:

						name = "BANDAI OEKAKIDS";
						id = Type::BANDAI_OEKAKIDS;
						break;

					case 97:

						name = "IREM KAIKETSU";
						id = Type::IREM_KAIKETSU;
						break;

					case 99:

						name = "VS.SYSTEM";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::CUSTOM_VSSYSTEM_1;
						}
						else
						{
							id = Type::CUSTOM_VSSYSTEM_0;
						}
						break;

					case 103:

						name = "BTL 2708";
						id = Type::BTL_2708;
						break;

					case 104:

						name = "BIC PEGASUS GOLDEN FIVE";
						id = Type::CAMERICA_GOLDENFIVE;
						break;

					case 105:

						name = "EVENT";
						id = Type::CUSTOM_EVENT;
						break;

					case 106:

						name = "BTL SMB3";
						id = Type::BTL_SMB3;
						break;

					case 107:

						name = "MAGICSERIES";
						id = Type::MAGICSERIES_MAGICDRAGON;
						break;

					case 108:

						name = "WHIRLWIND 2706";
						id = Type::WHIRLWIND_2706;
						break;

					case 112:

						name = "NTDEC / ASDER";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::NTDEC_ASDER_1;
						}
						else
						{
							id = Type::NTDEC_ASDER_0;
						}
						break;

					case 113:

						name = "HES";
						id = Type::HES_STD;
						break;

					case 114:

						name = "SUPERGAME LIONKING";
						id = Type::SUPERGAME_LIONKING;
						break;

					case 115:

						name = "KASING";
						id = Type::KASING_STD;
						break;

					case 116:

						name = "SOMERITEAM SL-12";
						id = Type::SOMERITEAM_SL12;
						break;

					case 117:

						name = "FUTUREMEDIA";
						id = Type::FUTUREMEDIA_STD;
						break;

					case 118:

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							name = "TKSROM";
							id = Type::STD_TKSROM;
						}
						else
						{
							name = "TLSROM";
							id = Type::STD_TLSROM;
						}
						break;

					case 119:

						name = "TQROM";
						id = Type::STD_TQROM;
						break;

					case 120:

						name = "BTL TOBIDASE DAISAKUSEN";
						id = Type::BTL_TOBIDASEDAISAKUSEN;
						break;

					case 121:

						name = "K PANDAPRINCE";
						id = Type::KAY_PANDAPRINCE;
						break;

					case 123:

						name = "K H2288";
						id = Type::KAY_H2288;
						break;

					case 126:

						name = "BMC POWERJOY 84-IN-1";
						id = Type::BMC_POWERJOY_84IN1;
						break;

					case 132:

						name = "TXC 22211 (a)";
						id = Type::TXC_22211A;
						break;

					case 133:

						name = "SACHEN SA72008";
						id = Type::SACHEN_SA72008;
						break;

					case 134:

						name = "BMC FAMILY 4646B";
						id = Type::BMC_FAMILY_4646B;
						break;

					case 136:

						name = "SACHEN TCU02";
						id = Type::SACHEN_TCU02;
						break;

					case 137:

						name = "SACHEN 8259D";
						id = Type::SACHEN_8259D;
						break;

					case 138:

						name = "SACHEN 8259B";
						id = Type::SACHEN_8259B;
						break;

					case 139:

						name = "SACHEN 8259C";
						id = Type::SACHEN_8259C;
						break;

					case 140:

						name = "JALECO JF-11/12/14";
						id = Type::JALECO_JF14;
						break;

					case 141:

						name = "SACHEN 8259A";
						id = Type::SACHEN_8259A;
						break;

					case 142:

						name = "KAISER KS-7032";
						id = Type::KAISER_KS7032;
						break;

					case 143:

						name = "SACHEN TCA01";
						id = Type::SACHEN_TCA01;
						break;

					case 144:

						name = "AGCI 50282";
						id = Type::AGCI_50282;
						break;

					case 145:

						name = "SACHEN SA72007";
						id = Type::SACHEN_SA72007;
						break;

					case 146:

						name = "SACHEN SA0161M";
						id = Type::SACHEN_SA0161M;
						break;

					case 147:

						name = "SACHEN TCU01";
						id = Type::SACHEN_TCU01;
						break;

					case 148:

						name = "AVE 74*161 / SACHEN SA0037";
						id = Type::SACHEN_SA0037;
						break;

					case 149:

						name = "SACHEN SA0036";
						id = Type::SACHEN_SA0036;
						break;

					case 150:

						name = "SACHEN 74*374 (b)";
						id = Type::SACHEN_74_374B;
						break;

					case 151:

						name = "KONAMI VS.SYSTEM";
						id = Type::KONAMI_VSSYSTEM;
						break;

					case 152:

						name = "74*161/161/32 (b)";
						id = Type::DISCRETE_74_161_161_32_B;
						break;

					case 154:

						name = "NAMCOT 108/109/118";
						id = Type::NAMCOT_34XX;
						break;

					case 156:

						name = "DAOU 306";
						id = Type::OPENCORP_DAOU306;
						break;

					case 157:

						name = "BANDAI DATACH JOINT SYSTEM";
						id = Type::BANDAI_DATACH;
						break;

					case 158:

						name = "TENGEN 800037";
						id = Type::TENGEN_800037;
						break;

					case 159:

						name = "BANDAI LZ93D50 +24C01";
						id = Type::BANDAI_LZ93D50_24C01;
						break;

					case 163:

						name = "NANJING";
						id = Type::NANJING_STD;
						break;

					case 164:

						name = "WAIXING FFV";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::WAIXING_FFV_1;
						}
						else
						{
							id = Type::WAIXING_FFV_0;
						}
						break;

					case 165:

						name = "WAIXING SH2";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::WAIXING_SH2_1;
						}
						else
						{
							id = Type::WAIXING_SH2_0;
						}
						break;

					case 166:

						name = "SUBOR (b)";
						id = Type::SUBOR_TYPE1;
						break;

					case 167:

						name = "SUBOR (a)";
						id = Type::SUBOR_TYPE0;
						break;

					case 170:

						name = "FUJIYA NROM +SECURITY";
						id = Type::FUJIYA_STD;
						break;

					case 171:

						name = "KAISER KS-7058";
						id = Type::KAISER_KS7058;
						break;

					case 172:

						name = "IDEA-TEK 22211";
						id = Type::TXC_22211B;
						break;

					case 173:

						name = "NEI-HU 22211";
						id = Type::TXC_22211C;
						break;

					case 175:

						name = "KAISER KS-7022";
						id = Type::KAISER_KS7022;
						break;

					case 176:

						name = "XIAO ZHUAN YUAN";
						id = Type::UNL_XZY;
						break;

					case 178:

						name = "WAIXING SAN GUO ZHONG LIE ZHUAN";
						id = Type::WAIXING_SGZLZ;
						break;

					case 180:

						name = "NIHON UNROM M5";
						id = Type::NIHON_UNROM_M5;
						break;

					case 182:

						name = "HOSENKAN ELECTRONICS";
						id = Type::HOSENKAN_STD;
						break;

					case 183:

						name = "BTL SHUI GUAN PIPE";
						id = Type::BTL_SHUIGUANPIPE;
						break;

					case 184:

						name = "SUNSOFT 1";
						id = Type::SUNSOFT_1;
						break;

					case 186:

						name = "SBX";
						id = Type::FUKUTAKE_SBX;
						break;

					case 187:

						name = "UNL SF3/KOF96";
						id = Type::UNL_KINGOFFIGHTERS96;
						break;

					case 188:

						name = "BANDAI KARAOKESTUDIO";
						id = Type::BANDAI_KARAOKESTUDIO;
						break;

					case 189:

						name = "YOKOSOFT / TXC";
						id = Type::TXC_TW;
						break;

					case 191:

						name = "WAIXING (b)";
						id = Type::WAIXING_TYPE_B;
						break;

					case 192:

						name = "WAIXING (c)";
						id = Type::WAIXING_TYPE_C;
						break;

					case 193:

						name = "NTDEC / MEGA SOFT";
						id = Type::NTDEC_FIGHTINGHERO;
						break;

					case 194:

						name = "WAIXING (d)";
						id = Type::WAIXING_TYPE_D;
						break;

					case 195:

						name = "WAIXING (e)";
						id = Type::WAIXING_TYPE_E;
						break;

					case 196:

						name = "BTL SUPER BROS 11";
						id = Type::BTL_SUPERBROS11;
						break;

					case 197:

						name = "HD-1047-A/AX5202P";
						id = Type::UNL_SUPERFIGHTER3;
						break;

					case 198:

						name = "WAIXING (f)";
						id = Type::WAIXING_TYPE_F;
						break;

					case 199:

						name = "WAIXING (g)";
						id = Type::WAIXING_TYPE_G;
						break;

					case 200:

						name = "BMC 1200/36-IN-1";
						id = Type::BMC_36IN1;
						break;

					case 201:

						name = "BMC 21/8-IN-1";
						id = Type::BMC_21IN1;
						break;

					case 202:

						name = "BMC 150-IN-1";
						id = Type::BMC_150IN1;
						break;

					case 203:

						name = "BMC 35-IN-1";
						id = Type::BMC_35IN1;
						break;

					case 204:

						name = "BMC 64-IN-1";
						id = Type::BMC_64IN1;
						break;

					case 205:

						name = "BMC 15/3-IN-1";
						id = Type::BMC_15IN1;
						break;

					case 206:

						if (nmt == Type::NMT_FOURSCREEN)
						{
							name = "DRROM";
							id = Type::STD_DRROM;
						}
						else if (prg >= SIZE_128K)
						{
							name = "DE1ROM / NAMCOT 34xx";
							id = Type::STD_DE1ROM;
						}
						else
						{
							name = "DEROM / TENGEN MIMIC-1 / NAMCOT 34xx";
							id = Type::STD_DEROM;
						}
						break;

					case 207:
					{
						Chips::Type& chip = chips.Add(L"X1-005");

						chip.Pin(17) = L"CIRAM A10";
						chip.Pin(31) = L"NC";

						name = "TAITO X1-005 (b)";
						id = Type::TAITO_X1005;
						break;
					}

					case 208:

						name = "GOUDER 37017";
						id = Type::GOUDER_37017;
						break;

					case 209:

						name = "J.Y.COMPANY (b)";
						id = Type::JYCOMPANY_TYPE_B;
						break;

					case 211:

						name = "J.Y.COMPANY (c)";
						id = Type::JYCOMPANY_TYPE_C;
						break;

					case 212:

						name = "BMC SUPERHIK 300-IN-1";
						id = Type::BMC_SUPERHIK_300IN1;
						break;

					case 213:

						name = "BMC 9999999-IN-1";
						id = Type::BMC_9999999IN1;
						break;

					case 214:

						name = "BMC SUPERGUN 20-IN-1";
						id = Type::BMC_SUPERGUN_20IN1;
						break;

					case 215:

						if (prg == SIZE_256K && chr == SIZE_512K)
						{
							name = "SUPERGAME MK3E";
							id = Type::SUPERGAME_MK3E;
						}
						else
						{
							name = "SUPERGAME BOOGERMAN";
							id = Type::SUPERGAME_BOOGERMAN;
						}
						break;

					case 216:

						name = "RCM GS-2015";
						id = Type::RCM_GS2015;
						break;

					case 217:

						name = "BMC SPC009";
						id = Type::BMC_GOLDENCARD_6IN1;
						break;

					case 222:

						name = "BTL DRAGON NINJA";
						id = Type::BTL_DRAGONNINJA;
						break;

					case 223:

						name = "WAIXING TANG MU LI XIAN JI";
						id = Type::WAIXING_TYPE_I;
						break;

					case 224:

						name = "WAIXING YING XIONG CHUAN QI";
						id = Type::WAIXING_TYPE_J;
						break;

					case 225:

						name = "BMC 58/64/72-IN-1";
						id = Type::BMC_72IN1;
						break;

					case 226:

						if (prg == SIZE_1024K)
						{
							name = "BMC SUPER 42-IN-1";
							id = Type::BMC_SUPER_42IN1;
						}
						else
						{
							name = "BMC 76-IN-1";
							id = Type::BMC_76IN1;
						}
						break;

					case 227:

						name = "BMC 1200-IN-1";
						id = Type::BMC_1200IN1;
						break;

					case 228:

						name = "ACTIVE ENTERTAINMENT";
						id = Type::AE_STD;
						break;

					case 229:

						name = "BMC 31-IN-1";
						id = Type::BMC_31IN1;
						break;

					case 230:

						name = "BMC SUPER 22 GAMES / 20-IN-1";
						id = Type::BMC_22GAMES;
						break;

					case 231:

						name = "BMC 20-IN-1";
						id = Type::BMC_20IN1;
						break;

					case 232:

						name = "CAMERICA BF9096";
						id = Type::CAMERICA_BF9096;
						break;

					case 233:

						name = "BMC SUPER 22 GAMES";
						id = Type::BMC_SUPER_22GAMES;
						break;

					case 234:

						name = "AVE D-1012";
						id = Type::AVE_D1012;
						break;

					case 235:

						if (prg <= SIZE_2048K)
						{
							name = "GOLDENGAME 150-IN-1";
							id = Type::BMC_GOLDENGAME_150IN1;
						}
						else
						{
							name = "GOLDENGAME 260-IN-1";
							id = Type::BMC_GOLDENGAME_260IN1;
						}
						break;

					case 236:

						name = "BMC 70/800-IN-1";
						id = Type::BMC_GAME_800IN1;
						break;

					case 240:

						name = "CNE SHLZ";
						id = Type::CNE_SHLZ;
						break;

					case 241:

						name = "MXMDHTWO / TXC";
						id = Type::TXC_MXMDHTWO;
						break;

					case 242:

						if (nmt == Type::NMT_CONTROLLED)
						{
							name = "WAIXING ZS";
							id = Type::WAIXING_ZS;
						}
						else
						{
							name = "WAIXING DQVII";
							id = Type::WAIXING_DQVII;
						}
						break;

					case 243:

						name = "SACHEN 74*374 (a)";
						id = Type::SACHEN_74_374A;
						break;

					case 244:

						name = "CNE DECATHLON";
						id = Type::CNE_DECATHLON;
						break;

					case 245:

						name = "WAIXING (h)";
						id = Type::WAIXING_TYPE_H;
						break;

					case 246:

						name = "CNE PHONE SERM BERM";
						id = Type::CNE_PSB;
						break;

					case 249:

						name = "WAIXING +SECURITY";

						if (wram || useWramAuto)
						{
							wramAuto = useWramAuto;
							id = Type::WAIXING_SECURITY_1;
						}
						else
						{
							id = Type::WAIXING_SECURITY_0;
						}
						break;

					case 250:

						name = "NITRA";
						id = Type::NITRA_TDA;
						break;

					case 252:

						name = "WAIXING SAN GUO ZHI";
						id = Type::WAIXING_SGZ;
						break;

					case 254:

						name = "BTL PIKACHU Y2K";
						id = Type::BTL_PIKACHUY2K;
						break;

					case 255:

						name = "BMC 110/115-IN-1";
						id = Type::BMC_110IN1;
						break;

					default:

						return false;
				}

				type = Type( id, this->prg, this->chr, nmt, wramBattery || mmcBattery, wramAuto );

				return true;
			}

			Board* Board::Create(const Context& c)
			{
				switch (c.type.GetId())
				{
					case Type::STD_NROM                   :
					case Type::UNL_NROM                   : return new NRom(c);
					case Type::STD_AMROM                  :
					case Type::STD_ANROM                  :
					case Type::STD_AN1ROM                 :
					case Type::STD_AOROM                  :
					case Type::UNL_AXROM                  : return new AxRom(c);
					case Type::STD_BNROM                  :
					case Type::UNL_BXROM                  : return new BxRom(c);
					case Type::STD_CNROM                  :
					case Type::STD_CXROM                  :
					case Type::UNL_CXROM                  : return new CnRom(c);
					case Type::STD_CPROM                  : return new CpRom(c);
					case Type::STD_DEROM                  :
					case Type::STD_DE1ROM                 :
					case Type::STD_DRROM                  : return new DxRom(c);
					case Type::STD_EKROM                  :
					case Type::STD_ELROM                  :
					case Type::STD_ETROM                  :
					case Type::STD_EWROM                  :
					case Type::STD_EXROM_0                :
					case Type::STD_EXROM_1                :
					case Type::STD_EXROM_2                :
					case Type::STD_EXROM_3                :
					case Type::STD_EXROM_4                :
					case Type::STD_EXROM_5                : return new ExRom(c);
					case Type::STD_FJROM                  :
					case Type::STD_FKROM                  : return new FxRom(c);
					case Type::STD_GNROM                  :
					case Type::UNL_GXROM                  : return new GxRom(c);
					case Type::STD_MHROM                  : return new MxRom(c);
					case Type::STD_HKROM                  : return new HxRom(c);
					case Type::STD_JLROM                  :
					case Type::STD_JSROM                  : return new JxRom(c);
					case Type::STD_NTBROM                 : return new NxRom(c);
					case Type::STD_PEEOROM                :
					case Type::STD_PNROM                  :
					case Type::STD_PNROM_PC10             : return new PxRom(c);
					case Type::STD_SAROM                  :
					case Type::STD_SBROM                  :
					case Type::STD_SCROM                  :
					case Type::STD_SEROM                  :
					case Type::STD_SFROM                  :
					case Type::STD_SGROM                  :
					case Type::STD_SHROM                  :
					case Type::STD_SJROM                  :
					case Type::STD_SKROM                  :
					case Type::STD_SLROM                  :
					case Type::STD_SNROM                  :
					case Type::STD_SOROM                  :
					case Type::STD_SUROM                  :
					case Type::STD_SXROM                  : return new SxRom(c);
					case Type::STD_TBROM                  :
					case Type::STD_TEROM                  :
					case Type::STD_TFROM                  :
					case Type::STD_TGROM                  :
					case Type::STD_TKROM                  :
					case Type::STD_TLROM                  :
					case Type::STD_TNROM                  :
					case Type::STD_TR1ROM                 :
					case Type::STD_TSROM                  :
					case Type::STD_TVROM                  :
					case Type::UNL_TRXROM                 : return new TxRom  (c);
					case Type::STD_TLSROM                 : return new TlsRom (c);
					case Type::STD_TKSROM                 : return new TksRom (c);
					case Type::STD_TQROM                  : return new TqRom  (c);
					case Type::STD_UNROM                  :
					case Type::STD_UN1ROM                 :
					case Type::STD_UOROM                  :
					case Type::STD_UXROM                  :
					case Type::UNL_UXROM                  : return new UxRom(c);
					case Type::DISCRETE_74_377            : return new Discrete::Ic74x377(c);
					case Type::DISCRETE_74_139_74         : return new Discrete::Ic74x139x74(c);
					case Type::DISCRETE_74_161_138        : return new Discrete::Ic74x161x138(c);
					case Type::DISCRETE_74_161_161_32_A   :
					case Type::DISCRETE_74_161_161_32_B   : return new Discrete::Ic74x161x161x32(c);
					case Type::CUSTOM_B4                  : return new TxRom(c);
					case Type::CUSTOM_BTR                 : return new JxRom(c);
					case Type::CUSTOM_EVENT               : return new Boards::Event(c);
					case Type::CUSTOM_FFE3                :
					case Type::CUSTOM_FFE4                :
					case Type::CUSTOM_FFE8                : return new Ffe(c);
					case Type::CUSTOM_FB02                :
					case Type::CUSTOM_FB04                : return new Fb(c);
					case Type::CUSTOM_RUMBLESTATION       : return new RumbleStation(c);
					case Type::CUSTOM_QJ                  : return new Qj(c);
					case Type::CUSTOM_VSSYSTEM_0          :
					case Type::CUSTOM_VSSYSTEM_1          : return new VsSystem(c);
					case Type::CUSTOM_WH                  : return new SxRom(c);
					case Type::CUSTOM_X79B                : return new CnRom(c);
					case Type::CUSTOM_ZZ                  : return new Zz(c);
					case Type::AE_STD                     : return new Ae::Standard(c);
					case Type::AGCI_50282                 : return new Agci::A50282(c);
					case Type::AVE_MB_91                  : return new Ave::Mb91(c);
					case Type::AVE_NINA001                : return new Ave::Nina001(c);
					case Type::AVE_NINA002                : return new Ave::Nina002(c);
					case Type::AVE_NINA03                 : return new Ave::Nina03(c);
					case Type::AVE_NINA06                 : return new Ave::Nina06(c);
					case Type::AVE_NINA07                 : return new Ave::Nina07(c);
					case Type::AVE_D1012                  : return new Ave::D1012(c);
					case Type::BANDAI_FCG1                : return new Bandai::Fcg1(c);
					case Type::BANDAI_FCG2                : return new Bandai::Fcg2(c);
					case Type::BANDAI_BAJUMP2             : return new Bandai::Lz93d50(c);
					case Type::BANDAI_LZ93D50_24C01       :
					case Type::BANDAI_LZ93D50_24C02       : return new Bandai::Lz93d50Ex(c);
					case Type::BANDAI_DATACH              : return new Bandai::Datach(c);
					case Type::BANDAI_KARAOKESTUDIO       : return new Bandai::KaraokeStudio(c);
					case Type::BANDAI_AEROBICSSTUDIO      : return new Bandai::AerobicsStudio(c);
					case Type::BANDAI_OEKAKIDS            : return new Bandai::OekaKids(c);
					case Type::BMC_BALLGAMES_11IN1        : return new Bmc::Ballgames11in1(c);
					case Type::BMC_A65AS                  : return new Bmc::A65as(c);
					case Type::BMC_CTC65                  : return new Bmc::Ctc65(c);
					case Type::BMC_DRAGONBOLLPARTY        : return new Cony::Standard(c);
					case Type::BMC_110IN1                 : return new Bmc::B110in1(c);
					case Type::BMC_1200IN1                : return new Bmc::B1200in1(c);
					case Type::BMC_150IN1                 : return new Bmc::B150in1(c);
					case Type::BMC_15IN1                  : return new Bmc::B15in1(c);
					case Type::BMC_20IN1                  : return new Bmc::B20in1(c);
					case Type::BMC_21IN1                  : return new Bmc::B21in1(c);
					case Type::BMC_22GAMES                : return new Bmc::B22Games(c);
					case Type::BMC_31IN1                  : return new Bmc::B31in1(c);
					case Type::BMC_35IN1                  : return new Bmc::B35in1(c);
					case Type::BMC_36IN1                  : return new Bmc::B36in1(c);
					case Type::BMC_64IN1                  : return new Bmc::B64in1(c);
					case Type::BMC_72IN1                  : return new Bmc::B72in1(c);
					case Type::BMC_76IN1                  :
					case Type::BMC_SUPER_42IN1            : return new Bmc::B76in1(c);
					case Type::BMC_8157                   : return new Bmc::B8157(c);
					case Type::BMC_9999999IN1             : return new Bmc::B9999999in1(c);
					case Type::BMC_FAMILY_4646B           : return new Bmc::Family4646B(c);
					case Type::BMC_FKC23C                 : return new Bmc::Fk23c(c);
					case Type::BMC_GAME_800IN1            : return new Bmc::Game800in1(c);
					case Type::BMC_GOLDEN_190IN1          : return new Bmc::Golden190in1(c);
					case Type::BMC_GOLDENGAME_150IN1      :
					case Type::BMC_GOLDENGAME_260IN1      : return new Bmc::GoldenGame260in1(c);
					case Type::BMC_GOLDENCARD_6IN1        : return new Bmc::GoldenCard6in1(c);
					case Type::BMC_GKA                    : return new Bmc::GamestarA(c);
					case Type::BMC_GKB                    : return new Bmc::GamestarB(c);
					case Type::BMC_HERO                   : return new Bmc::Hero(c);
					case Type::BMC_MARIOPARTY_7IN1        : return new Bmc::MarioParty7in1(c);
					case Type::BMC_NOVELDIAMOND           : return new Bmc::NovelDiamond(c);
					case Type::BMC_CH001                  : return new Bmc::Ch001(c);
					case Type::BMC_POWERJOY_84IN1         : return new Bmc::Powerjoy84in1(c);
					case Type::BMC_RESETBASED_4IN1        : return new Bmc::ResetBased4in1(c);
					case Type::BMC_VT5201                 : return new Bmc::Vt5201(c);
					case Type::BMC_SUPER_24IN1            : return new Bmc::Super24in1(c);
					case Type::BMC_SUPER_22GAMES          : return new Bmc::Super22Games(c);
					case Type::BMC_SUPER_40IN1            : return new Bmc::Super40in1(c);
					case Type::BMC_SUPER_700IN1           : return new Bmc::Super700in1(c);
					case Type::BMC_SUPERBIG_7IN1          : return new Bmc::SuperBig7in1(c);
					case Type::BMC_SUPERGUN_20IN1         : return new Bmc::SuperGun20in1(c);
					case Type::BMC_SUPERHIK_4IN1          : return new Bmc::SuperHiK4in1(c);
					case Type::BMC_SUPERHIK_300IN1        : return new Bmc::SuperHiK300in1(c);
					case Type::BMC_SUPERVISION_16IN1      : return new Bmc::SuperVision16in1(c);
					case Type::BMC_T262                   : return new Bmc::T262(c);
					case Type::BMC_VRC4                   : return new Bmc::Vrc4(c);
					case Type::BMC_Y2K_64IN1              : return new Bmc::Y2k64in1(c);
					case Type::BTL_AISENSHINICOL          :
					case Type::BTL_MARIOBABY              : return new Btl::MarioBaby(c);
					case Type::BTL_2708                   : return new Btl::B2708(c);
					case Type::BTL_AX5705                 : return new Btl::Ax5705(c);
					case Type::BTL_6035052                : return new Btl::B6035052(c);
					case Type::BTL_DRAGONNINJA            : return new Btl::DragonNinja(c);
					case Type::BTL_GENIUSMERIOBROS        : return new Btl::GeniusMerioBros(c);
					case Type::BTL_SHUIGUANPIPE           : return new Btl::ShuiGuanPipe(c);
					case Type::BTL_PIKACHUY2K             : return new Btl::PikachuY2k(c);
					case Type::BTL_SMB2_A                 : return new Btl::Smb2a(c);
					case Type::BTL_SMB2_B                 : return new Btl::Smb2b(c);
					case Type::BTL_SMB2_C                 : return new Btl::Smb2c(c);
					case Type::BTL_SMB3                   : return new Btl::Smb3(c);
					case Type::BTL_SUPERBROS11            : return new Btl::SuperBros11(c);
					case Type::BTL_T230                   : return new Btl::T230(c);
					case Type::BTL_TOBIDASEDAISAKUSEN     : return new Btl::TobidaseDaisakusen(c);
					case Type::CAMERICA_BF9093            : return new Camerica::Bf9093(c);
					case Type::CAMERICA_BF9096            : return new Camerica::Bf9096(c);
					case Type::CAMERICA_BF9097            :
					case Type::CAMERICA_BF909X            : return new Camerica::Bf9097(c);
					case Type::CAMERICA_ALGNV11           : return new Camerica::Algnv11(c);
					case Type::CAMERICA_ALGQV11           : return new Camerica::Algqv11(c);
					case Type::CAMERICA_GOLDENFIVE        : return new Camerica::GoldenFive(c);
					case Type::CALTRON_6IN1               : return new Caltron::Mc6in1(c);
					case Type::CNE_SHLZ                   : return new Cne::Shlz(c);
					case Type::CNE_DECATHLON              : return new Cne::Decathlon(c);
					case Type::CNE_PSB                    : return new Cne::Psb(c);
					case Type::CONY_STD                   : return new Cony::Standard(c);
					case Type::DREAMTECH_01               : return new DreamTech::D01(c);
					case Type::FUTUREMEDIA_STD            : return new FutureMedia::Standard(c);
					case Type::FUJIYA_STD                 : return new Fujiya::Standard(c);
					case Type::FUKUTAKE_SBX               : return new Fukutake::Sbx(c);
					case Type::GOUDER_37017               : return new Gouder::G37017(c);
					case Type::HES_STD                    : return new Hes::Standard(c);
					case Type::BENSHENG_BS5               : return new Bensheng::Bs5(c);
					case Type::HOSENKAN_STD               : return new Hosenkan::Standard(c);
					case Type::IREM_G101A_0               :
					case Type::IREM_G101A_1               :
					case Type::IREM_G101B_0               :
					case Type::IREM_G101B_1               : return new Irem::G101(c);
					case Type::IREM_H3001                 : return new Irem::H3001(c);
					case Type::IREM_LROG017               : return new Irem::Lrog017(c);
					case Type::IREM_HOLYDIVER             : return new Irem::HolyDiver(c);
					case Type::IREM_KAIKETSU              : return new Irem::Kaiketsu(c);
					case Type::JALECO_JF01                : return new Jaleco::Jf01(c);
					case Type::JALECO_JF02                : return new Jaleco::Jf02(c);
					case Type::JALECO_JF03                : return new Jaleco::Jf03(c);
					case Type::JALECO_JF04                : return new Jaleco::Jf04(c);
					case Type::JALECO_JF05                : return new Jaleco::Jf05(c);
					case Type::JALECO_JF06                : return new Jaleco::Jf06(c);
					case Type::JALECO_JF07                : return new Jaleco::Jf07(c);
					case Type::JALECO_JF08                : return new Jaleco::Jf08(c);
					case Type::JALECO_JF09                : return new Jaleco::Jf09(c);
					case Type::JALECO_JF10                : return new Jaleco::Jf10(c);
					case Type::JALECO_JF11                : return new Jaleco::Jf11(c);
					case Type::JALECO_JF12                : return new Jaleco::Jf12(c);
					case Type::JALECO_JF13                : return new Jaleco::Jf13(c);
					case Type::JALECO_JF14                : return new Jaleco::Jf14(c);
					case Type::JALECO_JF15                : return new Jaleco::Jf15(c);
					case Type::JALECO_JF16                : return new Jaleco::Jf16(c);
					case Type::JALECO_JF17                : return new Jaleco::Jf17(c);
					case Type::JALECO_JF18                : return new Jaleco::Jf18(c);
					case Type::JALECO_JF19                : return new Jaleco::Jf19(c);
					case Type::JALECO_JF20                : return new Jaleco::Jf20(c);
					case Type::JALECO_JF21                : return new Jaleco::Jf21(c);
					case Type::JALECO_JF22                : return new Jaleco::Jf22(c);
					case Type::JALECO_JF23                : return new Jaleco::Jf23(c);
					case Type::JALECO_JF24                : return new Jaleco::Jf24(c);
					case Type::JALECO_JF25                : return new Jaleco::Jf25(c);
					case Type::JALECO_JF26                : return new Jaleco::Jf26(c);
					case Type::JALECO_JF27                : return new Jaleco::Jf27(c);
					case Type::JALECO_JF28                : return new Jaleco::Jf28(c);
					case Type::JALECO_JF29                : return new Jaleco::Jf29(c);
					case Type::JALECO_JF30                : return new Jaleco::Jf30(c);
					case Type::JALECO_JF31                : return new Jaleco::Jf31(c);
					case Type::JALECO_JF32                : return new Jaleco::Jf32(c);
					case Type::JALECO_JF33                : return new Jaleco::Jf33(c);
					case Type::JALECO_JF34                : return new Jaleco::Jf34(c);
					case Type::JALECO_JF35                : return new Jaleco::Jf35(c);
					case Type::JALECO_JF36                : return new Jaleco::Jf36(c);
					case Type::JALECO_JF37                : return new Jaleco::Jf37(c);
					case Type::JALECO_JF38                : return new Jaleco::Jf38(c);
					case Type::JALECO_JF39                : return new Jaleco::Jf39(c);
					case Type::JALECO_JF40                : return new Jaleco::Jf40(c);
					case Type::JALECO_JF41                : return new Jaleco::Jf41(c);
					case Type::JALECO_SS88006             : return new Jaleco::Ss88006(c);
					case Type::JYCOMPANY_TYPE_A           :
					case Type::JYCOMPANY_TYPE_B           :
					case Type::JYCOMPANY_TYPE_C           : return new JyCompany::Standard(c);
					case Type::KAISER_KS202               : return new Kaiser::Ks202(c);
					case Type::KAISER_KS7022              : return new Kaiser::Ks7022(c);
					case Type::KAISER_KS7032              : return new Kaiser::Ks7032(c);
					case Type::KAISER_KS7058              : return new Kaiser::Ks7058(c);
					case Type::KASING_STD                 : return new Kasing::Standard(c);
					case Type::KAY_H2288                  : return new Kay::H2288(c);
					case Type::KAY_PANDAPRINCE            : return new Kay::PandaPrince(c);
					case Type::KONAMI_VRC1                : return new Konami::Vrc1(c);
					case Type::KONAMI_VRC2                : return new Konami::Vrc2(c);
					case Type::KONAMI_VRC3                : return new Konami::Vrc3(c);
					case Type::KONAMI_VRC4_0              :
					case Type::KONAMI_VRC4_1              :
					case Type::KONAMI_VRC4_2              : return new Konami::Vrc4(c);
					case Type::KONAMI_VRC6_0              :
					case Type::KONAMI_VRC6_1              : return new Konami::Vrc6(c);
					case Type::KONAMI_VRC7_0              :
					case Type::KONAMI_VRC7_1              : return new Konami::Vrc7(c);
					case Type::KONAMI_VSSYSTEM            : return new Konami::VsSystem(c);
					case Type::MAGICSERIES_MAGICDRAGON    : return new MagicSeries::MagicDragon(c);
					case Type::NAMCOT_3425                : return new Namcot::N3425(c);
					case Type::NAMCOT_3433                : return new Namcot::N3433(c);
					case Type::NAMCOT_3443                : return new Namcot::N3443(c);
					case Type::NAMCOT_3446                : return new Namcot::N3446(c);
					case Type::NAMCOT_34XX                : return new Namcot::N34xx(c);
					case Type::NAMCOT_163_0               :
					case Type::NAMCOT_163_1               :
					case Type::NAMCOT_163_S_0             :
					case Type::NAMCOT_163_S_1             : return new Namcot::N163(c);
					case Type::NANJING_STD                : return new Nanjing::Standard(c);
					case Type::NIHON_UNROM_M5             : return new Nihon::UnRomM5(c);
					case Type::NITRA_TDA                  : return new Nitra::Tda(c);
					case Type::NTDEC_N715062              : return new Ntdec::N715062(c);
					case Type::NTDEC_ASDER_0              :
					case Type::NTDEC_ASDER_1              : return new Ntdec::Asder(c);
					case Type::NTDEC_FIGHTINGHERO         : return new Ntdec::FightingHero(c);
					case Type::OPENCORP_DAOU306           : return new OpenCorp::Daou306(c);
					case Type::REXSOFT_SL1632             : return new RexSoft::Sl1632(c);
					case Type::REXSOFT_DBZ5               : return new RexSoft::Dbz5(c);
					case Type::RCM_GS2013                 : return new Rcm::Gs2013(c);
					case Type::RCM_GS2015                 : return new Rcm::Gs2015(c);
					case Type::RCM_GS2004                 : return new Rcm::Gs2004(c);
					case Type::RCM_TETRISFAMILY           : return new Rcm::TetrisFamily(c);
					case Type::SACHEN_8259A               :
					case Type::SACHEN_8259B               :
					case Type::SACHEN_8259C               :
					case Type::SACHEN_8259D               : return new Sachen::S8259(c);
					case Type::SACHEN_TCA01               : return new Sachen::Tca01(c);
					case Type::SACHEN_TCU01               : return new Sachen::Tcu01(c);
					case Type::SACHEN_TCU02               : return new Sachen::Tcu02(c);
					case Type::SACHEN_SA0036              : return new Sachen::Sa0036(c);
					case Type::SACHEN_SA0037              : return new Sachen::Sa0037(c);
					case Type::SACHEN_SA0161M             : return new Sachen::Sa0161m(c);
					case Type::SACHEN_SA72007             : return new Sachen::Sa72007(c);
					case Type::SACHEN_SA72008             : return new Sachen::Sa72008(c);
					case Type::SACHEN_74_374A             : return new Sachen::S74x374a(c);
					case Type::SACHEN_74_374B             : return new Sachen::S74x374b(c);
					case Type::SACHEN_STREETHEROES        : return new Sachen::StreetHeroes(c);
					case Type::SOMERITEAM_SL12            : return new SomeriTeam::Sl12(c);
					case Type::SUBOR_TYPE0                : return new Subor::Type0(c);
					case Type::SUBOR_TYPE1                : return new Subor::Type1(c);
					case Type::SUBOR_STUDYNGAME           : return new Subor::StudyNGame(c);
					case Type::SUNSOFT_1                  : return new Sunsoft::S1(c);
					case Type::SUNSOFT_2A                 : return new Sunsoft::S2a(c);
					case Type::SUNSOFT_2B                 : return new Sunsoft::S2b(c);
					case Type::SUNSOFT_3                  : return new Sunsoft::S3(c);
					case Type::SUNSOFT_4_0                :
					case Type::SUNSOFT_4_1                : return new Sunsoft::S4(c);
					case Type::SUNSOFT_5B_0               :
					case Type::SUNSOFT_5B_1               : return new Sunsoft::S5b(c);
					case Type::SUNSOFT_DCS                : return new Sunsoft::Dcs(c);
					case Type::SUNSOFT_FME7_0             :
					case Type::SUNSOFT_FME7_1             : return new Sunsoft::Fme7(c);
					case Type::SUPERGAME_LIONKING         : return new SuperGame::LionKing(c);
					case Type::SUPERGAME_BOOGERMAN        : return new SuperGame::Boogerman(c);
					case Type::SUPERGAME_MK3E             : return new SuperGame::Mk3e(c);
					case Type::SUPERGAME_POCAHONTAS2      : return new SuperGame::Pocahontas2(c);
					case Type::TAITO_TC0190FMC            : return new Taito::Tc0190fmc(c);
					case Type::TAITO_TC0190FMC_PAL16R4    : return new Taito::Tc0190fmcPal16r4(c);
					case Type::TAITO_X1005                : return new Taito::X1005(c);
					case Type::TAITO_X1017                : return new Taito::X1017(c);
					case Type::TENGEN_800002              : return new Tengen::T800002(c);
					case Type::TENGEN_800004              : return new Tengen::T800004(c);
					case Type::TENGEN_800008              : return new Tengen::T800008(c);
					case Type::TENGEN_800030              : return new Tengen::T800030(c);
					case Type::TENGEN_800032              : return new Tengen::T800032(c);
					case Type::TENGEN_800037              : return new Tengen::T800037(c);
					case Type::TENGEN_800042              : return new Tengen::T800042(c);
					case Type::TXC_22211A                 : return new Txc::T22211A(c);
					case Type::TXC_22211B                 : return new Txc::T22211B(c);
					case Type::TXC_22211C                 : return new Txc::T22211C(c);
					case Type::TXC_MXMDHTWO               : return new Txc::Mxmdhtwo(c);
					case Type::TXC_POLICEMAN              : return new Txc::Policeman(c);
					case Type::TXC_TW                     : return new Txc::Tw(c);
					case Type::UNL_CC21                   : return new Unlicensed::Cc21(c);
					case Type::UNL_EDU2000                : return new Unlicensed::Edu2000(c);
					case Type::UNL_KINGOFFIGHTERS96       : return new Unlicensed::KingOfFighters96(c);
					case Type::UNL_KINGOFFIGHTERS97       : return new Unlicensed::KingOfFighters97(c);
					case Type::UNL_MORTALKOMBAT2          : return new Unlicensed::MortalKombat2(c);
					case Type::UNL_SUPERFIGHTER3          : return new Unlicensed::SuperFighter3(c);
					case Type::UNL_TF1201                 : return new Unlicensed::Tf1201(c);
					case Type::UNL_WORLDHERO              : return new Unlicensed::WorldHero(c);
					case Type::UNL_XZY                    : return new Unlicensed::Xzy(c);
					case Type::WAIXING_PS2_0              :
					case Type::WAIXING_PS2_1              : return new Waixing::Ps2(c);
					case Type::WAIXING_TYPE_A             : return new Waixing::TypeA(c);
					case Type::WAIXING_TYPE_B             : return new Waixing::TypeB(c);
					case Type::WAIXING_TYPE_C             : return new Waixing::TypeC(c);
					case Type::WAIXING_TYPE_D             : return new Waixing::TypeD(c);
					case Type::WAIXING_TYPE_E             : return new Waixing::TypeE(c);
					case Type::WAIXING_TYPE_F             : return new Waixing::TypeF(c);
					case Type::WAIXING_TYPE_G             : return new Waixing::TypeG(c);
					case Type::WAIXING_TYPE_H             : return new Waixing::TypeH(c);
					case Type::WAIXING_TYPE_I             : return new Waixing::TypeI(c);
					case Type::WAIXING_TYPE_J             : return new Waixing::TypeJ(c);
					case Type::WAIXING_FFV_0              :
					case Type::WAIXING_FFV_1              : return new Waixing::Ffv(c);
					case Type::WAIXING_SH2_0              :
					case Type::WAIXING_SH2_1              : return new Waixing::Sh2(c);
					case Type::WAIXING_ZS                 : return new Waixing::Zs(c);
					case Type::WAIXING_DQVII              : return new Waixing::Dqv7(c);
					case Type::WAIXING_SGZ                : return new Waixing::Sgz(c);
					case Type::WAIXING_SGZLZ              : return new Waixing::Sgzlz(c);
					case Type::WAIXING_SECURITY_0         :
					case Type::WAIXING_SECURITY_1         : return new Waixing::Security(c);
					case Type::WHIRLWIND_2706             : return new Whirlwind::W2706(c);
				}

				return NULL;
			}

			void Board::Destroy(Board* board)
			{
				delete board;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}
