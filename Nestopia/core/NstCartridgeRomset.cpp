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
#include <new>
#include <vector>
#include "NstLog.hpp"
#include "NstPatcher.hpp"
#include "NstXml.hpp"
#include "NstStream.hpp"
#include "NstChecksum.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeRomset.hpp"
#include "api/NstApiCartridge.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Cartridge::Romset::Loader
		{
			typedef std::vector<Profile> Profiles;

			enum
			{
				DEFAULT_VERSION  = 10,
				MIN_PLAYERS      = 1,
				MAX_PLAYERS      = 255,
				MIN_CHIP_SIZE    = 1,
				MAX_CHIP_SIZE    = SIZE_16384K,
				MIN_IC_PINS      = 1,
				MAX_IC_PINS      = 127,
				MAX_CHIP_SAMPLES = 255,
				MAX_MAPPER       = 255
			};

			std::istream& imageStream;
			std::istream* const patchStream;
			const FavoredSystem favoredSystem;
			Ram& prg;
			Ram& chr;
			Profile& profile;
			Profiles profiles;
			Result* const patchResult;
			const bool askProfile;
			const bool readOnly;
			const bool patchBypassChecksum;

		public:

			Loader
			(
				std::istream& i,
				std::istream* t,
				const bool b,
				Result* const e,
				Ram& p,
				Ram& c,
				const FavoredSystem f,
				const bool a,
				Profile& r,
				const bool o
			)
			:
			imageStream         (i),
			patchStream         (t),
			favoredSystem       (f),
			prg                 (p),
			chr                 (c),
			profile             (r),
			patchResult         (e),
			askProfile          (a),
			readOnly            (o),
			patchBypassChecksum (b)
			{
				NST_ASSERT( prg.Empty() && chr.Empty() );
			}

			void Load()
			{
				Collect();
				ChooseProfile();
				LoadRoms();
				PatchRoms();
			}

		private:

			void Collect()
			{
				Xml xml;

				if (!xml.Read( imageStream ))
					throw RESULT_ERR_INVALID_FILE;

				const Xml::Node romset( xml.GetRoot() );

				if (!romset.IsType( L"romset" ))
					throw RESULT_ERR_INVALID_FILE;

				uint version = DEFAULT_VERSION;

				if (const Xml::Attribute attribute=romset.GetAttribute( L"version" ))
				{
					wcstring const string = attribute.GetValue();

					if
					(
						(string[0] >= L'1' && string[0] <= L'9') &&
						(string[1] == L'.') &&
						(string[2] >= L'0' && string[2] <= L'9') &&
						(string[3] == L'\0')
					)
					{
						version = (string[0] - L'0') * 10 + (string[2] - L'0');
					}
					else
					{
						throw RESULT_ERR_INVALID_FILE;
					}
				}

				bool strict = true;

				if (const Xml::Attribute attribute=romset.GetAttribute( L"conformance" ))
				{
					if (attribute.IsValue( L"loose" ))
					{
						strict = false;
					}
					else if (version != DEFAULT_VERSION && !attribute.IsValue( L"strict" ))
					{
						throw RESULT_ERR_INVALID_FILE;
					}
				}

				profiles.reserve( 4 );

				for (Xml::Node game(xml.GetRoot().GetFirstChild()); game; game=game.GetNextSibling())
				{
					for (Xml::Node image(game.GetFirstChild()); image; image=image.GetNextSibling())
					{
						Profile p;

						p.game.title         = game.GetAttribute( L"name"          ).GetValue();
						p.game.altTitle      = game.GetAttribute( L"altname"       ).GetValue();
						p.game.clss          = game.GetAttribute( L"class"         ).GetValue();
						p.game.subClss       = game.GetAttribute( L"subclass"      ).GetValue();
						p.game.catalog       = game.GetAttribute( L"catalog"       ).GetValue();
						p.game.publisher     = game.GetAttribute( L"publisher"     ).GetValue();
						p.game.developer     = game.GetAttribute( L"developer"     ).GetValue();
						p.game.portDeveloper = game.GetAttribute( L"portdeveloper" ).GetValue();
						p.game.region        = game.GetAttribute( L"region"        ).GetValue();

						if (const Xml::Attribute attribute=game.GetAttribute( L"players" ))
						{
							const ulong players = attribute.GetUnsignedValue();

							if (players >= MIN_PLAYERS && players <= MAX_PLAYERS)
							{
								p.game.players = players;
							}
							else if (strict)
							{
								throw RESULT_ERR_INVALID_FILE;
							}
						}

						if (image.IsType( L"cartridge" ))
						{
							if (const Xml::Attribute attribute=image.GetAttribute( L"system" ))
							{
								if (attribute.IsValue( L"famicom" ))
								{
									p.system.type = Profile::System::FAMICOM;
								}
								else if (attribute.IsValue( L"nes-ntsc" ))
								{
									p.system.type = Profile::System::NES_NTSC;
								}
								else if (attribute.IsValue( L"nes-pal" ))
								{
									p.system.type = Profile::System::NES_PAL;
									p.system.cpu = Profile::System::CPU_RP2A07;
									p.system.ppu = Profile::System::PPU_RP2C07;
								}
								else if (attribute.IsValue( L"nes-pal-a" ))
								{
									p.system.type = Profile::System::NES_PAL_A;
									p.system.cpu = Profile::System::CPU_RP2A07;
									p.system.ppu = Profile::System::PPU_RP2C07;
								}
								else if (attribute.IsValue( L"nes-pal-b" ))
								{
									p.system.type = Profile::System::NES_PAL_B;
									p.system.cpu = Profile::System::CPU_RP2A07;
									p.system.ppu = Profile::System::PPU_RP2C07;
								}
								else if (!strict)
								{
									if (favoredSystem == FAVORED_NES_PAL)
									{
										p.system.type = Profile::System::NES_PAL;
										p.system.cpu = Profile::System::CPU_RP2A07;
										p.system.ppu = Profile::System::PPU_RP2C07;
									}
									else if (favoredSystem == FAVORED_FAMICOM)
									{
										p.system.type = Profile::System::FAMICOM;
									}
									else
									{
										p.system.type = Profile::System::NES_NTSC;
									}
								}
								else
								{
									throw RESULT_ERR_INVALID_FILE;
								}
							}
							else if (strict)
							{
								throw RESULT_ERR_INVALID_FILE;
							}
						}
						else if (image.IsType( L"arcade" ))
						{
							p.system.ppu = Profile::System::PPU_RP2C03B;

							if (const Xml::Attribute attribute=image.GetAttribute( L"system" ))
							{
								if (attribute.IsValue( L"vs-unisystem" ))
								{
									p.system.type = Profile::System::VS_UNISYSTEM;
								}
								else if (attribute.IsValue( L"vs-dualsystem" ))
								{
									p.system.type = Profile::System::VS_DUALSYSTEM;
								}
								else if (attribute.IsValue( L"playchoice-10" ))
								{
									p.system.type = Profile::System::PLAYCHOICE_10;
								}
								else
								{
									throw RESULT_ERR_INVALID_FILE;
								}
							}
							else
							{
								throw RESULT_ERR_INVALID_FILE;
							}
						}
						else
						{
							throw RESULT_ERR_INVALID_FILE;
						}

						p.dump.by = image.GetAttribute( L"dumper" ).GetValue();
						p.dump.date = image.GetAttribute( L"datedumped" ).GetValue();
						p.dump.state = Profile::Dump::OK;

						if (const Xml::Attribute attribute=image.GetAttribute( L"dump" ))
						{
							if (attribute.IsValue( L"bad" ))
							{
								p.dump.state = Profile::Dump::BAD;
							}
							else if (attribute.IsValue( L"unknown" ))
							{
								p.dump.state = Profile::Dump::UNKNOWN;
							}
							else if (strict && !attribute.IsValue( L"ok" ))
							{
								throw RESULT_ERR_INVALID_FILE;
							}
						}

						if (Xml::Node node=image.GetChild( L"properties" ))
						{
							for (node=node.GetFirstChild(); node.IsType( L"property" ); node=node.GetNextSibling())
							{
								Profile::Property property;

								property.name = node.GetAttribute( L"name" ).GetValue();
								property.value = node.GetAttribute( L"value" ).GetValue();

								p.properties.push_back( property );
							}
						}

						if (p.system.type == Profile::System::VS_UNISYSTEM || p.system.type == Profile::System::VS_DUALSYSTEM)
						{
							if (const Xml::Attribute attribute=image.GetAttribute( L"ppu" ))
							{
                                     if (attribute.IsValue( L"rp2c03b"     )) p.system.ppu = Profile::System::PPU_RP2C03B;
								else if (attribute.IsValue( L"rp2c03g"     )) p.system.ppu = Profile::System::PPU_RP2C03G;
								else if (attribute.IsValue( L"rp2c04-0001" )) p.system.ppu = Profile::System::PPU_RP2C04_0001;
								else if (attribute.IsValue( L"rp2c04-0002" )) p.system.ppu = Profile::System::PPU_RP2C04_0002;
								else if (attribute.IsValue( L"rp2c04-0003" )) p.system.ppu = Profile::System::PPU_RP2C04_0003;
								else if (attribute.IsValue( L"rp2c04-0004" )) p.system.ppu = Profile::System::PPU_RP2C04_0004;
								else if (attribute.IsValue( L"rc2c03b"     )) p.system.ppu = Profile::System::PPU_RC2C03B;
								else if (attribute.IsValue( L"rc2c03c"     )) p.system.ppu = Profile::System::PPU_RC2C03C;
								else if (attribute.IsValue( L"rc2c05-01"   )) p.system.ppu = Profile::System::PPU_RC2C05_01;
								else if (attribute.IsValue( L"rc2c05-02"   )) p.system.ppu = Profile::System::PPU_RC2C05_02;
								else if (attribute.IsValue( L"rc2c05-03"   )) p.system.ppu = Profile::System::PPU_RC2C05_03;
								else if (attribute.IsValue( L"rc2c05-04"   )) p.system.ppu = Profile::System::PPU_RC2C05_04;
								else if (attribute.IsValue( L"rc2c05-05"   )) p.system.ppu = Profile::System::PPU_RC2C05_05;
							}
						}

						p.game.revision = image.GetAttribute( L"revision" ).GetValue();

						p.hash.Assign
						(
							image.GetAttribute( L"sha1" ).GetValue(),
							image.GetAttribute( L"crc"  ).GetValue()
						);

						if (const Xml::Node board=image.GetChild( L"board" ))
						{
							p.board.type = board.GetAttribute( L"type" ).GetValue();
							p.board.pcb = board.GetAttribute( L"pcb"  ).GetValue();
							p.board.cic = board.GetChild( L"cic" ).GetAttribute( L"type" ).GetValue();

							if (const Xml::Attribute attribute=board.GetAttribute( L"mapper" ))
							{
								const ulong mapper = attribute.GetUnsignedValue();

								if (mapper <= MAX_MAPPER)
								{
									p.board.mapper = mapper;
								}
								else if (strict)
								{
									throw RESULT_ERR_INVALID_FILE;
								}
							}

							if (const Xml::Node pad=board.GetChild( L"pad" ))
							{
								p.board.solderPads =
								(
									(pad.GetAttribute( L"h" ).IsValue( L"1" ) ? Profile::Board::SOLDERPAD_H : 0U) |
									(pad.GetAttribute( L"v" ).IsValue( L"1" ) ? Profile::Board::SOLDERPAD_V : 0U)
								);
							}

							for (Xml::Node node=board.GetFirstChild(); node; node=node.GetNextSibling())
							{
								dword size = 0;

								if (const Xml::Attribute attribute=node.GetAttribute( L"size" ))
								{
									wcstring end;
									const ulong value = attribute.GetUnsignedValue( end, 10 );

									if (end[0] == L'\0')
									{
										size = value;
									}
									else if ((end[0] == L'k' || end[0] == L'K') && end[1] == L'\0' && value <= MAX_CHIP_SIZE/SIZE_1K)
									{
										size = value * SIZE_1K;
									}
								}

								Profile::Board::Pins pins;
								Profile::Board::Samples samples;

								for (Xml::Node child(node.GetFirstChild()); child; child=child.GetNextSibling())
								{
									if (child.IsType(L"pin"))
									{
										const ulong number = child.GetAttribute(L"number").GetUnsignedValue();
										wcstring const function = child.GetAttribute(L"function").GetValue();

										if (number >= MIN_IC_PINS && number <= MAX_IC_PINS && *function)
										{
											Profile::Board::Pin pin;

											pin.number = number;
											pin.function = function;

											pins.push_back( pin );
										}
									}
									else if (child.IsType(L"sample"))
									{
										const ulong id = child.GetAttribute(L"id").GetUnsignedValue();
										wcstring const file = child.GetAttribute(L"file").GetValue();

										if (id <= MAX_CHIP_SAMPLES && *file)
										{
											Profile::Board::Sample sample;

											sample.id = id;
											sample.file = file;

											samples.push_back( sample );
										}
									}
								}

								bool first;

								if (true == (first=node.IsType( L"prg" )) || node.IsType( L"chr" ))
								{
									if (size < MIN_CHIP_SIZE || size > MAX_CHIP_SIZE)
										throw RESULT_ERR_INVALID_FILE;

									Profile::Board::Roms& roms = (first ? p.board.prg : p.board.chr);

									Profile::Board::Rom rom;

									rom.id = node.GetAttribute( L"id" ).GetUnsignedValue();
									rom.size = size;
									rom.name = node.GetAttribute( L"name" ).GetValue();
									rom.pins = pins;
									rom.file = node.GetAttribute( L"file" ).GetValue();
									rom.package = node.GetAttribute( L"package" ).GetValue();
									rom.hash.Assign( node.GetAttribute( L"sha1" ).GetValue(), node.GetAttribute( L"crc" ).GetValue() );

									for (Profile::Board::Roms::iterator it(roms.begin()), end(roms.end()); ; ++it)
									{
										if (it == end || it->id > rom.id)
										{
											roms.insert( it, rom );
											break;
										}
									}
								}
								else if (true == (first=node.IsType( L"wram" )) || node.IsType( L"vram" ))
								{
									if (size < MIN_CHIP_SIZE || size > MAX_CHIP_SIZE)
										throw RESULT_ERR_INVALID_FILE;

									Profile::Board::Rams& rams = (first ? p.board.wram : p.board.vram);

									Profile::Board::Ram ram;

									ram.id = node.GetAttribute( L"id" ).GetUnsignedValue();
									ram.size = size;
									ram.file = node.GetAttribute( L"file" ).GetValue();
									ram.pins = pins;
									ram.package = node.GetAttribute( L"package" ).GetValue();
									ram.battery = node.GetAttribute( L"battery" ).IsValue( L"1" );

									for (Profile::Board::Rams::iterator it(rams.begin()), end(rams.end()); ; ++it)
									{
										if (it == end || it->id > ram.id)
										{
											rams.insert( it, ram );
											break;
										}
									}
								}
								else if (node.IsType( L"chip" ))
								{
									Profile::Board::Chip chip;

									chip.type = node.GetAttribute( L"type" ).GetValue();
									chip.file = node.GetAttribute( L"file" ).GetValue();
									chip.pins = pins;
									chip.samples = samples;
									chip.battery = node.GetAttribute( L"battery" ).IsValue( L"1" );
									chip.package = node.GetAttribute( L"package" ).GetValue();

									p.board.chips.push_back( chip );
								}
							}
						}

						if (Xml::Node device=game.GetChild( L"peripherals" ))
						{
							uint i = 0;

							for (device=device.GetFirstChild(); i < 4 && device.IsType( L"device" ); device=device.GetNextSibling())
							{
								if (const Xml::Attribute attribute = device.GetAttribute( L"type" ))
								{
									if (attribute.IsValue( L"arkanoid" ))
									{
										if (p.system.type == Profile::System::FAMICOM)
											p.game.controllers[4] = Api::Input::PADDLE;
										else
											p.game.controllers[1] = Api::Input::PADDLE;
									}
									else if (attribute.IsValue( L"bandaihypershot" ))
									{
										p.game.controllers[4] = Api::Input::BANDAIHYPERSHOT;
									}
									else if (attribute.IsValue( L"barcodeworld" ))
									{
										p.game.controllers[4] = Api::Input::BARCODEWORLD;
									}
									else if (attribute.IsValue( L"crazyclimber" ))
									{
										p.game.controllers[4] = Api::Input::CRAZYCLIMBER;
									}
									else if (attribute.IsValue( L"doremikko" ))
									{
										p.game.controllers[4] = Api::Input::DOREMIKKOKEYBOARD;
									}
									else if (attribute.IsValue( L"excitingboxing" ))
									{
										p.game.controllers[4] = Api::Input::EXCITINGBOXING;
									}
									else if (attribute.IsValue( L"familykeyboard" ))
									{
										p.game.controllers[4] = Api::Input::FAMILYKEYBOARD;
									}
									else if (attribute.IsValue( L"familytrainer" ))
									{
										p.game.controllers[1] = Api::Input::UNCONNECTED;
										p.game.controllers[4] = Api::Input::FAMILYTRAINER;
									}
									else if (attribute.IsValue( L"fourplayer" ))
									{
										if (p.system.type == Profile::System::FAMICOM)
											p.game.adapter = Api::Input::ADAPTER_FAMICOM;
										else
											p.game.adapter = Api::Input::ADAPTER_NES;

										p.game.controllers[2] = Api::Input::PAD3;
										p.game.controllers[3] = Api::Input::PAD4;
									}
									else if (attribute.IsValue( L"horitrack" ))
									{
										p.game.controllers[4] = Api::Input::HORITRACK;
									}
									else if (attribute.IsValue( L"konamihypershot" ))
									{
										p.game.controllers[0] = Api::Input::UNCONNECTED;
										p.game.controllers[1] = Api::Input::UNCONNECTED;
										p.game.controllers[4] = Api::Input::KONAMIHYPERSHOT;
									}
									else if (attribute.IsValue( L"mahjong" ))
									{
										p.game.controllers[0] = Api::Input::UNCONNECTED;
										p.game.controllers[1] = Api::Input::UNCONNECTED;
										p.game.controllers[4] = Api::Input::MAHJONG;
									}
									else if (attribute.IsValue( L"oekakidstablet" ))
									{
										p.game.controllers[0] = Api::Input::UNCONNECTED;
										p.game.controllers[1] = Api::Input::UNCONNECTED;
										p.game.controllers[4] = Api::Input::OEKAKIDSTABLET;
									}
									else if (attribute.IsValue( L"pachinko" ))
									{
										p.game.controllers[4] = Api::Input::PACHINKO;
									}
									else if (attribute.IsValue( L"partytap" ))
									{
										p.game.controllers[1] = Api::Input::UNCONNECTED;
										p.game.controllers[4] = Api::Input::PARTYTAP;
									}
									else if (attribute.IsValue( L"pokkunmoguraa" ))
									{
										p.game.controllers[1] = Api::Input::UNCONNECTED;
										p.game.controllers[4] = Api::Input::POKKUNMOGURAA;
									}
									else if (attribute.IsValue( L"powerglove" ))
									{
										p.game.controllers[0] = Api::Input::POWERGLOVE;
									}
									else if (attribute.IsValue( L"powerpad" ) || attribute.IsValue( L"familyfunfitness" ))
									{
										if (p.system.type == Profile::System::FAMICOM)
										{
											p.game.controllers[1] = Api::Input::UNCONNECTED;
											p.game.controllers[4] = Api::Input::FAMILYTRAINER;
										}
										else
										{
											p.game.controllers[1] = Api::Input::POWERPAD;
										}
									}
									else if (attribute.IsValue( L"rob" ))
									{
										p.game.controllers[1] = Api::Input::ROB;
									}
									else if (attribute.IsValue( L"suborkeyboard" ))
									{
										p.game.controllers[4] = Api::Input::SUBORKEYBOARD;
									}
									else if (attribute.IsValue( L"subormouse" ))
									{
										p.game.controllers[1] = Api::Input::MOUSE;
									}
									else if (attribute.IsValue( L"standard" ))
									{
										p.game.controllers[0] = Api::Input::PAD1;
										p.game.controllers[1] = Api::Input::PAD2;
									}
									else if (attribute.IsValue( L"topriderbike" ))
									{
										p.game.controllers[0] = Api::Input::UNCONNECTED;
										p.game.controllers[1] = Api::Input::UNCONNECTED;
										p.game.controllers[4] = Api::Input::TOPRIDER;
									}
									else if (attribute.IsValue( L"turbofile" ))
									{
										p.game.controllers[4] = Api::Input::TURBOFILE;
									}
									else if (attribute.IsValue( L"zapper" ))
									{
										if (p.system.type == Profile::System::VS_UNISYSTEM || p.system.type == Profile::System::VS_DUALSYSTEM)
										{
											p.game.controllers[0] = Api::Input::ZAPPER;
											p.game.controllers[1] = Api::Input::UNCONNECTED;
										}
										else
										{
											p.game.controllers[1] = Api::Input::ZAPPER;
										}
									}
								}
							}
						}

						profiles.push_back( p );
					}
				}
			}

			void ChooseProfile()
			{
				if (profiles.empty())
					throw RESULT_ERR_INVALID_FILE;

				Profiles::const_iterator bestMatch( profiles.begin() );

				if (profiles.size() > 1)
				{
					for (Profiles::const_iterator it(profiles.begin()), end(profiles.end()); it != end; ++it)
					{
						if (it->system.type == Profile::System::NES_NTSC)
						{
							if (favoredSystem == FAVORED_NES_NTSC)
							{
								bestMatch = it;
								break;
							}
						}
						else if
						(
							it->system.type == Profile::System::NES_PAL ||
							it->system.type == Profile::System::NES_PAL_A ||
							it->system.type == Profile::System::NES_PAL_B
						)
						{
							if (favoredSystem == FAVORED_NES_PAL)
							{
								bestMatch = it;
								break;
							}
						}
						else if (it->system.type == Profile::System::FAMICOM)
						{
							if (favoredSystem == FAVORED_FAMICOM)
							{
								bestMatch = it;
								break;
							}
						}
					}

					if (askProfile && Api::Cartridge::chooseProfileCallback)
					{
						std::vector<std::wstring> names( profiles.size() );

						std::vector<std::wstring>::iterator dst( names.begin() );
						for (Profiles::const_iterator src(profiles.begin()), end(profiles.end()); src != end; ++src, ++dst)
						{
							*dst = src->game.title;

							if (!src->game.revision.empty())
							{
								*dst += dst->empty() ? L"(Rev. " : L" (Rev. ";
								*dst += src->game.revision;
								*dst += L')';
							}

							if (!src->game.region.empty())
							{
								*dst += dst->empty() ? L"(" : L" (";
								*dst += src->game.region;
								*dst += L')';
							}

							if (!dst->empty())
								*dst += L' ';

							*dst +=
							(
								src->system.type == Profile::System::NES_PAL       ? L"(NES-PAL)" :
								src->system.type == Profile::System::NES_PAL_A     ? L"(NES-PAL-A)" :
								src->system.type == Profile::System::NES_PAL_B     ? L"(NES-PAL-B)" :
								src->system.type == Profile::System::FAMICOM       ? L"(Famicom)" :
								src->system.type == Profile::System::VS_UNISYSTEM  ? L"(VS)" :
								src->system.type == Profile::System::VS_DUALSYSTEM ? L"(VS)" :
								src->system.type == Profile::System::PLAYCHOICE_10 ? L"(PC10)" :
                                                                                     L"(NES-NTSC)"
							);
						}

						const uint selected = Api::Cartridge::chooseProfileCallback( &profiles.front(), &names.front(), profiles.size() );

						if (selected < profiles.size())
							bestMatch = profiles.begin() + selected;
					}
				}

				profile = *bestMatch;

				if (profiles.size() > 1)
				{
					uint regions = 0x0;
					for (Profiles::const_iterator it(profiles.begin()), end(profiles.end()); it != end; ++it)
					{
						if (profile.hash == it->hash)
						{
							switch (it->system.type)
							{
								case Profile::System::NES_PAL:
								case Profile::System::NES_PAL_A:
								case Profile::System::NES_PAL_B:

									regions |= 0x1;
									break;

								default:

									regions |= 0x2;
									break;
							}
						}

						if (regions == 0x3)
						{
							profile.multiRegion = true;
							break;
						}
					}
				}
			}

			void LoadRoms()
			{
				for (uint i=0; i < 2; ++i)
				{
					Profile::Board::Roms& roms = (i ? profile.board.chr : profile.board.prg);

					if (roms.empty())
						continue;

					Ram& rom = (i ? chr : prg);

					dword size = 0;

					for (Profile::Board::Roms::iterator it(roms.begin()), end(roms.end()); it != end; ++it)
					{
						size += it->size;

						if (it->size < MIN_CHIP_SIZE || size > MAX_CHIP_SIZE)
							throw RESULT_ERR_INVALID_FILE;
					}

					rom.Set( size );

					for (Profile::Board::Pins::const_iterator pin(roms.begin()->pins.begin()), end(roms.begin()->pins.end()); pin != end; ++pin)
						rom.Pin(pin->number) = pin->function.c_str();

					if (readOnly)
						continue;

					if (!Api::User::fileIoCallback)
						throw RESULT_ERR_NOT_READY;

					size = 0;

					for (Profile::Board::Roms::iterator it(roms.begin()), end(roms.end()); it != end; ++it)
					{
						if (it->file.empty())
							throw RESULT_ERR_INVALID_FILE;

						class Loader : public Api::User::File
						{
							wcstring const filename;
							byte* const rom;
							const dword size;
							bool loaded;

							Action GetAction() const throw()
							{
								return LOAD_ROM;
							}

							wcstring GetName() const throw()
							{
								return filename;
							}

							ulong GetMaxSize() const throw()
							{
								return size;
							}

							Result SetContent(const void* filedata,ulong filesize) throw()
							{
								if (filesize)
								{
									if (filedata)
									{
										std::memcpy( rom, filedata, NST_MIN(size,filesize) );
										loaded = true;
									}
									else
									{
										return RESULT_ERR_INVALID_PARAM;
									}
								}

								return RESULT_OK;
							}

							Result SetContent(std::istream& stdStream) throw()
							{
								try
								{
									Stream::In stream( &stdStream );

									if (const ulong length = stream.Length())
									{
										stream.Read( rom, NST_MIN(size,length) );
										loaded = true;
									}
								}
								catch (Result result)
								{
									return result;
								}
								catch (const std::bad_alloc&)
								{
									return RESULT_ERR_OUT_OF_MEMORY;
								}
								catch (...)
								{
									return RESULT_ERR_GENERIC;
								}

								return RESULT_OK;
							}

						public:

							Loader(wcstring f,byte* r,dword s)
							: filename(f), rom(r), size(s), loaded(false) {}

							bool Loaded() const
							{
								return loaded;
							}
						};

						Loader loader( it->file.c_str(), rom.Mem(size), it->size );
						Api::User::fileIoCallback( loader );

						if (!loader.Loaded())
							throw RESULT_ERR_INVALID_FILE;

						size += it->size;
					}
				}
			}

			void PatchRoms()
			{
				if (patchStream)
				{
					Patcher patcher( patchBypassChecksum );

					*patchResult = patcher.Load( *patchStream );

					if (NES_SUCCEEDED(*patchResult))
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
								Log::Flush( "Romset: PRG-ROM was patched" NST_LINEBREAK );
							}

							if (patcher.Patch( chr.Mem(), chr.Mem(), chr.Size(), 16 + prg.Size() ))
							{
								profile.patched = true;
								Log::Flush( "Romset: CHR-ROM was patched" NST_LINEBREAK );
							}
						}
					}
				}
			}
		};

		void Cartridge::Romset::Load
		(
			std::istream& stdStreamImage,
			std::istream* const stdStreamPatch,
			const bool patchBypassChecksum,
			Result* const patchResult,
			Ram& prg,
			Ram& chr,
			const FavoredSystem favoredSystem,
			const bool askProfile,
			Profile& profile,
			const bool readOnly
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
				askProfile,
				profile,
				readOnly
			);

			loader.Load();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
