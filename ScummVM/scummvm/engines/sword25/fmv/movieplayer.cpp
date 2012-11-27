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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "sword25/sword25.h"	// for kDebugScript
#include "sword25/fmv/movieplayer.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/panel.h"
#include "sword25/kernel/kernel.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */

#ifdef USE_THEORADEC
MoviePlayer::MoviePlayer(Kernel *pKernel) : Service(pKernel), _decoder() {
	if (!registerScriptBindings())
		error("Script bindings could not be registered.");
	else
		debugC(kDebugScript, "Script bindings registered.");
}

MoviePlayer::~MoviePlayer() {
	_decoder.close();
}

bool MoviePlayer::loadMovie(const Common::String &filename, uint z) {
	// Get the file and load it into the decoder
	Common::SeekableReadStream *in = Kernel::getInstance()->getPackage()->getStream(filename);
	_decoder.loadStream(in);
	_decoder.start();

	GraphicEngine *pGfx = Kernel::getInstance()->getGfx();

#ifdef THEORA_INDIRECT_RENDERING
	_outputBitmap = pGfx->getMainPanel()->addDynamicBitmap(_decoder.getWidth(), _decoder.getHeight());
	if (!_outputBitmap.isValid()) {
		error("Output bitmap for movie playback could not be created.");
		return false;
	}

	// Compute the scaling of the output bitmap, so that it takes up the most space
	float screenToVideoWidth = (float)pGfx->getDisplayWidth() / (float)_outputBitmap->getWidth();
	float screenToVideoHeight = (float)pGfx->getDisplayHeight() / (float)_outputBitmap->getHeight();
	float scaleFactor = MIN(screenToVideoWidth, screenToVideoHeight);

	if (abs((int)(scaleFactor - 1.0f)) < FLT_EPSILON)
		scaleFactor = 1.0f;

	_outputBitmap->setScaleFactor(scaleFactor);
	_outputBitmap->setZ(z);

	// Center bitmap on screen
	_outputBitmap->setX((pGfx->getDisplayWidth() - _outputBitmap->getWidth()) / 2);
	_outputBitmap->setY((pGfx->getDisplayHeight() - _outputBitmap->getHeight()) / 2);
#else
	_backSurface = pGfx->getSurface();

	_outX = (pGfx->getDisplayWidth() - _decoder.getWidth()) / 2;
	_outY = (pGfx->getDisplayHeight() - _decoder.getHeight()) / 2;

	if (_outX < 0)
		_outX = 0;
	if (_outY < 0)
		_outY = 0;
#endif

	return true;
}

bool MoviePlayer::unloadMovie() {
	_decoder.close();
	_outputBitmap.erase();

	return true;
}

bool MoviePlayer::play() {
	_decoder.pauseVideo(false);
	return true;
}

bool MoviePlayer::pause() {
	_decoder.pauseVideo(true);
	return true;
}

void MoviePlayer::update() {
	if (_decoder.isVideoLoaded()) {
		if (_decoder.endOfVideo()) {
			// Movie complete, so unload the movie
			unloadMovie();
		} else {
			const Graphics::Surface *s = _decoder.decodeNextFrame();
			if (s) {
				// Transfer the next frame
				assert(s->format.bytesPerPixel == 4);

#ifdef THEORA_INDIRECT_RENDERING
				const byte *frameData = (const byte *)s->getBasePtr(0, 0);
				_outputBitmap->setContent(frameData, s->pitch * s->h, 0, s->pitch);
#else
				g_system->copyRectToScreen(s->getBasePtr(0, 0), s->pitch, _outX, _outY, MIN(s->w, _backSurface->w), MIN(s->h, _backSurface->h));
				g_system->updateScreen();
#endif
			}
		}
	}
}

bool MoviePlayer::isMovieLoaded() {
	return _decoder.isVideoLoaded();
}

bool MoviePlayer::isPaused() {
	return _decoder.isPaused() || _decoder.endOfVideo();
}

float MoviePlayer::getScaleFactor() {
	if (_decoder.isVideoLoaded())
		return _outputBitmap->getScaleFactorX();
	else
		return 0;
}

void MoviePlayer::setScaleFactor(float scaleFactor) {
	if (_decoder.isVideoLoaded()) {
		_outputBitmap->setScaleFactor(scaleFactor);

		// Ausgabebitmap auf dem Bildschirm zentrieren
		GraphicEngine *gfxPtr = Kernel::getInstance()->getGfx();
		_outputBitmap->setX((gfxPtr->getDisplayWidth() - _outputBitmap->getWidth()) / 2);
		_outputBitmap->setY((gfxPtr->getDisplayHeight() - _outputBitmap->getHeight()) / 2);
	}
}

double MoviePlayer::getTime() {
	return _decoder.getTime() / 1000.0;
}

#else // USE_THEORADEC

MoviePlayer::MoviePlayer(Kernel *pKernel) : Service(pKernel) {
	if (!registerScriptBindings())
		error("Script bindings could not be registered.");
	else
		debugC(kDebugScript, "Script bindings registered.");
}

MoviePlayer::~MoviePlayer() {
}

bool MoviePlayer::loadMovie(const Common::String &Filename, unsigned int Z) {
	return true;
}

bool MoviePlayer::unloadMovie() {
	return true;
}

bool MoviePlayer::play() {
	return true;
}

bool MoviePlayer::pause() {
	return true;
}

void MoviePlayer::update() {
}

bool MoviePlayer::isMovieLoaded() {
	return false;
}

bool MoviePlayer::isPaused() {
	return true;
}

float MoviePlayer::getScaleFactor() {
	return 1.0f;
}

void MoviePlayer::setScaleFactor(float ScaleFactor) {
}

double MoviePlayer::getTime() {
	return 1.0;
}

#endif // USE_THEORADEC

} // End of namespace Sword25
