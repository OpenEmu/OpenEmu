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

#ifndef NST_BOARD_SOMERITEAM_SL12_H
#define NST_BOARD_SOMERITEAM_SL12_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace SomeriTeam
			{
				class Sl12 : public Board
				{
				public:

					explicit Sl12(const Context&);

				private:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Sync(Event,Input::Controllers*);

					void UpdatePrg();
					void UpdateChr() const;
					void UpdateNmt() const;

					void Poke_Mmc1_8000(uint,uint);

					void Poke_Vrc2_8000(uint,uint);
					void Poke_Vrc2_9000(uint,uint);
					void Poke_Vrc2_B000(uint,uint);

					void Poke_Mmc3_8000(uint,uint);
					void Poke_Mmc3_A000(uint,uint);
					void Poke_Mmc3_C000(uint,uint);
					void Poke_Mmc3_E000(uint,uint);

					NES_DECL_POKE( 4100 );
					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 9000 );
					NES_DECL_POKE( A000 );
					NES_DECL_POKE( B000 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( D000 );
					NES_DECL_POKE( E000 );
					NES_DECL_POKE( F000 );

					uint mode;

					struct
					{
						byte chr[8];
						byte prg[2];
						byte nmt;
						byte padding;
					}   vrc2;

					struct
					{
						byte banks[10];
						byte ctrl;
						byte nmt;
					}   mmc3;

					struct
					{
						byte regs[4];
						byte buffer;
						byte shifter;
						byte padding[2];
					}   mmc1;

					Mmc3::Irq<> irq;
				};
			}
		}
	}
}

#endif
