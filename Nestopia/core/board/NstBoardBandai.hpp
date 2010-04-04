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

#ifndef NST_BOARD_BANDAI_H
#define NST_BOARD_BANDAI_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "../NstFile.hpp"
#include "NstBoardBandai24c0x.hpp"
#include "NstBoardBandaiLz93d50.hpp"
#include "NstBoardBandaiLz93d50ex.hpp"
#include "NstBoardBandaiDatach.hpp"
#include "NstBoardBandaiKaraokeStudio.hpp"
#include "NstBoardBandaiAerobicsStudio.hpp"
#include "NstBoardBandaiOekaKids.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				typedef Lz93d50 Fcg1;
				typedef Lz93d50 Fcg2;
			}
		}
	}
}

#endif
