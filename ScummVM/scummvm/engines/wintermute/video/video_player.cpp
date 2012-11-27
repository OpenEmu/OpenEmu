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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */


#include "engines/wintermute/video/video_player.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
VideoPlayer::VideoPlayer(BaseGame *inGame) : BaseClass(inGame) {
	setDefaults();
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::setDefaults() {
	_playing = false;
	_videoEndTime = 0;
	_soundAvailable = false;
	_startTime = 0;
	_totalVideoTime = 0;
	_playPosX = _playPosY = 0;
	_playZoom = 0.0f;

	_filename = NULL;

	_slowRendering = false;

	_currentSubtitle = 0;
	_showSubtitle = false;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
VideoPlayer::~VideoPlayer() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::cleanup() {
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::initialize(const char *inFilename, const char *subtitleFile) {
	warning("VideoPlayer: %s %s - Not implemented yet", inFilename, subtitleFile);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::update() {
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::display() {
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::play(TVideoPlayback type, int x, int y, bool freezeMusic) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::stop() {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::isPlaying() {
	return _playing;
}

//////////////////////////////////////////////////////////////////////////
bool VideoPlayer::loadSubtitles(const char *filename, const char *subtitleFile) {
	return STATUS_OK;
}

} // end of namespace Wintermute
