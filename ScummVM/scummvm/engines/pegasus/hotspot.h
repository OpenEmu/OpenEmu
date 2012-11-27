/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_HOTSPOT_H
#define PEGASUS_HOTSPOT_H

#include "common/list.h"
#include "common/rect.h"

#include "pegasus/constants.h"
#include "pegasus/types.h"
#include "pegasus/util.h"

/*

	Hot spots combine a pixel area, an ID value and an active flag.

	A point is considered in a hot spot if the point is in the hot spot's pixel area and
	the active flag is set.

	In addition, hot spots have a 32 bit word of bit flags for filtering use.

*/

namespace Common {
	class ReadStream;
}

namespace Pegasus {

// Our implementation of QuickDraw regions
class Region {
public:
	Region() {}
	Region(Common::ReadStream *stream);
	Region(const Common::Rect &rect);

	Common::Rect getBoundingBox() const { return _bounds; }

	bool pointInRegion(const Common::Point &point) const;

	void moveTo(CoordType h, CoordType v);
	void moveTo(const Common::Point &point);
	void translate(CoordType h, CoordType v);
	void translate(const Common::Point &point);
	void getCenter(CoordType &h, CoordType &v) const;
	void getCenter(Common::Point &point) const;

private:
	Common::Rect _bounds;

	struct Run {
		uint16 start, end;
	};

	class Vector : public Common::List<Run> {
	public:
		uint16 y;
	};

	Common::List<Vector> _vectors;
};

class Hotspot : public IDObject {
public:
	Hotspot(const HotSpotID);
	virtual ~Hotspot();

	void setArea(const Region &region) { _spotArea = region; }
	void setArea(const Common::Rect &);
	void setArea(const CoordType, const CoordType, const CoordType, const CoordType);
	void getBoundingBox(Common::Rect &) const;
	void getArea(Region &) const;
	void getCenter(Common::Point&) const;
	void getCenter(CoordType&, CoordType&) const;

	void moveSpotTo(const CoordType, const CoordType);
	void moveSpotTo(const Common::Point);
	void moveSpot(const CoordType, const CoordType);
	void moveSpot(const Common::Point);

	bool pointInSpot(const Common::Point) const;

	void setActive();
	void setInactive();
	bool isSpotActive() const;

	HotSpotFlags getHotspotFlags() const;
	void setHotspotFlags(const HotSpotFlags);
	void setMaskedHotspotFlags(const HotSpotFlags flags, const HotSpotFlags mask);

protected:
	Region _spotArea;
	HotSpotFlags _spotFlags;
	bool _spotActive;
};

class HotspotList : public Common::List<Hotspot *> {
public:
	HotspotList();
	virtual ~HotspotList();

	void deleteHotspots();

	Hotspot *findHotspot(const Common::Point);
	HotSpotID findHotspotID(const Common::Point);
	Hotspot *findHotspotByID(const HotSpotID);
	Hotspot *findHotspotByMask(const HotSpotFlags);

	void activateMaskedHotspots(const HotSpotFlags = kNoHotSpotFlags);
	void deactivateAllHotspots();
	void deactivateMaskedHotspots(const HotSpotFlags);

	void activateOneHotspot(const HotSpotID);
	void deactivateOneHotspot(const HotSpotID);

	void removeOneHotspot(const HotSpotID);
	void removeMaskedHotspots(const HotSpotFlags = kNoHotSpotFlags);

	void setHotspotRect(const HotSpotID, const Common::Rect&);
	void getHotspotRect(const HotSpotID, Common::Rect&);
};

typedef HotspotList::iterator HotspotIterator;

#define g_allHotspots (((PegasusEngine *)g_engine)->getAllHotspots())

} // End of namespace Pegasus

#endif
