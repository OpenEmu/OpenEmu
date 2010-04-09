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

#ifndef NST_BOARD_TAITO_X1005_H
#define NST_BOARD_TAITO_X1005_H

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
				class X1005 : public Board
				{
				public:

					explicit X1005(const Context&);

				private:

					enum Version
					{
						VERSION_A,
						VERSION_B
					};

					enum
					{
						SECURITY_DATA = 0xA3
					};

					void SubReset(bool);
					void Load(File&);
					void Save(File&) const;
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);

					static Version DetectVersion(const Context&);

					NES_DECL_POKE( 7EF0_0 );
					NES_DECL_POKE( 7EF0_1 );
					NES_DECL_POKE( 7EF2   );
					NES_DECL_PEEK( 7EF8   );
					NES_DECL_POKE( 7EF8   );
					NES_DECL_PEEK( 7F00   );
					NES_DECL_POKE( 7F00   );

					uint security;
					byte ram[0x80];
					const Version version;
				};
			}
		}
	}
}

#endif
