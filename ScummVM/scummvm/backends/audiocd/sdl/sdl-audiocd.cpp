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

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "common/textconsole.h"
#include "backends/audiocd/sdl/sdl-audiocd.h"

SdlAudioCDManager::SdlAudioCDManager()
	:
	_cdrom(0),
	_cdTrack(0),
	_cdNumLoops(0),
	_cdStartFrame(0),
	_cdDuration(0),
	_cdEndTime(0),
	_cdStopTime(0) {

}

SdlAudioCDManager::~SdlAudioCDManager() {
	if (_cdrom) {
		SDL_CDStop(_cdrom);
		SDL_CDClose(_cdrom);
	}
}

bool SdlAudioCDManager::openCD(int drive) {
	if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
		_cdrom = NULL;
	else {
		_cdrom = SDL_CDOpen(drive);
		// Did it open? Check if _cdrom is NULL
		if (!_cdrom) {
			warning("Couldn't open drive: %s", SDL_GetError());
		} else {
			_cdNumLoops = 0;
			_cdStopTime = 0;
			_cdEndTime = 0;
		}
	}

	return (_cdrom != NULL);
}

void SdlAudioCDManager::stopCD() {
	// Stop CD Audio in 1/10th of a second
	_cdStopTime = SDL_GetTicks() + 100;
	_cdNumLoops = 0;
}

void SdlAudioCDManager::playCD(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	if (!_cdrom)
		return;

	if (duration > 0)
		duration += 5;

	_cdTrack = track;
	_cdNumLoops = num_loops;
	_cdStartFrame = start_frame;

	SDL_CDStatus(_cdrom);
	if (start_frame == 0 && duration == 0)
		SDL_CDPlayTracks(_cdrom, track, 0, 1, 0);
	else
		SDL_CDPlayTracks(_cdrom, track, start_frame, 0, duration);
	_cdDuration = duration;
	_cdStopTime = 0;
	_cdEndTime = SDL_GetTicks() + _cdrom->track[track].length * 1000 / CD_FPS;
}

bool SdlAudioCDManager::pollCD() const {
	if (!_cdrom)
		return false;

	return (_cdNumLoops != 0 && (SDL_GetTicks() < _cdEndTime || SDL_CDStatus(_cdrom) == CD_PLAYING));
}

void SdlAudioCDManager::updateCD() {
	if (!_cdrom)
		return;

	if (_cdStopTime != 0 && SDL_GetTicks() >= _cdStopTime) {
		SDL_CDStop(_cdrom);
		_cdNumLoops = 0;
		_cdStopTime = 0;
		return;
	}

	if (_cdNumLoops == 0 || SDL_GetTicks() < _cdEndTime)
		return;

	if (_cdNumLoops != 1 && SDL_CDStatus(_cdrom) != CD_STOPPED) {
		// Wait another second for it to be done
		_cdEndTime += 1000;
		return;
	}

	if (_cdNumLoops > 0)
		_cdNumLoops--;

	if (_cdNumLoops != 0) {
		if (_cdStartFrame == 0 && _cdDuration == 0)
			SDL_CDPlayTracks(_cdrom, _cdTrack, 0, 1, 0);
		else
			SDL_CDPlayTracks(_cdrom, _cdTrack, _cdStartFrame, 0, _cdDuration);
		_cdEndTime = SDL_GetTicks() + _cdrom->track[_cdTrack].length * 1000 / CD_FPS;
	}
}

#endif
