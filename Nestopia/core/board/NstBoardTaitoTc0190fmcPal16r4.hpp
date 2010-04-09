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

#ifndef NST_BOARD_TAITO_TC0190FMCPAL16R4_H
#define NST_BOARD_TAITO_TC0190FMCPAL16R4_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstBoardMmc3.hpp"
#include "NstBoardTaitoTc0190fmc.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Taito
			{
				class Tc0190fmcPal16r4 : public Tc0190fmc
				{
				public:

					explicit Tc0190fmcPal16r4(const Context&);

				private:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Sync(Event,Input::Controllers*);

					enum
					{
						IRQ_DELAY = 2
					};

					NES_DECL_POKE( C000 );
					NES_DECL_POKE( C001 );
					NES_DECL_POKE( C002 );
					NES_DECL_POKE( C003 );
					NES_DECL_POKE( E000 );

					Mmc3::Irq<IRQ_DELAY> irq;
				};
			}
		}
	}
}

#endif
