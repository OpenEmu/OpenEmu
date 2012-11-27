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

#ifndef SWORD1_ANIMATION_H
#define SWORD1_ANIMATION_H

#include "common/list.h"

#include "sword1/screen.h"
#include "sword1/sound.h"

namespace Graphics {
struct Surface;
}

namespace Video {
class VideoDecoder;
}

namespace Sword1 {

enum DecoderType {
	kVideoDecoderDXA = 0,
	kVideoDecoderSMK = 1,
	kVideoDecoderPSX = 2
};

class MovieText {
public:
	uint16 _startFrame;
	uint16 _endFrame;
	uint16 _color;
	Common::String _text;
	MovieText(int startFrame, int endFrame, const Common::String &text, int color) {
		_startFrame = startFrame;
		_endFrame = endFrame;
		_text = text;
		_color = color;
	}
};

class MoviePlayer {
public:
	MoviePlayer(SwordEngine *vm, Text *textMan, ResMan *resMan, OSystem *system, Video::VideoDecoder *decoder, DecoderType decoderType);
	virtual ~MoviePlayer();
	bool load(uint32 id);
	void play();

protected:
	SwordEngine *_vm;
	Text *_textMan;
	ResMan *_resMan;
	OSystem *_system;
	Common::List<MovieText> _movieTexts;
	int _textX, _textY, _textWidth, _textHeight;
	int _textColor;
	uint32 _black;
	uint32 _c1Color, _c2Color, _c3Color, _c4Color;
	DecoderType _decoderType;

	Video::VideoDecoder *_decoder;

	bool playVideo();
	void performPostProcessing(byte *screen);
	void drawFramePSX(const Graphics::Surface *frame);

	uint32 getBlackColor();
	uint32 findTextColor();
	void convertColor(byte r, byte g, byte b, float &h, float &s, float &v);
};

MoviePlayer *makeMoviePlayer(uint32 id, SwordEngine *vm, Text *textMan, ResMan *resMan, OSystem *system);

} // End of namespace Sword1

#endif
