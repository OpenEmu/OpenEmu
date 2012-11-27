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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/gfx/image/art.h"
#include "sword25/gfx/image/vectorimage.h"
#include "sword25/gfx/image/renderedimage.h"

#include "graphics/colormasks.h"

namespace Sword25 {

#define BEZSMOOTHNESS 0.5

// -----------------------------------------------------------------------------
// SWF datatype
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Bitstream helper class
// -----------------------------------------------------------------------------
// The parsing of SWF files requires both bitwise readout and on Byte boundaries
// oriented reading.
// This class is specially equipped for this.
// -----------------------------------------------------------------------------

class VectorImage::SWFBitStream {
public:
	SWFBitStream(const byte *pData, uint dataSize) :
		m_Pos(pData), m_End(pData + dataSize), m_WordMask(0)
	{}

	inline uint32 getBits(uint bitCount) {
		if (bitCount == 0 || bitCount > 32) {
			error("SWFBitStream::GetBits() must read at least 1 and at most 32 bits at a time");
		}

		uint32 value = 0;
		while (bitCount) {
			if (m_WordMask == 0)
				flushByte();

			value <<= 1;
			value |= ((m_Word & m_WordMask) != 0) ? 1 : 0;
			m_WordMask >>= 1;

			--bitCount;
		}

		return value;
	}

	inline int32 getSignedBits(uint bitCount) {
		// readout bits
		uint32 temp = getBits(bitCount);

		// If the sign-bit is set, fill the rest of the return value with 1-bit (sign extension)
		if (temp & 1 << (bitCount - 1))
			return (0xffffffff << bitCount) | temp;
		else
			return temp;
	}

	inline uint32 getUInt32() {
		uint32 byte1 = getByte();
		uint32 byte2 = getByte();
		uint32 byte3 = getByte();
		uint32 byte4 = getByte();

		return byte1 | (byte2 << 8) | (byte3 << 16) | (byte4 << 24);
	}

	inline uint16 getUInt16() {
		uint32 byte1 = getByte();
		uint32 byte2 = getByte();

		return byte1 | (byte2 << 8);
	}

	inline byte getByte() {
		flushByte();
		byte value = m_Word;
		m_WordMask = 0;
		flushByte();

		return value;
	}

	inline void flushByte() {
		if (m_WordMask != 128) {
			if (m_Pos >= m_End) {
				error("Attempted to read past end of file");
			} else {
				m_Word = *m_Pos++;
				m_WordMask = 128;
			}
		}
	}

	inline void skipBytes(uint skipLength) {
		flushByte();
		if (m_Pos + skipLength >= m_End) {
			error("Attempted to read past end of file");
		} else {
			m_Pos += skipLength;
			m_Word = *(m_Pos - 1);
		}
	}

private:
	const byte    *m_Pos;
	const byte    *m_End;

	byte m_Word;
	uint m_WordMask;
};


// -----------------------------------------------------------------------------
// Constants and utility functions
// -----------------------------------------------------------------------------

namespace {
// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

const uint32 MAX_ACCEPTED_FLASH_VERSION = 3;   // The maximum flash file version that is accepted by the loader


// -----------------------------------------------------------------------------
// Converts SWF rectangle data in a bit stream in Common::Rect objects
// -----------------------------------------------------------------------------

Common::Rect flashRectToBSRect(VectorImage::SWFBitStream &bs) {
	bs.flushByte();

	// Determines how many bits of the single components are encoded
	uint32 bitsPerValue = bs.getBits(5);

	// Readout the single components
	int32 xMin = bs.getSignedBits(bitsPerValue);
	int32 xMax = bs.getSignedBits(bitsPerValue);
	int32 yMin = bs.getSignedBits(bitsPerValue);
	int32 yMax = bs.getSignedBits(bitsPerValue);

	return Common::Rect(xMin, yMin, xMax + 1, yMax + 1);
}

// -----------------------------------------------------------------------------
// Calculate the bounding box of a BS_VectorImageElement
// -----------------------------------------------------------------------------

Common::Rect CalculateBoundingBox(const VectorImageElement &vectorImageElement) {
	double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;

	for (int j = vectorImageElement.getPathCount() - 1; j >= 0; j--) {
		ArtBpath *bez = vectorImageElement.getPathInfo(j).getVec();
		ArtVpath *vec = art_bez_path_to_vec(bez, 0.5);

		if (vec[0].code == ART_END) {
			continue;
		} else {
			x0 = x1 = vec[0].x;
			y0 = y1 = vec[0].y;
			for (int i = 1; vec[i].code != ART_END; i++) {
				if (vec[i].x < x0) x0 = vec[i].x;
				if (vec[i].x > x1) x1 = vec[i].x;
				if (vec[i].y < y0) y0 = vec[i].y;
				if (vec[i].y > y1) y1 = vec[i].y;
			}
		}
		free(vec);
	}

	return Common::Rect(static_cast<int>(x0), static_cast<int>(y0), static_cast<int>(x1) + 1, static_cast<int>(y1) + 1);
}

}


// -----------------------------------------------------------------------------
// Construction
// -----------------------------------------------------------------------------

VectorImage::VectorImage(const byte *pFileData, uint fileSize, bool &success, const Common::String &fname) : _pixelData(0), _fname(fname) {
	success = false;

	// Create bitstream object
	// In the following the file data will be readout of the bitstream object.
	SWFBitStream bs(pFileData, fileSize);

	// Check SWF signature
	uint32 signature[3];
	signature[0] = bs.getByte();
	signature[1] = bs.getByte();
	signature[2] = bs.getByte();
	if (signature[0] != 'F' ||
	        signature[1] != 'W' ||
	        signature[2] != 'S') {
		error("File is not a valid SWF-file");
		return;
	}

	// Check the version
	uint32 version = bs.getByte();
	if (version > MAX_ACCEPTED_FLASH_VERSION) {
		error("File is of version %d. Highest accepted version is %d.", version, MAX_ACCEPTED_FLASH_VERSION);
		return;
	}

	// Readout filesize and compare with the actual size
	uint32 storedFileSize = bs.getUInt32();
	if (storedFileSize != fileSize) {
		error("File is not a valid SWF-file");
		return;
	}

	// readout SWF size
	flashRectToBSRect(bs);

	// Get frame rate and frame count
	/* uint32 frameRate = */
	bs.getUInt16();
	/* uint32 frameCount = */
	bs.getUInt16();

	// Parse tags
	// Because we are only interested in the first DifneShape-Tag...
	bool keepParsing = true;
	while (keepParsing) {
		// Tags always begin on byte boundaries
		bs.flushByte();

		// Readout tag type and length
		uint16 tagTypeAndLength = bs.getUInt16();
		uint32 tagType = tagTypeAndLength >> 6;
		uint32 tagLength = tagTypeAndLength & 0x3f;
		if (tagLength == 0x3f)
			tagLength = bs.getUInt32();

		switch (tagType) {
		case 2:
			// DefineShape
			success = parseDefineShape(2, bs);
			return;
		case 22:
			// DefineShape2
			success = parseDefineShape(2, bs);
			return;
		case 32:
			success = parseDefineShape(3, bs);
			return;
		default:
			// Ignore unknown tags
			bs.skipBytes(tagLength);
		}
	}

	// The execution must not arrive at this point: Either a shape is found, then the function will be leaved before, or it is found none, then
	// an exception occurs as soon as it is read beyond of the end of file.
	assert(false);
}

VectorImage::~VectorImage() {
	for (int j = _elements.size() - 1; j >= 0; j--)
		for (int i = _elements[j].getPathCount() - 1; i >= 0; i--)
			if (_elements[j].getPathInfo(i).getVec())
				free(_elements[j].getPathInfo(i).getVec());

	if (_pixelData)
		free(_pixelData);
}


ArtBpath *ensureBezStorage(ArtBpath *bez, int nodes, int *allocated) {
	if (*allocated <= nodes) {
		(*allocated) += 20;

		return art_renew(bez, ArtBpath, *allocated);
	}

	return bez;
}

ArtBpath *VectorImage::storeBez(ArtBpath *bez, int lineStyle, int fillStyle0, int fillStyle1, int *bezNodes, int *bezAllocated) {
	(*bezNodes)++;

	bez = ensureBezStorage(bez, *bezNodes, bezAllocated);
	bez[*bezNodes].code = ART_END;

	ArtBpath *bez1 = art_new(ArtBpath, *bezNodes + 1);
	if (!bez1)
		error("[VectorImage::storeBez] Cannot allocate memory");

	for (int i = 0; i <= *bezNodes; i++)
		bez1[i] = bez[i];

	_elements.back()._pathInfos.push_back(VectorPathInfo(bez1, *bezNodes, lineStyle, fillStyle0, fillStyle1));

	return bez;
}

bool VectorImage::parseDefineShape(uint shapeType, SWFBitStream &bs) {
	/*uint32 shapeID = */bs.getUInt16();

	// readout bounding box
	_boundingBox = flashRectToBSRect(bs);

	// create first image element
	_elements.resize(1);

	// read styles
	uint numFillBits;
	uint numLineBits;
	if (!parseStyles(shapeType, bs, numFillBits, numLineBits))
		return false;

	uint lineStyle = 0;
	uint fillStyle0 = 0;
	uint fillStyle1 = 0;

	// parse shaperecord
	// ------------------

	double curX = 0;
	double curY = 0;
	int bezNodes = 0;
	int bezAllocated = 10;
	ArtBpath *bez = art_new(ArtBpath, bezAllocated);

	bool endOfShapeDiscovered = false;
	while (!endOfShapeDiscovered) {
		uint32 typeFlag = bs.getBits(1);

		// Non-Edge Record
		if (typeFlag == 0) {
			// Determines which parameters are set
			uint32 stateNewStyles = bs.getBits(1);
			uint32 stateLineStyle = bs.getBits(1);
			uint32 stateFillStyle1 = bs.getBits(1);
			uint32 stateFillStyle0 = bs.getBits(1);
			uint32 stateMoveTo = bs.getBits(1);

			uint prevLineStyle = lineStyle;
			uint prevFillStyle0 = fillStyle0;
			uint prevFillStyle1 = fillStyle1;

			// End of the shape definition is reached?
			if (!stateNewStyles && !stateLineStyle && !stateFillStyle0 && !stateFillStyle1 && !stateMoveTo) {
				endOfShapeDiscovered = true;
				// Decode parameters
			} else {
				if (stateMoveTo) {
					uint32 moveToBits = bs.getBits(5);
					curX = bs.getSignedBits(moveToBits);
					curY = bs.getSignedBits(moveToBits);
				}

				if (stateFillStyle0) {
					if (numFillBits > 0)
						fillStyle0 = bs.getBits(numFillBits);
					else
						fillStyle0 = 0;
				}

				if (stateFillStyle1) {
					if (numFillBits > 0)
						fillStyle1 = bs.getBits(numFillBits);
					else
						fillStyle1 = 0;
				}

				if (stateLineStyle) {
					if (numLineBits)
						lineStyle = bs.getBits(numLineBits);
					else
						numLineBits = 0;
				}

				// Create a new path, unless there were only defined new styles
				if (stateLineStyle || stateFillStyle0 || stateFillStyle1 || stateMoveTo) {
					// Store previous curve if any
					if (bezNodes) {
						bez = storeBez(bez, prevLineStyle, prevFillStyle0, prevFillStyle1, &bezNodes, &bezAllocated);
					}

					// Start new curve
					bez = ensureBezStorage(bez, 1, &bezAllocated);
					bez[0].code = ART_MOVETO_OPEN;
					bez[0].x3 = curX;
					bez[0].y3 = curY;
					bezNodes = 0;
				}

				if (stateNewStyles) {
					// The old style definitions will be discarded and overwritten with the new at this point in Flash.
					// A new element will be started with a new element.
					_elements.resize(_elements.size() + 1);
					if (!parseStyles(shapeType, bs, numFillBits, numLineBits))
						return false;
				}
			}
		} else {
			// Edge record
			uint32 edgeFlag = bs.getBits(1);
			uint32 numBits = bs.getBits(4) + 2;

			// Curved edge
			if (edgeFlag == 0) {
				double controlDeltaX = bs.getSignedBits(numBits);
				double controlDeltaY = bs.getSignedBits(numBits);
				double anchorDeltaX = bs.getSignedBits(numBits);
				double anchorDeltaY = bs.getSignedBits(numBits);

				double controlX = curX + controlDeltaX;
				double controlY = curY + controlDeltaY;
				double newX = controlX + anchorDeltaX;
				double newY = controlY + anchorDeltaY;

#define WEIGHT (2.0/3.0)

				bezNodes++;
				bez = ensureBezStorage(bez, bezNodes, &bezAllocated);
				bez[bezNodes].code = ART_CURVETO;
				bez[bezNodes].x1 = WEIGHT * controlX + (1 - WEIGHT) * curX;
				bez[bezNodes].y1 = WEIGHT * controlY + (1 - WEIGHT) * curY;
				bez[bezNodes].x2 = WEIGHT * controlX + (1 - WEIGHT) * newX;
				bez[bezNodes].y2 = WEIGHT * controlY + (1 - WEIGHT) * newY;
				bez[bezNodes].x3 = newX;
				bez[bezNodes].y3 = newY;

				curX = newX;
				curY = newY;
			} else {
				// Staight edge
				int32 deltaX = 0;
				int32 deltaY = 0;

				uint32 generalLineFlag = bs.getBits(1);
				if (generalLineFlag) {
					deltaX = bs.getSignedBits(numBits);
					deltaY = bs.getSignedBits(numBits);
				} else {
					uint32 vertLineFlag = bs.getBits(1);
					if (vertLineFlag)
						deltaY = bs.getSignedBits(numBits);
					else
						deltaX = bs.getSignedBits(numBits);
				}

				curX += deltaX;
				curY += deltaY;

				bezNodes++;
				bez = ensureBezStorage(bez, bezNodes, &bezAllocated);
				bez[bezNodes].code = ART_LINETO;
				bez[bezNodes].x3 = curX;
				bez[bezNodes].y3 = curY;
			}
		}
	}

	// Store last curve
	if (bezNodes)
		bez = storeBez(bez, lineStyle, fillStyle0, fillStyle1, &bezNodes, &bezAllocated);

	free(bez);

	// Calculate the bounding boxes of each element
	Common::Array<VectorImageElement>::iterator it = _elements.begin();
	for (; it != _elements.end(); ++it)
		it->_boundingBox = CalculateBoundingBox(*it);

	return true;
}


// -----------------------------------------------------------------------------

bool VectorImage::parseStyles(uint shapeType, SWFBitStream &bs, uint &numFillBits, uint &numLineBits) {
	bs.flushByte();

	// Parse fill styles
	// -----------------

	// Determine number of fill styles
	uint fillStyleCount = bs.getByte();
	if (fillStyleCount == 0xff)
		fillStyleCount = bs.getUInt16();

	// Readout all fill styles. If a fill style with Typ != 0 is found, the parsing is aborted.
	// Only "solid fill" (Typ 0) is supported.
	_elements.back()._fillStyles.reserve(fillStyleCount);
	for (uint i = 0; i < fillStyleCount; ++i) {
		byte type = bs.getByte();
		uint32 color;
		byte r = bs.getByte();
		byte g = bs.getByte();
		byte b = bs.getByte();
		byte a = 0xff;

		if (shapeType == 3)
			a = bs.getByte();

		color = Graphics::ARGBToColor<Graphics::ColorMasks<8888> >(a, r, g, b);

		if (type != 0)
			return false;

		_elements.back()._fillStyles.push_back(color);
	}

	// Line styles parsen
	// -----------------

	// Determine number of line styles
	uint lineStyleCount = bs.getByte();
	if (lineStyleCount == 0xff)
		lineStyleCount = bs.getUInt16();

	// Readout all line styles
	_elements.back()._lineStyles.reserve(lineStyleCount);
	for (uint i = 0; i < lineStyleCount; ++i) {
		double width = bs.getUInt16();
		uint32 color;
		byte r = bs.getByte();
		byte g = bs.getByte();
		byte b = bs.getByte();
		byte a = 0xff;

		if (shapeType == 3)
			a = bs.getByte();

		color = Graphics::ARGBToColor<Graphics::ColorMasks<8888> >(a, r, g, b);

		_elements.back()._lineStyles.push_back(VectorImageElement::LineStyleType(width, color));
	}

	// Readout the bit width for the following style indices
	numFillBits = bs.getBits(4);
	numLineBits = bs.getBits(4);

	return true;
}


// -----------------------------------------------------------------------------

bool VectorImage::fill(const Common::Rect *pFillRect, uint color) {
	error("Fill() is not supported.");
	return false;
}


// -----------------------------------------------------------------------------

uint VectorImage::getPixel(int x, int y) {
	error("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool VectorImage::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	error("SetContent() is not supported.");
	return 0;
}

bool VectorImage::blit(int posX, int posY,
                       int flipping,
                       Common::Rect *pPartRect,
                       uint color,
                       int width, int height) {
	static VectorImage *oldThis = 0;
	static int              oldWidth = -2;
	static int              oldHeight = -2;

	// If width or height to 0, nothing needs to be shown.
	if (width == 0 || height == 0)
		return true;

	// Determine if the old image in the cache can not be reused and must be recalculated
	if (!(oldThis == this && oldWidth == width && oldHeight == height)) {
		render(width, height);

		oldThis = this;
		oldHeight = height;
		oldWidth = width;
	}

	RenderedImage *rend = new RenderedImage();

	rend->replaceContent(_pixelData, width, height);
	rend->blit(posX, posY, flipping, pPartRect, color, width, height);

	delete rend;

	return true;
}

} // End of namespace Sword25
