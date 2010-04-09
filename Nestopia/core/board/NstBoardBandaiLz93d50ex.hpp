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

#ifndef NST_BOARD_BANDAI_LZ93D50EX_H
#define NST_BOARD_BANDAI_LZ93D50EX_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				class Lz93d50Ex : public Lz93d50
				{
				public:

					explicit Lz93d50Ex(const Context&);

				protected:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);

				private:

					void Save(File&) const;
					void Load(File&);

					NES_DECL_PEEK( 6000_24c01       );
					NES_DECL_PEEK( 6000_24c02       );
					NES_DECL_PEEK( 6000_24c01_24c02 );
					NES_DECL_POKE( 8000_24c01_24c02 );
					NES_DECL_POKE( 800D_24c01       );
					NES_DECL_POKE( 800D_24c02       );
					NES_DECL_POKE( 800D_24c01_24c02 );

					Pointer<X24C01> x24c01;
					Pointer<X24C02> x24c02;
				};
			}
		}
	}
}

#endif
