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

#ifndef NST_IO_MAP_H
#define NST_IO_MAP_H

#include "NstIoPort.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Io
		{
			template<dword N> class Map
			{
			public:

				enum
				{
					SIZE = N,
					OVERFLOW_SIZE = 0x100,
					FULL_SIZE = SIZE + OVERFLOW_SIZE
				};

			protected:

				Port ports[FULL_SIZE];

			public:

				class Section
				{
					Port* NST_RESTRICT port;
					const Port* const end;

				public:

					Section(Port* b,const Port* e)
					: port(b), end(e) {}

					template<typename A,typename B,typename C>
					void Set(A a,B b,C c)
					{
						do
						{
							port->Set( a, b, c );
						}
						while (++port != end);
					}

					template<typename A,typename B>
					void Set(A a,B b)
					{
						do
						{
							port->Set( a, b );
						}
						while (++port != end);
					}

					template<typename A>
					void Set(A a)
					{
						do
						{
							port->Set( a );
						}
						while (++port != end);
					}
				};

				template<typename A,typename B,typename C>
				Map(A a,B b,C c)
				{
					for (dword i=SIZE; i < FULL_SIZE; ++i)
						ports[i].Set( a, b, c );
				}

				const Port& operator [] (Address address) const
				{
					NST_ASSERT( address < FULL_SIZE );
					return ports[address];
				}

				Port& operator () (Address address)
				{
					NST_ASSERT( address < FULL_SIZE );
					return ports[address];
				}

				Section operator () (Address first,Address last)
				{
					NST_ASSERT( first <= last && last < SIZE );
					return Section( ports + first, ports + last + 1 );
				}
			};
		}
	}
}

#endif
