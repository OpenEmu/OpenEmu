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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_HERMITE_H
#define PEGASUS_NEIGHBORHOOD_MARS_HERMITE_H

#include "common/rect.h"
#include "pegasus/types.h"

namespace Pegasus {

CoordType hermite(CoordType p1, CoordType p4, CoordType r1, CoordType r4, int32 t, int32 duration);
CoordType dHermite(CoordType p1, CoordType p4, CoordType r1, CoordType r4, int32 t, int32 duration);
void hermite(Common::Point p1, Common::Point p4, Common::Point r1, Common::Point r4, int32 t, int32 duration, Common::Point &result);
void dHermite(Common::Point p1, Common::Point p4, Common::Point r1, Common::Point r4, int32 t, int32 duration, Common::Point &result);

} // End of namespace Pegasus

#endif
