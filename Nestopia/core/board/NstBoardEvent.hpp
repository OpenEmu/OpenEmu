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

#ifndef NST_BOARD_EVENT_H
#define NST_BOARD_EVENT_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "../NstDipSwitches.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Event : public Mmc1
			{
			public:

				explicit Event(const Context&);

			private:

				class CartSwitches : public DipSwitches
				{
				public:

					CartSwitches();

					inline bool ShowTime() const;
					inline dword GetTime() const;

				private:

					enum
					{
						DEFAULT_DIP = 4,
						BASE_TIME = 0x2000000
					};

					uint NumDips() const;
					uint NumValues(uint) const;
					cstring GetDipName(uint) const;
					cstring GetValueName(uint,uint) const;
					uint GetValue(uint) const;
					void SetValue(uint,uint);

					uint time;
					ibool showTime;
				};

				void SubReset(bool);
				void SubSave(State::Saver&) const;
				void SubLoad(State::Loader&,dword);
				Device QueryDevice(DeviceType);
				void NST_FASTCALL UpdateRegisters(uint);
				void Sync(Board::Event,Input::Controllers*);

				struct Irq
				{
					void Reset(bool);
					bool Clock();

					dword count;
				};

				enum
				{
					TIME_TEXT_MIN_OFFSET = 11,
					TIME_TEXT_SEC_OFFSET = 13
				};

				ClockUnits::M2<Irq> irq;
				dword time;
				CartSwitches cartSwitches;
				char text[16];
			};
		}
	}
}

#endif
