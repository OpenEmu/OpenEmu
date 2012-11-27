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

#ifndef GRAPHICS_TRANSPARENTSURFACE_H
#define GRAPHICS_TRANSPARENTSURFACE_H

#include "graphics/surface.h"

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// TODO: Find a better solution for this.
#define BS_RGB(R,G,B)       (0xFF000000 | ((R) << 16) | ((G) << 8) | (B))
#define BS_ARGB(A,R,G,B)    (((A) << 24) | ((R) << 16) | ((G) << 8) | (B))

namespace Wintermute {

/**
 * A transparent graphics surface, which implements alpha blitting.
 */
struct TransparentSurface : public Graphics::Surface {
	TransparentSurface();
	TransparentSurface(const Graphics::Surface &surf, bool copyData = false);

	void setColorKey(char r, char g, char b);
	void disableColorKey();

	// Enums
	/**
	 @brief The possible flipping parameters for the blit methode.
	 */
	enum FLIP_FLAGS {
	    /// The image will not be flipped.
	    FLIP_NONE = 0,
	    /// The image will be flipped at the horizontal axis.
	    FLIP_H = 1,
	    /// The image will be flipped at the vertical axis.
	    FLIP_V = 2,
	    /// The image will be flipped at the horizontal and vertical axis.
	    FLIP_HV = FLIP_H | FLIP_V,
	    /// The image will be flipped at the horizontal and vertical axis.
	    FLIP_VH = FLIP_H | FLIP_V
	};

	bool _enableAlphaBlit;

	/**
	 @brief renders the surface to another surface
	 @param pDest a pointer to the target image. In most cases this is the framebuffer.
	 @param PosX the position on the X-axis in the target image in pixels where the image is supposed to be rendered.<br>
	 The default value is 0.
	 @param PosY the position on the Y-axis in the target image in pixels where the image is supposed to be rendered.<br>
	 The default value is 0.
	 @param Flipping how the the image should be flipped.<br>
	 The default value is BS_Image::FLIP_NONE (no flipping)
	 @param pSrcPartRect Pointer on Common::Rect which specifies the section to be rendered. If the whole image has to be rendered the Pointer is NULL.<br>
	 This referes to the unflipped and unscaled image.<br>
	 The default value is NULL.
	 @param Color an ARGB color value, which determines the parameters for the color modulation und alpha blending.<br>
	 The alpha component of the color determines the alpha blending parameter (0 = no covering, 255 = full covering).<br>
	 The color components determines the color for color modulation.<br>
	 The default value is BS_ARGB(255, 255, 255, 255) (full covering, no color modulation).
	 The macros BS_RGB and BS_ARGB can be used for the creation of the color value.
	 @param Width the output width of the screen section.
	 The images will be scaled if the output width of the screen section differs from the image section.<br>
	 The value -1 determines that the image should not be scaled.<br>
	 The default value is -1.
	 @param Width the output height of the screen section.
	 The images will be scaled if the output width of the screen section differs from the image section.<br>
	 The value -1 determines that the image should not be scaled.<br>
	 The default value is -1.
	 @return returns false if the rendering failed.
	 */

	Common::Rect blit(Graphics::Surface &target, int posX = 0, int posY = 0,
	                  int flipping = FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  uint color = BS_ARGB(255, 255, 255, 255),
	                  int width = -1, int height = -1);
	void applyColorKey(uint8 r, uint8 g, uint8 b, bool overwriteAlpha = false);
	// The following scale-code supports arbitrary scaling (i.e. no repeats of column 0 at the end of lines)
	TransparentSurface *scale(uint16 newWidth, uint16 newHeight) const;
	TransparentSurface *scale(const Common::Rect &srcRect, const Common::Rect &dstRect) const;
	static byte *_lookup;
	static void destroyLookup();
private:
	static void doBlitAlpha(byte *ino, byte* outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep);
	static void generateLookup();
};

/**
 * A deleter for Surface objects which can be used with SharedPtr.
 *
 * This deleter assures Surface::free is called on deletion.
 */
/*struct SharedPtrTransparentSurfaceDeleter {
    void operator()(TransparentSurface *ptr) {
        ptr->free();
        delete ptr;
    }
};*/


} // End of namespace Graphics


#endif
