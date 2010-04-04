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

#ifndef NST_PINS_H
#define NST_PINS_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstProperties.hpp"

namespace Nes
{
	namespace Core
	{
		class Pins : public Properties
		{
		public:

			class PinsProxy : public Properties::Proxy
			{
			public:

				PinsProxy(Container*& c,uint i)
				: Proxy(c,i) {}

				void operator = (wcstring s)
				{
					*static_cast<Proxy*>(this) = s;
				}
			};

			class ConstPinsProxy : public Properties::ConstProxy
			{
			public:

				ConstPinsProxy(const Container* c,uint i)
				: ConstProxy(c,i) {}

				ConstPinsProxy(PinsProxy p)
				: ConstProxy(p) {}

				class ComponentProxy : public ImplicitBool<ComponentProxy>
				{
					static wcstring Init(wcstring);

					wcstring const component;
					wcstring const end;

				public:

					explicit ComponentProxy(wcstring);

					bool operator == (wcstring) const;

					class LineProxy : public ImplicitBool<LineProxy>
					{
						static uint Init(wchar_t,wcstring);

						const uint line;

					public:

						LineProxy(wchar_t,wcstring);

						bool operator == (uint l) const
						{
							return line == l;
						}

						bool operator != (uint l) const
						{
							return line != l;
						}

						uint operator * () const
						{
							return line;
						}

						bool operator ! () const
						{
							return line == ~0U;
						}
					};

					bool operator != (wcstring s) const
					{
						return !(*this == s);
					}

					bool operator ! () const
					{
						return !(end - component);
					}

					LineProxy A() const
					{
						return LineProxy( L'A', end );
					}

					LineProxy D() const
					{
						return LineProxy( L'D', end );
					}
				};

				ComponentProxy C(wcstring c) const
				{
					return ComponentProxy(function);
				}
			};

			Pins() {}

			Pins(const Pins& pins)
			: Properties(pins) {}

			Pins& operator = (const Pins& pins)
			{
				*static_cast<Properties*>(this) = pins;
				return *this;
			}

			PinsProxy operator [] (uint i)
			{
				return PinsProxy( container, i );
			}

			ConstPinsProxy operator [] (uint i) const
			{
				return ConstPinsProxy( container, i );
			}
		};
	}
}

#endif
