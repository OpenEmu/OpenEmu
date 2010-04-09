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

#ifndef NST_BOARD_TENGEN_H
#define NST_BOARD_TENGEN_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstBoardTengenRambo1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Tengen
			{
				typedef Namcot::N34x3 Mimic1;
				typedef Sunsoft::S4 T337007;

				typedef Mimic1  T800002;
				typedef Mimic1  T800004;
				typedef Mimic1  T800030;
				typedef Rambo1  T800032;
				typedef T337007 T800042;

				class T800008 : public Board
				{
				public:

					explicit T800008(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);

					NES_DECL_POKE( 8000 );
				};

				class T800037 : public Rambo1
				{
				public:

					explicit T800037(const Context& c)
					: Rambo1(c) {}

				private:

					void SubReset(bool);
					void UpdateChr() const;
				};
			}
		}
	}
}

#endif
