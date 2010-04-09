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
#include "NstProperties.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		struct Properties::Container
		{
			typedef std::map<uint,std::wstring> Map;
			Map map;
		};

		Properties::Properties(const Properties& properties)
		: container(properties.container ? new Container(*properties.container) : NULL) {}

		Properties::~Properties()
		{
			Clear();
		}

		Properties& Properties::operator = (const Properties& properties)
		{
			if (this != &properties)
			{
				Clear();

				if (properties.container)
					container = new Container(*properties.container);
			}

			return *this;
		}

		bool Properties::operator ! () const
		{
			return container == NULL || container->map.empty();
		}

		void Properties::Clear()
		{
			if (Container* tmp = container)
			{
				container = NULL;
				delete tmp;
			}
		}

		void Properties::Proxy::operator = (wcstring function)
		{
			NST_ASSERT( function );

			if (container == NULL)
				container = new Container;

			container->map[id] = function;
		}

		wcstring Properties::Find(const Container* container,uint id)
		{
			if (container)
			{
				Container::Map::const_iterator it(container->map.find(id));

				if (it != container->map.end())
					return it->second.c_str();
			}

			return L"";
		}

		bool Properties::Proxy::operator == (wcstring string) const
		{
			return Core::StringCompare( Find(container,id), string ) == 0;
		}

		wcstring Properties::Proxy::operator * () const
		{
			return Find(container,id);
		}

		Properties::ConstProxy::ConstProxy(const Container* container,uint id)
		: function(Find(container,id)) {}

		Properties::ConstProxy::ConstProxy(Proxy proxy)
		: function(Find(proxy.container,proxy.id)) {}

		bool Properties::ConstProxy::operator == (wcstring string) const
		{
			return Core::StringCompare( function, string ) == 0;
		}

		wcstring Properties::ConstProxy::operator * () const
		{
			return function;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
