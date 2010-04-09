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

#ifndef NST_PATCHER_H
#define NST_PATCHER_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include <iosfwd>

namespace Nes
{
	namespace Core
	{
		class Ips;
		class Ups;

		class Patcher
		{
		public:

			explicit Patcher(bool=false);
			~Patcher();

			enum Type
			{
				IPS,
				UPS
			};

			struct Block
			{
				const byte* data;
				dword size;
			};

			Result Load(std::istream&);
			Result Load(std::istream&,std::istream&);
			Result Test(std::istream&) const;
			Result Test(const byte*,dword) const;
			Result Test(const Block*,uint) const;
			Result Save(std::ostream&) const;
			Result Create(Type,const byte*,const byte*,dword);
			bool   Patch(const byte*,byte*,dword,dword=0) const;
			void   Destroy();
			bool   Empty() const;

		private:

			Ips* ips;
			Ups* ups;
			const bool bypassChecksum;

		public:

			template<dword N>
			Result Test(const Block (&blocks)[N])
			{
				NST_COMPILE_ASSERT( N > 0 );
				return Test( blocks, N );
			}
		};
	}
}

#endif
