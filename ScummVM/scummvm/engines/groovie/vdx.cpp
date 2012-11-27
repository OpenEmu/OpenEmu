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

#include "groovie/vdx.h"
#include "groovie/graphics.h"
#include "groovie/groovie.h"
#include "groovie/lzss.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "graphics/palette.h"

#define TILE_SIZE 4			// Size of each tile on the image: only ever seen 4 so far
#define VDX_IDENT 0x9267	// 37479

namespace Groovie {

VDXPlayer::VDXPlayer(GroovieEngine *vm) :
	VideoPlayer(vm), _origX(0), _origY(0), _flagOnePrev(false),
	_fg(&_vm->_graphicsMan->_foreground), _bg(&_vm->_graphicsMan->_background) {
}

VDXPlayer::~VDXPlayer() {
	//delete _audioStream;
}

void VDXPlayer::resetFlags() {
	_flagOnePrev = false;
}

void VDXPlayer::setOrigin(int16 x, int16 y) {
	_origX = x;
	_origY = y;
}

uint16 VDXPlayer::loadInternal() {
	if (DebugMan.isDebugChannelEnabled(kGroovieDebugVideo) ||
	    DebugMan.isDebugChannelEnabled(kGroovieDebugAll)) {
		int8 i;
		debugN(1, "Groovie::VDX: New VDX: bitflags are ");
		for (i = 15; i >= 0; i--) {
			debugN(1, "%d", _flags & (1 << i)? 1 : 0);
			if (i % 4 == 0) {
				debugN(1, " ");
			}
		}
		debug(1, " <- 0 ");
	}
	// Flags:
	// - 1 Puzzle piece? Skip palette, don't redraw full screen, draw still to b/ack buffer
	// - 2 Transparent color is 0xFF
	// - 5 Skip still chunks
	// - 7
	// - 8 Just show the first frame
	// - 9 Start a palette fade in
	_flagZero =		((_flags & (1 << 0)) != 0);
	_flagOne =		((_flags & (1 << 1)) != 0);
	_flag2Byte =	(_flags & (1 << 2)) ? 0xFF : 0x00;
	_flagThree =	((_flags & (1 << 3)) != 0);
	_flagFour =		((_flags & (1 << 4)) != 0);
	_flagFive =		((_flags & (1 << 5)) != 0);
	_flagSix =		((_flags & (1 << 6)) != 0);
	_flagSeven =	((_flags & (1 << 7)) != 0);
	_flagEight =	((_flags & (1 << 8)) != 0);
	_flagNine =		((_flags & (1 << 9)) != 0);

	// Enable highspeed if we're not obeying fps, and not marked as special
	// This will be disabled in chunk audio if we're actually an audio vdx
	if ( _vm->_modeSpeed == kGroovieSpeediOS || (_vm->_modeSpeed == kGroovieSpeedTweaked && ((_flags & (1 << 15)) == 0)))
		setOverrideSpeed(true);

	if (_flagOnePrev && !_flagOne && !_flagEight) {
		_flagSeven = true;
	}

	// Save _flagOne for the next video
	_flagOnePrev = _flagOne;

	//_flagTransparent =	_flagOne;
	_flagFirstFrame =	_flagEight;
	//_flagSkipPalette =	_flagSeven;
	_flagSkipPalette =	false;
	//_flagSkipStill =	_flagFive || _flagSeven;
	//_flagUpdateStill =	_flagNine || _flagSix;

	// Begin reading the file
	debugC(1, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: Playing video");

	if (_file->readUint16LE() != VDX_IDENT) {
		error("Groovie::VDX: This does not appear to be a 7th guest VDX file");
		return 0;
	} else {
		debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: VDX file identified correctly");
	}

	uint16 tmp;

	// Skip unknown data: 6 bytes, ref Martine
	tmp = _file->readUint16LE();
	debugC(2, kGroovieDebugVideo | kGroovieDebugUnknown | kGroovieDebugAll, "Groovie::VDX: Martine1 = 0x%04X", tmp);
	tmp = _file->readUint16LE();
	debugC(2, kGroovieDebugVideo | kGroovieDebugUnknown | kGroovieDebugAll, "Groovie::VDX: Martine2 = 0x%04X", tmp);
	tmp = _file->readUint16LE();
	debugC(2, kGroovieDebugVideo | kGroovieDebugUnknown | kGroovieDebugAll, "Groovie::VDX: Martine3 (FPS?) = %d", tmp);

	return tmp;
}

bool VDXPlayer::playFrameInternal() {
	byte currRes = 0x80;
	Common::ReadStream *vdxData = 0;
	while (currRes == 0x80) {
		currRes = _file->readByte();

		// Skip unknown data: 1 byte, ref Edward
		byte tmp = _file->readByte();

		uint32 compSize = _file->readUint32LE();
		uint8 lengthmask = _file->readByte();
		uint8 lengthbits = _file->readByte();

		if (_file->eos())
			break;

		debugC(5, kGroovieDebugVideo | kGroovieDebugUnknown | kGroovieDebugAll, "Groovie::VDX: Edward = 0x%04X", tmp);

		// Read the chunk data and decompress if needed
		if (compSize)
			vdxData = _file->readStream(compSize);

		if (lengthmask && lengthbits) {
			Common::ReadStream *decompData = new LzssReadStream(vdxData, lengthmask, lengthbits);
			delete vdxData;
			vdxData = decompData;
		}

		// Use the current chunk
		switch (currRes) {
			case 0x00:
				debugC(6, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: Replay frame");
				break;
			case 0x20:
				debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: Still frame");
				getStill(vdxData);
				break;
			case 0x25:
				debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: Animation frame");
				getDelta(vdxData);
				break;
			case 0x80:
				debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: Sound resource");
				chunkSound(vdxData);
				break;
			default:
				error("Groovie::VDX: Invalid resource type: %d", currRes);
		}
		delete vdxData;
		vdxData = 0;
	}

	// Wait until the current frame can be shown

	if (!DebugMan.isDebugChannelEnabled(kGroovieDebugFast)) {
		waitFrame();
	}
	// TODO: Move it to a better place
	// Update the screen
	if (currRes == 0x25) {
		//if (_flagSeven) {
			//_vm->_graphicsMan->mergeFgAndBg();
		//}
		_vm->_graphicsMan->updateScreen(_bg);
	}

	// Report the end of the video if we reached the end of the file or if we
	// just wanted to play one frame.
	if (_file->eos() || _flagFirstFrame) {
		_origX = _origY = 0;
		return 1;
	} else {
		return 0;
	}
}

static const uint16 vdxBlockMapLookup[] = {
0xc800, 0xec80, 0xfec8, 0xffec, 0xfffe, 0x3100, 0x7310, 0xf731, 0xff73, 0xfff7, 0x6c80, 0x36c8, 0x136c, 0x6310, 0xc631, 0x8c63,
0xf000, 0xff00, 0xfff0, 0x1111, 0x3333, 0x7777, 0x6666, 0xcccc, 0x0ff0, 0x00ff, 0xffcc, 0x0076, 0xff33, 0x0ee6, 0xccff, 0x6770,
0x33ff, 0x6ee0, 0x4800, 0x2480, 0x1248, 0x0024, 0x0012, 0x2100, 0x4210, 0x8421, 0x0042, 0x0084, 0xf888, 0x0044, 0x0032, 0x111f,
0x22e0, 0x4c00, 0x888f, 0x4470, 0x2300, 0xf111, 0x0e22, 0x00c4, 0xf33f, 0xfccf, 0xff99, 0x99ff, 0x4444, 0x2222, 0xccee, 0x7733,
0x00f8, 0x00f1, 0x00bb, 0x0cdd, 0x0f0f, 0x0f88, 0x13f1, 0x19b3, 0x1f80, 0x226f, 0x27ec, 0x3077, 0x3267, 0x37e4, 0x38e3, 0x3f90,
0x44cf, 0x4cd9, 0x4c99, 0x5555, 0x603f, 0x6077, 0x6237, 0x64c9, 0x64cd, 0x6cd9, 0x70ef, 0x0f00, 0x00f0, 0x0000, 0x4444, 0x2222
};

void VDXPlayer::getDelta(Common::ReadStream *in) {
	uint16 k, l;

	// Get the size of the local palette
	uint16 palSize = in->readUint16LE();

	// Load the palette if it isn't empty
	if (palSize) {
		uint16 palBitField[16];

		// Load the bit field
		for (l = 0; l < 16; l++) {
			palBitField[l] = in->readUint16LE();
		}

		// Load the actual palette
		for (l = 0; l < 16; l++) {
			int flag = 1 << 15;
			for (uint16 j = 0; j < 16; j++) {
				int palIndex = (l * 16) + j;

				if (flag & palBitField[l]) {
					for (k = 0; k < 3; k++) {
						_palBuf[(palIndex * 3) + k] = in->readByte();
					}
				}
				flag = flag >> 1;
			}
		}

		// Apply the palette
		if (!_flagSeven) {
		//if (!_flagSix && !_flagSeven) {
			setPalette(_palBuf);
		}
	}

	uint8 currOpCode = in->readByte();
	uint8 param1, param2, param3;

	uint16 currentLine = 0;
	uint32 offset = 0;
	while (!in->eos()) {
		byte colors[16];
		if (currOpCode < 0x60) {
			param1 = in->readByte();
			param2 = in->readByte();
			expandColorMap(colors, vdxBlockMapLookup[currOpCode], param1, param2);
			decodeBlockDelta(offset, colors, 640);
			offset += TILE_SIZE;
		} else if (currOpCode > 0x7f) {
			param1 = in->readByte();
			param2 = in->readByte();
			param3 = in->readByte();
			expandColorMap(colors, (param1 << 8) + currOpCode, param2, param3);
			decodeBlockDelta(offset, colors, 640);
			offset += TILE_SIZE;
		} else switch (currOpCode) {
			case 0x60: /* Fill tile with the 16 colors given as parameters */
				for (l = 0; l < 16; l++) {
					colors[l] = in->readByte();
				}
				decodeBlockDelta(offset, colors, 640);
				offset += TILE_SIZE;
				break;
			case 0x61: /* Skip to the end of this line, next block is start of next */
				/* Note this is used at the end of EVERY line */
				currentLine++;
				offset = currentLine * TILE_SIZE * 640;
				break;
			case 0x62:
			case 0x63:
			case 0x64:
			case 0x65:
			case 0x66:
			case 0x67:
			case 0x68:
			case 0x69:
			case 0x6a:
			case 0x6b: /* Skip next param1 blocks (within line) */
				offset += (currOpCode - 0x62) * TILE_SIZE;
				break;
			case 0x6c:
			case 0x6d:
			case 0x6e:
			case 0x6f:
			case 0x70:
			case 0x71:
			case 0x72:
			case 0x73:
			case 0x74:
			case 0x75: /* Next param1 blocks are filled with color param2 */
				param1 = currOpCode - 0x6b;
				param2 = in->readByte();
				for (l = 0; l < 16; l++) {
					colors[l] = param2;
				}
				for (k = 0; k < param1; k++) {
					decodeBlockDelta(offset, colors, 640);
					offset += TILE_SIZE;
				}
				break;
			case 0x76:
			case 0x77:
			case 0x78:
			case 0x79:
			case 0x7a:
			case 0x7b:
			case 0x7c:
			case 0x7d:
			case 0x7e:
			case 0x7f: /* Next bytes contain colors to fill the next param1 blocks in the current line*/
				param1 = currOpCode - 0x75;
				for (k = 0; k < param1; k++) {
					param2 = in->readByte();
					for (l = 0; l < 16; l++) {
						colors[l] = param2;
					}
					decodeBlockDelta(offset, colors, 640);
					offset += TILE_SIZE;
				}
				break;
			default:
				error("Groovie::VDX: Broken somehow");
		}
		currOpCode = in->readByte();
	}
}

void VDXPlayer::getStill(Common::ReadStream *in) {
	uint16 numXTiles = in->readUint16LE();
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: numXTiles=%d", numXTiles);
	uint16 numYTiles = in->readUint16LE();
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: numYTiles=%d", numYTiles);

	// It's skipped in the original:
	uint16 colorDepth = in->readUint16LE();
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: colorDepth=%d", colorDepth);

	uint16 imageWidth = TILE_SIZE * numXTiles;

	uint8 mask = 0;
	byte *buf;
	if (_flagOne) {
		// Paint to the foreground
		buf = (byte *)_fg->getBasePtr(0, 0);
		if (_flag2Byte) {
			mask = 0xff;
		} else {
			mask = 0;
		}

		// TODO: Verify this is the right procedure. Couldn't find it on the
		// disassembly, but it's required to work properly
		_flagFirstFrame = true;
	} else {
		// Paint to the background
		buf = (byte *)_bg->getBasePtr(0, 0);
	}

	// Read the palette
	in->read(_palBuf, 3 * 256);

	if (_flagSeven) {
		_flagFive = true;
	}

	// Skip the frame when flag 5 is set, unless flag 1 is set
	if (!_flagFive || _flagOne) {

		byte colors[16];
		for (uint16 j = 0; j < numYTiles; j++) {
			byte *currentTile = buf + j * TILE_SIZE * imageWidth;
			for (uint16 i = numXTiles; i; i--) {
				uint8 color1 = in->readByte();
				uint8 color0 = in->readByte();
				uint16 colorMap = in->readUint16LE();
				expandColorMap(colors, colorMap, color1, color0);
				decodeBlockStill(currentTile, colors, 640, mask);

				currentTile += TILE_SIZE;
			}
		}

		// Apply the palette
		if (_flagNine) {
			// Flag 9 starts a fade in
			fadeIn(_palBuf);
		} else {
			if (!_flagOne && !_flagSeven) {
				// Actually apply the palette
				setPalette(_palBuf);
			}
		}

		if (!_flagOne) {
			_vm->_graphicsMan->updateScreen(_bg);
		}
		/*
		if (_flagSix) {
			if (_flagOne) {
				_vm->_graphicsMan->updateScreen(_fg);
			} else {
				_vm->_graphicsMan->updateScreen(_bg);
			}
			_flagSix = 0;
		}
		*/
	} else {
		// Skip the remaining data
		debugC(10, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: Skipping still frame");
		while (!in->eos()) {
			in->readByte();
		}
	}
}

void VDXPlayer::expandColorMap(byte *out, uint16 colorMap, uint8 color1, uint8 color0) {
	// It's a bit faster to start from the end
	out += 16;
	for (int i = 16; i; i--) {
		// Set the corresponding color
		// The following is an optimized version of:
		// *--out = (colorMap & 1) ? color1 : color0;
		uint8 selector = -(colorMap & 1);
		*--out = (selector & color1) | (~selector & color0);

		// Update the flag map to test the next color
		colorMap >>= 1;
	}
}

void VDXPlayer::decodeBlockStill(byte *buf, byte *colors, uint16 imageWidth, uint8 mask) {
	assert(TILE_SIZE == 4);

	for (int y = TILE_SIZE; y; y--) {
		if (_flagOne) {
			// TODO: optimize with bit logic?
			for (int x = 0; x < TILE_SIZE; x++) {
				// 0xff pixels don't modify the buffer
				if (*colors != 0xff) {
					// Write the color
					*buf = *colors | mask;
					// Note: if the mask is 0, it paints the image
					// else, it paints the image's mask using 0xff
				}

				// Point to the next color
				colors++;

				// Point to the next pixel
				buf++;
			}

			// Point to the start of the next line
			buf += imageWidth - TILE_SIZE;
		} else {
			*((uint32 *)buf) = *((uint32 *)colors);
			colors += 4;

			// Point to the start of the next line
			buf += imageWidth;
		}
	}
}

void VDXPlayer::decodeBlockDelta(uint32 offset, byte *colors, uint16 imageWidth) {
	assert(TILE_SIZE == 4);

	byte *dest;
	// TODO: Verify just the else block is required
	//if (_flagOne) {
		// Paint to the foreground
		//dest = (byte *)_fg->getBasePtr(0, 0) + offset;
	//} else {
		dest = (byte *)_bg->getBasePtr(0, 0) + offset;
	//}

	// Move the pointers to the beginning of the current block
	int32 blockOff = _origX + _origY * imageWidth;
	dest += blockOff;
	byte *fgBuf = 0;
	if (_flagSeven) {
		fgBuf = (byte *)_fg->getBasePtr(0, 0) + offset + blockOff;
		//byte *bgBuf = (byte *)_bg->getBasePtr(0, 0) + offset + blockOff;
	}

	for (int y = TILE_SIZE; y; y--) {
		if (_flagSeven) {
			// Paint mask
			for (int x = 0; x < TILE_SIZE; x++) {
				// TODO: this can probably be optimized with bit logic
				if (fgBuf[x] != 0xff) {
					if (*colors == 0xff) {
						dest[x] = fgBuf[x];
					} else {
						dest[x] = *colors;
					}
				}
				colors++;
			}
			fgBuf += imageWidth;
		} else {
			// Paint directly
			*((uint32 *)dest) = *((uint32 *)colors);
			colors += 4;
		}

		// Move to the next line
		dest += imageWidth;
	}
}

void VDXPlayer::chunkSound(Common::ReadStream *in) {
	if (getOverrideSpeed())
		setOverrideSpeed(false);

	if (!_audioStream) {
		_audioStream = Audio::makeQueuingAudioStream(22050, false);
		Audio::SoundHandle sound_handle;
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &sound_handle, _audioStream);
	}

	byte *data = (byte *)malloc(60000);
	int chunksize = in->read(data, 60000);
	if (!DebugMan.isDebugChannelEnabled(kGroovieDebugFast)) {
		_audioStream->queueBuffer(data, chunksize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	}
}

void VDXPlayer::fadeIn(uint8 *targetpal) {
	// Don't do anything if we're asked to skip palette changes
	if (_flagSkipPalette)
		return;

	// TODO: Is it required? If so, move to an appropiate place
	// Copy the foreground to the background
	memcpy((byte *)_vm->_graphicsMan->_foreground.getBasePtr(0, 0), (byte *)_vm->_graphicsMan->_background.getBasePtr(0, 0), 640 * 320);

	// Start a fadein
	_vm->_graphicsMan->fadeIn(targetpal);

	// Show the background
	_vm->_graphicsMan->updateScreen(_bg);
}

void VDXPlayer::setPalette(uint8 *palette) {
	if (_flagSkipPalette)
		return;

	debugC(7, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::VDX: Setting palette");
	_syst->getPaletteManager()->setPalette(palette, 0, 256);
}

} // End of Groovie namespace
