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

#include "../NstCpu.hpp"
#include "../vssystem/NstVsSystem.hpp"
#include "../vssystem/NstVsSuperXevious.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Cartridge::VsSystem::SuperXevious::Reset()
		{
			cpu.Map( 0x54FF ).Set( &SuperXevious::Peek_54FF );
			cpu.Map( 0x5567 ).Set( &SuperXevious::Peek_5567 );
			cpu.Map( 0x5678 ).Set( &SuperXevious::Peek_5678 );
			cpu.Map( 0x578F ).Set( &SuperXevious::Peek_578F );

			protection = 0;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Cartridge::VsSystem::SuperXevious,54FF) { return 0x05;                              }
		NES_PEEK(Cartridge::VsSystem::SuperXevious,5567) { return (protection ^= 0x1) ? 0x37 : 0x3E; }
		NES_PEEK(Cartridge::VsSystem::SuperXevious,5678) { return protection ? 0x00 : 0x01;          }
		NES_PEEK(Cartridge::VsSystem::SuperXevious,578F) { return protection ? 0xD1 : 0x89;          }
	}
}
