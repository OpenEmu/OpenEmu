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

#include "NstCore.hpp"
#include "NstCrc32.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Crc32
		{
			static dword NST_CALL Iterate(uint data,dword crc)
			{
				struct Lut
				{
					dword data[256];

					Lut()
					{
						for (uint i=0; i < 256; ++i)
						{
							dword n = i;

							for (uint j=0; j < 8; ++j)
								n = (n >> 1) ^ (((~n & 1) - 1) & 0xEDB88320);

							data[i] = n;
						}
					}
				};

				static const Lut lut;

				return (crc >> 8) ^ lut.data[(crc ^ data) & 0xFF];
			}

			dword NST_CALL Compute(uint data,dword crc)
			{
				return Iterate( data, crc ^ 0xFFFFFFFF ) ^ 0xFFFFFFFF;
			}

			dword NST_CALL Compute(const byte* NST_RESTRICT data,const dword length,dword crc)
			{
				crc ^= 0xFFFFFFFF;

				for (const byte* const end=data+length; data != end; ++data)
					crc = Iterate( *data, crc );

				crc ^= 0xFFFFFFFF;

				return crc;
			}
		}
	}
}
