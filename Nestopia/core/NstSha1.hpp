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

#ifndef NST_SHA1_H
#define NST_SHA1_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Sha1
		{
			class Key;

			void NST_CALL Compute(Key&,const byte*,dword);

			class Key : public ImplicitBool<Key>
			{
			public:

				Key();

				enum
				{
					DIGEST_SIZE = 20
				};

				typedef const dword (&Digest)[5];

				void Clear();
				Digest GetDigest() const;

				bool operator == (const Key& key) const;

			private:

				friend void NST_CALL Compute(Key&,const byte*,dword);

				inline void Update() const;
				void Compute(const byte*,dword);
				void Finalize() const;

				qword count;
				dword state[5];
				mutable ibool finalized;
				mutable dword final[5];
				byte buffer[64];

			public:

				bool operator != (const Key& key) const
				{
					return !(*this == key);
				}

				bool operator ! () const
				{
					return !count;
				}
			};
		}
	}
}

#endif
