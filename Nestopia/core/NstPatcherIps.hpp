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

#ifndef NST_IPS_H
#define NST_IPS_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include <vector>

namespace Nes
{
	namespace Core
	{
		class Ips
		{
		public:

			~Ips();

			enum
			{
				MAX_LENGTH = 0xFFFFFF
			};

			static bool IsIps(std::istream&);

			Result Load(std::istream&);
			Result Test(std::istream&) const;
			Result Test(const byte*,dword) const;
			Result Save(std::ostream&) const;
			Result Create(const byte*,const byte*,dword);
			bool Patch(const byte*,byte*,dword,dword=0) const;
			void Destroy();

		private:

			enum
			{
				MIN_EQUAL   = 5,
				MIN_BEG_RUN = 9,
				MIN_MID_RUN = 13,
				MIN_END_RUN = 9,
				NO_FILL     = 0xFFFF,
				MAX_BLOCK   = 0xFFFF
			};

			struct Block
			{
				byte* data;
				dword offset;
				word length;
				word fill;
			};

			typedef std::vector<Block> Blocks;

			Blocks blocks;

		public:

			bool Empty() const
			{
				return blocks.empty();
			}
		};
	}
}

#endif
