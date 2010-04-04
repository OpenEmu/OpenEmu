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

#ifndef NST_RAM_H
#define NST_RAM_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstPins.hpp"

namespace Nes
{
	namespace Core
	{
		class Ram
		{
		public:

			enum Type
			{
				RAM,
				NVRAM,
				ROM
			};

			Ram();
			Ram(const Ram&);
			Ram(Type,bool,bool,dword,byte* = NULL);
			~Ram();

			void Set(dword,byte* = NULL);
			void Set(Type,bool,bool,dword,byte* = NULL);
			void Destroy();
			void Fill(uint) const;
			void Mirror(dword);

			Ram& operator = (const Ram&);

		private:

			byte* mem;
			dword mask;
			dword size;
			byte type;
			bool readable;
			bool writable;
			bool internal;
			Pins pins;

		public:

			dword Size() const
			{
				return size;
			}

			dword Masking() const
			{
				return mask;
			}

			bool Empty() const
			{
				return size == 0;
			}

			bool Readable() const
			{
				return readable;
			}

			bool Writable() const
			{
				return writable;
			}

			bool Internal() const
			{
				return internal;
			}

			Type GetType() const
			{
				return static_cast<Type>(type);
			}

			void ReadEnable(bool r)
			{
				readable = r;
			}

			void WriteEnable(bool w)
			{
				writable = w;
			}

			void SetSecurity(bool r,bool w)
			{
				readable = r;
				writable = w;
			}

			void SetType(Type t)
			{
				type = t;
			}

			byte* Mem(dword offset=0) const
			{
				return mem + (offset & mask);
			}

			byte& operator [] (dword i) const
			{
				return mem[i];
			}

			Pins::PinsProxy Pin(uint i)
			{
				return pins[i];
			}

			Pins::ConstPinsProxy Pin(uint i) const
			{
				return pins[i];
			}

			bool PinsDefined() const
			{
				return pins;
			}
		};
	}
}

#endif
