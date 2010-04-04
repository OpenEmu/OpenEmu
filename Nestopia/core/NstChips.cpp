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

#include <string>
#include <map>
#include "NstAssert.hpp"
#include "NstChips.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		struct Chips::Container
		{
			struct Less
			{
				bool operator () (const std::wstring& a,const std::wstring& b) const
				{
					return Core::StringCompare( a.c_str(), b.c_str() ) < 0;
				}
			};

			typedef std::multimap<std::wstring,Type,Less> Map;
			Map map;
		};

		Chips::Chips(const Chips& chips)
		: container(chips.container ? new Container(*chips.container) : NULL) {}

		Chips::~Chips()
		{
			Clear();
		}

		Chips& Chips::operator = (const Chips& chips)
		{
			if (this != &chips)
			{
				Clear();

				if (chips.container)
					container = new Container(*chips.container);
			}

			return *this;
		}

		Chips::Type& Chips::Add(wcstring type)
		{
			if (container == NULL)
				container = new Container;

			return container->map.insert( Container::Map::value_type(type,Type()) )->second;
		}

		void Chips::Clear()
		{
			if (Container* tmp = container)
			{
				container = NULL;
				delete tmp;
			}
		}

		Chips::Type* Chips::Find(wcstring type)
		{
			NST_ASSERT( type );

			if (container)
			{
				Container::Map::iterator it(container->map.find(type));

				if (it != container->map.end())
					return &it->second;
			}

			return NULL;
		}

		const Chips::Type* Chips::Find(wcstring type) const
		{
			NST_ASSERT( type );

			if (container)
			{
				Container::Map::const_iterator it(static_cast<const Container::Map&>(container->map).find(type));

				if (it != container->map.end())
					return &it->second;
			}

			return NULL;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
