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

#include "common/memstream.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/image/imgloader.h"
#include "graphics/pixelformat.h"
#include "graphics/decoders/png.h"

namespace Sword25 {

bool ImgLoader::decodePNGImage(const byte *fileDataPtr, uint fileSize, byte *&uncompressedDataPtr, int &width, int &height, int &pitch) {
	Common::MemoryReadStream *fileStr = new Common::MemoryReadStream(fileDataPtr, fileSize, DisposeAfterUse::NO);

	Graphics::PNGDecoder png;
	if (!png.loadStream(*fileStr)) // the fileStr pointer, and thus pFileData will be deleted after this is done
		error("Error while reading PNG image");

	const Graphics::Surface *sourceSurface = png.getSurface();
	Graphics::Surface *pngSurface = sourceSurface->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24), png.getPalette());

	width = pngSurface->w;
	height = pngSurface->h;
	uncompressedDataPtr = new byte[pngSurface->pitch * pngSurface->h];
	memcpy(uncompressedDataPtr, (byte *)pngSurface->pixels, pngSurface->pitch * pngSurface->h);
	pngSurface->free();

	delete pngSurface;
	delete fileStr;

	// Signal success
	return true;
}

bool ImgLoader::decodeThumbnailImage(const byte *pFileData, uint fileSize, byte *&pUncompressedData, int &width, int &height, int &pitch) {
	const byte *src = pFileData + 4;	// skip header
	width = READ_LE_UINT16(src); src += 2;
	height = READ_LE_UINT16(src); src += 2;
	src++;	// version, ignored for now
	pitch = width * 4;

	uint32 totalSize = pitch * height;
	pUncompressedData = new byte[totalSize];
	uint32 *dst = (uint32 *)pUncompressedData;	// treat as uint32, for pixelformat output
	const Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
	byte r, g, b;

	for (uint32 i = 0; i < totalSize / 4; i++) {
		r = *src++;
		g = *src++;
		b = *src++;
		*dst++ = format.RGBToColor(r, g, b);
	}

	return true;
}

} // End of namespace Sword25
