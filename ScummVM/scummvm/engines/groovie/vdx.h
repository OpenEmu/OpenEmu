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

#ifndef GROOVIE_VDX_H
#define GROOVIE_VDX_H

#include "groovie/player.h"

namespace Common {
class ReadStream;
}

namespace Groovie {

class VDXPlayer : public VideoPlayer {
public:
	VDXPlayer(GroovieEngine *vm);
	~VDXPlayer();
	void resetFlags();
	void setOrigin(int16 x, int16 y);

protected:
	uint16 loadInternal();
	bool playFrameInternal();

private:
	Graphics::Surface *_fg, *_bg;
	uint8 _palBuf[3 * 256];

	// Origin
	int16 _origX, _origY;

	// Video flags
	bool _flagZero;
	bool _flagOne;
	bool _flagOnePrev;
	byte _flag2Byte;
	bool _flagThree;
	bool _flagFour;
	bool _flagFive;
	bool _flagSix;
	bool _flagSeven;
	bool _flagEight;
	bool _flagNine;

	bool _flagSkipStill;
	bool _flagSkipPalette;
	bool _flagFirstFrame;
	bool _flagTransparent;
	bool _flagUpdateStill;

	void getStill(Common::ReadStream *in);
	void getDelta(Common::ReadStream *in);
	void expandColorMap(byte *out, uint16 colorMap, uint8 color1, uint8 color0);
	void decodeBlockStill(byte *buf, byte *colors, uint16 imageWidth, uint8 mask);
	void decodeBlockDelta(uint32 offset, byte *colors, uint16 imageWidth);
	void chunkSound(Common::ReadStream *in);
	void setPalette(uint8 *palette);
	void fadeIn(uint8 *palette);
};

} // End of Groovie namespace

#endif // GROOVIE_VDX_H
