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

#ifndef WINTERMUTE_VIDPLAYER_H
#define WINTERMUTE_VIDPLAYER_H

#include "engines/wintermute/dctypes.h"    // Added by ClassView
#include "engines/wintermute/base/base.h"

#define MAX_AUDIO_STREAMS 5
#define MAX_VIDEO_STREAMS 5


namespace Wintermute {

// AVI-Video-player, currently fully stubbed
class VideoPlayer : public BaseClass {
public:
	bool _showSubtitle;
	int _currentSubtitle;
	bool loadSubtitles(const char *filename, const char *subtitleFile);
	bool _slowRendering;
	bool isPlaying();
	char *_filename;
	bool stop();
	bool play(TVideoPlayback Type = VID_PLAY_CENTER, int x = 0, int y = 0, bool freezeMusic = true);
	uint32 _totalVideoTime;
	uint32 _startTime;
	//CVidRenderer *_vidRenderer;
	//BaseSoundAVI *_sound;
	bool _soundAvailable;
	bool setDefaults();
	bool _playing;
	bool display();
	bool update();
	bool initialize(const char *inFilename, const char *subtitleFile = NULL);
	bool cleanup();
	VideoPlayer(BaseGame *inGame);
	virtual ~VideoPlayer();

	/*PAVIFILE _aviFile;

	LONG _lastSample;

	PAVISTREAM _audioStream;
	PAVISTREAM _videoStream;

	LPWAVEFORMAT _audioFormat;

	LPBITMAPINFO _videoFormat;
	PGETFRAME _videoPGF;*/
	uint32 _videoEndTime;

	int _playPosX;
	int _playPosY;
	float _playZoom;

	/*  LPBITMAPV4HEADER _targetFormat;

	    BaseArray<CVidSubtitle *, CVidSubtitle *> _subtitles;*/
};

} // end of namespace Wintermute

#endif
