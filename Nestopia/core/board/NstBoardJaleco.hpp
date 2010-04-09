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

#ifndef NST_BOARD_JALECO_H
#define NST_BOARD_JALECO_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstBoardKonamiVrc1.hpp"
#include "NstBoardJalecoJf11.hpp"
#include "NstBoardJalecoJf13.hpp"
#include "NstBoardJalecoJf16.hpp"
#include "NstBoardJalecoJf17.hpp"
#include "NstBoardJalecoJf19.hpp"
#include "NstBoardJalecoSs88006.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Jaleco
			{
				typedef NRom                  Jf01;
				typedef NRom                  Jf02;
				typedef NRom                  Jf03;
				typedef NRom                  Jf04;
				typedef Discrete::Ic74x139x74 Jf05;
				typedef Discrete::Ic74x139x74 Jf06;
				typedef Discrete::Ic74x139x74 Jf07;
				typedef Discrete::Ic74x139x74 Jf08;
				typedef Discrete::Ic74x139x74 Jf09;
				typedef Discrete::Ic74x139x74 Jf10;
				typedef Jf11                  Jf12;
				typedef Jf11                  Jf14;
				typedef UxRom                 Jf15;
				typedef UxRom                 Jf18;
				typedef Konami::Vrc1          Jf20;
				typedef Jf19                  Jf21;
				typedef Konami::Vrc1          Jf22;
				typedef Ss88006               Jf23;
				typedef Ss88006               Jf24;
				typedef Ss88006               Jf25;
				typedef Jf17                  Jf26;
				typedef Ss88006               Jf27;
				typedef Jf17                  Jf28;
				typedef Ss88006               Jf29;
				typedef Ss88006               Jf30;
				typedef Ss88006               Jf31;
				typedef Ss88006               Jf32;
				typedef Ss88006               Jf33;
				typedef Ss88006               Jf34;
				typedef Ss88006               Jf35;
				typedef Ss88006               Jf36;
				typedef Ss88006               Jf37;
				typedef Ss88006               Jf38;
				typedef UxRom                 Jf39;
				typedef Ss88006               Jf40;
				typedef Ss88006               Jf41;
			}
		}
	}
}

#endif
