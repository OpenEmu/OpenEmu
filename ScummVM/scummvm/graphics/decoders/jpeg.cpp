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

#include "graphics/pixelformat.h"
#include "graphics/yuv_to_rgb.h"
#include "graphics/decoders/jpeg.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Graphics {

// Order used to traverse the quantization tables
static const uint8 _zigZagOrder[64] = {
	0,   1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

JPEGDecoder::JPEGDecoder() : ImageDecoder(),
	_stream(NULL), _w(0), _h(0), _numComp(0), _components(NULL), _numScanComp(0),
	_scanComp(NULL), _currentComp(NULL), _rgbSurface(0) {

	// Initialize the quantization tables
	for (int i = 0; i < JPEG_MAX_QUANT_TABLES; i++)
		_quant[i] = NULL;

	// Initialize the Huffman tables
	for (int i = 0; i < 2 * JPEG_MAX_HUFF_TABLES; i++) {
		_huff[i].count = 0;
		_huff[i].values = NULL;
		_huff[i].sizes = NULL;
		_huff[i].codes = NULL;
	}
}

JPEGDecoder::~JPEGDecoder() {
	destroy();
}

const Surface *JPEGDecoder::getSurface() const {
	// Make sure we have loaded data
	if (!isLoaded())
		return 0;

	if (_rgbSurface)
		return _rgbSurface;

	// Create an RGBA8888 surface
	_rgbSurface = new Graphics::Surface();
	_rgbSurface->create(_w, _h, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));

	// Get our component surfaces
	const Graphics::Surface *yComponent = getComponent(1);
	const Graphics::Surface *uComponent = getComponent(2);
	const Graphics::Surface *vComponent = getComponent(3);

	YUVToRGBMan.convert444(_rgbSurface, Graphics::YUVToRGBManager::kScaleFull, (byte *)yComponent->pixels, (byte *)uComponent->pixels, (byte *)vComponent->pixels, yComponent->w, yComponent->h, yComponent->pitch, uComponent->pitch);

	return _rgbSurface;
}

void JPEGDecoder::destroy() {
	// Reset member variables
	_stream = NULL;
	_w = _h = 0;
	_restartInterval = 0;

	// Free the components
	for (int c = 0; c < _numComp; c++)
		_components[c].surface.free();
	delete[] _components; _components = NULL;
	_numComp = 0;

	// Free the scan components
	delete[] _scanComp; _scanComp = NULL;
	_numScanComp = 0;
	_currentComp = NULL;

	// Free the quantization tables
	for (int i = 0; i < JPEG_MAX_QUANT_TABLES; i++) {
		delete[] _quant[i];
		_quant[i] = NULL;
	}

	// Free the Huffman tables
	for (int i = 0; i < 2 * JPEG_MAX_HUFF_TABLES; i++) {
		_huff[i].count = 0;
		delete[] _huff[i].values; _huff[i].values = NULL;
		delete[] _huff[i].sizes; _huff[i].sizes = NULL;
		delete[] _huff[i].codes; _huff[i].codes = NULL;
	}

	if (_rgbSurface) {
		_rgbSurface->free();
		delete _rgbSurface;
	}
}

bool JPEGDecoder::loadStream(Common::SeekableReadStream &stream) {
	// Reset member variables and tables from previous reads
	destroy();

	// Save the input stream
	_stream = &stream;

	bool ok = true;
	bool done = false;
	while (!_stream->eos() && ok && !done) {
		// Read the marker

		// WORKAROUND: While each and every JPEG file should end with
		// an EOI (end of image) tag, in reality this may not be the
		// case. For instance, at least one image in the Masterpiece
		// edition of Myst doesn't, yet other programs are able to read
		// the image without complaining.
		//
		// Apparently, the customary workaround is to insert a fake
		// EOI tag.

		uint16 marker = _stream->readByte();
		bool fakeEOI = false;

		if (_stream->eos()) {
			fakeEOI = true;
			marker = 0xFF;
		}

		if (marker != 0xFF) {
			error("JPEG: Invalid marker[0]: 0x%02X", marker);
			ok = false;
			break;
		}

		while (marker == 0xFF && !_stream->eos())
			marker = _stream->readByte();

		if (_stream->eos()) {
			fakeEOI = true;
			marker = 0xD9;
		}

		if (fakeEOI)
			warning("JPEG: Inserted fake EOI");

		// Process the marker data
		switch (marker) {
		case 0xC0: // Start Of Frame
			ok = readSOF0();
			break;
		case 0xC4: // Define Huffman Tables
			ok = readDHT();
			break;
		case 0xD8: // Start Of Image
			break;
		case 0xD9: // End Of Image
			done = true;
			break;
		case 0xDA: // Start Of Scan
			ok = readSOS();
			break;
		case 0xDB: // Define Quantization Tables
			ok = readDQT();
			break;
		case 0xE0: // JFIF/JFXX segment
			ok = readJFIF();
			break;
		case 0xDD: // Define Restart Interval
			ok = readDRI();
			break;
		case 0xFE: // Comment
			_stream->seek(_stream->readUint16BE() - 2, SEEK_CUR);
			break;
		default: { // Unknown marker
			uint16 size = _stream->readUint16BE();

			if ((marker & 0xE0) != 0xE0)
				warning("JPEG: Unknown marker %02X, skipping %d bytes", marker, size - 2);

			_stream->seek(size - 2, SEEK_CUR);
		}
		}
	}

	_stream = 0;
	return ok;
}

bool JPEGDecoder::readJFIF() {
	uint16 length = _stream->readUint16BE();
	uint32 tag = _stream->readUint32BE();
	if (tag != MKTAG('J', 'F', 'I', 'F')) {
		warning("JPEGDecoder::readJFIF() tag mismatch");
		return false;
	}
	if (_stream->readByte() != 0)  { // NULL
		warning("JPEGDecoder::readJFIF() NULL mismatch");
		return false;
	}
	byte majorVersion = _stream->readByte();
	byte minorVersion = _stream->readByte();
	if (majorVersion != 1 || minorVersion != 1)
		warning("JPEGDecoder::readJFIF() Non-v1.1 JPEGs may not be handled correctly");
	/* byte densityUnits = */_stream->readByte();
	/* uint16 xDensity = */_stream->readUint16BE();
	/* uint16 yDensity = */_stream->readUint16BE();
	byte thumbW = _stream->readByte();
	byte thumbH = _stream->readByte();
	_stream->seek(thumbW * thumbH * 3, SEEK_CUR); // Ignore thumbnail
	if (length != (thumbW * thumbH * 3) + 16) {
		warning("JPEGDecoder::readJFIF() length mismatch");
		return false;
	}
	return true;
}

// Marker 0xC0 (Start Of Frame, Baseline DCT)
bool JPEGDecoder::readSOF0() {
	debug(5, "JPEG: readSOF0");
	uint16 size = _stream->readUint16BE();

	// Read the sample precision
	uint8 precision = _stream->readByte();
	if (precision != 8) {
		warning("JPEG: Just 8 bit precision supported at the moment");
		return false;
	}

	// Image size
	_h = _stream->readUint16BE();
	_w = _stream->readUint16BE();

	// Number of components
	_numComp = _stream->readByte();
	if (size != 8 + 3 * _numComp) {
		warning("JPEG: Invalid number of components");
		return false;
	}

	// Allocate the new components
	delete[] _components;
	_components = new Component[_numComp];

	// Read the components details
	for (int c = 0; c < _numComp; c++) {
		_components[c].id = _stream->readByte();
		_components[c].factorH = _stream->readByte();
		_components[c].factorV = _components[c].factorH & 0xF;
		_components[c].factorH >>= 4;
		_components[c].quantTableSelector = _stream->readByte();
	}

	return true;
}

// Marker 0xC4 (Define Huffman Tables)
bool JPEGDecoder::readDHT() {
	debug(5, "JPEG: readDHT");
	uint16 size = _stream->readUint16BE() - 2;
	uint32 pos = _stream->pos();

	while ((uint32)_stream->pos() < (size + pos)) {
		// Read the table type and id
		uint8 tableId = _stream->readByte();
		uint8 tableType = tableId >> 4; // type 0: DC, 1: AC
		tableId &= 0xF;
		uint8 tableNum = (tableId << 1) + tableType;

		// Free the Huffman table
		delete[] _huff[tableNum].values; _huff[tableNum].values = NULL;
		delete[] _huff[tableNum].sizes; _huff[tableNum].sizes = NULL;
		delete[] _huff[tableNum].codes; _huff[tableNum].codes = NULL;

		// Read the number of values for each length
		uint8 numValues[16];
		_huff[tableNum].count = 0;
		for (int len = 0; len < 16; len++) {
			numValues[len] = _stream->readByte();
			_huff[tableNum].count += numValues[len];
		}

		// Allocate memory for the current table
		_huff[tableNum].values = new uint8[_huff[tableNum].count];
		_huff[tableNum].sizes = new uint8[_huff[tableNum].count];
		_huff[tableNum].codes = new uint16[_huff[tableNum].count];

		// Read the table contents
		int cur = 0;
		for (int len = 0; len < 16; len++) {
			for (int i = 0; i < numValues[len]; i++) {
				_huff[tableNum].values[cur] = _stream->readByte();
				_huff[tableNum].sizes[cur] = len + 1;
				cur++;
			}
		}

		// Fill the table of Huffman codes
		cur = 0;
		uint16 curCode = 0;
		uint8 curCodeSize = _huff[tableNum].sizes[0];
		while (cur < _huff[tableNum].count) {
			// Increase the code size to fit the request
			while (_huff[tableNum].sizes[cur] != curCodeSize) {
				curCode <<= 1;
				curCodeSize++;
			}

			// Assign the current code
			_huff[tableNum].codes[cur] = curCode;
			curCode++;
			cur++;
		}
	}

	return true;
}

// Marker 0xDA (Start Of Scan)
bool JPEGDecoder::readSOS() {
	debug(5, "JPEG: readSOS");
	uint16 size = _stream->readUint16BE();

	// Number of scan components
	_numScanComp = _stream->readByte();
	if (size != 6 + 2 * _numScanComp) {
		warning("JPEG: Invalid number of components");
		return false;
	}

	// Allocate the new scan components
	delete[] _scanComp;
	_scanComp = new Component *[_numScanComp];

	// Reset the maximum sampling factors
	_maxFactorV = 0;
	_maxFactorH = 0;

	// Component-specification parameters
	for (int c = 0; c < _numScanComp; c++) {
		// Read the desired component id
		uint8 id = _stream->readByte();

		// Search the component with the specified id
		bool found = false;
		for (int i = 0; !found && i < _numComp; i++) {
			if (_components[i].id == id) {
				// We found the desired component
				found = true;

				// Assign the found component to the c'th scan component
				_scanComp[c] = &_components[i];
			}
		}

		if (!found) {
			warning("JPEG: Invalid component");
			return false;
		}

		// Read the entropy table selectors
		_scanComp[c]->DCentropyTableSelector = _stream->readByte();
		_scanComp[c]->ACentropyTableSelector = _scanComp[c]->DCentropyTableSelector & 0xF;
		_scanComp[c]->DCentropyTableSelector >>= 4;

		// Calculate the maximum sampling factors
		if (_scanComp[c]->factorV > _maxFactorV)
			_maxFactorV = _scanComp[c]->factorV;

		if (_scanComp[c]->factorH > _maxFactorH)
			_maxFactorH = _scanComp[c]->factorH;

		// Initialize the DC predictor
		_scanComp[c]->DCpredictor = 0;
	}

	// Start of spectral selection
	if (_stream->readByte() != 0) {
		warning("JPEG: Progressive scanning not supported");
		return false;
	}

	// End of spectral selection
	if (_stream->readByte() != 63) {
		warning("JPEG: Progressive scanning not supported");
		return false;
	}

	// Successive approximation parameters
	if (_stream->readByte() != 0) {
		warning("JPEG: Progressive scanning not supported");
		return false;
	}

	// Entropy coded sequence starts, initialize Huffman decoder
	_bitsNumber = 0;

	// Read all the scan MCUs
	uint16 xMCU = _w / (_maxFactorH * 8);
	uint16 yMCU = _h / (_maxFactorV * 8);

	// Check for non- multiple-of-8 dimensions
	if (_w % (_maxFactorH * 8) != 0)
		xMCU++;
	if (_h % (_maxFactorV * 8) != 0)
		yMCU++;

	// Initialize the scan surfaces
	for (uint16 c = 0; c < _numScanComp; c++) {
		_scanComp[c]->surface.create(xMCU * _maxFactorH * 8, yMCU * _maxFactorV * 8, PixelFormat::createFormatCLUT8());
	}

	bool ok = true;
	uint16 interval = _restartInterval;

	for (int y = 0; ok && (y < yMCU); y++) {
		for (int x = 0; ok && (x < xMCU); x++) {
			ok = readMCU(x, y);

			// If we have a restart interval, we'll need to reset a couple
			// variables
			if (_restartInterval != 0) {
				interval--;

				if (interval == 0) {
					interval = _restartInterval;
					_bitsNumber = 0;

					for (byte i = 0; i < _numScanComp; i++)
						_scanComp[i]->DCpredictor = 0;
				}
			}
		}
	}

	// Trim Component surfaces back to image height and width
	// Note: Code using jpeg must use surface.pitch correctly...
	for (uint16 c = 0; c < _numScanComp; c++) {
		_scanComp[c]->surface.w = _w;
		_scanComp[c]->surface.h = _h;
	}

	return ok;
}

// Marker 0xDB (Define Quantization Tables)
bool JPEGDecoder::readDQT() {
	debug(5, "JPEG: readDQT");
	uint16 size = _stream->readUint16BE() - 2;
	uint32 pos = _stream->pos();

	while ((uint32)_stream->pos() < (pos + size)) {
		// Read the table precision and id
		uint8 tableId = _stream->readByte();
		bool highPrecision = (tableId & 0xF0) != 0;

		// Validate the table id
		tableId &= 0xF;
		if (tableId > JPEG_MAX_QUANT_TABLES) {
			warning("JPEG: Invalid number of components");
			return false;
		}

		// Create the new table if necessary
		if (!_quant[tableId])
			_quant[tableId] = new uint16[64];

		// Read the table (stored in Zig-Zag order)
		for (int i = 0; i < 64; i++)
			_quant[tableId][i] = highPrecision ? _stream->readUint16BE() : _stream->readByte();
	}

	return true;
}

// Marker 0xDD (Define Restart Interval)
bool JPEGDecoder::readDRI() {
	debug(5, "JPEG: readDRI");
	uint16 size = _stream->readUint16BE() - 2;

	if (size != 2) {
		warning("JPEG: Invalid DRI size %d", size);
		return false;
	}

	_restartInterval = _stream->readUint16BE();
	debug(5, "Restart interval: %d", _restartInterval);
	return true;
}

bool JPEGDecoder::readMCU(uint16 xMCU, uint16 yMCU) {
	bool ok = true;
	for (int c = 0; ok && (c < _numComp); c++) {
		// Set the current component
		_currentComp = _scanComp[c];

		// Read the data units of the current component
		for (int y = 0; ok && (y < _scanComp[c]->factorV); y++)
			for (int x = 0; ok && (x < _scanComp[c]->factorH); x++)
				ok = readDataUnit(xMCU * _scanComp[c]->factorH + x, yMCU * _scanComp[c]->factorV + y);
	}

	return ok;
}

// triple-butterfly-add (and possible rounding)
#define xadd3(xa, xb, xc, xd, h) \
	p = xa + xb; \
	n = xa - xb; \
	xa = p + xc + h; \
	xb = n + xd + h; \
	xc = p - xc + h; \
	xd = n - xd + h;

// butterfly-mul
#define xmul(xa, xb, k1, k2, sh) \
	n = k1 * (xa + xb); \
	p = xa; \
	xa = (n + (k2 - k1) * xb) >> sh; \
	xb = (n - (k2 + k1) * p) >> sh;

// IDCT based on public domain code from http://halicery.com/jpeg/idct.html
void JPEGDecoder::idct1D8x8(int32 src[8], int32 dest[64], int32 ps, int32 half) {
	int p, n;

	src[0] <<= 9;
	src[1] <<= 7;
	src[3] *= 181;
	src[4] <<= 9;
	src[5] *= 181;
	src[7] <<= 7;

	// Even part
	xmul(src[6], src[2], 277, 669, 0)
	xadd3(src[0], src[4], src[6], src[2], half)

	// Odd part
	xadd3(src[1], src[7], src[3], src[5], 0)
	xmul(src[5], src[3], 251, 50, 6)
	xmul(src[1], src[7], 213, 142, 6)

	dest[0 * 8] = (src[0] + src[1]) >> ps;
	dest[1 * 8] = (src[4] + src[5]) >> ps;
	dest[2 * 8] = (src[2] + src[3]) >> ps;
	dest[3 * 8] = (src[6] + src[7]) >> ps;
	dest[4 * 8] = (src[6] - src[7]) >> ps;
	dest[5 * 8] = (src[2] - src[3]) >> ps;
	dest[6 * 8] = (src[4] - src[5]) >> ps;
	dest[7 * 8] = (src[0] - src[1]) >> ps;
}

void JPEGDecoder::idct2D8x8(int32 block[64]) {
	int32 tmp[64];

	// Apply 1D IDCT to rows
	for (int i = 0; i < 8; i++)
		idct1D8x8(&block[i * 8], &tmp[i], 9, 1 << 8);

	// Apply 1D IDCT to columns
	for (int i = 0; i < 8; i++)
		idct1D8x8(&tmp[i * 8], &block[i], 12, 1 << 11);
 }

bool JPEGDecoder::readDataUnit(uint16 x, uint16 y) {
	// Prepare an empty data array
	int16 readData[64];
	for (int i = 1; i < 64; i++)
		readData[i] = 0;

	// Read the DC component
	readData[0] = _currentComp->DCpredictor + readDC();
	_currentComp->DCpredictor = readData[0];

	// Read the AC components (stored in Zig-Zag)
	readAC(readData);

	// Calculate the DCT coefficients from the input sequence
	int32 block[64];
	for (uint8 i = 0; i < 64; i++) {
		// Dequantize
		int32 val = readData[i];
		int16 quant = _quant[_currentComp->quantTableSelector][i];
		val *= quant;

		// Store the normalized coefficients, undoing the Zig-Zag
		block[_zigZagOrder[i]] = val;
	}

	// Apply the IDCT
	idct2D8x8(block);

	// Level shift to make the values unsigned
	for (int i = 0; i < 64; i++) {
		block[i] = block[i] + 128;

		if (block[i] < 0)
			block[i] = 0;

		if (block[i] > 255)
			block[i] = 255;
	}

	// Paint the component surface
	uint8 scalingV = _maxFactorV / _currentComp->factorV;
	uint8 scalingH = _maxFactorH / _currentComp->factorH;

	// Convert coordinates from MCU blocks to pixels
	x <<= 3;
	y <<= 3;

	for (uint8 j = 0; j < 8; j++) {
		for (uint16 sV = 0; sV < scalingV; sV++) {
			// Get the beginning of the block line
			byte *ptr = (byte *)_currentComp->surface.getBasePtr(x * scalingH, (y + j) * scalingV + sV);

			for (uint8 i = 0; i < 8; i++) {
				for (uint16 sH = 0; sH < scalingH; sH++) {
					*ptr = (byte)(block[j * 8 + i]);
					ptr++;
				}
			}
		}
	}

	return true;
}

int16 JPEGDecoder::readDC() {
	// DC is type 0
	uint8 tableNum = _currentComp->DCentropyTableSelector << 1;

	// Get the number of bits to read
	uint8 numBits = readHuff(tableNum);

	// Read the requested bits
	return readSignedBits(numBits);
}

void JPEGDecoder::readAC(int16 *out) {
	// AC is type 1
	uint8 tableNum = (_currentComp->ACentropyTableSelector << 1) + 1;

	// Start reading AC element 1
	uint8 cur = 1;
	while (cur < 64) {
		uint8 s = readHuff(tableNum);
		uint8 r = s >> 4;
		s &= 0xF;

		if (s == 0) {
			if (r == 15) {
				// Skip 16 values
				cur += 16;
			} else {
				// EOB: end of block
				cur = 64;
			}
		} else {
			// Skip r values
			cur += r;

			// Read the next value
			out[cur] = readSignedBits(s);
			cur++;
		}
	}
}

int16 JPEGDecoder::readSignedBits(uint8 numBits) {
	uint16 ret = 0;
	if (numBits > 16)
		error("requested %d bits", numBits); //XXX

	// MSB=0 for negatives, 1 for positives
	for (int i = 0; i < numBits; i++)
		ret = (ret << 1) + readBit();

	// Extend sign bits (PAG109)
	if (!(ret >> (numBits - 1))) {
		uint16 tmp = ((uint16)-1 << numBits) + 1;
		ret = ret + tmp;
	}
	return ret;
}

// TODO: optimize?
uint8 JPEGDecoder::readHuff(uint8 table) {
	bool foundCode = false;
	uint8 val = 0;

	uint8 cur = 0;
	uint8 codeSize = 1;
	uint16 code = readBit();
	while (!foundCode) {
		// Prepare a code of the current size
		while (codeSize < _huff[table].sizes[cur]) {
			code = (code << 1) + readBit();
			codeSize++;
		}

		// Compare the codes of the current size
		while (!foundCode && (codeSize == _huff[table].sizes[cur])) {
			if (code == _huff[table].codes[cur]) {
				// Found the code
				val = _huff[table].values[cur];
				foundCode = true;
			} else {
				// Continue reading
				cur++;
			}
		}
	}

	return val;
}

uint8 JPEGDecoder::readBit() {
	// Read a whole byte if necessary
	if (_bitsNumber == 0) {
		_bitsData = _stream->readByte();
		_bitsNumber = 8;

		// Detect markers
		if (_bitsData == 0xFF) {
			uint8 byte2 = _stream->readByte();

			// A stuffed 0 validates the previous byte
			if (byte2 != 0) {
				if (byte2 == 0xDC) {
					// DNL marker: Define Number of Lines
					// TODO: terminate scan
					warning("DNL marker detected: terminate scan");
				} else if (byte2 >= 0xD0 && byte2 <= 0xD7) {
					debug(7, "RST%d marker detected", byte2 & 7);
					_bitsData = _stream->readByte();
				} else {
					warning("Error: marker 0x%02X read in entropy data", byte2);
				}
			}
		}
	}
	_bitsNumber--;

	return (_bitsData & (1 << _bitsNumber)) ? 1 : 0;
}

const Surface *JPEGDecoder::getComponent(uint c) const {
	for (int i = 0; i < _numComp; i++)
		if (_components[i].id == c) // We found the desired component
			return &_components[i].surface;

	error("JPEGDecoder::getComponent: No component %d present", c);
	return NULL;
}

} // End of Graphics namespace
