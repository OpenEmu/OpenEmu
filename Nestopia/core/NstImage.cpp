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

#include "NstStream.hpp"
#include "NstCartridge.hpp"
#include "NstFds.hpp"
#include "NstNsf.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Image::Image(Type t)
		: type(t) {}

		Image* Image::Load(Context& context)
		{
			switch (Stream::In(&context.stream).Peek32())
			{
				case INES_ID:
				case UNIF_ID:
				default:

					if (context.type == CARTRIDGE || context.type == UNKNOWN)
						return new Cartridge (context);

					break;

				case FDS_ID:
				case FDS_RAW_ID:

					if (context.type == DISK || context.type == UNKNOWN)
						return new Fds (context);

					break;

				case NSF_ID:

					if (context.type == SOUND || context.type == UNKNOWN)
						return new Nsf (context);

					break;
			}

			throw RESULT_ERR_INVALID_FILE;
		}

		void Image::Unload(Image* image)
		{
			delete image;
		}

		uint Image::GetDesiredController(uint port) const
		{
			switch (port)
			{
				case Api::Input::PORT_1: return Api::Input::PAD1;
				case Api::Input::PORT_2: return Api::Input::PAD2;
				default: return Api::Input::UNCONNECTED;
			}
		}

		uint Image::GetDesiredAdapter() const
		{
			return Api::Input::ADAPTER_NES;
		}

		System Image::GetDesiredSystem(Region region,CpuModel* cpu,PpuModel* ppu) const
		{
			if (region == REGION_NTSC)
			{
				if (cpu)
					*cpu = CPU_RP2A03;

				if (ppu)
					*ppu = PPU_RP2C02;

				return SYSTEM_NES_NTSC;
			}
			else
			{
				if (cpu)
					*cpu = CPU_RP2A07;

				if (ppu)
					*ppu = PPU_RP2C07;

				return SYSTEM_NES_PAL;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
