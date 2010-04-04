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

#include "NstState.hpp"
#include "NstMemory.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Memory<0,0,0>::SaveState
		(
			State::Saver& state,
			const dword baseChunk,
			const Ram* const NST_RESTRICT sources,
			const uint numSources,
			const byte* const NST_RESTRICT pages,
			const uint numPages
		)   const
		{
			NST_ASSERT( numSources >= 1 && numSources <= MAX_SOURCES && numPages );

			state.Begin( baseChunk );

			{
				byte data[MAX_SOURCES];

				for (uint i=0; i < numSources; ++i)
					data[i] = (sources[i].Readable() ? 0x1U : 0x0U) | (sources[i].Writable() ? 0x2U : 0x0U);

				state.Begin( AsciiId<'A','C','C'>::V ).Write( data, numSources ).End();
			}

			state.Begin( AsciiId<'B','N','K'>::V ).Write( pages, numPages * 3 ).End();

			state.End();
		}

		bool Memory<0,0,0>::LoadState
		(
			State::Loader& state,
			Ram* const NST_RESTRICT sources,
			const uint numSources,
			byte* const NST_RESTRICT pages,
			const uint numPages
		)   const
		{
			NST_ASSERT( numSources >= 1 && numSources <= MAX_SOURCES && numPages );

			bool paged = false;

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'A','C','C'>::V:
					{
						byte data[MAX_SOURCES];
						state.Read( data, numSources );

						for (uint i=0; i < numSources; ++i)
						{
							sources[i].ReadEnable( data[i] & 0x1U );

							NST_VERIFY( sources[i].GetType() != Ram::ROM || !(data[i] & 0x2U) );

							if (sources[i].GetType() != Ram::ROM)
								sources[i].WriteEnable( data[i] & 0x2U );
						}
						break;
					}

					case AsciiId<'B','N','K'>::V:

						paged = true;
						state.Read( pages, numPages * 3 );
						break;

					default:

						// deprecated

						for (uint i=0; i < numSources; ++i)
						{
							if (chunk == AsciiId<'R','M','0'>::R(0,0,i))
							{
								NST_DEBUG_MSG("Memory::LoadState() deprecated!");
								state.Uncompress( sources[i].Mem(), sources[i].Size() );
								break;
							}
						}
						break;
				}

				state.End();
			}

			return paged;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
