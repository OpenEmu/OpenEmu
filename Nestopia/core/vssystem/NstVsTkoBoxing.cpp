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
#include "../NstState.hpp"
#include "../vssystem/NstVsSystem.hpp"
#include "../vssystem/NstVsTkoBoxing.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Cartridge::VsSystem::TkoBoxing::Reset()
		{
			cpu.Map( 0x5E00 ).Set( &TkoBoxing::Peek_5E00 );
			cpu.Map( 0x5E01 ).Set( &TkoBoxing::Peek_5E01 );

			counter = 0;
		}

		void Cartridge::VsSystem::TkoBoxing::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'T','K','O'>::V ).Write8( counter & 0x1F ).End();
		}

		void Cartridge::VsSystem::TkoBoxing::SubLoad(State::Loader& state,const dword chunk)
		{
			if (chunk == AsciiId<'T','K','O'>::V)
				counter = state.Read8() & 0x1F;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Cartridge::VsSystem::TkoBoxing,5E00)
		{
			counter = 0;
			return 0;
		}

		NES_PEEK(Cartridge::VsSystem::TkoBoxing,5E01)
		{
			static const byte securityData[32] =
			{
				0xFF, 0xBF, 0xB7, 0x97, 0x97, 0x17, 0x57, 0x4F,
				0x6F, 0x6B, 0xEB, 0xA9, 0xB1, 0x90, 0x94, 0x14,
				0x56, 0x4E, 0x6F, 0x6B, 0xEB, 0xA9, 0xB1, 0x90,
				0xD4, 0x5C, 0x3E, 0x26, 0x87, 0x83, 0x13, 0x00
			};

			return securityData[counter++ & 0x1F];
		}
	}
}
