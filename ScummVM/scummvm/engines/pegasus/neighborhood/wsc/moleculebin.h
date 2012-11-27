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

#ifndef PEGASUS_NEIGHBORHOOD_WSC_MOLECULEBIN_H
#define PEGASUS_NEIGHBORHOOD_WSC_MOLECULEBIN_H

#include "pegasus/elements.h"
#include "pegasus/surface.h"
#include "pegasus/util.h"

namespace Pegasus {

enum {
	kMolecule1,
	kMolecule2,
	kMolecule3,
	kMolecule4,
	kMolecule5,
	kMolecule6
};

class MoleculeBin : public DisplayElement {
public:
	MoleculeBin();
	virtual ~MoleculeBin() {}

	void initMoleculeBin();
	void cleanUpMoleculeBin();

	void setBinLayout(const uint32 *);

	void highlightMolecule(const uint32 whichMolecule);
	void selectMolecule(const int whichMolecule);
	void resetBin();

	bool isMoleculeHighlighted(uint32);

protected:
	void draw(const Common::Rect &);

	Surface _binImages;
	FlagsArray<byte, kMolecule6 + 1> _moleculeFlags;
	int _selectedMolecule;
	uint32 _binLayout[6];
	uint32 _highlightColor;
};

} // End of namespace Pegasus

#endif
