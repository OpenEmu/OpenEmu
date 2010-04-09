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

#ifndef NST_BOARD_UNL_TF1201_H
#define NST_BOARD_UNL_TF1201_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				class Tf1201 : public Board
				{
				public:

					explicit Tf1201(const Context&);

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);
					void SubSave(State::Saver&) const;
					void UpdatePrg(uint);
					void Sync(Event,Input::Controllers*);

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 9001 );
					NES_DECL_POKE( B000 );
					NES_DECL_POKE( B002 );
					NES_DECL_POKE( F000 );
					NES_DECL_POKE( F001 );
					NES_DECL_POKE( F002 );

					struct Irq
					{
						void Reset(bool);
						bool Clock();

						enum
						{
							CLOCK_FILTER = 16
						};

						ibool enabled;
						uint count;
					};

					uint prgSelect;
					ClockUnits::A12<Irq,Irq::CLOCK_FILTER> irq;
				};
			}
		}
	}
}

#endif
