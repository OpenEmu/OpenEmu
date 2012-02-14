/* 
 * Metaphrasis is a static conversion class for transforming RGBA image
 * buffers into verious GX texture formats for Wii homebrew development.
 * Copyright (C) 2008 Armin Tamzarian
 * 
 * This file is part of Metaphrasis.
 * 
 * Metaphrasis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Metaphrasis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Metaphrasis.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \mainpage Metaphrasis
 *
 * \section sec_intro Introduction
 * 
 * Metaphrasis is a static conversion class for transforming RGBA image buffers into verious GX texture formats for Wii homebrew development.
 * <br>
 * Metaphrasis is written in C++ and makes use of a community standard and newly developed algorithms for conversion of 32-bit RGBA data buffers into various GX texture formats common to both the Gamecube and Wii platforms.  
 * <p>
 * This library was developed in-full by Armin Tamzarian with the support of developers in \#wiibrew on EFnet, Chaosteil of libwiisprite, and DrTwox of GRRLIB. 
 * 
 * \section sec_installation_source Installation (Source Code)
 * 
 * -# Extract the Metaphrasis archive.
 * -# Copy the contents of the <i>src</i> directory into your project's development path.
 * -# Include the Metaphrasis header file in your code using syntax such as the following:
 * \code
 * #include "Metaphrasis.h"
 * \endcode
 * 
 * \section sec_installation_library Installation (Library)
 * 
 * -# Extract the Metaphrasis archive.
 * -# Copy the contents of the <i>lib</i> directory into your <i>devKitPro/libogc</i> directory.
 * -# Include the Metaphrasis header file in your code using syntax such as the following:
 * \code
 * #include "Metaphrasis.h"
 * \endcode
 * 
 * \section sec_usage Usage
 * 
 * -# Create a buffer full of 32-bit RGBA values noting both the pixel height and width of the buffer.
 * -# Call one of the many conversion routines from within your code. (Note: All methods within the Metaphrasis class are static and thus no class instance need be allocated)
 * \code
 * uint32_t* rgba8Buffer = Metaphrasis::convertBufferToRGBA8(rgbaBuffer, bufferWidth, bufferHeight);
 * \endcode
 * -# Free your temporary RGBA value buffer if you no longer need said values.
 * 
 * Currently supported conversion routines are as follows:
 * \li convertBufferToI4
 * \li convertBufferToI8
 * \li convertBufferToIA4
 * \li convertBufferToIA8
 * \li convertBufferToRGBA8
 * \li convertBufferToRGB565
 * \li convertBufferToRGB5A3
 * 
 * \section sec_license License
 * 
 * Metaphrasis is distributed under the GNU Lesser General Public License.
 *    
 * \section sec_contact Contact
 * 
 * If you have any suggestions, questions, or comments regarding this library feel free to e-mail me at tamzarian1989 [at] gmail [dawt] com.
 */

#ifndef METAPHRASIS_H_
#define METAPHRASIS_H_

#include <gccore.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

#ifdef __cplusplus

/*! \class Metaphrasis
 * \brief A static conversion class for transforming RGBA image buffers into verious GX texture formats for
 * Wii homebrew development.
 * \author Armin Tamzarian
 * \version 0.1.0
 * 
 * Metaphrasis is a static conversion class for transforming RGBA image buffers into verious GX texture formats for
 * Wii homebrew development. Metaphrasis is written in C++ and makes use of a community standard and newly developed
 * algorithms for conversion of 32-bit RGBA data buffers into various GX texture formats common to both the Gamecube
 * and Wii platforms.
 */
class Metaphrasis {
	public:
		Metaphrasis();
		virtual ~Metaphrasis();

		static uint32_t* convertBufferToI4(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight);
		static uint32_t* convertBufferToI8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight);
		static uint32_t* convertBufferToIA4(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight);
		static uint32_t* convertBufferToIA8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight);
		static uint32_t* convertBufferToRGBA8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight);
		static uint32_t* convertBufferToRGB565(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight);
		static uint32_t* convertBufferToRGB5A3(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight);
		
		static uint8_t convertRGBAToIA4(uint32_t rgba);
		static uint16_t convertRGBAToIA8(uint32_t rgba);
		static uint16_t convertRGBAToRGB565(uint32_t rgba);
		static uint16_t convertRGBAToRGB5A3(uint32_t rgba);

};

#endif

#endif /*METAPHRASIS_H_*/
