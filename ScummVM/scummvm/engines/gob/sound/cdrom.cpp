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

#include "common/endian.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "backends/audiocd/audiocd.h"

#include "gob/gob.h"
#include "gob/sound/cdrom.h"
#include "gob/dataio.h"

namespace Gob {

CDROM::CDROM() {
	_cdPlaying = false;

	_LICbuffer = 0;
	for (int i = 0; i < 16; i++)
		_curTrack[i] = 0;
	_numTracks = 0;
	_trackStop = 0;
	_startTime = 0;
}

CDROM::~CDROM() {
	stop();
}

void CDROM::readLIC(Common::SeekableReadStream &stream) {
	uint16 version, startChunk, pos;

	freeLICBuffer();
	*_curTrack = 0;

	version = stream.readUint16LE();
	startChunk = stream.readUint16LE();
	_numTracks = stream.readUint16LE();

	if (version != 3)
		error("Unknown version %d while reading LIC", version);

	stream.seek(50);

	for (int i = 0; i < startChunk; i++) {
		pos = stream.readUint16LE();

		if (!pos)
			break;

		stream.skip(pos);
	}

	_LICbuffer = new byte[_numTracks * 22];
	stream.read(_LICbuffer, _numTracks * 22);
}

void CDROM::freeLICBuffer() {
	delete[] _LICbuffer;
	_LICbuffer = 0;
}

void CDROM::startTrack(const char *trackName) {
	if (!_LICbuffer)
		return;

	byte *matchPtr = getTrackBuffer(trackName);
	if (!matchPtr) {
		warning("CDROM: Track \"%s\" not found", trackName);
		return;
	}

	Common::strlcpy(_curTrack, trackName, 16);

	stopPlaying();
	_curTrackBuffer = matchPtr;

	while (getTrackPos() >= 0)
		;

	uint32 start = READ_LE_UINT32(matchPtr + 12);
	uint32 end   = READ_LE_UINT32(matchPtr + 16);

	play(start, end);

	_startTime = g_system->getMillis();
	_trackStop = _startTime + (end - start + 1 + 150) * 40 / 3;
}

void CDROM::play(uint32 from, uint32 to) {
	// play from sector [from] to sector [to]
	//
	// format is HSG:
	// HSG encodes frame information into a double word:
	// minute multiplied by 4500, plus second multiplied by 75,
	// plus frame, minus 150
	g_system->getAudioCDManager()->play(1, 1, from, to - from + 1);
	_cdPlaying = true;
}

bool CDROM::isPlaying() const {
	return _cdPlaying;
}

int32 CDROM::getTrackPos(const char *keyTrack) const {
	byte *keyBuffer = getTrackBuffer(keyTrack);
	uint32 curPos = (g_system->getMillis() - _startTime) * 3 / 40;

	if (_cdPlaying && (g_system->getMillis() < _trackStop)) {
		if (keyBuffer && _curTrackBuffer && (keyBuffer != _curTrackBuffer)) {
			uint32 kStart = READ_LE_UINT32(keyBuffer + 12);
			uint32 kEnd = READ_LE_UINT32(keyBuffer + 16);
			uint32 cStart = READ_LE_UINT32(_curTrackBuffer + 12);
			uint32 cEnd = READ_LE_UINT32(_curTrackBuffer + 16);

			if ((kStart >= cStart) && (kEnd <= cEnd)) {
				if ((kStart - cStart) > curPos)
					return -2;
				if ((kEnd - cStart) < curPos)
					return -1;
			}
		}

		return curPos;
	} else
		return -1;
}

const char *CDROM::getCurTrack() const {
	return _curTrack;
}

void CDROM::stopPlaying() {
	stop();

	while (getTrackPos() != -1)
		;
}

void CDROM::stop() {
	_curTrackBuffer = 0;
	g_system->getAudioCDManager()->stop();
	_cdPlaying = false;
}

void CDROM::testCD(int trySubst, const char *label) {
	if (!trySubst) {
		error("CDROM track substitution is not supported");
		return;
	}

	_LICbuffer = 0;
	_cdPlaying = false;

	// Original checked CD label here
	// but will skip it as it will require OSystem extensions of direct
	// CD secor reading
}

byte *CDROM::getTrackBuffer(const char *trackName) const {
	if (!_LICbuffer || !trackName)
		return 0;

	byte *matchPtr = 0;
	byte *curPtr = _LICbuffer;

	for (int i = 0; i < _numTracks; i++) {
		if (!scumm_stricmp((char *)curPtr, trackName)) {
			matchPtr = curPtr;
			break;
		}
		curPtr += 22;
	}

	return matchPtr;
}

} // End of namespace Gob
