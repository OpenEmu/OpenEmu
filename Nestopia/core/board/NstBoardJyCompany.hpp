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

#ifndef NST_BOARD_JYCOMPANY_H
#define NST_BOARD_JYCOMPANY_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace JyCompany
			{
				class Standard : public Board
				{
				public:

					Standard(const Context&);

				private:

					enum DefaultDipSwitch
					{
						DEFAULT_DIP_NMT_OFF,
						DEFAULT_DIP_NMT_CONTROLLED,
						DEFAULT_DIP_NMT_ON
					};

					class CartSwitches : public DipSwitches
					{
					public:

						CartSwitches(uint,bool);

						inline uint GetSetting() const;
						inline bool IsPpuLatched() const;

					private:

						uint NumDips() const;
						uint NumValues(uint) const;
						cstring GetDipName(uint) const;
						cstring GetValueName(uint,uint) const;
						uint GetValue(uint) const;
						void SetValue(uint,uint);

						uint data;
						const ibool ppuLatched;
					};

					enum
					{
						DIPSWITCH_NMT  = 0x03,
						DIPSWITCH_GAME = 0xC0
					};

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void UpdatePrg();
					void UpdateChr() const;
					void UpdateChrLatch() const;
					void UpdateExChr();
					void UpdateNmt();
					Device QueryDevice(DeviceType);
					void Sync(Event,Input::Controllers*);

					NES_DECL_HOOK( HActive );
					NES_DECL_HOOK( HBlank  );

					NES_DECL_ACCESSOR( Chr_0000 );
					NES_DECL_ACCESSOR( Chr_1000 );

					NES_DECL_PEEK( 5000 );
					NES_DECL_PEEK( 6000 );
					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 9000 );
					NES_DECL_POKE( A000 );
					NES_DECL_POKE( B000 );
					NES_DECL_POKE( B004 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( C001 );
					NES_DECL_POKE( C002 );
					NES_DECL_POKE( C003 );
					NES_DECL_POKE( C004 );
					NES_DECL_POKE( C005 );
					NES_DECL_POKE( C006 );
					NES_DECL_POKE( D000 );
					NES_DECL_POKE( D001 );
					NES_DECL_POKE( D002 );
					NES_DECL_POKE( D003 );

					struct Regs
					{
						void Reset();

						enum
						{
							CTRL0_PRG_MODE      = 0x03,
							CTRL0_PRG_SWAP_32K  = 0x00,
							CTRL0_PRG_SWAP_16K  = 0x01,
							CTRL0_PRG_SWAP_8K   = 0x02,
							CTRL0_PRG_SWAP_8K_R = 0x03,
							CTRL0_PRG_NOT_LAST  = 0x04,
							CTRL0_CHR_MODE      = 0x18,
							CTRL0_CHR_SWAP_8K   = 0x00,
							CTRL0_CHR_SWAP_4K   = 0x08,
							CTRL0_CHR_SWAP_2K   = 0x10,
							CTRL0_CHR_SWAP_1K   = 0x18,
							CTRL0_NMT_CHR       = 0x20,
							CTRL0_NMT_CHR_ROM   = 0x40,
							CTRL0_PRG6_ENABLE   = 0x80,
							CTRL1_MIRRORING     = 0x03,
							CTRL2_NMT_USE_RAM   = 0x80,
							CTRL3_NO_EX_CHR     = 0x20,
							CTRL3_EX_CHR_0      = 0x01,
							CTRL3_EX_CHR_1      = 0x18,
							CTRL3_EX_PRG        = 0x06
						};

						NES_DECL_PEEK( 5001 );
						NES_DECL_PEEK( 5800 );
						NES_DECL_POKE( 5800 );
						NES_DECL_POKE( 5801 );
						NES_DECL_PEEK( 5801 );
						NES_DECL_POKE( 5803 );
						NES_DECL_PEEK( 5803 );

						uint mul[2];
						uint tmp;
						uint ctrl[4];
					};

					struct Banks
					{
						void Reset();

						static uint Unscramble(uint);

						uint prg[4];
						uint chr[8];
						uint nmt[4];

						struct
						{
							uint mask;
							uint bank;
						}   exChr;

						const byte* prg6;
						uint chrLatch[2];
					};

					struct Irq
					{
						struct A12
						{
							explicit A12(Irq&);

							void Reset(bool);
							bool Clock();

							Irq& base;
						};

						struct M2
						{
							explicit M2(Irq&);

							void Reset(bool);
							bool Clock();

							Irq& base;
						};

						Irq(Cpu&,Ppu&);

						void Reset();
						bool IsEnabled() const;
						bool IsEnabled(uint) const;
						bool Clock();
						inline void Update();

						enum
						{
							TOGGLE            = 0x01,
							MODE_SOURCE       = 0x03,
							MODE_M2           = 0x00,
							MODE_PPU_A12      = 0x01,
							MODE_PPU_READ     = 0x02,
							MODE_CPU_WRITE    = 0x03,
							MODE_SCALE_3BIT   = 0x04,
							MODE_SCALE_ADJUST = 0x08,
							MODE_COUNT_ENABLE = 0xC0,
							MODE_COUNT_UP     = 0x40,
							MODE_COUNT_DOWN   = 0x80
						};

						uint enabled;
						uint mode;
						uint prescaler;
						uint scale;
						uint count;
						uint flip;
						ClockUnits::A12<A12> a12;
						ClockUnits::M2<M2> m2;
					};

					Regs regs;
					Banks banks;
					Irq irq;
					CartSwitches cartSwitches;
				};
			}
		}
	}
}

#endif
