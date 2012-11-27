/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_SURFACE_H
#define PEGASUS_SURFACE_H

#include "common/rect.h"
#include "common/str.h"

#include "pegasus/elements.h"
#include "pegasus/types.h"

namespace Common {
	class MacResManager;
}

namespace Graphics {
	struct Surface;
}

namespace Video {
	class VideoDecoder;
}

namespace Pegasus {

// Surface bounds are always normalized.

class Surface {
public:
	Surface();
	virtual ~Surface();

	virtual void allocateSurface(const Common::Rect &);
	virtual void deallocateSurface();
	virtual void shareSurface(Surface *surface);
	bool isSurfaceValid() const { return _surface != 0; }

	Graphics::Surface *getSurface() const { return _surface; }
	void getSurfaceBounds(Common::Rect &r) { r = _bounds; }

	// None of the copyToCurrentPort* functions do any sanity checks.
	// It's up to clients to make sure that the Surface is valid.
	void copyToCurrentPort() const;
	void copyToCurrentPortTransparent() const;
	void copyToCurrentPort(const Common::Rect &) const;
	void copyToCurrentPortTransparent(const Common::Rect &) const;
	void copyToCurrentPort(const Common::Rect &, const Common::Rect &) const;
	void copyToCurrentPortTransparent(const Common::Rect &, const Common::Rect &) const;
	void copyToCurrentPortMasked(const Common::Rect &, const Common::Rect &, const Surface *) const;
	void copyToCurrentPortTransparentGlow(const Common::Rect &, const Common::Rect &) const;
	void scaleTransparentCopy(const Common::Rect &, const Common::Rect &) const;
	void scaleTransparentCopyGlow(const Common::Rect &, const Common::Rect &) const;

	virtual void getImageFromPICTFile(const Common::String &fileName);
	virtual void getImageFromPICTResource(Common::MacResManager *resFork, uint16 id);
	virtual void getImageFromMovieFrame(Video::VideoDecoder *, TimeValue);

protected:
	bool _ownsSurface;
	Graphics::Surface *_surface;
	Common::Rect _bounds;

private:
	bool getImageFromPICTStream(Common::SeekableReadStream *stream);

	uint32 getGlowColor(uint32 color) const;
	bool isTransparent(uint32 color) const;
};

class PixelImage : public Surface {
public:
	PixelImage();
	virtual ~PixelImage() {}

	void drawImage(const Common::Rect &, const Common::Rect &);

protected:
	virtual void setTransparent(bool transparent) { _transparent = transparent; }

	bool _transparent;
};

class Frame : public PixelImage {
public:
	Frame() {}
	virtual ~Frame() {}

	virtual void initFromPICTFile(const Common::String &fileName, bool transparent = false);
	virtual void initFromPICTResource(Common::MacResManager *resFork, uint16 id, bool transparent = false);
	virtual void initFromMovieFrame(Video::VideoDecoder *, TimeValue, bool transparent = false);
};

class SpriteFrame : public Frame {
friend class Sprite;
public:
	SpriteFrame() { _referenceCount = 0; }
	virtual ~SpriteFrame() {}

protected:
	uint32 _referenceCount;
};

class Picture : public DisplayElement, public Frame {
public:
	Picture(const DisplayElementID id) : DisplayElement(id) {}
	virtual ~Picture() {}

	virtual void initFromPICTFile(const Common::String &fileName, bool transparent = false);
	virtual void initFromPICTResource(Common::MacResManager *resFork, uint16 id, bool transparent = false);
	virtual void initFromMovieFrame(Video::VideoDecoder *, TimeValue, bool transparent = false);

	virtual void draw(const Common::Rect &);
};

} // End of namespace Pegasus

#endif
