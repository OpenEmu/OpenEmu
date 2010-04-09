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
#include <new>
#include "NstVector.hpp"

namespace Nes
{
	namespace Core
	{
		void* Vector<void>::Malloc(dword size)
		{
			NST_ASSERT( size );

			if (void* mem = std::malloc( size ))
				return mem;
			else
				throw std::bad_alloc();
		}

		void* Vector<void>::Realloc(void* mem,dword size)
		{
			NST_ASSERT( size );

			if (NULL != (mem = std::realloc( mem, size )))
				return mem;
			else
				throw std::bad_alloc();
		}

		void Vector<void>::Free(void* mem)
		{
			std::free( mem );
		}

		void Vector<void>::Copy(void* dst,const void* src,dword size)
		{
			std::memcpy( dst, src, size );
		}

		void Vector<void>::Move(void* dst,const void* src,dword size)
		{
			std::memmove( dst, src, size );
		}
	}
}
