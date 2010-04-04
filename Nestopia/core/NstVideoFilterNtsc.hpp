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

#ifndef NST_VIDEO_FILTER_NTSC_H
#define NST_VIDEO_FILTER_NTSC_H

#include "../nes_ntsc/nes_ntsc.h"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Renderer::FilterNtsc : public Renderer::Filter
			{
			public:

				FilterNtsc(const RenderState&,const byte (&)[PALETTE][3],schar,schar,schar,schar,schar,bool);

				static bool Check(const RenderState&);

			private:

				~FilterNtsc() {}

				enum
				{
					NTSC_WIDTH = 602
				};

				typedef void (FilterNtsc::*Path)(const Input&,const Output&,uint) const;

				void Blit(const Input&,const Output&,uint);

				template<typename T,uint BITS>
				void BlitType(const Input&,const Output&,uint) const;

				class Lut : public nes_ntsc_t
				{
					enum
					{
						DEF_BLACK = 15
					};

					static inline uint GetBlack(const byte (&)[PALETTE][3]);

				public:

					Lut(const byte (&)[PALETTE][3],schar,schar,schar,schar,schar,bool);

					const uint noFieldMerging;
					const uint black;
				};

				static Path GetPath(const RenderState&,const Lut&);

				const Path path;
				const Lut lut;
			};
		}
	}
}

#endif
