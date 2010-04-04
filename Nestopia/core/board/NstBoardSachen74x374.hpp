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

#ifndef NST_BOARD_SACHEN_74X374_H
#define NST_BOARD_SACHEN_74X374_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sachen
			{
				class S74x374a : public Board
				{
				public:

					explicit S74x374a(const Context& c)
					: Board(c) {}

				protected:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);

					void UpdatePrg(uint);
					void UpdateChr(uint) const;
					void UpdateNmt(uint) const;

					uint ctrl;

				private:

					NES_DECL_POKE( 4100 );
					NES_DECL_POKE( 4101 );
				};

				class S74x374b : public S74x374a
				{
				public:

					S74x374b(const Context&);

				private:

					~S74x374b();

					class CartSwitches;

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					Device QueryDevice(DeviceType);

					NES_DECL_PEEK( 4100 );
					NES_DECL_POKE( 4101 );

					CartSwitches* const cartSwitches;
				};
			}
		}
	}
}

#endif
