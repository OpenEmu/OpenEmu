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

/*
    BS_Image
    --------

    Autor: Malte Thiesen
*/

#ifndef SWORD25_IMAGE_H
#define SWORD25_IMAGE_H

// Includes
#include "sword25/kernel/common.h"
#include "common/rect.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

class Image {
public:
	virtual ~Image() {}

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

	//@{
	/** @name Accessor methods */

	/**
	    @brief Returns the width of the image in pixels
	*/
	virtual int getWidth() const = 0;

	/**
	    @brief Returns the height of the image in pixels
	*/
	virtual int getHeight() const = 0;

	/**
	    @brief Returns the color format of the image
	*/
	virtual GraphicEngine::COLOR_FORMATS getColorFormat() const = 0;

	//@}

	//@{
	/** @name Render methodes */

	/**
	    @brief renders the image in the framebuffer
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
	    @remark Not all blitting operations of all BS_Image classes are supported.<br>
	            More information can be find in the class description of BS_Image and the following methodes:
	            - IsBlitTarget()
	            - IsScalingAllowed()
	            - IsFillingAllowed()
	            - IsAlphaAllowed()
	            - IsColorModulationAllowed()
	            - IsSetContentAllowed()
	*/
	virtual bool blit(int posX = 0, int posY = 0,
	                  int flipping = FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  uint color = BS_ARGB(255, 255, 255, 255),
	                  int width = -1, int height = -1) = 0;

	/**
	    @brief fills a rectangular section of the image with a color.
	    @param pFillRect Pointer on Common::Rect which specifies the section of the image which is supposed to be filled. If the whole image has to be filled this value is NULL.<br>
	                      The default value is NULL.
	    @param Color the 32 Bit color value for filling the image section.
	    @remark It is possible to create transparent rectangulars by using a color with alpha value not equal to 255.
	    @remark Independent from the color format of the image, it must be given a 32 bit color value. The macros BS_RGB and BS_ARGB can be used for the creation of the color value.
	    @remark If the rectangular is not completely inside the screen area, it will be automatically trimmed.
	*/
	virtual bool fill(const Common::Rect *pFillRect = 0, uint color = BS_RGB(0, 0, 0)) = 0;

	/**
	    @brief Fills the content of the image with pixel data.
	    @param Pixeldata a vector which cotains the pixel data. They must be present in the color format of the image and there must be enough data available for filling the whole image.
	    @param Offset the offset in Byte in Pixeldata-Vector on which the first pixel to write is located.<br>
	           The default value is 0.
	    @param Stride the distance in Byte between the end of line and the beginning of a new line in Pixeldata-Vector.<br>
	           The default value is 0.
	    @return returns false, if the call failed.
	    @remark A call of this methode is only allowd if IsSetContentAllowed() returns true.
	*/
	virtual bool setContent(const byte *pixeldata, uint size, uint offset, uint stride) = 0;

	/**
	    @brief Reads out a pixel of the image.
	    @param X the X-coordinate of the pixel.
	    @param Y the y-coordinate of the pixel.
	    @return Returns the 32-bit color value of the pixel at the given position.
	    @remark This methode should not be used in no way to read out bigger parts of the image because the method is very slow. The method is rather intended for reading out single pixels of the image..
	*/
	virtual uint getPixel(int x, int y) = 0;

	//@{
	/** @name Information methodes */

	/**
	    @brief Checks, if it is allowed to call BS_Image Blit().
	    @return Returns false, if a Blit() call is not allowed at this object.
	*/
	virtual bool isBlitSource() const = 0;

	/**
	    @brief Checks, if the BS_Image can be a target image for a Blit call.
	    @return Returns false, if a Blit() call with this object as an target is not allowed.
	*/
	virtual bool isBlitTarget() const = 0;

	/**
	    @brief Returns true, if the BS_Image is allowed to be scaled by a Blit() call.
	*/
	virtual bool isScalingAllowed() const = 0;

	/**
	    @brief Returns true, if the BS_Image is allowed to be filled by a Fill() call.
	*/
	virtual bool isFillingAllowed() const = 0;

	/**
	    @brief Returns true, if the BS_Image is allowed to be displayed with an alpha value.
	*/
	virtual bool isAlphaAllowed() const = 0;

	/**
	    @brief Return true, if the BS_Image is allowed to be displayed with color modulation by a Blit() call
	*/
	virtual bool isColorModulationAllowed() const = 0;

	/**
	    @brief Returns true, if the content of the BS_Image is allowed to be replaced by call of SetContent().
	*/
	virtual bool isSetContentAllowed() const = 0;

	//@}
};

} // End of namespace Sword25

#endif
