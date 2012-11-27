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

#ifdef ENABLE_AGOS2

#ifndef AGOS_ANIMATION_H
#define AGOS_ANIMATION_H

#include "video/dxa_decoder.h"
#include "video/smk_decoder.h"
#include "audio/mixer.h"

namespace AGOS {

class AGOSEngine_Feeble;

class MoviePlayer {
protected:
	AGOSEngine_Feeble *_vm;

	Audio::Mixer *_mixer;

	Audio::SoundHandle _bgSound;
	Audio::AudioStream *_bgSoundStream;

	bool _leftButtonDown;
	bool _rightButtonDown;
	bool _skipMovie;
	uint32 _ticks;

	char baseName[40];
public:
	enum VideoFlags {
		TYPE_OMNITV  = 1,
		TYPE_LOOPING = 2
	};

	MoviePlayer(AGOSEngine_Feeble *vm);
	virtual ~MoviePlayer();

	virtual bool load() = 0;
	virtual void play();
	virtual void playVideo() = 0;
	virtual void nextFrame() = 0;
	virtual void stopVideo() = 0;

protected:
	virtual void handleNextFrame();
	virtual bool processFrame() = 0;
	virtual void startSound() {}
};

class MoviePlayerDXA : public MoviePlayer, Video::DXADecoder {
	static const char *const _sequenceList[90];
	uint8 _sequenceNum;
public:
	MoviePlayerDXA(AGOSEngine_Feeble *vm, const char *name);

	bool load();
	void playVideo();
	void nextFrame();
	virtual void stopVideo();

protected:
	void readSoundData(Common::SeekableReadStream *stream);

private:
	void handleNextFrame();
	bool processFrame();
	void startSound();
	void copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch);
};

class MoviePlayerSMK : public MoviePlayer, Video::SmackerDecoder {
public:
	MoviePlayerSMK(AGOSEngine_Feeble *vm, const char *name);

	bool load();
	void playVideo();
	void nextFrame();
	virtual void stopVideo();

private:
	void handleNextFrame();
	bool processFrame();
	void startSound();
	void copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch);
};

MoviePlayer *makeMoviePlayer(AGOSEngine_Feeble *vm, const char *name);

} // End of namespace AGOS

#endif

#endif // ENABLE_AGOS2
