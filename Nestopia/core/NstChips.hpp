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

#ifndef NST_CHIPS_H
#define NST_CHIPS_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstPins.hpp"

namespace Nes
{
	namespace Core
	{
		class Chips
		{
		public:

			Chips(const Chips&);
			~Chips();

			Chips& operator = (const Chips&);

			class Type
			{
				friend class Chips;

				Pins pins;
				Properties samples;

			public:

				Pins::PinsProxy Pin(uint number)
				{
					return pins[number];
				}

				Pins::ConstPinsProxy Pin(uint number) const
				{
					return pins[number];
				}

				Properties::Proxy Sample(uint number)
				{
					return samples[number];
				}

				Properties::ConstProxy Sample(uint number) const
				{
					return samples[number];
				}

				bool PinsDefined() const
				{
					return pins;
				}

				bool HasSamples() const
				{
					return samples;
				}
			};

			Type& Add(wcstring);
			Type* Find(wcstring);
			const Type* Find(wcstring) const;
			void Clear();

		private:

			struct Container;
			Container* container;

		public:

			Chips()
			: container(NULL) {}

			bool Has(wcstring type) const
			{
				return Find(type);
			}

			Type& operator [] (wcstring type)
			{
				return *Find(type);
			}

			const Type& operator [] (wcstring type) const
			{
				return *Find(type);
			}
		};
	}
}

#endif
