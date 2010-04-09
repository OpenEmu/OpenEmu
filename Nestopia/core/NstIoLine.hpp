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

#ifndef NST_IO_LINE_H
#define NST_IO_LINE_H

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

			class Line : public ImplicitBool<Line>
			{
				class Component {};
				typedef void (NST_FASTCALL Component::*Toggler)(Cycle);

				Component* component;
				Toggler toggler;

				NST_COMPILE_ASSERT( sizeof(Toggler) <= sizeof(void (*)(void*,Cycle)) );

			public:

				Line() {}

				template<typename T>
				Line(T* c,void (NST_FASTCALL T::*t)(Cycle))
				:
				component ( reinterpret_cast<Component*>(c) ),
				toggler   ( reinterpret_cast<Toggler>(t) )
				{
					NST_COMPILE_ASSERT( sizeof(toggler) == sizeof(t) );
				}

				template<typename T>
				void Set(T* c,void (NST_FASTCALL T::*t)(Cycle))
				{
					NST_COMPILE_ASSERT( sizeof(toggler) == sizeof(t) );

					component = reinterpret_cast<Component*>(c);
					toggler   = reinterpret_cast<Toggler>(t);
				}

				void Unset()
				{
					component = NULL;
					toggler = NULL;
				}

				bool operator ! () const
				{
					return component == NULL;
				}

				void Toggle(Cycle cycle) const
				{
					(*component.*toggler)( cycle );
				}
			};

			#define NES_DECL_LINE(a_) void NST_FASTCALL Line_##a_(Cycle)
			#define NES_LINE(o_,a_) void NST_FASTCALL o_::Line_##a_(Cycle cycle)
			#define NES_LINE_T(t_,o_,a_) t_ void NST_FASTCALL o_::Line_##a_(Cycle cycle)

		#else

			class Line : public ImplicitBool<Line>
			{
				typedef void* Component;
				typedef void (NST_REGCALL *Toggler)(Component,Cycle);

				Component component;
				Toggler toggler;

			public:

				Line() {}

				Line(Component c,Toggler t)
				:
				component ( c ),
				toggler   ( t )
				{}

				void Set(Component c,Toggler t)
				{
					component = c;
					toggler   = t;
				}

				void Unset()
				{
					component = NULL;
					toggler = NULL;
				}

				bool operator ! () const
				{
					return component == NULL;
				}

				void Toggle(Cycle cycle) const
				{
					toggler( component, cycle );
				}
			};

			#define NES_DECL_LINE(a_)                                              \
                                                                                   \
				NST_FORCE_INLINE void NST_FASTCALL Line_M_##a_(Cycle);             \
				static NST_NO_INLINE void NST_REGCALL Line_##a_(void*,Cycle)

			#define NES_LINE(o_,a_)                                                \
                                                                                   \
				NST_NO_INLINE void NST_REGCALL o_::Line_##a_(void* p_,Cycle c_)    \
				{                                                                  \
					static_cast<o_*>(p_)->Line_M_##a_(c_);                         \
				}                                                                  \
                                                                                   \
				NST_FORCE_INLINE void NST_FASTCALL o_::Line_M_##a_(Cycle cycle)

			#define NES_LINE_T(t_,o_,a_)                                           \
                                                                                   \
				t_ NST_NO_INLINE void NST_REGCALL o_::Line_##a_(void* p_,Cycle c_) \
				{                                                                  \
					static_cast<o_*>(p_)->Line_M_##a_(c_);                         \
				}                                                                  \
                                                                                   \
				t_ NST_FORCE_INLINE void NST_FASTCALL o_::Line_M_##a_(Cycle cycle)

		#endif
		}
	}
}

#endif
