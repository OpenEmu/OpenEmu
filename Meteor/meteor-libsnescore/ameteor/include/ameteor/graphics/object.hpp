// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __GRAPHICS_OBJECT_H__
#define __GRAPHICS_OBJECT_H__

#include <stdint.h>
#include <vector>
#include <list>

namespace AMeteor
{
	namespace Graphics
	{
		class Object
		{
			public :
				static const uint8_t FLIP_HORIZONTAL = 1;
				static const uint8_t FLIP_VERTICAL = 2;

				Object (uint16_t* pPalette, uint8_t* pChar);
				// Warning : this copy constructor must not be used on an used object
				// use it only on just created objects
				Object (const Object& obj);

				inline uint8_t GetPriority () const;
				inline int8_t GetRotationParam () const;
				inline uint16_t GetTileNum () const;
				inline bool IsWindow () const;

				void DrawLine (uint8_t line, uint32_t* surface, bool oneDim,
						uint8_t mosaic);
				void DrawLineRot (uint8_t line, uint32_t* surface, bool oneDim,
						int16_t a, int16_t b, int16_t c, int16_t d, uint8_t mosaic);
				void DrawWindow (uint8_t line, uint8_t* surface, bool oneDim,
						uint8_t mask);
				void DrawWindowRot (uint8_t line, uint8_t* surface,
						bool oneDim, int16_t a, int16_t b, int16_t c, int16_t d,
						uint8_t mask);

				void UpdateAttrs (uint16_t attr0, uint16_t attr1, uint16_t attr2);
				void UpdateAttr0 (uint16_t attr);
				void UpdateAttr1 (uint16_t attr);
				void UpdateAttr2 (uint16_t attr);

			private :
				inline void SetSize ();

				enum Shape
				{
					SHAPE_SQUARE = 0,
					SHAPE_HORIZONTAL,
					SHAPE_VERTICAL,
					SHAPE_PROHIBITED
				};

				uint16_t m_attr0, m_attr1, m_attr2;
				uint8_t m_width, m_height;
				uint16_t* m_pPalette;
				uint8_t* m_pChar;
				uint32_t m_charBegin;
				uint32_t m_charEnd;
		};

		inline int8_t Object::GetRotationParam () const
		{
			return (m_attr0 & (0x1 << 8)) ? (m_attr1 >> 9) & 0x1F : -1;
		}

		inline uint8_t Object::GetPriority () const
		{
			return (m_attr2 >> 10) & 0x3;
		}

		inline bool Object::IsWindow () const
		{
			return (m_attr0 & (0x3 << 10)) == (0x2 << 10);
		}

		inline uint16_t Object::GetTileNum () const
		{
			return (m_attr2 & 0x3FF);
		}
	}
}

#endif
