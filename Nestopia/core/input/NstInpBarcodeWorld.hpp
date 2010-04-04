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

#ifndef NST_INPUT_BARCODEWORLD_H
#define NST_INPUT_BARCODEWORLD_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "../NstBarcodeReader.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			class BarcodeWorld : public Device
			{
			public:

				explicit BarcodeWorld(const Cpu&);

			private:

				void Reset();
				void LoadState(State::Loader&,dword);
				void SaveState(State::Saver&,byte) const;
				uint Peek(uint);

				class Reader : public BarcodeReader
				{
				public:

					void Reset();
					uint Read();
					void LoadState(State::Loader&,dword);
					void SaveState(State::Saver&,byte) const;

				private:

					bool Transfer(cstring,uint);
					bool IsTransferring() const;
					bool IsDigitsSupported(uint) const;

					enum
					{
						NUM_DIGITS = 13,
						MAX_DATA_LENGTH = 0x100,
						END = 0xFF
					};

					const byte* stream;
					byte data[MAX_DATA_LENGTH];
				};

				Reader reader;

			public:

				BarcodeReader& GetReader()
				{
					return reader;
				}
			};
		}
	}
}

#endif
