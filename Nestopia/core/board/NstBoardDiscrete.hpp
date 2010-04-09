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

#ifndef NST_BOARD_DISCRETE_H
#define NST_BOARD_DISCRETE_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Discrete
			{
				class Ic74x161x161x32 : public Board
				{
				public:

					explicit Ic74x161x161x32(const Context& c)
					: Board(c) {}

				protected:

					void SubReset(bool);

				private:

					NES_DECL_POKE( 8000_0 );
					NES_DECL_POKE( 8000_1 );
				};

				class Ic74x139x74 : public Board
				{
				public:

					explicit Ic74x139x74(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);

					NES_DECL_POKE( 6000 );
				};

				class Ic74x161x138 : public Board
				{
				public:

					explicit Ic74x161x138(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);

					NES_DECL_POKE( 6000 );
				};

				class Ic74x377 : public Board
				{
				public:

					explicit Ic74x377(const Context& c)
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
