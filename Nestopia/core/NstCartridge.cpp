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

#include <cstdlib>
#include <cstring>
#include "NstLog.hpp"
#include "NstChecksum.hpp"
#include "NstImageDatabase.hpp"
#include "board/NstBoard.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeRomset.hpp"
#include "NstCartridgeInes.hpp"
#include "NstCartridgeUnif.hpp"
#include "vssystem/NstVsSystem.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Cartridge::ProfileEx::ProfileEx()
		: nmt(NMT_DEFAULT), battery(false), wramAuto(false) {}

		Cartridge::Cartridge(Context& context)
		: Image(CARTRIDGE), board(NULL), vs(NULL)
		{
			try
			{
				ProfileEx profileEx;

				switch (Stream::In(&context.stream).Peek32())
				{
					case INES_ID:

						Ines::Load
						(
							context.stream,
							context.patch,
							context.patchBypassChecksum,
							context.patchResult,
							prg,
							chr,
							context.favoredSystem,
							profile,
							profileEx,
							context.database
						);
						break;

					case UNIF_ID:

						Unif::Load
						(
							context.stream,
							context.patch,
							context.patchBypassChecksum,
							context.patchResult,
							prg,
							chr,
							context.favoredSystem,
							profile,
							profileEx,
							context.database
						);
						break;

					default:

						Romset::Load
						(
							context.stream,
							context.patch,
							context.patchBypassChecksum,
							context.patchResult,
							prg,
							chr,
							context.favoredSystem,
							context.askProfile,
							profile
						);
						break;
				}

				if (profile.dump.state == Profile::Dump::BAD)
					context.result = RESULT_WARN_BAD_DUMP;
				else
					context.result = RESULT_OK;

				const Result result = SetupBoard( prg, chr, &board, &context, profile, profileEx, &prgCrc );

				if (NES_FAILED(result))
					throw result;

				board->Load( savefile );

				switch (profile.system.type)
				{
					case Profile::System::VS_UNISYSTEM:

						vs = VsSystem::Create
						(
							context.cpu,
							context.ppu,
							static_cast<PpuModel>(profile.system.ppu),
							prgCrc
						);

						profile.system.ppu = static_cast<Profile::System::Ppu>(vs->GetPpuModel());
						break;

					case Profile::System::VS_DUALSYSTEM:
						throw RESULT_ERR_UNSUPPORTED_VSSYSTEM;
				}

				if (Cartridge::QueryExternalDevice( EXT_DIP_SWITCHES ))
					Log::Flush( "Cartridge: DIP Switches present" NST_LINEBREAK );
			}
			catch (...)
			{
				Destroy();
				throw;
			}
		}

		void Cartridge::Destroy()
		{
			VsSystem::Destroy( vs );
			Boards::Board::Destroy( board );
		}

		Cartridge::~Cartridge()
		{
			Destroy();
		}

		void Cartridge::ReadRomset(std::istream& stream,FavoredSystem favoredSystem,bool askSystem,Profile& profile)
		{
			Log::Suppressor logSupressor;
			Ram prg, chr;
			ProfileEx profileEx;
			Romset::Load( stream, NULL, false, NULL, prg, chr, favoredSystem, askSystem, profile, true );
			SetupBoard( prg, chr, NULL, NULL, profile, profileEx, NULL, true );
		}

		void Cartridge::ReadInes(std::istream& stream,FavoredSystem favoredSystem,Profile& profile)
		{
			Log::Suppressor logSupressor;
			Ram prg, chr;
			ProfileEx profileEx;
			Ines::Load( stream, NULL, false, NULL, prg, chr, favoredSystem, profile, profileEx, NULL );
			SetupBoard( prg, chr, NULL, NULL, profile, profileEx, NULL );
		}

		void Cartridge::ReadUnif(std::istream& stream,FavoredSystem favoredSystem,Profile& profile)
		{
			Log::Suppressor logSupressor;
			Ram prg, chr;
			ProfileEx profileEx;
			Unif::Load( stream, NULL, false, NULL, prg, chr, favoredSystem, profile, profileEx, NULL );
			SetupBoard( prg, chr, NULL, NULL, profile, profileEx, NULL );
		}

		uint Cartridge::GetDesiredController(uint port) const
		{
			NST_ASSERT( port < Api::Input::NUM_CONTROLLERS );
			return profile.game.controllers[port];
		}

		uint Cartridge::GetDesiredAdapter() const
		{
			return profile.game.adapter;
		}

		Cartridge::ExternalDevice Cartridge::QueryExternalDevice(ExternalDeviceType deviceType)
		{
			switch (deviceType)
			{
				case EXT_DIP_SWITCHES:

					if (vs)
						return &vs->GetDipSwiches();
					else
						return board->QueryDevice( Boards::Board::DEVICE_DIP_SWITCHES );

				case EXT_BARCODE_READER:

					return board->QueryDevice( Boards::Board::DEVICE_BARCODE_READER );

				default:

					return Image::QueryExternalDevice( deviceType );
			}
		}

		Result Cartridge::SetupBoard
		(
			Ram& prg,
			Ram& chr,
			Boards::Board** board,
			const Context* const context,
			Profile& profile,
			const ProfileEx& profileEx,
			dword* const prgCrc,
			const bool readOnly
		)
		{
			NST_ASSERT( bool(board) == bool(context) );

			Boards::Board::Type::Nmt nmt;

			if (profile.board.solderPads & (Profile::Board::SOLDERPAD_H|Profile::Board::SOLDERPAD_V))
			{
				if (profile.board.solderPads & Profile::Board::SOLDERPAD_H)
					nmt = Boards::Board::Type::NMT_VERTICAL;
				else
					nmt = Boards::Board::Type::NMT_HORIZONTAL;
			}
			else switch (profileEx.nmt)
			{
				case ProfileEx::NMT_HORIZONTAL:   nmt = Boards::Board::Type::NMT_HORIZONTAL;   break;
				case ProfileEx::NMT_VERTICAL:     nmt = Boards::Board::Type::NMT_VERTICAL;     break;
				case ProfileEx::NMT_SINGLESCREEN: nmt = Boards::Board::Type::NMT_SINGLESCREEN; break;
				case ProfileEx::NMT_FOURSCREEN:   nmt = Boards::Board::Type::NMT_FOURSCREEN;   break;
				default:                          nmt = Boards::Board::Type::NMT_CONTROLLED;   break;
			}

			Chips chips;

			for (Profile::Board::Chips::const_iterator i(profile.board.chips.begin()), end(profile.board.chips.end()); i != end; ++i)
			{
				Chips::Type& type = chips.Add( i->type.c_str() );

				for (Profile::Board::Pins::const_iterator j(i->pins.begin()), end(i->pins.end()); j != end; ++j)
					type.Pin(j->number) = j->function.c_str();

				for (Profile::Board::Samples::const_iterator j(i->samples.begin()), end(i->samples.end()); j != end; ++j)
					type.Sample(j->id) = j->file.c_str();
			}

			Boards::Board::Context b
			(
				context ? &context->cpu : NULL,
				context ? &context->apu : NULL,
				context ? &context->ppu : NULL,
				prg,
				chr,
				profileEx.trainer,
				nmt,
				profileEx.battery || profile.board.HasWramBattery(),
				profile.board.HasMmcBattery(),
				chips
			);

			if (profile.board.type.empty() || !b.DetectBoard( profile.board.type.c_str(), profile.board.GetWram() ))
			{
				if (profile.board.mapper == Profile::Board::NO_MAPPER || (!b.DetectBoard( profile.board.mapper, profile.board.GetWram(), profileEx.wramAuto ) && board))
					return RESULT_ERR_UNSUPPORTED_MAPPER;

				if (profile.board.type.empty())
				{
					const std::wstring tmp( b.name, b.name + std::strlen(b.name) );
					profile.board.type = tmp;
				}
			}

			for (uint i=0; i < 2; ++i)
			{
				dword size = (i ? b.chr : b.prg).Size();

				if (size != (i ? profile.board.GetChr() : profile.board.GetPrg()))
				{
					Profile::Board::Roms& roms = (i ? profile.board.chr : profile.board.prg);
					roms.clear();

					if (size)
					{
						Profile::Board::Rom rom;
						rom.size = size;
						roms.push_back( rom );
					}
				}

				size = (i ? b.type.GetVram() : b.type.GetWram());

				if (size != (i ? profile.board.GetVram() : profile.board.GetWram()))
				{
					Profile::Board::Rams& rams = (i ? profile.board.vram : profile.board.wram);
					rams.clear();

					for (uint j=0; j < 2; ++j)
					{
						size = i ? (j ? b.type.GetNonSavableVram() : b.type.GetSavableVram()) :
                                   (j ? b.type.GetNonSavableWram() : b.type.GetSavableWram());

						if (size)
						{
							Profile::Board::Ram ram;
							ram.size = size;
							ram.battery = (j == 0);
							rams.push_back( ram );
						}
					}
				}

				Profile::Board::Roms& roms = (i ? profile.board.chr : profile.board.prg);

				for (dword j=0, k=0, n=roms.size(); j < n; k += roms[j].size, ++j)
					roms[j].hash.Compute( (i ? chr : prg).Mem(k), roms[j].size );
			}

			if (!readOnly)
			{
				Checksum checksum;

				checksum.Compute( prg.Mem(), prg.Size() );

				if (prgCrc)
					*prgCrc = checksum.GetCrc();

				checksum.Compute( chr.Mem(), chr.Size() );
				profile.hash.Assign( checksum.GetSha1(), checksum.GetCrc() );
			}

			if (board)
				*board = Boards::Board::Create( b );

			return RESULT_OK;
		}

		void Cartridge::Reset(const bool hard)
		{
			board->Reset( hard );

			if (vs)
				vs->Reset( hard );
		}

		bool Cartridge::PowerOff()
		{
			try
			{
				if (board)
				{
					board->Sync( Boards::Board::EVENT_POWER_OFF, NULL );
					board->Save( savefile );
				}

				return true;
			}
			catch (...)
			{
				return false;
			}
		}

		void Cartridge::SaveState(State::Saver& state,const dword baseChunk) const
		{
			state.Begin( baseChunk );

			board->SaveState( state, AsciiId<'M','P','R'>::V );

			if (vs)
				vs->SaveState( state, AsciiId<'V','S','S'>::V );

			state.End();
		}

		void Cartridge::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'M','P','R'>::V:

						board->LoadState( state );
						break;

					case AsciiId<'V','S','S'>::V:

						NST_VERIFY( vs );

						if (vs)
							vs->LoadState( state );

						break;
				}

				state.End();
			}
		}

		Region Cartridge::GetDesiredRegion() const
		{
			switch (profile.system.type)
			{
				case Profile::System::NES_PAL:
				case Profile::System::NES_PAL_A:
				case Profile::System::NES_PAL_B:

					return REGION_PAL;

				default:

					return REGION_NTSC;
			}
		}

		System Cartridge::GetDesiredSystem(Region region,CpuModel* cpu,PpuModel* ppu) const
		{
			if (region == Cartridge::GetDesiredRegion())
			{
				if (cpu)
					*cpu = static_cast<CpuModel>(profile.system.cpu);

				if (ppu)
					*ppu = static_cast<PpuModel>(profile.system.ppu);

				return static_cast<System>(profile.system.type);
			}
			else
			{
				return Image::GetDesiredSystem( region, cpu, ppu );
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Cartridge::BeginFrame(const Api::Input& input,Input::Controllers* controllers)
		{
			board->Sync( Boards::Board::EVENT_BEGIN_FRAME, controllers );

			if (vs)
				vs->BeginFrame( input, controllers );
		}

		void Cartridge::VSync()
		{
			board->Sync( Boards::Board::EVENT_END_FRAME, NULL );

			if (vs)
				vs->VSync();
		}
	}
}
