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

#ifndef NST_BOARD_KAISER_H
#define NST_BOARD_KAISER_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Kaiser
			{
				class Ks7058 : public Board
				{
				public:

					explicit Ks7058(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);
				};

				class Ks202 : public Board
				{
				public:

					explicit Ks202(const Context&);

				protected:

					void SubReset(bool);

				private:

					void SubLoad(State::Loader&,dword);
					void SubSave(State::Saver&) const;
					void Sync(Event,Input::Controllers*);

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 9000 );
					NES_DECL_POKE( A000 );
					NES_DECL_POKE( B000 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( D000 );
					NES_DECL_POKE( E000 );
					NES_DECL_POKE( F000 );

					struct Irq
					{
						void Reset(bool);
						bool Clock();

						uint count;
						uint latch;
						uint ctrl;
					};

					uint ctrl;
					ClockUnits::M2<Irq> irq;
				};

				class Ks7032 : public Ks202
				{
				public:

					explicit Ks7032(const Context& c)
					: Ks202(c) {}

				private:

					void SubReset(bool);

					NES_DECL_PEEK( 6000 );
				};

				class Ks7022 : public Board
				{
				public:

					explicit Ks7022(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);
					void SubSave(State::Saver&) const;

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( A000 );
					NES_DECL_PEEK( FFFC );

					uint reg;
				};
			}
		}
	}
}

#endif
