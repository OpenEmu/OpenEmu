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
#include "NstCartridgeUnif.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Cartridge::Unif::Loader
		{
			class Context
			{
			public:

				bool operator () (uint,dword);

				enum System
				{
					SYSTEM_NTSC,
					SYSTEM_PAL,
					SYSTEM_BOTH
				};

				struct Rom
				{
					Rom();

					Ram data;
					dword truncated;
					char crc[12];
				};

				System system;
				Rom roms[2][16];

			private:

				byte chunks[80];

			public:

				Context()
				: system(SYSTEM_NTSC)
				{
					std::memset( chunks, 0, sizeof(chunks) );
				}
			};

			enum
			{
				HEADER_RESERVED_LENGTH = 24,
				MAX_ROM_SIZE = SIZE_16K * 0x1000UL,
				DUMPER_NAME_LENGTH = 100,
				DUMPER_AGENT_LENGTH = 100,
				DUMPER_LENGTH = DUMPER_NAME_LENGTH + 4 + DUMPER_AGENT_LENGTH
			};

			dword ReadString(cstring,Vector<char>*);

			static cstring ChunkName (char (&)[5],dword);

			Stream::In stream;
			const FavoredSystem favoredSystem;
			Profile& profile;
			ProfileEx& profileEx;
			Ram& prg;
			Ram& chr;
			Patcher patcher;
			Result* const patchResult;
			const ImageDatabase* const database;

		public:

			Loader
			(
				std::istream& stdStreamImage,
				std::istream* const stdStreamPatch,
				const bool patchBypassChecksum,
				Result* const e,
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
			patcher       (patchBypassChecksum),
			patchResult   (e),
			database      (d)
			{
				NST_ASSERT( prg.Empty() && chr.Empty() );

				profile = Profile();
				profileEx = ProfileEx();

				if (stdStreamPatch)
					*patchResult = patcher.Load( *stdStreamPatch );
			}

			void Load()
			{
				ReadHeader();
				ReadChunks();

				if (database && database->Enabled())
				{
					Checksum checksum;

					checksum.Compute( prg.Mem(), prg.Size() );
					checksum.Compute( chr.Mem(), chr.Size() );

					if (const ImageDatabase::Entry entry = database->Search( Profile::Hash(checksum.GetSha1(),checksum.GetCrc()), favoredSystem ))
						entry.Fill( profile, patcher.Empty() );
				}

				if (!patcher.Empty())
				{
					const Patcher::Block blocks[] =
					{
						{ prg.Mem(), prg.Size() },
						{ chr.Mem(), chr.Size() }
					};

					*patchResult = patcher.Test( blocks );

					if (NES_SUCCEEDED(*patchResult))
					{
						if (patcher.Patch( prg.Mem(), prg.Mem(), prg.Size(), 16 ))
						{
							profile.patched = true;
							Log::Flush( "Unif: PRG-ROM was patched" NST_LINEBREAK );
						}

						if (patcher.Patch( chr.Mem(), chr.Mem(), chr.Size(), 16 + prg.Size() ))
						{
							profile.patched = true;
							Log::Flush( "Unif: CHR-ROM was patched" NST_LINEBREAK );
						}
					}
				}
			}

		private:

			void ReadHeader()
			{
				if (stream.Read32() != AsciiId<'U','N','I','F'>::V)
					throw RESULT_ERR_INVALID_FILE;

				dword version = stream.Read32();

				Log() << "Unif: revision " << version << NST_LINEBREAK;

				byte reserved[HEADER_RESERVED_LENGTH];
				stream.Read( reserved );

				for (uint i=0; i < HEADER_RESERVED_LENGTH; ++i)
				{
					NST_VERIFY( !reserved[i] );

					if (reserved[i])
					{
						Log() << "Unif: warning, unknown header data" NST_LINEBREAK;
						break;
					}
				}
			}

			void ReadChunks()
			{
				Context context;

				while (!stream.Eof())
				{
					dword id = stream.Read32();
					const dword length = stream.Read32();
					NST_VERIFY( length <= SIZE_1K * 4096UL );

					switch (id)
					{
						case AsciiId<'N','A','M','E'>::V: id = (context( 0, id ) ? ReadName       (         ) : 0); break;
						case AsciiId<'R','E','A','D'>::V: id = (context( 1, id ) ? ReadComment    (         ) : 0); break;
						case AsciiId<'D','I','N','F'>::V: id = (context( 2, id ) ? ReadDumper     (         ) : 0); break;
						case AsciiId<'T','V','C','I'>::V: id = (context( 3, id ) ? ReadSystem     ( context ) : 0); break;
						case AsciiId<'B','A','T','R'>::V: id = (context( 4, id ) ? ReadBattery    (         ) : 0); break;
						case AsciiId<'M','A','P','R'>::V: id = (context( 5, id ) ? ReadBoard      (         ) : 0); break;
						case AsciiId<'M','I','R','R'>::V: id = (context( 6, id ) ? ReadMirroring  (         ) : 0); break;
						case AsciiId<'C','T','R','L'>::V: id = (context( 7, id ) ? ReadController (         ) : 0); break;
						case AsciiId<'V','R','O','R'>::V: id = (context( 8, id ) ? ReadChrRam     (         ) : 0); break;

						default: switch (id & 0x00FFFFFF)
						{
							case AsciiId<'P','C','K'>::V:
							case AsciiId<'C','C','K'>::V:
							case AsciiId<'P','R','G'>::V:
							case AsciiId<'C','H','R'>::V:
							{
								uint index = id >> 24 & 0xFF;

								if (index >= Ascii<'0'>::V && index <= Ascii<'9'>::V)
								{
									index -= Ascii<'0'>::V;
								}
								else if (index >= Ascii<'A'>::V && index <= Ascii<'F'>::V)
								{
									index = index - Ascii<'A'>::V + 10;
								}
								else
								{
									index = ~0U;
								}

								if (index < 16)
								{
									switch (dword part = (id & 0x00FFFFFF))
									{
										case AsciiId<'P','C','K'>::V:
										case AsciiId<'C','C','K'>::V:

											part = (part == AsciiId<'C','C','K'>::V);
											id = (context( 9 + (part << 4) + index, id) ? ReadChecksum( part, index, context.roms[part][index] ) : 0);
											break;

										case AsciiId<'P','R','G'>::V:
										case AsciiId<'C','H','R'>::V:

											part = (part == AsciiId<'C','H','R'>::V);
											id = (context( 9 + 32 + (part << 4) + index, id ) ? ReadRom( part, index, length, context.roms[part] ) : 0);
											break;
									}

									break;
								}
							}

							default:

								id = ReadUnknown( id );
								break;
						}
					}

					if (id < length)
					{
						for (id = length - id; id > 0x7FFFFFFF; id -= 0x7FFFFFFF)
							stream.Seek( 0x7FFFFFFF );

						if (id)
							stream.Seek( id );
					}
					else if (id > length)
					{
						throw RESULT_ERR_CORRUPT_FILE;
					}
				}

				for (uint i=0; i < 2; ++i)
				{
					uint count = 0;
					dword size = 0;

					for (uint j=0; j < 16; ++j)
					{
						if (const dword n=context.roms[i][j].data.Size())
						{
							count++;
							size += n;
						}
					}

					if (count)
					{
						Profile::Board::Roms& rom = (i ? profile.board.chr : profile.board.prg);
						rom.resize( count );

						Ram& dst = (i ? chr : prg);
						dst.Set( size );

						if (!rom.empty())
						{
							for (Profile::Board::Pins::const_iterator it(rom.front().pins.begin()), end(rom.front().pins.end()); it != end; ++it)
								dst.Pin(it->number) = it->function.c_str();
						}

						size = 0;

						for (uint j=0, k=0; j < 16; ++j)
						{
							const Context::Rom& src = context.roms[i][j];

							if (src.data.Size())
							{
								rom[k].id = k;
								rom[k].size = src.data.Size();
								rom[k].hash.Assign( NULL, src.crc );
								k++;

								std::memcpy( dst.Mem(size), src.data.Mem(), src.data.Size() );
								size += src.data.Size();
							}
						}
					}
				}

				if (profileEx.nmt == ProfileEx::NMT_HORIZONTAL)
				{
					profile.board.solderPads = Profile::Board::SOLDERPAD_V;
				}
				else if (profileEx.nmt == ProfileEx::NMT_HORIZONTAL)
				{
					profile.board.solderPads = Profile::Board::SOLDERPAD_H;
				}

				switch (context.system)
				{
					case Context::SYSTEM_NTSC:

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

					case Context::SYSTEM_PAL:

						profile.system.type = Profile::System::NES_PAL;
						profile.system.cpu = Profile::System::CPU_RP2A07;
						profile.system.ppu = Profile::System::PPU_RP2C07;
						break;
				}
			}

			dword ReadName()
			{
				Vector<char> buffer;
				const dword length = ReadString( "Unif: name: ", &buffer );

				if (length && *buffer.Begin())
					profile.game.title.assign( buffer.Begin(), buffer.End() );

				return length;
			}

			dword ReadComment()
			{
				return ReadString( "Unif: comment: ", NULL );
			}

			dword ReadDumper()
			{
				struct
				{
					char name[DUMPER_NAME_LENGTH];
					byte day;
					byte month;
					word year;
					char agent[DUMPER_AGENT_LENGTH];
				}   dumper;

				stream.Read( dumper.name, DUMPER_NAME_LENGTH );
				dumper.name[DUMPER_NAME_LENGTH-1] = '\0';

				dumper.day   = stream.Read8();
				dumper.month = stream.Read8();
				dumper.year  = stream.Read16();

				stream.Read( dumper.agent, DUMPER_AGENT_LENGTH );
				dumper.agent[DUMPER_AGENT_LENGTH-1] = '\0';

				Log log;

				if (*dumper.name)
					log << "Unif: dumped by: " << dumper.name << NST_LINEBREAK;

				log << "Unif: dump year: "  << dumper.year << NST_LINEBREAK
                       "Unif: dump month: " << dumper.month << NST_LINEBREAK
                       "Unif: dump day: "   << dumper.day << NST_LINEBREAK;

				if (*dumper.agent)
					log << "Unif: dumper agent: " << dumper.agent << NST_LINEBREAK;

				return DUMPER_LENGTH;
			}

			dword ReadSystem(Context& context)
			{
				switch (stream.Read8())
				{
					case 0:

						context.system = Context::SYSTEM_NTSC;
						Log::Flush( "Unif: NTSC system" NST_LINEBREAK );
						break;

					case 1:

						context.system = Context::SYSTEM_PAL;
						Log::Flush( "Unif: PAL system" NST_LINEBREAK );
						break;

					default:

						context.system = Context::SYSTEM_BOTH;
						Log::Flush( "Unif: dual system" NST_LINEBREAK );
						break;
				}

				return 1;
			}

			dword ReadBattery()
			{
				profileEx.battery = true;
				Log::Flush( "Unif: battery present" NST_LINEBREAK );
				return 0;
			}

			dword ReadBoard()
			{
				Vector<char> buffer;
				const dword length = ReadString( "Unif: board: ", &buffer );

				if (length && *buffer.Begin())
					profile.board.type.assign( buffer.Begin(), buffer.End() );

				return length;
			}

			dword ReadMirroring()
			{
				switch (stream.Read8())
				{
					case 0: profileEx.nmt = ProfileEx::NMT_HORIZONTAL;   Log::Flush( "Unif: horizontal mirroring"        NST_LINEBREAK ); break;
					case 1: profileEx.nmt = ProfileEx::NMT_VERTICAL;     Log::Flush( "Unif: vertical mirroring"          NST_LINEBREAK ); break;
					case 2:
					case 3: profileEx.nmt = ProfileEx::NMT_SINGLESCREEN; Log::Flush( "Unif: single-screen mirroring"     NST_LINEBREAK ); break;
					case 4: profileEx.nmt = ProfileEx::NMT_FOURSCREEN;   Log::Flush( "Unif: four-screen mirroring"       NST_LINEBREAK ); break;
					case 5: profileEx.nmt = ProfileEx::NMT_CONTROLLED;   Log::Flush( "Unif: mapper controlled mirroring" NST_LINEBREAK ); break;
				}

				return 1;
			}

			dword ReadChecksum(const uint type,const uint index,Context::Rom& rom)
			{
				NST_ASSERT( type < 2 && index < 16 );

				for (dword crc=stream.Read32(), i=0; i < 8; ++i)
				{
					uint c = crc >> (i*4) & 0xF;
					rom.crc[i] = (c < 0xA ? '0' + c : 'A' + (c - 0xA) );
				}

				Log() << "Unif: "
                      << (type ? "CHR-ROM " : "PRG-ROM ")
                      << char(index < 10 ? index + '0' : index-10 + 'A')
                      << " CRC: "
                      << rom.crc
                      << NST_LINEBREAK;

				return 4;
			}

			dword ReadRom(const uint type,const uint index,dword length,Context::Rom* const roms)
			{
				NST_ASSERT( type < 2 && index < 16 );

				Log() << "Unif: "
                      << (type ? "CHR-ROM " : "PRG-ROM ")
                      << char(index < 10 ? index + '0' : index-10 + 'A')
                      << " size: "
                      << (length / SIZE_1K)
                      << "k" NST_LINEBREAK;

				dword available = 0;

				for (uint i=0; i < 16; ++i)
					available += roms[i].data.Size();

				available = MAX_ROM_SIZE - available;
				NST_VERIFY( length <= available );

				if (length > available)
				{
					roms[index].truncated = length - available;
					length = available;

					Log() << "Unif: warning, "
                          << (type ? "CHR-ROM " : "PRG-ROM ")
                          << char(index < 10 ? index + '0' : index-10 + 'A')
                          << " truncated to: "
                          << (length / SIZE_1K)
                          << "k" NST_LINEBREAK;
				}

				if (length)
				{
					roms[index].data.Set( length );
					stream.Read( roms[index].data.Mem(), length );
				}

				return length;
			}

			dword ReadController()
			{
				Log log;

				log << "Unif: controllers: ";

				const uint controller = stream.Read8();
				NST_VERIFY( !(controller & (0x40|0x80)) );

				if (controller & (0x1|0x2|0x4|0x8|0x10|0x20))
				{
					if (controller & 0x01)
					{
						profile.game.controllers[0] = Api::Input::PAD1;
						profile.game.controllers[1] = Api::Input::PAD2;

						log << "standard joypad";
					}

					if (controller & 0x02)
					{
						profile.game.controllers[1] = Api::Input::ZAPPER;

						cstring const zapper = ", zapper";
						log << (zapper + ((controller & 0x1) ? 0 : 2));
					}

					if (controller & 0x04)
					{
						profile.game.controllers[1] = Api::Input::ROB;

						cstring const rob = ", R.O.B.";
						log << (rob + ((controller & (0x1|0x2)) ? 0 : 2));
					}

					if (controller & 0x08)
					{
						profile.game.controllers[0] = Api::Input::PADDLE;

						cstring const paddle = ", paddle";
						log << (paddle + ((controller & (0x1|0x2|0x4)) ? 0 : 2));
					}

					if (controller & 0x10)
					{
						profile.game.controllers[1] = Api::Input::POWERPAD;

						cstring const powerpad = ", power pad";
						log << (powerpad + ((controller & (0x1|0x2|0x4|0x8)) ? 0 : 2));
					}

					if (controller & 0x20)
					{
						profile.game.controllers[2] = Api::Input::PAD3;
						profile.game.controllers[3] = Api::Input::PAD4;

						cstring const fourplayer = ", four player adapter";
						log << (fourplayer + ((controller & (0x1|0x2|0x4|0x8|0x10)) ? 0 : 2));
					}

					log << NST_LINEBREAK;
				}
				else
				{
					log << ((controller & (0x40|0x80)) ? "unknown" NST_LINEBREAK : "unspecified" NST_LINEBREAK);
				}

				return 1;
			}

			static dword ReadChrRam()
			{
				Log::Flush( "Unif: CHR is writable" NST_LINEBREAK );
				return 0;
			}

			static dword ReadUnknown(dword id)
			{
				NST_DEBUG_MSG("unknown unif chunk");

				char name[5];
				Log() << "Unif: warning, skipping unknown chunk: \"" << ChunkName(name,id) << "\"" NST_LINEBREAK;

				return 0;
			}
		};

		void Cartridge::Unif::Load
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

		Cartridge::Unif::Loader::Context::Rom::Rom()
		: truncated(0)
		{
			for (uint i=0; i < sizeof(crc); ++i)
				crc[i] = '\0';
		}

		bool Cartridge::Unif::Loader::Context::operator () (const uint id,const dword chunk)
		{
			NST_VERIFY( chunks[id] == 0 );

			if (chunks[id] == 0)
			{
				chunks[id] = 1;
				return true;
			}
			else
			{
				char name[5];
				Log() << "Unif: warning, duplicate chunk: \"" << ChunkName(name,chunk) << "\" ignored" NST_LINEBREAK;

				return false;
			}
		}

		dword Cartridge::Unif::Loader::ReadString(cstring const logtext,Vector<char>* string)
		{
			Vector<char> tmp;

			if (string == NULL)
				string = &tmp;

			const dword count = stream.Read( *string );

			if (string->Size() > 1)
				Log() << logtext << string->Begin() << NST_LINEBREAK;

			return count;
		}

		cstring Cartridge::Unif::Loader::ChunkName(char (&name)[5],const dword id)
		{
			const byte bytes[] =
			{
				id >>  0 & 0xFF,
				id >>  8 & 0xFF,
				id >> 16 & 0xFF,
				id >> 24 & 0xFF,
				0
			};

			Stream::In::AsciiToC( name, bytes, 5 );

			return name;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
