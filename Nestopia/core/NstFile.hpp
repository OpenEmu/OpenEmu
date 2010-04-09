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

#ifndef NST_FILE_H
#define NST_FILE_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		template<typename T>
		class Vector;

		class File
		{
			struct Context;
			Context& context;

		public:

			File();
			~File();

			enum Type
			{
				BATTERY,
				EEPROM,
				TAPE,
				TURBOFILE,
				DISK
			};

			struct LoadBlock
			{
				byte* NST_RESTRICT data;
				dword size;
			};

			struct SaveBlock
			{
				const byte* NST_RESTRICT data;
				dword size;
			};

			void Load(const byte*,dword) const;
			void Load(byte*,dword,Type) const;
			void Load(Type,byte*,dword) const;
			void Load(Type,Vector<byte>&,dword) const;
			void Save(Type,const byte*,dword) const;

		private:

			void Load(Type,const LoadBlock*,uint,bool* = NULL) const;
			void Save(Type,const SaveBlock*,uint) const;

		public:

			template<uint N>
			void Load(Type type,const LoadBlock (&block)[N]) const
			{
				Load( type, block, N );
			}

			template<uint N>
			void Save(Type type,const SaveBlock (&block)[N]) const
			{
				Save( type, block, N );
			}
		};
	}
}

#endif
