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

#ifndef NST_IO_ACCESSOR_H
#define NST_IO_ACCESSOR_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Io
		{
		#ifdef NST_FASTDELEGATE

			class Accessor
			{
			public:

				template<typename T> struct Type
				{
					typedef Data (NST_FASTCALL T::*Function)(Address);
				};

			private:

				class Component {};
				typedef Type<Component>::Function Function;

				Component* component;
				Function function;

			public:

				Accessor() {}

				template<typename T>
				Accessor(T* c,typename Type<T>::Function f)
				:
				component ( reinterpret_cast<Component*>(c) ),
				function  ( reinterpret_cast<Function>(f) )
				{
					NST_COMPILE_ASSERT( sizeof(function) == sizeof(f) );
				}

				template<typename T>
				void Set(T* c,typename Type<T>::Function f)
				{
					NST_COMPILE_ASSERT( sizeof(function) == sizeof(f) );

					component = reinterpret_cast<Component*>(c);
					function  = reinterpret_cast<Function>(f);
				}

				Data Fetch(Address address) const
				{
					return (*component.*function)( address );
				}
			};

			#define NES_DECL_ACCESSOR(a_) Data NST_FASTCALL Access_##a_(Address)
			#define NES_ACCESSOR(o_,a_) Data NST_FASTCALL o_::Access_##a_(Address address)

		#else

			class Accessor
			{
				typedef void* Component;
				typedef Data (NST_REGCALL *Function)(Component,Address);

				Component component;
				Function function;

			public:

				Accessor() {}

				Accessor(Component c,Function t)
				:
				component ( c ),
				function  ( t )
				{}

				void Set(Component c,Function t)
				{
					component = c;
					function  = t;
				}

				Data Fetch(Address address) const
				{
					return function( component, address );
				}

				template<typename T> struct Type
				{
					typedef Data (NST_REGCALL *Function)(Component,Address);
				};
			};

			#define NES_DECL_ACCESSOR(a_)                                             \
                                                                                      \
				NST_SINGLE_CALL Data NST_FASTCALL Access_M_##a_(Address);             \
				static NST_NO_INLINE Data NST_REGCALL Access_##a_(void*,Address)

			#define NES_ACCESSOR(o_,a_)                                               \
                                                                                      \
				NST_NO_INLINE Data NST_REGCALL o_::Access_##a_(void* p_,Address i_)   \
				{                                                                     \
					return static_cast<o_*>(p_)->Access_M_##a_(i_);                   \
				}                                                                     \
                                                                                      \
				NST_SINGLE_CALL Data NST_FASTCALL o_::Access_M_##a_(Address address)

		#endif
		}
	}
}

#endif
