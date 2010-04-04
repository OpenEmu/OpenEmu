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

#ifndef NST_BOARD_TXC_H
#define NST_BOARD_TXC_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstBoardMmc3.hpp"
#include "NstBoardTxcTw.hpp"
#include "NstBoardTxcMxmdhtwo.hpp"
#include "NstBoardTxcPoliceman.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Txc
			{
				class T22211A : public Board
				{
				public:

					explicit T22211A(const Context& c)
					: Board(c) {}

				protected:

					void SubReset(bool);

					byte regs[4];

				private:

					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);

					NES_DECL_PEEK( 4100 );
					NES_DECL_POKE( 4100 );
					NES_DECL_POKE( 8000 );
				};

				class T22211B : public T22211A
				{
				public:

					explicit T22211B(const Context& c)
					: T22211A(c) {}

				private:

					void SubReset(bool);

					NES_DECL_POKE( 8000 );
				};

				class T22211C : public T22211A
				{
				public:

					explicit T22211C(const Context& c)
					: T22211A(c) {}

				private:

					void SubReset(bool);

					NES_DECL_PEEK( 4100 );
				};
			}
		}
	}
}

#endif
