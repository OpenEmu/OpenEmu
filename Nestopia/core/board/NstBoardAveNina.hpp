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

#ifndef NST_BOARD_AVE_NINA001_H
#define NST_BOARD_AVE_NINA001_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstBoardDiscrete.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Ave
			{
				class Nina001 : public Board
				{
				public:

					explicit Nina001(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);
				};

				typedef Nina001 Nina002;

				class Nina06 : public Board
				{
				public:

					explicit Nina06(const Context& c)
					: Board(c) {}

				protected:

					NES_DECL_POKE( 4100 );

				private:

					void SubReset(bool);
				};

				typedef Nina06 Nina03;
				typedef Discrete::Ic74x377 Nina07;
				typedef Nina06 Mb91;
			}
		}
	}
}

#endif
