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

#ifndef NST_BOARD_BTL_AX5705_H
#define NST_BOARD_BTL_AX5705_H

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
				class Ax5705 : public Board
				{
				public:

					explicit Ax5705(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);
					void SwapChr(uint,uint) const;
					void SwapChrLo(uint,uint) const;
					void SwapChrHi(uint,uint) const;

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( A008 );
					NES_DECL_POKE( A009 );
					NES_DECL_POKE( A00A );
					NES_DECL_POKE( A00B );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( C001 );
					NES_DECL_POKE( C002 );
					NES_DECL_POKE( C003 );
					NES_DECL_POKE( C008 );
					NES_DECL_POKE( C009 );
					NES_DECL_POKE( C00A );
					NES_DECL_POKE( C00B );
					NES_DECL_POKE( E000 );
					NES_DECL_POKE( E001 );
					NES_DECL_POKE( E002 );
					NES_DECL_POKE( E003 );
				};
			}
		}
	}
}

#endif
