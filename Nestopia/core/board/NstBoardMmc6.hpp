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

#ifndef NST_BOARD_MMC6_H
#define NST_BOARD_MMC6_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstBoardMmc3.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Mmc6 : public Mmc3
			{
			protected:

				explicit Mmc6(const Context&);

			private:

				void SubReset(bool);
				void Save(File&) const;
				void Load(File&);
				void SubSave(State::Saver&) const;
				void SubLoad(State::Loader&,dword);

				enum
				{
					RAM_ENABLE           = 0x20,
					RAM_LO_BANK_WRITABLE = 0x10,
					RAM_LO_BANK_ENABLED  = 0x20,
					RAM_HI_BANK_WRITABLE = 0x40,
					RAM_HI_BANK_ENABLED  = 0x80
				};

				inline bool IsRamEnabled() const;
				inline bool IsRamReadable(uint) const;
				inline bool IsRamWritable(uint) const;

				NES_DECL_POKE( 7000 );
				NES_DECL_PEEK( 7000 );
				NES_DECL_POKE( 8000 );
				NES_DECL_POKE( A001 );

				uint reg;
				byte ram[SIZE_1K];
			};
		}
	}
}

#endif
