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

#include "../NstLog.hpp"
#include "../NstCpu.hpp"
#include "../NstPpu.hpp"
#include "../NstState.hpp"
#include "NstVsSystem.hpp"
#include "NstVsRbiBaseball.hpp"
#include "NstVsTkoBoxing.hpp"
#include "NstVsSuperXevious.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Cartridge::VsSystem::Dip
		{
			class Proxy;

			struct Setting
			{
				uint data;
				cstring name;
			};

			Setting* settings;
			uint size;
			uint selection;
			uint mask;
			cstring name;

		public:

			struct Value
			{
				cstring const name;
				const uint data;
				const uint selection;

				Value(cstring n,uint d,uint s=0)
				: name(n), data(d), selection(s) {}
			};

			void operator = (const Value&);

		private:

			class Proxy
			{
				Dip& dip;
				const uint index;

			public:

				Proxy(Dip& d,uint i)
				: dip(d), index(i) {}

				void operator = (const Value& value)
				{
					dip.settings[index].data = value.data;
					dip.settings[index].name = value.name;
				}

				operator uint() const
				{
					return dip.settings[index].data;
				}

				cstring Name() const
				{
					return dip.settings[index].name;
				}
			};

		public:

			Dip()
			: settings(NULL) {}

			~Dip()
			{
				delete [] settings;
			}

			uint Size() const
			{
				return size;
			}

			void Select(uint i)
			{
				NST_ASSERT( i < size );
				selection = i;
			}

			uint Selection() const
			{
				return selection;
			}

			Proxy operator [] (uint i)
			{
				NST_ASSERT( i < size );
				return Proxy(*this,i);
			}

			cstring Name() const
			{
				return name;
			}
		};

		void Cartridge::VsSystem::Dip::operator = (const Value& value)
		{
			NST_ASSERT( settings == NULL && value.data && value.selection < value.data );

			name = value.name;
			size = value.data;
			selection = value.selection;
			settings = new Setting [size];
		}

		Cartridge::VsSystem::VsDipSwitches::VsDipSwitches(Dip*& old,uint n)
		: table(old), size(n)
		{
			old = NULL;

			regs[0] = 0;
			regs[1] = 0;

			for (uint i=0; i < n; ++i)
			{
				regs[0] |= (table[i][table[i].Selection()] & DIPSWITCH_4016_MASK) << DIPSWITCH_4016_SHIFT;
				regs[1] |= (table[i][table[i].Selection()] & DIPSWITCH_4017_MASK) << DIPSWITCH_4017_SHIFT;
			}
		}

		Cartridge::VsSystem::VsDipSwitches::~VsDipSwitches()
		{
			delete [] table;
		}

		inline void Cartridge::VsSystem::VsDipSwitches::Reset()
		{
			coinTimer = 0;
			regs[0] &= ~uint(COIN);
		}

		inline uint Cartridge::VsSystem::VsDipSwitches::Reg(uint i) const
		{
			return regs[i];
		}

		uint Cartridge::VsSystem::VsDipSwitches::NumDips() const
		{
			return size;
		}

		uint Cartridge::VsSystem::VsDipSwitches::NumValues(uint dip) const
		{
			NST_ASSERT( dip < size );
			return table[dip].Size();
		}

		cstring Cartridge::VsSystem::VsDipSwitches::GetDipName(uint dip) const
		{
			NST_ASSERT( dip < size );
			return table[dip].Name();
		}

		cstring Cartridge::VsSystem::VsDipSwitches::GetValueName(uint dip,uint value) const
		{
			NST_ASSERT( dip < size && value < table[dip].Size() );
			return table[dip][value].Name();
		}

		uint Cartridge::VsSystem::VsDipSwitches::GetValue(uint dip) const
		{
			NST_ASSERT( dip < size );
			return table[dip].Selection();
		}

		void Cartridge::VsSystem::VsDipSwitches::SetValue(uint dip,uint value)
		{
			NST_ASSERT( dip < size && value < table[dip].Size() );

			const uint old = table[dip].Selection();

			regs[0] &= ~((table[dip][old] & DIPSWITCH_4016_MASK) << DIPSWITCH_4016_SHIFT);
			regs[1] &= ~((table[dip][old] & DIPSWITCH_4017_MASK) << DIPSWITCH_4017_SHIFT);

			table[dip].Select( value );

			regs[0] |= (table[dip][value] & DIPSWITCH_4016_MASK) << DIPSWITCH_4016_SHIFT;
			regs[1] |= (table[dip][value] & DIPSWITCH_4017_MASK) << DIPSWITCH_4017_SHIFT;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Cartridge::VsSystem::VsDipSwitches::BeginFrame(Input::Controllers* const input)
		{
			if (!coinTimer)
			{
				if (input)
				{
					Input::Controllers::VsSystem::callback( input->vsSystem );

					if (input->vsSystem.insertCoin & COIN)
					{
						regs[0] |= input->vsSystem.insertCoin & COIN;
						coinTimer = 20;
					}
				}
			}
			else if (--coinTimer == 15)
			{
				regs[0] &= ~uint(COIN);
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		struct Cartridge::VsSystem::Context
		{
			Dip* dips;
			uint numDips;
			Cpu& cpu;
			Ppu& ppu;
			PpuModel ppuModel;
			Mode mode;
			InputMapper::Type inputMapper;

			Context(Cpu& c,Ppu& p)
			:
			dips        (NULL),
			numDips     (0),
			cpu         (c),
			ppu         (p),
			ppuModel    (PPU_RP2C03B),
			mode        (MODE_STD),
			inputMapper (InputMapper::TYPE_NONE)
			{
			}

			void SetDips(uint n)
			{
				numDips = n;
				dips = new Dip [n];
			}
		};

		Cartridge::VsSystem* Cartridge::VsSystem::Create
		(
			Cpu& cpu,
			Ppu& ppu,
			const PpuModel ppuModel,
			const dword prgCrc
		)
		{
			switch (prgCrc)
			{
				// VS. Dual-System Games are unsupported

				case 0xB90497AA: // Tennis
				case 0x2A909613: // Tennis (alt)
				case 0xBC202DB6: // Tennis P2
				case 0x008A9C16: // Wrecking Crew P1
				case 0x30C42B1E: // Wrecking Crew P2
				case 0xAD407F52: // Balloon Fight P1
				case 0x6AD67502: // Balloon Fight P2
				case 0x18A93B7B: // Mahjong (J)
				case 0xA2AD7D61: // Mahjong (J) (alt)
				case 0xA9A4A6C5: // Mahjong (J) P1
				case 0x78D1D213: // Mahjong (J) P2
				case 0x13A91937: // Baseball P1
				case 0xC4DD2523: // Baseball P1 (alt 1)
				case 0xB5853830: // Baseball P1 (alt 2)
				case 0x968A6E9D: // Baseball P2
				case 0xF64D7252: // Baseball P2 (alt 1)
				case 0xF5DEBF88: // Baseball P2 (alt 2)
				case 0xF42DAB14: // Ice Climber P1
				case 0x7D6B764F: // Ice Climber P2

					throw RESULT_ERR_UNSUPPORTED_VSSYSTEM;
			}

			Context context( cpu, ppu );

			try
			{
				// Credit to the MAME devs for much of the DIP switch info.

				switch (prgCrc)
				{
					case 0xEB2DBA63: // TKO Boxing

						context.SetDips(7);
						context.dips[0]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x01 );
						context.dips[0][2] = Dip::Value( "2 Coins / 1 Credit", 0x02 );
						context.dips[0][3] = Dip::Value( "3 Coins / 1 Credit", 0x03 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x04 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x08 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x10 );
						context.dips[4]    = Dip::Value( "Palette Color",      2, 1 );
						context.dips[4][0] = Dip::Value( "Black",              0x00 );
						context.dips[4][1] = Dip::Value( "White",              0x20 );
						context.dips[5]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x40 );
						context.dips[6]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[6][0] = Dip::Value( "Off",                0x00 );
						context.dips[6][1] = Dip::Value( "On",                 0x80 );

						context.inputMapper = InputMapper::TYPE_1;
						context.mode = MODE_TKO;
						break;

					case 0x135ADF7C: // RBI Baseball

						context.SetDips(4);
						context.dips[0]    = Dip::Value( "Coinage",                4, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",      0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",     0x01 );
						context.dips[0][2] = Dip::Value( "2 Coins / 1 Credit",     0x02 );
						context.dips[0][3] = Dip::Value( "3 Coins / 1 Credit",     0x03 );
						context.dips[1]    = Dip::Value( "Max. 1p/in, 2p/in, Min", 4, 1 );
						context.dips[1][0] = Dip::Value( "2, 1, 3",                0x04 );
						context.dips[1][1] = Dip::Value( "2, 2, 4",                0x0C );
						context.dips[1][2] = Dip::Value( "3, 2, 6",                0x00 );
						context.dips[1][3] = Dip::Value( "4, 3, 7",                0x08 );
						context.dips[2]    = Dip::Value( "Demo Sounds",            2, 1 );
						context.dips[2][0] = Dip::Value( "Off",                    0x00 );
						context.dips[2][1] = Dip::Value( "On",                     0x10 );
						context.dips[3]    = Dip::Value( "PPU",                    5, 0 );
						context.dips[3][0] = Dip::Value( "RP2C03",                 0x20 );
						context.dips[3][1] = Dip::Value( "RP2C04-0001",            0x00 );
						context.dips[3][2] = Dip::Value( "RP2C04-0002",            0x40 );
						context.dips[3][3] = Dip::Value( "RP2C04-0003",            0x80 );
						context.dips[3][4] = Dip::Value( "RP2C04-0004",            0xC0 );

						context.inputMapper = InputMapper::TYPE_2;
						context.mode = MODE_RBI;
						break;

					case 0xED588F00: // Duck Hunt

						context.SetDips(4);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",   0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits",  0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit",  0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",           0x07 );
						context.dips[1]    = Dip::Value( "Difficulty",          4, 1 );
						context.dips[1][0] = Dip::Value( "Easy",                0x00 );
						context.dips[1][1] = Dip::Value( "Normal",              0x08 );
						context.dips[1][2] = Dip::Value( "Hard",                0x10 );
						context.dips[1][3] = Dip::Value( "Very Hard",           0x18 );
						context.dips[2]    = Dip::Value( "Misses per Game",     2, 1 );
						context.dips[2][0] = Dip::Value( "3",                   0x00 );
						context.dips[2][1] = Dip::Value( "5",                   0x20 );
						context.dips[3]    = Dip::Value( "Bonus Life",          4, 0 );
						context.dips[3][0] = Dip::Value( "30000",               0x00 );
						context.dips[3][1] = Dip::Value( "50000",               0x40 );
						context.dips[3][2] = Dip::Value( "80000",               0x80 );
						context.dips[3][3] = Dip::Value( "100000",              0xC0 );

						break;

					case 0x16D3F469: // Ninja Jajamaru Kun (J)

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",   0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits",  0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits",  0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",           0x07 );
						context.dips[1]    = Dip::Value( "Lives",               3, 0 );
						context.dips[1][0] = Dip::Value( "3",                   0x00 );
						context.dips[1][1] = Dip::Value( "4",                   0x10 );
						context.dips[1][2] = Dip::Value( "5",                   0x08 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",      2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                 0x00 );
						context.dips[2][1] = Dip::Value( "On",                  0x20 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",      2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                 0x00 );
						context.dips[3][1] = Dip::Value( "On",                  0x40 );
						context.dips[4]    = Dip::Value( "Demo Sounds",         2, 1 );
						context.dips[4][0] = Dip::Value( "Off",                 0x00 );
						context.dips[4][1] = Dip::Value( "On",                  0x80 );

						context.ppuModel = PPU_RC2C05_01;
						context.inputMapper = InputMapper::TYPE_3;
						break;

					case 0x8850924B: // Tetris

						context.SetDips(6);
						context.dips[0]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x02 );
						context.dips[0][2] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][3] = Dip::Value( "3 Coins / 1 Credit", 0x03 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x04 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x08 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x10 );
						context.dips[4]    = Dip::Value( "Palette Color",      3, 2 );
						context.dips[4][0] = Dip::Value( "Black",              0x40 );
						context.dips[4][1] = Dip::Value( "Green",              0x20 );
						context.dips[4][2] = Dip::Value( "Grey",               0x60 );
						context.dips[5]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x80 );

						context.inputMapper = InputMapper::TYPE_2;
						break;

					case 0x8C0C2DF5: // Top Gun

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",   0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits",  0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit",  0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",           0x07 );
						context.dips[1]    = Dip::Value( "Lives per Coin",      2, 0 );
						context.dips[1][0] = Dip::Value( "3 - 12 Max",          0x00 );
						context.dips[1][1] = Dip::Value( "2 - 9 Max",           0x08 );
						context.dips[2]    = Dip::Value( "Bonus",               4, 0 );
						context.dips[2][0] = Dip::Value( "30k and every 50k",   0x00 );
						context.dips[2][1] = Dip::Value( "50k and every 100k",  0x20 );
						context.dips[2][2] = Dip::Value( "100k and every 150k", 0x10 );
						context.dips[2][3] = Dip::Value( "200k and every 200k", 0x30 );
						context.dips[3]    = Dip::Value( "Difficulty",          2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",              0x00 );
						context.dips[3][1] = Dip::Value( "Hard",                0x40 );
						context.dips[4]    = Dip::Value( "Demo Sounds",         2, 1 );
						context.dips[4][0] = Dip::Value( "Off",                 0x00 );
						context.dips[4][1] = Dip::Value( "On",                  0x80 );

						context.ppuModel = PPU_RC2C05_04;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0x70901B25: // Slalom

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",   0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits",  0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit",  0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",           0x07 );
						context.dips[1]    = Dip::Value( "Freestyle Points",    2, 0 );
						context.dips[1][0] = Dip::Value( "Left / Right",        0x00 );
						context.dips[1][1] = Dip::Value( "Hold Time",           0x08 );
						context.dips[2]    = Dip::Value( "Difficulty",          4, 1 );
						context.dips[2][0] = Dip::Value( "Easy",                0x00 );
						context.dips[2][1] = Dip::Value( "Normal",              0x10 );
						context.dips[2][2] = Dip::Value( "Hard",                0x20 );
						context.dips[2][3] = Dip::Value( "Hardest",             0x30 );
						context.dips[3]    = Dip::Value( "Allow Continue",      2, 1 );
						context.dips[3][0] = Dip::Value( "No",                  0x40 );
						context.dips[3][1] = Dip::Value( "Yes",                 0x00 );
						context.dips[4]    = Dip::Value( "Inverted input",      2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                 0x00 );
						context.dips[4][1] = Dip::Value( "On",                  0x80 );

						context.ppuModel = PPU_RP2C04_0002;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0xCF36261E: // Super Sky Kid

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[0][0] = Dip::Value( "Off",                0x00 );
						context.dips[0][1] = Dip::Value( "On",                 0x01 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x02 );
						context.dips[2]    = Dip::Value( "Lives",              2, 0 );
						context.dips[2][0] = Dip::Value( "2",                  0x00 );
						context.dips[2][1] = Dip::Value( "3",                  0x04 );
						context.dips[3]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[3][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[3][1] = Dip::Value( "1 Coin / 2 Credits", 0x08 );
						context.dips[3][2] = Dip::Value( "2 Coins / 1 Credit", 0x10 );
						context.dips[3][3] = Dip::Value( "3 Coins / 1 Credit", 0x18 );
						context.dips[4]    = Dip::Value( "PPU",                5, 0 );
						context.dips[4][0] = Dip::Value( "RP2C03",             0x20 );
						context.dips[4][1] = Dip::Value( "RP2C04-0001",        0x00 );
						context.dips[4][2] = Dip::Value( "RP2C04-0002",        0x40 );
						context.dips[4][3] = Dip::Value( "RP2C04-0003",        0x80 );
						context.dips[4][4] = Dip::Value( "RP2C04-0004",        0xC0 );

						context.inputMapper = InputMapper::TYPE_3;
						break;

					case 0xE1AA8214: // Star Luster

						context.SetDips(6);
						context.dips[0]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x02 );
						context.dips[0][2] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][3] = Dip::Value( "3 Coins / 1 Credit", 0x03 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x04 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x08 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x10 );
						context.dips[4]    = Dip::Value( "Palette Color",      3, 0 );
						context.dips[4][0] = Dip::Value( "Black",              0x40 );
						context.dips[4][1] = Dip::Value( "Green",              0x20 );
						context.dips[4][2] = Dip::Value( "Grey",               0x60 );
						context.dips[5]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x80 );

						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0xD5D7EAC4: // Dr. Mario

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Drop Rate Increases After", 4, 0 );
						context.dips[0][0] = Dip::Value( "7 Pills",                   0x00 );
						context.dips[0][1] = Dip::Value( "8 Pills",                   0x01 );
						context.dips[0][2] = Dip::Value( "9 Pills",                   0x02 );
						context.dips[0][3] = Dip::Value( "10 Pills",                  0x03 );
						context.dips[1]    = Dip::Value( "Virus Level",               4, 0 );
						context.dips[1][0] = Dip::Value( "1",                         0x00 );
						context.dips[1][1] = Dip::Value( "3",                         0x04 );
						context.dips[1][2] = Dip::Value( "5",                         0x08 );
						context.dips[1][3] = Dip::Value( "7",                         0x0C );
						context.dips[2]    = Dip::Value( "Drop Speed Up",             4, 0 );
						context.dips[2][0] = Dip::Value( "Slow",                      0x00 );
						context.dips[2][1] = Dip::Value( "Medium",                    0x10 );
						context.dips[2][2] = Dip::Value( "Fast",                      0x20 );
						context.dips[2][3] = Dip::Value( "Fastest",                   0x30 );
						context.dips[3]    = Dip::Value( "Free Play",                 2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                       0x00 );
						context.dips[3][1] = Dip::Value( "On",                        0x40 );
						context.dips[4]    = Dip::Value( "Demo Sounds",               2, 1 );
						context.dips[4][0] = Dip::Value( "Off",                       0x00 );
						context.dips[4][1] = Dip::Value( "On",                        0x80 );

						context.ppuModel = PPU_RP2C04_0003;
						context.inputMapper = InputMapper::TYPE_2;
						break;

					case 0xFFBEF374: // Castlevania

						context.SetDips(4);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit", 0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Lives",              2, 1 );
						context.dips[1][0] = Dip::Value( "2",                  0x08 );
						context.dips[1][1] = Dip::Value( "3",                  0x00 );
						context.dips[2]    = Dip::Value( "Bonus",              4, 0 );
						context.dips[2][0] = Dip::Value( "100k",               0x00 );
						context.dips[2][1] = Dip::Value( "200k",               0x20 );
						context.dips[2][2] = Dip::Value( "300k",               0x10 );
						context.dips[2][3] = Dip::Value( "400k",               0x30 );
						context.dips[3]    = Dip::Value( "Difficulty",         2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",             0x00 );
						context.dips[3][1] = Dip::Value( "Hard",               0x40 );

						context.ppuModel = PPU_RP2C04_0002;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0xE2C0A2BE: // Platoon

						context.SetDips(6);
						context.dips[0]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[0][0] = Dip::Value( "Off",                0x00 );
						context.dips[0][1] = Dip::Value( "On",                 0x01 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x02 );
						context.dips[2]    = Dip::Value( "Demo Sounds",        2, 1 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x04 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x08 );
						context.dips[4]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x10 );
						context.dips[5]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[5][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[5][1] = Dip::Value( "1 Coin / 2 Credits", 0x20 );
						context.dips[5][2] = Dip::Value( "1 Coin / 3 Credits", 0x40 );
						context.dips[5][3] = Dip::Value( "2 Coins / 1 Credit", 0x60 );
						context.dips[5][4] = Dip::Value( "3 Coins / 1 Credit", 0x80 );
						context.dips[5][5] = Dip::Value( "4 Coins / 1 Credit", 0xA0 );
						context.dips[5][6] = Dip::Value( "5 Coins / 1 Credit", 0xC0 );
						context.dips[5][7] = Dip::Value( "Free Play",          0xE0 );

						context.ppuModel = PPU_RP2C04_0001;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0xCBE85490: // Excitebike
					case 0x29155E0C: // Excitebike (alt)

						context.SetDips(4);
						context.dips[0]    = Dip::Value( "Coinage",                  8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",        0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",       0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits",       0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits",       0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit",       0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit",       0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit",       0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",                0x07 );
						context.dips[1]    = Dip::Value( "Bonus",                    4, 0 );
						context.dips[1][0] = Dip::Value( "100k and Every 50k",       0x00 );
						context.dips[1][1] = Dip::Value( "Every 100k",               0x10 );
						context.dips[1][2] = Dip::Value( "100k Only",                0x08 );
						context.dips[1][3] = Dip::Value( "None",                     0x18 );
						context.dips[2]    = Dip::Value( "1st Half Qualifying Time", 2, 0 );
						context.dips[2][0] = Dip::Value( "Normal",                   0x00 );
						context.dips[2][1] = Dip::Value( "Hard",                     0x20 );
						context.dips[3]    = Dip::Value( "2nd Half Qualifying Time", 2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",                   0x00 );
						context.dips[3][1] = Dip::Value( "Hard",                     0x40 );

						if (prgCrc == 0x29155E0C)
							context.ppuModel = PPU_RP2C04_0004;
						else
							context.ppuModel = PPU_RP2C04_0003;

						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0x07138C06: // Clu Clu Land

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x08 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x10 );
						context.dips[3]    = Dip::Value( "Lives",              4, 1 );
						context.dips[3][0] = Dip::Value( "2",                  0x60 );
						context.dips[3][1] = Dip::Value( "3",                  0x00 );
						context.dips[3][2] = Dip::Value( "4",                  0x40 );
						context.dips[3][3] = Dip::Value( "5",                  0x20 );
						context.dips[4]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x80 );

						context.ppuModel = PPU_RP2C04_0004;
						context.inputMapper = InputMapper::TYPE_2;
						break;

					case 0x43A357EF: // Ice Climber
					case 0xD4EB5923: // -||-

						context.SetDips(4);
						context.dips[0]    = Dip::Value( "Coinage",              8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",    0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",   0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits",   0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits",   0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit",   0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit",   0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit",   0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",            0x07 );
						context.dips[1]    = Dip::Value( "Lives",                4, 0 );
						context.dips[1][0] = Dip::Value( "3",                    0x00 );
						context.dips[1][1] = Dip::Value( "4",                    0x10 );
						context.dips[1][2] = Dip::Value( "5",                    0x08 );
						context.dips[1][3] = Dip::Value( "7",                    0x18 );
						context.dips[2]    = Dip::Value( "Difficulty",           2, 0 );
						context.dips[2][0] = Dip::Value( "Normal",               0x00 );
						context.dips[2][1] = Dip::Value( "Hard",                 0x20 );
						context.dips[3]    = Dip::Value( "Time before the bear", 2, 0 );
						context.dips[3][0] = Dip::Value( "Long",                 0x00 );
						context.dips[3][1] = Dip::Value( "Short",                0x40 );

						context.ppuModel = PPU_RP2C04_0004;

						if (prgCrc == 0x43A357EF)
							context.inputMapper = InputMapper::TYPE_2;
						else
							context.inputMapper = InputMapper::TYPE_4;

						break;

					case 0x737DD1BF: // Super Mario Bros
					case 0x4BF3972D: // -||-
					case 0x8B60CC58: // -||-
					case 0x8192C804: // -||-

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x06 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x01 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits", 0x05 );
						context.dips[0][4] = Dip::Value( "1 Coin / 5 Credits", 0x03 );
						context.dips[0][5] = Dip::Value( "2 Coins / 1 Credit", 0x04 );
						context.dips[0][6] = Dip::Value( "3 Coins / 1 Credit", 0x02 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Lives",              2, 1 );
						context.dips[1][0] = Dip::Value( "2",                  0x08 );
						context.dips[1][1] = Dip::Value( "3",                  0x00 );
						context.dips[2]    = Dip::Value( "Bonus Life",         4, 0 );
						context.dips[2][0] = Dip::Value( "100",                0x00 );
						context.dips[2][1] = Dip::Value( "150",                0x20 );
						context.dips[2][2] = Dip::Value( "200",                0x10 );
						context.dips[2][3] = Dip::Value( "250",                0x30 );
						context.dips[3]    = Dip::Value( "Timer",              2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",             0x00 );
						context.dips[3][1] = Dip::Value( "Fast",               0x40 );
						context.dips[4]    = Dip::Value( "Continue Lives",     2, 0 );
						context.dips[4][0] = Dip::Value( "3",                  0x80 );
						context.dips[4][1] = Dip::Value( "4",                  0x00 );

						context.ppuModel = PPU_RP2C04_0004;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0xEC461DB9: // Pinball
					case 0xE528F651: // -||- (alt)

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x01 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x06 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits", 0x04 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x05 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x03 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x07 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x00 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x08 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x10 );
						context.dips[3]    = Dip::Value( "Balls",              4, 1 );
						context.dips[3][0] = Dip::Value( "2",                  0x60 );
						context.dips[3][1] = Dip::Value( "3",                  0x00 );
						context.dips[3][2] = Dip::Value( "4",                  0x40 );
						context.dips[3][3] = Dip::Value( "5",                  0x20 );
						context.dips[4]    = Dip::Value( "Ball Speed",         2, 0 );
						context.dips[4][0] = Dip::Value( "Normal",             0x00 );
						context.dips[4][1] = Dip::Value( "Fast",               0x80 );

						if (prgCrc == 0xEC461DB9)
						{
							context.ppuModel = PPU_RP2C04_0001;
							context.inputMapper = InputMapper::TYPE_1;
						}
						else
						{
							context.inputMapper = InputMapper::TYPE_5;
						}
						break;

					case 0xAE8063EF: // Mach Rider - Fighting Course

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Km 1st Race",        2, 0 );
						context.dips[1][0] = Dip::Value( "12",                 0x00 );
						context.dips[1][1] = Dip::Value( "15",                 0x10 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x20 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x40 );
						context.dips[4]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x80 );

						context.ppuModel = PPU_RP2C04_0001;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0x0B65A917: // Mach Rider
					case 0x8A6A9848: // -||-

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Time",               4, 0 );
						context.dips[1][0] = Dip::Value( "280",                0x00 );
						context.dips[1][1] = Dip::Value( "250",                0x10 );
						context.dips[1][2] = Dip::Value( "220",                0x08 );
						context.dips[1][3] = Dip::Value( "200",                0x18 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x20 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x40 );
						context.dips[4]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x80 );

						context.ppuModel = PPU_RP2C04_0002;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0x46914E3E: // Soccer

						context.SetDips(3);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Points Timer",       4, 2 );
						context.dips[1][0] = Dip::Value( "600 Pts",            0x00 );
						context.dips[1][1] = Dip::Value( "800 Pts",            0x10 );
						context.dips[1][2] = Dip::Value( "1000 Pts",           0x08 );
						context.dips[1][3] = Dip::Value( "1200 Pts",           0x18 );
						context.dips[2]    = Dip::Value( "Difficulty",         4, 1 );
						context.dips[2][0] = Dip::Value( "Easy",               0x00 );
						context.dips[2][1] = Dip::Value( "Normal",             0x40 );
						context.dips[2][2] = Dip::Value( "Hard",               0x20 );
						context.dips[2][3] = Dip::Value( "Very Hard",          0x60 );

						context.ppuModel = PPU_RP2C04_0003;
						context.inputMapper = InputMapper::TYPE_2;
						break;

					case 0x70433F2C: // Battle City

						context.SetDips(7);
						context.dips[0]    = Dip::Value( "Credits for 2 Players", 2, 1 );
						context.dips[0][0] = Dip::Value( "1",                     0x00 );
						context.dips[0][1] = Dip::Value( "2",                     0x01 );
						context.dips[1]    = Dip::Value( "Lives",                 2, 0 );
						context.dips[1][0] = Dip::Value( "3",                     0x00 );
						context.dips[1][1] = Dip::Value( "5",                     0x02 );
						context.dips[2]    = Dip::Value( "Demo Sounds",           2, 1 );
						context.dips[2][0] = Dip::Value( "Off",                   0x00 );
						context.dips[2][1] = Dip::Value( "On",                    0x04 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",        2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                   0x00 );
						context.dips[3][1] = Dip::Value( "On",                    0x08 );
						context.dips[4]    = Dip::Value( "Unknown/Unused",        2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                   0x00 );
						context.dips[4][1] = Dip::Value( "On",                    0x10 );
						context.dips[5]    = Dip::Value( "Unknown/Unused",        2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                   0x00 );
						context.dips[5][1] = Dip::Value( "On",                    0x20 );
						context.dips[6]    = Dip::Value( "PPU",                   4, 0 );
						context.dips[6][0] = Dip::Value( "RP2C04-0001",           0x00 );
						context.dips[6][1] = Dip::Value( "RP2C04-0002",           0x40 );
						context.dips[6][2] = Dip::Value( "RP2C04-0003",           0x80 );
						context.dips[6][3] = Dip::Value( "RP2C04-0004",           0xC0 );

						context.ppuModel = PPU_RP2C04_0001;
						context.inputMapper = InputMapper::TYPE_2;
						break;

					case 0xD99A2087: // Gradius

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit", 0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Lives",              2, 0 );
						context.dips[1][0] = Dip::Value( "3",                  0x08 );
						context.dips[1][1] = Dip::Value( "4",                  0x00 );
						context.dips[2]    = Dip::Value( "Bonus",              4, 0 );
						context.dips[2][0] = Dip::Value( "100k",               0x00 );
						context.dips[2][1] = Dip::Value( "200k",               0x20 );
						context.dips[2][2] = Dip::Value( "300k",               0x10 );
						context.dips[2][3] = Dip::Value( "400k",               0x30 );
						context.dips[3]    = Dip::Value( "Difficulty",         2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",             0x00 );
						context.dips[3][1] = Dip::Value( "Hard",               0x40 );
						context.dips[4]    = Dip::Value( "Demo Sounds",        2, 1 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x80 );

						context.ppuModel = PPU_RP2C04_0001;
						context.inputMapper = InputMapper::TYPE_2;
						break;

					case 0x1E438D52: // Goonies

						context.SetDips(6);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit", 0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Lives",              2, 0 );
						context.dips[1][0] = Dip::Value( "3",                  0x00 );
						context.dips[1][1] = Dip::Value( "2",                  0x08 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x10 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x20 );
						context.dips[4]    = Dip::Value( "Timer",              2, 0 );
						context.dips[4][0] = Dip::Value( "Normal",             0x00 );
						context.dips[4][1] = Dip::Value( "Fast",               0x40 );
						context.dips[5]    = Dip::Value( "Demo Sounds",        2, 1 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x80 );

						context.ppuModel = PPU_RP2C04_0003;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0xFF5135A3: // Hogan's Alley

						context.SetDips(4);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit", 0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Difficulty",         4, 1 );
						context.dips[1][0] = Dip::Value( "Easy",               0x00 );
						context.dips[1][1] = Dip::Value( "Normal",             0x08 );
						context.dips[1][2] = Dip::Value( "Hard",               0x10 );
						context.dips[1][3] = Dip::Value( "Very Hard",          0x18 );
						context.dips[2]    = Dip::Value( "Misses per Game",    2, 1 );
						context.dips[2][0] = Dip::Value( "3",                  0x00 );
						context.dips[2][1] = Dip::Value( "5",                  0x20 );
						context.dips[3]    = Dip::Value( "Bonus Life",         4, 0 );
						context.dips[3][0] = Dip::Value( "30000",              0x00 );
						context.dips[3][1] = Dip::Value( "50000",              0x40 );
						context.dips[3][2] = Dip::Value( "80000",              0x80 );
						context.dips[3][3] = Dip::Value( "100000",             0xC0 );

						context.ppuModel = PPU_RP2C04_0001;
						break;

					case 0x17AE56BE: // Freedom Force

						context.SetDips(6);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit", 0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x08 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x10 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x20 );
						context.dips[4]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x40 );
						context.dips[5]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x80 );

						context.ppuModel = PPU_RP2C04_0001;
						break;

					case 0xC99EC059: // Raid on Bungeling Bay

						context.SetDips(6);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x07 );
						context.dips[1]    = Dip::Value( "Lives",              2, 0 );
						context.dips[1][0] = Dip::Value( "2",                  0x00 );
						context.dips[1][1] = Dip::Value( "3",                  0x08 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x10 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x20 );
						context.dips[4]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x40 );
						context.dips[5]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x80 );

						context.ppuModel = PPU_RP2C04_0002;
						context.inputMapper = InputMapper::TYPE_4;
						break;

					case 0xF9D3B0A3: // Super Xevious
					case 0x66BB838F: // -||-
					case 0x9924980A: // -||-

						context.SetDips(6);
						context.dips[0]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[0][0] = Dip::Value( "Off",                0x00 );
						context.dips[0][1] = Dip::Value( "On",                 0x01 );
						context.dips[1]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x02 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x04 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x08 );
						context.dips[4]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[4][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[4][1] = Dip::Value( "1 Coin / 2 Credits", 0x10 );
						context.dips[4][2] = Dip::Value( "2 Coins / 1 Credit", 0x20 );
						context.dips[4][3] = Dip::Value( "3 Coins / 1 Credit", 0x30 );
						context.dips[5]    = Dip::Value( "PPU",                4, 0 );
						context.dips[5][0] = Dip::Value( "RP2C04-0001",        0x00 );
						context.dips[5][1] = Dip::Value( "RP2C04-0002",        0x40 );
						context.dips[5][2] = Dip::Value( "RP2C04-0003",        0x80 );
						context.dips[5][3] = Dip::Value( "RP2C04-0004",        0xC0 );

						context.inputMapper = InputMapper::TYPE_1;
						context.ppuModel = PPU_RP2C04_0001;
						context.mode = MODE_XEV;
						break;

					case 0xCC2C4B5D: // Golf (J)
					case 0x86167220: // Lady Golf

						context.ppuModel = PPU_RP2C04_0002;

					case 0xA93A5AEE: // Golf

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",                 8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",       0x01 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",      0x06 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits",      0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits",      0x04 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit",      0x05 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit",      0x03 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit",      0x07 );
						context.dips[0][7] = Dip::Value( "Free Play",               0x00 );
						context.dips[1]    = Dip::Value( "Hole Size",               2, 0 );
						context.dips[1][0] = Dip::Value( "Large",                   0x00 );
						context.dips[1][1] = Dip::Value( "Small",                   0x08 );
						context.dips[2]    = Dip::Value( "Points per Stroke",       2, 0 );
						context.dips[2][0] = Dip::Value( "Easier",                  0x00 );
						context.dips[2][1] = Dip::Value( "Harder",                  0x10 );
						context.dips[3]    = Dip::Value( "Starting Points",         4, 0 );
						context.dips[3][0] = Dip::Value( "10",                      0x00 );
						context.dips[3][1] = Dip::Value( "13",                      0x40 );
						context.dips[3][2] = Dip::Value( "16",                      0x20 );
						context.dips[3][3] = Dip::Value( "20",                      0x60 );
						context.dips[4]    = Dip::Value( "Difficulty Vs. Computer", 2, 0 );
						context.dips[4][0] = Dip::Value( "Easy",                    0x00 );
						context.dips[4][1] = Dip::Value( "Hard",                    0x80 );

						context.inputMapper = InputMapper::TYPE_2;
						break;

					case 0xCA85E56D: // Mighty Bomb Jack

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coin / 4 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "5 Coins / 1 Credit", 0x07 );
						context.dips[1]    = Dip::Value( "Lives",              4, 0 );
						context.dips[1][0] = Dip::Value( "2",                  0x10 );
						context.dips[1][1] = Dip::Value( "3",                  0x00 );
						context.dips[1][2] = Dip::Value( "4",                  0x08 );
						context.dips[1][3] = Dip::Value( "5",                  0x18 );
						context.dips[2]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x20 );
						context.dips[3]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x40 );
						context.dips[4]    = Dip::Value( "Unknown/Unused",     2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x80 );

						context.ppuModel = PPU_RC2C05_02;
						context.inputMapper = InputMapper::TYPE_1;
						break;

					case 0xFE446787: // Gumshoe

						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coin / 1 Credit",   0x00 );
						context.dips[0][1] = Dip::Value( "1 Coin / 2 Credits",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coin / 3 Credits",  0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit",  0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",           0x07 );
						context.dips[1]    = Dip::Value( "Difficulty",          4, 1 );
						context.dips[1][0] = Dip::Value( "Easy",                0x00 );
						context.dips[1][1] = Dip::Value( "Normal",              0x08 );
						context.dips[1][2] = Dip::Value( "Hard",                0x10 );
						context.dips[1][3] = Dip::Value( "Very Hard",           0x18 );
						context.dips[2]    = Dip::Value( "Lives",               2, 1 );
						context.dips[2][0] = Dip::Value( "3",                   0x10 );
						context.dips[2][1] = Dip::Value( "5",                   0x00 );
						context.dips[3]    = Dip::Value( "Bullets per Balloon", 2, 1 );
						context.dips[3][0] = Dip::Value( "2",                   0x40 );
						context.dips[3][1] = Dip::Value( "3",                   0x00 );
						context.dips[4]    = Dip::Value( "Bonus Life",          2, 0 );
						context.dips[4][0] = Dip::Value( "80000",               0x00 );
						context.dips[4][1] = Dip::Value( "100000",              0x80 );

						context.ppuModel = PPU_RC2C05_03;
						break;

					default:

						context.SetDips(8);

						for (uint i=0; i < 8; ++i)
						{
							context.dips[i]    = Dip::Value( "Unknown", 2, 0    );
							context.dips[i][0] = Dip::Value( "Off",     0x00    );
							context.dips[i][1] = Dip::Value( "On",      1U << i );
						}

						if (ppuModel != PPU_RP2C02)
							context.ppuModel = ppuModel;

						break;
				}

				switch (context.mode)
				{
					case MODE_RBI: return new RbiBaseball  ( context );
					case MODE_TKO: return new TkoBoxing    ( context );
					case MODE_XEV: return new SuperXevious ( context );
					default:       return new VsSystem     ( context );
				}
			}
			catch (...)
			{
				delete [] context.dips;
				throw;
			}
		}

		void Cartridge::VsSystem::Destroy(Cartridge::VsSystem* vsSystem)
		{
			delete vsSystem;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		struct Cartridge::VsSystem::InputMapper::Type1 : InputMapper
		{
			void Fix(Pad (&pads)[4],const uint (&ports)[2]) const
			{
				const uint p[2] = { ports[0] < 4 ? pads[ports[0]].buttons : 0, ports[1] < 4 ? pads[ports[1]].buttons : 0 };

				for (uint i=2; i--; )
				{
					if (ports[i] < 4)
						pads[ports[i]].buttons = (p[i] & ~uint(Pad::SELECT|Pad::START)) | ((p[i] & Pad::SELECT) << 1) | ((p[i] & Pad::START) >> 1);
				}
			}
		};

		struct Cartridge::VsSystem::InputMapper::Type2 : InputMapper
		{
			void Fix(Pad (&pads)[4],const uint (&ports)[2]) const
			{
				const uint p[2] = { ports[0] < 4 ? pads[ports[0]].buttons : 0, ports[1] < 4 ? pads[ports[1]].buttons : 0 };

				for (uint i=2; i--; )
				{
					if (ports[i] < 4)
						pads[ports[i]].buttons = (p[i^1] & ~uint(Pad::SELECT|Pad::START)) | ((p[i^0] & Pad::SELECT) << 1) | ((p[i^0] & Pad::START) >> 1);
				}
			}
		};

		struct Cartridge::VsSystem::InputMapper::Type3 : InputMapper
		{
			void Fix(Pad (&pads)[4],const uint (&ports)[2]) const
			{
				const uint p[2] = { ports[0] < 4 ? pads[ports[0]].buttons : 0, ports[1] < 4 ? pads[ports[1]].buttons : 0 };

				if (ports[1] < 4)
					pads[ports[1]].buttons = p[0] & ~uint(Pad::SELECT|Pad::START);

				if (ports[0] < 4)
					pads[ports[0]].buttons = (p[1] & ~uint(Pad::SELECT|Pad::START)) | ((p[0] & Pad::START) >> 1) | (p[1] & Pad::START);
			}
		};

		struct Cartridge::VsSystem::InputMapper::Type4 : InputMapper
		{
			void Fix(Pad (&pads)[4],const uint (&ports)[2]) const
			{
				const uint p[2] = { ports[0] < 4 ? pads[ports[0]].buttons : 0, ports[1] < 4 ? pads[ports[1]].buttons : 0 };

				for (uint i=2; i--; )
				{
					if (ports[i] < 4)
						pads[ports[i]].buttons = (p[i^1] & ~uint(Pad::SELECT|Pad::START)) | (((p[i^0] & Pad::SELECT) ^ Pad::SELECT) << 1) | ((p[i^0] & Pad::START) >> 1);
				}
			}
		};

		struct Cartridge::VsSystem::InputMapper::Type5 : InputMapper
		{
			void Fix(Pad (&pads)[4],const uint (&ports)[2]) const
			{
				const uint p[2] = { ports[0] < 4 ? pads[ports[0]].buttons : 0, ports[1] < 4 ? pads[ports[1]].buttons : 0 };

				if (ports[1] < 4)
					pads[ports[1]].buttons = (p[1] & ~uint(Pad::A|Pad::SELECT|Pad::START)) | ((p[0] & Pad::B) >> 1) | ((p[1] & Pad::SELECT) << 1) | ((p[1] & Pad::START) >> 1);

				if (ports[0] < 4)
					pads[ports[0]].buttons = (p[0] & ~uint(Pad::B|Pad::SELECT|Pad::START)) | ((p[1] & Pad::A) << 1) | ((p[0] & Pad::SELECT) << 1) | ((p[0] & Pad::START) >> 1);
			}
		};

		void Cartridge::VsSystem::InputMapper::Begin(const Api::Input input,Input::Controllers* const controllers)
		{
			Input::Controllers::Pad::callback.Get( userCallback, userData );

			if (controllers)
			{
				uint ports[2];

				for (uint i=0; i < 2; ++i)
				{
					ports[i] = input.GetConnectedController(i) - Api::Input::PAD1;

					if (ports[i] < 4)
						Input::Controllers::Pad::callback( controllers->pad[ports[i]], ports[i] );
				}

				Input::Controllers::Pad::callback.Set( NULL, NULL );

				Fix( controllers->pad, ports );
			}
		}

		void Cartridge::VsSystem::InputMapper::End() const
		{
			Input::Controllers::Pad::callback.Set( userCallback, userData );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Cartridge::VsSystem::InputMapper* Cartridge::VsSystem::InputMapper::Create(Type type)
		{
			switch (type)
			{
				case TYPE_1: return new Type1;
				case TYPE_2: return new Type2;
				case TYPE_3: return new Type3;
				case TYPE_4: return new Type4;
				case TYPE_5: return new Type5;
			}

			return NULL;
		}

		Cartridge::VsSystem::VsSystem(Context& context)
		:
		cpu         (context.cpu),
		ppu         (context.ppu),
		inputMapper (InputMapper::Create( context.inputMapper )),
		dips        (context.dips,context.numDips),
		ppuModel    (context.ppuModel)
		{
		}

		Cartridge::VsSystem::~VsSystem()
		{
			delete inputMapper;
		}

		void Cartridge::VsSystem::Reset(bool)
		{
			dips.Reset();

			coin = 0;

			p4016 = cpu.Map( 0x4016 );
			p4017 = cpu.Map( 0x4017 );

			cpu.Map( 0x4016 ).Set( this, &VsSystem::Peek_4016, &VsSystem::Poke_4016 );
			cpu.Map( 0x4017 ).Set( this, &VsSystem::Peek_4017, &VsSystem::Poke_4017 );
			cpu.Map( 0x4020 ).Set( this, &VsSystem::Peek_4020, &VsSystem::Poke_4020 );

			cpu.Map( 0x5000, 0x5FFF ).Set( this, &VsSystem::Peek_Nop, &VsSystem::Poke_Nop );

			Reset();
		}

		void Cartridge::VsSystem::SaveState(State::Saver& state,const dword baseChunk) const
		{
			state.Begin( baseChunk );

			state.Write8( coin );
			SubSave( state );

			state.End();
		}

		void Cartridge::VsSystem::LoadState(State::Loader& state)
		{
			coin = state.Read8();

			while (const dword chunk = state.Begin())
			{
				SubLoad( state, chunk );
				state.End();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK_A(Cartridge::VsSystem,Nop)
		{
			return address >> 8;
		}

		NES_POKE(Cartridge::VsSystem,Nop)
		{
		}

		NES_PEEK_A(Cartridge::VsSystem,4016)
		{
			return dips.Reg(0) | p4016.Peek( address ) & (STATUS_4016_MASK^0xFFU);
		}

		NES_POKE_AD(Cartridge::VsSystem,4016)
		{
			p4016.Poke( address, data );
		}

		NES_PEEK_A(Cartridge::VsSystem,4017)
		{
			return dips.Reg(1) | p4017.Peek( address ) & (STATUS_4017_MASK^0xFFU);
		}

		NES_POKE_AD(Cartridge::VsSystem,4017)
		{
			p4017.Poke( address, data );
		}

		NES_PEEK(Cartridge::VsSystem,4020)
		{
			return coin;
		}

		NES_POKE_D(Cartridge::VsSystem,4020)
		{
			coin = data;
		}
	}
}
