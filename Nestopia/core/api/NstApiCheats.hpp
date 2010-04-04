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

#ifndef NST_API_CHEATS_H
#define NST_API_CHEATS_H

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
	namespace Api
	{
		/**
		* Cheats interface.
		*/
		class Cheats : public Base
		{
			struct Lut;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Cheats(T& instance)
			: Base(instance) {}

			/**
			* Cheat code.
			*/
			struct Code
			{
				/**
				* Address.
				*/
				ushort address;
				/**
				* Value.
				*/
				uchar value;
				/**
				* Compare-value.
				*/
				uchar compare;
				/**
				* Compare-value enable.
				*/
				bool useCompare;

				/**
				* Constructor
				*
				* @param a address
				* @param v value
				* @param c compare-value
				* @param u compare-value enable
				*/
				Code(ushort a=0,uchar v=0,uchar c=0,bool u=false)
				: address(a), value(v), compare(c), useCompare(u) {}
			};

			/**
			* Adds a new code.
			*
			* @param code code, any existing code using the same address will be replaced
			* @return result code
			*/
			Result SetCode(const Code& code) throw();

			/**
			* Returns an existing code.
			*
			* @param index code index
			* @param code object to be filled
			* @return result code
			*/
			Result GetCode(ulong index,Code& code) const throw();

			/**
			* Returns attributes of an existing code.
			*
			* @param index code index
			* @param address address to be filled or NULL if not needed
			* @param value value to be filled or NULL if not needed
			* @param compare compare-value to be filled or NULL if not needed
			* @param useCompare compare-value enable to be filled or NULL if not needed
			* @return result code
			*/
			Result GetCode(ulong index,ushort* address,uchar* value,uchar* compare,bool* useCompare) const throw();

			/**
			* Removes an existing code.
			*
			* @param index code index
			* @return result code
			*/
			Result DeleteCode(ulong index) throw();

			/**
			* Returns current number of codes.
			*
			* @return number
			*/
			ulong NumCodes() const throw();

			/**
			* Removes all existing codes.
			*
			* @return result code
			*/
			Result ClearCodes() throw();

			enum
			{
				RAM_SIZE = 0x800
			};

			/**
			* CPU RAM pointer reference.
			*/
			typedef const uchar (&Ram)[RAM_SIZE];

			/**
			* Returns read-only content of CPU RAM.
			*
			* @return CPU RAM
			*/
			Ram GetRam() const throw();

			/**
			* Encodes into a Game Genie code.
			*
			* @param code code to be encoded
			* @param string Game Genie code string to be filled
			* @return result code
			*/
			static Result NST_CALL GameGenieEncode(const Code& code,char (&string)[9]) throw();

			/**
			* Decodes a Game Genie code.
			*
			* @param string Game Genie encoded string
			* @param code object to be filled
			* @return result code
			*/
			static Result NST_CALL GameGenieDecode(const char* string,Code& code) throw();

			/**
			* Encodes into a Pro-Action Rocky code.
			*
			* @param code code to be encoded
			* @param string Pro-Action Rocky code string to be filled
			* @return result code
			*/
			static Result NST_CALL ProActionRockyEncode(const Code& code,char (&string)[9]) throw();

			/**
			* Decodes a Pro-Action Rocky code.
			*
			* @param string Pro-Action Rocky encoded string
			* @param code object to be filled
			* @return result
			*/
			static Result NST_CALL ProActionRockyDecode(const char* string,Code& code) throw();
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
