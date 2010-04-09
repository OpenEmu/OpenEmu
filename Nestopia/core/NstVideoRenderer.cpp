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
#include <cmath>
#include <new>
#include "NstCore.hpp"
#include "NstAssert.hpp"
#include "NstFpuPrecision.hpp"
#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterNone.hpp"
#include "NstVideoFilterNtsc.hpp"

#ifndef NST_NO_SCALEX
#include "NstVideoFilterScaleX.hpp"
#endif
#ifndef NST_NO_HQ2X
#include "NstVideoFilterHqX.hpp"
#endif
#ifndef NST_NO_2XSAI
#include "NstVideoFilter2xSaI.hpp"
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			const byte Renderer::Palette::pc10Palette[64][3] =
			{
				{0x6D,0x6D,0x6D}, {0x00,0x24,0x92}, {0x00,0x00,0xDB}, {0x6D,0x49,0xDB},
				{0x92,0x00,0x6D}, {0xB6,0x00,0x6D}, {0xB6,0x24,0x00}, {0x92,0x49,0x00},
				{0x6D,0x49,0x00}, {0x24,0x49,0x00}, {0x00,0x6D,0x24}, {0x00,0x92,0x00},
				{0x00,0x49,0x49}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xB6,0xB6,0xB6}, {0x00,0x6D,0xDB}, {0x00,0x49,0xFF}, {0x92,0x00,0xFF},
				{0xB6,0x00,0xFF}, {0xFF,0x00,0x92}, {0xFF,0x00,0x00}, {0xDB,0x6D,0x00},
				{0x92,0x6D,0x00}, {0x24,0x92,0x00}, {0x00,0x92,0x00}, {0x00,0xB6,0x6D},
				{0x00,0x92,0x92}, {0x24,0x24,0x24}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xFF,0xFF,0xFF}, {0x6D,0xB6,0xFF}, {0x92,0x92,0xFF}, {0xDB,0x6D,0xFF},
				{0xFF,0x00,0xFF}, {0xFF,0x6D,0xFF}, {0xFF,0x92,0x00}, {0xFF,0xB6,0x00},
				{0xDB,0xDB,0x00}, {0x6D,0xDB,0x00}, {0x00,0xFF,0x00}, {0x49,0xFF,0xDB},
				{0x00,0xFF,0xFF}, {0x49,0x49,0x49}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xFF,0xFF,0xFF}, {0xB6,0xDB,0xFF}, {0xDB,0xB6,0xFF}, {0xFF,0xB6,0xFF},
				{0xFF,0x92,0xFF}, {0xFF,0xB6,0xB6}, {0xFF,0xDB,0x92}, {0xFF,0xFF,0x49},
				{0xFF,0xFF,0x6D}, {0xB6,0xFF,0x49}, {0x92,0xFF,0x6D}, {0x49,0xFF,0xDB},
				{0x92,0xDB,0xFF}, {0x92,0x92,0x92}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
			};

			const byte Renderer::Palette::vsPalette[4][64][3] =
			{
				{
					{0xFF,0xB6,0xB6}, {0xDB,0x6D,0xFF}, {0xFF,0x00,0x00}, {0x92,0x92,0xFF},
					{0x00,0x92,0x92}, {0x24,0x49,0x00}, {0x49,0x49,0x49}, {0xFF,0x00,0x92},
					{0xFF,0xFF,0xFF}, {0x6D,0x6D,0x6D}, {0xFF,0xB6,0x00}, {0xB6,0x00,0x6D},
					{0x92,0x00,0x6D}, {0xDB,0xDB,0x00}, {0x6D,0x49,0x00}, {0xFF,0xFF,0xFF},
					{0x6D,0xB6,0xFF}, {0xDB,0xB6,0x6D}, {0x6D,0x24,0x00}, {0x6D,0xDB,0x00},
					{0x92,0xDB,0xFF}, {0xDB,0xB6,0xFF}, {0xFF,0xDB,0x92}, {0x00,0x49,0xFF},
					{0xFF,0xDB,0x00}, {0x49,0xFF,0xDB}, {0x00,0x00,0x00}, {0x49,0x00,0x00},
					{0xDB,0xDB,0xDB}, {0x92,0x92,0x92}, {0xFF,0x00,0xFF}, {0x00,0x24,0x92},
					{0x00,0x00,0x6D}, {0xB6,0xDB,0xFF}, {0xFF,0xB6,0xFF}, {0x00,0xFF,0x00},
					{0x00,0xFF,0xFF}, {0x00,0x49,0x49}, {0x00,0xB6,0x6D}, {0xB6,0x00,0xFF},
					{0x00,0x00,0x00}, {0x92,0x49,0x00}, {0xFF,0x92,0xFF}, {0xB6,0x24,0x00},
					{0x92,0x00,0xFF}, {0x00,0x00,0xDB}, {0xFF,0x92,0x00}, {0x00,0x00,0x00},
					{0x00,0x00,0x00}, {0x24,0x92,0x00}, {0xB6,0xB6,0xB6}, {0x00,0x6D,0x24},
					{0xB6,0xFF,0x49}, {0x6D,0x49,0xDB}, {0xFF,0xFF,0x00}, {0xDB,0x6D,0x00},
					{0x00,0x49,0x00}, {0x00,0x6D,0xDB}, {0x00,0x92,0x00}, {0x24,0x24,0x24},
					{0xFF,0xFF,0x6D}, {0xFF,0x6D,0xFF}, {0x92,0x6D,0x00}, {0x92,0xFF,0x6D}
				},
				{
					{0x00,0x00,0x00}, {0xFF,0xB6,0x00}, {0x92,0x6D,0x00}, {0xB6,0xFF,0x49},
					{0x92,0xFF,0x6D}, {0xFF,0x6D,0xFF}, {0x00,0x92,0x92}, {0xB6,0xDB,0xFF},
					{0xFF,0x00,0x00}, {0x92,0x00,0xFF}, {0xFF,0xFF,0x6D}, {0xFF,0x92,0xFF},
					{0xFF,0xFF,0xFF}, {0xDB,0x6D,0xFF}, {0x92,0xDB,0xFF}, {0x00,0x92,0x00},
					{0x00,0x49,0x00}, {0x6D,0xB6,0xFF}, {0xB6,0x24,0x00}, {0xDB,0xDB,0xDB},
					{0x00,0xB6,0x6D}, {0x6D,0xDB,0x00}, {0x49,0x00,0x00}, {0x92,0x92,0xFF},
					{0x49,0x49,0x49}, {0xFF,0x00,0xFF}, {0x00,0x00,0x6D}, {0x49,0xFF,0xDB},
					{0xDB,0xB6,0xFF}, {0x6D,0x49,0x00}, {0x00,0x00,0x00}, {0x6D,0x49,0xDB},
					{0x92,0x00,0x6D}, {0xFF,0xDB,0x92}, {0xFF,0x92,0x00}, {0xFF,0xB6,0xFF},
					{0x00,0x6D,0xDB}, {0x6D,0x24,0x00}, {0xB6,0xB6,0xB6}, {0x00,0x00,0xDB},
					{0xB6,0x00,0xFF}, {0xFF,0xDB,0x00}, {0x6D,0x6D,0x6D}, {0x24,0x49,0x00},
					{0x00,0x49,0xFF}, {0x00,0x00,0x00}, {0xDB,0xDB,0x00}, {0xFF,0xFF,0xFF},
					{0xDB,0xB6,0x6D}, {0x24,0x24,0x24}, {0x00,0xFF,0x00}, {0xDB,0x6D,0x00},
					{0x00,0x49,0x49}, {0x00,0x24,0x92}, {0xFF,0x00,0x92}, {0x24,0x92,0x00},
					{0x00,0x00,0x00}, {0x00,0xFF,0xFF}, {0x92,0x49,0x00}, {0xFF,0xFF,0x00},
					{0xFF,0xB6,0xB6}, {0xB6,0x00,0x6D}, {0x00,0x6D,0x24}, {0x92,0x92,0x92}
				},
				{
					{0xB6,0x00,0xFF}, {0xFF,0x6D,0xFF}, {0x92,0xFF,0x6D}, {0xB6,0xB6,0xB6},
					{0x00,0x92,0x00}, {0xFF,0xFF,0xFF}, {0xB6,0xDB,0xFF}, {0x24,0x49,0x00},
					{0x00,0x24,0x92}, {0x00,0x00,0x00}, {0xFF,0xDB,0x92}, {0x6D,0x49,0x00},
					{0xFF,0x00,0x92}, {0xDB,0xDB,0xDB}, {0xDB,0xB6,0x6D}, {0x92,0xDB,0xFF},
					{0x92,0x92,0xFF}, {0x00,0x92,0x92}, {0xB6,0x00,0x6D}, {0x00,0x49,0xFF},
					{0x24,0x92,0x00}, {0x92,0x6D,0x00}, {0xDB,0x6D,0x00}, {0x00,0xB6,0x6D},
					{0x6D,0x6D,0x6D}, {0x6D,0x49,0xDB}, {0x00,0x00,0x00}, {0x00,0x00,0xDB},
					{0xFF,0x00,0x00}, {0xB6,0x24,0x00}, {0xFF,0x92,0xFF}, {0xFF,0xB6,0xB6},
					{0xDB,0x6D,0xFF}, {0x00,0x49,0x00}, {0x00,0x00,0x6D}, {0xFF,0xFF,0x00},
					{0x24,0x24,0x24}, {0xFF,0xB6,0x00}, {0xFF,0x92,0x00}, {0xFF,0xFF,0xFF},
					{0x6D,0xDB,0x00}, {0x92,0x00,0x6D}, {0x6D,0xB6,0xFF}, {0xFF,0x00,0xFF},
					{0x00,0x6D,0xDB}, {0x92,0x92,0x92}, {0x00,0x00,0x00}, {0x6D,0x24,0x00},
					{0x00,0xFF,0xFF}, {0x49,0x00,0x00}, {0xB6,0xFF,0x49}, {0xFF,0xB6,0xFF},
					{0x92,0x49,0x00}, {0x00,0xFF,0x00}, {0xDB,0xDB,0x00}, {0x49,0x49,0x49},
					{0x00,0x6D,0x24}, {0x00,0x00,0x00}, {0xDB,0xB6,0xFF}, {0xFF,0xFF,0x6D},
					{0x92,0x00,0xFF}, {0x49,0xFF,0xDB}, {0xFF,0xDB,0x00}, {0x00,0x49,0x49}
				},
				{
					{0x92,0x6D,0x00}, {0x6D,0x49,0xDB}, {0x00,0x92,0x92}, {0xDB,0xDB,0x00},
					{0x00,0x00,0x00}, {0xFF,0xB6,0xB6}, {0x00,0x24,0x92}, {0xDB,0x6D,0x00},
					{0xB6,0xB6,0xB6}, {0x6D,0x24,0x00}, {0x00,0xFF,0x00}, {0x00,0x00,0x6D},
					{0xFF,0xDB,0x92}, {0xFF,0xFF,0x00}, {0x00,0x92,0x00}, {0xB6,0xFF,0x49},
					{0xFF,0x6D,0xFF}, {0x49,0x00,0x00}, {0x00,0x49,0xFF}, {0xFF,0x92,0xFF},
					{0x00,0x00,0x00}, {0x49,0x49,0x49}, {0xB6,0x24,0x00}, {0xFF,0x92,0x00},
					{0xDB,0xB6,0x6D}, {0x00,0xB6,0x6D}, {0x92,0x92,0xFF}, {0x24,0x92,0x00},
					{0x92,0x00,0x6D}, {0x00,0x00,0x00}, {0x92,0xFF,0x6D}, {0x6D,0xB6,0xFF},
					{0xB6,0x00,0x6D}, {0x00,0x6D,0x24}, {0x92,0x49,0x00}, {0x00,0x00,0xDB},
					{0x92,0x00,0xFF}, {0xB6,0x00,0xFF}, {0x6D,0x6D,0x6D}, {0xFF,0x00,0x92},
					{0x00,0x49,0x49}, {0xDB,0xDB,0xDB}, {0x00,0x6D,0xDB}, {0x00,0x49,0x00},
					{0x24,0x24,0x24}, {0xFF,0xFF,0x6D}, {0x92,0x92,0x92}, {0xFF,0x00,0xFF},
					{0xFF,0xB6,0xFF}, {0xFF,0xFF,0xFF}, {0x6D,0x49,0x00}, {0xFF,0x00,0x00},
					{0xFF,0xDB,0x00}, {0x49,0xFF,0xDB}, {0xFF,0xFF,0xFF}, {0x92,0xDB,0xFF},
					{0x00,0x00,0x00}, {0xFF,0xB6,0x00}, {0xDB,0x6D,0xFF}, {0xB6,0xDB,0xFF},
					{0x6D,0xDB,0x00}, {0xDB,0xB6,0xFF}, {0x00,0xFF,0xFF}, {0x24,0x49,0x00}
				}
			};

			const double Renderer::Palette::Constants::pi = 3.141592653589793;
			const double Renderer::Palette::Constants::deg = 0.017453292519943296;

			const double Renderer::Palette::Constants::levels[2][4] =
			{
				{-0.12, 0.00, 0.31, 0.72 },
				{ 0.40, 0.68, 1.00, 1.00 }
			};

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			inline Renderer::Palette::Custom::Custom()
			: emphasis(NULL) {}

			inline Renderer::Palette::Custom::~Custom()
			{
				delete [] emphasis;
			}

			bool Renderer::Palette::Custom::EnableEmphasis(bool enable)
			{
				if (!enable)
				{
					delete [] emphasis;
					emphasis = NULL;
				}
				else if (!emphasis)
				{
					emphasis = new (std::nothrow) byte [7][64][3];
				}

				return bool(emphasis) == enable;
			}

			Renderer::Palette::Palette()
			: type(PALETTE_YUV), custom(NULL)
			{
			}

			Renderer::Palette::~Palette()
			{
				delete custom;
			}

			Result Renderer::Palette::SetDecoder(const Decoder& d)
			{
				if (decoder == d)
					return RESULT_NOP;

				for (uint i=0; i < 3; ++i)
				{
					if (d.axes[i].angle >= 360 || d.axes[i].gain > 2.0)
						return RESULT_ERR_INVALID_PARAM;
				}

				decoder = d;
				return RESULT_OK;
			}

			void Renderer::Palette::Store(const double (&src)[3],byte (&dst)[3])
			{
				for (uint i=0; i < 3; ++i)
					dst[i] = Clamp<0,255>( src[i] * 255 + 0.5 );
			}

			Result Renderer::Palette::LoadCustom(const byte (*colors)[3],const bool emphasis)
			{
				if (!colors)
					return RESULT_ERR_INVALID_PARAM;

				if ((custom == NULL && NULL == (custom = new (std::nothrow) Custom)) || !custom->EnableEmphasis( emphasis ))
					return RESULT_ERR_OUT_OF_MEMORY;

				std::memcpy( custom->palette, colors, 64*3 );

				if (emphasis)
					std::memcpy( custom->emphasis, colors + 64, 7*64*3 );

				return RESULT_OK;
			}

			uint Renderer::Palette::SaveCustom(byte (*colors)[3],const bool emphasis) const
			{
				if (!colors)
					return 0;

				std::memcpy( colors, custom ? custom->palette : pc10Palette, 64*3 );

				if (!emphasis || !custom || !custom->emphasis)
					return 64;

				std::memcpy( colors + 64, custom->emphasis, 7*64*3 );

				return 7*64;
			}

			bool Renderer::Palette::ResetCustom()
			{
				if (custom)
				{
					custom->EnableEmphasis( false );
					std::memcpy( custom->palette, pc10Palette, 64*3 );
					return true;
				}

				return false;
			}

			Result Renderer::Palette::SetType(PaletteType t)
			{
				if (t == type)
					return RESULT_NOP;

				if (t == PALETTE_CUSTOM && !custom)
				{
					if (NULL == (custom = new (std::nothrow) Custom))
						return RESULT_ERR_OUT_OF_MEMORY;

					ResetCustom();
				}

				type = t;

				return RESULT_OK;
			}

			void Renderer::Palette::GenerateEmphasis(uint tint,double s,double& y,double& i,double& q)
			{
				if (tint == 7)
				{
					y = y * (0.79399 * 1.13) - (0.0782838 * 1.13);
				}
				else
				{
					s = s * (0.5 - 0.79399 * 0.5) + 0.0782838 * 0.5;
					y -= s * 0.5;

					if (tint >= 3 && tint != 4)
					{
						s *= 0.6;
						y -= s;
					}

					static const byte tints[8] =
					{
						0, 6, 10, 8, 2, 4, 0, 0
					};

					const double a = Constants::pi / 12 * (tints[tint] * 2 - 7);

					i += std::sin( a ) * s;
					q += std::cos( a ) * s;
				}
			}

			void Renderer::Palette::Build(const int bi,const int si,const int ci,const int hue)
			{
				NST_ASSERT( type != PALETTE_YUV );

				const double brightness = bi / 200.0;
				const double saturation = (si + 100) / 100.0;
				const double contrast = (ci + 100) / 100.0;

				const double matrix[6] =
				{
					std::sin( (90  - 33 - hue) * Constants::deg ) * (0.570 * 2),
					std::cos( (90  - 33 - hue) * Constants::deg ) * (0.570 * 2),
					std::sin( (236 - 33 - hue) * Constants::deg ) * (0.351 * 2),
					std::cos( (236 - 33 - hue) * Constants::deg ) * (0.351 * 2),
					std::sin( (0   - 33 - hue) * Constants::deg ) * (1.015 * 2),
					std::cos( (0   - 33 - hue) * Constants::deg ) * (1.015 * 2)
				};

				const byte (*from)[3] =
				(
					type == PALETTE_CUSTOM ? custom->palette :
					type == PALETTE_VS1    ? vsPalette[0] :
					type == PALETTE_VS2    ? vsPalette[1] :
					type == PALETTE_VS3    ? vsPalette[2] :
					type == PALETTE_VS4    ? vsPalette[3] :
                                             pc10Palette
				);

				NST_ASSERT( from );

				for (uint i=0; i < 8; ++i)
				{
					if (i && type == PALETTE_CUSTOM && custom->emphasis)
						from = custom->emphasis[i-1];

					for (uint j=0; j < 64; ++j)
					{
						double rgb[3] =
						{
							from[j][0] / 255.0,
							from[j][1] / 255.0,
							from[j][2] / 255.0
						};

						if (i && type != PALETTE_CUSTOM)
						{
							switch (i)
							{
								case 1: rgb[0] = 1;                         break;
								case 2: rgb[1] = 1;                         break;
								case 3: rgb[0] = 1; rgb[1] = 1;             break;
								case 4: rgb[2] = 1;                         break;
								case 5: rgb[0] = 1; rgb[2] = 1;             break;
								case 6: rgb[1] = 1; rgb[2] = 1;             break;
								case 7: rgb[0] = 1; rgb[1] = 1; rgb[2] = 1; break;
							}
						}

						double yiq[3] =
						{
							0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2],
							0.596 * rgb[0] - 0.275 * rgb[1] - 0.321 * rgb[2],
							0.212 * rgb[0] - 0.523 * rgb[1] + 0.311 * rgb[2]
						};

						if (i && type == PALETTE_CUSTOM && !custom->emphasis && (j & 0xF) <= 0xD)
							GenerateEmphasis( i, Constants::levels[(j & 0xF) != 0xD][j >> 4 & 0x3], yiq[0], yiq[1], yiq[2] );

						yiq[0] = yiq[0] * contrast + brightness;
						yiq[1] *= saturation;
						yiq[2] *= saturation;

						for (uint k=0; k < 3; ++k)
							rgb[k] = yiq[0] + matrix[k*2+0] * yiq[1] + matrix[k*2+1] * yiq[2];

						Store( rgb, palette[(i * 64) + j] );
					}
				}
			}

			void Renderer::Palette::Generate(const int b,const int s,const int c,int hue)
			{
				NST_ASSERT( type == PALETTE_YUV );

				const double brightness = b / 200.0;
				const double saturation = (s + 100) / 100.0;
				const double contrast = (c + 100) / 100.0;
				hue += 33;

				const double matrix[6] =
				{
					std::sin( (int(decoder.axes[0].angle) - hue) * Constants::deg ) * decoder.axes[0].gain * 2,
					std::cos( (int(decoder.axes[0].angle) - hue) * Constants::deg ) * decoder.axes[0].gain * 2,
					std::sin( (int(decoder.axes[1].angle) - hue) * Constants::deg ) * decoder.axes[1].gain * 2,
					std::cos( (int(decoder.axes[1].angle) - hue) * Constants::deg ) * decoder.axes[1].gain * 2,
					std::sin( (int(decoder.axes[2].angle) - hue) * Constants::deg ) * decoder.axes[2].gain * 2,
					std::cos( (int(decoder.axes[2].angle) - hue) * Constants::deg ) * decoder.axes[2].gain * 2
				};

				for (uint n=0; n < PALETTE; ++n)
				{
					double level[2] =
					{
						Constants::levels[0][n >> 4 & 3],
						Constants::levels[1][n >> 4 & 3]
					};

					const int color = n & 0x0F;

					if (color == 0x00)
					{
						level[0] = level[1];
					}
					else if (color == 0x0D)
					{
						level[1] = level[0];
					}
					else if (color > 0x0D)
					{
						level[0] = 0;
						level[1] = 0;
					}

					double y = (level[1] + level[0]) * 0.5;
					double s = (level[1] - level[0]) * 0.5;
					double h = Constants::pi / 6 * (color - 3);

					double i = std::sin( h ) * s;
					double q = std::cos( h ) * s;

					const uint tint = n >> 6 & 7;

					if (tint && color <= 0x0D)
						GenerateEmphasis( tint, level[1], y, i, q );

					if (decoder.boostYellow)
					{
						const double yellowness = i - q;

						if (yellowness > DBL_EPSILON)
						{
							i = i + yellowness * ((n >> 4 & 3) / 4.0);
							q = q - yellowness * ((n >> 4 & 3) / 4.0);
						}
					}

					i *= saturation;
					q *= saturation;
					y = y * contrast + brightness;

					const double rgb[3] =
					{
						y + matrix[0] * i + matrix[1] * q,
						y + matrix[2] * i + matrix[3] * q,
						y + matrix[4] * i + matrix[5] * q
					};

					Store( rgb, palette[n] );
				}
			}

			void Renderer::Palette::Update(int brightness,int saturation,int contrast,int hue)
			{
				FpuPrecision precision;
#pragma unused (precision)
				(*this.*(type == PALETTE_YUV ? &Palette::Generate : &Palette::Build))( brightness, saturation, contrast, hue );
			}

			inline const Renderer::PaletteEntries& Renderer::Palette::Get() const
			{
				return palette;
			}

			Renderer::Filter::Format::Format(const RenderState& state)
			: bpp(state.bits.count)
			{
				for (uint i=0; i < 3; ++i)
				{
					ulong mask = (i == 0 ? state.bits.mask.r : i == 1 ? state.bits.mask.g : state.bits.mask.b);

					shifts[i] = 0;

					if (mask)
					{
						while (!(mask & 0x1))
						{
							mask >>= 1;
							shifts[i]++;
						}
					}

					masks[i] = mask;
				}
			}

			Renderer::Filter::Filter(const RenderState& state)
			: format(state) {}

			void Renderer::Filter::Transform(const byte (&src)[PALETTE][3],Input::Palette& dst) const
			{
				for (uint i=0; i < PALETTE; ++i)
				{
					dst[i] =
					(
						((src[i][0] * format.masks[0] + 0x7F) / 0xFF) << format.shifts[0] |
						((src[i][1] * format.masks[1] + 0x7F) / 0xFF) << format.shifts[1] |
						((src[i][2] * format.masks[2] + 0x7F) / 0xFF) << format.shifts[2]
					);
				}
			}

			Renderer::State::State()
			:
			width        (0),
			height       (0),
			filter       (RenderState::FILTER_NONE),
			update       (UPDATE_PALETTE),
			brightness   (0),
			saturation   (0),
			hue          (0),
			contrast     (0),
			sharpness    (0),
			resolution   (0),
			bleed        (0),
			artifacts    (0),
			fringing     (0),
			fieldMerging (0)
			{
				mask.r = 0;
				mask.g = 0;
				mask.b = 0;
			}

			Renderer::Renderer()
			: filter(NULL) {}

			Renderer::~Renderer()
			{
				delete filter;
			}

			Result Renderer::SetState(const RenderState& renderState)
			{
				if (filter)
				{
					if
					(
						state.filter == renderState.filter &&
						state.width == renderState.width &&
						state.height == renderState.height &&
						filter->format.bpp == renderState.bits.count &&
						state.mask.r == renderState.bits.mask.r &&
						state.mask.g == renderState.bits.mask.g &&
						state.mask.b == renderState.bits.mask.b
					)
						return RESULT_NOP;

					delete filter;
					filter = NULL;
				}

				try
				{
					switch (renderState.filter)
					{
						case RenderState::FILTER_NONE:

							if (FilterNone::Check( renderState ))
								filter = new FilterNone( renderState );

							break;

					#ifndef NST_NO_SCALEX

						case RenderState::FILTER_SCALE2X:
						case RenderState::FILTER_SCALE3X:

							if (FilterScaleX::Check( renderState ))
								filter = new FilterScaleX( renderState );

							break;
					#endif
					#ifndef NST_NO_HQ2X

						case RenderState::FILTER_HQ2X:
						case RenderState::FILTER_HQ3X:
						case RenderState::FILTER_HQ4X:

							if (FilterHqX::Check( renderState ))
								filter = new FilterHqX( renderState );

							break;

					#endif
					#ifndef NST_NO_2XSAI

						case RenderState::FILTER_2XSAI:

							if (Filter2xSaI::Check( renderState ))
								filter = new Filter2xSaI( renderState );

							break;

					#endif

						case RenderState::FILTER_NTSC:

							if (FilterNtsc::Check( renderState ))
							{
								filter = new FilterNtsc
								(
									renderState,
									GetPalette(),
									state.sharpness,
									state.resolution,
									state.bleed,
									state.artifacts,
									state.fringing,
									state.fieldMerging
								);
							}
							break;
					}
				}
				catch (const std::bad_alloc&)
				{
					delete filter;
					filter = NULL;

					return RESULT_ERR_OUT_OF_MEMORY;
				}

				if (filter)
				{
					state.filter = renderState.filter;
					state.width = renderState.width;
					state.height = renderState.height;
					state.mask = renderState.bits.mask;

					if (state.filter == RenderState::FILTER_NTSC)
						state.update = 0;
					else
						state.update |= uint(State::UPDATE_FILTER);

					return RESULT_OK;
				}
				else
				{
					return RESULT_ERR_UNSUPPORTED;
				}
			}

			Result Renderer::GetState(RenderState& output) const
			{
				if (filter)
				{
					output.filter = static_cast<RenderState::Filter>(state.filter);
					output.width = state.width;
					output.height = state.height;
					output.bits.count = filter->format.bpp;
					output.bits.mask = state.mask;

					return RESULT_OK;
				}

				return RESULT_ERR_NOT_READY;
			}

			void Renderer::EnableFieldMerging(bool fieldMerging)
			{
				const bool old = state.fieldMerging;
				state.fieldMerging &= uint(State::FIELD_MERGING_FORCED);

				if (fieldMerging)
					state.fieldMerging |= uint(State::FIELD_MERGING_USER);

				if (bool(state.fieldMerging) != old)
					state.update |= uint(State::UPDATE_NTSC);
			}

			void Renderer::EnableForcedFieldMerging(bool fieldMerging)
			{
				const bool old = state.fieldMerging;
				state.fieldMerging &= uint(State::FIELD_MERGING_USER);

				if (fieldMerging)
					state.fieldMerging |= uint(State::FIELD_MERGING_FORCED);

				if (bool(state.fieldMerging) != old)
					state.update |= uint(State::UPDATE_NTSC);
			}

			Result Renderer::SetHue(int hue)
			{
				if (hue < -45 || hue > 45)
					return RESULT_ERR_INVALID_PARAM;

				if (state.hue == hue)
					return RESULT_NOP;

				state.hue = hue;
				state.update |= uint(State::UPDATE_PALETTE|State::UPDATE_FILTER);

				return RESULT_OK;
			}

			Result Renderer::SetLevel(schar& type,int value,uint update)
			{
				if (value < -100 || value > 100)
					return RESULT_ERR_INVALID_PARAM;

				if (type == value)
					return RESULT_NOP;

				type = value;
				state.update |= update;

				return RESULT_OK;
			}

			Result Renderer::SetDecoder(const Decoder& decoder)
			{
				const Result result = palette.SetDecoder( decoder );

				if (NES_SUCCEEDED(result) && result != RESULT_NOP && palette.GetType() == PALETTE_YUV)
					state.update |= uint(State::UPDATE_PALETTE|State::UPDATE_FILTER);

				return result;
			}

			Result Renderer::SetPaletteType(PaletteType type)
			{
				const Result result = palette.SetType( type );

				if (NES_SUCCEEDED(result) && result != RESULT_NOP)
					state.update |= uint(State::UPDATE_PALETTE|State::UPDATE_FILTER);

				return result;
			}

			Result Renderer::LoadCustomPalette(const byte (*colors)[3],const bool emphasis)
			{
				const Result result = palette.LoadCustom( colors, emphasis );

				if (NES_SUCCEEDED(result) && result != RESULT_NOP && palette.GetType() == PALETTE_CUSTOM)
					state.update |= uint(State::UPDATE_PALETTE|State::UPDATE_FILTER);

				return result;
			}

			void Renderer::ResetCustomPalette()
			{
				if (palette.ResetCustom() && palette.GetType() == PALETTE_CUSTOM)
					state.update |= uint(State::UPDATE_PALETTE|State::UPDATE_FILTER);
			}

			const Renderer::PaletteEntries& Renderer::GetPalette()
			{
				if (state.update & uint(State::UPDATE_PALETTE))
				{
					state.update &= ~uint(State::UPDATE_PALETTE);
					palette.Update( state.brightness, state.saturation, state.contrast, state.hue );
				}

				return palette.Get();
			}

			void Renderer::UpdateFilter(Input& input)
			{
				NST_VERIFY( state.update );

				if (state.filter == RenderState::FILTER_NTSC)
				{
					RenderState renderState;
					GetState( renderState );

					delete filter;
					filter = NULL;

					SetState( renderState );
				}
				else if (state.update & uint(State::UPDATE_FILTER))
				{
					filter->Transform( GetPalette(), input.palette );
				}

				state.update = 0;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Renderer::Blit(Output& output,Input& input,uint burstPhase)
			{
				if (filter)
				{
					if (state.update)
						UpdateFilter( input );

					if (Output::lockCallback( output ))
					{
						NST_VERIFY( std::labs(output.pitch) >= dword(state.width) << (filter->format.bpp / 16) );

						if (std::labs(output.pitch) >= dword(state.width) << (filter->format.bpp / 16))
							filter->Blit( input, output, burstPhase );

						Output::unlockCallback( output );
					}
				}
			}
		}
	}
}
