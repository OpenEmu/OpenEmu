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

#ifndef NST_API_BARCODEREADER_H
#define NST_API_BARCODEREADER_H

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
		class BarcodeReader;
	}

	namespace Api
	{
		/**
		* Bar code reader interface.
		*/
		class BarcodeReader : public Base
		{
			Core::BarcodeReader* Query() const;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			BarcodeReader(T& instance)
			: Base(instance) {}

			enum
			{
				MIN_DIGITS = 8,
				MAX_DIGITS = 13
			};

			/**
			* Checks if the number of bar code digits is supported.
			*
			* @param length number of digits in the range 8 to 13
			* @return true if supported
			*/
			bool IsDigitsSupported(uint length) const throw();

			/**
			* Checks if the reader is ready to scan.
			*
			* @return true if ready
			*/
			bool CanTransfer() const throw();

			/**
			* Generates a randomized bar code.
			*
			* @param string string to be filled
			* @return length of randomized bar code, 0 if reader is disconnected
			*/
			uint Randomize(char (&string)[MAX_DIGITS+1]) const throw();

			/**
			* Transfers a bar code to the reader.
			*
			* @param string bar code string
			* @param length string length
			* @return result code
			*/
			Result Transfer(const char* string,uint length) throw();

			/**
			* Checks if a reader is connected.
			*
			* @return true if connected
			*/
			bool IsConnected() const
			{
				return Query();
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
