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

#ifndef NST_NSF_H
#define NST_NSF_H

#include "NstImage.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Nsf : public Image
		{
		public:

			explicit Nsf(Context&);

			void BeginFrame();
			uint GetChips() const;

			Result SelectSong(uint);
			Result PlaySong();
			Result StopSong();

		private:

			~Nsf();

			void Reset(bool);
			bool PowerOff();
			void InitSong();
			Region GetDesiredRegion() const;

			inline uint FetchLast(uint) const;

			typedef Memory<SIZE_32K,SIZE_4K> Prg;

			enum
			{
				JAM = 0x02,
				JMP = 0x4C,
				JSR = 0x20,
				SEI = 0x78,
				LDA = 0xA9,
				LDX = 0xA2,
				TXS = 0x9A
			};

			NES_DECL_PEEK( 38EC );
			NES_DECL_PEEK( 38ED );
			NES_DECL_PEEK( 38EE );
			NES_DECL_PEEK( 38EF );
			NES_DECL_PEEK( 38F0 );
			NES_DECL_PEEK( 38F1 );
			NES_DECL_PEEK( 38F2 );
			NES_DECL_PEEK( 38F3 );
			NES_DECL_PEEK( 38F4 );
			NES_DECL_PEEK( 38F5 );
			NES_DECL_PEEK( 38F6 );
			NES_DECL_PEEK( 38F7 );
			NES_DECL_PEEK( 38F8 );
			NES_DECL_PEEK( 38F9 );
			NES_DECL_PEEK( 38FA );
			NES_DECL_PEEK( 38FB );
			NES_DECL_PEEK( 38FC );
			NES_DECL_PEEK( 38FD );
			NES_DECL_PEEK( 38FE );
			NES_DECL_PEEK( 38FF );

			NES_DECL_POKE( 4017 );

			NES_DECL_POKE( 5FF8 );
			NES_DECL_POKE( 5FF9 );
			NES_DECL_POKE( 5FFA );
			NES_DECL_POKE( 5FFB );
			NES_DECL_POKE( 5FFC );
			NES_DECL_POKE( 5FFD );
			NES_DECL_POKE( 5FFE );
			NES_DECL_POKE( 5FFF );

			NES_DECL_PEEK( Prg_8 );
			NES_DECL_PEEK( Prg_9 );
			NES_DECL_PEEK( Prg_A );
			NES_DECL_PEEK( Prg_B );
			NES_DECL_PEEK( Prg_C );
			NES_DECL_PEEK( Prg_D );
			NES_DECL_PEEK( Prg_E );
			NES_DECL_PEEK( Prg_F );

			NES_DECL_PEEK( Wrk );
			NES_DECL_POKE( Wrk );

			NES_DECL_PEEK( Fds_4040 );
			NES_DECL_POKE( Fds_4040 );
			NES_DECL_POKE( Fds_4080 );
			NES_DECL_POKE( Fds_4082 );
			NES_DECL_POKE( Fds_4083 );
			NES_DECL_POKE( Fds_4084 );
			NES_DECL_POKE( Fds_4085 );
			NES_DECL_POKE( Fds_4086 );
			NES_DECL_POKE( Fds_4087 );
			NES_DECL_POKE( Fds_4088 );
			NES_DECL_POKE( Fds_4089 );
			NES_DECL_POKE( Fds_408A );
			NES_DECL_PEEK( Fds_4090 );
			NES_DECL_PEEK( Fds_4092 );
			NES_DECL_POKE( Fds_5FF6 );
			NES_DECL_POKE( Fds_5FF7 );
			NES_DECL_POKE( Fds_5FF8 );
			NES_DECL_POKE( Fds_5FF9 );
			NES_DECL_POKE( Fds_5FFA );
			NES_DECL_POKE( Fds_5FFB );
			NES_DECL_POKE( Fds_5FFC );
			NES_DECL_POKE( Fds_5FFD );
			NES_DECL_POKE( Fds_5FFE );
			NES_DECL_POKE( Fds_5FFF );
			NES_DECL_PEEK( Fds_Ram  );
			NES_DECL_POKE( Fds_Ram  );

			NES_DECL_POKE( Mmc5_5000 );
			NES_DECL_POKE( Mmc5_5002 );
			NES_DECL_POKE( Mmc5_5003 );
			NES_DECL_POKE( Mmc5_5004 );
			NES_DECL_POKE( Mmc5_5006 );
			NES_DECL_POKE( Mmc5_5007 );
			NES_DECL_POKE( Mmc5_5010 );
			NES_DECL_POKE( Mmc5_5011 );
			NES_DECL_POKE( Mmc5_5015 );
			NES_DECL_PEEK( Mmc5_5015 );
			NES_DECL_PEEK( Mmc5_5205 );
			NES_DECL_PEEK( Mmc5_5206 );
			NES_DECL_POKE( Mmc5_5205 );
			NES_DECL_POKE( Mmc5_5206 );
			NES_DECL_PEEK( Mmc5_5C00 );
			NES_DECL_POKE( Mmc5_5C00 );

			NES_DECL_POKE( Vrc6_9000 );
			NES_DECL_POKE( Vrc6_9001 );
			NES_DECL_POKE( Vrc6_9002 );
			NES_DECL_POKE( Vrc6_A000 );
			NES_DECL_POKE( Vrc6_A001 );
			NES_DECL_POKE( Vrc6_A002 );
			NES_DECL_POKE( Vrc6_B000 );
			NES_DECL_POKE( Vrc6_B001 );
			NES_DECL_POKE( Vrc6_B002 );

			NES_DECL_POKE( Vrc7_9010 );
			NES_DECL_POKE( Vrc7_9030 );

			NES_DECL_PEEK( N163_48 );
			NES_DECL_POKE( N163_48 );
			NES_DECL_POKE( N163_F8 );

			NES_DECL_POKE( S5b_C );
			NES_DECL_POKE( S5b_E );

			NES_DECL_PEEK( FFFA );
			NES_DECL_PEEK( FFFB );
			NES_DECL_PEEK( FFFC );
			NES_DECL_PEEK( FFFD );

			NES_DECL_PEEK( Nop );
			NES_DECL_POKE( Nop );

			enum Header
			{
				HEADER_SIZE = 128,
				HEADER_RESERVED_LENGTH = 4
			};

			class Chips;

			struct Songs
			{
				byte start;
				byte current;
				byte count;
				const byte padding;

				struct
				{
					char name[32];
					char artist[32];
					char copyright[32];
				}   info;

				Songs()
				:
				start    (0),
				current  (0),
				count    (0),
				padding  (0)
				{}
			};

			struct Speed
			{
				word ntsc;
				word pal;

				Speed()
				:
				ntsc (0),
				pal  (0)
				{}
			};

			struct Addressing
			{
				word play;
				word init;
				word load;
				word bankSwitched;

				Addressing()
				:
				play         (0x0000),
				init         (0x0000),
				load         (0x0000),
				bankSwitched (false)
				{}
			};

			struct Routine
			{
				enum
				{
					RESET_A = 0x1,
					RESET_B = 0x2,
					RESET   = RESET_A|RESET_B,
					NMI_A   = 0x1,
					NMI_B   = 0x2,
					NMI     = NMI_A|NMI_B
				};

				bool playing;
				byte nmi;
				byte reset;
				byte jmp;

				Routine()
				:
				playing (false),
				nmi     (0),
				reset   (0),
				jmp     (0x00)
				{}
			};

			Prg        prg;
			Routine    routine;
			Cpu&       cpu;
			Apu&       apu;
			Chips*     chips;
			Songs      songs;
			Addressing addressing;
			Speed      speed;
			uint       tuneMode;
			byte       banks[8];
			byte       wrk[SIZE_8K];

		public:

			cstring GetName() const
			{
				return songs.info.name;
			}

			cstring GetArtist() const
			{
				return songs.info.artist;
			}

			cstring GetCopyright() const
			{
				return songs.info.copyright;
			}

			uint GetTuneMode() const
			{
				return tuneMode;
			}

			uint NumSongs() const
			{
				return songs.count;
			}

			uint CurrentSong() const
			{
				return songs.current;
			}

			uint StartingSong() const
			{
				return songs.start;
			}

			bool UsesBankSwitching() const
			{
				return addressing.bankSwitched;
			}

			bool IsPlaying() const
			{
				return routine.playing;
			}

			uint GetInitAddress() const
			{
				return addressing.init;
			}

			uint GetLoadAddress() const
			{
				return addressing.load;
			}

			uint GetPlayAddress() const
			{
				return addressing.play;
			}
		};
	}
}

#endif
