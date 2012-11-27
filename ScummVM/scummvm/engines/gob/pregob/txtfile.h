/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GOB_PREGOB_TXTFILE_H
#define GOB_PREGOB_TXTFILE_H

#include "common/system.h"
#include "common/str.h"
#include "common/array.h"

#include "gob/backbuffer.h"

namespace Common {
	class SeekableReadStream;
}

namespace Gob {

class Surface;
class Font;

class TXTFile : public BackBuffer {
public:
	enum Format {
		kFormatString,
		kFormatStringPosition,
		kFormatStringPositionColor,
		kFormatStringPositionColorFont
	};

	struct Line {
		Common::String text;
		int x, y;
		int color;
		uint font;
	};

	typedef Common::Array<Line> LineArray;

	TXTFile(Common::SeekableReadStream &txt, Format format);
	~TXTFile();

	LineArray &getLines();

	bool draw(           Surface &surface, const Font * const *fonts, uint fontCount, int color = -1);
	bool draw(uint line, Surface &surface, const Font * const *fonts, uint fontCount, int color = -1);

	bool draw(           Surface &surface, int16 &left, int16 &top, int16 &right, int16 &bottom,
	          const Font * const *fonts, uint fontCount, int color = -1);
	bool draw(uint line, Surface &surface, int16 &left, int16 &top, int16 &right, int16 &bottom,
	          const Font * const *fonts, uint fontCount, int color = -1);

	bool clear(Surface &surface, int16 &left, int16 &top, int16 &right, int16 &bottom);

private:
	LineArray _lines;

	void load(Common::SeekableReadStream &txt, Format format);

	Common::String getStr(Common::SeekableReadStream &txt);
	int getInt(Common::SeekableReadStream &txt);


	bool getArea(           int16 &left, int16 &top, int16 &right, int16 &bottom,
	             const Font * const *fonts, uint fontCount) const;
	bool getArea(uint line, int16 &left, int16 &top, int16 &right, int16 &bottom,
	             const Font * const *fonts, uint fontCount) const;
};

} // End of namespace Gob

#endif // GOB_PREGOB_TXTFILE_H
