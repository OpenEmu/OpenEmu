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
#include "NstLog.hpp"
#include "NstFds.hpp"
#include "board/NstBoard.hpp"
#include "board/NstBoardMmc5.hpp"
#include "board/NstBoardKonami.hpp"
#include "board/NstBoardNamcot.hpp"
#include "board/NstBoardSunsoft.hpp"
#include "api/NstApiNsf.hpp"
#include "NstNsf.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Nsf::Chips : Apu::Channel
		{
			struct Mmc5 : Boards::Mmc5::Sound
			{
				uint mul[2];
				byte exRam[SIZE_1K];

				explicit Mmc5(Apu& a)
				: Sound(a,false) {}

				void Reset();
				void ClearExRam();

				using Boards::Mmc5::Sound::UpdateSettings;
				using Boards::Mmc5::Sound::GetSample;
				using Boards::Mmc5::Sound::Clock;
			};

			struct Fds : Core::Fds::Sound
			{
				byte ram[SIZE_8K+SIZE_32K];

				explicit Fds(Apu& a)
				: Sound(a,false) {}

				void Reset();
				void SwapBank(const Prg&,uint,uint);

				using Core::Fds::Sound::UpdateSettings;
				using Core::Fds::Sound::GetSample;
				using Core::Fds::Sound::Clock;
			};

			struct N163 : Boards::Namcot::N163::Sound
			{
				explicit N163(Apu& a)
				: Sound(a,false) {}

				using Boards::Namcot::N163::Sound::Reset;
				using Boards::Namcot::N163::Sound::UpdateSettings;
				using Boards::Namcot::N163::Sound::GetSample;
			};

			struct Vrc6 : Boards::Konami::Vrc6::Sound
			{
				explicit Vrc6(Apu& a)
				: Sound(a,false) {}

				using Boards::Konami::Vrc6::Sound::Reset;
				using Boards::Konami::Vrc6::Sound::UpdateSettings;
				using Boards::Konami::Vrc6::Sound::GetSample;
			};

			struct Vrc7 : Boards::Konami::Vrc7::Sound
			{
				explicit Vrc7(Apu& a)
				: Sound(a,false) {}

				using Boards::Konami::Vrc7::Sound::Reset;
				using Boards::Konami::Vrc7::Sound::UpdateSettings;
				using Boards::Konami::Vrc7::Sound::GetSample;
			};

			struct S5b : Boards::Sunsoft::S5b::Sound
			{
				explicit S5b(Apu& a)
				: Sound(a,false) {}

				using Boards::Sunsoft::S5b::Sound::Reset;
				using Boards::Sunsoft::S5b::Sound::UpdateSettings;
				using Boards::Sunsoft::S5b::Sound::GetSample;
			};

			template<typename T>
			struct Chip : Pointer<T>
			{
				Chip(Apu& a,uint t)
				: Pointer<T>(t ? new T(a) : NULL) {}
			};

			struct Clocks
			{
				void Reset(bool,bool);

				Cycle next;
				Cycle mmc5;
				Cycle fds;
			};

			void Reset();
			bool UpdateSettings();
			Sample GetSample();
			Cycle Clock(Cycle,Cycle,Cycle);

			Clocks clocks;

		public:

			Chips(uint,Apu&);

			Chip<Mmc5> mmc5;
			Chip<Vrc6> vrc6;
			Chip<Vrc7> vrc7;
			Chip<Fds>  fds;
			Chip<S5b>  s5b;
			Chip<N163> n163;
		};

		void Nsf::Chips::Mmc5::ClearExRam()
		{
			std::memset( exRam, 0, sizeof(exRam) );
		}

		void Nsf::Chips::Mmc5::Reset()
		{
			mul[0] = 0;
			mul[1] = 0;

			ClearExRam();

			Sound::Reset();
		}

		void Nsf::Chips::Fds::Reset()
		{
			std::memset( ram, 0, sizeof(ram) );

			Sound::Reset();
		}

		void Nsf::Chips::Fds::SwapBank(const Prg& prg,uint page,uint bank)
		{
			std::memcpy( ram + SIZE_4K * page, prg.Source().Mem(bank * SIZE_4K), SIZE_4K );
		}

		Nsf::Chips::Chips(const uint types,Apu& apu)
		:
		Channel  ( apu ),
		mmc5     ( apu, types & Api::Nsf::CHIP_MMC5 ),
		vrc6     ( apu, types & Api::Nsf::CHIP_VRC6 ),
		vrc7     ( apu, types & Api::Nsf::CHIP_VRC7 ),
		fds      ( apu, types & Api::Nsf::CHIP_FDS  ),
		s5b      ( apu, types & Api::Nsf::CHIP_S5B  ),
		n163     ( apu, types & Api::Nsf::CHIP_N163 )
		{
			Connect( UpdateSettings() );
		}

		void Nsf::Chips::Clocks::Reset(bool mmc5Chip,bool fdsChip)
		{
			next = (mmc5Chip || fdsChip ? 0UL : Cpu::CYCLE_MAX);
			mmc5 = (mmc5Chip            ? 0UL : Cpu::CYCLE_MAX);
			fds  = (fdsChip             ? 0UL : Cpu::CYCLE_MAX);
		}

		void Nsf::Chips::Reset()
		{
			clocks.Reset( mmc5, fds );

			if ( mmc5 ) mmc5->Reset();
			if ( vrc6 ) vrc6->Reset();
			if ( vrc7 ) vrc7->Reset();
			if ( fds  ) fds->Reset();
			if ( s5b  ) s5b->Reset();
			if ( n163 ) n163->Reset();
		}

		bool Nsf::Chips::UpdateSettings()
		{
			clocks.Reset( mmc5, fds );

			return
			(
				( mmc5 ? mmc5->UpdateSettings() : 0U ) |
				( vrc6 ? vrc6->UpdateSettings() : 0U ) |
				( vrc7 ? vrc7->UpdateSettings() : 0U ) |
				( fds  ? fds->UpdateSettings()  : 0U ) |
				( s5b  ? s5b->UpdateSettings()  : 0U ) |
				( n163 ? n163->UpdateSettings() : 0U )
			);
		}

		Nsf::Nsf(Context& context)
		:
		Image    (SOUND),
		cpu      (context.cpu),
		apu      (context.apu),
		chips    (NULL),
		tuneMode (Api::Nsf::TUNE_MODE_NTSC)
		{
			if (context.patch && context.patchResult)
				*context.patchResult = RESULT_ERR_UNSUPPORTED;

			Stream::In stream( &context.stream );

			uint version;

			{
				byte data[5+1+2+6];
				stream.Read( data );

				if
				(
					data[0] != Ascii<'N'>::V ||
					data[1] != Ascii<'E'>::V ||
					data[2] != Ascii<'S'>::V ||
					data[3] != Ascii<'M'>::V ||
					data[4] != 0x1A
				)
					throw RESULT_ERR_INVALID_FILE;

				if (!data[6] || data[9] < 0x60 || data[11] < 0x60 || data[13] < 0x60)
					throw RESULT_ERR_CORRUPT_FILE;

				songs.count = data[6];
				songs.start = data[7] >= 1 && data[7] <= data[6] ? data[7] - 1 : 0;

				addressing.load = data[8]  | uint( data[9]  ) << 8;
				addressing.init = data[10] | uint( data[11] ) << 8;
				addressing.play = data[12] | uint( data[13] ) << 8;

				version = data[5];
			}

			stream.Read( songs.info.name, 32 );
			stream.Read( songs.info.artist, 32 );
			stream.Read( songs.info.copyright, 32 );

			songs.info.name[31] = '\0';
			songs.info.artist[31] = '\0';
			songs.info.copyright[31] = '\0';

			speed.ntsc = stream.Read16();
			stream.Read( banks );

			addressing.bankSwitched = 0 !=
			(
				uint( banks[0] ) |
				uint( banks[1] ) |
				uint( banks[2] ) |
				uint( banks[3] ) |
				uint( banks[4] ) |
				uint( banks[5] ) |
				uint( banks[6] ) |
				uint( banks[7] )
			);

			speed.pal = stream.Read16();
			songs.current = songs.start;

			switch (stream.Read8() & 0x3)
			{
				case 0x0: tuneMode = Api::Nsf::TUNE_MODE_NTSC; break;
				case 0x1: tuneMode = Api::Nsf::TUNE_MODE_PAL;  break;
				default:  tuneMode = Api::Nsf::TUNE_MODE_BOTH; break;
			}

			uint types = stream.Read8();

			if (!(types & Api::Nsf::CHIP_FDS) && addressing.load < 0x8000)
				throw RESULT_ERR_CORRUPT_FILE;

			dword length = 0;

			while (length < SIZE_4096K && stream.SafeRead8() <= 0xFF)
				++length;

			if (length <= HEADER_RESERVED_LENGTH)
				throw RESULT_ERR_CORRUPT_FILE;

			length -= HEADER_RESERVED_LENGTH;
			stream.Seek( -idword(length) );

			{
				const uint offset = addressing.load & 0xFFFU;

				prg.Source().Set( Ram::ROM, true, false, offset + length );
				prg.Source().Fill( JAM );
				stream.Read( prg.Source().Mem() + offset, length );
			}

			if (types & Api::Nsf::CHIP_ALL)
				chips = new Chips (types,apu);

			if (Log::Available())
			{
				Log log;

				log << "Nsf: version " << version;

				if (*songs.info.name)
					log << NST_LINEBREAK "Nsf: name: " << songs.info.name;

				if (*songs.info.artist)
					log << NST_LINEBREAK "Nsf: artist: " << songs.info.artist;

				if (*songs.info.copyright)
					log << NST_LINEBREAK "Nsf: copyright: " << songs.info.copyright;

				log << NST_LINEBREAK "Nsf: starting song "
					<< (songs.start+1U)
					<< " of "
					<< songs.count
					<<
					(
						tuneMode == Api::Nsf::TUNE_MODE_NTSC ? NST_LINEBREAK "Nsf: NTSC mode"     :
						tuneMode == Api::Nsf::TUNE_MODE_PAL  ? NST_LINEBREAK "Nsf: PAL mode"      :
                                                               NST_LINEBREAK "Nsf: PAL/NTSC mode"
					)
					<< NST_LINEBREAK "Nsf: "
					<< (length / SIZE_1K)
					<< (addressing.bankSwitched ? "k bank-switched " : "k flat ")
					<< ((types & Api::Nsf::CHIP_FDS) ? "PRG-RAM" : "PRG-ROM")
					<< NST_LINEBREAK "Nsf: load address - " << Log::Hex( 16, addressing.load )
					<< NST_LINEBREAK "Nsf: init address - " << Log::Hex( 16, addressing.init )
					<< NST_LINEBREAK "Nsf: play address - " << Log::Hex( 16, addressing.play )
					<< NST_LINEBREAK;

				if (types & Api::Nsf::CHIP_ALL)
				{
					if ( chips->mmc5 ) log << "Nsf: MMC5 sound chip present" NST_LINEBREAK;
					if ( chips->vrc6 ) log << "Nsf: VRC6 sound chip present" NST_LINEBREAK;
					if ( chips->vrc7 ) log << "Nsf: VRC7 sound chip present" NST_LINEBREAK;
					if ( chips->fds  ) log << "Nsf: FDS sound chip present" NST_LINEBREAK;
					if ( chips->s5b  ) log << "Nsf: Sunsoft5B sound chip present" NST_LINEBREAK;
					if ( chips->n163 ) log << "Nsf: N163 sound chip present" NST_LINEBREAK;
				}
			}
		}

		Nsf::~Nsf()
		{
			delete chips;
		}

		Region Nsf::GetDesiredRegion() const
		{
			return tuneMode == Api::Nsf::TUNE_MODE_PAL ? REGION_PAL : REGION_NTSC;
		}

		uint Nsf::GetChips() const
		{
			uint types = 0;

			if (chips)
			{
				if ( chips->vrc6 ) types |= Api::Nsf::CHIP_VRC6;
				if ( chips->vrc7 ) types |= Api::Nsf::CHIP_VRC7;
				if ( chips->fds  ) types |= Api::Nsf::CHIP_FDS;
				if ( chips->mmc5 ) types |= Api::Nsf::CHIP_MMC5;
				if ( chips->n163 ) types |= Api::Nsf::CHIP_N163;
				if ( chips->s5b  ) types |= Api::Nsf::CHIP_S5B;
			}

			return types;
		}

		void Nsf::Reset(bool)
		{
			cpu.Map( 0x38EC ).Set( this, &Nsf::Peek_38EC, &Nsf::Poke_Nop );
			cpu.Map( 0x38ED ).Set( this, &Nsf::Peek_38ED, &Nsf::Poke_Nop );
			cpu.Map( 0x38EE ).Set( this, &Nsf::Peek_38EE, &Nsf::Poke_Nop );
			cpu.Map( 0x38EF ).Set( this, &Nsf::Peek_38EF, &Nsf::Poke_Nop );
			cpu.Map( 0x38F0 ).Set( this, &Nsf::Peek_38F0, &Nsf::Poke_Nop );
			cpu.Map( 0x38F1 ).Set( this, &Nsf::Peek_38F1, &Nsf::Poke_Nop );
			cpu.Map( 0x38F2 ).Set( this, &Nsf::Peek_38F2, &Nsf::Poke_Nop );
			cpu.Map( 0x38F3 ).Set( this, &Nsf::Peek_38F3, &Nsf::Poke_Nop );
			cpu.Map( 0x38F4 ).Set( this, &Nsf::Peek_38F4, &Nsf::Poke_Nop );
			cpu.Map( 0x38F5 ).Set( this, &Nsf::Peek_38F5, &Nsf::Poke_Nop );
			cpu.Map( 0x38F6 ).Set( this, &Nsf::Peek_38F6, &Nsf::Poke_Nop );
			cpu.Map( 0x38F7 ).Set( this, &Nsf::Peek_38F7, &Nsf::Poke_Nop );
			cpu.Map( 0x38F8 ).Set( this, &Nsf::Peek_38F8, &Nsf::Poke_Nop );
			cpu.Map( 0x38F9 ).Set( this, &Nsf::Peek_38F9, &Nsf::Poke_Nop );
			cpu.Map( 0x38FA ).Set( this, &Nsf::Peek_38FA, &Nsf::Poke_Nop );
			cpu.Map( 0x38FB ).Set( this, &Nsf::Peek_38FB, &Nsf::Poke_Nop );
			cpu.Map( 0x38FC ).Set( this, &Nsf::Peek_38FC, &Nsf::Poke_Nop );
			cpu.Map( 0x38FD ).Set( this, &Nsf::Peek_38FD, &Nsf::Poke_Nop );
			cpu.Map( 0x38FE ).Set( this, &Nsf::Peek_38FE, &Nsf::Poke_Nop );
			cpu.Map( 0x38FF ).Set( this, &Nsf::Peek_38FF, &Nsf::Poke_Nop );

			cpu.Map( 0x4017 ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_4017 );

			const bool fds = chips && chips->fds;

			if (addressing.bankSwitched)
			{
				if (fds)
				{
					cpu.Map( 0x5FF6 ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_Fds_5FF6 );
					cpu.Map( 0x5FF7 ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_Fds_5FF7 );
				}

				cpu.Map( 0x5FF8 ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FF8 : &Nsf::Poke_5FF8 );
				cpu.Map( 0x5FF9 ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FF9 : &Nsf::Poke_5FF9 );
				cpu.Map( 0x5FFA ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFA : &Nsf::Poke_5FFA );
				cpu.Map( 0x5FFB ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFB : &Nsf::Poke_5FFB );
				cpu.Map( 0x5FFC ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFC : &Nsf::Poke_5FFC );
				cpu.Map( 0x5FFD ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFD : &Nsf::Poke_5FFD );
				cpu.Map( 0x5FFE ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFE : &Nsf::Poke_5FFE );
				cpu.Map( 0x5FFF ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFF : &Nsf::Poke_5FFF );
			}
			else if (!fds)
			{
				for (dword i=0x8000, j=0; i < 0x10000; j += (i >= (addressing.load & 0xF000U)), i += 0x1000)
					prg.SwapBank<SIZE_4K>( i-0x8000, j );
			}

			if (fds)
			{
				cpu.Map( 0x4040, 0x407F ).Set( this, &Nsf::Peek_Fds_4040, &Nsf::Poke_Fds_4040 );
				cpu.Map( 0x4080         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4080 );
				cpu.Map( 0x4082         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4082 );
				cpu.Map( 0x4083         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4083 );
				cpu.Map( 0x4084         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4084 );
				cpu.Map( 0x4085         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4085 );
				cpu.Map( 0x4086         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4086 );
				cpu.Map( 0x4087         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4087 );
				cpu.Map( 0x4088         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4088 );
				cpu.Map( 0x4089         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_4089 );
				cpu.Map( 0x408A         ).Set( this, &Nsf::Peek_Nop,      &Nsf::Poke_Fds_408A );
				cpu.Map( 0x4090         ).Set( this, &Nsf::Peek_Fds_4090, &Nsf::Poke_Nop      );
				cpu.Map( 0x4092         ).Set( this, &Nsf::Peek_Fds_4092, &Nsf::Poke_Nop      );
				cpu.Map( 0x6000, 0xFFFF ).Set( this, &Nsf::Peek_Fds_Ram,  &Nsf::Poke_Fds_Ram  );
			}
			else
			{
				cpu.Map( 0x6000, 0x7FFF ).Set( this, &Nsf::Peek_Wrk,   &Nsf::Poke_Wrk );
				cpu.Map( 0x8000, 0x8FFF ).Set( this, &Nsf::Peek_Prg_8, &Nsf::Poke_Nop );
				cpu.Map( 0x9000, 0x9FFF ).Set( this, &Nsf::Peek_Prg_9, &Nsf::Poke_Nop );
				cpu.Map( 0xA000, 0xAFFF ).Set( this, &Nsf::Peek_Prg_A, &Nsf::Poke_Nop );
				cpu.Map( 0xB000, 0xBFFF ).Set( this, &Nsf::Peek_Prg_B, &Nsf::Poke_Nop );
				cpu.Map( 0xC000, 0xCFFF ).Set( this, &Nsf::Peek_Prg_C, &Nsf::Poke_Nop );
				cpu.Map( 0xD000, 0xDFFF ).Set( this, &Nsf::Peek_Prg_D, &Nsf::Poke_Nop );
				cpu.Map( 0xE000, 0xEFFF ).Set( this, &Nsf::Peek_Prg_E, &Nsf::Poke_Nop );
				cpu.Map( 0xF000, 0xFFFF ).Set( this, &Nsf::Peek_Prg_F, &Nsf::Poke_Nop );
			}

			if (chips)
			{
				if (chips->mmc5)
				{
					cpu.Map( 0x5000         ).Set( this, &Nsf::Peek_Nop,       &Nsf::Poke_Mmc5_5000 );
					cpu.Map( 0x5002         ).Set( this, &Nsf::Peek_Nop,       &Nsf::Poke_Mmc5_5002 );
					cpu.Map( 0x5003         ).Set( this, &Nsf::Peek_Nop,       &Nsf::Poke_Mmc5_5003 );
					cpu.Map( 0x5004         ).Set( this, &Nsf::Peek_Nop,       &Nsf::Poke_Mmc5_5004 );
					cpu.Map( 0x5006         ).Set( this, &Nsf::Peek_Nop,       &Nsf::Poke_Mmc5_5006 );
					cpu.Map( 0x5007         ).Set( this, &Nsf::Peek_Nop,       &Nsf::Poke_Mmc5_5007 );
					cpu.Map( 0x5010         ).Set( this, &Nsf::Peek_Nop,       &Nsf::Poke_Mmc5_5010 );
					cpu.Map( 0x5011         ).Set( this, &Nsf::Peek_Nop,       &Nsf::Poke_Mmc5_5011 );
					cpu.Map( 0x5015         ).Set( this, &Nsf::Peek_Mmc5_5015, &Nsf::Poke_Mmc5_5015 );
					cpu.Map( 0x5205         ).Set( this, &Nsf::Peek_Mmc5_5205, &Nsf::Poke_Mmc5_5205 );
					cpu.Map( 0x5206         ).Set( this, &Nsf::Peek_Mmc5_5206, &Nsf::Poke_Mmc5_5206 );
					cpu.Map( 0x5C00, 0x5FF5 ).Set( this, &Nsf::Peek_Mmc5_5C00, &Nsf::Poke_Mmc5_5C00 );
				}

				if (chips->vrc6)
				{
					cpu.Map( 0x9000 ).Set( &Nsf::Poke_Vrc6_9000 );
					cpu.Map( 0x9001 ).Set( &Nsf::Poke_Vrc6_9001 );
					cpu.Map( 0x9002 ).Set( &Nsf::Poke_Vrc6_9002 );
					cpu.Map( 0xA000 ).Set( &Nsf::Poke_Vrc6_A000 );
					cpu.Map( 0xA001 ).Set( &Nsf::Poke_Vrc6_A001 );
					cpu.Map( 0xA002 ).Set( &Nsf::Poke_Vrc6_A002 );
					cpu.Map( 0xB000 ).Set( &Nsf::Poke_Vrc6_B000 );
					cpu.Map( 0xB001 ).Set( &Nsf::Poke_Vrc6_B001 );
					cpu.Map( 0xB002 ).Set( &Nsf::Poke_Vrc6_B002 );
				}

				if (chips->vrc7)
				{
					cpu.Map( 0x9010 ).Set( &Nsf::Poke_Vrc7_9010 );
					cpu.Map( 0x9030 ).Set( &Nsf::Poke_Vrc7_9030 );
				}

				if (chips->n163)
				{
					cpu.Map( 0x4800 ).Set( this, &Nsf::Peek_N163_48, &Nsf::Poke_N163_48 );
					cpu.Map( 0xF800 ).Set( &Nsf::Poke_N163_F8 );
				}

				if (chips->s5b)
				{
					cpu.Map( 0xC000 ).Set( &Nsf::Poke_S5b_C );
					cpu.Map( 0xE000 ).Set( &Nsf::Poke_S5b_E );
				}
			}

			cpu.Map( 0xFFFA ).Set( &Nsf::Peek_FFFA );
			cpu.Map( 0xFFFB ).Set( &Nsf::Peek_FFFB );
			cpu.Map( 0xFFFC ).Set( &Nsf::Peek_FFFC );
			cpu.Map( 0xFFFD ).Set( &Nsf::Peek_FFFD );

			routine.reset = Routine::RESET;
			routine.nmi = Routine::NMI;

			cpu.SetFrameCycles( cpu.GetModel() == CPU_RP2A03 ? PPU_RP2C02_HVSYNC : PPU_RP2C07_HVSYNC );
		}

		bool Nsf::PowerOff()
		{
			StopSong();
			return true;
		}

		Result Nsf::SelectSong(const uint song)
		{
			if (song < songs.count)
			{
				if (songs.current != song)
				{
					songs.current = song;

					if (routine.playing)
					{
						routine.nmi = Routine::NMI;
						apu.ClearBuffers();
					}

					Api::Nsf::eventCallback( Api::Nsf::EVENT_SELECT_SONG );

					return RESULT_OK;
				}

				return RESULT_NOP;
			}

			return RESULT_ERR_INVALID_PARAM;
		}

		Result Nsf::PlaySong()
		{
			if (!routine.playing)
			{
				routine.nmi = Routine::NMI;
				routine.playing = true;

				Api::Nsf::eventCallback( Api::Nsf::EVENT_PLAY_SONG );

				return RESULT_OK;
			}

			return RESULT_NOP;
		}

		Result Nsf::StopSong()
		{
			if (routine.playing)
			{
				routine.playing = false;
				routine.nmi = Routine::NMI;
				apu.ClearBuffers();

				Api::Nsf::eventCallback( Api::Nsf::EVENT_STOP_SONG );

				return RESULT_OK;
			}

			return RESULT_NOP;
		}

		void Nsf::InitSong()
		{
			std::memset( wrk, 0x00, SIZE_8K );

			if (chips && chips->mmc5)
				chips->mmc5->ClearExRam();

			const bool fds = chips && chips->fds;

			if (addressing.bankSwitched)
			{
				if (fds)
				{
					for (uint i=0; i < 2; ++i)
						cpu.Poke( 0x5FF6+i, banks[6+i] );
				}

				for (uint i=0; i < 8; ++i)
					cpu.Poke( 0x5FF8+i, banks[i] );
			}
			else if (fds)
			{
				for (dword i=0x6000, j=0; i < 0x10000; j += (i >= (addressing.load & 0xF000U)), i += 0x1000)
					std::memcpy( chips->fds->ram + (i-0x6000), prg.Source().Mem(j * 0x1000), 0x1000 );
			}

			if (fds)
			{
				cpu.Poke( 0x4089, 0x80 );
				cpu.Poke( 0x408A, 0xE8 );
			}

			apu.ClearBuffers();
			std::memset( cpu.GetRam(), 0x00, Cpu::RAM_SIZE );

			for (uint i=0x4000; i <= 0x4013; ++i)
				cpu.Poke( i, 0x00 );

			cpu.Poke( 0x4015, 0x0F );
			cpu.Poke( 0x4017, 0xC0 );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Nsf::BeginFrame()
		{
			routine.jmp = (routine.playing ? 0xFA : 0xFD);

			if (routine.nmi)
				cpu.DoNMI(0);
		}

		Cycle Nsf::Chips::Clock(Cycle rateCycles,Cycle rateClock,const Cycle targetCycles)
		{
			if (clocks.next != Cpu::CYCLE_MAX)
			{
				NST_ASSERT( (mmc5 || fds) && (clocks.mmc5 != Cpu::CYCLE_MAX || clocks.fds != Cpu::CYCLE_MAX) );

				if (clocks.mmc5 == clocks.next)
					clocks.mmc5 = mmc5->Clock( rateCycles, rateClock, targetCycles ) - rateCycles;

				if (clocks.fds == clocks.next)
					clocks.fds = fds->Clock( rateCycles, rateClock, targetCycles ) - rateCycles;

				clocks.next = NST_MIN(clocks.mmc5,clocks.fds);

				rateCycles += clocks.next;

				return rateCycles;
			}
			else
			{
				NST_ASSERT( !mmc5 && !fds );

				return Channel::Clock( rateCycles, rateClock, targetCycles );
			}
		}

		Nsf::Chips::Sample Nsf::Chips::GetSample()
		{
			return
			(
				(mmc5 ? mmc5->GetSample() : 0) +
				(vrc6 ? vrc6->GetSample() : 0) +
				(vrc7 ? vrc7->GetSample() : 0) +
				(fds  ? fds->GetSample()  : 0) +
				(s5b  ? s5b->GetSample()  : 0) +
				(n163 ? n163->GetSample() : 0)
			);
		}

		inline uint Nsf::FetchLast(uint offset) const
		{
			NST_ASSERT( offset <= 0xFFF );
			return offset[chips && chips->fds ? chips->fds->ram + (sizeof(array(chips->fds->ram))-SIZE_4K) : prg[7]];
		}

		NES_PEEK(Nsf,FFFA)
		{
			return routine.nmi ? routine.nmi &= Routine::NMI_B, routine.playing ? 0xEC : 0xFD : FetchLast(0xFFA);
		}

		NES_PEEK(Nsf,FFFB)
		{
			return routine.nmi ? routine.nmi &= Routine::NMI_A, 0x38 : FetchLast(0xFFB);
		}

		NES_PEEK(Nsf,FFFC)
		{
			return routine.reset ? routine.reset &= Routine::RESET_B, 0xFD : FetchLast(0xFFC);
		}

		NES_PEEK(Nsf,FFFD)
		{
			return routine.reset ? routine.reset &= Routine::RESET_A, 0x38 : FetchLast(0xFFD);
		}

		NES_PEEK(Nsf,38EC)
		{
			NST_VERIFY( routine.playing );

			InitSong();
			return LDA;
		}

		NES_PEEK(Nsf,38ED)
		{
			NST_VERIFY( routine.playing );
			return songs.current;
		}

		NES_PEEK(Nsf,38EE)
		{
			NST_VERIFY( routine.playing );
			return LDX;
		}

		NES_PEEK(Nsf,38EF)
		{
			NST_VERIFY( routine.playing );
			return 0xFC;
		}

		NES_PEEK(Nsf,38F0)
		{
			NST_VERIFY( routine.playing );
			return TXS;
		}

		NES_PEEK(Nsf,38F1)
		{
			NST_VERIFY( routine.playing );
			return LDX;
		}

		NES_PEEK(Nsf,38F2)
		{
			NST_VERIFY( routine.playing );
			return cpu.GetModel() == CPU_RP2A07;
		}

		NES_PEEK(Nsf,38F3)
		{
			NST_VERIFY( routine.playing );
			return JSR;
		}

		NES_PEEK(Nsf,38F4)
		{
			NST_VERIFY( routine.playing );
			return addressing.init & 0xFFU;
		}

		NES_PEEK(Nsf,38F5)
		{
			NST_VERIFY( routine.playing );
			return addressing.init >> 8;
		}

		NES_PEEK(Nsf,38F6)
		{
			NST_VERIFY( routine.playing );
			return SEI;
		}

		NES_PEEK(Nsf,38F7)
		{
			NST_VERIFY( routine.playing );
			routine.jmp = 0xFD;
			return JMP;
		}

		NES_PEEK(Nsf,38F8)
		{
			NST_VERIFY( routine.playing );
			return 0xFD;
		}

		NES_PEEK(Nsf,38F9)
		{
			NST_VERIFY( routine.playing );
			return 0x38;
		}

		NES_PEEK(Nsf,38FA)
		{
			NST_VERIFY( routine.playing );
			routine.jmp = 0xFD;
			return JSR;
		}

		NES_PEEK(Nsf,38FB)
		{
			NST_VERIFY( routine.playing );
			return addressing.play & 0xFFU;
		}

		NES_PEEK(Nsf,38FC)
		{
			NST_VERIFY( routine.playing );
			return addressing.play >> 8;
		}

		NES_PEEK(Nsf,38FD)
		{
			return JMP;
		}

		NES_PEEK(Nsf,38FE)
		{
			return routine.jmp;
		}

		NES_PEEK(Nsf,38FF)
		{
			return 0x38;
		}

		NES_POKE_D(Nsf,4017)
		{
			apu.WriteFrameCtrl( data );
		}

		NES_PEEK_A (Nsf,Nop) { return address >> 8; }
		NES_POKE   (Nsf,Nop) {}

		NES_PEEK_A (Nsf,Prg_8) { return prg[0][address - 0x8000]; }
		NES_PEEK_A (Nsf,Prg_9) { return prg[1][address - 0x9000]; }
		NES_PEEK_A (Nsf,Prg_A) { return prg[2][address - 0xA000]; }
		NES_PEEK_A (Nsf,Prg_B) { return prg[3][address - 0xB000]; }
		NES_PEEK_A (Nsf,Prg_C) { return prg[4][address - 0xC000]; }
		NES_PEEK_A (Nsf,Prg_D) { return prg[5][address - 0xD000]; }
		NES_PEEK_A (Nsf,Prg_E) { return prg[6][address - 0xE000]; }
		NES_PEEK_A (Nsf,Prg_F) { return prg[7][address - 0xF000]; }

		NES_PEEK_A  (Nsf,Wrk) { return wrk[address - 0x6000]; }
		NES_POKE_AD (Nsf,Wrk) { wrk[address - 0x6000] = data; }

		NES_POKE_D (Nsf,5FF8) { prg.SwapBank<SIZE_4K,0x0000>( data ); }
		NES_POKE_D (Nsf,5FF9) { prg.SwapBank<SIZE_4K,0x1000>( data ); }
		NES_POKE_D (Nsf,5FFA) { prg.SwapBank<SIZE_4K,0x2000>( data ); }
		NES_POKE_D (Nsf,5FFB) { prg.SwapBank<SIZE_4K,0x3000>( data ); }
		NES_POKE_D (Nsf,5FFC) { prg.SwapBank<SIZE_4K,0x4000>( data ); }
		NES_POKE_D (Nsf,5FFD) { prg.SwapBank<SIZE_4K,0x5000>( data ); }
		NES_POKE_D (Nsf,5FFE) { prg.SwapBank<SIZE_4K,0x6000>( data ); }
		NES_POKE_D (Nsf,5FFF) { prg.SwapBank<SIZE_4K,0x7000>( data ); }

		NES_PEEK_A  (Nsf,Fds_4040) { return chips->fds->ReadWaveData( address ); }
		NES_POKE_AD (Nsf,Fds_4040) { chips->fds->WriteWaveData( address, data ); }
		NES_POKE_D  (Nsf,Fds_4080) { chips->fds->WriteReg0( data ); }
		NES_POKE_D  (Nsf,Fds_4082) { chips->fds->WriteReg1( data ); }
		NES_POKE_D  (Nsf,Fds_4083) { chips->fds->WriteReg2( data ); }
		NES_POKE_D  (Nsf,Fds_4084) { chips->fds->WriteReg3( data ); }
		NES_POKE_D  (Nsf,Fds_4085) { chips->fds->WriteReg4( data ); }
		NES_POKE_D  (Nsf,Fds_4086) { chips->fds->WriteReg5( data ); }
		NES_POKE_D  (Nsf,Fds_4087) { chips->fds->WriteReg6( data ); }
		NES_POKE_D  (Nsf,Fds_4088) { chips->fds->WriteReg7( data ); }
		NES_POKE_D  (Nsf,Fds_4089) { chips->fds->WriteReg8( data ); }
		NES_POKE_D  (Nsf,Fds_408A) { chips->fds->WriteReg9( data ); }
		NES_PEEK    (Nsf,Fds_4090) { return chips->fds->ReadVolumeGain(); }
		NES_PEEK    (Nsf,Fds_4092) { return chips->fds->ReadSweepGain();  }
		NES_POKE_D  (Nsf,Fds_5FF6) { chips->fds->SwapBank( prg, 0, data ); }
		NES_POKE_D  (Nsf,Fds_5FF7) { chips->fds->SwapBank( prg, 1, data ); }
		NES_POKE_D  (Nsf,Fds_5FF8) { chips->fds->SwapBank( prg, 2, data ); }
		NES_POKE_D  (Nsf,Fds_5FF9) { chips->fds->SwapBank( prg, 3, data ); }
		NES_POKE_D  (Nsf,Fds_5FFA) { chips->fds->SwapBank( prg, 4, data ); }
		NES_POKE_D  (Nsf,Fds_5FFB) { chips->fds->SwapBank( prg, 5, data ); }
		NES_POKE_D  (Nsf,Fds_5FFC) { chips->fds->SwapBank( prg, 6, data ); }
		NES_POKE_D  (Nsf,Fds_5FFD) { chips->fds->SwapBank( prg, 7, data ); }
		NES_POKE_D  (Nsf,Fds_5FFE) { chips->fds->SwapBank( prg, 8, data ); }
		NES_POKE_D  (Nsf,Fds_5FFF) { chips->fds->SwapBank( prg, 9, data ); }
		NES_PEEK_A  (Nsf,Fds_Ram)  { return chips->fds->ram[address - 0x6000]; }
		NES_POKE_AD (Nsf,Fds_Ram)  { chips->fds->ram[address - 0x6000] = data; }

		NES_POKE_D  (Nsf,Mmc5_5000) { chips->mmc5->WriteSquareReg0( 0, data ); }
		NES_POKE_D  (Nsf,Mmc5_5002) { chips->mmc5->WriteSquareReg1( 0, data ); }
		NES_POKE_D  (Nsf,Mmc5_5003) { chips->mmc5->WriteSquareReg2( 0, data ); }
		NES_POKE_D  (Nsf,Mmc5_5004) { chips->mmc5->WriteSquareReg0( 1, data ); }
		NES_POKE_D  (Nsf,Mmc5_5006) { chips->mmc5->WriteSquareReg1( 1, data ); }
		NES_POKE_D  (Nsf,Mmc5_5007) { chips->mmc5->WriteSquareReg2( 1, data ); }
		NES_POKE_D  (Nsf,Mmc5_5010) { chips->mmc5->WritePcmReg0( data ); }
		NES_POKE_D  (Nsf,Mmc5_5011) { chips->mmc5->WritePcmReg1( data ); }
		NES_POKE_D  (Nsf,Mmc5_5015) { chips->mmc5->WriteCtrl( data ); }
		NES_PEEK    (Nsf,Mmc5_5015) { return chips->mmc5->ReadCtrl(); }
		NES_PEEK    (Nsf,Mmc5_5205) { return (chips->mmc5->mul[0] * chips->mmc5->mul[1]) >> 0 & 0xFF; }
		NES_PEEK    (Nsf,Mmc5_5206) { return (chips->mmc5->mul[0] * chips->mmc5->mul[1]) >> 8 & 0xFF; }
		NES_POKE_D  (Nsf,Mmc5_5205) { chips->mmc5->mul[0] = data; }
		NES_POKE_D  (Nsf,Mmc5_5206) { chips->mmc5->mul[1] = data; }
		NES_PEEK_A  (Nsf,Mmc5_5C00) { return chips->mmc5->exRam[address - 0x5C00]; }
		NES_POKE_AD (Nsf,Mmc5_5C00) { chips->mmc5->exRam[address - 0x5C00] = data; }

		NES_POKE_D (Nsf,Vrc6_9000) { chips->vrc6->WriteSquareReg0( 0, data ); }
		NES_POKE_D (Nsf,Vrc6_9001) { chips->vrc6->WriteSquareReg1( 0, data ); }
		NES_POKE_D (Nsf,Vrc6_9002) { chips->vrc6->WriteSquareReg2( 0, data ); }
		NES_POKE_D (Nsf,Vrc6_A000) { chips->vrc6->WriteSquareReg0( 1, data ); }
		NES_POKE_D (Nsf,Vrc6_A001) { chips->vrc6->WriteSquareReg1( 1, data ); }
		NES_POKE_D (Nsf,Vrc6_A002) { chips->vrc6->WriteSquareReg2( 1, data ); }
		NES_POKE_D (Nsf,Vrc6_B000) { chips->vrc6->WriteSawReg0( data );       }
		NES_POKE_D (Nsf,Vrc6_B001) { chips->vrc6->WriteSawReg1( data );       }
		NES_POKE_D (Nsf,Vrc6_B002) { chips->vrc6->WriteSawReg2( data );       }

		NES_POKE_D (Nsf,Vrc7_9010) { chips->vrc7->SelectReg( data ); }
		NES_POKE_D (Nsf,Vrc7_9030) { chips->vrc7->WriteReg( data ); }

		NES_POKE_D (Nsf,S5b_C) { chips->s5b->SelectReg( data ); }
		NES_POKE_D (Nsf,S5b_E) { chips->s5b->WriteReg( data );  }

		NES_PEEK   (Nsf,N163_48) { return chips->n163->ReadData(); }
		NES_POKE_D (Nsf,N163_48) { chips->n163->WriteData( data );  }
		NES_POKE_D (Nsf,N163_F8) { chips->n163->WriteAddress( data );  }
	}
}
