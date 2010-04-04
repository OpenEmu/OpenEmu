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

#ifndef NST_HOOK_H
#define NST_HOOK_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
	#ifdef NST_FASTDELEGATE

		class Hook
		{
			class Component {};
			typedef void (Component::*Executor)();

			Component* component;
			Executor executor;

			NST_COMPILE_ASSERT( sizeof(Executor) <= sizeof(void (*)(void*)) );

		public:

			Hook() {}

			template<typename T>
			Hook(T* c,void (T::*e)())
			:
			component ( reinterpret_cast<Component*>(c) ),
			executor  ( reinterpret_cast<Executor>(e) )
			{
				NST_COMPILE_ASSERT( sizeof(executor) == sizeof(e) );
			}

			void Execute() const
			{
				(*component.*executor)();
			}

			bool operator == (const Hook& h) const
			{
				return executor == h.executor && component == h.component;
			}
		};

		#define NES_DECL_HOOK(a_) void Hook_##a_()
		#define NES_HOOK(o_,a_) void o_::Hook_##a_()
		#define NES_HOOK_T(t_,o_,a_) t_ void o_::Hook_##a_()
		#define NES_DO_HOOK(a_) Hook_##a_()

	#else

		class Hook
		{
			typedef void* Component;
			typedef void (NST_REGCALL *Executor)(Component);

			Component component;
			Executor executor;

		public:

			Hook() {}

			Hook(Component c,Executor e)
			:
			component ( c ),
			executor  ( e )
			{}

			void Execute() const
			{
				executor( component );
			}

			bool operator == (const Hook& h) const
			{
				return executor == h.executor && component == h.component;
			}
		};

		#define NES_DECL_HOOK(a_)                                   \
																	\
			NST_FORCE_INLINE void NST_FASTCALL Hook_M_##a_();       \
			static void NST_REGCALL Hook_##a_(void*)

		#define NES_HOOK(o_,a_)                                     \
																	\
			void NST_REGCALL o_::Hook_##a_(void* p_)                \
			{                                                       \
				static_cast<o_*>(p_)->Hook_M_##a_();                \
			}                                                       \
																	\
			NST_FORCE_INLINE void NST_FASTCALL o_::Hook_M_##a_()

		#define NES_HOOK_T(t_,o_,a_)                                \
																	\
			t_ void NST_REGCALL o_::Hook_##a_(void* p_)             \
			{                                                       \
				static_cast<o_*>(p_)->Hook_M_##a_();                \
			}                                                       \
																	\
			t_ NST_FORCE_INLINE void NST_FASTCALL o_::Hook_M_##a_()

		#define NES_DO_HOOK(a_) Hook_##a_(this)

	#endif
	}
}

#endif
