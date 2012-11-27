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

// Based on the Xentax Wiki documentation:
// http://wiki.xentax.com/index.php/The_Last_Express_SBE

#include "lastexpress/data/subtitle.h"

#include "lastexpress/data/font.h"

#include "lastexpress/debug.h"

#include "common/debug.h"
#include "common/rect.h"
#include "common/stream.h"

namespace LastExpress {

//////////////////////////////////////////////////////////////////////////
// Subtitle
//////////////////////////////////////////////////////////////////////////
class Subtitle {
public:
	Subtitle() : _timeStart(0), _timeStop(0), _topLength(0), _topText(NULL),
		_bottomLength(0), _bottomText(NULL) {}
	~Subtitle() { reset(); }

	bool load(Common::SeekableReadStream *in);
	Common::Rect draw(Graphics::Surface *surface, Font *font);

	uint16 getTimeStart() const { return _timeStart; }
	uint16 getTimeStop() const { return _timeStop; }

private:
	uint16 _timeStart;    ///< display start time
	uint16 _timeStop;     ///< display stop time

	uint16 _topLength;    ///< top line length
	uint16 *_topText;     ///< bottom line length

	uint16 _bottomLength; ///< top line (UTF-16 string)
	uint16 *_bottomText;  ///< bottom line (UTF-16 string)

	void reset();
};

void Subtitle::reset() {
	delete[] _topText;
	delete[] _bottomText;
	_topText = NULL;
	_bottomText = NULL;
}

template<typename T>
T *newArray(size_t n) {
	if (n <= (size_t)-1 / sizeof(T))
		return new T[n];

	// n is too large
	return NULL;
}

bool Subtitle::load(Common::SeekableReadStream *in) {
	reset();

	if (!in)
		return false;

	// Read the display times
	_timeStart = in->readUint16LE();
	_timeStop = in->readUint16LE();

	// Read the text lengths
	_topLength = in->readUint16LE();
	_bottomLength = in->readUint16LE();

	// Create the buffers
	if (_topLength) {
		_topText = newArray<uint16>(_topLength + 1);
		if (!_topText)
			return false;

		_topText[_topLength] = 0;
	}
	if (_bottomLength) {
		_bottomText = newArray<uint16>(_bottomLength + 1);
		if (!_bottomText)
			return false;

		_bottomText[_bottomLength] = 0;
	}

	// Read the texts
	for (int i = 0; i < _topLength; i++)
		_topText[i] = in->readUint16LE();
	for (int i = 0; i < _bottomLength; i++)
		_bottomText[i] = in->readUint16LE();

	debugC(9, kLastExpressDebugSubtitle, "  %d -> %d:", _timeStart, _timeStop);
	if (_topLength)
		debugC(9, kLastExpressDebugSubtitle, "\t%ls", (wchar_t *)_topText);
	if (_bottomLength)
		debugC(9, kLastExpressDebugSubtitle, "\t%ls", (wchar_t *)_bottomText);

	return true;
}

Common::Rect Subtitle::draw(Graphics::Surface *surface, Font *font) {
	Common::Rect rectTop, rectBottom;

	//FIXME find out proper subtitles coordinates (and hope it's hardcoded and not stored in the sequence or animation)
	rectTop = font->drawString(surface, 100, 100, _topText, _topLength);
	rectBottom = font->drawString(surface, 100, 300, _bottomText, _bottomLength);

	rectTop.extend(rectBottom);

	return rectTop;
}


//////////////////////////////////////////////////////////////////////////
// SubtitleManager
//////////////////////////////////////////////////////////////////////////
SubtitleManager::SubtitleManager(Font *font) : _font(font), _maxTime(0), _currentIndex(-1), _lastIndex(-1) {}

SubtitleManager::~SubtitleManager() {
	reset();

	// Zero passed pointers
	_font = NULL;
}

void SubtitleManager::reset() {
	for (uint i = 0; i < _subtitles.size(); i++)
		delete _subtitles[i];

	_subtitles.clear();
	_currentIndex = -1;
	_lastIndex = -1;
}

bool SubtitleManager::load(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	reset();

	// Read header to get the number of subtitles
	uint32 numSubtitles = stream->readUint16LE();
	if (stream->eos())
		error("[SubtitleManager::load] Cannot read from subtitle file");

	debugC(3, kLastExpressDebugSubtitle, "Number of subtitles in file: %d", numSubtitles);

	// TODO: Check that stream contain enough data
	//if (stream->size() < (signed)(numSubtitles * sizeof(SubtitleData))) {
		//debugC(2, kLastExpressDebugSubtitle, "Subtitle file does not contain valid data");
		//return false;
	//}

	// Read the list of subtitles
	_maxTime = 0;
	for (uint i = 0; i < numSubtitles; i++) {
		Subtitle *subtitle = new Subtitle();
		if (!subtitle->load(stream)) {
			// Failed to read this line
			reset();

			delete subtitle;

			return false;
		}

		// Update the max time
		if (subtitle->getTimeStop() > _maxTime)
			_maxTime = subtitle->getTimeStop();

		_subtitles.push_back(subtitle);
	}

	delete stream;

	return true;
}

uint16 SubtitleManager::getMaxTime() const {
	return _maxTime;
}

void SubtitleManager::setTime(uint16 time) {
	_currentIndex = -1;

	// Find the appropriate line to show
	for (uint i = 0; i < _subtitles.size(); i++) {
		if ((time >= _subtitles[i]->getTimeStart()) && (time <= _subtitles[i]->getTimeStop())) {
			// Keep the index of the line to show
			_currentIndex = (int16)i;
			return;
		}
	}
}

bool SubtitleManager::hasChanged() const {
	// TODO: mark the old line rect as dirty
	if (_currentIndex != _lastIndex)
		return true;
	else
		return false;
}

Common::Rect SubtitleManager::draw(Graphics::Surface *surface) {
	// Update the last drawn index
	_lastIndex = _currentIndex;

	// Return if we don't have to draw any line
	if (_currentIndex == -1)
		return Common::Rect();

	// Draw the current line
	assert(_currentIndex >= 0 && _currentIndex < (int16)_subtitles.size());
	return _subtitles[(uint16)_currentIndex]->draw(surface, _font);
}

} // End of namespace LastExpress
