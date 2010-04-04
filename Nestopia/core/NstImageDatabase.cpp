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
#include <cwchar>
#include <new>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include "NstLog.hpp"
#include "NstImageDatabase.hpp"
#include "NstXml.hpp"

namespace Nes
{
	namespace Core
	{
		class ImageDatabase::Item
		{
		public:

			~Item();

			enum
			{
				PERIPHERAL_UNSPECIFIED,
				PERIPHERAL_STANDARD,
				PERIPHERAL_ZAPPER,
				PERIPHERAL_POWERPAD,
				PERIPHERAL_FAMILYTRAINER,
				PERIPHERAL_ARKANOID,
				PERIPHERAL_FOURPLAYER,
				PERIPHERAL_3DGLASSES,
				PERIPHERAL_MIRACLEPIANO,
				PERIPHERAL_SUBORKEYBOARD,
				PERIPHERAL_SUBORMOUSE,
				PERIPHERAL_FAMILYKEYBOARD,
				PERIPHERAL_PARTYTAP,
				PERIPHERAL_CRAZYCLIMBER,
				PERIPHERAL_EXCITINGBOXING,
				PERIPHERAL_KONAMIHYPERSHOT,
				PERIPHERAL_BANDAIHYPERSHOT,
				PERIPHERAL_POKKUNMOGURAA,
				PERIPHERAL_OEKAKIDSTABLET,
				PERIPHERAL_MAHJONG,
				PERIPHERAL_ROB,
				PERIPHERAL_POWERGLOVE,
				PERIPHERAL_HORITRACK,
				PERIPHERAL_PACHINKO,
				PERIPHERAL_TOPRIDERBIKE,
				PERIPHERAL_DOREMIKKO,
				PERIPHERAL_TURBOFILE,
				PERIPHERAL_BARCODEWORLD,
				MAX_PERIPHERALS = 4
			};

		private:

			class String
			{
				union
				{
					mutable wcstring string;
					dword id;
				};

			public:

				explicit String(dword i=0)
				: id(i) {}

				bool operator == (const String& s) const
				{
					return id == s.id;
				}

				bool operator < (const String& s) const
				{
					return id < s.id;
				}

				void operator << (wcstring s) const
				{
					string = s + id;
				}

				operator wcstring () const
				{
					return string;
				}
			};

		public:

			struct Ic
			{
				struct Pin
				{
					uint number;
					String function;

					Pin() {}

					Pin(uint n,dword s)
					: number(n), function(s) {}
				};

				typedef std::vector<Pin> Pins;

				String package;
				Pins pins;

				Ic() {}

				Ic(dword p,const Pins& i)
				: package(p), pins(i) {}
			};

			struct Rom : Ic
			{
				dword id;
				String name;
				dword size;
				Hash hash;

				Rom() {}

				Rom(dword i,dword n,dword s,dword e,const Pins& p,const Hash& c)
				: Ic(e,p), id(i), name(n), size(s), hash(c) {}
			};

			typedef std::vector<Rom> Roms;

			struct Ram : Ic
			{
				dword id;
				dword size;
				bool battery;

				Ram() {}

				Ram(dword i,dword s,bool b,dword e,const Pins& p)
				: Ic(e,p), id(i), size(s), battery(b) {}
			};

			typedef std::vector<Ram> Rams;

			struct Chip : Ic
			{
				String type;
				bool battery;

				Chip() {}

				Chip(dword id,bool b,dword e,const Pins& p)
				: Ic(e,p), type(id), battery(b) {}

				bool operator == (const Chip& chip) const
				{
					return type == chip.type;
				}

				bool operator < (const Chip& chip) const
				{
					return type < chip.type;
				}
			};

			typedef std::vector<Chip> Chips;

			struct Dump
			{
				const String by;
				const String date;
				const Profile::Dump::State state;

				Dump(dword b,dword d,Profile::Dump::State o)
				: by(b), date(d), state(o) {}
			};

			struct Property
			{
				String name;
				String value;

				Property() {}

				Property(dword n,dword v)
				: name(n), value(v) {}
			};

			typedef std::vector<Property> Properties;

		private:

			const Hash hash;
			Item* sibling;
			const Dump dump;
			const String title;
			const String altTitle;
			const String clss;
			const String subClss;
			const String catalog;
			const String publisher;
			const String developer;
			const String portDeveloper;
			const String region;
			const String revision;
			const String pcb;
			const String board;
			const String cic;
			const Roms prg;
			const Roms chr;
			const Rams wram;
			const Rams vram;
			Chips chips;
			const Properties properties;
			byte peripherals[MAX_PERIPHERALS];
			const word mapper;
			const byte solderPads;
			const byte system;
			const byte cpu;
			const byte ppu;
			const byte players;
			bool multiRegion;

		public:

			Item
			(
				const Hash& hashIn,
				dword dumpBy,
				dword dumpDate,
				Profile::Dump::State dumpState,
				dword titleIn,
				dword altTitleIn,
				dword clssIn,
				dword subClssIn,
				dword catalogIn,
				dword publisherIn,
				dword developerIn,
				dword portDeveloperIn,
				dword regionIn,
				const Properties& propertiesIn,
				dword playersIn,
				const byte (&e)[MAX_PERIPHERALS],
				Profile::System::Type systemIn,
				Profile::System::Cpu cpuIn,
				Profile::System::Ppu ppuIn,
				dword revisionIn,
				dword boardIn,
				dword pcbIn,
				uint mapperIn,
				const Roms& prgIn,
				const Roms& chrIn,
				const Rams& wramIn,
				const Rams& vramIn,
				const Chips& chipsIn,
				dword cicIn,
				uint solderPadsIn
			)
			:
			hash          ( hashIn                      ),
			sibling       ( NULL                        ),
			dump          ( dumpBy, dumpDate, dumpState ),
			title         ( titleIn                     ),
			altTitle      ( altTitleIn                  ),
			clss          ( clssIn                      ),
			subClss       ( subClssIn                   ),
			catalog       ( catalogIn                   ),
			publisher     ( publisherIn                 ),
			developer     ( developerIn                 ),
			portDeveloper ( portDeveloperIn             ),
			region        ( regionIn                    ),
			revision      ( revisionIn                  ),
			pcb           ( pcbIn                       ),
			board         ( boardIn                     ),
			cic           ( cicIn                       ),
			prg           ( prgIn                       ),
			chr           ( chrIn                       ),
			wram          ( wramIn                      ),
			vram          ( vramIn                      ),
			chips         ( chipsIn                     ),
			properties    ( propertiesIn                ),
			mapper        ( mapperIn                    ),
			solderPads    ( solderPadsIn                ),
			system        ( systemIn                    ),
			cpu           ( cpuIn                       ),
			ppu           ( ppuIn                       ),
			players       ( playersIn                   ),
			multiRegion   ( false                       )
			{
				for (uint i=0; i < MAX_PERIPHERALS; ++i)
					peripherals[i] = e[i];

				std::sort( chips.begin(), chips.end() );
			}

		private:

			template<typename T>
			static dword GetMemSize(const T& t)
			{
				dword size = 0;

				for (typename T::const_iterator it(t.begin()), end(t.end()); it != end; ++it)
					size += it->size;

				return size;
			}

			template<typename T>
			static bool HasBattery(const T& t)
			{
				for (typename T::const_iterator it(t.begin()), end(t.end()); it != end; ++it)
				{
					if (it->battery)
						return true;
				}

				return false;
			}

		public:

			wcstring GetTitle() const
			{
				return title;
			}

			wcstring GetPublisher() const
			{
				return publisher;
			}

			wcstring GetDeveloper() const
			{
				return developer;
			}

			wcstring GetRegion() const
			{
				return region;
			}

			wcstring GetRevision() const
			{
				return revision;
			}

			wcstring GetPcb() const
			{
				return pcb;
			}

			wcstring GetBoard() const
			{
				return board;
			}

			wcstring GetCic() const
			{
				return cic;
			}

			uint NumPlayers() const
			{
				return players;
			}

			uint GetMapper() const
			{
				return mapper;
			}

			uint GetSolderPads() const
			{
				return solderPads;
			}

			Profile::System::Type GetSystem() const
			{
				return static_cast<Profile::System::Type>(system);
			}

			Profile::Dump::State GetDumpState() const
			{
				return dump.state;
			}

			const Hash& GetHash() const
			{
				return hash;
			}

			dword GetPrgSize() const
			{
				return GetMemSize( prg );
			}

			dword GetChrSize() const
			{
				return GetMemSize( chr );
			}

			dword GetWramSize() const
			{
				return GetMemSize( wram );
			}

			dword GetVramSize() const
			{
				return GetMemSize( vram );
			}

			bool HasVRamBattery() const
			{
				return HasBattery( vram );
			}

			bool HasWRamBattery() const
			{
				return HasBattery( wram );
			}

			bool HasChipBattery() const
			{
				return HasBattery( chips );
			}

			bool HasBattery() const
			{
				return HasWRamBattery() || HasVRamBattery() || HasChipBattery();
			}

			const Item* GetNextSibling() const
			{
				return sibling;
			}

			bool IsMultiRegion() const
			{
				return multiRegion;
			}

			void Fill(Profile& profile,const bool full) const
			{
				if (full)
				{
					if (*dump.by)
						profile.dump.by = dump.by;

					if (*dump.date)
						profile.dump.date = dump.date;

					if (dump.state != Profile::Dump::UNKNOWN)
						profile.dump.state = dump.state;

					if (*title)
						profile.game.title = title;

					if (*altTitle)
						profile.game.altTitle = altTitle;

					if (*clss)
						profile.game.clss = clss;

					if (*subClss)
						profile.game.subClss = subClss;

					if (*catalog)
						profile.game.catalog = catalog;

					if (*publisher)
						profile.game.publisher = publisher;

					if (*developer)
						profile.game.developer = developer;

					if (*portDeveloper)
						profile.game.portDeveloper = portDeveloper;

					if (*region)
						profile.game.region = region;

					if (*revision)
						profile.game.revision = revision;

					if (players)
						profile.game.players = players;

					if (*cic)
						profile.board.cic = cic;

					if (*pcb)
						profile.board.pcb = pcb;

					if (const dword size = properties.size())
					{
						profile.properties.resize( size );

						Profile::Properties::iterator b(profile.properties.begin());
						for (Properties::const_iterator a(properties.begin()), end(properties.end()); a != end; ++a, ++b)
						{
							b->name = a->name;
							b->value = a->value;
						}
					}
				}

				for (uint i=0; i < MAX_PERIPHERALS; ++i)
				{
					if (peripherals[i] != PERIPHERAL_UNSPECIFIED)
					{
						switch (peripherals[i])
						{
							case PERIPHERAL_STANDARD:

								profile.game.controllers[0] = Api::Input::PAD1;
								profile.game.controllers[1] = Api::Input::PAD2;
								break;

							case PERIPHERAL_FOURPLAYER:

								if (system == Profile::System::FAMICOM)
									profile.game.adapter = Api::Input::ADAPTER_FAMICOM;
								else
									profile.game.adapter = Api::Input::ADAPTER_NES;

								profile.game.controllers[2] = Api::Input::PAD3;
								profile.game.controllers[3] = Api::Input::PAD4;
								break;

							case PERIPHERAL_ZAPPER:

								if (system == Profile::System::VS_UNISYSTEM || system == Profile::System::VS_DUALSYSTEM)
								{
									profile.game.controllers[0] = Api::Input::ZAPPER;
									profile.game.controllers[1] = Api::Input::UNCONNECTED;
								}
								else
								{
									profile.game.controllers[1] = Api::Input::ZAPPER;
								}
								break;

							case PERIPHERAL_POWERPAD:
							case PERIPHERAL_FAMILYTRAINER:

								if (system == Profile::System::FAMICOM || peripherals[i] == PERIPHERAL_FAMILYTRAINER)
								{
									profile.game.controllers[1] = Api::Input::UNCONNECTED;
									profile.game.controllers[4] = Api::Input::FAMILYTRAINER;
								}
								else
								{
									profile.game.controllers[1] = Api::Input::POWERPAD;
								}
								break;

							case PERIPHERAL_ARKANOID:

								if (system == Profile::System::FAMICOM)
									profile.game.controllers[4] = Api::Input::PADDLE;
								else
									profile.game.controllers[1] = Api::Input::PADDLE;
								break;

							case PERIPHERAL_SUBORKEYBOARD:

								profile.game.controllers[4] = Api::Input::SUBORKEYBOARD;
								break;

							case PERIPHERAL_SUBORMOUSE:

								profile.game.controllers[1] = Api::Input::MOUSE;
								break;

							case PERIPHERAL_FAMILYKEYBOARD:

								profile.game.controllers[4] = Api::Input::FAMILYKEYBOARD;
								break;

							case PERIPHERAL_PARTYTAP:

								profile.game.controllers[1] = Api::Input::UNCONNECTED;
								profile.game.controllers[4] = Api::Input::PARTYTAP;
								break;

							case PERIPHERAL_CRAZYCLIMBER:

								profile.game.controllers[4] = Api::Input::CRAZYCLIMBER;
								break;

							case PERIPHERAL_EXCITINGBOXING:

								profile.game.controllers[4] = Api::Input::EXCITINGBOXING;
								break;

							case PERIPHERAL_BANDAIHYPERSHOT:

								profile.game.controllers[4] = Api::Input::BANDAIHYPERSHOT;
								break;

							case PERIPHERAL_KONAMIHYPERSHOT:

								profile.game.controllers[0] = Api::Input::UNCONNECTED;
								profile.game.controllers[1] = Api::Input::UNCONNECTED;
								profile.game.controllers[4] = Api::Input::KONAMIHYPERSHOT;
								break;

							case PERIPHERAL_POKKUNMOGURAA:

								profile.game.controllers[1] = Api::Input::UNCONNECTED;
								profile.game.controllers[4] = Api::Input::POKKUNMOGURAA;
								break;

							case PERIPHERAL_OEKAKIDSTABLET:

								profile.game.controllers[0] = Api::Input::UNCONNECTED;
								profile.game.controllers[1] = Api::Input::UNCONNECTED;
								profile.game.controllers[4] = Api::Input::OEKAKIDSTABLET;
								break;

							case PERIPHERAL_MAHJONG:

								profile.game.controllers[0] = Api::Input::UNCONNECTED;
								profile.game.controllers[1] = Api::Input::UNCONNECTED;
								profile.game.controllers[4] = Api::Input::MAHJONG;
								break;

							case PERIPHERAL_TOPRIDERBIKE:

								profile.game.controllers[0] = Api::Input::UNCONNECTED;
								profile.game.controllers[1] = Api::Input::UNCONNECTED;
								profile.game.controllers[4] = Api::Input::TOPRIDER;
								break;

							case PERIPHERAL_HORITRACK:

								profile.game.controllers[4] = Api::Input::HORITRACK;
								break;

							case PERIPHERAL_PACHINKO:

								profile.game.controllers[4] = Api::Input::PACHINKO;
								break;

							case PERIPHERAL_ROB:

								profile.game.controllers[1] = Api::Input::ROB;
								break;

							case PERIPHERAL_DOREMIKKO:

								profile.game.controllers[4] = Api::Input::DOREMIKKOKEYBOARD;
								break;

							case PERIPHERAL_POWERGLOVE:

								profile.game.controllers[0] = Api::Input::POWERGLOVE;
								break;

							case PERIPHERAL_TURBOFILE:

								profile.game.controllers[4] = Api::Input::TURBOFILE;
								break;

							case PERIPHERAL_BARCODEWORLD:

								profile.game.controllers[4] = Api::Input::BARCODEWORLD;
								break;
						}
					}
				}

				profile.multiRegion = multiRegion;

				profile.system.type = static_cast<Profile::System::Type>(system);
				profile.system.cpu = static_cast<Profile::System::Cpu>(cpu);
				profile.system.ppu = static_cast<Profile::System::Ppu>(ppu);

				if (*board)
					profile.board.type = board;

				if (mapper != Profile::Board::NO_MAPPER)
					profile.board.mapper = mapper;

				profile.board.solderPads = solderPads;

				for (uint j=0; j < 2; ++j)
				{
					if (full || (j ? profile.board.GetChr() == GetChrSize() : profile.board.GetPrg() == GetPrgSize()))
					{
						const Roms& src = (j ? chr : prg);
						Profile::Board::Roms& dst = (j ? profile.board.chr : profile.board.prg);

						dst.resize( src.size() );

						Profile::Board::Roms::iterator b(dst.begin());
						for (Roms::const_iterator a(src.begin()), end(src.end()); a != end; ++a, ++b)
						{
							b->size = a->size;

							if (full)
							{
								b->name = a->name;
								b->package = a->package;
								b->hash = a->hash;
							}

							b->pins.resize( a->pins.size() );

							Profile::Board::Pins::iterator d(b->pins.begin());
							for (Rom::Pins::const_iterator c(a->pins.begin()), end(a->pins.end()); c != end; ++c, ++d)
							{
								d->number = c->number;
								d->function = c->function;
							}
						}
					}
				}

				for (uint j=0; j < 2; ++j)
				{
					if (full || (j ? profile.board.GetVram() == GetVramSize() : profile.board.GetWram() == GetWramSize()))
					{
						const Rams& src = (j ? vram : wram);
						Profile::Board::Rams& dst = (j ? profile.board.vram : profile.board.wram);

						dst.resize( src.size() );

						Profile::Board::Rams::iterator b(dst.begin());
						for (Rams::const_iterator a(src.begin()), end(src.end()); a != end; ++a, ++b)
						{
							b->id = a->id;
							b->size = a->size;
							b->battery = a->battery;

							if (full)
								b->package = a->package;

							b->pins.resize( a->pins.size() );

							Profile::Board::Pins::iterator d(b->pins.begin());
							for (Ram::Pins::const_iterator c(a->pins.begin()), end(a->pins.end()); c != end; ++c, ++d)
							{
								d->number = c->number;
								d->function = c->function;
							}
						}
					}
				}

				profile.board.chips.resize( chips.size() );

				Profile::Board::Chips::iterator b(profile.board.chips.begin());
				for (Chips::const_iterator a(chips.begin()), end(chips.end()); a != end; ++a, ++b)
				{
					b->type = a->type;
					b->package = a->package;
					b->battery = a->battery;

					b->pins.resize( a->pins.size() );

					Profile::Board::Pins::iterator d(b->pins.begin());
					for (Chip::Pins::const_iterator c(a->pins.begin()), end(a->pins.end()); c != end; ++c, ++d)
					{
						d->number = c->number;
						d->function = c->function;
					}
				}
			}

		private:

			bool operator == (const Item& item) const
			{
				return
				(
					system == item.system &&
					mapper == item.mapper &&
					board == item.board &&
					solderPads == item.solderPads &&
					chips.size() == item.chips.size() &&
					cpu == item.cpu &&
					ppu == item.ppu &&
					GetVramSize() == item.GetVramSize() &&
					GetWramSize() == item.GetWramSize() &&
					HasVRamBattery() == item.HasVRamBattery() &&
					HasWRamBattery() == item.HasWRamBattery() &&
					HasChipBattery() == item.HasChipBattery() &&
					std::equal( chips.begin(), chips.end(), item.chips.begin() )
				);
			}

			bool Add(Item* const item)
			{
				item->multiRegion = this->multiRegion ||
				(
					(
						this->system == Profile::System::NES_PAL   ||
						this->system == Profile::System::NES_PAL_A ||
						this->system == Profile::System::NES_PAL_B
					)
						!=
					(
						item->system == Profile::System::NES_PAL   ||
						item->system == Profile::System::NES_PAL_A ||
						item->system == Profile::System::NES_PAL_B
					)
				);

				Item* it = this;

				for (;;)
				{
					if (*it == *item)
						return false;

					it->multiRegion = item->multiRegion;

					if (!it->sibling)
						break;

					it = it->sibling;
				}

				it->sibling = item;

				return true;
			}

			void Finalize(wcstring const lut) const
			{
				dump.by   << lut;
				dump.date << lut;

				title         << lut;
				altTitle      << lut;
				clss          << lut;
				subClss       << lut;
				catalog       << lut;
				developer     << lut;
				portDeveloper << lut;
				publisher     << lut;
				region        << lut;
				revision      << lut;
				pcb           << lut;
				board         << lut;
				cic           << lut;

				for (uint i=0; i < 2; ++i)
				{
					for (Roms::const_iterator j((i ? chr : prg).begin()), end((i ? chr : prg).end()); j != end; ++j)
					{
						j->name << lut;
						j->package << lut;

						for (Ic::Pins::const_iterator k(j->pins.begin()), end(j->pins.end()); k != end; ++k)
							k->function << lut;
					}
				}

				for (uint i=0; i < 2; ++i)
				{
					for (Rams::const_iterator j((i ? wram : vram).begin()), end((i ? wram : vram).end()); j != end; ++j)
						j->package << lut;
				}

				for (Chips::const_iterator i(chips.begin()), end(chips.end()); i != end; ++i)
				{
					i->type << lut;
					i->package << lut;

					for (Ic::Pins::const_iterator j(i->pins.begin()), end(i->pins.end()); j != end; ++j)
						j->function << lut;
				}

				for (Properties::const_iterator i(properties.begin()), end(properties.end()); i != end; ++i)
				{
					i->name << lut;
					i->value << lut;
				}

				if (sibling)
					sibling->Finalize( lut );
			}

		public:

			struct Less
			{
				bool operator () (const Item* a,const Item* b) const
				{
					return a->hash < b->hash;
				}

				bool operator () (const Item* a,const Hash& b) const
				{
					return a->hash < b;
				}

				bool operator () (const Hash& a,const Item* b) const
				{
					return a < b->hash;
				}
			};

			class Builder
			{
			public:

				~Builder();

				dword operator << (wcstring);
				void operator << (Item*);

			private:

				struct Less
				{
					bool operator () (wcstring a,wcstring b) const
					{
						return std::wcscmp( a, b ) < 0;
					}

					bool operator () (const Item* a,const Item* b) const
					{
						return a->hash < b->hash;
					}
				};

				typedef std::map<wcstring,dword,Less> StringMap;
				typedef std::set<Item*,Less> ItemMap;

				dword stringLength;
				StringMap stringMap;
				ItemMap itemMap;

			public:

				Builder()
				: stringLength(0)
				{
					(*this) << L"";
				}

				void Construct(Strings& strings,const Item**& itemsBegin,const Item**& itemsEnd)
				{
					NST_ASSERT( !strings.Size() && itemsBegin == NULL && itemsEnd == NULL );

					if (const dword size = itemMap.size())
					{
						strings.Resize( stringLength );
						wchar_t* const NST_RESTRICT sdst = strings.Begin();

						for (StringMap::const_iterator src(stringMap.begin()), end(stringMap.end()); src != end; ++src)
							std::wcscpy( sdst + src->second, src->first );

						const Item** dst = new const Item* [size];

						itemsBegin = dst;
						itemsEnd = dst + size;

						for (ItemMap::const_iterator src(itemMap.begin()), end(itemMap.end()); src != end; ++src, ++dst)
						{
							(*src)->Finalize( sdst );
							*dst = *src;
						}

						itemMap.clear();
					}
				}
			};
		};

		ImageDatabase::ImageDatabase()
		: enabled(true)
		{
			items.begin = NULL;
			items.end = NULL;
			items.hashing = HASHING_DETECT;
		}

		ImageDatabase::~ImageDatabase()
		{
			Unload();
		}

		ImageDatabase::Entry ImageDatabase::Search(const Hash& hash,const FavoredSystem favoredSystem) const
		{
			if (items.begin)
			{
				const Hash searchHash
				(
					( items.hashing & HASHING_SHA1 ) ? hash.GetSha1() : NULL,
					( items.hashing & HASHING_CRC  ) ? hash.GetCrc32() : 0UL
				);

				const Item** item = std::lower_bound( items.begin, items.end, searchHash, Item::Less() );

				if (item != items.end && (*item)->GetHash() == searchHash)
				{
					for (const Item* it = *item; it; it = it->GetNextSibling())
					{
						switch (it->GetSystem())
						{
							case Profile::System::NES_NTSC:

								if (favoredSystem == FAVORED_NES_NTSC)
									return it;

								break;

							case Profile::System::NES_PAL:
							case Profile::System::NES_PAL_A:
							case Profile::System::NES_PAL_B:

								if (favoredSystem == FAVORED_NES_PAL)
									return it;

								break;

							case Profile::System::FAMICOM:

								if (favoredSystem == FAVORED_FAMICOM)
									return it;

								break;
						}
					}

					return *item;
				}
			}

			return NULL;
		}

		wcstring ImageDatabase::Entry::GetTitle() const
		{
			return item ? item->GetTitle() : L"";
		}

		wcstring ImageDatabase::Entry::GetPublisher() const
		{
			return item ? item->GetPublisher() : L"";
		}

		wcstring ImageDatabase::Entry::GetDeveloper() const
		{
			return item ? item->GetDeveloper() : L"";
		}

		wcstring ImageDatabase::Entry::GetRegion() const
		{
			return item ? item->GetRegion() : L"";
		}

		wcstring ImageDatabase::Entry::GetRevision() const
		{
			return item ? item->GetRevision() : L"";
		}

		wcstring ImageDatabase::Entry::GetPcb() const
		{
			return item ? item->GetPcb() : L"";
		}

		wcstring ImageDatabase::Entry::GetBoard() const
		{
			return item ? item->GetBoard() : L"";
		}

		wcstring ImageDatabase::Entry::GetCic() const
		{
			return item ? item->GetCic() : L"";
		}

		uint ImageDatabase::Entry::NumPlayers() const
		{
			return item ? item->NumPlayers() : 0;
		}

		uint ImageDatabase::Entry::GetMapper() const
		{
			return item ? item->GetMapper() : Profile::Board::NO_MAPPER;
		}

		uint ImageDatabase::Entry::GetSolderPads() const
		{
			return item ? item->GetSolderPads() : 0;
		}

		ImageDatabase::Profile::System::Type ImageDatabase::Entry::GetSystem() const
		{
			return item ? item->GetSystem() : Profile::System::NES_NTSC;
		}

		bool ImageDatabase::Entry::IsMultiRegion() const
		{
			return item && item->IsMultiRegion();
		}

		ImageDatabase::Profile::Dump::State ImageDatabase::Entry::GetDumpState() const
		{
			return item ? item->GetDumpState() : Profile::Dump::UNKNOWN;
		}

		const ImageDatabase::Hash* ImageDatabase::Entry::GetHash() const
		{
			return item ? &item->GetHash() : NULL;
		}

		dword ImageDatabase::Entry::GetPrg() const
		{
			return item ? item->GetPrgSize() : 0;
		}

		dword ImageDatabase::Entry::GetChr() const
		{
			return item ? item->GetChrSize() : 0;
		}

		dword ImageDatabase::Entry::GetWram() const
		{
			return item ? item->GetWramSize() : 0;
		}

		dword ImageDatabase::Entry::GetVram() const
		{
			return item ? item->GetVramSize() : 0;
		}

		bool ImageDatabase::Entry::HasBattery() const
		{
			return item && item->HasBattery();
		}

		void ImageDatabase::Entry::Fill(Profile& profile,bool full) const
		{
			if (item)
				item->Fill( profile, full );
		}

		Result ImageDatabase::Load(std::istream& baseStream,std::istream* overrideStream)
		{
			Unload();

			try
			{
				Xml baseXml, overrideXml;
				Item::Builder builder;

				for (uint multi=0; multi < (overrideStream ? 2 : 1); ++multi)
				{
					Xml& xml = (multi ? overrideXml : baseXml);

					try
					{
						if (!xml.Read( multi ? *overrideStream : baseStream ))
							return RESULT_ERR_CORRUPT_FILE;
					}
					catch (...)
					{
						throw RESULT_ERR_CORRUPT_FILE;
					}

					if (!xml.GetRoot().IsType( L"database" ))
						return RESULT_ERR_INVALID_FILE;

					if (const Xml::Attribute attribute=xml.GetRoot().GetAttribute( L"version" ))
					{
						wcstring const version = attribute.GetValue();

						if
						(
							(version[0] < L'1' || version[0] > L'9') ||
							(version[1] != L'.') ||
							(version[2] < L'0' || version[2] > L'9') ||
							(version[3] != L'\0')
						)
							throw RESULT_ERR_INVALID_FILE;
					}

					const bool strict = !xml.GetRoot().GetAttribute( L"conformance" ).IsValue( L"loose" );

					for (Xml::Node game(xml.GetRoot().GetFirstChild()); game.IsType( L"game" ); game=game.GetNextSibling())
					{
						byte peripherals[4] =
						{
							Item::PERIPHERAL_UNSPECIFIED,
							Item::PERIPHERAL_UNSPECIFIED,
							Item::PERIPHERAL_UNSPECIFIED,
							Item::PERIPHERAL_UNSPECIFIED
						};

						if (Xml::Node device=game.GetChild( L"peripherals" ))
						{
							uint i = 0;

							for (device=device.GetFirstChild(); i < 4 && device.IsType( L"device" ); device=device.GetNextSibling())
							{
								if (const Xml::Attribute attribute = device.GetAttribute( L"type" ))
								{
                                         if (attribute.IsValue( L"3dglasses"        )) peripherals[i++] = Item::PERIPHERAL_3DGLASSES;
									else if (attribute.IsValue( L"arkanoid"         )) peripherals[i++] = Item::PERIPHERAL_ARKANOID;
									else if (attribute.IsValue( L"bandaihypershot"  )) peripherals[i++] = Item::PERIPHERAL_BANDAIHYPERSHOT;
									else if (attribute.IsValue( L"barcodeworld"     )) peripherals[i++] = Item::PERIPHERAL_BARCODEWORLD;
									else if (attribute.IsValue( L"crazyclimber"     )) peripherals[i++] = Item::PERIPHERAL_CRAZYCLIMBER;
									else if (attribute.IsValue( L"doremikko"        )) peripherals[i++] = Item::PERIPHERAL_DOREMIKKO;
									else if (attribute.IsValue( L"excitingboxing"   )) peripherals[i++] = Item::PERIPHERAL_EXCITINGBOXING;
									else if (attribute.IsValue( L"familykeyboard"   )) peripherals[i++] = Item::PERIPHERAL_FAMILYKEYBOARD;
									else if (attribute.IsValue( L"familyfunfitness" )) peripherals[i++] = Item::PERIPHERAL_POWERPAD;
									else if (attribute.IsValue( L"familytrainer"    )) peripherals[i++] = Item::PERIPHERAL_FAMILYTRAINER;
									else if (attribute.IsValue( L"fourplayer"       )) peripherals[i++] = Item::PERIPHERAL_FOURPLAYER;
									else if (attribute.IsValue( L"horitrack"        )) peripherals[i++] = Item::PERIPHERAL_HORITRACK;
									else if (attribute.IsValue( L"konamihypershot"  )) peripherals[i++] = Item::PERIPHERAL_KONAMIHYPERSHOT;
									else if (attribute.IsValue( L"mahjong"          )) peripherals[i++] = Item::PERIPHERAL_MAHJONG;
									else if (attribute.IsValue( L"miraclepiano"     )) peripherals[i++] = Item::PERIPHERAL_MIRACLEPIANO;
									else if (attribute.IsValue( L"oekakidstablet"   )) peripherals[i++] = Item::PERIPHERAL_OEKAKIDSTABLET;
									else if (attribute.IsValue( L"pachinko"         )) peripherals[i++] = Item::PERIPHERAL_PACHINKO;
									else if (attribute.IsValue( L"partytap"         )) peripherals[i++] = Item::PERIPHERAL_PARTYTAP;
									else if (attribute.IsValue( L"pokkunmoguraa"    )) peripherals[i++] = Item::PERIPHERAL_POKKUNMOGURAA;
									else if (attribute.IsValue( L"powerglove"       )) peripherals[i++] = Item::PERIPHERAL_POWERGLOVE;
									else if (attribute.IsValue( L"powerpad"         )) peripherals[i++] = Item::PERIPHERAL_POWERPAD;
									else if (attribute.IsValue( L"rob"              )) peripherals[i++] = Item::PERIPHERAL_ROB;
									else if (attribute.IsValue( L"suborkeyboard"    )) peripherals[i++] = Item::PERIPHERAL_SUBORKEYBOARD;
									else if (attribute.IsValue( L"subormouse"       )) peripherals[i++] = Item::PERIPHERAL_SUBORMOUSE;
									else if (attribute.IsValue( L"topriderbike"     )) peripherals[i++] = Item::PERIPHERAL_TOPRIDERBIKE;
									else if (attribute.IsValue( L"turbofile"        )) peripherals[i++] = Item::PERIPHERAL_TURBOFILE;
									else if (attribute.IsValue( L"zapper"           )) peripherals[i++] = Item::PERIPHERAL_ZAPPER;
								}
							}
						}

						for (Xml::Node image(game.GetFirstChild()); image; image=image.GetNextSibling())
						{
							Profile::System::Type system = Profile::System::NES_NTSC;
							Profile::System::Cpu cpu = Profile::System::CPU_RP2A03;
							Profile::System::Ppu ppu = Profile::System::PPU_RP2C02;

							if (image.IsType( L"cartridge" ))
							{
								if (const Xml::Attribute attribute=image.GetAttribute( L"system" ))
								{
									if (attribute.IsValue( L"famicom" ))
									{
										system = Profile::System::FAMICOM;
									}
									else if (attribute.IsValue( L"nes-ntsc" ))
									{
										system = Profile::System::NES_NTSC;
									}
									else if (attribute.IsValue( L"nes-pal" ))
									{
										system = Profile::System::NES_PAL;
										cpu = Profile::System::CPU_RP2A07;
										ppu = Profile::System::PPU_RP2C07;
									}
									else if (attribute.IsValue( L"nes-pal-a" ))
									{
										system = Profile::System::NES_PAL_A;
										cpu = Profile::System::CPU_RP2A07;
										ppu = Profile::System::PPU_RP2C07;
									}
									else if (attribute.IsValue( L"nes-pal-b" ))
									{
										system = Profile::System::NES_PAL_B;
										cpu = Profile::System::CPU_RP2A07;
										ppu = Profile::System::PPU_RP2C07;
									}
									else if (strict)
									{
										continue;
									}
								}
								else if (strict)
								{
									continue;
								}
							}
							else if (image.IsType( L"arcade" ))
							{
								ppu = Profile::System::PPU_RP2C03B;

								if (const Xml::Attribute attribute=image.GetAttribute( L"system" ))
								{
									if (attribute.IsValue( L"vs-unisystem" ))
									{
										system = Profile::System::VS_UNISYSTEM;
									}
									else if (attribute.IsValue( L"vs-dualsystem" ))
									{
										system = Profile::System::VS_DUALSYSTEM;
									}
									else if (attribute.IsValue( L"playchoice-10" ))
									{
										system = Profile::System::PLAYCHOICE_10;
									}
									else
									{
										continue;
									}
								}
								else
								{
									continue;
								}
							}
							else
							{
								continue;
							}

							if (system == Profile::System::VS_UNISYSTEM || system == Profile::System::VS_DUALSYSTEM)
							{
								if (const Xml::Attribute attribute=image.GetAttribute( L"ppu" ))
								{
                                         if (attribute.IsValue( L"rp2c03b"     )) ppu = Profile::System::PPU_RP2C03B;
									else if (attribute.IsValue( L"rp2c03g"     )) ppu = Profile::System::PPU_RP2C03G;
									else if (attribute.IsValue( L"rp2c04-0001" )) ppu = Profile::System::PPU_RP2C04_0001;
									else if (attribute.IsValue( L"rp2c04-0002" )) ppu = Profile::System::PPU_RP2C04_0002;
									else if (attribute.IsValue( L"rp2c04-0003" )) ppu = Profile::System::PPU_RP2C04_0003;
									else if (attribute.IsValue( L"rp2c04-0004" )) ppu = Profile::System::PPU_RP2C04_0004;
									else if (attribute.IsValue( L"rc2c03b"     )) ppu = Profile::System::PPU_RC2C03B;
									else if (attribute.IsValue( L"rc2c03c"     )) ppu = Profile::System::PPU_RC2C03C;
									else if (attribute.IsValue( L"rc2c05-01"   )) ppu = Profile::System::PPU_RC2C05_01;
									else if (attribute.IsValue( L"rc2c05-02"   )) ppu = Profile::System::PPU_RC2C05_02;
									else if (attribute.IsValue( L"rc2c05-03"   )) ppu = Profile::System::PPU_RC2C05_03;
									else if (attribute.IsValue( L"rc2c05-04"   )) ppu = Profile::System::PPU_RC2C05_04;
									else if (attribute.IsValue( L"rc2c05-05"   )) ppu = Profile::System::PPU_RC2C05_05;
								}
							}

							Profile::Dump::State dump = Profile::Dump::OK;

							if (const Xml::Attribute attribute=image.GetAttribute( L"dump" ))
							{
								if (attribute.IsValue( L"bad" ))
								{
									if (strict)
										continue;

									dump = Profile::Dump::BAD;
								}
								else if (attribute.IsValue( L"unknown" ))
								{
									if (strict)
										continue;

									dump = Profile::Dump::UNKNOWN;
								}
							}

							if (items.hashing == HASHING_DETECT)
							{
								if (*image.GetAttribute( L"sha1" ).GetValue())
									items.hashing |= HASHING_SHA1;

								if (*image.GetAttribute( L"crc" ).GetValue())
									items.hashing |= HASHING_CRC;
							}

							const Hash hash
							(
								( items.hashing & HASHING_SHA1 ) ? image.GetAttribute( L"sha1" ).GetValue() : L"",
								( items.hashing & HASHING_CRC  ) ? image.GetAttribute( L"crc"  ).GetValue() : L""
							);

							if (!hash)
								continue;

							if (const Xml::Node board=image.GetChild( L"board" ))
							{
								uint players = 0;

								if (const Xml::Attribute attribute=game.GetAttribute( L"players" ))
								{
									ulong value = attribute.GetUnsignedValue();

									if (value >= MIN_PLAYERS && value <= MAX_PLAYERS)
										players = value;
								}

								uint mapper = Profile::Board::NO_MAPPER;

								if (const Xml::Attribute attribute=board.GetAttribute( L"mapper" ))
								{
									ulong value = attribute.GetUnsignedValue();

									if (value <= MAX_MAPPER)
										mapper = value;
								}

								uint solderPads = 0;

								if (const Xml::Node pad=board.GetChild( L"pad" ))
								{
									solderPads =
									(
										(pad.GetAttribute( L"h" ).IsValue( L"1" ) ? Profile::Board::SOLDERPAD_H : 0U) |
										(pad.GetAttribute( L"v" ).IsValue( L"1" ) ? Profile::Board::SOLDERPAD_V : 0U)
									);
								}

								Item::Properties properties;

								if (Xml::Node node=image.GetChild( L"properties" ))
								{
									for (node=node.GetFirstChild(); node.IsType( L"property" ); node=node.GetNextSibling())
									{
										properties.push_back
										(
											Item::Property
											(
												builder << node.GetAttribute(L"name").GetValue(),
												builder << node.GetAttribute(L"value").GetValue()
											)
										);
									}
								}

								Item::Roms prg, chr;
								Item::Rams wram, vram;
								Item::Chips chips;

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

									Item::Ic::Pins pins;

									for (Xml::Node child(node.GetFirstChild()); child; child=child.GetNextSibling())
									{
										if (child.IsType(L"pin"))
										{
											const ulong number = child.GetAttribute(L"number").GetUnsignedValue();
											wcstring const function = child.GetAttribute(L"function").GetValue();

											if (number >= MIN_IC_PINS && number <= MAX_IC_PINS && *function)
												pins.push_back( Item::Ic::Pin(number,builder << function) );
										}
									}

									bool first;

									if (true == (first=node.IsType( L"prg" )) || node.IsType( L"chr" ))
									{
										if (size >= MIN_CHIP_SIZE && size <= MAX_CHIP_SIZE)
										{
											(first ? prg : chr).push_back
											(
												Item::Rom
												(
													node.GetAttribute( L"id" ).GetUnsignedValue(),
													builder << node.GetAttribute( L"name" ).GetValue(),
													size,
													builder << node.GetAttribute( L"package" ).GetValue(),
													pins,
													Hash(node.GetAttribute( L"sha1" ).GetValue(),node.GetAttribute( L"crc" ).GetValue())
												)
											);
										}
									}
									else if (true == (first=node.IsType( L"wram" )) || node.IsType( L"vram" ))
									{
										if (size >= MIN_CHIP_SIZE && size <= MAX_CHIP_SIZE)
										{
											(first ? wram : vram).push_back
											(
												Item::Ram
												(
													node.GetAttribute( L"id" ).GetUnsignedValue(),
													size,
													node.GetAttribute( L"battery" ).IsValue( L"1" ),
													builder << node.GetAttribute( L"package" ).GetValue(),
													pins
												)
											);
										}
									}
									else if (node.IsType( L"chip" ))
									{
										chips.push_back
										(
											Item::Chip
											(
												builder << node.GetAttribute( L"type" ).GetValue(),
												node.GetAttribute( L"battery" ).IsValue( L"1" ),
												builder << node.GetAttribute( L"package" ).GetValue(),
												pins
											)
										);
									}
								}

								builder << new Item
								(
									hash,
									builder << image.GetAttribute( L"dumper" ).GetValue(),
									builder << image.GetAttribute( L"datedumped" ).GetValue(),
									dump,
									builder << game.GetAttribute( L"name" ).GetValue(),
									builder << game.GetAttribute( L"altname" ).GetValue(),
									builder << game.GetAttribute( L"class" ).GetValue(),
									builder << game.GetAttribute( L"subclass" ).GetValue(),
									builder << game.GetAttribute( L"catalog" ).GetValue(),
									builder << game.GetAttribute( L"publisher" ).GetValue(),
									builder << game.GetAttribute( L"developer" ).GetValue(),
									builder << game.GetAttribute( L"portdeveloper" ).GetValue(),
									builder << game.GetAttribute( L"region" ).GetValue(),
									properties,
									players,
									peripherals,
									system,
									cpu,
									ppu,
									builder << image.GetAttribute( L"revision" ).GetValue(),
									builder << board.GetAttribute( L"type" ).GetValue(),
									builder << board.GetAttribute( L"pcb" ).GetValue(),
									mapper,
									prg,
									chr,
									wram,
									vram,
									chips,
									builder << board.GetChild( L"cic" ).GetAttribute( L"type" ).GetValue(),
									solderPads
								);
							}
						}
					}
				}

				builder.Construct( strings, items.begin, items.end );
			}
			catch (Result result)
			{
				Unload( true );
				return result;
			}
			catch (const std::bad_alloc&)
			{
				Unload( true );
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				Unload( true );
				return RESULT_ERR_GENERIC;
			}

			Log() << "Database: "
                  << (items.end - items.begin)
                  << " items imported from "
                  << (overrideStream ? "internal & external" : "internal")
                  <<  " DB" NST_LINEBREAK;

			return RESULT_OK;
		}

		void ImageDatabase::Unload(const bool error)
		{
			if (const Item** it=items.begin)
			{
				do
				{
					delete *it;
				}
				while (++it != items.end);

				delete [] items.begin;

				items.begin = NULL;
				items.end = NULL;
			}

			items.hashing = HASHING_DETECT;

			strings.Destroy();

			if (error)
				Log::Flush( "Database: error, aborting.." NST_LINEBREAK );
		}

		ImageDatabase::Item::Builder::~Builder()
		{
			for (ItemMap::const_iterator it(itemMap.begin()), end(itemMap.end()); it != end; ++it)
				delete *it;
		}

		dword ImageDatabase::Item::Builder::operator << (wcstring string)
		{
			const std::pair<StringMap::iterator,bool> entry
			(
				stringMap.insert( std::pair<wcstring,dword>(string,stringLength) )
			);

			if (entry.second)
				stringLength += std::wcslen(string) + 1;

			return entry.first->second;
		}

		void ImageDatabase::Item::Builder::operator << (Item* item)
		{
			std::pair<ItemMap::iterator,bool> entry;

			try
			{
				entry = itemMap.insert( item );
			}
			catch (...)
			{
				delete item;
				throw;
			}

			if (!entry.second && !(*entry.first)->Add(item))
				delete item;
		}

		ImageDatabase::Item::~Item()
		{
			if (const Item* item=sibling)
			{
				sibling = NULL;
				delete item;
			}
		}
	}
}
