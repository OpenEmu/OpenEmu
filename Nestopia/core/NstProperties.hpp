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

#ifndef NST_PROPERTIES_H
#define NST_PROPERTIES_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Properties : public ImplicitBool<Properties>
		{
		public:

			Properties(const Properties&);
			~Properties();

			Properties& operator = (const Properties&);
			bool operator ! () const;

			void Clear();

		protected:

			struct Container;

			static wcstring Find(const Container*,uint);

			Container* container;

		public:

			class ConstProxy;

			class Proxy
			{
			protected:

				friend class Properties;
				friend class ConstProxy;

				Container*& container;
				const uint id;

				Proxy(Container*& c,uint i)
				: container(c), id(i) {}

			public:

				bool operator == (wcstring) const;
				void operator = (wcstring);

				wcstring operator * () const;

				bool operator != (wcstring s) const
				{
					return !(*this == s);
				}
			};

			class ConstProxy
			{
			protected:

				friend class Properties;

				wcstring const function;

				ConstProxy(const Container*,uint);

			public:

				ConstProxy(Proxy);

				bool operator == (wcstring) const;

				wcstring operator * () const;

				bool operator != (wcstring s) const
				{
					return !(*this == s);
				}
			};

			Properties()
			: container(NULL) {}

			Proxy operator [] (uint i)
			{
				return Proxy( container, i );
			}

			ConstProxy operator [] (uint i) const
			{
				return ConstProxy( container, i );
			}
		};
	}
}

#endif
