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

#include "sword25/gfx/bitmap.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

namespace Sword25 {

Bitmap::Bitmap(RenderObjectPtr<RenderObject> parentPtr, TYPES type, uint handle) :
	RenderObject(parentPtr, type, handle),
	_modulationColor(0xffffffff),
	_scaleFactorX(1.0f),
	_scaleFactorY(1.0f),
	_flipH(false),
	_flipV(false) {
}

Bitmap::~Bitmap() {
}

void Bitmap::setAlpha(int alpha) {
	if (!isAlphaAllowed()) {
		warning("Tried to set alpha value on a bitmap that does not support alpha blending. Call was ignored.");
		return;
	}

	if (alpha < 0 || alpha > 255) {
		int oldAlpha = alpha;
		if (alpha < 0)
			alpha = 0;
		if (alpha > 255)
			alpha = 255;
		warning("Tried to set an invalid alpha value (%d) on a bitmap. Value was changed to %d.", oldAlpha, alpha);

		return;
	}

	uint newModulationColor = (_modulationColor & 0x00ffffff) | alpha << 24;
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

void Bitmap::setModulationColor(uint modulationColor) {
	if (!isColorModulationAllowed()) {
		warning("Tried to set modulation color of a bitmap that does not support color modulation. Call was ignored.");
		return;
	}

	uint newModulationColor = (modulationColor & 0x00ffffff) | (_modulationColor & 0xff000000);
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

void Bitmap::setScaleFactor(float scaleFactor) {
	setScaleFactorX(scaleFactor);
	setScaleFactorY(scaleFactor);
}

void Bitmap::setScaleFactorX(float scaleFactorX) {
	if (!isScalingAllowed()) {
		warning("Tried to set scale factor of a bitmap that does not support scaling. Call was ignored.");
		return;
	}

	if (scaleFactorX < 0) {
		warning("Tried to set scale factor of a bitmap to a negative value. Call was ignored.");
		return;
	}

	if (scaleFactorX != _scaleFactorX) {
		_scaleFactorX = scaleFactorX;
		_width = static_cast<int>(_originalWidth * _scaleFactorX);
		if (_scaleFactorX <= 0.0f)
			_scaleFactorX = 0.001f;
		if (_width <= 0)
			_width = 1;
		forceRefresh();
	}
}

void Bitmap::setScaleFactorY(float scaleFactorY) {
	if (!isScalingAllowed()) {
		warning("Tried to set scale factor of a bitmap that does not support scaling. Call was ignored.");
		return;
	}

	if (scaleFactorY < 0) {
		warning("Tried to set scale factor of a bitmap to a negative value. Call was ignored.");
		return;
	}

	if (scaleFactorY != _scaleFactorY) {
		_scaleFactorY = scaleFactorY;
		_height = static_cast<int>(_originalHeight * scaleFactorY);
		if (_scaleFactorY <= 0.0f)
			_scaleFactorY = 0.001f;
		if (_height <= 0)
			_height = 1;
		forceRefresh();
	}
}

void Bitmap::setFlipH(bool flipH) {
	_flipH = flipH;
	forceRefresh();
}

void Bitmap::setFlipV(bool flipV) {
	_flipV = flipV;
	forceRefresh();
}

bool Bitmap::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	result &= RenderObject::persist(writer);
	writer.write(_flipH);
	writer.write(_flipV);
	writer.write(_scaleFactorX);
	writer.write(_scaleFactorY);
	writer.write(_modulationColor);
	writer.write(_originalWidth);
	writer.write(_originalHeight);

	return result;
}

bool Bitmap::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	result &= RenderObject::unpersist(reader);
	reader.read(_flipH);
	reader.read(_flipV);
	reader.read(_scaleFactorX);
	reader.read(_scaleFactorY);
	reader.read(_modulationColor);
	reader.read(_originalWidth);
	reader.read(_originalHeight);

	forceRefresh();

	return reader.isGood() && result;
}

} // End of namespace Sword25
