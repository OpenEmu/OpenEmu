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

#ifndef NST_UPS_H
#define NST_UPS_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Ups
		{
		public:

			Ups();
			~Ups();

			static bool IsUps(std::istream&);

			Result Load(std::istream&,bool);
			Result Save(std::ostream&) const;
			Result Test(std::istream&,bool) const;
			Result Test(const byte* NST_RESTRICT,dword,bool) const;
			Result Create(const byte*,const byte*,dword);
			bool Patch(const byte*,byte*,dword,dword=0) const;
			void Destroy();

		private:

			class Reader;
			class Writer;

			enum
			{
				MAX_SIZE = SIZE_16384K,
				MAX_OFFSET = SIZE_16384K
			};

			dword srcSize;
			dword srcCrc;
			dword dstSize;
			dword dstCrc;
			byte* patch;

		public:

			bool Empty() const
			{
				return dstSize == 0;
			}
		};
	}
}

#endif
