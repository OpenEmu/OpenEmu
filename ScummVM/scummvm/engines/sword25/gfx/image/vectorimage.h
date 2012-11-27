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

#ifndef SWORD25_VECTORIMAGE_H
#define SWORD25_VECTORIMAGE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/image.h"
#include "common/rect.h"

#include "art.h"

namespace Sword25 {

class VectorImage;

/**
    @brief Pfadinformationen zu BS_VectorImageElement Objekten

    Jedes BS_VectorImageElement besteht aus Kantenzügen, oder auch Pfaden. Jeder dieser Pfad hat Eigenschaften, die in Objekten diesen Typs
    gespeichert werden.
*/

class VectorPathInfo {
public:
	VectorPathInfo(ArtBpath *vec, int len, uint lineStyle, uint fillStyle0, uint fillStyle1) :
		_vec(vec), _lineStyle(lineStyle), _fillStyle0(fillStyle0), _fillStyle1(fillStyle1), _len(len) {}

	VectorPathInfo() {
		_lineStyle = _fillStyle0 = _fillStyle1 = _len = 0;
		_vec = 0;
	}

	ArtBpath *getVec() const {
		return _vec;
	}
	int getVecLen() const {
		return _len;
	}
	uint getLineStyle() const {
		return _lineStyle;
	}
	uint getFillStyle0() const {
		return _fillStyle0;
	}
	uint getFillStyle1() const {
		return _fillStyle1;
	}

private:
	ArtBpath *_vec;
	uint _lineStyle;
	uint _fillStyle0;
	uint _fillStyle1;
	uint _len;
};

/**
    @brief Ein Element eines Vektorbild. Ein BS_VectorImage besteht aus diesen Elementen, die jeweils einen Teil der Graphik definieren.
           Werden alle Elemente eines Vektorbildes übereinandergelegt, ergibt sich das komplette Bild.
*/
class VectorImageElement {
	friend class VectorImage;
public:
	uint getPathCount() const {
		return _pathInfos.size();
	}
	const VectorPathInfo &getPathInfo(uint pathNr) const {
		assert(pathNr < getPathCount());
		return _pathInfos[pathNr];
	}

	double getLineStyleWidth(uint lineStyle) const {
		assert(lineStyle < _lineStyles.size());
		return _lineStyles[lineStyle].width;
	}

	uint getLineStyleCount() const {
		return _lineStyles.size();
	}

	uint32 getLineStyleColor(uint lineStyle) const {
		assert(lineStyle < _lineStyles.size());
		return _lineStyles[lineStyle].color;
	}

	uint getFillStyleCount() const {
		return _fillStyles.size();
	}

	uint32 getFillStyleColor(uint fillStyle) const {
		assert(fillStyle < _fillStyles.size());
		return _fillStyles[fillStyle];
	}

	const Common::Rect &getBoundingBox() const {
		return _boundingBox;
	}

private:
	struct LineStyleType {
		LineStyleType(double width_, uint32 color_) : width(width_), color(color_) {}
		LineStyleType() {
			width = 0;
			color = 0;
		}
		double width;
		uint32 color;
	};

	Common::Array<VectorPathInfo> _pathInfos;
	Common::Array<LineStyleType> _lineStyles;
	Common::Array<uint32>  _fillStyles;
	Common::Rect _boundingBox;
};


/**
    @brief Eine Vektorgraphik

    Objekte dieser Klasse enthalten die Informationen eines SWF-Shapes.
*/

class VectorImage : public Image {
public:
	VectorImage(const byte *pFileData, uint fileSize, bool &success, const Common::String &fname);
	~VectorImage();

	uint getElementCount() const {
		return _elements.size();
	}
	const VectorImageElement &getElement(uint elementNr) const {
		assert(elementNr < _elements.size());
		return _elements[elementNr];
	}
	const Common::Rect &getBoundingBox() const {
		return _boundingBox;
	}

	//
	// Die abstrakten Methoden von BS_Image
	//
	virtual int getWidth() const {
		return _boundingBox.width();
	}
	virtual int getHeight() const {
		return _boundingBox.height();
	}
	virtual GraphicEngine::COLOR_FORMATS getColorFormat() const {
		return GraphicEngine::CF_ARGB32;
	}
	virtual bool fill(const Common::Rect *pFillRect = 0, uint color = BS_RGB(0, 0, 0));

	void render(int width, int height);

	virtual uint getPixel(int x, int y);
	virtual bool isBlitSource() const {
		return true;
	}
	virtual bool isBlitTarget() const {
		return false;
	}
	virtual bool isScalingAllowed() const {
		return true;
	}
	virtual bool isFillingAllowed() const {
		return false;
	}
	virtual bool isAlphaAllowed() const {
		return true;
	}
	virtual bool isColorModulationAllowed() const {
		return true;
	}
	virtual bool isSetContentAllowed() const {
		return false;
	}
	virtual bool setContent(const byte *pixeldata, uint size, uint offset, uint stride);
	virtual bool blit(int posX = 0, int posY = 0,
	                  int flipping = FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  uint color = BS_ARGB(255, 255, 255, 255),
	                  int width = -1, int height = -1);

	class SWFBitStream;

private:
	bool parseDefineShape(uint shapeType, SWFBitStream &bs);
	bool parseStyles(uint shapeType, SWFBitStream &bs, uint &numFillBits, uint &numLineBits);

	ArtBpath *storeBez(ArtBpath *bez, int lineStyle, int fillStyle0, int fillStyle1, int *bezNodes, int *bezAllocated);
	Common::Array<VectorImageElement>    _elements;
	Common::Rect                         _boundingBox;

	byte *_pixelData;

	Common::String _fname;
};

} // End of namespace Sword25

#endif
