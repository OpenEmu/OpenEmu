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

#ifndef NST_INPUT_ADAPTER_H
#define NST_INPUT_ADAPTER_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			class NST_NO_VTABLE Adapter
			{
			protected:

				typedef Api::Input::Adapter Type;
				Type type;

				explicit Adapter(Type);

			public:

				virtual ~Adapter() {}

				virtual void Reset() = 0;
				virtual void BeginFrame(Controllers*) = 0;
				virtual void EndFrame() = 0;
				virtual void Initialize(bool) = 0;
				virtual void Poke(uint) = 0;
				virtual uint Peek(uint) = 0;
				virtual uint NumPorts() const = 0;
				virtual Device& GetDevice(uint) const = 0;
				virtual Device& Connect(uint,Device&) = 0;
				virtual bool SetType(Type) = 0;

				Type GetType() const
				{
					return type;
				}
			};

			class AdapterTwo : public Adapter
			{
				~AdapterTwo() {}

				void Reset();
				void BeginFrame(Controllers*);
				void EndFrame();
				void Initialize(bool);
				void Poke(uint);
				uint Peek(uint);
				uint NumPorts() const;
				Device& GetDevice(uint) const;
				Device& Connect(uint,Device&);
				bool SetType(Type);

				Device* devices[2];

			public:

				AdapterTwo(Device&,Device&,Type=Api::Input::ADAPTER_NES);
			};

			class AdapterFour : public Adapter
			{
				~AdapterFour() {}

				void Reset();
				void BeginFrame(Controllers*);
				void EndFrame();
				void Initialize(bool);
				uint Peek(uint);
				void Poke(uint);
				uint NumPorts() const;
				Device& GetDevice(uint) const;
				Device& Connect(uint,Device&);
				bool SetType(Type);

				uint increaser;
				uint count[2];
				Device* devices[4];

			public:

				AdapterFour(Device&,Device&,Device&,Device&,Type=Api::Input::ADAPTER_NES);

				void SaveState(State::Saver&,dword) const;
				void LoadState(State::Loader&);
			};
		}
	}
}

#endif
