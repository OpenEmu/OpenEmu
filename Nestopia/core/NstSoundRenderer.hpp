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

#ifndef NST_SOUND_RENDERER_H
#define NST_SOUND_RENDERER_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			typedef idword Sample;

			class Buffer
			{
			public:

				explicit Buffer(uint);
				~Buffer();

				enum
				{
					SIZE = 0x4000,
					MASK = SIZE-1
				};

				struct Block
				{
					inline explicit Block(uint);

					const iword* data;
					uint start;
					uint length;
				};

				void Reset(uint,bool=true);
				void operator >> (Block&);

				template<typename,uint>
				class Renderer;

			private:

				template<typename T>
				class BaseRenderer : public ImplicitBool< BaseRenderer<T> >
				{
				protected:

					T* NST_RESTRICT dst;
					const T* const end;

					inline BaseRenderer(void*,uint);

				public:

					inline bool operator !() const;
				};

				struct History
				{
					template<typename T>
					inline void operator >> (T&) const;

					inline void operator << (Sample);

					enum
					{
						SIZE = 0x40,
						MASK = SIZE-1
					};

					uint pos;
					iword buffer[SIZE];
				};

				uint pos;
				uint start;
				iword* const NST_RESTRICT output;

			public:

				inline void operator << (const Sample);

				History history;
			};

			template<>
			class Buffer::Renderer<iword,0U> : public Buffer::BaseRenderer<iword>
			{
			public:

				inline Renderer(void*,uint,const History&);

				inline void operator << (Sample);
				NST_FORCE_INLINE bool operator << (const Block&);
			};

			template<>
			class Buffer::Renderer<iword,1U> : public Buffer::BaseRenderer<iword>
			{
				History& history;

			public:

				inline Renderer(void*,uint,History&);

				inline void operator << (Sample);
				NST_FORCE_INLINE bool operator << (Block&);
			};

			template<>
			class Buffer::Renderer<byte,0U> : public Buffer::BaseRenderer<byte>
			{
			public:

				inline Renderer(void*,uint,const History&);

				inline void operator << (Sample);
				NST_FORCE_INLINE bool operator << (Block&);
			};

			template<>
			class Buffer::Renderer<byte,1U> : public Buffer::BaseRenderer<byte>
			{
				History& history;

			public:

				inline Renderer(void*,uint,History&);

				inline void operator << (Sample);
				NST_FORCE_INLINE bool operator << (Block&);
			};
		}
	}
}

#endif
