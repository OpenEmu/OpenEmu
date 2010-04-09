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

#ifndef NST_INPUT_DEVICE_H
#define NST_INPUT_DEVICE_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "../NstState.hpp"
#include "../api/NstApiInput.hpp"

namespace Nes
{
	namespace Core
	{
		class Cpu;

		namespace Input
		{
			class Device
			{
			protected:

				typedef Api::Input::Type Type;

				const Type type;
				Controllers* input;
				const Cpu& cpu;

			public:

				explicit Device(const Cpu& c,Type t=Api::Input::UNCONNECTED)
				: type(t), input(NULL), cpu(c) {}

				virtual ~Device() {}

				virtual void Initialize(bool) {}
				virtual void Reset() {}
				virtual void LoadState(State::Loader&,dword) {}
				virtual void SaveState(State::Saver&,byte) const {}

				virtual void BeginFrame(Controllers* i)
				{
					input = i;
				}

				virtual void EndFrame()
				{
				}

				virtual void Poke(uint)
				{
				}

				virtual uint Peek(uint)
				{
					return 0;
				}

				Type GetType() const
				{
					return type;
				}
			};
		}
	}
}

#endif
