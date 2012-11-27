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

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resmanager.h" // for PRECACHE_RESOURCES
#include "sword25/package/packagemanager.h"

#include "sword25/gfx/fontresource.h"

namespace Sword25 {

enum {
	DEFAULT_LINEHEIGHT = 20,
	DEFAULT_GAPWIDTH = 1
};

FontResource::FontResource(Kernel *pKernel, const Common::String &fileName) :
	_pKernel(pKernel),
	_valid(false),
	Resource(fileName, Resource::TYPE_FONT),
	Common::XMLParser() {

	// Get a pointer to the package manager
	assert(_pKernel);
	PackageManager *pPackage = _pKernel->getPackage();
	assert(pPackage);

	// Load the contents of the file
	uint fileSize;
	char *xmlData = pPackage->getXmlFile(getFileName(), &fileSize);
	if (!xmlData) {
		error("Could not read \"%s\".", getFileName().c_str());
		return;
	}

	// Parse the contents
	if (!loadBuffer((const byte *)xmlData, fileSize))
		return;

	_valid = parse();
	close();
	free(xmlData);
}

bool FontResource::parserCallback_font(ParserNode *node) {
	// Get the attributes of the font
	Common::String bitmapFilename = node->values["bitmap"];

	if (!parseIntegerKey(node->values["lineheight"], 1, &_lineHeight)) {
		warning("Illegal or missing lineheight attribute in <font> tag in \"%s\". Assuming default (\"%d\").",
		                 getFileName().c_str(), DEFAULT_LINEHEIGHT);
		_lineHeight = DEFAULT_LINEHEIGHT;
	}

	if (!parseIntegerKey(node->values["gap"], 1, &_gapWidth)) {
		warning("Illegal or missing gap attribute in <font> tag in \"%s\". Assuming default (\"%d\").",
		                 getFileName().c_str(), DEFAULT_GAPWIDTH);
		_gapWidth = DEFAULT_GAPWIDTH;
	}

	// Get a reference to the package manager
	assert(_pKernel);
	PackageManager *pPackage = _pKernel->getPackage();
	assert(pPackage);

	// Get the full path and filename for the bitmap resource
	_bitmapFileName = pPackage->getAbsolutePath(bitmapFilename);
	if (_bitmapFileName == "") {
		error("Image file \"%s\" was specified in <font> tag of \"%s\" but could not be found.",
		               _bitmapFileName.c_str(), getFileName().c_str());
	}

#ifdef PRECACHE_RESOURCES
	// Pre-cache the resource
	if (!_pKernel->getResourceManager()->precacheResource(_bitmapFileName)) {
		error("Could not precache \"%s\".", _bitmapFileName.c_str());
	}
#else
	_pKernel->getResourceManager()->requestResource(_bitmapFileName);
#endif

	return true;
}

bool FontResource::parserCallback_character(ParserNode *node) {
	// Get the attributes of the character
	int charCode, top, left, right, bottom;

	if (!parseIntegerKey(node->values["code"], 1, &charCode) || (charCode < 0) || (charCode >= 256)) {
		return parserError("Illegal or missing code attribute in <character> tag in '" + getFileName() + "'.");
	}

	if (!parseIntegerKey(node->values["top"], 1, &top) || (top < 0)) {
		return parserError("Illegal or missing top attribute in <character> tag in '" + getFileName() + "'.");
	}
	if (!parseIntegerKey(node->values["left"], 1, &left) || (left < 0)) {
		return parserError("Illegal or missing left attribute in <character> tag in '" + getFileName() + "'.");
	}
	if (!parseIntegerKey(node->values["right"], 1, &right) || (right < 0)) {
		return parserError("Illegal or missing right attribute in <character> tag in '" + getFileName() + "'.");
	}
	if (!parseIntegerKey(node->values["bottom"], 1, &bottom) || (bottom < 0)) {
		return parserError("Illegal or missing bottom attribute in <character> tag in '" + getFileName() + "'.");
	}

	_characterRects[charCode] = Common::Rect(left, top, right, bottom);
	return true;
}

} // End of namespace Sword25
