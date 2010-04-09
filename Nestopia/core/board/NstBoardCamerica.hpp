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

#ifndef NST_BOARD_CAMERICA_H
#define NST_BOARD_CAMERICA_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Camerica
			{
				class Bf9093 : public Board
				{
				public:

					explicit Bf9093(const Context& c)
					: Board(c) {}

				protected:

					void SubReset(bool);
				};

				class Bf9096 : public Board
				{
				public:

					explicit Bf9096(const Context& c)
					: Board(c) {}

				protected:

					void SubReset(bool);
					void SwapBasePrg(uint);

				private:

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( A000 );
				};

				class Bf9097 : public Bf9093
				{
				public:

					explicit Bf9097(const Context& c)
					: Bf9093(c) {}

				private:

					void SubReset(bool);

					NES_DECL_POKE( 8000 );
				};

				typedef Bf9093 Algnv11;
				typedef Bf9096 Algqv11;

				class GoldenFive : public Board
				{
				public:

					explicit GoldenFive(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( C000 );
				};
			}
		}
	}
}

#endif
