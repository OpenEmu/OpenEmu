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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GRAPHICS_SURFACE_H
#define GRAPHICS_SURFACE_H

#include "common/scummsys.h"

namespace Common {
struct Rect;
}

#include "graphics/pixelformat.h"

namespace Graphics {

/**
 * An arbitrary graphics surface, which can be the target (or source) of blit
 * operations, font rendering, etc.
 */
struct Surface {
	/*
	 * IMPORTANT implementation specific detail:
	 *
	 * ARM code relies on the layout of the first 3 of these fields. Do not
	 * change them.
	 */

	/**
	 * The width of the surface.
	 */
	uint16 w;

	/**
	 * The height of the surface.
	 */
	uint16 h;

	/**
	 * The number of bytes a pixel line has.
	 *
	 * Note that this might not equal w * bytesPerPixel.
	 */
	uint16 pitch;

	/**
	 * The surface's pixel data.
	 */
	void *pixels;

	/**
	 * The pixel format of the surface.
	 */
	PixelFormat format;

	/**
	 * Construct a simple Surface object.
	 */
	Surface() : w(0), h(0), pitch(0), pixels(0), format() {
	}

	/**
	 * Return a pointer to the pixel at the specified point.
	 *
	 * @param x The x coordinate of the pixel.
	 * @param y The y coordinate of the pixel.
	 * @return Pointer to the pixel.
	 */
	inline const void *getBasePtr(int x, int y) const {
		return (const byte *)(pixels) + y * pitch + x * format.bytesPerPixel;
	}

	/**
	 * Return a pointer to the pixel at the specified point.
	 *
	 * @param x The x coordinate of the pixel.
	 * @param y The y coordinate of the pixel.
	 * @return Pointer to the pixel.
	 */
	inline void *getBasePtr(int x, int y) {
		return static_cast<byte *>(pixels) + y * pitch + x * format.bytesPerPixel;
	}

	/**
	 * Allocate memory for the pixel data of the surface.
	 *
	 * Note that you are responsible for calling free yourself.
	 * @see free
	 *
	 * @param width Width of the surface object.
	 * @param height Height of the surface object.
	 * @param format The pixel format the surface should use.
	 */
	void create(uint16 width, uint16 height, const PixelFormat &format);

	/**
	 * Release the memory used by the pixels memory of this surface. This is the
	 * counterpart to create().
	 *
	 * Note that you should only use this, when you created the Surface data via
	 * create! Otherwise this function has undefined behavior.
	 * @see create
	 */
	void free();

	/**
	 * Copy the data from another Surface.
	 *
	 * Note that this calls free on the current surface, to assure it being
	 * clean. So be sure the current data was created via create, otherwise
	 * the results are undefined.
	 * @see create
	 * @see free
	 *
	 * @param surf Surface to copy from.
	 */
	void copyFrom(const Surface &surf);

	/**
	 * Convert the data to another pixel format.
	 *
	 * This works in-place. This means it will not create an additional buffer
	 * for the conversion process. The value of pixels might change though.
	 *
	 * Note that you should only use this, when you created the Surface data via
	 * create! Otherwise this function has undefined behavior.
	 *
	 * @param dstFormat The desired format
	 * @param palette   The palette (in RGB888), if the source format has a Bpp of 1
	 */
	void convertToInPlace(const PixelFormat &dstFormat, const byte *palette = 0);

	/**
	 * Convert the data to another pixel format.
	 *
	 * The calling code must call free on the returned surface and then delete
	 * it.
	 *
	 * @param dstFormat The desired format
	 * @param palette   The palette (in RGB888), if the source format has a Bpp of 1
	 */
	Graphics::Surface *convertTo(const PixelFormat &dstFormat, const byte *palette = 0) const;

	/**
	 * Draw a line.
	 *
	 * @param x0 The x coordinate of the start point.
	 * @param y0 The y coordiante of the start point.
	 * @param x1 The x coordinate of the end point.
	 * @param y1 The y coordinate of the end point.
	 * @param color The color of the line.
	 * @note This is just a wrapper around Graphics::drawLine
	 */
	void drawLine(int x0, int y0, int x1, int y1, uint32 color);

	/**
	 * Draw a thick line.
	 *
	 * @param x0 The x coordinate of the start point.
	 * @param y0 The y coordiante of the start point.
	 * @param x1 The x coordinate of the end point.
	 * @param y1 The y coordinate of the end point.
	 * @param penX The width of the pen (thickness in the x direction)
	 * @param penY The height of the pen (thickness in the y direction)
	 * @param color The color of the line.
	 * @note This is just a wrapper around Graphics::drawThickLine
	 * @note The x/y coordinates of the start and end points are the upper-left most part of the pen
	 */
	void drawThickLine(int x0, int y0, int x1, int y1, int penX, int penY, uint32 color);

	/**
	 * Draw a horizontal line.
	 *
	 * @param x The start x coordinate of the line.
	 * @param y The y coordiante of the line.
	 * @param x2 The end x coordinate of the line.
	 *           In case x > x2 the coordinates are swapped.
	 * @param color The color of the line.
	 */
	void hLine(int x, int y, int x2, uint32 color);

	/**
	 * Draw a vertical line.
	 *
	 * @param x The x coordinate of the line.
	 * @param y The start y coordiante of the line.
	 * @param y2 The end y coordinate of the line.
	 *           In case y > y2 the coordinates are swapped.
	 * @param color The color of the line.
	 */
	void vLine(int x, int y, int y2, uint32 color);

	/**
	 * Fill a rect with a given color.
	 *
	 * @param r Rect to fill
	 * @param color The color of the rect's contents.
	 */
	void fillRect(Common::Rect r, uint32 color);

	/**
	 * Draw a frame around a specified rect.
	 *
	 * @param r Rect to frame
	 * @param color The color of the frame.
	 */
	void frameRect(const Common::Rect &r, uint32 color);

	// See comment in graphics/surface.cpp about it
	void move(int dx, int dy, int height);
};

/**
 * A deleter for Surface objects which can be used with SharedPtr.
 *
 * This deleter assures Surface::free is called on deletion.
 */
struct SharedPtrSurfaceDeleter {
	void operator()(Surface *ptr) {
		ptr->free();
		delete ptr;
	}
};


} // End of namespace Graphics


#endif
