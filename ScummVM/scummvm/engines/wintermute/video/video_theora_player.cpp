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


#include "engines/wintermute/video/video_theora_player.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/osystem/base_surface_osystem.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/platform_osystem.h"
#include "video/theora_decoder.h"
#include "engines/wintermute/wintermute.h"
#include "common/system.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(VideoTheoraPlayer, false)

//////////////////////////////////////////////////////////////////////////
VideoTheoraPlayer::VideoTheoraPlayer(BaseGame *inGame) : BaseClass(inGame) {
	SetDefaults();
}

//////////////////////////////////////////////////////////////////////////
void VideoTheoraPlayer::SetDefaults() {

	_file = NULL;
	_filename = "";
	_startTime = 0;
	_looping = false;

	_freezeGame = false;
	_currentTime = 0;

	_state = THEORA_STATE_NONE;

	_videoFrameReady = false;
	_audioFrameReady = false;
	_videobufTime = 0;

	_playbackStarted = false;
	_dontDropFrames = false;

	_texture = NULL;
	_alphaImage = NULL;
	_alphaFilename = "";

	_frameRendered = false;

	_seekingKeyframe = false;
	_timeOffset = 0.0f;

	_posX = _posY = 0;
	_playbackType = VID_PLAY_CENTER;
	_playZoom = 0.0f;

	_savedState = THEORA_STATE_NONE;
	_savedPos = 0;
	_volume = 100;
	_theoraDecoder = NULL;

	// TODO: Add subtitles-support
	//_subtitler = NULL;
}

//////////////////////////////////////////////////////////////////////////
VideoTheoraPlayer::~VideoTheoraPlayer(void) {
	cleanup();
//	SAFE_DELETE(_subtitler);
}

//////////////////////////////////////////////////////////////////////////
void VideoTheoraPlayer::cleanup() {
	if (_file) {
		BaseFileManager::getEngineInstance()->closeFile(_file);
		_file = NULL;
	}

	_surface.free();
	if (_theoraDecoder) {
		_theoraDecoder->close();
	}
	delete _theoraDecoder;
	_theoraDecoder = NULL;
	delete _alphaImage;
	_alphaImage = NULL;
	delete _texture;
	_texture = NULL;
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::initialize(const Common::String &filename, const Common::String &subtitleFile) {
	cleanup();

	_filename = filename;
	_file = BaseFileManager::getEngineInstance()->openFile(filename, true, false);
	if (!_file) {
		return STATUS_FAILED;
	}

#if defined (USE_THEORADEC)
	_theoraDecoder = new Video::TheoraDecoder();
#else
	return STATUS_FAILED;
#endif
	_theoraDecoder->loadStream(_file);

	if (!_theoraDecoder->isVideoLoaded()) {
		return STATUS_FAILED;
	}

	_state = THEORA_STATE_PAUSED;

	// Additional setup.
	_surface.create(_theoraDecoder->getWidth(), _theoraDecoder->getHeight(), _theoraDecoder->getPixelFormat());
	_texture = new BaseSurfaceOSystem(_gameRef);
	_texture->create(_theoraDecoder->getWidth(), _theoraDecoder->getHeight());
	_state = THEORA_STATE_PLAYING;
	_playZoom = 100;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::resetStream() {
	warning("VidTheoraPlayer::resetStream - hacked");
	// HACK: Just reopen the same file again.
	if (_theoraDecoder) {
		_theoraDecoder->close();
	}
	delete _theoraDecoder;
	_theoraDecoder = NULL;

	_file = BaseFileManager::getEngineInstance()->openFile(_filename, true, false);
	if (!_file) {
		return STATUS_FAILED;
	}
	
#if defined (USE_THEORADEC)
	_theoraDecoder = new Video::TheoraDecoder();
#else
	return STATUS_FAILED;
#endif
	_theoraDecoder->loadStream(_file);
	
	if (!_theoraDecoder->isVideoLoaded()) {
		return STATUS_FAILED;
	}

	return play(_playbackType, _posX, _posY, false, false, _looping, 0, _playZoom);
	// End of hack.
#if 0 // Stubbed for now, as theora isn't seekable
	if (_sound) {
		_sound->Stop();
	}

	m_TimeOffset = 0.0f;
	Initialize(m_Filename);
	Play(m_PlaybackType, m_PosX, m_PosY, false, false, m_Looping, 0, m_PlayZoom);
#endif
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::play(TVideoPlayback type, int x, int y, bool freezeGame, bool freezeMusic, bool looping, uint32 startTime, float forceZoom, int volume) {
	if (forceZoom < 0.0f) {
		forceZoom = 100.0f;
	}
	if (volume < 0) {
		_volume = _gameRef->_soundMgr->getVolumePercent(Audio::Mixer::kSFXSoundType);
	} else {
		_volume = volume;
	}

	_freezeGame = freezeGame;

	if (!_playbackStarted && _freezeGame) {
		_gameRef->freeze(freezeMusic);
	}

	_playbackStarted = false;
	float width, height;
	if (_theoraDecoder) {
		_surface.free();
		_surface.copyFrom(*_theoraDecoder->decodeNextFrame());
		_state = THEORA_STATE_PLAYING;
		_looping = looping;
		_playbackType = type;

		_startTime = startTime;
		_volume = volume;
		_posX = x;
		_posY = y;
		_playZoom = forceZoom;

		width = (float)_theoraDecoder->getWidth();
		height = (float)_theoraDecoder->getHeight();
	} else {
		width = (float)_gameRef->_renderer->_width;
		height = (float)_gameRef->_renderer->_height;
	}

	switch (type) {
	case VID_PLAY_POS:
		_playZoom = forceZoom;
		_posX = x;
		_posY = y;
		break;

	case VID_PLAY_STRETCH: {
		float zoomX = (float)((float)_gameRef->_renderer->_width / width * 100);
		float zoomY = (float)((float)_gameRef->_renderer->_height / height * 100);
		_playZoom = MIN(zoomX, zoomY);
		_posX = (int)((_gameRef->_renderer->_width - width * (_playZoom / 100)) / 2);
		_posY = (int)((_gameRef->_renderer->_height - height * (_playZoom / 100)) / 2);
	}
	break;

	case VID_PLAY_CENTER:
		_playZoom = 100.0f;
		_posX = (int)((_gameRef->_renderer->_width - width) / 2);
		_posY = (int)((_gameRef->_renderer->_height - height) / 2);
		break;
	}
	_theoraDecoder->start();

	return STATUS_OK;
#if 0 // Stubbed for now as theora isn't seekable
	if (StartTime) SeekToTime(StartTime);

	Update();
#endif
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::stop() {
	_theoraDecoder->close();
	_state = THEORA_STATE_FINISHED;
	if (_freezeGame) {
		_gameRef->unfreeze();
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::update() {
	_currentTime = _freezeGame ? _gameRef->_liveTimer : _gameRef->_timer;

	if (!isPlaying()) {
		return STATUS_OK;
	}

	if (_playbackStarted /*&& m_Sound && !m_Sound->IsPlaying()*/) {
		return STATUS_OK;
	}

	if (_playbackStarted && !_freezeGame && _gameRef->_state == GAME_FROZEN) {
		return STATUS_OK;
	}

	if (_theoraDecoder) {
		if (_theoraDecoder->endOfVideo() && _looping) {
			warning("Should loop movie %s, hacked for now", _filename.c_str());
			_theoraDecoder->rewind();
			//HACK: Just reinitialize the same video again:
			return resetStream();
		} else if (_theoraDecoder->endOfVideo() && !_looping) {
			debugC(kWintermuteDebugLog, "Finished movie %s", _filename.c_str());
			_state = THEORA_STATE_FINISHED;
			_playbackStarted = false;
			if (_freezeGame) {
				_gameRef->unfreeze();
			}
		}
		if (_state == THEORA_STATE_PLAYING) {
			if (!_theoraDecoder->endOfVideo() && _theoraDecoder->getTimeToNextFrame() == 0) {
				const Graphics::Surface *decodedFrame = _theoraDecoder->decodeNextFrame();
				if (decodedFrame) {
					_surface.free();
					_surface.copyFrom(*decodedFrame);
					if (_texture) {
						writeVideo();
					}
				}
			}
			return STATUS_OK;
		}
	}
	// Skip the busy-loop?
	if ((!_texture || !_videoFrameReady) && !_theoraDecoder->endOfVideo()) {
		// end playback
		if (!_looping) {
			_state = THEORA_STATE_FINISHED;
			if (_freezeGame) {
				_gameRef->unfreeze();
			}
			return STATUS_OK;
		} else {
			resetStream();
			return STATUS_OK;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
uint32 VideoTheoraPlayer::getMovieTime() {
	if (!_playbackStarted) {
		return 0;
	} else {
		return _theoraDecoder->getTime();
	}
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::writeVideo() {
	if (!_texture) {
		return STATUS_FAILED;
	}

	_texture->startPixelOp();

	writeAlpha();
	if (_alphaImage) {
		_texture->putSurface(_surface, true);
	} else {
		_texture->putSurface(_surface, false);
	}

	//RenderFrame(_texture, &yuv);

	_texture->endPixelOp();
	_videoFrameReady = true;
	return STATUS_OK;
}

void VideoTheoraPlayer::writeAlpha() {
	if (_alphaImage && _surface.w == _alphaImage->getSurface()->w && _surface.h == _alphaImage->getSurface()->h) {
		assert(_alphaImage->getSurface()->format.bytesPerPixel == 4);
		assert(_surface.format.bytesPerPixel == 4);
		const byte *alphaData = (const byte *)_alphaImage->getSurface()->getBasePtr(0, 0);
#ifdef SCUMM_LITTLE_ENDIAN
		int alphaPlace = (_alphaImage->getSurface()->format.aShift / 8);
#else
		int alphaPlace = 3 - (_alphaImage->getSurface()->format.aShift / 8);
#endif
		alphaData += alphaPlace;
		byte *imgData = (byte *)_surface.getBasePtr(0, 0);
#ifdef SCUMM_LITTLE_ENDIAN
		imgData += (_surface.format.aShift / 8);
#else
		imgData += 3 - (_surface.format.aShift / 8);
#endif
		for (int i = 0; i < _surface.w * _surface.h; i++) {
			*imgData = *alphaData;
			alphaData += 4;
			imgData += 4;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::display(uint32 alpha) {
	Rect32 rc;
	bool res;

	if (_texture && _videoFrameReady) {
		BasePlatform::setRect(&rc, 0, 0, _texture->getWidth(), _texture->getHeight());
		if (_playZoom == 100.0f) {
			res = _texture->displayTrans(_posX, _posY, rc, alpha);
		} else {
			res = _texture->displayTransZoom(_posX, _posY, rc, _playZoom, _playZoom, alpha);
		}
	} else {
		res = STATUS_FAILED;
	}
	// TODO: Add subtitles-support
/*	if (m_Subtitler && _gameRef->m_VideoSubtitles) {
		m_Subtitler->display();
	}*/

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::setAlphaImage(const Common::String &filename) {
	delete _alphaImage;
	_alphaImage = new BaseImage();
	if (!_alphaImage || DID_FAIL(_alphaImage->loadFile(filename))) {
		delete _alphaImage;
		_alphaImage = NULL;
		_alphaFilename = "";
		return STATUS_FAILED;
	}

	if (_alphaFilename != filename) {
		_alphaFilename = filename;
	}
	//TODO: Conversion.
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
byte VideoTheoraPlayer::getAlphaAt(int x, int y) {
	if (_alphaImage) {
		return _alphaImage->getAlphaAt(x, y);
	} else {
		return 0xFF;
	}
}


//////////////////////////////////////////////////////////////////////////
inline int intlog(int num) {
	int r = 0;
	while (num > 0) {
		num = num / 2;
		r = r + 1;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::seekToTime(uint32 time) {
	warning("VideoTheoraPlayer::SeekToTime(%d) - not supported", time);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::pause() {
	if (_state == THEORA_STATE_PLAYING) {
		_state = THEORA_STATE_PAUSED;
		_theoraDecoder->pauseVideo(true);
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::resume() {
	if (_state == THEORA_STATE_PAUSED) {
		_state = THEORA_STATE_PLAYING;
		_theoraDecoder->pauseVideo(false);
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::persist(BasePersistenceManager *persistMgr) {
	//BaseClass::persist(persistMgr);

	if (persistMgr->getIsSaving()) {
		_savedPos = getMovieTime() * 1000;
		_savedState = _state;
	} else {
		SetDefaults();
	}

	persistMgr->transfer(TMEMBER(_gameRef));
	persistMgr->transfer(TMEMBER(_savedPos));
	persistMgr->transfer(TMEMBER(_savedState));
	persistMgr->transfer(TMEMBER(_filename));
	persistMgr->transfer(TMEMBER(_alphaFilename));
	persistMgr->transfer(TMEMBER(_posX));
	persistMgr->transfer(TMEMBER(_posY));
	persistMgr->transfer(TMEMBER(_playZoom));
	persistMgr->transfer(TMEMBER_INT(_playbackType));
	persistMgr->transfer(TMEMBER(_looping));
	persistMgr->transfer(TMEMBER(_volume));

	if (!persistMgr->getIsSaving() && (_savedState != THEORA_STATE_NONE)) {
		initializeSimple();
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool VideoTheoraPlayer::initializeSimple() {
	if (DID_SUCCEED(initialize(_filename))) {
		if (_alphaFilename != "") {
			setAlphaImage(_alphaFilename);
		}
		play(_playbackType, _posX, _posY, false, false, _looping, _savedPos, _playZoom);
	} else {
		_state = THEORA_STATE_FINISHED;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
BaseSurface *VideoTheoraPlayer::getTexture() {
	return _texture;
}

} // end of namespace Wintermute
