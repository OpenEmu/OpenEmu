////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2001, 2002, 2003, 2004 Andrea Mazzoleni
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

#ifndef NST_NO_SCALEX

#include "NstVideoRenderer.hpp"
#include "NstVideoFilterScaleX.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			void Renderer::FilterScaleX::Blit(const Input& input,const Output& output,uint)
			{
				path( input, output );
			}

			template<typename T,int PREV,int NEXT>
			NST_FORCE_INLINE T* Renderer::FilterScaleX::Blit2xBorder(T* NST_RESTRICT dst,const Input::Pixel* NST_RESTRICT src,const Input::Palette& palette)
			{
				{
					dword p[4] =
					{
						palette[src[ PREV ]],
						palette[src[ 0    ]],
						palette[src[ 1    ]],
						palette[src[ NEXT ]]
					};

					if (p[0] != p[3] && p[2] != p[1] && p[2] == p[0])
						p[1] = p[0];

					dst[0] = p[1];
					dst[1] = p[1];
				}

				src += 1;
				dst += 2;

				for (uint x=WIDTH-2; x; --x)
				{
					const dword p[4] =
					{
						palette[src[  PREV ]],
						palette[src[ -1    ]],
						palette[src[  0    ]],
						palette[src[  1    ]]
					};

					if (p[0] != p[1] && p[1] != p[3])
					{
						dst[0] = p[1] == p[0] ? p[0] : p[2];
						dst[1] = p[3] == p[0] ? p[0] : p[2];
					}
					else
					{
						dst[0] = p[2];
						dst[1] = p[2];
					}

					src += 1;
					dst += 2;
				}

				const dword p[4] =
				{
					palette[src[  PREV ]],
					palette[src[ -1    ]],
					palette[src[  0    ]],
					palette[src[  NEXT ]]
				};

				if (p[0] != p[3] && p[1] != p[2])
				{
					dst[0] = p[1] == p[0] ? p[0] : p[2];
					dst[1] = p[2] == p[0] ? p[0] : p[2];
				}
				else
				{
					dst[0] = p[2];
					dst[1] = p[2];
				}

				return dst + 2;
			}

			template<typename T,int PREV,int NEXT>
			NST_FORCE_INLINE T* Renderer::FilterScaleX::Blit3xBorder(T* NST_RESTRICT dst,const Input::Pixel* NST_RESTRICT src,const Input::Palette& palette)
			{
				{
					const dword p = palette[src[0]];

					dst[0] = p;
					dst[1] = p;

					const dword q = palette[src[PREV]];

					dst[2] = (q != palette[src[1]] && q != palette[src[NEXT]]) ? q : p;
				}

				src += 1;
				dst += 3;

				for (uint x=WIDTH-2; x; --x)
				{
					const dword p[5] =
					{
						palette[src[  PREV ]],
						palette[src[ -1    ]],
						palette[src[  0    ]],
						palette[src[  1    ]],
						palette[src[  NEXT ]]
					};

					dst[0] = (p[1] == p[0] && p[4] != p[0] && p[3] != p[0]) ? p[0] : p[2];
					dst[1] = p[2];
					dst[2] = (p[3] == p[0] && p[4] != p[0] && p[1] != p[0]) ? p[0] : p[2];

					src += 1;
					dst += 3;
				}

				const dword p[2] =
				{
					palette[src[ PREV ]],
					palette[src[ 0    ]]
				};

				dst[0] = p[p[0] != palette[src[-1]] || p[0] == palette[src[NEXT]]];
				dst[1] = p[1];
				dst[2] = p[1];

				return dst + 3;
			}

			template<typename T>
			NST_FORCE_INLINE T* Renderer::FilterScaleX::Blit3xCenter(T* NST_RESTRICT dst,const Input::Pixel* NST_RESTRICT src,const Input::Palette& palette)
			{
				for (uint x=WIDTH; x; --x)
				{
					const dword p = palette[*src++];

					dst[0] = p;
					dst[1] = p;
					dst[2] = p;

					dst += 3;
				}

				return dst;
			}

			template<typename T,int PREV,int NEXT>
			NST_FORCE_INLINE T* Renderer::FilterScaleX::Blit2xLine(T* dst,const Input::Pixel* const src,const Input::Palette& palette,const long pad)
			{
				dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(Blit2xBorder<T,PREV,NEXT>( dst, src, palette )) + pad);
				dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(Blit2xBorder<T,NEXT,PREV>( dst, src, palette )) + pad);

				return dst;
			}

			template<typename T,int PREV,int NEXT>
			NST_FORCE_INLINE T* Renderer::FilterScaleX::Blit3xLine(T* dst,const Input::Pixel* const src,const Input::Palette& palette,const long pad)
			{
				dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(Blit3xBorder<T,PREV,NEXT>( dst, src, palette )) + pad);
				dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(Blit3xCenter<T>( dst, src, palette )) + pad);
				dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(Blit3xBorder<T,NEXT,PREV>( dst, src, palette )) + pad);

				return dst;
			}

			template<typename T>
			void Renderer::FilterScaleX::Blit2x(const Input& input,const Output& output)
			{
				const Input::Pixel* src = input.pixels;
				T* dst = static_cast<T*>(output.pixels);
				const long pad = output.pitch - long(sizeof(T) * WIDTH*2);

				dst = Blit2xLine<T,0,WIDTH>( dst, src, input.palette, pad );

				for (uint y=HEIGHT-2; y; --y)
					dst = Blit2xLine<T,-WIDTH,WIDTH>( dst, src += WIDTH, input.palette, pad );

				Blit2xLine<T,-WIDTH,0>( dst, src + WIDTH, input.palette, pad );
			}

			template<typename T>
			void Renderer::FilterScaleX::Blit3x(const Input& input,const Output& output)
			{
				const Input::Pixel* src = input.pixels;
				T* dst = static_cast<T*>(output.pixels);
				const long pad = output.pitch - long(sizeof(T) * WIDTH*3);

				dst = Blit3xLine<T,0,WIDTH>( dst, src, input.palette, pad );

				for (uint y=HEIGHT-2; y; --y)
					dst = Blit3xLine<T,-WIDTH,WIDTH>( dst, src += WIDTH, input.palette, pad );

				Blit3xLine<T,-WIDTH,0>( dst, src + WIDTH, input.palette, pad );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Renderer::FilterScaleX::Path Renderer::FilterScaleX::GetPath(const RenderState& state)
			{
				if (state.filter == RenderState::FILTER_SCALE2X)
				{
					if (state.bits.count == 32)
						return &FilterScaleX::Blit2x<dword>;
					else
						return &FilterScaleX::Blit2x<word>;
				}
				else
				{
					if (state.bits.count == 32)
						return &FilterScaleX::Blit3x<dword>;
					else
						return &FilterScaleX::Blit3x<word>;
				}
			}

			Renderer::FilterScaleX::FilterScaleX(const RenderState& state)
			:
			Filter (state),
			path   (GetPath(state))
			{
			}

			bool Renderer::FilterScaleX::Check(const RenderState& state)
			{
				return (state.bits.count == 16 || state.bits.count == 32) &&
				(
					(state.filter == RenderState::FILTER_SCALE2X && state.width == WIDTH*2 && state.height == HEIGHT*2) ||
					(state.filter == RenderState::FILTER_SCALE3X && state.width == WIDTH*3 && state.height == HEIGHT*3)
				);
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}

#endif

