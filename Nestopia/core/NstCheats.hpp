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

#ifndef NST_CHEATS_H
#define NST_CHEATS_H

#ifndef NST_VECTOR_H
#include "NstVector.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Cheats
		{
		public:

			explicit Cheats(Cpu&);
			~Cheats();

			void Reset();
			void BeginFrame(bool);
			void ClearCodes();

			Result GetCode (dword,ushort*,uchar*,uchar*,bool*) const;
			Result SetCode (word,byte,byte,bool,bool);
			Result DeleteCode (dword);

		private:

			NES_DECL_PEEK( Wizard );
			NES_DECL_POKE( Wizard );

			struct LoCode
			{
				word address;
				byte data;
				byte compare;
				ibool useCompare;
			};

			struct HiCode
			{
				inline bool operator < (const HiCode&) const;
				inline bool operator < (Address) const;

				word address;
				byte data;
				byte compare;
				ibool useCompare;
				const Io::Port* port;
			};

			inline friend bool operator < (Address,const HiCode&);

			typedef Vector<LoCode> LoCodes;
			typedef Vector<HiCode> HiCodes;

			void Map(HiCode&);

			Cpu& cpu;
			ibool frameLocked;
			LoCodes loCodes;
			HiCodes hiCodes;

		public:

			dword NumCodes() const
			{
				return loCodes.Size() + hiCodes.Size();
			}
		};
	}
}

#endif
