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
 *
 */

/**
 * @file
 * YUV to RGB conversion used in engines:
 * - mohawk
 * - scumm (he)
 * - sword25
 */

#ifndef GRAPHICS_YUV_TO_RGB_H
#define GRAPHICS_YUV_TO_RGB_H

#include "common/scummsys.h"
#include "common/singleton.h"
#include "graphics/surface.h"

namespace Graphics {

class YUVToRGBLookup;

class YUVToRGBManager : public Common::Singleton<YUVToRGBManager> {
public:
	/** The scale of the luminance values */
	enum LuminanceScale {
		kScaleFull, /** Luminance values range from [0, 255] */
		kScaleITU   /** Luminance values range from [16, 235], the range from ITU-R BT.601 */
	};

	/**
	 * Convert a YUV444 image to an RGB surface
	 *
	 * @param dst     the destination surface
	 * @param scale   the scale of the luminance values
	 * @param ySrc    the source of the y component
	 * @param uSrc    the source of the u component
	 * @param vSrc    the source of the v component
	 * @param yWidth  the width of the y surface
	 * @param yHeight the height of the y surface
	 * @param yPitch  the pitch of the y surface
	 * @param uvPitch the pitch of the u and v surfaces
	 */
	void convert444(Graphics::Surface *dst, LuminanceScale scale, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch);

	/**
	 * Convert a YUV420 image to an RGB surface
	 *
	 * @param dst     the destination surface
	 * @param scale   the scale of the luminance values
	 * @param ySrc    the source of the y component
	 * @param uSrc    the source of the u component
	 * @param vSrc    the source of the v component
	 * @param yWidth  the width of the y surface (must be divisible by 2)
	 * @param yHeight the height of the y surface (must be divisible by 2)
	 * @param yPitch  the pitch of the y surface
	 * @param uvPitch the pitch of the u and v surfaces
	 */
	void convert420(Graphics::Surface *dst, LuminanceScale scale, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch);

	/**
	 * Convert a YUV410 image to an RGB surface
	 *
	 * Since the chroma has a very low resolution in 410, we perform bilinear scaling
	 * on the two chroma planes to produce the image. The chroma planes must have
	 * at least one extra row and one extra column that can be read from in order to
	 * produce a proper image. It is suggested that you fill these in with the previous
	 * row and column's data. This is required in order to speed up this function.
	 *
	 * @param dst     the destination surface
	 * @param scale   the scale of the luminance values
	 * @param ySrc    the source of the y component
	 * @param uSrc    the source of the u component
	 * @param vSrc    the source of the v component
	 * @param yWidth  the width of the y surface (must be divisible by 4)
	 * @param yHeight the height of the y surface (must be divisible by 4)
	 * @param yPitch  the pitch of the y surface
	 * @param uvPitch the pitch of the u and v surfaces
	 */
	void convert410(Graphics::Surface *dst, LuminanceScale scale, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch);

private:
	friend class Common::Singleton<SingletonBaseType>;
	YUVToRGBManager();
	~YUVToRGBManager();

	const YUVToRGBLookup *getLookup(Graphics::PixelFormat format, LuminanceScale scale);

	YUVToRGBLookup *_lookup;
	int16 _colorTab[4 * 256]; // 2048 bytes
};

} // End of namespace Graphics

#define YUVToRGBMan (::Graphics::YUVToRGBManager::instance())

#endif
