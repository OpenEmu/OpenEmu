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

#ifndef NST_FPUPRECISION_H
#define NST_FPUPRECISION_H

#include <cfloat>

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class FpuPrecision
		{
		#if defined(NST_WIN32) && defined(_MCW_PC) && defined(_PC_24) && defined(_PC_53)

			const uint ctrl;

		public:

			FpuPrecision()
			: ctrl(::_controlfp( 0, 0 ) & _MCW_PC)
			{
				if (ctrl == _PC_24)
					::_controlfp( _PC_53, _MCW_PC );
			}

			~FpuPrecision()
			{
				if (ctrl == _PC_24)
					::_controlfp( _PC_24, _MCW_PC );
			}
		#endif
		};
	}
}

#endif
