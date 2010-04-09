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

#include "NstCore.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterNone.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			template<typename T>
			void Renderer::FilterNone::BlitAligned(const Input& input,const Output& output)
			{
				const Input::Pixel* NST_RESTRICT src = input.pixels;
				T* NST_RESTRICT dst = static_cast<T*>(output.pixels);

				for (uint prefetched=*src++, i=PIXELS; i; --i)
				{
					const dword reg = input.palette[prefetched];
					prefetched = *src++;
					*dst++ = reg;
				}
			}

			template<typename T>
			void Renderer::FilterNone::BlitUnaligned(const Input& input,const Output& output)
			{
				const Input::Pixel* NST_RESTRICT src = input.pixels;
				T* NST_RESTRICT dst = static_cast<T*>(output.pixels);

				const long pad = output.pitch - WIDTH * sizeof(T);

				for (uint prefetched=*src++, y=HEIGHT; y; --y)
				{
					for (uint x=WIDTH; x; --x)
					{
						const dword reg = input.palette[prefetched];
						prefetched = *src++;
						*dst++ = reg;
					}

					dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst) + pad);
				}
			}

			void Renderer::FilterNone::Blit(const Input& input,const Output& output,uint)
			{
				if (format.bpp == 32)
				{
					if (output.pitch == WIDTH * sizeof(dword))
						BlitAligned<dword>( input, output );
					else
						BlitUnaligned<dword>( input, output );
				}
				else
				{
					if (output.pitch == WIDTH * sizeof(word))
						BlitAligned<word>( input, output );
					else
						BlitUnaligned<word>( input, output );
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Renderer::FilterNone::FilterNone(const RenderState& state)
			: Filter(state)
			{
				NST_COMPILE_ASSERT( Video::Screen::PIXELS_PADDING >= 1 );
			}

			bool Renderer::FilterNone::Check(const RenderState& state)
			{
				return
				(
					(state.bits.count == 16 || state.bits.count == 32) &&
					(state.width == WIDTH && state.height == HEIGHT)
				);
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}
