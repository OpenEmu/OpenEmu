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

#include "groovie/cursor.h"
#include "groovie/groovie.h"

#include "common/debug.h"
#include "common/archive.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"

namespace Groovie {

// Cursor Manager

GrvCursorMan::GrvCursorMan(OSystem *system) :
	_syst(system), _lastTime(0), _current(255), _cursor(NULL) {
}

GrvCursorMan::~GrvCursorMan() {
	// Delete the cursors
	for (uint cursor = 0; cursor < _cursors.size(); cursor++) {
		delete _cursors[cursor];
	}

	CursorMan.popAllCursors();
}

void GrvCursorMan::show(bool visible) {
	CursorMan.showMouse(visible);
}

uint8 GrvCursorMan::getStyle() {
	return _current;
}

void GrvCursorMan::setStyle(uint8 newStyle) {
	// Reset the animation
	_lastFrame = 254;
	_lastTime = 1;

	// Save the new cursor
	_current = newStyle;
	_cursor = _cursors[newStyle];

	// Show the first frame
	_cursor->enable();
	animate();
}

void GrvCursorMan::animate() {
	if (_lastTime) {
		int newTime = _syst->getMillis();
		if (_lastTime - newTime >= 75) {
			_lastFrame++;
			_lastFrame %= _cursor->getFrames();
			_cursor->showFrame(_lastFrame);
			_lastTime = _syst->getMillis();
		}
	}
}


// t7g Cursor

class Cursor_t7g : public Cursor {
public:
	Cursor_t7g(uint8 *img, uint8 *pal);

	void enable();
	void showFrame(uint16 frame);

private:
	byte *_img;
	byte *_pal;
};

Cursor_t7g::Cursor_t7g(uint8 *img, uint8 *pal) :
	_pal(pal) {

	_width = img[0];
	_height = img[1];
	_numFrames = img[2];
	uint8 elinor1 = img[3];
	uint8 elinor2 = img[4];

	_img = img + 5;

	debugC(1, kGroovieDebugCursor | kGroovieDebugAll, "Groovie::Cursor: width: %d, height: %d, frames:%d", _width, _height, _numFrames);
	debugC(1, kGroovieDebugCursor | kGroovieDebugUnknown | kGroovieDebugAll, "Groovie::Cursor: elinor: 0x%02X (%d), 0x%02X (%d)", elinor1, elinor1, elinor2, elinor2);
}

void Cursor_t7g::enable() {
	// Apply the palette
	CursorMan.replaceCursorPalette(_pal, 0, 32);
}

void Cursor_t7g::showFrame(uint16 frame) {
	// Set the mouse cursor
	int offset = _width * _height * frame;
	CursorMan.replaceCursor((const byte *)_img + offset, _width, _height, _width >> 1, _height >> 1, 0);
}


// t7g Cursor Manager

#define NUM_IMGS 9
static const uint16 cursorDataOffsets[NUM_IMGS] = {
0x0000, 0x182f, 0x3b6d, 0x50cc, 0x6e79, 0x825d, 0x96d7, 0xa455, 0xa776
};

#define NUM_PALS 7
//Pals: 0xb794, 0xb7f4, 0xb854, 0xb8b4, 0xb914, 0xb974, 0xb9d4

#define NUM_STYLES 11
// pyramid is cursor 8, eyes are 9 & 10
const uint GrvCursorMan_t7g::_cursorImg[NUM_STYLES] = {3, 5, 4, 3, 1, 0, 2, 6, 7, 8, 8};
const uint GrvCursorMan_t7g::_cursorPal[NUM_STYLES] = {0, 0, 0, 0, 2, 0, 1, 3, 5, 4, 6};

GrvCursorMan_t7g::GrvCursorMan_t7g(OSystem *system, Common::MacResManager *macResFork) :
	GrvCursorMan(system) {

	Common::SeekableReadStream *robgjd = 0;

	if (macResFork) {
		// Open the cursors file from the resource fork
		robgjd = macResFork->getResource("rob.gjd");
	} else {
		// Open the cursors file
		robgjd = SearchMan.createReadStreamForMember("rob.gjd");
	}

	if (!robgjd)
		error("Groovie::Cursor: Couldn't open rob.gjd");

	// Load the images
	for (uint imgnum = 0; imgnum < NUM_IMGS; imgnum++) {
		robgjd->seek(cursorDataOffsets[imgnum]);
		_images.push_back(loadImage(*robgjd));
	}

	// Load the palettes
	robgjd->seek(-0x60 * NUM_PALS, SEEK_END);
	for (uint palnum = 0; palnum < NUM_PALS; palnum++) {
		_palettes.push_back(loadPalette(*robgjd));
	}

	// Build the cursors
	for (uint cursor = 0; cursor < NUM_STYLES; cursor++) {
		Cursor *s = new Cursor_t7g(_images[_cursorImg[cursor]], _palettes[_cursorPal[cursor]]);
		_cursors.push_back(s);
	}

	delete robgjd;
}

GrvCursorMan_t7g::~GrvCursorMan_t7g() {
	// Delete the images
	for (uint img = 0; img < _images.size(); img++) {
		delete[] _images[img];
	}

	// Delete the palettes
	for (uint pal = 0; pal < _palettes.size(); pal++) {
		delete[] _palettes[pal];
	}
}

byte *GrvCursorMan_t7g::loadImage(Common::SeekableReadStream &file) {
	uint16 decompbytes = 0, offset, i, length;
	uint8 flagbyte, lengthmask = 0x0F, offsetlen, var_8;
	byte *cursorStorage = new byte[65536];
	uint8 *runningcursor = cursorStorage;

	bool finished = false;
	while (!(finished || file.eos())) {
		flagbyte = file.readByte();
		for (i = 1; i <= 8; i++) {
			if (!file.eos()) {
				if (flagbyte & 1) {
					*(runningcursor++) = file.readByte();
					decompbytes++;
				} else {
					var_8 = file.readByte();
					offsetlen = file.readByte();
					if (var_8 == 0 && offsetlen == 0) {
						finished = true;
						break;
					}
					length = (offsetlen & lengthmask) + 3;
					offsetlen >>= 4;
					offset = (offsetlen << 8) + var_8;
					decompbytes += length;

					for (; length > 0; length--, runningcursor++) {
						*(runningcursor) = *(runningcursor - offset);
					}
				}
				flagbyte = flagbyte >> 1;
			}
		}
	}

	return cursorStorage;
}

byte *GrvCursorMan_t7g::loadPalette(Common::SeekableReadStream &file) {
	byte *palette = new byte[3 * 32];
	file.read(palette, 3 * 32);
	return palette;
}


// v2 Cursor

class Cursor_v2 : public Cursor {
public:
	Cursor_v2(Common::File &file);
	~Cursor_v2();

	void enable();
	void showFrame(uint16 frame);

private:
	// Currently locked to 16bit
	byte *_img;

	Graphics::PixelFormat _format;

	void decodeFrame(byte *pal, byte *data, byte *dest);
};

Cursor_v2::Cursor_v2(Common::File &file) {
	byte *pal = new byte[0x20 * 3];

	_format = g_system->getScreenFormat();

	_numFrames = file.readUint16LE();
	_width = file.readUint16LE();
	_height = file.readUint16LE();

	_img = new byte[_width * _height * _numFrames * 2];

	debugC(1, kGroovieDebugCursor | kGroovieDebugAll, "Groovie::Cursor: width: %d, height: %d, frames:%d", _width, _height, _numFrames);

	uint16 tmp16 = file.readUint16LE();
	debugC(5, kGroovieDebugCursor | kGroovieDebugAll, "hotspot x?: %d\n", tmp16);
	tmp16 = file.readUint16LE();
	debugC(5, kGroovieDebugCursor | kGroovieDebugAll, "hotspot y?: %d\n", tmp16);
	int loop2count = file.readUint16LE();
	debugC(5, kGroovieDebugCursor | kGroovieDebugAll, "loop2count?: %d\n", loop2count);
	for (int l = 0; l < loop2count; l++) {
		tmp16 = file.readUint16LE();
		debugC(5, kGroovieDebugCursor | kGroovieDebugAll, "loop2a: %d\n", tmp16);	// Index frame can merge to/from?
		tmp16 = file.readUint16LE();
		debugC(5, kGroovieDebugCursor | kGroovieDebugAll, "loop2b: %d\n", tmp16);	// Number of frames?
	}

	file.read(pal, 0x20 * 3);

	for (int f = 0; f < _numFrames; f++) {
		uint32 tmp32 = file.readUint32LE();
		debugC(5, kGroovieDebugCursor | kGroovieDebugAll, "loop3: %d\n", tmp32);

		byte *data = new byte[tmp32];
		file.read(data, tmp32);
		decodeFrame(pal, data, _img + (f * _width * _height * 2));

		delete[] data;
	}

	delete[] pal;
}

Cursor_v2::~Cursor_v2() {
	delete[] _img;
}

void Cursor_v2::decodeFrame(byte *pal, byte *data, byte *dest) {
	// Scratch memory
	byte *tmp = new byte[_width * _height * 4];
	byte *ptr = tmp;
	memset(tmp, 0, _width * _height * 4);

	byte ctrA = 0, ctrB = 0;

	byte alpha = 0, palIdx = 0;

	byte r, g, b;

	// Start frame decoding
	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			// If both counters are empty
			if (ctrA == 0 && ctrB == 0) {
				if (*data & 0x80) {
					ctrA = (*data++ & 0x7F) + 1;
				} else {
					ctrB = *data++ + 1;
					alpha = *data & 0xE0;
					palIdx = *data++ & 0x1F;
				}
			}

			if (ctrA) {
				// Block type A - chunk of non-continuous pixels
				palIdx = *data & 0x1F;
				alpha = *data++ & 0xE0;

				r = *(pal + palIdx);
				g = *(pal + palIdx + 0x20);
				b = *(pal + palIdx + 0x40);

				ctrA--;
			} else {
				// Block type B - chunk of continuous pixels
				r = *(pal + palIdx);
				g = *(pal + palIdx + 0x20);
				b = *(pal + palIdx + 0x40);

				ctrB--;
			}

			// Decode pixel
			if (alpha) {
				if (alpha != 0xE0) {
					alpha = ((alpha << 8) / 224);

					// TODO: The * 0 to be replaced by the component value of each pixel
					//       below, respectively - does blending
					r = (byte)((alpha * r + (256 - alpha) * 0) >> 8);
					g = (byte)((alpha * g + (256 - alpha) * 0) >> 8);
					b = (byte)((alpha * b + (256 - alpha) * 0) >> 8);
				}

				*ptr = 1;
				*(ptr + 1) = r;
				*(ptr + 2) = g;
				*(ptr + 3) = b;
			}
			ptr += 4;
		}
	}

	// Convert to screen format
	// NOTE: Currently locked to 16bit
	ptr = tmp;
	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			if (*ptr == 1) {
				*(uint16 *)dest = (uint16)_format.RGBToColor(*(ptr + 1), *(ptr + 2), *(ptr + 3));
			} else {
				*(uint16 *)dest = 0;
			}
			dest += 2;
			ptr += 4;
		}
	}



}

void Cursor_v2::enable() {
}

void Cursor_v2::showFrame(uint16 frame) {
	int offset = _width * _height * frame * 2;
	CursorMan.replaceCursor((const byte *)(_img + offset), _width, _height, _width >> 1, _height >> 1, 0, false, &_format);
}


// v2 Cursor Manager

GrvCursorMan_v2::GrvCursorMan_v2(OSystem *system) :
	GrvCursorMan(system) {

	// Open the icons file
	Common::File iconsFile;
	if (!iconsFile.open("icons.ph"))
		error("Groovie::Cursor: Couldn't open icons.ph");

	// Verify the signature
	uint32 tmp32 = iconsFile.readUint32BE();
	uint16 tmp16 = iconsFile.readUint16LE();
	if (tmp32 != MKTAG('i','c','o','n') || tmp16 != 1)
		error("Groovie::Cursor: icons.ph signature failed: %s %d", tag2str(tmp32), tmp16);


	// Read the number of icons
	uint16 nicons = iconsFile.readUint16LE();

	// Read the icons
	for (int i = 0; i < nicons; i++) {
		Cursor *s = new Cursor_v2(iconsFile);
		_cursors.push_back(s);
	}

	iconsFile.close();
}

GrvCursorMan_v2::~GrvCursorMan_v2() {
}

void GrvCursorMan_v2::setStyle(uint8 newStyle) {
	// Cursor 4 is actually cursor 3, but with some changes to alpha blending
	// (which is currently not handled)
	GrvCursorMan::setStyle(newStyle == 4 ? 3 : newStyle);
}

} // End of Groovie namespace
