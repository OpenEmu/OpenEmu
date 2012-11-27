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

#include "pegasus/graphics.h"
#include "pegasus/neighborhood/wsc/moleculebin.h"
#include "pegasus/neighborhood/wsc/wsc.h"

namespace Pegasus {

static const CoordType kMoleculeBinWidth = 138;
static const CoordType kMoleculeBinHeight = 128;

static const CoordType kMoleculeWidth = 66;
static const CoordType kMoleculeHeight = 40;

static const CoordType kMoleculeBinLeft = kNavAreaLeft + 286;
static const CoordType kMoleculeBinTop = kNavAreaLeft + 96;

//	Layouts:

MoleculeBin::MoleculeBin() : DisplayElement(kNoDisplayElement) {
	_highlightColor = g_system->getScreenFormat().RGBToColor(0xff, 0xff, 102);
	_selectedMolecule = -1;
}

void MoleculeBin::initMoleculeBin() {
	if (!isDisplaying()) {
		for (int i = 0; i < 6; i++)
			_binLayout[i] = i;

		resetBin();
		_binImages.getImageFromPICTFile("Images/World Science Center/Molecules");
		setDisplayOrder(kWSCMoleculeBinOrder);
		setBounds(kMoleculeBinLeft, kMoleculeBinTop, kMoleculeBinLeft + kMoleculeBinWidth,
				kMoleculeBinTop + kMoleculeBinHeight);
		startDisplaying();
		show();
	}
}

void MoleculeBin::cleanUpMoleculeBin() {
	if (isDisplaying()) {
		stopDisplaying();
		_binImages.deallocateSurface();
	}
}

void MoleculeBin::setBinLayout(const uint32 *layout) {
	for (int i = 0; i < 6; i++)
		_binLayout[i] = layout[i];
}

void MoleculeBin::highlightMolecule(const uint32 whichMolecule) {
	if (!_moleculeFlags.getFlag(whichMolecule)) {
		_moleculeFlags.setFlag(whichMolecule, true);
		triggerRedraw();
	}
}

bool MoleculeBin::isMoleculeHighlighted(uint32 whichMolecule) {
	return _moleculeFlags.getFlag(whichMolecule);
}

void MoleculeBin::selectMolecule(const int whichMolecule) {
	if (_selectedMolecule != whichMolecule) {
		_selectedMolecule = whichMolecule;
		triggerRedraw();
	}
}

void MoleculeBin::resetBin() {
	_moleculeFlags.clearAllFlags();
	_selectedMolecule = -1;
	triggerRedraw();
}

void MoleculeBin::draw(const Common::Rect &) {
	Common::Rect r1(0, 0, kMoleculeWidth, kMoleculeHeight);
	Common::Rect r2 = r1;

	for (int i = 0; i < 6; i++) {
		r1.moveTo(i * (kMoleculeWidth * 2), 0);

		if (_moleculeFlags.getFlag(_binLayout[i]))
			r1.translate(kMoleculeWidth, 0);

		r2.moveTo((_binLayout[i] & 1) * (kMoleculeWidth + 2) + _bounds.left + 2,
				(_binLayout[i] >> 1) * (kMoleculeHeight + 2) + _bounds.top + 2);

		_binImages.copyToCurrentPort(r1, r2);
	}

	if (_selectedMolecule >= 0) {
		r2.moveTo((_selectedMolecule & 1) * (kMoleculeWidth + 2) + _bounds.left + 2,
				(_selectedMolecule >> 1) * (kMoleculeHeight + 2) + _bounds.top + 2);

		Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();

		screen->frameRect(r2, _highlightColor);
		r2.grow(1);
		screen->frameRect(r2, _highlightColor);
	}
}

} // End of namespace Pegasus
