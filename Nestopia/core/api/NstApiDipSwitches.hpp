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

#ifndef NST_API_DIPSWITCHES_H
#define NST_API_DIPSWITCHES_H

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		class DipSwitches;
	}

	namespace Api
	{
		/**
		* DIP switches interface.
		*/
		class DipSwitches : public Base
		{
			Core::DipSwitches* Query() const;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			DipSwitches(T& instance)
			: Base(instance) {}

			enum
			{
				INVALID = -1
			};

			/**
			* Returns the number of available DIP switches.
			*
			* @return number
			*/
			uint NumDips() const throw();

			/**
			* Returns the number of values that can be chosen for a DIP switch.
			*
			* @param dip DIP switch ID
			* @return number
			*/
			uint NumValues(uint dip) const throw();

			/**
			* Checks if the DIP switches can be changed at this time.
			*
			* @return true if DIP switches can be changed
			*/
			bool CanModify() const throw();

			/**
			* Returns the name of a DIP switch.
			*
			* @param dip DIP switch ID
			* @return DIP switch name or NULL if unavailable
			*/
			const char* GetDipName(uint dip) const throw();

			/**
			* Returns the name of a DIP switch value.
			*
			* @param dip DIP switch ID
			* @param value value ID
			* @return value name or NULL if unavailable
			*/
			const char* GetValueName(uint dip,uint value) const throw();

			/**
			* Returns the current DIP switch value.
			*
			* @param dip DIP switch ID
			* @return value ID or INVALID if unavailable
			*/
			int GetValue(uint dip) const throw();

			/**
			* Sets a DIP switch value.
			*
			* @param dip DIP switch ID
			* @param value value ID
			* @return result code
			*/
			Result SetValue(uint dip,uint value) throw();
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
