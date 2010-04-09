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

#ifndef NST_BOARD_RCM_H
#define NST_BOARD_RCM_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Rcm
			{
				class Gs2015 : public Board
				{
				public:

					explicit Gs2015(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);

					NES_DECL_POKE( 8000 );
				};

				class Gs2013 : public Board
				{
				public:

					explicit Gs2013(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);

					NES_DECL_PEEK( 6000 );
					NES_DECL_POKE( 8000 );
				};

				class Gs2004 : public Board
				{
				public:

					explicit Gs2004(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);

					NES_DECL_PEEK( 6000 );
				};

				class TetrisFamily : public Board
				{
				public:

					explicit TetrisFamily(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);

					NES_DECL_POKE( 8000 );
				};
			}
		}
	}
}

#endif
