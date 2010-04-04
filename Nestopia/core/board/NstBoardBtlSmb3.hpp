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

#ifndef NST_BOARD_BTL_SMB3_H
#define NST_BOARD_BTL_SMB3_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Btl
			{
				class Smb3 : public Board
				{
				public:

					explicit Smb3(const Context&);

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);
					void SubSave(State::Saver&) const;
					void Sync(Event,Input::Controllers*);
					void UpdateChr(uint,uint) const;

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 8001 );
					NES_DECL_POKE( 8004 );
					NES_DECL_POKE( 8008 );
					NES_DECL_POKE( 800C );
					NES_DECL_POKE( 800D );
					NES_DECL_POKE( 800E );
					NES_DECL_POKE( 800F );

					struct Irq
					{
						void Reset(bool);
						bool Clock();

						ibool enabled;
						uint count;
					};

					ClockUnits::M2<Irq> irq;
				};
			}
		}
	}
}

#endif
