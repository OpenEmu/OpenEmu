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

#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/mars/shuttleenergymeter.h"

namespace Pegasus {

ShuttleEnergyMeter::ShuttleEnergyMeter() : FaderAnimation(kNoDisplayElement) {
	setBounds(kShuttleEnergyLeft, kShuttleEnergyTop, kShuttleEnergyLeft + kShuttleEnergyWidth,
			kShuttleEnergyTop + kShuttleEnergyHeight);
	setDisplayOrder(kShuttleStatusOrder);
	setFaderValue(0);
}

void ShuttleEnergyMeter::initShuttleEnergyMeter() {
	_meterImage.getImageFromPICTFile("Images/Mars/Shuttle Energy.pict");
	_lowWarning.getImageFromPICTFile("Images/Mars/Shuttle Low Energy.pict");
	startDisplaying();
	show();
}

void ShuttleEnergyMeter::disposeShuttleEnergyMeter() {
	stopFader();
	hide();
	stopDisplaying();
	_meterImage.deallocateSurface();
	_lowWarning.deallocateSurface();
}

void ShuttleEnergyMeter::draw(const Common::Rect &) {
	int32 currentValue = getFaderValue();

	Common::Rect r1, r2, bounds;
	getBounds(bounds);

	if (currentValue < kLowShuttleEnergy) {
		_lowWarning.getSurfaceBounds(r1);
		r2 = r1;
		r2.moveTo(bounds.left, bounds.top);
		_lowWarning.copyToCurrentPort(r1, r2);
	}

	_meterImage.getSurfaceBounds(r1);
	r1.right = r1.left + r1.width() * currentValue / kFullShuttleEnergy;
	r2 = r1;
	r2.moveTo(bounds.left + 102, bounds.top + 6);
	_meterImage.copyToCurrentPort(r1, r2);
}

void ShuttleEnergyMeter::powerUpMeter() {
	FaderMoveSpec moveSpec;
	moveSpec.makeTwoKnotFaderSpec(kThirtyTicksPerSecond, 0, 0, 45, kFullShuttleEnergy);
	startFader(moveSpec);
}

void ShuttleEnergyMeter::setEnergyValue(const int32 value) {
	stopFader();
	FaderMoveSpec moveSpec;
	moveSpec.makeTwoKnotFaderSpec(kFifteenTicksPerSecond, value * 3, value, kFullShuttleEnergy * 3, kFullShuttleEnergy);
	startFader(moveSpec);
}

void ShuttleEnergyMeter::drainForTractorBeam() {
	stopFader();
	TimeValue startTime = 0, stopTime;
	int32 startValue = getFaderValue(), stopValue;

	if (startValue < kTractorBeamEnergy) {
		stopTime = startValue * kTractorBeamTime / kTractorBeamEnergy;
		stopValue = 0;
	} else {
		stopTime = kTractorBeamTime;
		stopValue = startValue - kTractorBeamEnergy;
	}

	FaderMoveSpec moveSpec;
	moveSpec.makeTwoKnotFaderSpec(kTractorBeamScale, startTime, startValue, stopTime, stopValue);
	startFader(moveSpec);
}

int32 ShuttleEnergyMeter::getEnergyValue() const {
	return getFaderValue();
}

void ShuttleEnergyMeter::dropEnergyValue(const int32 delta) {
	setEnergyValue(getFaderValue() - delta);
}

bool ShuttleEnergyMeter::enoughEnergyForTractorBeam() const {
	return getEnergyValue() >= kTractorBeamEnergy;
}

} // End of namespace Pegasus
