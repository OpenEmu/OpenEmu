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

#include "Metaphrasis.h"

/**
 * Default constructor for the Metaphrasis class.
 */

Metaphrasis::Metaphrasis() {
}

/**
 * Default destructor for the Metaphrasis class.
 */

Metaphrasis::~Metaphrasis() {
}

/**
 * Convert the specified RGBA data buffer into the I4 texture format
 * 
 * This routine converts the RGBA data buffer into the I4 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToI4(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = bufferWidth * bufferHeight >> 1;
	uint32_t* dataBufferI4 = (uint32_t *)memalign(32, bufferSize);
	memset(dataBufferI4, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint8_t *dst = (uint8_t *)dataBufferI4;

	for(uint16_t y = 0; y < bufferHeight; y += 8) {
		for(uint16_t x = 0; x < bufferWidth; x += 8) {
			for(uint16_t rows = 0; rows < 8; rows++) {
				*dst++ = (src[((y + rows) * bufferWidth) + (x + 0)] & 0xf0) | ((src[((y + rows) * bufferWidth) + (x + 1)] & 0xf0) >> 4);
				*dst++ = (src[((y + rows) * bufferWidth) + (x + 2)] & 0xf0) | ((src[((y + rows) * bufferWidth) + (x + 3)] & 0xf0) >> 4);
				*dst++ = (src[((y + rows) * bufferWidth) + (x + 4)] & 0xf0) | ((src[((y + rows) * bufferWidth) + (x + 5)] & 0xf0) >> 4);
				*dst++ = (src[((y + rows) * bufferWidth) + (x + 5)] & 0xf0) | ((src[((y + rows) * bufferWidth) + (x + 7)] & 0xf0) >> 4);
			}
		}
	}
	DCFlushRange(dataBufferI4, bufferSize);

	return dataBufferI4;
}

/**
 * Convert the specified RGBA data buffer into the I8 texture format
 * 
 * This routine converts the RGBA data buffer into the I8 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToI8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = bufferWidth * bufferHeight;
	uint32_t* dataBufferI8 = (uint32_t *)memalign(32, bufferSize);
	memset(dataBufferI8, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint8_t *dst = (uint8_t *)dataBufferI8;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 8) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = src[((y + rows) * bufferWidth) + (x + 0)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 1)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 2)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 3)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 4)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 5)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 6)] & 0xff;
				*dst++ = src[((y + rows) * bufferWidth) + (x + 7)] & 0xff;
			}
		}
	}
	DCFlushRange(dataBufferI8, bufferSize);

	return dataBufferI8;
}

/**
 * Downsample the specified RGBA value data buffer to an IA4 value.
 * 
 * This routine downsamples the given RGBA data value into the IA4 texture data format.
 * 
 * @param rgba	A 32-bit RGBA value to convert to the IA4 format.
 * @return The IA4 value of the given RGBA value.
 */

uint8_t Metaphrasis::convertRGBAToIA4(uint32_t rgba) {
	uint8_t i, a;
	
	i = (rgba >> 8) & 0xf0;
	a = (rgba     ) & 0xff;

	return i | (a >> 4);
}

/**
 * Convert the specified RGBA data buffer into the IA4 texture format
 * 
 * This routine converts the RGBA data buffer into the IA4 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToIA4(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = bufferWidth * bufferHeight;
	uint32_t* dataBufferIA4 = (uint32_t *)memalign(32, bufferSize);
	memset(dataBufferIA4, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint8_t *dst = (uint8_t *)dataBufferIA4;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 8) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = Metaphrasis::convertRGBAToIA4(src[((y + rows) * bufferWidth) + (x + 0)]);
				*dst++ = Metaphrasis::convertRGBAToIA4(src[((y + rows) * bufferWidth) + (x + 1)]);
				*dst++ = Metaphrasis::convertRGBAToIA4(src[((y + rows) * bufferWidth) + (x + 2)]);
				*dst++ = Metaphrasis::convertRGBAToIA4(src[((y + rows) * bufferWidth) + (x + 3)]);
				*dst++ = Metaphrasis::convertRGBAToIA4(src[((y + rows) * bufferWidth) + (x + 4)]);
				*dst++ = Metaphrasis::convertRGBAToIA4(src[((y + rows) * bufferWidth) + (x + 5)]);
				*dst++ = Metaphrasis::convertRGBAToIA4(src[((y + rows) * bufferWidth) + (x + 6)]);
				*dst++ = Metaphrasis::convertRGBAToIA4(src[((y + rows) * bufferWidth) + (x + 7)]);
			}
		}
	}
	DCFlushRange(dataBufferIA4, bufferSize);

	return dataBufferIA4;
}

/**
 * Downsample the specified RGBA value data buffer to an IA8 value.
 * 
 * This routine downsamples the given RGBA data value into the IA8 texture data format.
 * 
 * @param rgba	A 32-bit RGBA value to convert to the IA8 format.
 * @return The IA8 value of the given RGBA value.
 */

uint16_t Metaphrasis::convertRGBAToIA8(uint32_t rgba) {
	uint8_t i, a;
	
	i = (rgba >> 8) & 0xff;
	a = (rgba     ) & 0xff;

	return (i << 8) | a;
}

/**
 * Convert the specified RGBA data buffer into the IA8 texture format
 * 
 * This routine converts the RGBA data buffer into the IA8 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToIA8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = (bufferWidth * bufferHeight) << 1;
	uint32_t* dataBufferIA8 = (uint32_t *)memalign(32, bufferSize);
	memset(dataBufferIA8, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint16_t *dst = (uint16_t *)dataBufferIA8;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 4) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = Metaphrasis::convertRGBAToIA8(src[((y + rows) * bufferWidth) + (x + 0)]);
				*dst++ = Metaphrasis::convertRGBAToIA8(src[((y + rows) * bufferWidth) + (x + 1)]);
				*dst++ = Metaphrasis::convertRGBAToIA8(src[((y + rows) * bufferWidth) + (x + 2)]);
				*dst++ = Metaphrasis::convertRGBAToIA8(src[((y + rows) * bufferWidth) + (x + 3)]);
			}
		}
	}
	DCFlushRange(dataBufferIA8, bufferSize);

	return dataBufferIA8;
}

/**
 * Convert the specified RGBA data buffer into the RGBA8 texture format
 * 
 * This routine converts the RGBA data buffer into the RGBA8 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToRGBA8(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = (bufferWidth * bufferHeight) << 2;
	uint32_t* dataBufferRGBA8 = (uint32_t *)memalign(32, bufferSize);
	memset(dataBufferRGBA8, 0x00, bufferSize);

	uint8_t *src = (uint8_t *)rgbaBuffer;
	uint8_t *dst = (uint8_t *)dataBufferRGBA8;

	for(uint16_t block = 0; block < bufferHeight; block += 4) {
		for(uint16_t i = 0; i < bufferWidth; i += 4) {
			for (uint16_t c = 0; c < 4; c++) {
				for (uint16_t ar = 0; ar < 4; ar++) {
					*dst++ = src[(((i + ar) + ((block + c) * bufferWidth)) * 4) + 3];
					*dst++ = src[((i + ar) + ((block + c) * bufferWidth)) * 4];
				}
			}
			for (uint16_t c = 0; c < 4; c++) {
				for (uint16_t gb = 0; gb < 4; gb++) {
					*dst++ = src[(((i + gb) + ((block + c) * bufferWidth)) * 4) + 1];
					*dst++ = src[(((i + gb) + ((block + c) * bufferWidth)) * 4) + 2];
				}
			}
		}
	}
	DCFlushRange(dataBufferRGBA8, bufferSize);

	return dataBufferRGBA8;
}

/**
 * Downsample the specified RGBA value data buffer to an RGB565 value.
 * 
 * This routine downsamples the given RGBA data value into the RGB565 texture data format.
 * Attribution for this routine is given fully to NoNameNo of GRRLIB Wii library.
 * 
 * @param rgba	A 32-bit RGBA value to convert to the RGB565 format.
 * @return The RGB565 value of the given RGBA value.
 */

uint16_t Metaphrasis::convertRGBAToRGB565(uint32_t rgba) {
	uint8_t r, g, b;
	
	r = (((rgba >> 24) & 0xff) * 31) / 255;
	g = (((rgba >> 16) & 0xff) * 63) / 255;
	b = (((rgba >>  8) & 0xff) * 31) / 255;

	return (((r << 6) | g ) << 5 ) | b;
}

/**
 * Convert the specified RGBA data buffer into the RGB565 texture format
 * 
 * This routine converts the RGBA data buffer into the RGB565 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToRGB565(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = (bufferWidth * bufferHeight) << 1;
	uint32_t* dataBufferRGB565 = (uint32_t *)memalign(32, bufferSize);
	memset(dataBufferRGB565, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint16_t *dst = (uint16_t *)dataBufferRGB565;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 4) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = Metaphrasis::convertRGBAToRGB565(src[((y + rows) * bufferWidth) + (x + 0)]);
				*dst++ = Metaphrasis::convertRGBAToRGB565(src[((y + rows) * bufferWidth) + (x + 1)]);
				*dst++ = Metaphrasis::convertRGBAToRGB565(src[((y + rows) * bufferWidth) + (x + 2)]);
				*dst++ = Metaphrasis::convertRGBAToRGB565(src[((y + rows) * bufferWidth) + (x + 3)]);
			}
		}
	}
	DCFlushRange(dataBufferRGB565, bufferSize);

	return dataBufferRGB565;
}

/**
 * Downsample the specified RGBA value data buffer to an RGB5A3 value.
 * 
 * This routine downsamples the given RGBA data value into the RGB5A3 texture data format.
 * Attribution for this routine is given fully to WiiGator via the TehSkeen forum.
 * 
 * @param rgba	A 32-bit RGBA value to convert to the RGB5A3 format.
 * @return The RGB5A3 value of the given RGBA value.
 */

uint16_t Metaphrasis::convertRGBAToRGB5A3(uint32_t rgba) {
	uint32_t r, g, b, a;
	uint16_t color;

	r = (rgba >> 24) & 0xff;
	g = (rgba >> 16) & 0xff;
	b = (rgba >>  8) & 0xff;
	a = (rgba      ) & 0xff;

	if (a > 0xe0) {
		r = r >> 3;
		g = g >> 3;
		b = b >> 3;
	
		color = (r << 10) | (g << 5) | b;
		color |= 0x8000;
	}
	else {
		r = r >> 4;
		g = g >> 4;
		b = b >> 4;
		a = a >> 5;
	
		color = (a << 12) | (r << 8) | (g << 4) | b;
	}

	return color;
}
	
/**
 * Convert the specified RGBA data buffer into the RGB5A3 texture format
 * 
 * This routine converts the RGBA data buffer into the RGB5A3 texture format and returns a pointer to the converted buffer.
 * 
 * @param rgbaBuffer	Buffer containing the temporarily rendered RGBA data.
 * @param bufferWidth	Pixel width of the data buffer.
 * @param bufferHeight	Pixel height of the data buffer.
 * @return	A pointer to the allocated buffer.
 */

uint32_t* Metaphrasis::convertBufferToRGB5A3(uint32_t* rgbaBuffer, uint16_t bufferWidth, uint16_t bufferHeight) {
	uint32_t bufferSize = (bufferWidth * bufferHeight) << 1;
	uint32_t* dataBufferRGB5A3 = (uint32_t *)memalign(32, bufferSize);
	memset(dataBufferRGB5A3, 0x00, bufferSize);

	uint32_t *src = (uint32_t *)rgbaBuffer;
	uint16_t *dst = (uint16_t *)dataBufferRGB5A3;

	for(uint16_t y = 0; y < bufferHeight; y += 4) {
		for(uint16_t x = 0; x < bufferWidth; x += 4) {
			for(uint16_t rows = 0; rows < 4; rows++) {
				*dst++ = Metaphrasis::convertRGBAToRGB5A3(src[((y + rows) * bufferWidth) + (x + 0)]);
				*dst++ = Metaphrasis::convertRGBAToRGB5A3(src[((y + rows) * bufferWidth) + (x + 1)]);
				*dst++ = Metaphrasis::convertRGBAToRGB5A3(src[((y + rows) * bufferWidth) + (x + 2)]);
				*dst++ = Metaphrasis::convertRGBAToRGB5A3(src[((y + rows) * bufferWidth) + (x + 3)]);
			}
		}
	}
	DCFlushRange(dataBufferRGB5A3, bufferSize);

	return dataBufferRGB5A3;
}
