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

#ifndef PEGASUS_ENERGYMONITOR_H
#define PEGASUS_ENERGYMONITOR_H

#include "pegasus/elements.h"

namespace Pegasus {

class Sprite;

class Blinker : private IdlerTimeBase {
public:
	Blinker();
	virtual ~Blinker() {}

	void startBlinking(Sprite *sprite, int32 frame1, int32 frame2, uint32 numBlinks, TimeValue blinkDuration, TimeScale blinkScale);
	void stopBlinking();

protected:
	virtual void timeChanged(const TimeValue);

	Sprite *_sprite;
	int32 _frame1;
	int32 _frame2;
	TimeValue _blinkDuration;
};

// Energy monitor constants.

// These are in seconds.
// Max is two hours
static const uint32 kMaxJMPEnergy = 7200;

static const uint32 kCasualEnergy = kMaxJMPEnergy * 100 / 100;      // 100%
static const uint32 kWorriedEnergy = kMaxJMPEnergy * 50 / 100;      // 50%
static const uint32 kNervousEnergy = kMaxJMPEnergy * 25 / 100;      // 25%
static const uint32 kPanicStrickenEnergy = kMaxJMPEnergy * 5 / 100; // 5%

static const uint32 kFullEnergy = kCasualEnergy;

static const uint32 kFrameLightOff = 0;
static const uint32 kFrameLightYellow = 1;
static const uint32 kFrameLightOrange = 2;
static const uint32 kFrameLightRed = 3;

static const int kEnergyDrainNormal = 1;
static const int kMarsReactorEnergyDrainNoShield = 6;
static const int kMarsReactorEnergyDrainWithShield = 3;
static const int kWSCPoisonEnergyDrainWithDart = 20;
static const int kWSCPoisonEnergyDrainNoDart = 10;

class EnergyMonitor : private IdlerAnimation {
public:
	EnergyMonitor();
	virtual ~EnergyMonitor();

	void setEnergyValue(const uint32);
	void startEnergyDraining();
	void setEnergyDrainRate(Common::Rational);
	Common::Rational getEnergyDrainRate();
	void stopEnergyDraining();
	void drainEnergy(const int32);
	int32 getCurrentEnergy();

	void restoreLastEnergyValue();
	void saveCurrentEnergyValue();

	void calibrateEnergyBar();

protected:
	void timeChanged(const TimeValue);
	void calcLevelRect(Common::Rect &);
	void draw(const Common::Rect &);

	uint32 _barColor;
	Common::Rect _levelRect;
	EnergyStage _stage;
	Sprite _energyLight;
	Blinker _lightBlinker;
	bool _calibrating, _dontFlash;
};

extern EnergyMonitor *g_energyMonitor;

} // End of namespace Pegasus

#endif
