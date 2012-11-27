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

#include "graphics/thumbnail.h"
#include "graphics/scaler.h"
#include "graphics/colormasks.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Graphics {

namespace {
#define THMB_VERSION 1

struct ThumbnailHeader {
	uint32 type;
	uint32 size;
	byte version;
	uint16 width, height;
	byte bpp;
};

#define ThumbnailHeaderSize (4+4+1+2+2+1)

bool loadHeader(Common::SeekableReadStream &in, ThumbnailHeader &header, bool outputWarnings) {
	header.type = in.readUint32BE();
	// We also accept the bad 'BMHT' header here, for the sake of compatibility
	// with some older savegames which were written incorrectly due to a bug in
	// ScummVM which wrote the thumb header type incorrectly on LE systems.
	if (header.type != MKTAG('T','H','M','B') && header.type != MKTAG('B','M','H','T')) {
		if (outputWarnings)
			warning("couldn't find thumbnail header type");
		return false;
	}

	header.size = in.readUint32BE();
	header.version = in.readByte();

	if (header.version > THMB_VERSION) {
		if (outputWarnings)
			warning("trying to load a newer thumbnail version: %d instead of %d", header.version, THMB_VERSION);
		return false;
	}

	header.width = in.readUint16BE();
	header.height = in.readUint16BE();
	header.bpp = in.readByte();

	return true;
}
} // end of anonymous namespace

bool checkThumbnailHeader(Common::SeekableReadStream &in) {
	uint32 position = in.pos();
	ThumbnailHeader header;

	bool hasHeader = loadHeader(in, header, false);

	in.seek(position, SEEK_SET);

	return hasHeader;
}

bool skipThumbnail(Common::SeekableReadStream &in) {
	uint32 position = in.pos();
	ThumbnailHeader header;

	if (!loadHeader(in, header, false)) {
		in.seek(position, SEEK_SET);
		return false;
	}

	in.seek(header.size - (in.pos() - position), SEEK_CUR);
	return true;
}

Graphics::Surface *loadThumbnail(Common::SeekableReadStream &in) {
	ThumbnailHeader header;

	if (!loadHeader(in, header, true))
		return 0;

	if (header.bpp != 2) {
		warning("trying to load thumbnail with unsupported bit depth %d", header.bpp);
		return 0;
	}

	Graphics::PixelFormat format = g_system->getOverlayFormat();
	Graphics::Surface *const to = new Graphics::Surface();
	to->create(header.width, header.height, format);

	OverlayColor *pixels = (OverlayColor *)to->pixels;
	for (int y = 0; y < to->h; ++y) {
		for (int x = 0; x < to->w; ++x) {
			uint8 r, g, b;
			colorToRGB<ColorMasks<565> >(in.readUint16BE(), r, g, b);

			// converting to current OSystem Color
			*pixels++ = format.RGBToColor(r, g, b);
		}
	}

	return to;
}

bool saveThumbnail(Common::WriteStream &out) {
	Graphics::Surface thumb;

	if (!createThumbnailFromScreen(&thumb)) {
		warning("Couldn't create thumbnail from screen, aborting thumbnail save");
		return false;
	}

	bool success = saveThumbnail(out, thumb);
	thumb.free();

	return success;
}

bool saveThumbnail(Common::WriteStream &out, const Graphics::Surface &thumb) {
	if (thumb.format.bytesPerPixel != 2) {
		warning("trying to save thumbnail with bpp different than 2");
		return false;
	}

	ThumbnailHeader header;
	header.type = MKTAG('T','H','M','B');
	header.size = ThumbnailHeaderSize + thumb.w*thumb.h*thumb.format.bytesPerPixel;
	header.version = THMB_VERSION;
	header.width = thumb.w;
	header.height = thumb.h;
	header.bpp = thumb.format.bytesPerPixel;

	out.writeUint32BE(header.type);
	out.writeUint32BE(header.size);
	out.writeByte(header.version);
	out.writeUint16BE(header.width);
	out.writeUint16BE(header.height);
	out.writeByte(header.bpp);

	// TODO: for later this shouldn't be casted to uint16...
	uint16 *pixels = (uint16 *)thumb.pixels;
	for (uint16 p = 0; p < thumb.w*thumb.h; ++p, ++pixels)
		out.writeUint16BE(*pixels);

	return true;
}

} // End of namespace Graphics
