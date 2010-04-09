////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
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

#ifndef NST_NO_HQ2X

#include "NstAssert.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterHqX.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			void Renderer::FilterHqX::Blit(const Input& input,const Output& output,uint)
			{
				(*this.*path)( input, output );
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate1(dword c1,dword c2)
			{
				return ((((c1 & G)*3 + (c2 & G)) & (G << 2)) + (((c1 & (R|B))*3 + (c2 & (R|B))) & ((R|B) << 2))) >> 2;
			}

			template<>
			inline dword Renderer::FilterHqX::Interpolate1<0xFF0000,0x00FF00,0x0000FF>(dword c1,dword c2)
			{
				return (c1 * 3 + c2) >> 2;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate2(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + (c2 & G) + (c3 & G)) & (G << 2)) + (((c1 & (R|B))*2 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 2))) >> 2;
			}

			template<>
			inline dword Renderer::FilterHqX::Interpolate2<0xFF0000,0x00FF00,0x0000FF>(dword c1,dword c2,dword c3)
			{
				return (c1 * 2 + c2 + c3) >> 2;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate3(dword c1,dword c2)
			{
				return ((((c1 & G)*7 + (c2 & G)) & (G << 3)) + (((c1 & (R|B))*7 + (c2 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate4(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + ((c2 & G) + (c3 & G))*7) & (G << 4)) + (((c1 & (R|B))*2 + ((c2 & (R|B)) + (c3 & (R|B)))*7) & ((R|B) << 4))) >> 4;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate5(dword c1,dword c2)
			{
				return (((c1 & G) + (c2 & G) & (G << 1)) + ((c1 & (R|B)) + (c2 & (R|B)) & ((R|B) << 1))) >> 1;
			}

			template<>
			inline dword Renderer::FilterHqX::Interpolate5<0xFF0000,0x00FF00,0x0000FF>(dword c1,dword c2)
			{
				return (c1 + c2) >> 1;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate6(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*5 + (c2 & G)*2 + (c3 & G)) & (G << 3)) + (((c1 & (R|B))*5 + (c2 & (R|B))*2 + (c3 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate7(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*6 + (c2 & G) + (c3 & G)) & (G << 3)) + (((c1 & (R|B))*6 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate8(dword c1,dword c2)
			{
				return ((((c1 & G)*5 + (c2 & G)*3) & (G << 3)) + (((c1 & (R|B))*5 + (c2 & (R|B))*3) & ((R|B) << 3))) >> 3;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate9(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + ((c2 & G) + (c3 & G))*3 ) & (G << 3)) + (((c1 & (R|B))*2 + ((c2 & (R|B)) + (c3 & (R|B)))*3 ) & ((R|B) << 3))) >> 3;
			}

			template<dword R,dword G,dword B>
			dword Renderer::FilterHqX::Interpolate10(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*14 + (c2 & G) + (c3 & G)) & (G << 4)) + (((c1 & (R|B))*14 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 4))) >> 4;
			}

			inline dword Renderer::FilterHqX::Diff(uint w1,uint w2) const
			{
				return (lut.yuv[w1] - lut.yuv[w2] + Lut::YUV_OFFSET) & Lut::YUV_MASK;
			}

			template<typename T>
			struct Renderer::FilterHqX::Buffer
			{
				uint w[10];
				dword c[10];

				NST_FORCE_INLINE void Convert(const Lut& lut)
				{
					for (uint k=0; k < 9; ++k)
						c[k] = lut.rgb[w[k]];
				}
			};

			template<>
			struct Renderer::FilterHqX::Buffer<word>
			{
				union
				{
					uint w[10];
					dword c[10];
				};

				void Convert(const Lut&)
				{
				}
			};

			template<typename T,dword R,dword G,dword B>
			void Renderer::FilterHqX::Blit2x(const Input& input,const Output& output) const
			{
				const byte* NST_RESTRICT src = reinterpret_cast<const byte*>(input.pixels);
				const long pitch = output.pitch + output.pitch - (WIDTH*2 * sizeof(T));

				T* NST_RESTRICT dst[2] =
				{
					static_cast<T*>(output.pixels) - 2,
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch) - 2
				};

				for (uint y=HEIGHT; y; --y)
				{
					const uint lines[2] =
					{
						y < HEIGHT ? WIDTH * sizeof(Input::Pixel) : 0,
						y > 1      ? WIDTH * sizeof(Input::Pixel) : 0
					};

					Buffer<T> b;

					b.w[2] = (b.w[1] = input.palette[*reinterpret_cast<const Input::Pixel*>(src - lines[0])]);
					b.w[5] = (b.w[4] = input.palette[*reinterpret_cast<const Input::Pixel*>(src)]);
					b.w[8] = (b.w[7] = input.palette[*reinterpret_cast<const Input::Pixel*>(src + lines[1])]);

					for (uint x=WIDTH; x; )
					{
						src += sizeof(Input::Pixel);
						dst[0] += 2;
						dst[1] += 2;

						b.w[0] = b.w[1];
						b.w[1] = b.w[2];
						b.w[3] = b.w[4];
						b.w[4] = b.w[5];
						b.w[6] = b.w[7];
						b.w[7] = b.w[8];

						if (--x)
						{
							b.w[2] = input.palette[*reinterpret_cast<const Input::Pixel*>(src - lines[0])];
							b.w[5] = input.palette[*reinterpret_cast<const Input::Pixel*>(src)];
							b.w[8] = input.palette[*reinterpret_cast<const Input::Pixel*>(src + lines[1])];
						}

						b.Convert( lut );

						const uint yuv5 = lut.yuv[b.w[4]];

						#include "NstVideoFilterHq2x.inl"
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[0]) + pitch);
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[1]) + pitch);
				}
			}

			template<typename T,dword R,dword G,dword B>
			void Renderer::FilterHqX::Blit3x(const Input& input,const Output& output) const
			{
				const byte* NST_RESTRICT src = reinterpret_cast<const byte*>(input.pixels);
				const long pitch = (output.pitch * 2) + output.pitch - (WIDTH*3 * sizeof(T));

				T* NST_RESTRICT dst[3] =
				{
					static_cast<T*>(output.pixels) - 3,
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch) - 3,
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch * 2) - 3
				};

				for (uint y=HEIGHT; y; --y)
				{
					const uint lines[2] =
					{
						y < HEIGHT ? WIDTH * sizeof(Input::Pixel) : 0,
						y > 1      ? WIDTH * sizeof(Input::Pixel) : 0
					};

					Buffer<T> b;

					b.w[2] = (b.w[1] = input.palette[*reinterpret_cast<const Input::Pixel*>(src - lines[0])]);
					b.w[5] = (b.w[4] = input.palette[*reinterpret_cast<const Input::Pixel*>(src)]);
					b.w[8] = (b.w[7] = input.palette[*reinterpret_cast<const Input::Pixel*>(src + lines[1])]);

					for (uint x=WIDTH; x; )
					{
						src += sizeof(Input::Pixel);
						dst[0] += 3;
						dst[1] += 3;
						dst[2] += 3;

						b.w[0] = b.w[1];
						b.w[1] = b.w[2];
						b.w[3] = b.w[4];
						b.w[4] = b.w[5];
						b.w[6] = b.w[7];
						b.w[7] = b.w[8];

						if (--x)
						{
							b.w[2] = input.palette[*reinterpret_cast<const Input::Pixel*>(src - lines[0])];
							b.w[5] = input.palette[*reinterpret_cast<const Input::Pixel*>(src)];
							b.w[8] = input.palette[*reinterpret_cast<const Input::Pixel*>(src + lines[1])];
						}

						b.Convert( lut );

						const uint yuv5 = lut.yuv[b.w[4]];

						#include "NstVideoFilterHq3x.inl"
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[0]) + pitch);
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[1]) + pitch);
					dst[2] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[2]) + pitch);
				}
			}

			template<typename T,dword R,dword G,dword B>
			void Renderer::FilterHqX::Blit4x(const Input& input,const Output& output) const
			{
				const byte* NST_RESTRICT src = reinterpret_cast<const byte*>(input.pixels);
				const long pitch = (output.pitch * 3) + output.pitch - (WIDTH*4 * sizeof(T));

				T* NST_RESTRICT dst[4] =
				{
					static_cast<T*>(output.pixels) - 4,
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch) - 4,
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch * 2) - 4,
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch * 3) - 4
				};

				for (uint y=HEIGHT; y; --y)
				{
					const uint lines[2] =
					{
						y < HEIGHT ? WIDTH * sizeof(Input::Pixel) : 0,
						y > 1      ? WIDTH * sizeof(Input::Pixel) : 0
					};

					Buffer<T> b;

					b.w[2] = (b.w[1] = input.palette[*reinterpret_cast<const Input::Pixel*>(src - lines[0])]);
					b.w[5] = (b.w[4] = input.palette[*reinterpret_cast<const Input::Pixel*>(src)]);
					b.w[8] = (b.w[7] = input.palette[*reinterpret_cast<const Input::Pixel*>(src + lines[1])]);

					for (uint x=WIDTH; x; )
					{
						src += sizeof(Input::Pixel);
						dst[0] += 4;
						dst[1] += 4;
						dst[2] += 4;
						dst[3] += 4;

						b.w[0] = b.w[1];
						b.w[1] = b.w[2];
						b.w[3] = b.w[4];
						b.w[4] = b.w[5];
						b.w[6] = b.w[7];
						b.w[7] = b.w[8];

						if (--x)
						{
							b.w[2] = input.palette[*reinterpret_cast<const Input::Pixel*>(src - lines[0])];
							b.w[5] = input.palette[*reinterpret_cast<const Input::Pixel*>(src)];
							b.w[8] = input.palette[*reinterpret_cast<const Input::Pixel*>(src + lines[1])];
						}

						b.Convert( lut );

						const uint yuv5 = lut.yuv[b.w[4]];

						#include "NstVideoFilterHq4x.inl"
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[0]) + pitch);
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[1]) + pitch);
					dst[2] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[2]) + pitch);
					dst[3] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[3]) + pitch);
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Renderer::FilterHqX::Lut::Lut(const bool bpp32,const byte (&formatShifts)[3],dword* tmp)
			: rgb(tmp = (bpp32 ? new dword [0x10000] : NULL))
			{
				const uint shifts[3] =
				{
					bpp32 ? 11 : formatShifts[0],
					bpp32 ?  5 : formatShifts[1],
					bpp32 ?  0 : formatShifts[2]
				};

				for (uint i=0; i < 32; ++i)
				{
					for (uint j=0; j < 64; ++j)
					{
						for (uint k=0; k < 32; ++k)
						{
							uint r = i << 3;
							uint g = j << 2;
							uint b = k << 3;

							dword y = ((r + g + b) >> 2) & 0xFF;
							dword u = (128 + ((r - b) >> 2)) & 0xFF;
							dword v = (128 + ((2*g - r - b) >> 3)) & 0xFF;

							yuv[(i << shifts[0]) | (j << shifts[1]) | (k << shifts[2])] = (y << 16) | (u << 8) | (v << 0);
						}
					}
				}

				if (bpp32)
				{
					for (dword i=0; i < 0x10000; ++i)
						tmp[i] = ((i & 0xF800) << 8) | ((i & 0x07E0) << 5) | ((i & 0x001F) << 3);
				}
			}

			Renderer::FilterHqX::Lut::~Lut()
			{
				delete [] rgb;
			}

			Renderer::FilterHqX::Path Renderer::FilterHqX::GetPath(const RenderState& state)
			{
				if (state.filter == RenderState::FILTER_HQ2X)
				{
					if (state.bits.count == 32)
					{
						return &FilterHqX::Blit2x<dword,0xFF0000,0x00FF00,0x0000FF>;
					}
					else if (state.bits.mask.g == 0x07E0)
					{
						return &FilterHqX::Blit2x<word,0xF800,0x07E0,0x001F>;
					}
					else
					{
						return &FilterHqX::Blit2x<word,0x7C00,0x03E0,0x001F>;
					}
				}
				else if (state.filter == RenderState::FILTER_HQ3X)
				{
					if (state.bits.count == 32)
					{
						return &FilterHqX::Blit3x<dword,0xFF0000,0x00FF00,0x0000FF>;
					}
					else if (state.bits.mask.g == 0x07E0)
					{
						return &FilterHqX::Blit3x<word,0xF800,0x07E0,0x001F>;
					}
					else
					{
						return &FilterHqX::Blit3x<word,0x7C00,0x03E0,0x001F>;
					}
				}
				else
				{
					if (state.bits.count == 32)
					{
						return &FilterHqX::Blit4x<dword,0xFF0000,0x00FF00,0x0000FF>;
					}
					else if (state.bits.mask.g == 0x07E0)
					{
						return &FilterHqX::Blit4x<word,0xF800,0x07E0,0x001F>;
					}
					else
					{
						return &FilterHqX::Blit4x<word,0x7C00,0x03E0,0x001F>;
					}
				}
			}

			Renderer::FilterHqX::FilterHqX(const RenderState& state)
			:
			Filter (state),
			path   (GetPath(state)),
			lut    (state.bits.count == 32,format.shifts)
			{
			}

			bool Renderer::FilterHqX::Check(const RenderState& state)
			{
				return
				(
					(state.bits.count == 16 && state.bits.mask.b == 0x001F && ((state.bits.mask.g == 0x07E0 && state.bits.mask.r == 0xF800) || (state.bits.mask.g == 0x03E0 && state.bits.mask.r == 0x7C00))) ||
					(state.bits.count == 32 && state.bits.mask.r == 0xFF0000 && state.bits.mask.g == 0x00FF00 && state.bits.mask.b == 0x0000FF)
				)
				&&
				(
					(state.filter == RenderState::FILTER_HQ2X && state.width == WIDTH*2 && state.height == HEIGHT*2) ||
					(state.filter == RenderState::FILTER_HQ3X && state.width == WIDTH*3 && state.height == HEIGHT*3) ||
					(state.filter == RenderState::FILTER_HQ4X && state.width == WIDTH*4 && state.height == HEIGHT*4)
				);
			}

			void Renderer::FilterHqX::Transform(const byte (&src)[PALETTE][3],Input::Palette& dst) const
			{
				if (format.bpp == 32)
				{
					for (uint i=0; i < PALETTE; ++i)
					{
						dst[i] =
						(
							((src[i][0] * 0x1FU + 0x7F) / 0xFF) << 11 |
							((src[i][1] * 0x3FU + 0x7F) / 0xFF) <<  5 |
							((src[i][2] * 0x1FU + 0x7F) / 0xFF) <<  0
						);
					}
				}
				else
				{
					Filter::Transform( src, dst );
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}

#endif
