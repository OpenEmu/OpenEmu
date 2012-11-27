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

#include "common/stream.h"

#include "gob/draw.h"

#include "gob/pregob/txtfile.h"

namespace Gob {

TXTFile::TXTFile(Common::SeekableReadStream &txt, Format format) {
	load(txt, format);
}

TXTFile::~TXTFile() {
}

TXTFile::LineArray &TXTFile::getLines() {
	return _lines;
}

void TXTFile::load(Common::SeekableReadStream &txt, Format format) {
	if (format == kFormatStringPositionColorFont) {
		int numLines = getInt(txt);

		_lines.reserve(numLines);
	}

	while (!txt.eos()) {
		Line line;

		line.text  =                                              getStr(txt);
		line.x     = (format >= kFormatStringPosition)          ? getInt(txt) : 0;
		line.y     = (format >= kFormatStringPosition)          ? getInt(txt) : 0;
		line.color = (format >= kFormatStringPositionColor)     ? getInt(txt) : 0;
		line.font  = (format >= kFormatStringPositionColorFont) ? getInt(txt) : 0;

		_lines.push_back(line);
	}

	while (!_lines.empty() && _lines.back().text.empty())
		_lines.pop_back();
}

bool TXTFile::draw(Surface &surface, int16 &left, int16 &top, int16 &right, int16 &bottom,
                   const Font * const *fonts, uint fontCount, int color) {

	trashBuffer();

	if (!getArea(left, top, right, bottom, fonts, fontCount))
		return false;

	resizeBuffer(right - left + 1, bottom - top + 1);
	saveScreen(surface, left, top, right, bottom);

	for (LineArray::const_iterator l = _lines.begin(); l != _lines.end(); ++l) {
		if (l->font >= fontCount)
			continue;

		fonts[l->font]->drawString(l->text, l->x, l->y, (color < 0) ? l->color : color, 0, true, surface);
	}

	return true;
}

bool TXTFile::draw(uint line, Surface &surface, int16 &left, int16 &top, int16 &right, int16 &bottom,
                   const Font * const *fonts, uint fontCount, int color) {

	trashBuffer();

	if (!getArea(line, left, top, right, bottom, fonts, fontCount))
		return false;

	resizeBuffer(right - left + 1, bottom - top + 1);
	saveScreen(surface, left, top, right, bottom);

	const Line &l = _lines[line];

	fonts[l.font]->drawString(l.text, l.x, l.y, (color < 0) ? l.color : color, 0, true, surface);

	return true;
}

bool TXTFile::draw(Surface &surface, const Font * const *fonts, uint fontCount, int color) {
	int16 left, top, right, bottom;

	return draw(surface, left, top, right, bottom, fonts, fontCount, color);
}

bool TXTFile::draw(uint line, Surface &surface, const Font * const *fonts, uint fontCount, int color) {
	int16 left, top, right, bottom;

	return draw(line, surface, left, top, right, bottom, fonts, fontCount, color);
}

bool TXTFile::clear(Surface &surface, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	return restoreScreen(surface, left, top, right, bottom);
}

bool TXTFile::getArea(int16 &left, int16 &top, int16 &right, int16 &bottom,
                      const Font * const *fonts, uint fontCount) const {

	bool hasLine = false;

	left   = 0x7FFF;
	top    = 0x7FFF;
	right  = 0x0000;
	bottom = 0x0000;

	for (uint i = 0; i < _lines.size(); i++) {
		int16 lLeft, lTop, lRight, lBottom;

		if (getArea(i, lLeft, lTop, lRight, lBottom, fonts, fontCount)) {
			left   = MIN(left  , lLeft  );
			top    = MIN(top   , lTop   );
			right  = MAX(right , lRight );
			bottom = MAX(bottom, lBottom);

			hasLine = true;
		}
	}

	return hasLine;
}

bool TXTFile::getArea(uint line, int16 &left, int16 &top, int16 &right, int16 &bottom,
                      const Font * const *fonts, uint fontCount) const {


	if ((line >= _lines.size()) || (_lines[line].font >= fontCount))
		return false;

	const Line &l = _lines[line];

	left   = l.x;
	top    = l.y;
	right  = l.x + l.text.size() * fonts[l.font]->getCharWidth()  - 1;
	bottom = l.y +                 fonts[l.font]->getCharHeight() - 1;

	return true;
}

Common::String TXTFile::getStr(Common::SeekableReadStream &txt) {
	// Skip all ' ', '\n' and '\r'
	while (!txt.eos()) {
		char c = txt.readByte();
		if (txt.eos())
			break;

		if ((c != ' ') && (c != '\n') && (c != '\r')) {
			txt.seek(-1, SEEK_CUR);
			break;
		}
	}

	if (txt.eos())
		return "";

	// Read string until ' ', '\n' or '\r'
	Common::String string;
	while (!txt.eos()) {
		char c = txt.readByte();
		if ((c == ' ') || (c == '\n') || (c == '\r'))
			break;

		string += c;
	}

	// Replace all '#' with ' ' and throw out non-printables
	Common::String cleanString;

	for (uint i = 0; i < string.size(); i++) {
		if      (string[i] == '#')
			cleanString += ' ';
		else if ((unsigned char)string[i] >= ' ')
			cleanString += string[i];
	}

	return cleanString;
}

int TXTFile::getInt(Common::SeekableReadStream &txt) {
	// Skip all [^-0-9]
	while (!txt.eos()) {
		char c = txt.readByte();
		if (txt.eos())
			break;

		if ((c == '-') || ((c >= '0') && (c <= '9'))) {
			txt.seek(-1, SEEK_CUR);
			break;
		}
	}

	if (txt.eos())
		return 0;

	// Read until [^-0-9]
	Common::String string;
	while (!txt.eos()) {
		char c = txt.readByte();
		if ((c != '-') && ((c < '0') || (c > '9')))
			break;

		string += c;
	}

	// Convert to integer
	return atoi(string.c_str());
}

} // End of namespace Gob
