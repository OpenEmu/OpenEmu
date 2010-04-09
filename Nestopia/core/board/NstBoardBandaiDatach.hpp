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

#ifndef NST_BOARD_BANDAI_DATACH_H
#define NST_BOARD_BANDAI_DATACH_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "../NstBarcodeReader.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				class Datach : public Lz93d50Ex
				{
				public:

					explicit Datach(const Context&);

				private:

					class Reader : public BarcodeReader
					{
					public:

						explicit Reader(Cpu&);

						void Reset(bool=true);
						void SaveState(State::Saver&,dword) const;
						void LoadState(State::Loader&);

						inline void Sync();
						inline uint GetOutput() const;

					private:

						enum
						{
							MIN_DIGITS = 8,
							MAX_DIGITS = 13,
							MAX_DATA_LENGTH = 0x100,
							END = 0xFF,
							CC_INTERVAL = 1000
						};

						bool Transfer(cstring,uint);
						bool IsTransferring() const;
						bool IsDigitsSupported(uint) const;

						NES_DECL_HOOK( Fetcher );

						Cpu& cpu;
						Cycle cycles;
						uint output;
						const byte* stream;
						byte data[MAX_DATA_LENGTH];
					};

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Sync(Event,Input::Controllers*);
					Device QueryDevice(DeviceType);

					NES_DECL_PEEK( 6000 );

					Reader barcodeReader;
					Io::Port p6000;
				};
			}
		}
	}
}

#endif
