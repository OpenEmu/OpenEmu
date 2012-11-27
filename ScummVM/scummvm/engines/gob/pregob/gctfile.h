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

#ifndef GOB_PREGOB_GCTFILE_H
#define GOB_PREGOB_GCTFILE_H

#include "common/str.h"
#include "common/array.h"
#include "common/list.h"

#include "gob/backbuffer.h"

namespace Common {
	class RandomSource;
	class SeekableReadStream;
}

namespace Gob {

class Surface;
class Font;

class GCTFile : public BackBuffer {
public:
	static const uint16 kSelectorAll    = 0xFFFE; ///< Print all lines.
	static const uint16 kSelectorRandom = 0xFFFF; ///< Print a random line.


	GCTFile(Common::SeekableReadStream &gct, Common::RandomSource &rnd);
	~GCTFile();

	/** Return the number of lines in an item. */
	uint16 getLineCount(uint item) const;

	/** Set the area the text will be printed in. */
	void setArea(int16 left, int16 top, int16 right, int16 bottom);

	/** Set which line of this item should be printed. */
	void selectLine(uint item, uint16 line);

	/** Change the text of an items' line. */
	void setText(uint item, uint16 line, const Common::String &text);
	/** Change the item into one one line and set that line's text. */
	void setText(uint item, const Common::String &text);

	/** Reset the item drawing state. */
	void reset();

	/** Clear the drawn text, restoring the original content. */
	bool clear(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);

	/** Fill the text area with a color. */
	bool fill(Surface &dest, uint8 color, int16 &left, int16 &top, int16 &right, int16 &bottom);

	/** Draw an item onto the surface, until all text has been drawn or the area is filled. */
	bool draw(Surface &dest, uint16 item, const Font &font, uint8 color,
	          int16 &left, int16 &top, int16 &right, int16 &bottom);

	/** Did we draw all text? */
	bool finished() const;

private:
	/** The type of a chunk. */
	enum ChunkType {
		kChunkTypeNone   = 0, ///< Do nothing.
		kChunkTypeString    , ///< A direct string.
		kChunkTypeItem        ///< A reference to an item to print instead.
	};

	/** A chunk in an item text line. */
	struct Chunk {
		ChunkType type; ///< The type of the chunk.

		Common::String text; ///< Text to print.

		int item; ///< Item to print instead.

		Chunk();
	};

	typedef Common::List<Chunk> Chunks;

	/** A line in an item. */
	struct Line {
		Chunks chunks; ///< The chunks that make up the line.
	};

	typedef Common::Array<Line> Lines;

	/** A GCT item. */
	struct Item {
		Lines  lines;    ///< The text lines in the item
		uint16 selector; ///< Which line to print.
	};

	typedef Common::Array<Item> Items;


	Common::RandomSource *_rnd;

	Items _items; ///< All GCT items.

	// The area on which to print
	bool  _hasArea;
	int16 _areaLeft;
	int16 _areaTop;
	int16 _areaRight;
	int16 _areaBottom;

	/** Index of the current item we're drawing. */
	uint16 _currentItem;
	/** Text left to draw. */
	Common::List<Common::String> _currentText;


	// -- Loading helpers --

	void load(Common::SeekableReadStream &gct);
	void readLine(Common::SeekableReadStream &gct, Line &line, uint16 lineSize) const;


	// -- Draw helpers --

	Common::String getLineText(const Line &line) const;
	void getItemText(uint item, Common::List<Common::String> &text) const;
};

} // End of namespace Gob

#endif // GOB_PREGOB_GCTFILE_H
