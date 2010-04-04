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

#ifndef NST_API_H
#define NST_API_H

#ifndef NST_BASE_H
#include "../NstBase.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#ifndef NST_CALLBACK
#define NST_CALLBACK NST_CALL
#endif

#if NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		class Machine;

		template<typename T>
		class UserCallback : public ImplicitBool< UserCallback<T> >
		{
		public:

			typedef void* UserData;
			typedef T Function;

		protected:

			Function function;
			UserData userdata;

			UserCallback()
			: function(0), userdata(0) {}

		public:

			void Set(Function f,UserData d)
			{
				function = f;
				userdata = d;
			}

			void Unset()
			{
				function = 0;
				userdata = 0;
			}

			void Get(Function& f,UserData& d) const
			{
				f = function;
				d = userdata;
			}

			bool operator ! () const
			{
				return !function;
			}
		};
	}

	namespace Api
	{
		class Base
		{
		public:

			typedef void* UserData;

		protected:

			Core::Machine& emulator;

			Base(Core::Machine& e)
			: emulator(e) {}
		};
	}
}

#if NST_MSVC >= 1200
#pragma warning( pop )
#endif

#endif
