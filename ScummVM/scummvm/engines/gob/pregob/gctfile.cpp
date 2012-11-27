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

#include "common/random.h"
#include "common/stream.h"

#include "gob/surface.h"
#include "gob/video.h"

#include "gob/pregob/gctfile.h"

namespace Gob {

GCTFile::Chunk::Chunk() : type(kChunkTypeNone) {
}


GCTFile::GCTFile(Common::SeekableReadStream &gct, Common::RandomSource &rnd) : _rnd(&rnd),
	_areaLeft(0), _areaTop(0), _areaRight(0), _areaBottom(0), _currentItem(0xFFFF) {

	load(gct);
}

GCTFile::~GCTFile() {
}

void GCTFile::load(Common::SeekableReadStream &gct) {
	gct.skip(4); // Required buffer size
	gct.skip(2); // Unknown

	// Read the selector and line counts for each item
	const uint16 itemCount = gct.readUint16LE();
	_items.resize(itemCount);

	for (Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		const uint16 selector  = gct.readUint16LE();
		const uint16 lineCount = gct.readUint16LE();

		i->selector = selector;
		i->lines.resize(lineCount);
	}

	// Read all item lines
	for (Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		for (Lines::iterator l = i->lines.begin(); l != i->lines.end(); ++l) {
			const uint16 lineSize = gct.readUint16LE();

			readLine(gct, *l, lineSize);
		}
	}

	if (gct.err())
		error("GCTFile::load(): Failed reading GCT");
}

void GCTFile::readLine(Common::SeekableReadStream &gct, Line &line, uint16 lineSize) const {
	line.chunks.push_back(Chunk());

	while (lineSize > 0) {
		byte c = gct.readByte();
		lineSize--;

		if (c == 0) {
			// Command byte

			if (lineSize == 0)
				break;

			byte cmd = gct.readByte();
			lineSize--;

			// Line end command
			if (cmd == 0)
				break;

			// Item reference command
			if (cmd == 1) {
				if (lineSize < 2) {
					warning("GCTFile::readLine(): Item reference command is missing parameters");
					break;
				}

				const uint32 itemRef = gct.readUint16LE();
				lineSize -= 2;

				line.chunks.push_back(Chunk());
				line.chunks.back().type = kChunkTypeItem;
				line.chunks.back().item = itemRef;

				line.chunks.push_back(Chunk());
				continue;
			}

			warning("GCTFile::readLine(): Invalid command 0x%02X", cmd);
			break;
		}

		// Text
		line.chunks.back().type = kChunkTypeString;
		line.chunks.back().text += (char)c;
	}

	// Skip bytes we didn't read (because of errors)
	gct.skip(lineSize);

	// Remove empty chunks from the end of the list
	while (!line.chunks.empty() && (line.chunks.back().type == kChunkTypeNone))
		line.chunks.pop_back();
}

uint16 GCTFile::getLineCount(uint item) const {
	if (item >= _items.size())
		return 0;

	return _items[item].lines.size();
}

void GCTFile::selectLine(uint item, uint16 line) {
	if ((item >= _items.size()) && (item != kSelectorAll) && (item != kSelectorRandom))
		return;

	_items[item].selector = line;
}

void GCTFile::setText(uint item, uint16 line, const Common::String &text) {
	if ((item >= _items.size()) || (line >= _items[item].lines.size()))
		return;

	_items[item].lines[line].chunks.clear();
	_items[item].lines[line].chunks.push_back(Chunk());

	_items[item].lines[line].chunks.back().type = kChunkTypeString;
	_items[item].lines[line].chunks.back().text = text;
}

void GCTFile::setText(uint item, const Common::String &text) {
	if (item >= _items.size())
		return;

	_items[item].selector = 0;

	_items[item].lines.resize(1);

	setText(item, 0, text);
}

void GCTFile::reset() {
	_currentItem = 0xFFFF;
	_currentText.clear();
}

Common::String GCTFile::getLineText(const Line &line) const {
	Common::String text;

	// Go over all chunks in this line
	for (Chunks::const_iterator c = line.chunks.begin(); c != line.chunks.end(); ++c) {
		// A chunk is either a direct string, or a reference to another item

		if        (c->type == kChunkTypeItem) {
			Common::List<Common::String> lines;

			getItemText(c->item, lines);
			if (lines.empty())
				continue;

			if (lines.size() > 1)
				warning("GCTFile::getLineText(): Referenced item has multiple lines");

			text += lines.front();
		} else if (c->type == kChunkTypeString)
			text += c->text;
	}

	return text;
}

void GCTFile::getItemText(uint item, Common::List<Common::String> &text) const {
	text.clear();

	if ((item >= _items.size()) || _items[item].lines.empty())
		return;

	uint16 line = _items[item].selector;

	// Draw all lines
	if (line == kSelectorAll) {
		for (Lines::const_iterator l = _items[item].lines.begin(); l != _items[item].lines.end(); ++l)
			text.push_back(getLineText(*l));

		return;
	}

	// Draw random line
	if (line == kSelectorRandom)
		line = _rnd->getRandomNumber(_items[item].lines.size() - 1);

	if (line >= _items[item].lines.size())
		return;

	text.push_back(getLineText(_items[item].lines[line]));
}

void GCTFile::setArea(int16 left, int16 top, int16 right, int16 bottom) {
	trashBuffer();

	_hasArea = false;

	const int16 width  = right  - left + 1;
	const int16 height = bottom - top  + 1;
	if ((width <= 0) || (height <= 0))
		return;

	_areaLeft   = left;
	_areaTop    = top;
	_areaRight  = right;
	_areaBottom = bottom;

	_hasArea = true;

	resizeBuffer(width, height);
}

bool GCTFile::clear(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	return restoreScreen(dest, left, top, right, bottom);
}

bool GCTFile::fill(Surface &dest, uint8 color, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	left   = _areaLeft;
	top    = _areaTop;
	right  = _areaRight;
	bottom = _areaBottom;

	if (!hasSavedBackground())
		saveScreen(dest, left, top, right, bottom);

	dest.fillRect(left, top, right, bottom, color);

	return true;
}

bool GCTFile::finished() const {
	return (_currentItem != 0xFFFF) && _currentText.empty();
}

bool GCTFile::draw(Surface &dest, uint16 item, const Font &font, uint8 color,
                   int16 &left, int16 &top, int16 &right, int16 &bottom) {

	if ((item >= _items.size()) || !_hasArea)
		return false;

	left   = _areaLeft;
	top    = _areaTop;
	right  = _areaRight;
	bottom = _areaBottom;

	const int16 width  = right  - left + 1;
	const int16 height = bottom - top  + 1;

	const uint lineCount = height / font.getCharHeight();
	if (lineCount == 0)
		return false;

	if (!hasSavedBackground())
		saveScreen(dest, left, top, right, bottom);

	if (item != _currentItem) {
		_currentItem = item;

		getItemText(_currentItem, _currentText);
	}

	if (_currentText.empty())
		return false;

	int16 y = top;
	for (uint i = 0; (i < lineCount) && !_currentText.empty(); i++, y += font.getCharHeight()) {
		const Common::String &line = _currentText.front();
		const int16 x = left + ((width - (line.size() * font.getCharWidth())) / 2);

		font.drawString(line, x, y, color, 0, true, dest);
		_currentText.pop_front();
	}

	return true;
}

} // End of namespace Gob
