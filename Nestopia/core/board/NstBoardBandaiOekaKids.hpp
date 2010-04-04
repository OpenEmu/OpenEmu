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

#ifndef NST_BOARD_BANDAI_OEKAKIDS_H
#define NST_BOARD_BANDAI_OEKAKIDS_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				class OekaKids : public Board
				{
				public:

					explicit OekaKids(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);
					void UpdateLatch(uint) const;

					NES_DECL_POKE( 8000 );
					NES_DECL_PEEK( 2006 );
					NES_DECL_POKE( 2006 );

					NES_DECL_ACCESSOR( 2000 );
					NES_DECL_ACCESSOR( 2400 );
					NES_DECL_ACCESSOR( 2800 );
					NES_DECL_ACCESSOR( 2C00 );

					Io::Port p2006;
				};
			}
		}
	}
}

#endif
