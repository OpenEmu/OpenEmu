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

#include <cstdlib>
#include <cerrno>
#include <cwchar>
#include "NstCore.hpp"
#include "NstPins.hpp"

namespace Nes
{
	namespace Core
	{
		wcstring Pins::ConstPinsProxy::ComponentProxy::Init(wcstring s)
		{
			while (*s && *s != L' ')
				s++;

			return s;
		}

		Pins::ConstPinsProxy::ComponentProxy::ComponentProxy(wcstring s)
		: component(s), end(Init(s))
		{
		}

		bool Pins::ConstPinsProxy::ComponentProxy::operator == (wcstring s) const
		{
			return ulong(end - component) == wcslen(s) && Core::StringCompare( component, s, end - component ) == 0;
		}

		uint Pins::ConstPinsProxy::ComponentProxy::LineProxy::Init(wchar_t c,wcstring s)
		{
			if (*s == L' ' && Core::StringCompare(s+1,&c,1) == 0)
			{
				const ulong line = std::wcstoul( s+2, NULL, 10 );

				if (errno != ERANGE && line < ~0U)
					return line;
			}

			return ~0U;
		}

		Pins::ConstPinsProxy::ComponentProxy::LineProxy::LineProxy(wchar_t c,wcstring s)
		: line(Init(c,s))
		{
		}
	}
}
