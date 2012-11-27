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

#include "cine/cine.h"
#include "cine/various.h"
#include "cine/pal.h"
#include "common/system.h" // For g_system->getPaletteManager()->setPalette
#include "common/textconsole.h"

#include "graphics/palette.h"

namespace Cine {

static byte paletteBuffer1[16];
static byte paletteBuffer2[16];

void loadPal(const char *fileName) {
	char buffer[20];

	removeExtention(buffer, fileName);

	strcat(buffer, ".PAL");
	g_cine->_palArray.clear();

	Common::File palFileHandle;
	if (!palFileHandle.open(buffer))
		error("loadPal(): Cannot open file %s", fileName);

	uint16 palEntriesCount = palFileHandle.readUint16LE();
	palFileHandle.readUint16LE(); // entry size

	g_cine->_palArray.resize(palEntriesCount);
	for (uint i = 0; i < g_cine->_palArray.size(); ++i) {
		palFileHandle.read(g_cine->_palArray[i].name, 10);
		palFileHandle.read(g_cine->_palArray[i].pal1, 16);
		palFileHandle.read(g_cine->_palArray[i].pal2, 16);
	}
	palFileHandle.close();
}

int16 findPaletteFromName(const char *fileName) {
	char buffer[10];
	uint16 position = 0;
	uint16 i;

	strcpy(buffer, fileName);

	while (position < strlen(fileName)) {
		if (buffer[position] > 'a' && buffer[position] < 'z') {
			buffer[position] += 'A' - 'a';
		}

		position++;
	}

	for (i = 0; i < g_cine->_palArray.size(); i++) {
		if (!strcmp(buffer, g_cine->_palArray[i].name)) {
			return i;
		}
	}

	return -1;

}

void loadRelatedPalette(const char *fileName) {
	char localName[16];
	byte i;
	int16 paletteIndex;

	removeExtention(localName, fileName);

	paletteIndex = findPaletteFromName(localName);

	if (paletteIndex == -1) {
		// generate default palette
		for (i = 0; i < 16; i++) {
			paletteBuffer1[i] = paletteBuffer2[i] = (i << 4) + i;
		}
	} else {
		assert(paletteIndex < (int32)g_cine->_palArray.size());
		memcpy(paletteBuffer1, g_cine->_palArray[paletteIndex].pal1, 16);
		memcpy(paletteBuffer2, g_cine->_palArray[paletteIndex].pal2, 16);
	}
}

namespace {
/** Is given endian type big endian? (Handles native endian type too, otherwise this would be trivial). */
bool isBigEndian(const EndianType endian) {
	assert(endian == CINE_NATIVE_ENDIAN || endian == CINE_LITTLE_ENDIAN || endian == CINE_BIG_ENDIAN);

	// Handle explicit little and big endian types here
	if (endian != CINE_NATIVE_ENDIAN) {
		return (endian == CINE_BIG_ENDIAN);
	}

	// Handle native endian type here
#if defined(SCUMM_BIG_ENDIAN)
	return true;
#elif defined(SCUMM_LITTLE_ENDIAN)
	return false;
#else
	#error No endianness defined
#endif
}

/** Calculate byte position of given bit position in a multibyte variable using defined endianness. */
int bytePos(const int bitPos, const int numBytes, const bool bigEndian) {
	if (bigEndian)
		return (numBytes - 1) - (bitPos / 8);
	else // little endian
		return bitPos / 8;
}

/** Calculate the value of "base" to the power of "power". */
int power(int base, int power) {
	int result = 1;
	while (power--)
		result *= base;
	return result;
}
} // end of anonymous namespace

// a.k.a. palRotate
Palette &Palette::rotateRight(byte firstIndex, byte lastIndex, signed rotationAmount) {
	debug(1, "Palette::rotateRight(firstIndex: %d, lastIndex: %d, rotationAmount:%d)", firstIndex, lastIndex, rotationAmount);
	assert(rotationAmount >= 0);

	for (int j = 0; j < rotationAmount; j++) {
		const Color lastColor = _colors[lastIndex];

		for (int i = lastIndex; i > firstIndex; i--)
			_colors[i] = _colors[i - 1];

		_colors[firstIndex] = lastColor;
	}
	return *this;
}

bool Palette::empty() const {
	return _colors.empty();
}

uint Palette::colorCount() const {
	return _colors.size();
}

Palette &Palette::fillWithBlack() {
	for (uint i = 0; i < _colors.size(); i++) {
		_colors[i].r = 0;
		_colors[i].g = 0;
		_colors[i].b = 0;
	}

	return *this;
}

// TODO: Add better heuristic for checking whether the color format is valid
bool Palette::isValid() const {
	// Check that the color format has been actually set and not just default constructed.
	// Also check that the alpha channel is discarded.
	return _format != Graphics::PixelFormat() && _format.aLoss == 8;
}

const Graphics::PixelFormat &Palette::colorFormat() const {
	return _format;
}

void Palette::setGlobalOSystemPalette() const {
	byte buf[256 * 3]; // Allocate space for the largest possible palette
	// The color format used by OSystem's setPalette-function:
	save(buf, sizeof(buf), Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0), CINE_LITTLE_ENDIAN);

	if (g_cine->getPlatform() == Common::kPlatformAmiga && colorCount() == 16) {
		// The Amiga version of Future Wars does use the upper 16 colors for a darkened
		// game palette to allow transparent dialog boxes. To support that in our code
		// we do calculate that palette over here and append it to the screen palette.
		for (uint i = 0; i < 16 * 3; ++i)
			buf[16 * 3 + i] = buf[i] >> 1;

		g_system->getPaletteManager()->setPalette(buf, 0, colorCount() * 2);
	} else {
		g_system->getPaletteManager()->setPalette(buf, 0, colorCount());
	}
}

Cine::Palette::Color Palette::getColor(byte index) const {
	return _colors[index];
}

uint8 Palette::getR(byte index) const {
	return _colors[index].r;
}

uint8 Palette::getG(byte index) const {
	return _colors[index].g;
}

uint8 Palette::getB(byte index) const {
	return _colors[index].b;
}

void Palette::setColorFormat(const Graphics::PixelFormat format) {
	_format = format;
}

// a.k.a. transformPaletteRange
Palette &Palette::saturatedAddColor(Palette &output, byte firstIndex, byte lastIndex, signed r, signed g, signed b) const {
	assert(firstIndex < colorCount() && lastIndex < colorCount());
	assert(firstIndex < output.colorCount() && lastIndex < output.colorCount());
	assert(output.colorFormat() == colorFormat());

	for (uint i = firstIndex; i <= lastIndex; i++)
		saturatedAddColor(output._colors[i], _colors[i], r, g, b);

	return output;
}

Palette &Palette::saturatedAddColor(Palette &output, byte firstIndex, byte lastIndex, signed rSource, signed gSource, signed bSource, const Graphics::PixelFormat &sourceFormat) const {
	// Convert the source color to the internal color format ensuring that no divide by zero will happen
	const signed r = ((signed) _format.rMax()) * rSource / MAX<int>(sourceFormat.rMax(), 1);
	const signed g = ((signed) _format.gMax()) * gSource / MAX<int>(sourceFormat.gMax(), 1);
	const signed b = ((signed) _format.bMax()) * bSource / MAX<int>(sourceFormat.bMax(), 1);

	return saturatedAddColor(output, firstIndex, lastIndex, r, g, b);
}

Palette &Palette::saturatedAddNormalizedGray(Palette &output, byte firstIndex, byte lastIndex, int grayDividend, int grayDenominator) const {
	assert(grayDenominator != 0);
	const signed r = ((signed) _format.rMax()) * grayDividend / grayDenominator;
	const signed g = ((signed) _format.gMax()) * grayDividend / grayDenominator;
	const signed b = ((signed) _format.bMax()) * grayDividend / grayDenominator;

	return saturatedAddColor(output, firstIndex, lastIndex, r, g, b);
}

// a.k.a. transformColor
void Palette::saturatedAddColor(Color &result, const Color &baseColor, signed r, signed g, signed b) const {
	result.r = CLIP<int>(baseColor.r + r, 0, _format.rMax());
	result.g = CLIP<int>(baseColor.g + g, 0, _format.gMax());
	result.b = CLIP<int>(baseColor.b + b, 0, _format.bMax());
}

Palette::Palette(const Graphics::PixelFormat format, const uint numColors) : _format(format), _colors() {
	_colors.resize(numColors);
	fillWithBlack();
}

Palette &Palette::clear() {
	_format = Graphics::PixelFormat();
	_colors.clear();
	return *this;
}

Palette &Palette::load(const byte *buf, const uint size, const Graphics::PixelFormat format, const uint numColors, const EndianType endian) {
	assert(format.bytesPerPixel * numColors <= size); // Make sure there's enough input space
	assert(format.aLoss == 8); // No alpha
	assert(format.rShift / 8 == (format.rShift + MAX<int>(0, format.rBits() - 1)) / 8); // R must be inside one byte
	assert(format.gShift / 8 == (format.gShift + MAX<int>(0, format.gBits() - 1)) / 8); // G must be inside one byte
	assert(format.bShift / 8 == (format.bShift + MAX<int>(0, format.bBits() - 1)) / 8); // B must be inside one byte

	setColorFormat(format);

	_colors.clear();
	_colors.resize(numColors);

	const int rBytePos = bytePos(format.rShift, format.bytesPerPixel, isBigEndian(endian));
	const int gBytePos = bytePos(format.gShift, format.bytesPerPixel, isBigEndian(endian));
	const int bBytePos = bytePos(format.bShift, format.bytesPerPixel, isBigEndian(endian));

	for (uint i = 0; i < numColors; i++) {
		// format.rMax(), format.gMax(), format.bMax() are also used as masks here
		_colors[i].r = (buf[i * format.bytesPerPixel + rBytePos] >> (format.rShift % 8)) & format.rMax();
		_colors[i].g = (buf[i * format.bytesPerPixel + gBytePos] >> (format.gShift % 8)) & format.gMax();
		_colors[i].b = (buf[i * format.bytesPerPixel + bBytePos] >> (format.bShift % 8)) & format.bMax();
	}

	return *this;
}

byte *Palette::save(byte *buf, const uint size, const EndianType endian) const {
	return save(buf, size, colorFormat(), colorCount(), endian);
}

byte *Palette::save(byte *buf, const uint size, const Graphics::PixelFormat format, const EndianType endian) const {
	return save(buf, size, format, colorCount(), endian);
}

byte *Palette::save(byte *buf, const uint size, const Graphics::PixelFormat format, const uint numColors, const EndianType endian, const byte firstIndex) const {
	assert(format.bytesPerPixel * numColors <= size); // Make sure there's enough output space
	assert(format.aLoss == 8); // No alpha
	assert(format.rShift / 8 == (format.rShift + MAX<int>(0, format.rBits() - 1)) / 8); // R must be inside one byte
	assert(format.gShift / 8 == (format.gShift + MAX<int>(0, format.gBits() - 1)) / 8); // G must be inside one byte
	assert(format.bShift / 8 == (format.bShift + MAX<int>(0, format.bBits() - 1)) / 8); // B must be inside one byte

	// Clear the part of the output palette we're going to be writing to with all black
	memset(buf, 0, format.bytesPerPixel * numColors);

	// Calculate original R/G/B max values
	const int rOrigMax = power(2, 8 - colorFormat().rLoss) - 1;
	const int gOrigMax = power(2, 8 - colorFormat().gLoss) - 1;
	const int bOrigMax = power(2, 8 - colorFormat().bLoss) - 1;

	// Calculate new R/G/B max values
	const int rNewMax = power(2, 8 - format.rLoss) - 1;
	const int gNewMax = power(2, 8 - format.gLoss) - 1;
	const int bNewMax = power(2, 8 - format.bLoss) - 1;

	// Calculate the byte position
	const int rBytePos = bytePos(format.rShift, format.bytesPerPixel, isBigEndian(endian));
	const int gBytePos = bytePos(format.gShift, format.bytesPerPixel, isBigEndian(endian));
	const int bBytePos = bytePos(format.bShift, format.bytesPerPixel, isBigEndian(endian));

	// Save the palette to the output in the specified format
	for (uint i = firstIndex; i < firstIndex + numColors; i++) {
		const uint r = (_colors[i].r * rNewMax) / rOrigMax;
		const uint g = (_colors[i].g * gNewMax) / gOrigMax;
		const uint b = (_colors[i].b * bNewMax) / bOrigMax;

		buf[i * format.bytesPerPixel + rBytePos] |= r << (format.rShift % 8);
		buf[i * format.bytesPerPixel + gBytePos] |= g << (format.gShift % 8);
		buf[i * format.bytesPerPixel + bBytePos] |= b << (format.bShift % 8);
	}

	// Return the pointer to the output palette
	return buf;
}

} // End of namespace Cine
