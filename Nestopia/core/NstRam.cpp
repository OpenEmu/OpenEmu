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

#include <cstdlib>
#include <cstring>
#include "NstAssert.hpp"
#include "NstRam.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Ram::Ram()
		:
		mem      ( NULL  ),
		mask     ( 0     ),
		size     ( 0     ),
		type     ( RAM   ),
		readable ( false ),
		writable ( false ),
		internal ( false )
		{}

		Ram::Ram(Type t,bool r,bool w,dword s,byte* m)
		:
		mem      ( NULL  ),
		mask     ( 0     ),
		size     ( 0     ),
		type     ( t     ),
		readable ( r     ),
		writable ( w     ),
		internal ( false )
		{
			Set( s, m );
		}

		Ram::Ram(const Ram& ram)
		:
		mem      ( ram.mem      ),
		mask     ( ram.mask     ),
		size     ( ram.size     ),
		type     ( ram.type     ),
		readable ( ram.readable ),
		writable ( ram.writable ),
		internal ( false        ),
		pins     ( ram.pins     )
		{}

		Ram::~Ram()
		{
			if (internal)
				std::free( mem );
		}

		Ram& Ram::operator = (const Ram& ram)
		{
			if (this != &ram)
			{
				Destroy();

				mem      = ram.mem;
				mask     = ram.mask;
				size     = ram.size;
				type     = ram.type;
				readable = ram.readable;
				writable = ram.writable;
				internal = false;
				pins     = ram.pins;
			}

			return *this;
		}

		void Ram::Destroy()
		{
			pins.Clear();

			mask = 0;
			size = 0;

			if (byte* const tmp = mem)
			{
				mem = NULL;

				if (internal)
				{
					internal = false;
					std::free( tmp );
				}
			}
		}

		void Ram::Set(dword s,byte* m)
		{
			if (s)
			{
				dword prev = mask+1;

				mask = s - 1;
				mask |= mask >> 1;
				mask |= mask >> 2;
				mask |= mask >> 4;
				mask |= mask >> 8;
				mask |= mask >> 16;

				size = s;

				NST_VERIFY( s == mask+1 );

				if (m)
				{
					if (internal)
					{
						internal = false;
						std::free( mem );
					}
				}
				else
				{
					m = static_cast<byte*>(std::realloc( internal ? mem : NULL, mask+1 ));

					if (m)
					{
						if (!internal)
						{
							internal = true;
							prev = 0;
						}

						if (prev < mask+1)
							std::memset( m+prev, 0, mask+1-prev );
					}
					else
					{
						Destroy();
						throw RESULT_ERR_OUT_OF_MEMORY;
					}
				}

				mem = m;
			}
			else
			{
				Destroy();
			}
		}

		void Ram::Set(Type t,bool r,bool w,dword s,byte* m)
		{
			Set( s, m );
			type = t;
			readable = r;
			writable = w;
		}

		void Ram::Fill(uint value) const
		{
			NST_ASSERT( bool(mem) == bool(size) );
			NST_VERIFY( value <= 0xFF );

			std::memset( mem, value & 0xFF, size );
		}

		void Ram::Mirror(dword block)
		{
			NST_VERIFY( block );

			if (block)
			{
				const dword nearest = mask+1;

				if (internal || !size)
				{
					block--;
					block |= block >> 1;
					block |= block >> 2;
					block |= block >> 4;
					block |= block >> 8;
					block |= block >> 16;
					block++;

					if (mask+1 < block)
					{
						const dword tmp = size;
						Set( block );
						size = tmp;
					}
				}

				NST_ASSERT( nearest <= mask+1 && !((mask+1) & mask) && !(nearest & (nearest-1)) );

				if (size)
				{
					for (block=nearest; size % block; )
						block /= 2;

					for (dword i=size, n=size-block; i != nearest; i += block)
						std::memcpy( mem + i, mem + n, block );

					for (dword i=nearest, n=mask+1; i != n; i += nearest)
						std::memcpy( mem + i, mem, nearest );
				}
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
