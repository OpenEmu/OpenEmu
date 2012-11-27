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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SWORD2_ANIMATION_H
#define SWORD2_ANIMATION_H

#include "sword2/screen.h"

namespace Graphics {
struct Surface;
}

namespace Video {
class VideoDecoder;
}

namespace Sword2 {

enum DecoderType {
	kVideoDecoderDXA = 0,
	kVideoDecoderSMK = 1,
	kVideoDecoderPSX = 2
};

struct MovieText {
	uint16 _startFrame;
	uint16 _endFrame;
	uint32 _textNumber;
	byte *_textMem;
	SpriteInfo _textSprite;
	uint16 _speechId;
	bool _played;

	void reset() {
		_textMem = NULL;
		_speechId = 0;
		_played = false;
	}
};

class MoviePlayer {
public:
	MoviePlayer(Sword2Engine *vm, OSystem *system, Video::VideoDecoder *decoder, DecoderType decoderType);
	virtual ~MoviePlayer();

	bool load(const char *name);
	void play(MovieText *movieTexts, uint32 numMovieTexts, uint32 leadIn, uint32 leadOut);

protected:
	Sword2Engine *_vm;
	OSystem *_system;
	MovieText *_movieTexts;
	uint32 _numMovieTexts;
	uint32 _currentMovieText;
	byte *_textSurface;
	int _textX, _textY;
	byte _white, _black;
	DecoderType _decoderType;

	Video::VideoDecoder *_decoder;

	uint32 _leadOut;
	int _leadOutFrame;

	void performPostProcessing(Graphics::Surface *screen, uint16 pitch);
	bool playVideo();
	void drawFramePSX(const Graphics::Surface *frame);

	void openTextObject(uint32 index);
	void closeTextObject(uint32 index, Graphics::Surface *screen, uint16 pitch);
	void drawTextObject(uint32 index, Graphics::Surface *screen, uint16 pitch);

	uint32 getBlackColor();
	uint32 getWhiteColor();
};

MoviePlayer *makeMoviePlayer(const char *name, Sword2Engine *vm, OSystem *system, uint32 frameCount);

} // End of namespace Sword2

#endif
