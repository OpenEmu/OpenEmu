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

#ifndef NST_BOARD_MMC3_H
#define NST_BOARD_MMC3_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "../NstClock.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Mmc3 : public Board
			{
				class BaseIrq
				{
				public:

					enum
					{
						CLOCK_FILTER = 16
					};

					void Reset(bool);
					void LoadState(State::Loader&);
					void SaveState(State::Saver&,dword) const;

				private:

					uint count;
					uint latch;
					ibool reload;
					ibool enabled;
					const ibool persistant;

				public:

					explicit BaseIrq(bool p=false)
					: persistant(p) {}

					bool Clock()
					{
						uint tmp = count;

						if (!count || reload)
						{
							count = latch;
							reload = false;
						}
						else
						{
							--count;
						}

						return (tmp | persistant) && !count && enabled;
					}

					void SetLatch(uint data)
					{
						latch = data;
					}

					void Reload()
					{
						reload = true;
					}

					void Enable()
					{
						enabled = true;
					}

					void Disable(Cpu& cpu)
					{
						enabled = false;
						cpu.ClearIRQ();
					}
				};

			public:

				enum Revision
				{
					REV_A,
					REV_B,
					REV_C
				};

				template<uint Delay=0>
				struct Irq : ClockUnits::A12<BaseIrq,BaseIrq::CLOCK_FILTER,Delay>
				{
					Irq(Cpu& c,Ppu& p,bool persistant)
					: ClockUnits::A12<BaseIrq,BaseIrq::CLOCK_FILTER,Delay>(c,p,persistant) {}
				};

			protected:

				explicit Mmc3(const Context&,Revision=REV_B);

				void SubReset(bool);
				void SubSave(State::Saver&) const;
				void SubLoad(State::Loader&,dword);
				void Sync(Event,Input::Controllers*);

				void UpdatePrg();
				void UpdateChr() const;

				virtual void NST_FASTCALL UpdatePrg(uint,uint);
				virtual void NST_FASTCALL UpdateChr(uint,uint) const;

				NES_DECL_POKE( 8000 );
				NES_DECL_POKE( 8001 );
				NES_DECL_POKE( A001 );
				NES_DECL_POKE( C000 );
				NES_DECL_POKE( C001 );
				NES_DECL_POKE( E000 );
				NES_DECL_POKE( E001 );

				enum
				{
					CTRL1_WRAM_READONLY = 0x40,
					CTRL1_WRAM_ENABLED  = 0x80,
					CTRL1_WRAM          = CTRL1_WRAM_ENABLED|CTRL1_WRAM_READONLY
				};

				struct Regs
				{
					uint ctrl0;
					uint ctrl1;
				}   regs;

				struct
				{
					byte prg[4];
					byte chr[8];
				}   banks;

			private:

				Irq<> irq;
			};
		}
	}
}

#endif
