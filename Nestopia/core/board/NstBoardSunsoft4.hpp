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

#ifndef NST_BOARD_SUNSOFT_SUNSOFT4_H
#define NST_BOARD_SUNSOFT_SUNSOFT4_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sunsoft
			{
				class S4 : public Board
				{
				public:

					explicit S4(const Context& c)
					: Board(c) {}

				protected:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);

				private:

					void UpdateMirroring() const;

					NES_DECL_POKE( C000 );
					NES_DECL_POKE( D000 );
					NES_DECL_POKE( E000 );

					struct Regs
					{
						enum
						{
							CTRL_MIRRORING = 0x03,
							CTRL_CROM      = 0x10,
							BANK_OFFSET    = 0x80
						};

						uint ctrl;
						uint prg;
						uint nmt[2];
					};

					Regs regs;
				};
			}
		}
	}
}

#endif
