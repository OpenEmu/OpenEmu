////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2001 Derek Liauw Kie Fa
// Copyright (C) 2008 Martin Freij
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

#include "NstAssert.hpp"

#ifndef NST_NO_2XSAI

#include "NstVideoRenderer.hpp"
#include "NstVideoFilter2xSaI.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Renderer::Filter2xSaI::Filter2xSaI(const RenderState& state)
			:
			Filter (state),
			lsb0   (~((1UL << format.shifts[0]) | (1UL << format.shifts[1]) | (1UL << format.shifts[2]))),
			lsb1   (~((3UL << format.shifts[0]) | (3UL << format.shifts[1]) | (3UL << format.shifts[2])))
			{
			}

			bool Renderer::Filter2xSaI::Check(const RenderState& state)
			{
				return
				(
					(state.bits.count == 16 || state.bits.count == 32) &&
					(state.filter == RenderState::FILTER_2XSAI && state.width == WIDTH*2 && state.height == HEIGHT*2)
				);
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			inline dword Renderer::Filter2xSaI::Blend(dword a,dword b) const
			{
				return (a != b) ? ((a & lsb0) >> 1) + ((b & lsb0) >> 1) + (a & b & ~lsb0) : a;
			}

			inline dword Renderer::Filter2xSaI::Blend(dword a,dword b,dword c,dword d) const
			{
				return
				(
					(((a & lsb1) >> 2) + ((b & lsb1) >> 2) + ((c & lsb1) >> 2) + ((d & lsb1) >> 2)) +
					((((a & ~lsb1) + (b & ~lsb1) + (c & ~lsb1) + (d & ~lsb1)) >> 2) & ~lsb1)
				);
			}

			template<typename T>
			void Renderer::Filter2xSaI::BlitType(const Input& input,const Output& output) const
			{
				const word* NST_RESTRICT src = input.pixels;
				const long pitch = output.pitch;

				T* NST_RESTRICT dst[2] =
				{
					static_cast<T*>(output.pixels),
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + pitch)
				};

				dword a,b,c,d,e=0,f=0,g,h,i=0,j=0,k,l,m,n,o;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x, ++src, dst[0] += 2, dst[1] += 2)
					{
						if (y)
						{
							i = x > 0 ?       input.palette[src[ -WIDTH-1 ]] : 0;
							e =               input.palette[src[ -WIDTH   ]];
							f = x < WIDTH-1 ? input.palette[src[ -WIDTH+1 ]] : 0;
							j = x < WIDTH-2 ? input.palette[src[ -WIDTH+2 ]] : 0;
						}

						g = x > 0 ?       input.palette[src[ -1 ]] : 0;
						a =               input.palette[src[  0 ]];
						b = x < WIDTH-1 ? input.palette[src[  1 ]] : 0;
						k = x < WIDTH-2 ? input.palette[src[  2 ]] : 0;

						if (y < HEIGHT-1)
						{
							h = x > 0 ?       input.palette[src[ WIDTH-1 ]] : 0;
							c =               input.palette[src[ WIDTH   ]];
							d = x < WIDTH-1 ? input.palette[src[ WIDTH+1 ]] : 0;
							l = x < WIDTH-2 ? input.palette[src[ WIDTH+2 ]] : 0;

							if (y < HEIGHT-2)
							{
								m = x > 0 ?       input.palette[src[ WIDTH*2-1 ]] : 0;
								n =               input.palette[src[ WIDTH*2   ]];
								o = x < WIDTH-1 ? input.palette[src[ WIDTH*2+1 ]] : 0;
							}
							else
							{
								m = n = o = 0;
							}
						}
						else
						{
							h = c = d = l = m = n = o = 0;
						}

						dword q[3];

						if (a == d && b != c)
						{
							if ((a == e && b == l) || (a == c && a == f && b != e && b == j))
							{
								q[0] = a;
							}
							else
							{
								q[0] = Blend( a, b );
							}

							if ((a == g && c == o) || (a == b && a == h && g != c && c == m))
							{
								q[1] = a;
							}
							else
							{
								q[1] = Blend( a, c );
							}

							q[2] = a;
						}
						else if (b == c && a != d)
						{
							if ((b == f && a == h) || (b == e && b == d && a != f && a == i))
							{
								q[0] = b;
							}
							else
							{
								q[0] = Blend( a, b );
							}

							if ((c == h && a == f) || (c == g && c == d && a != h && a == i))
							{
								q[1] = c;
							}
							else
							{
								q[1] = Blend( a, c );
							}

							q[2] = b;
						}
						else if (a == d && b == c)
						{
							if (a == b)
							{
								q[0] = a;
								q[1] = a;
								q[2] = a;
							}
							else
							{
								q[1] = Blend( a, c );
								q[0] = Blend( a, b );

								const int result =
								(
									(a == g && a == e ? -1 : b == g && b == e ? +1 : 0) +
									(b == k && b == f ? -1 : a == k && a == f ? +1 : 0) +
									(b == h && b == n ? -1 : a == h && a == n ? +1 : 0) +
									(a == l && a == o ? -1 : b == l && b == o ? +1 : 0)
								);

								if (result > 0)
								{
									q[2] = a;
								}
								else if (result < 0)
								{
									q[2] = b;
								}
								else
								{
									q[2] = Blend( a, b, c, d );
								}
							}
						}
						else
						{
							q[2] = Blend( a, b, c, d );

							if (a == c && a == f && b != e && b == j)
							{
								q[0] = a;
							}
							else if (b == e && b == d && a != f && a == i)
							{
								q[0] = b;
							}
							else
							{
								q[0] = Blend( a, b );
							}

							if (a == b && a == h && g != c && c == m)
							{
								q[1] = a;
							}
							else if (c == g && c == d && a != h && a == i)
							{
								q[1] = c;
							}
							else
							{
								q[1] = Blend( a, c );
							}
						}

						dst[0][0] = a;
						dst[0][1] = q[0];
						dst[1][0] = q[1];
						dst[1][1] = q[2];
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[1]) + (pitch - long(sizeof(T) * WIDTH*2)));
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[0]) + pitch);
				}
			}

			void Renderer::Filter2xSaI::Blit(const Input& input,const Output& output,uint)
			{
				switch (format.bpp)
				{
					case 32: BlitType< dword >( input, output ); break;
					case 16: BlitType< word  >( input, output ); break;
					default: NST_UNREACHABLE();
				}
			}
		}
	}
}

#endif
