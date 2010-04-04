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

#ifndef NST_CHECKSUM_H
#define NST_CHECKSUM_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstCore.hpp"
#include "NstSha1.hpp"

namespace Nes
{
	namespace Core
	{
		class Checksum : public ImplicitBool<Checksum>
		{
		public:

			Checksum();
			Checksum(const byte*,dword);

			void Clear();
			void Compute(const byte*,dword);
			void Recompute(const byte*,dword);

			bool operator == (const Checksum&) const;
			bool operator ! () const;

		private:

			Sha1::Key sha1;
			dword crc32;

		public:

			bool operator != (const Checksum& checksum) const
			{
				return !(*this == checksum);
			}

			Sha1::Key::Digest GetSha1() const
			{
				return sha1.GetDigest();
			}

			dword GetCrc() const
			{
				return crc32;
			}
		};
	}
}

#endif
