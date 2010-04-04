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

#ifndef NST_IMAGEDATABASE_H
#define NST_IMAGEDATABASE_H

#include "NstVector.hpp"
#include "api/NstApiCartridge.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class ImageDatabase
		{
			class Item;

		public:

			typedef Api::Cartridge::Profile Profile;
			typedef Profile::Hash Hash;

			ImageDatabase();
			~ImageDatabase();

			class Entry : public ImplicitBool<Entry>
			{
			public:

				void Fill(Profile&,bool=true) const;

			private:

				const Item* item;

			public:

				Entry(const void* i=NULL)
				: item(static_cast<const Item*>(i)) {}

				const void* Reference() const
				{
					return item;
				}

				bool operator ! () const
				{
					return !item;
				}

				const Hash* GetHash() const;

				wcstring GetTitle()     const;
				wcstring GetPublisher() const;
				wcstring GetDeveloper() const;
				wcstring GetRegion()    const;
				wcstring GetRevision()  const;
				wcstring GetPcb()       const;
				wcstring GetBoard()     const;
				wcstring GetCic()       const;

				bool IsMultiRegion() const;
				uint NumPlayers() const;
				uint GetMapper() const;
				uint GetSolderPads() const;
				Profile::System::Type GetSystem() const;
				Profile::Dump::State GetDumpState() const;

				dword GetPrg()  const;
				dword GetChr()  const;
				dword GetWram() const;
				dword GetVram() const;

				bool HasBattery() const;
			};

			Entry Search(const Hash&,FavoredSystem) const;

		private:

			Result Load(std::istream&,std::istream*);
			void Unload(bool);

			typedef Vector<wchar_t> Strings;

			enum
			{
				MIN_PLAYERS    = 1,
				MAX_PLAYERS    = 255,
				MAX_MAPPER     = 255,
				MIN_CHIP_SIZE  = 1,
				MAX_CHIP_SIZE  = SIZE_16384K,
				MIN_IC_PINS    = 1,
				MAX_IC_PINS    = 127,
				HASHING_DETECT = 0x0,
				HASHING_SHA1   = 0x1,
				HASHING_CRC    = 0x2
			};

			ibool enabled;

			struct
			{
				const Item** begin;
				const Item** end;
				uint hashing;
			}   items;

			Strings strings;

		public:

			Result Load(std::istream& stream)
			{
				return Load( stream, NULL );
			}

			Result Load(std::istream& baseStream,std::istream& overrideStream)
			{
				return Load( baseStream, &overrideStream );
			}

			void Unload()
			{
				Unload( false );
			}

			void Enable(bool state=true)
			{
				enabled = state;
			}

			bool Enabled() const
			{
				return enabled;
			}
		};
	}
}

#endif
