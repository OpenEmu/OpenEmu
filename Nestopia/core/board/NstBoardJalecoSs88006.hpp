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

#ifndef NST_BOARD_JALECO_SS88006_H
#define NST_BOARD_JALECO_SS88006_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Player;
		}

		namespace Boards
		{
			namespace Jaleco
			{
				class Ss88006 : public Board
				{
				public:

					explicit Ss88006(const Context&);

				private:

					~Ss88006();

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Sync(Event,Input::Controllers*);

					template<uint MASK,uint SHIFT> void SwapPrg(uint,uint);
					template<uint MASK,uint SHIFT> void SwapChr(uint,uint) const;

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 8001 );
					NES_DECL_POKE( 8002 );
					NES_DECL_POKE( 8003 );
					NES_DECL_POKE( 9000 );
					NES_DECL_POKE( 9001 );
					NES_DECL_POKE( 9002 );
					NES_DECL_POKE( A000 );
					NES_DECL_POKE( A001 );
					NES_DECL_POKE( A002 );
					NES_DECL_POKE( A003 );
					NES_DECL_POKE( B000 );
					NES_DECL_POKE( B001 );
					NES_DECL_POKE( B002 );
					NES_DECL_POKE( B003 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( C001 );
					NES_DECL_POKE( C002 );
					NES_DECL_POKE( C003 );
					NES_DECL_POKE( D000 );
					NES_DECL_POKE( D001 );
					NES_DECL_POKE( D002 );
					NES_DECL_POKE( D003 );
					NES_DECL_POKE( E000 );
					NES_DECL_POKE( E001 );
					NES_DECL_POKE( E002 );
					NES_DECL_POKE( E003 );
					NES_DECL_POKE( F000 );
					NES_DECL_POKE( F001 );
					NES_DECL_POKE( F003 );

					struct Irq
					{
						void Reset(bool);
						bool Clock();

						uint mask;
						uint count;
						uint latch;
					};

					ClockUnits::M2<Irq> irq;
					uint reg;
					Sound::Player* const sound;
				};
			}
		}
	}
}

#endif
