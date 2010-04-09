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

#ifndef NST_BOARD_FB_H
#define NST_BOARD_FB_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Fb : public Board
			{
			public:

				explicit Fb(const Context&);

			private:

				class CartSwitch : public DipSwitches
				{
				public:

					explicit CartSwitch(Wrk&);

					void Flush() const;
					void Reset(bool);

				private:

					Wrk& wrk;
					bool init;

					uint NumDips() const;
					uint NumValues(uint) const;
					cstring GetDipName(uint) const;
					cstring GetValueName(uint,uint) const;
					uint GetValue(uint) const;
					void SetValue(uint,uint);
				};

				void SubReset(bool);
				void Sync(Event,Input::Controllers*);
				Device QueryDevice(DeviceType);

				NES_DECL_PEEK( Wrk_6 );
				NES_DECL_POKE( Wrk_6 );
				NES_DECL_PEEK( Wrk_7 );
				NES_DECL_POKE( Wrk_7 );

				CartSwitch cartSwitch;
			};
		}
	}
}

#endif
