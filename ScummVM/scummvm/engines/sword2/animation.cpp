/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "common/file.h"
#include "common/mutex.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/screen.h"
#include "sword2/animation.h"

#include "graphics/palette.h"

#include "gui/message.h"

#include "video/dxa_decoder.h"
#include "video/smk_decoder.h"
#include "video/psx_decoder.h"

#include "engines/util.h"

namespace Sword2 {

///////////////////////////////////////////////////////////////////////////////
// Basic movie player
///////////////////////////////////////////////////////////////////////////////

MoviePlayer::MoviePlayer(Sword2Engine *vm, OSystem *system, Video::VideoDecoder *decoder, DecoderType decoderType)
	: _vm(vm), _system(system) {
	_decoderType = decoderType;
	_decoder = decoder;

	_white = 255;
	_black = 0;
}

MoviePlayer::~MoviePlayer() {
	delete _decoder;
}

/**
 * Plays an animated cutscene.
 * @param id the id of the file
 */
bool MoviePlayer::load(const char *name) {
	// This happens when quitting during the "eye" cutscene.
	if (_vm->shouldQuit())
		return false;

	_textSurface = NULL;

	Common::String filename;
	switch (_decoderType) {
	case kVideoDecoderDXA:
		filename = Common::String::format("%s.dxa", name);
		break;
	case kVideoDecoderSMK:
		filename = Common::String::format("%s.smk", name);
		break;
	case kVideoDecoderPSX:
		filename = Common::String::format("%s.str", name);

		// Need to switch to true color
		initGraphics(640, 480, true, 0);

		// Need to load here in case it fails in which case we'd need
		// to go back to paletted mode
		if (_decoder->loadFile(filename)) {
			_decoder->start();
			return true;
		} else {
			initGraphics(640, 480, true);
			return false;
		}
	}

	if (!_decoder->loadFile(filename))
		return false;

	// For DXA, also add the external sound file
	if (_decoderType == kVideoDecoderDXA)
		_decoder->addStreamFileTrack(name);

	_decoder->start();
	return true;
}

void MoviePlayer::play(MovieText *movieTexts, uint32 numMovieTexts, uint32 leadIn, uint32 leadOut) {
	_leadOutFrame = _decoder->getFrameCount();
	if (_leadOutFrame > 60)
		_leadOutFrame -= 60;

	_movieTexts = movieTexts;
	_numMovieTexts = numMovieTexts;
	_currentMovieText = 0;
	_leadOut = leadOut;

	if (leadIn)
		_vm->_sound->playMovieSound(leadIn, kLeadInSound);

	bool terminated = !playVideo();

	closeTextObject(_currentMovieText, NULL, 0);

	if (terminated) {
		_vm->_sound->stopMovieSounds();
		_vm->_sound->stopSpeech();
	}

	if (_decoderType == kVideoDecoderPSX) {
		// Need to jump back to paletted color
		initGraphics(640, 480, true);
	}
}

void MoviePlayer::openTextObject(uint32 index) {
	MovieText *text = &_movieTexts[index];

	// Pull out the text line to get the official text number (for WAV id)

	uint32 res = text->_textNumber / SIZE;
	uint32 localText = text->_textNumber & 0xffff;

	// Open text resource and get the line

	byte *textData = _vm->fetchTextLine(_vm->_resman->openResource(res), localText);

	text->_speechId = READ_LE_UINT16(textData);

	// Is it speech or subtitles, or both?

	// If we want subtitles, or there was no sound

	if (_vm->getSubtitles() || !text->_speechId) {
		text->_textMem = _vm->_fontRenderer->makeTextSprite(textData + 2, 600, 255, _vm->_speechFontId, 1);
	}

	_vm->_resman->closeResource(res);

	if (text->_textMem) {
		FrameHeader frame;

		frame.read(text->_textMem);

		text->_textSprite.x = 320 - frame.width / 2;
		text->_textSprite.y = 440 - frame.height;
		text->_textSprite.w = frame.width;
		text->_textSprite.h = frame.height;
		text->_textSprite.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION;
		text->_textSprite.data = text->_textMem + FrameHeader::size();
		text->_textSprite.isText = true;
		_vm->_screen->createSurface(&text->_textSprite, &_textSurface);

		_textX = 320 - text->_textSprite.w / 2;
		_textY = 420 - text->_textSprite.h;
	}
}

void MoviePlayer::closeTextObject(uint32 index, Graphics::Surface *screen, uint16 pitch) {
	if (index < _numMovieTexts) {
		MovieText *text = &_movieTexts[index];

		free(text->_textMem);
		text->_textMem = NULL;

		if (_textSurface) {
			if (screen) {
				// If the frame doesn't cover the entire
				// screen, we have to erase the subtitles
				// manually.

				int frameWidth = _decoder->getWidth();
				int frameHeight = _decoder->getHeight();

				if (_decoderType == kVideoDecoderPSX)
					frameHeight *= 2;

				int frameX = (_system->getWidth() - frameWidth) / 2;
				int frameY = (_system->getHeight() - frameHeight) / 2;
				uint32 black = getBlackColor();

				for (int y = 0; y < text->_textSprite.h; y++) {
					if (_textY + y < frameY || _textY + y >= frameY + frameHeight) {
						screen->hLine(_textX, _textY + y, _textX + text->_textSprite.w, black);
					} else {
						if (frameX > _textX)
							screen->hLine(_textX, _textY + y, frameX, black);
						if (frameX + frameWidth < _textX + text->_textSprite.w)
							screen->hLine(frameX + frameWidth, _textY + y, text->_textSprite.w, black);
					}
				}
			}

			_vm->_screen->deleteSurface(_textSurface);
			_textSurface = NULL;
		}
	}
}

#define PUT_PIXEL(c) \
	switch (screen->format.bytesPerPixel) { \
	case 1: \
		*dst = (c); \
		break; \
	case 2: \
		WRITE_UINT16(dst, (c)); \
		break; \
	case 4: \
		WRITE_UINT32(dst, (c)); \
		break; \
	}

void MoviePlayer::drawTextObject(uint32 index, Graphics::Surface *screen, uint16 pitch) {
	MovieText *text = &_movieTexts[index];

	uint32 white = getWhiteColor();
	uint32 black = getBlackColor();

	if (text->_textMem && _textSurface) {
		byte *src = text->_textSprite.data;
		uint16 width = text->_textSprite.w;
		uint16 height = text->_textSprite.h;

		// Resize text sprites for PSX version
		byte *psxSpriteBuffer = 0;
		if (Sword2Engine::isPsx()) {
			height *= 2;
			psxSpriteBuffer = (byte *)malloc(width * height);
			Screen::resizePsxSprite(psxSpriteBuffer, src, width, height);
			src = psxSpriteBuffer;
		}

		for (int y = 0; y < height; y++) {
			byte *dst = (byte *)screen->getBasePtr(_textX, _textY + y);

			for (int x = 0; x < width; x++) {
				if (src[x] == 1) {
					PUT_PIXEL(black);
				} else if (src[x] == 255) {
					PUT_PIXEL(white);
				}

				dst += screen->format.bytesPerPixel;
			}

			src += width;
		}

		// Free buffer used to resize psx sprite
		if (Sword2Engine::isPsx())
			free(psxSpriteBuffer);
	}
}

#undef PUT_PIXEL

void MoviePlayer::performPostProcessing(Graphics::Surface *screen, uint16 pitch) {
	MovieText *text;
	int frame = _decoder->getCurFrame();

	if (_currentMovieText < _numMovieTexts) {
		text = &_movieTexts[_currentMovieText];
	} else {
		text = NULL;
	}

	if (text && frame == text->_startFrame) {
		if ((_vm->getSubtitles() || !text->_speechId) && _currentMovieText < _numMovieTexts) {
			openTextObject(_currentMovieText);
		}
	}

	if (text && frame >= text->_startFrame) {
		if (text->_speechId && !text->_played && _vm->_sound->amISpeaking() == RDSE_QUIET) {
			text->_played = true;
			_vm->_sound->playCompSpeech(text->_speechId, 16, 0);
		}
		if (frame < text->_endFrame) {
			drawTextObject(_currentMovieText, screen, pitch);
		} else {
			closeTextObject(_currentMovieText, screen, pitch);
			_currentMovieText++;
		}
	}

	if (_leadOut && _decoder->getCurFrame() == _leadOutFrame) {
		_vm->_sound->playMovieSound(_leadOut, kLeadOutSound);
	}
}

bool MoviePlayer::playVideo() {
	uint16 x = (g_system->getWidth() - _decoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - _decoder->getHeight()) / 2;

	while (!_vm->shouldQuit() && !_decoder->endOfVideo()) {
		if (_decoder->needsUpdate()) {
			const Graphics::Surface *frame = _decoder->decodeNextFrame();
			if (frame) {
				if (_decoderType == kVideoDecoderPSX)
					drawFramePSX(frame);
				else
					_vm->_system->copyRectToScreen(frame->pixels, frame->pitch, x, y, frame->w, frame->h);
			}

			if (_decoder->hasDirtyPalette()) {
				_vm->_system->getPaletteManager()->setPalette(_decoder->getPalette(), 0, 256);

				uint32 maxWeight = 0;
				uint32 minWeight = 0xFFFFFFFF;
				uint32 weight;
				byte r, g, b;

				const byte *palette = _decoder->getPalette();

				for (int i = 0; i < 256; i++) {
					r = *palette++;
					g = *palette++;
					b = *palette++;

					weight = 3 * r * r + 6 * g * g + 2 * b * b;

					if (weight >= maxWeight) {
						maxWeight = weight;
						_white = i;
					}

					if (weight <= minWeight) {
						minWeight = weight;
						_black = i;
					}
				}
			}

			Graphics::Surface *screen = _vm->_system->lockScreen();
			performPostProcessing(screen, screen->pitch);
			_vm->_system->unlockScreen();
			_vm->_system->updateScreen();
		}

		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event))
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				return false;

		_vm->_system->delayMillis(10);
	}

	return !_vm->shouldQuit();
}

uint32 MoviePlayer::getBlackColor() {
	return (_decoderType == kVideoDecoderPSX) ? g_system->getScreenFormat().RGBToColor(0x00, 0x00, 0x00) : _black;
}

uint32 MoviePlayer::getWhiteColor() {
	return (_decoderType == kVideoDecoderPSX) ? g_system->getScreenFormat().RGBToColor(0xFF, 0xFF, 0xFF) : _white;
}

void MoviePlayer::drawFramePSX(const Graphics::Surface *frame) {
	// The PSX videos have half resolution

	Graphics::Surface scaledFrame;
	scaledFrame.create(frame->w, frame->h * 2, frame->format);

	for (int y = 0; y < scaledFrame.h; y++)
		memcpy(scaledFrame.getBasePtr(0, y), frame->getBasePtr(0, y / 2), scaledFrame.w * scaledFrame.format.bytesPerPixel);

	uint16 x = (g_system->getWidth() - scaledFrame.w) / 2;
	uint16 y = (g_system->getHeight() - scaledFrame.h) / 2;

	_vm->_system->copyRectToScreen(scaledFrame.pixels, scaledFrame.pitch, x, y, scaledFrame.w, scaledFrame.h);

	scaledFrame.free();
}

///////////////////////////////////////////////////////////////////////////////
// Factory function for creating the appropriate cutscene player
///////////////////////////////////////////////////////////////////////////////

MoviePlayer *makeMoviePlayer(const char *name, Sword2Engine *vm, OSystem *system, uint32 frameCount) {
	Common::String filename;

	filename = Common::String::format("%s.str", name);

	if (Common::File::exists(filename)) {
#ifdef USE_RGB_COLOR
		Video::VideoDecoder *psxDecoder = new Video::PSXStreamDecoder(Video::PSXStreamDecoder::kCD2x, frameCount);
		return new MoviePlayer(vm, system, psxDecoder, kVideoDecoderPSX);
#else
		GUI::MessageDialog dialog(_("PSX cutscenes found but ScummVM has been built without RGB color support"), _("OK"));
		dialog.runModal();
		return NULL;
#endif
	}

	filename = Common::String::format("%s.smk", name);

	if (Common::File::exists(filename)) {
		Video::SmackerDecoder *smkDecoder = new Video::SmackerDecoder();
		return new MoviePlayer(vm, system, smkDecoder, kVideoDecoderSMK);
	}

	filename = Common::String::format("%s.dxa", name);

	if (Common::File::exists(filename)) {
#ifdef USE_ZLIB
		Video::DXADecoder *dxaDecoder = new Video::DXADecoder();
		return new MoviePlayer(vm, system, dxaDecoder, kVideoDecoderDXA);
#else
		GUI::MessageDialog dialog(_("DXA cutscenes found but ScummVM has been built without zlib support"), _("OK"));
		dialog.runModal();
		return NULL;
#endif
	}

	// Old MPEG2 cutscenes
	filename = Common::String::format("%s.mp2", name);

	if (Common::File::exists(filename)) {
		GUI::MessageDialog dialog(_("MPEG2 cutscenes are no longer supported"), _("OK"));
		dialog.runModal();
		return NULL;
	}

	// The demo tries to play some cutscenes that aren't there, so make those warnings more discreet.
	// In addition, some of the later re-releases of the game don't have the "eye" Virgin logo movie.
	if (!vm->_logic->readVar(DEMO) && strcmp(name, "eye") != 0) {
		Common::String buf = Common::String::format(_("Cutscene '%s' not found"), name);
		GUI::MessageDialog dialog(buf, _("OK"));
		dialog.runModal();
	} else
		warning("Cutscene '%s' not found", name);

	return NULL;
}

} // End of namespace Sword2
