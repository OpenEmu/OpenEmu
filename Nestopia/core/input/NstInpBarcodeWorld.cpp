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

#include <cstring>
#include "NstInpDevice.hpp"
#include "NstInpBarcodeWorld.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			BarcodeWorld::BarcodeWorld(const Cpu& cpu)
			: Device(cpu,Api::Input::BARCODEWORLD)
			{
				BarcodeWorld::Reset();
			}

			void BarcodeWorld::Reset()
			{
				reader.Reset();
			}

			void BarcodeWorld::LoadState(State::Loader& loader,const dword id)
			{
				reader.LoadState( loader, id );
			}

			void BarcodeWorld::SaveState(State::Saver& saver,const byte id) const
			{
				reader.SaveState( saver, id );
			}

			void BarcodeWorld::Reader::Reset()
			{
				stream = data;
				std::memset( data, END, MAX_DATA_LENGTH );
			}

			bool BarcodeWorld::Reader::IsDigitsSupported(uint count) const
			{
				return count == NUM_DIGITS;
			}

			bool BarcodeWorld::Reader::IsTransferring() const
			{
				return *stream != END;
			}

			void BarcodeWorld::Reader::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'B','W'>::V)
				{
					Reset();

					while (const dword chunk = loader.Begin())
					{
						switch (chunk)
						{
						case AsciiId<'P','T','R'>::V:

							stream = data + (loader.Read8() & (MAX_DATA_LENGTH-1));
							break;

						case AsciiId<'D','A','T'>::V:

							loader.Uncompress( data );
							data[MAX_DATA_LENGTH-1] = END;
							break;
						}

						loader.End();
					}
				}
			}

			void BarcodeWorld::Reader::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'B','W'>::R(0,0,id) );

				if (Reader::IsTransferring())
				{
					saver.Begin( AsciiId<'P','T','R'>::V ).Write8( stream - data ).End();
					saver.Begin( AsciiId<'D','A','T'>::V ).Compress( data ).End();
				}

				saver.End();
			}

			bool BarcodeWorld::Reader::Transfer(cstring const string,const uint length)
			{
				NST_COMPILE_ASSERT( MAX_DATA_LENGTH >= 191 );

				Reset();

				if (!string || length != NUM_DIGITS)
					return false;

				byte code[NUM_DIGITS+7];

				for (uint i=0; i < NUM_DIGITS; ++i)
				{
					const int c = string[i];

					if (c >= '0' && c <= '9')
						code[i] = c - '0' + Ascii<'0'>::V;
					else
						return false;
				}

				code[NUM_DIGITS+0] = Ascii<'S'>::V;
				code[NUM_DIGITS+1] = Ascii<'U'>::V;
				code[NUM_DIGITS+2] = Ascii<'N'>::V;
				code[NUM_DIGITS+3] = Ascii<'S'>::V;
				code[NUM_DIGITS+4] = Ascii<'O'>::V;
				code[NUM_DIGITS+5] = Ascii<'F'>::V;
				code[NUM_DIGITS+6] = Ascii<'T'>::V;

				byte* NST_RESTRICT output = data;

				*output++ = 0x04;

				for (uint i=0; i < NUM_DIGITS+7; ++i)
				{
					*output++ = 0x04;

					for (uint j=0x01, c=code[i]; j != 0x100; j <<= 1)
						*output++ = (c & j) ? 0x00 : 0x04;

					*output++ = 0x00;
				}

				return true;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint BarcodeWorld::Reader::Read()
			{
				if (Reader::IsTransferring())
				{
					uint next = *stream;
					stream += (next != END);
					return next;
				}
				else
				{
					return 0;
				}
			}

			uint BarcodeWorld::Peek(uint port)
			{
				return port == 1 ? reader.Read() : 0;
			}
		}
	}
}
