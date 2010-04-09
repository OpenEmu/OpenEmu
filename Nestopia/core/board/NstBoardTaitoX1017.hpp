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

#ifndef NST_BOARD_TAITO_X1017_H
#define NST_BOARD_TAITO_X1017_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Taito
			{
				class X1017 : public Board
				{
				public:

					explicit X1017(const Context&);

				private:

					void SubReset(bool);
					void Load(File&);
					void Save(File&) const;
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void StoreChr();
					void UpdateChr() const;

					enum
					{
						SECURITY_2K_0_0 = 0xCA,
						SECURITY_2K_0_1 = 0x69,
						SECURITY_1K_0_2 = 0x84,
						SECURITY_2K_1_0 = 0x35,
						SECURITY_2K_1_1 = 0x96,
						SECURITY_1K_1_2 = 0x7B
					};

					NES_DECL_PEEK( 6000 );
					NES_DECL_POKE( 6000 );
					NES_DECL_POKE( 7EF0 );
					NES_DECL_POKE( 7EF2 );
					NES_DECL_POKE( 7EF6 );
					NES_DECL_POKE( 7EF7 );
					NES_DECL_POKE( 7EFA );
					NES_DECL_POKE( 7EFB );
					NES_DECL_POKE( 7EFC );
					NES_DECL_POKE( 7EFD );

					struct
					{
						uint ctrl;
						byte security[3];
						byte unused[3];
						byte chr[6];
					}   regs;

					byte ram[SIZE_5K];
				};
			}
		}
	}
}

#endif
