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
#include "NstPatcher.hpp"
#include "NstStream.hpp"
#include "NstChecksum.hpp"
#include "NstImageDatabase.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeInes.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Cartridge::Ines::Loader
		{
			bool Load(Ram&,dword);

			enum TrainerSetup
			{
				TRAINER_NONE,
				TRAINER_IGNORE,
				TRAINER_READ
			};

			enum
			{
				VS_MAPPER_99 = 99,
				VS_MAPPER_151 = 151,
				FFE_MAPPER_6 = 6,
				FFE_MAPPER_8 = 8,
				FFE_MAPPER_17 = 17,
				TRAINER_LENGTH = 0x200,
				MIN_DB_SEARCH_STRIDE = SIZE_8K,
				MAX_DB_SEARCH_LENGTH = SIZE_16K * 0xFFFUL + SIZE_8K * 0xFFFUL
			};

			Stream::In stream;
			const FavoredSystem favoredSystem;
			Profile& profile;
			ProfileEx& profileEx;
			Ram& prg;
			Ram& chr;
			const ImageDatabase* const database;
			Patcher patcher;

		public:

			Loader
			(
				std::istream& stdStreamImage,
				std::istream* const stdStreamPatch,
				const bool patchBypassChecksum,
				Result* const patchResult,
				Ram& p,
				Ram& c,
				const FavoredSystem f,
				Profile& r,
				ProfileEx& x,
				const ImageDatabase* const d
			)
			:
			stream        (&stdStreamImage),
			favoredSystem (f),
			profile       (r),
			profileEx     (x),
			prg           (p),
			chr           (c),
			database      (d),
			patcher       (patchBypassChecksum)
			{
				NST_ASSERT( prg.Empty() && chr.Empty() );

				if (stdStreamPatch)
					*patchResult = patcher.Load( *stdStreamPatch, stdStreamImage );

				profile = Profile();
				profileEx = ProfileEx();
			}

			void Load()
			{
				const TrainerSetup trainerSetup = Collect();

				if (!profile.patched)
				{
					if (const ImageDatabase::Entry entry = SearchDatabase( trainerSetup ))
					{
						entry.Fill( profile, patcher.Empty() );
						profileEx.wramAuto = false;
					}
				}

				prg.Set( profile.board.GetPrg() );
				chr.Set( profile.board.GetChr() );

				if (!profile.board.prg.empty())
				{
					for (Profile::Board::Pins::const_iterator it(profile.board.prg.front().pins.begin()), end(profile.board.prg.front().pins.end()); it != end; ++it)
						prg.Pin(it->number) = it->function.c_str();
				}

				if (!profile.board.chr.empty())
				{
					for (Profile::Board::Pins::const_iterator it(profile.board.chr.front().pins.begin()), end(profile.board.chr.front().pins.end()); it != end; ++it)
						chr.Pin(it->number) = it->function.c_str();
				}

				if (trainerSetup == TRAINER_READ)
				{
					profileEx.trainer.Set( TRAINER_LENGTH );
					stream.Read( profileEx.trainer.Mem(), TRAINER_LENGTH );
				}
				else if (trainerSetup == TRAINER_IGNORE)
				{
					stream.Seek( TRAINER_LENGTH );
				}

				if (Load( prg, 16 ))
					Log::Flush( "Ines: PRG-ROM was patched" NST_LINEBREAK );

				if (Load( chr, 16 + prg.Size() ))
					Log::Flush( "Ines: PRG-ROM was patched" NST_LINEBREAK );
			}

		private:

			TrainerSetup Collect()
			{
				NST_COMPILE_ASSERT
				(
					Header::PPU_RP2C03B     ==  1 &&
					Header::PPU_RP2C03G     ==  2 &&
					Header::PPU_RP2C04_0001 ==  3 &&
					Header::PPU_RP2C04_0002 ==  4 &&
					Header::PPU_RP2C04_0003 ==  5 &&
					Header::PPU_RP2C04_0004 ==  6 &&
					Header::PPU_RC2C03B     ==  7 &&
					Header::PPU_RC2C03C     ==  8 &&
					Header::PPU_RC2C05_01   ==  9 &&
					Header::PPU_RC2C05_02   == 10 &&
					Header::PPU_RC2C05_03   == 11 &&
					Header::PPU_RC2C05_04   == 12 &&
					Header::PPU_RC2C05_05   == 13
				);

				Header setup;

				byte header[16];
				stream.Read( header );

				if (patcher.Patch( header, header, 16 ))
				{
					profile.patched = true;
					Log::Flush( "Ines: header was patched" NST_LINEBREAK );
				}

				Result result = ReadHeader( setup, header, 16 );

				if (NES_FAILED(result))
					throw RESULT_ERR_CORRUPT_FILE;

				Log log;

				static const char title[] = "Ines: ";

				if (setup.version)
					log << title << "version 2.0 detected" NST_LINEBREAK;

				if (result == RESULT_WARN_BAD_FILE_HEADER)
					log << title << "warning, unknown or invalid header data!" NST_LINEBREAK;

				log << title
					<< (setup.prgRom / SIZE_1K)
					<< "k PRG-ROM set" NST_LINEBREAK;

				if (setup.version)
				{
					if (setup.prgRam)
					{
						log << title
							<< (setup.prgRam % SIZE_1K ? setup.prgRam : setup.prgRam / SIZE_1K)
							<< (setup.prgRam % SIZE_1K ? " byte" : "k")
							<< " PRG-RAM set" NST_LINEBREAK;
					}

					if (setup.prgNvRam)
					{
						log << title
							<< (setup.prgNvRam % SIZE_1K ? setup.prgNvRam : setup.prgNvRam / SIZE_1K)
							<< (setup.prgNvRam % SIZE_1K ? " bytes" : "k")
							<< " non-volatile PRG-RAM set" NST_LINEBREAK;
					}
				}

				if (setup.chrRom)
				{
					log << title
						<< (setup.chrRom / SIZE_1K)
						<< "k CHR-ROM set" NST_LINEBREAK;
				}

				if (setup.version)
				{
					if (setup.chrRam)
					{
						log << title
							<< (setup.chrRam % SIZE_1K ? setup.chrRam : setup.chrRam / SIZE_1K)
							<< (setup.chrRam % SIZE_1K ? " bytes" : "k")
							<< " CHR-RAM set" NST_LINEBREAK;
					}

					if (setup.chrNvRam)
					{
						log << title
							<< (setup.chrNvRam % SIZE_1K ? setup.chrNvRam : setup.chrNvRam / SIZE_1K)
							<< (setup.chrNvRam % SIZE_1K ? " bytes" : "k")
							<< " non-volatile CHR-RAM set" NST_LINEBREAK;
					}
				}
				else
				{
					if (header[8])
					{
						log << title
							<< (header[8] * 8U)
							<< "k W-RAM set" NST_LINEBREAK;
					}

					if (header[6] & 0x2U)
					{
						log << title
							<< "battery set" NST_LINEBREAK;
					}
				}

				log << title <<
				(
					setup.mirroring == Header::MIRRORING_FOURSCREEN ? "four-screen" :
					setup.mirroring == Header::MIRRORING_VERTICAL   ? "vertical" :
                                                                      "horizontal"
				) << " mirroring set" NST_LINEBREAK;

				log << title <<
				(
					setup.region == Header::REGION_BOTH ? "NTSC/PAL" :
					setup.region == Header::REGION_PAL  ? "PAL":
                                                          "NTSC"
				) << " set" NST_LINEBREAK;

				if (setup.system == Header::SYSTEM_VS)
				{
					log << title << "VS System set" NST_LINEBREAK;

					if (setup.version)
					{
						if (setup.ppu)
						{
							static cstring const names[] =
							{
								"RP2C03B",
								"RP2C03G",
								"RP2C04-0001",
								"RP2C04-0002",
								"RP2C04-0003",
								"RP2C04-0004",
								"RC2C03B",
								"RC2C03C",
								"RC2C05-01",
								"RC2C05-02",
								"RC2C05-03",
								"RC2C05-04",
								"RC2C05-05"
							};

							NST_ASSERT( setup.ppu < 1+sizeof(array(names)) );
							log << title << names[setup.ppu-1] << " PPU set" NST_LINEBREAK;
						}

						if (setup.security)
						{
							static const cstring names[] =
							{
								"RBI Baseball",
								"TKO Boxing",
								"Super Xevious"
							};

							NST_ASSERT( setup.security < 1+sizeof(array(names)) );
							log << title << names[setup.security-1] << " VS mode set" NST_LINEBREAK;
						}
					}
				}
				else if (setup.system == Header::SYSTEM_PC10)
				{
					log << title << "PlayChoice-10 set" NST_LINEBREAK;
				}

				log << title << "mapper " << setup.mapper << " set";

				if (setup.system != Header::SYSTEM_VS && (setup.mapper == VS_MAPPER_99 || setup.mapper == VS_MAPPER_151))
				{
					setup.system = Header::SYSTEM_VS;
					setup.ppu = Header::PPU_RP2C03B;
					log << ", forcing VS System";
				}

				log << NST_LINEBREAK;

				if (setup.version && setup.subMapper)
					log << title << "unknown sub-mapper " << setup.subMapper << " set" NST_LINEBREAK;

				TrainerSetup trainerSetup;

				if (!setup.trainer)
				{
					trainerSetup = TRAINER_NONE;
				}
				else if (setup.mapper == FFE_MAPPER_6 || setup.mapper == FFE_MAPPER_8 || setup.mapper == FFE_MAPPER_17)
				{
					trainerSetup = TRAINER_READ;
					log << title << "trainer set" NST_LINEBREAK;

					if (setup.prgRam + setup.prgNvRam < SIZE_8K)
					{
						setup.prgRam = SIZE_8K - setup.prgNvRam;
						log << title << "warning, forcing 8k of W-RAM for trainer" NST_LINEBREAK;
					}
				}
				else
				{
					trainerSetup = TRAINER_IGNORE;
					log << title << "warning, trainer ignored" NST_LINEBREAK;
				}

				if (setup.prgRom)
				{
					Profile::Board::Rom rom;
					rom.size = setup.prgRom;
					profile.board.prg.push_back( rom );
				}

				if (setup.chrRom)
				{
					Profile::Board::Rom rom;
					rom.size = setup.chrRom;
					profile.board.chr.push_back( rom );
				}

				if (setup.prgNvRam)
				{
					Profile::Board::Ram ram;
					ram.size = setup.prgNvRam;
					ram.battery = true;
					profile.board.wram.push_back( ram );
				}

				if (setup.prgRam)
				{
					Profile::Board::Ram ram;
					ram.size = setup.prgRam;
					profile.board.wram.push_back( ram );
				}

				if (setup.chrNvRam)
				{
					Profile::Board::Ram ram;
					ram.size = setup.chrNvRam;
					ram.battery = true;
					profile.board.vram.push_back( ram );
				}

				if (setup.chrRam)
				{
					Profile::Board::Ram ram;
					ram.size = setup.chrRam;
					profile.board.vram.push_back( ram );
				}

				profile.board.mapper = setup.mapper;
				profileEx.wramAuto = (setup.version == 0 && profile.board.wram.empty());

				switch (setup.mirroring)
				{
					case Header::MIRRORING_HORIZONTAL:

						profile.board.solderPads = Profile::Board::SOLDERPAD_V;
						break;

					case Header::MIRRORING_VERTICAL:

						profile.board.solderPads = Profile::Board::SOLDERPAD_H;
						break;

					case Header::MIRRORING_FOURSCREEN:

						profileEx.nmt = ProfileEx::NMT_FOURSCREEN;
						break;
				}

				profile.system.cpu = Profile::System::CPU_RP2A03;
				profile.system.ppu = static_cast<Profile::System::Ppu>(setup.ppu);

				switch (setup.system)
				{
					case Header::SYSTEM_VS:

						profile.system.type = Profile::System::VS_UNISYSTEM;
						break;

					case Header::SYSTEM_PC10:

						profile.system.type = Profile::System::PLAYCHOICE_10;
						break;

					default:

						switch (setup.region)
						{
							case Header::REGION_NTSC:

								if (favoredSystem == FAVORED_FAMICOM)
									profile.system.type = Profile::System::FAMICOM;
								else
									profile.system.type = Profile::System::NES_NTSC;
								break;

							default:

								profile.multiRegion = true;

								if (favoredSystem == FAVORED_FAMICOM)
								{
									profile.system.type = Profile::System::FAMICOM;
									break;
								}
								else if (favoredSystem != FAVORED_NES_PAL)
								{
									profile.system.type = Profile::System::NES_NTSC;
									break;
								}

							case Header::REGION_PAL:

								profile.system.type = Profile::System::NES_PAL;
								profile.system.cpu = Profile::System::CPU_RP2A07;
								break;
						}
						break;
				}

				return trainerSetup;
			}

			ImageDatabase::Entry SearchDatabase(const TrainerSetup trainerSetup)
			{
				ImageDatabase::Entry entry;

				if (database && database->Enabled())
				{
					if (trainerSetup != TRAINER_NONE)
						stream.Seek( TRAINER_LENGTH );

					const dword romLength = profile.board.GetPrg() + profile.board.GetChr();
					dword count = 0;

					for (Checksum it, checksum;;)
					{
						const uint data = stream.SafeRead8();

						if (data <= 0xFF)
						{
							const byte in = data;
							it.Compute( &in, 1 );

							if (++count % MIN_DB_SEARCH_STRIDE == 0)
								checksum = it;
						}

						const bool stop = (data > 0xFF || count == MAX_DB_SEARCH_LENGTH);

						if (stop || count == romLength)
						{
							entry = database->Search( Profile::Hash(checksum.GetSha1(),checksum.GetCrc()), favoredSystem );

							if (stop || entry)
								break;
						}
					}

					if (count)
						stream.Seek( -idword(count) + (trainerSetup == TRAINER_NONE ? 0 : -TRAINER_LENGTH)  );
				}

				return entry;
			}
		};

		bool Cartridge::Ines::Loader::Load(Ram& rom,const dword offset)
		{
			if (rom.Size())
			{
				if (patcher.Empty())
				{
					stream.Read( rom.Mem(), rom.Size() );
				}
				else
				{
					dword size = stream.Length();

					NST_VERIFY( size >= rom.Size() );

					if (size > rom.Size())
						size = rom.Size();

					if (size)
						stream.Read( rom.Mem(), size );

					if (patcher.Patch( rom.Mem(), rom.Mem(), rom.Size(), offset ))
					{
						profile.patched = true;
						return true;
					}
				}
			}

			return false;
		}

		void Cartridge::Ines::Load
		(
			std::istream& stdStreamImage,
			std::istream* const stdStreamPatch,
			const bool patchBypassChecksum,
			Result* const patchResult,
			Ram& prg,
			Ram& chr,
			const FavoredSystem favoredSystem,
			Profile& profile,
			ProfileEx& profileEx,
			const ImageDatabase* const database
		)
		{
			Loader loader
			(
				stdStreamImage,
				stdStreamPatch,
				patchBypassChecksum,
				patchResult,
				prg,
				chr,
				favoredSystem,
				profile,
				profileEx,
				database
			);

			loader.Load();
		}

		Result Cartridge::Ines::ReadHeader(Header& setup,const byte* const file,const ulong length)
		{
			if (file == NULL)
				return RESULT_ERR_INVALID_PARAM;

			if
			(
				length < 4 ||
				file[0] != Ascii<'N'>::V ||
				file[1] != Ascii<'E'>::V ||
				file[2] != Ascii<'S'>::V ||
				file[3] != 0x1A
			)
				return RESULT_ERR_INVALID_FILE;

			if (length < 16)
				return RESULT_ERR_CORRUPT_FILE;

			byte header[16];
			std::memcpy( header, file, 16 );

			Result result = RESULT_OK;

			setup.version = ((header[7] & 0xCU) == 0x8 ? 2 : 0);

			if (!setup.version)
			{
				for (uint i=10; i < 16; ++i)
				{
					if (header[i])
					{
						header[7] = 0;
						header[8] = 0;
						header[9] = 0;
						result = RESULT_WARN_BAD_FILE_HEADER;
						break;
					}
				}
			}

			setup.prgRom = header[4];
			setup.chrRom = header[5];

			if (setup.version)
			{
				setup.prgRom |= uint(header[9]) << 8 & 0xF00;
				setup.chrRom |= uint(header[9]) << 4 & 0xF00;
			}

			setup.prgRom *= SIZE_16K;
			setup.chrRom *= SIZE_8K;

			setup.trainer = bool(header[6] & 0x4U);

			setup.mapper = (header[6] >> 4) | (header[7] & 0xF0U);
			setup.subMapper = 0;

			if (setup.version)
			{
				setup.mapper |= uint(header[8]) << 8 & 0x100;
				setup.subMapper = header[8] >> 4;
			}

			if (header[6] & 0x8U)
			{
				setup.mirroring = Header::MIRRORING_FOURSCREEN;
			}
			else if (header[6] & 0x1U)
			{
				setup.mirroring = Header::MIRRORING_VERTICAL;
			}
			else
			{
				setup.mirroring = Header::MIRRORING_HORIZONTAL;
			}

			setup.security = 0;

			if (header[7] & 0x1U)
			{
				setup.system = Header::SYSTEM_VS;
				setup.ppu = Header::PPU_RP2C03B;

				if (setup.version)
				{
					if ((header[13] & 0xFU) < 13)
						setup.ppu = static_cast<Header::Ppu>((header[13] & 0xFU) + 1);

					if ((header[13] >> 4) < 4)
						setup.security = header[13] >> 4;
				}
			}
			else if (setup.version && (header[7] & 0x2U))
			{
				setup.system = Header::SYSTEM_PC10;
				setup.ppu = Header::PPU_RP2C03B;
			}
			else
			{
				setup.system = Header::SYSTEM_CONSOLE;
				setup.ppu = Header::PPU_RP2C02;
			}

			if (setup.version && (header[12] & 0x2U))
			{
				setup.region = Header::REGION_BOTH;
			}
			else if (header[setup.version ? 12 : 9] & 0x1U)
			{
				if (setup.system == Header::SYSTEM_CONSOLE)
				{
					setup.region = Header::REGION_PAL;
					setup.ppu = Header::PPU_RP2C07;
				}
				else
				{
					setup.region = Header::REGION_NTSC;
				}
			}
			else
			{
				setup.region = Header::REGION_NTSC;
			}

			if (setup.version)
			{
				setup.prgRam   = ((header[10]) & 0xFU) - 1U < 14 ? 64UL << (header[10] & 0xFU) : 0;
				setup.prgNvRam = ((header[10]) >>   4) - 1U < 14 ? 64UL << (header[10] >>   4) : 0;
				setup.chrRam   = ((header[11]) & 0xFU) - 1U < 14 ? 64UL << (header[11] & 0xFU) : 0;
				setup.chrNvRam = ((header[11]) >>   4) - 1U < 14 ? 64UL << (header[11] >>   4) : 0;
			}
			else
			{
				setup.prgRam   = ((header[6] & 0x2U) ? 0 : header[8] * dword(SIZE_8K));
				setup.prgNvRam = ((header[6] & 0x2U) ? NST_MAX(header[8],1U) * dword(SIZE_8K) : 0);
				setup.chrRam   = (setup.chrRom ? 0 : SIZE_8K);
				setup.chrNvRam = 0;
			}

			return result;
		}

		Result Cartridge::Ines::WriteHeader(const Header& setup,byte* const file,const ulong length)
		{
			if
			(
				(file == NULL || length < 16) ||
				(setup.prgRom > (setup.version ? 0xFFFUL * SIZE_16K : 0xFFUL * SIZE_16K)) ||
				(setup.chrRom > (setup.version ? 0xFFFUL * SIZE_8K : 0xFFUL * SIZE_8K)) ||
				(setup.mapper > (setup.version ? 0x1FF : 0xFF)) ||
				(setup.version && setup.subMapper > 0xF)
			)
				return RESULT_ERR_INVALID_PARAM;

			byte header[16] =
			{
				Ascii<'N'>::V,
				Ascii<'E'>::V,
				Ascii<'S'>::V,
				0x1A,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00
			};

			if (setup.version)
				header[7] |= 0x8U;

			header[4] = (setup.prgRom / SIZE_16K) & 0xFF;
			header[5] = (setup.chrRom / SIZE_8K) & 0xFF;

			if (setup.version)
			{
				header[9] |= (setup.prgRom / SIZE_16K) >> 8;
				header[9] |= (setup.chrRom / SIZE_8K) >> 4 & 0xF0;
			}

			if (setup.mirroring == Header::MIRRORING_FOURSCREEN)
			{
				header[6] |= 0x8U;
			}
			else if (setup.mirroring == Header::MIRRORING_VERTICAL)
			{
				header[6] |= 0x1U;
			}

			if (setup.prgNvRam)
				header[6] |= 0x2U;

			if (setup.trainer)
				header[6] |= 0x4U;

			if (setup.system == Header::SYSTEM_VS)
			{
				header[7] |= 0x1U;
			}
			else if (setup.version && setup.system == Header::SYSTEM_PC10)
			{
				header[7] |= 0x2U;
			}

			header[6] |= setup.mapper << 4 & 0xF0U;
			header[7] |= setup.mapper & 0xF0U;

			if (setup.version)
			{
				header[8] |= setup.mapper >> 8;
				header[8] |= setup.subMapper << 4;

				uint i, data;

				for (i=0, data=setup.prgRam >> 7; data; data >>= 1, ++i)
				{
					if (i > 0xF)
						return RESULT_ERR_INVALID_PARAM;
				}

				header[10] |= i;

				for (i=0, data=setup.prgNvRam >> 7; data; data >>= 1, ++i)
				{
					if (i > 0xF)
						return RESULT_ERR_INVALID_PARAM;
				}

				header[10] |= i << 4;

				for (i=0, data=setup.chrRam >> 7; data; data >>= 1, ++i)
				{
					if (i > 0xF)
						return RESULT_ERR_INVALID_PARAM;
				}

				header[11] |= i;

				for (i=0, data=setup.chrNvRam >> 7; data; data >>= 1, ++i);
				{
					if (i > 0xF)
						return RESULT_ERR_INVALID_PARAM;
				}

				header[11] |= i << 4;

				if (setup.region == Header::REGION_BOTH)
				{
					header[12] |= 0x2U;
				}
				else if (setup.region == Header::REGION_PAL)
				{
					header[12] |= 0x1U;
				}

				if (setup.system == Header::SYSTEM_VS)
				{
					if (setup.ppu > 0xF || setup.security > 0xF)
						return RESULT_ERR_INVALID_PARAM;

					if (setup.ppu)
						header[13] = setup.ppu - 1;

					header[13] |= setup.security << 4;
				}
			}
			else
			{
				header[8] = (setup.prgRam + setup.prgNvRam) / SIZE_8K;
				header[9] = (setup.region == Header::REGION_PAL ? 0x1 : 0x0);
			}

			std::memcpy( file, header, 16 );

			return RESULT_OK;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
