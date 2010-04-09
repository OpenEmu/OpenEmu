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
#include "NstInpTurboFile.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			TurboFile::TurboFile(const Cpu& cpu)
			: Device(cpu,Api::Input::TURBOFILE)
			{
				std::memset( ram, 0, SIZE );
				file.Load( File::TURBOFILE, ram, SIZE );
			}

			TurboFile::~TurboFile()
			{
				file.Save( File::TURBOFILE, ram, SIZE );
			}

			void TurboFile::Reset()
			{
				pos = 0x00;
				bit = 0x01;
				old = 0x00;
				out = 0x00;
			}

			void TurboFile::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'T','F'>::R(0,0,id) );

				uint count;
				for (count=0; bit && bit != (1U << count); ++count);

				const byte data[3] =
				{
					pos & 0xFF,
					pos >> 8,
					count | (old << 1) | (out << 2)
				};

				saver.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				saver.Begin( AsciiId<'R','A','M'>::V ).Compress( ram ).End();

				saver.End();
			}

			void TurboFile::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'T','F'>::V)
				{
					while (const dword chunk = loader.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:
							{
								State::Loader::Data<3> data( loader );

								pos = data[0] | (data[1] << 8 & 0x1F00);
								bit = 1U << (data[2] & 0x7);
								old = data[2] >> 1 & WRITE_BIT;
								out = data[2] >> 2 & READ_BIT;

								break;
							}

							case AsciiId<'R','A','M'>::V:

								loader.Uncompress( ram );
								break;
						}

						loader.End();
					}
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void TurboFile::Poke(uint data)
			{
				if (!(data & NO_RESET))
				{
					pos = 0x00;
					bit = 0x01;
				}

				const uint advance = old;
				old = data & WRITE_ENABLE;

				if (old)
				{
					ram[pos] = (ram[pos] & ~bit) | (bit * (data & WRITE_BIT));
				}
				else if (advance)
				{
					if (bit != 0x80)
					{
						bit <<= 1;
					}
					else
					{
						bit = 0x01;
						pos = (pos + 1) & (SIZE-1);
					}
				}

				out = (ram[pos] & bit) ? READ_BIT : 0;
			}

			uint TurboFile::Peek(uint port)
			{
				return port ? out : 0;
			}
		}
	}
}
