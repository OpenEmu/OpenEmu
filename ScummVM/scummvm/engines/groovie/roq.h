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

#ifndef GROOVIE_ROQ_H
#define GROOVIE_ROQ_H

#include "groovie/player.h"

namespace Groovie {

class GroovieEngine;

struct ROQBlockHeader {
	uint16 type;
	uint32 size;
	uint16 param;
};

class ROQPlayer : public VideoPlayer {
public:
	ROQPlayer(GroovieEngine *vm);
	~ROQPlayer();

protected:
	uint16 loadInternal();
	bool playFrameInternal();

private:
	bool readBlockHeader(ROQBlockHeader &blockHeader);

	bool processBlock();
	bool processBlockInfo(ROQBlockHeader &blockHeader);
	bool processBlockQuadCodebook(ROQBlockHeader &blockHeader);
	bool processBlockQuadVector(ROQBlockHeader &blockHeader);
	void processBlockQuadVectorBlock(int baseX, int baseY, int8 Mx, int8 My);
	void processBlockQuadVectorBlockSub(int baseX, int baseY, int8 Mx, int8 My);
	bool processBlockStill(ROQBlockHeader &blockHeader);
	bool processBlockSoundMono(ROQBlockHeader &blockHeader);
	bool processBlockSoundStereo(ROQBlockHeader &blockHeader);
	bool processBlockAudioContainer(ROQBlockHeader &blockHeader);

	void paint2(byte i, int destx, int desty);
	void paint4(byte i, int destx, int desty);
	void paint8(byte i, int destx, int desty);
	void copy(byte size, int destx, int desty, int offx, int offy);

	// Block coding type
	byte getCodingType();
	uint16 _codingType;
	byte _codingTypeCount;

	// Codebooks
	uint16 _num2blocks;
	uint16 _num4blocks;
	byte _codebook2[256 * 10];
	byte _codebook4[256 * 4];

	// Buffers
	Graphics::Surface *_fg, *_bg, *_thirdBuf;
	Graphics::Surface *_currBuf, *_prevBuf;
	void buildShowBuf();
	byte _scaleX, _scaleY;
	byte _offScale;
	bool _dirty;
	byte _alpha;

};

} // End of Groovie namespace

#endif // GROOVIE_ROQ_H
