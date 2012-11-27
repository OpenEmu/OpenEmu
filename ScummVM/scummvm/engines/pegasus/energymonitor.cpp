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

#include "pegasus/energymonitor.h"
#include "pegasus/pegasus.h"
#include "pegasus/surface.h"

namespace Pegasus {

Blinker::Blinker() {
	_sprite = 0;
	_frame1 = -1;
	_frame2 = -1;
	_blinkDuration = 0;
}

void Blinker::startBlinking(Sprite *sprite, int32 frame1, int32 frame2, uint32 numBlinks, TimeValue blinkDuration, TimeScale blinkScale) {
	stopBlinking();
	_sprite = sprite;
	_frame1 = frame1;
	_frame2 = frame2;
	_blinkDuration = blinkDuration;
	setScale(blinkScale);
	setSegment(0, blinkDuration * numBlinks * 2, blinkScale);
	setTime(0);
	start();
}

void Blinker::stopBlinking() {
	if (_sprite) {
		_sprite->setCurrentFrameIndex(_frame2);
		_sprite = 0;
		stop();
	}
}

void Blinker::timeChanged(const TimeValue time) {
	if (_sprite && _blinkDuration != 0) {
		if (((time / _blinkDuration) & 1) != 0 || time == getDuration()) {
			_sprite->setCurrentFrameIndex(_frame2);
			if (!isRunning())
				stopBlinking();
		} else {
			_sprite->setCurrentFrameIndex(_frame1);
		}
	}
}

static const NotificationFlags kEnergyExpiredFlag = 1;

EnergyMonitor *g_energyMonitor = 0;

EnergyMonitor::EnergyMonitor() : IdlerAnimation(kEnergyBarID), _energyLight(kWarningLightID) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	_stage = kStageNoStage;

	_calibrating = false;
	_dontFlash = false;

	setBounds(338, 48, 434, 54);

	setDisplayOrder(kEnergyBarOrder);
	startDisplaying();

	SpriteFrame *frame = new SpriteFrame();
	frame->initFromPICTResource(vm->_resFork, kLightOffID);
	_energyLight.addFrame(frame, 0, 0);

	frame = new SpriteFrame();
	frame->initFromPICTResource(vm->_resFork, kLightYellowID);
	_energyLight.addFrame(frame, 0, 0);

	frame = new SpriteFrame();
	frame->initFromPICTResource(vm->_resFork, kLightOrangeID);
	_energyLight.addFrame(frame, 0, 0);

	frame = new SpriteFrame();
	frame->initFromPICTResource(vm->_resFork, kLightRedID);
	_energyLight.addFrame(frame, 0, 0);

	_energyLight.setBounds(540, 35, 600, 59);
	_energyLight.setDisplayOrder(kEnergyLightOrder);
	_energyLight.startDisplaying();

	setScale(1);
	setSegment(0, kMaxJMPEnergy);

	setEnergyValue(kCasualEnergy);

	g_energyMonitor = this;
}

EnergyMonitor::~EnergyMonitor() {
	g_energyMonitor = 0;
}

void EnergyMonitor::setEnergyValue(const uint32 value) {
	if (isRunning()) {
		stop();
		setTime(getStop() - value);
		start();
	} else {
		setTime(getStop() - value);
	}
}

void EnergyMonitor::startEnergyDraining() {
	if (!isRunning()) {
		_energyLight.show();
		start();
		show();
	}
}

void EnergyMonitor::setEnergyDrainRate(Common::Rational rate) {
	setRate(rate);
}

Common::Rational EnergyMonitor::getEnergyDrainRate() {
	return getRate();
}

void EnergyMonitor::stopEnergyDraining() {
	if (isRunning()) {
		stop();
		_energyLight.hide();
		hide();
	}
}

void EnergyMonitor::drainEnergy(const int32 delta) {
	setTime(getTime() + delta);
}

int32 EnergyMonitor::getCurrentEnergy() {
	return kMaxJMPEnergy - getTime();
}

void EnergyMonitor::timeChanged(const TimeValue currentTime) {
	if (currentTime == getStop()) {
		PegasusEngine *vm = (PegasusEngine *)g_engine;
		if (vm->getEnergyDeathReason() != -1)
			vm->die(vm->getEnergyDeathReason());
	} else {
		uint32 currentEnergy = kMaxJMPEnergy - currentTime;

		EnergyStage newStage;
		if (currentEnergy > kWorriedEnergy)
			newStage = kStageCasual;
		else if (currentEnergy > kNervousEnergy)
			newStage = kStageWorried;
		else if (currentEnergy > kPanicStrickenEnergy)
			newStage = kStageNervous;
		else
			newStage = kStagePanicStricken;

		if (_stage != newStage) {
			uint32 newFrame;

			switch (newStage) {
			case kStageCasual:
				_barColor = g_system->getScreenFormat().RGBToColor(0x48, 0xB0, 0xD8);
				newFrame = kFrameLightOff;
				break;
			case kStageWorried:
				_barColor = g_system->getScreenFormat().RGBToColor(0xD8, 0xC0, 0x30);
				newFrame = kFrameLightYellow;
				break;
			case kStageNervous:
				_barColor = g_system->getScreenFormat().RGBToColor(0xD8, 0x78, 0x38);
				newFrame = kFrameLightOrange;
				break;
			case kStagePanicStricken:
				_barColor = g_system->getScreenFormat().RGBToColor(0xD8, 0x40, 0x38);
				newFrame = kFrameLightRed;
				break;
			default:
				error("no stage in energy monitor?");
				break;
			}

			_stage = newStage;
			uint32 oldFrame = _energyLight.getCurrentFrameIndex();

			if (!_calibrating) {
				if (oldFrame > newFrame || oldFrame == 0xffffffff || _dontFlash) {
					_energyLight.setCurrentFrameIndex(newFrame);
					_dontFlash = false;
				} else {
					_lightBlinker.startBlinking(&_energyLight, oldFrame, newFrame, 4, 1, 3);
					triggerRedraw();
				}
			}
		}

		Common::Rect r;
		calcLevelRect(r);
		if (r != _levelRect) {
			_levelRect = r;
			triggerRedraw();
		}
	}
}

void EnergyMonitor::calcLevelRect(Common::Rect &r) {
	if (getStop() == 0) {
		r = Common::Rect();
	} else {
		getBounds(r);
		r.left = r.right - r.width() * (kMaxJMPEnergy - getTime()) / getStop();
	}
}

void EnergyMonitor::draw(const Common::Rect &r) {
	Common::Rect r2 = r.findIntersectingRect(_levelRect);

	if (!r2.isEmpty()) {
		Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();
		screen->fillRect(r2, _barColor);
	}
}

void EnergyMonitor::calibrateEnergyBar() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	_calibrating = true;

	vm->setEnergyDeathReason(-1);

	uint32 numFrames = _energyLight.getNumFrames();
	for (uint32 i = 1; i < numFrames; i++) {
		_energyLight.setCurrentFrameIndex(i);
		_energyLight.show();
		vm->delayShell(1, 3);
		_energyLight.hide();
		vm->delayShell(1, 3);
	}

	_energyLight.setCurrentFrameIndex(0);
	_energyLight.hide();

	show();
	setEnergyValue(0);
	setEnergyDrainRate(-(int32)kMaxJMPEnergy / 2);

	// Make sure warning light is hidden...
	_energyLight.hide();
	while (getCurrentEnergy() != (int32)kMaxJMPEnergy) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	setEnergyDrainRate(0);
	hide();

	_calibrating = false;
}

void EnergyMonitor::restoreLastEnergyValue() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	_dontFlash = true;
	setEnergyValue(vm->getSavedEnergyValue());
	vm->resetEnergyDeathReason();
}

void EnergyMonitor::saveCurrentEnergyValue() {
	((PegasusEngine *)g_engine)->setLastEnergyValue(getCurrentEnergy());
}

} // End of namespace Pegasus
