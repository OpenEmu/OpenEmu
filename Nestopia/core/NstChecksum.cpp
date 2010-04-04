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

#include "NstAssert.hpp"
#include "NstChecksum.hpp"
#include "NstCrc32.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Checksum::Checksum()
		: crc32(0) {}

		Checksum::Checksum(const byte* data,dword size)
		: crc32(0)
		{
			Compute( data, size );
		}

		void Checksum::Clear()
		{
			crc32 = 0;
			sha1.Clear();
		}

		void Checksum::Compute(const byte* data,dword size)
		{
			crc32 = Crc32::Compute( data, size, crc32 );
			Sha1::Compute( sha1, data, size );
		}

		void Checksum::Recompute(const byte* data,dword size)
		{
			Clear();
			Compute( data, size );
		}

		bool Checksum::operator == (const Checksum& checksum) const
		{
			NST_VERIFY( (crc32 == checksum.crc32) == (sha1 == checksum.sha1) );
			return crc32 == checksum.crc32 && sha1 == checksum.sha1;
		}

		bool Checksum::operator ! () const
		{
			NST_VERIFY( !crc32 == !sha1 );
			return !crc32 && !sha1;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
