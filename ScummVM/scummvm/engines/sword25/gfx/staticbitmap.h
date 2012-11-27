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

#ifndef SWORD25_STATIC_BITMAP_H
#define SWORD25_STATIC_BITMAP_H

#include "sword25/kernel/common.h"
#include "sword25/gfx/bitmap.h"

namespace Sword25 {

class StaticBitmap : public Bitmap {
	friend class RenderObject;

private:
	/**
	    @remark Filename muss absoluter Pfad sein
	*/
	StaticBitmap(RenderObjectPtr<RenderObject> parentPtr, const Common::String &filename);
	StaticBitmap(InputPersistenceBlock &reader, RenderObjectPtr<RenderObject> parentPtr, uint handle);

public:
	virtual ~StaticBitmap();

	virtual uint getPixel(int x, int y) const;

	virtual bool setContent(const byte *pixeldata, uint size, uint offset, uint stride);

	virtual bool isScalingAllowed() const;
	virtual bool isAlphaAllowed() const;
	virtual bool isColorModulationAllowed() const;
	virtual bool isSetContentAllowed() const {
		return false;
	}

	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);

protected:
	virtual bool doRender();

private:
	Common::String _resourceFilename;

	bool initBitmapResource(const Common::String &filename);
};

} // End of namespace Sword25

#endif
