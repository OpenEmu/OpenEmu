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

#ifndef NST_BOARD_SACHEN_H
#define NST_BOARD_SACHEN_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstBoardSachenS8259.hpp"
#include "NstBoardSachenTca01.hpp"
#include "NstBoardSachenTcu.hpp"
#include "NstBoardSachenSa0036.hpp"
#include "NstBoardSachenSa0037.hpp"
#include "NstBoardSachenSa72007.hpp"
#include "NstBoardSachenSa72008.hpp"
#include "NstBoardSachen74x374.hpp"
#include "NstBoardSachenStreetHeroes.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sachen
			{
				typedef Ave::Nina06 Sa0161m;
			}
		}
	}
}

#endif
