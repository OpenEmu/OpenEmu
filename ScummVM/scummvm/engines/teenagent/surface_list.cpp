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

#include "teenagent/surface_list.h"
#include "teenagent/surface.h"
#include "teenagent/objects.h"
#include "teenagent/teenagent.h"

namespace TeenAgent {

SurfaceList::SurfaceList() : surfaces(NULL), surfacesCount(0) {}

SurfaceList::~SurfaceList() {
	free();
}

void SurfaceList::load(Common::SeekableReadStream &stream, int subHack) {
	free();

	byte fn = stream.readByte();
	if (stream.eos())
		return;

	surfacesCount = fn - subHack;
	debugC(0, kDebugSurface, "loading %u surfaces from list (skip %d)", surfacesCount, subHack);

	if (surfacesCount == 0)
		return;

	surfaces = new Surface[surfacesCount];

	for (byte i = 0; i < surfacesCount; ++i) {
		uint offset = stream.readUint16LE();
		uint pos = stream.pos();
		stream.seek(offset);
		surfaces[i].load(stream, Surface::kTypeOns);
		stream.seek(pos);
	}
}

void SurfaceList::free() {
	delete[] surfaces;
	surfaces = NULL;
	surfacesCount = 0;
}

void SurfaceList::render(Graphics::Surface *surface, const Common::Rect &clip) const {
	for (uint i = 0; i < surfacesCount; ++i) {
		const Surface &s = surfaces[i];
		Common::Rect r(s.x, s.y, s.x + s.w, s.y + s.h);
		if (r.bottom < clip.bottom || !clip.intersects(r))
			continue;

		r.clip(clip);
		r.translate(-s.x, -s.y);
		s.render(surface, r.left, r.top, false, r);
	}
}

}
