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

#include "sword25/gfx/dynamicbitmap.h"
#include "sword25/gfx/bitmapresource.h"
#include "sword25/package/packagemanager.h"
#include "sword25/kernel/inputpersistenceblock.h"

namespace Sword25 {

DynamicBitmap::DynamicBitmap(RenderObjectPtr<RenderObject> parentPtr, uint width, uint height) :
	Bitmap(parentPtr, TYPE_DYNAMICBITMAP) {
	// The BS_BITMAP could not be created, so stop.
	if (!_initSuccess)
		return;

	_initSuccess = createRenderedImage(width, height);
}

DynamicBitmap::DynamicBitmap(InputPersistenceBlock &reader, RenderObjectPtr<RenderObject> parentPtr, uint handle) :
	Bitmap(parentPtr, TYPE_DYNAMICBITMAP, handle) {
	_initSuccess = unpersist(reader);
}

bool DynamicBitmap::createRenderedImage(uint width, uint height) {
	bool result = false;
	_image.reset(new RenderedImage(width, height, result));

	_originalWidth = _width = width;
	_originalHeight = _height = height;

	return result;
}

DynamicBitmap::~DynamicBitmap() {
}

uint DynamicBitmap::getPixel(int x, int y) const {
	assert(x >= 0 && x < _width);
	assert(y >= 0 && y < _height);

	return _image->getPixel(x, y);
}

bool DynamicBitmap::doRender() {
	// Get the frame buffer object
	GraphicEngine *pGfx = Kernel::getInstance()->getGfx();
	assert(pGfx);

	// Draw the bitmap
	bool result;
	if (_scaleFactorX == 1.0f && _scaleFactorY == 1.0f) {
#if 1
		// This is what the game does originally, which can be
		// a bit slow when drawing videos, but it's not the main
		// bottleneck.
		result = _image->blit(_absoluteX, _absoluteY,
		                       (_flipV ? BitmapResource::FLIP_V : 0) |
		                       (_flipH ? BitmapResource::FLIP_H : 0),
		                       0, _modulationColor, -1, -1);
#else
		// WIP: A bit faster code

		// We don't need to check for transparency when drawing
		// videos, thus just copy the buffer contents directly.
		// This messes up the fire animation in the menu, for
		// some odd reason. It also makes the video colors a
		// bit lighter, resulting in a visible black border
		// around the typed letters in the intro (which are
		// drawn over a black border)
		_image->copyDirectly(_absoluteX, _absoluteY);
#endif
		return true;
	} else {
		result = _image->blit(_absoluteX, _absoluteY,
		                       (_flipV ? BitmapResource::FLIP_V : 0) |
		                       (_flipH ? BitmapResource::FLIP_H : 0),
		                       0, _modulationColor, _width, _height);
	}

	return result;
}

bool DynamicBitmap::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	return _image->setContent(pixeldata, size, offset, stride);
}

bool DynamicBitmap::isScalingAllowed() const {
	return _image->isScalingAllowed();
}

bool DynamicBitmap::isAlphaAllowed() const {
	return _image->isAlphaAllowed();
}

bool DynamicBitmap::isColorModulationAllowed() const {
	return _image->isColorModulationAllowed();
}

bool DynamicBitmap::isSetContentAllowed() const {
	return true;
}

bool DynamicBitmap::persist(OutputPersistenceBlock &writer) {
#if 0
	bool result = true;

	result &= Bitmap::persist(writer);

	// Image data is not saved. This is not important, since BS_DynamicBitmap is only used by the video player.
	// A video cannot be saved while it's being played. Only the state of a BS_DynamicBitmap can be persisted.
	warning("Persisting a BS_DynamicBitmap. Bitmap content is not persisted.");

	result &= RenderObject::persistChildren(writer);

	return result;
#endif

	error("Request to persist a dynamic bitmap (video) - probably a bug");

	return true;
}

bool DynamicBitmap::unpersist(InputPersistenceBlock &reader) {
#if 0
	bool result = true;

	result &= Bitmap::unpersist(reader);
	result &= createRenderedImage(_width, _height);

	warning("Unpersisting a BS_DynamicBitmap. Bitmap contents are missing.");

	// Initialize a transparent image.
	byte *transparentImageData = (byte *)calloc(_width * _height * 4, 1);
	_image->setContent(transparentImageData, _width * _height);
	free(transparentImageData);

	result &= RenderObject::unpersistChildren(reader);

	return reader.isGood() && result;
#endif

	error("Request to unpersist a dynamic bitmap (video) - probably a corrupted saved game or a bug");
}

} // End of namespace Sword25
