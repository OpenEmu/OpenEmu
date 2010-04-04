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

#include "NstBoard.hpp"
#include "../NstDipSwitches.hpp"
#include "../NstLog.hpp"
#include "NstBoardFb.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Fb::Fb(const Context& c)
			: Board(c), cartSwitch(wrk) {}

			void Fb::SubReset(const bool hard)
			{
				cartSwitch.Reset( hard );

				switch (board.GetWram())
				{
					case SIZE_8K:

						Map( 0x6000U, 0x7FFFU, &Fb::Peek_Wrk_6, &Fb::Poke_Wrk_6 );
						break;

					case SIZE_4K:

						Map( 0x6000U, 0x7000U, &Fb::Peek_Wrk_6, &Fb::Poke_Wrk_6 );
						break;

					case SIZE_2K:

						Map( 0x7000U, 0x7800U, &Fb::Peek_Wrk_7, &Fb::Poke_Wrk_7 );
						break;
				}
			}

			void Fb::Sync(Event event,Input::Controllers* controllers)
			{
				if (event == EVENT_POWER_OFF)
					cartSwitch.Flush();

				Board::Sync( event, controllers );
			}

			Fb::Device Fb::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_DIP_SWITCHES)
					return &cartSwitch;
				else
					return Board::QueryDevice( type );
			}

			Fb::CartSwitch::CartSwitch(Wrk& w)
			: wrk(w), init(true) {}

			void Fb::CartSwitch::Reset(bool hard)
			{
				if (init)
				{
					init = false;
				}
				else if (hard)
				{
					Flush();
				}
			}

			void Fb::CartSwitch::Flush() const
			{
				if (wrk.Source().Writable())
				{
					wrk.Source().Fill( 0x00 );
					Log::Flush( "Fb: battery-switch OFF, discarding W-RAM.." NST_LINEBREAK );
				}
			}

			uint Fb::CartSwitch::NumDips() const
			{
				return 1;
			}

			uint Fb::CartSwitch::NumValues(uint) const
			{
				return 2;
			}

			cstring Fb::CartSwitch::GetDipName(uint) const
			{
				return "Backup Switch";
			}

			cstring Fb::CartSwitch::GetValueName(uint,uint value) const
			{
				NST_ASSERT( value < 2 );
				return value == 0 ? "Off" : "On";
			}

			uint Fb::CartSwitch::GetValue(uint) const
			{
				return wrk.Source().Writable() ? 0 : 1;
			}

			void Fb::CartSwitch::SetValue(uint,uint value)
			{
				NST_ASSERT( value < 2 );
				wrk.Source().SetSecurity( true, !value );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE_AD(Fb,Wrk_6)
			{
				NST_VERIFY( wrk.Writable(0) );

				if (wrk.Writable(0))
					wrk[0][address - 0x6000] = data;
			}

			NES_PEEK_A(Fb,Wrk_6)
			{
				return wrk[0][address - 0x6000];
			}

			NES_POKE_AD(Fb,Wrk_7)
			{
				NST_VERIFY( wrk.Writable(0) );

				if (wrk.Writable(0))
					wrk[0][address - 0x7000] = data;
			}

			NES_PEEK_A(Fb,Wrk_7)
			{
				return wrk[0][address - 0x7000];
			}
		}
	}
}
