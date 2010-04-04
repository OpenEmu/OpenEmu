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

#include "NstCpu.hpp"
#include "NstSoundPcm.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Pcm::Pcm(Apu& a)
			: Channel(a)
			{
				Connect( UpdateSettings() );
			}

			bool Pcm::CanDo(uint bits,dword rate)
			{
				return (bits == 8 || bits == 16) && (rate >= 8000 && rate <= 96000);
			}

			void Pcm::Reset()
			{
				Stop();
			}

			bool Pcm::UpdateSettings()
			{
				Stop();

				rate = GetSampleRate();

				return true;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Pcm::Play(const iword* w,dword l,dword r)
			{
				NST_ASSERT( w && l && CanDo(16,r) );

				pos = 0;

				wave.data = w;
				wave.length = l;
				wave.rate = r;
			}

			void Pcm::Stop()
			{
				pos = 0;

				wave.data = NULL;
				wave.length = 0;
				wave.rate = 0;
			}

			Pcm::Sample Pcm::GetSample()
			{
				if (wave.data)
				{
					const dword i = pos / rate;

					if (i < wave.length)
					{
						pos += wave.rate;
						return wave.data[i];
					}

					wave.data = NULL;
				}

				return 0;
			}
		}
	}
}
