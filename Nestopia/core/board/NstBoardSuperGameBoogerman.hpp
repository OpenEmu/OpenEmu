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

#ifndef NST_BOARD_SUPERGAME_BOOGERMAN_H
#define NST_BOARD_SUPERGAME_BOOGERMAN_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace SuperGame
			{
				class Boogerman : public Mmc3
				{
				public:

					explicit Boogerman(const Context& c)
					: Mmc3(c) {}

				private:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);

					void UpdatePrg();
					void NST_FASTCALL UpdatePrg(uint,uint);
					void NST_FASTCALL UpdateChr(uint,uint) const;

					NES_DECL_POKE( 5000 );
					NES_DECL_POKE( 5001 );
					NES_DECL_POKE( 5007 );
					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 8001 );
					NES_DECL_POKE( A000 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( C001 );
					NES_DECL_POKE( E001 );

					byte exRegs[4];
				};
			}
		}
	}
}

#endif
