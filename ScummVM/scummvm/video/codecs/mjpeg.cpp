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

#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "graphics/decoders/jpeg.h"

#include "video/codecs/mjpeg.h"

namespace Common {
class SeekableReadStream;
}

namespace Video {

JPEGDecoder::JPEGDecoder() : Codec() {
	_pixelFormat = g_system->getScreenFormat();
	_surface = NULL;
}

JPEGDecoder::~JPEGDecoder() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

const Graphics::Surface *JPEGDecoder::decodeImage(Common::SeekableReadStream *stream) {
	Graphics::JPEGDecoder jpeg;

	if (!jpeg.loadStream(*stream)) {
		warning("Failed to decode JPEG frame");
		return 0;
	}

	if (_surface) {
		_surface->free();
		delete _surface;
	}

	_surface = jpeg.getSurface()->convertTo(_pixelFormat);

	return _surface;
}

} // End of namespace Video
