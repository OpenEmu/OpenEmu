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

#ifndef LASTEXPRESS_SUBTITLE_H
#define LASTEXPRESS_SUBTITLE_H

/*
	Subtitle format (.SBE)

	uint16 {2}   - number of subtitles

	// for each subtitle
	    uint16 {2}   - display start time
	    uint16 {2}   - display stop time
	    uint16 {2}   - top line length
	    uint16 {2}   - bottom line length
	    byte {x}     - top line (UTF-16 string)
	    byte {x}     - bottom line (UTF-16 string)

	Subtitles seem to be drawn on screen at (80, 420) x (560, 458)
*/

#include "lastexpress/drawable.h"

#include "common/array.h"

namespace Common {
class SeekableReadStream;
}

namespace LastExpress {

class Font;
class Subtitle;

class SubtitleManager : public Drawable {
public:
	SubtitleManager(Font *font);
	~SubtitleManager();

	bool load(Common::SeekableReadStream *stream);
	uint16 getMaxTime() const;
	void setTime(uint16 time);
	bool hasChanged() const;
	Common::Rect draw(Graphics::Surface *surface);

private:
	Common::Array<Subtitle *> _subtitles;
	Font *_font;
	uint16 _maxTime;

	int16 _currentIndex;
	int16 _lastIndex;

	void reset();
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SUBTITLE_H
