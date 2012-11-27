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

#include "common/memstream.h"

#include "pegasus/cursor.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/aichip.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/items/biochips/pegasuschip.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/items/inventory/inventoryitem.h"

namespace Pegasus {

AIArea *g_AIArea = 0;

AIArea::AIArea(InputHandler *nextHandler) : InputHandler(nextHandler), _leftAreaMovie(kAILeftAreaID),
		_middleAreaMovie(kAIMiddleAreaID), _rightAreaMovie(kAIRightAreaID), _AIMovie(kAIMovieID) {
	g_AIArea = this;
	_leftAreaOwner = kNoClientSignature;
	_middleAreaOwner = kNoClientSignature;
	_rightAreaOwner = kNoClientSignature;
	_leftInventoryTime = 0xffffffff;
	_middleInventoryTime = 0xffffffff;
	_middleBiochipTime = 0xffffffff;
	_rightBiochipTime = 0xffffffff;
	_lockCount = 0;
	startIdling();
}

AIArea::~AIArea() {
	if (_middleAreaOwner == kBiochipSignature) {
		BiochipItem *currentBiochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();
		if (currentBiochip && currentBiochip->isSelected())
			currentBiochip->giveUpSharedArea();
	} else if (_middleAreaOwner == kInventorySignature) {
		InventoryItem *currentItem = ((PegasusEngine *)g_engine)->getCurrentInventoryItem();
		if (currentItem && currentItem->isSelected())
			currentItem->giveUpSharedArea();
	}

	stopIdling();

	for (AIRuleList::iterator it = _AIRules.begin(); it != _AIRules.end(); it++)
		delete *it;

	g_AIArea = 0;
}

// Save last state of AI rules...
void AIArea::saveAIState() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	delete vm->_aiSaveStream;

	Common::MemoryWriteStreamDynamic out;
	writeAIRules(&out);

	vm->_aiSaveStream = new Common::MemoryReadStream(out.getData(), out.size(), DisposeAfterUse::YES);
}

void AIArea::restoreAIState() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	if (vm->_aiSaveStream)
		readAIRules(vm->_aiSaveStream);
}

void AIArea::writeAIRules(Common::WriteStream *stream) {
	_AIRules.writeAIRules(stream);
}

void AIArea::readAIRules(Common::ReadStream *stream) {
	_AIRules.readAIRules(stream);
}

void AIArea::initAIArea() {
	allocateSurface(Common::Rect(0, 0, 384, 96));

	_leftAreaMovie.shareSurface(this);
	_leftAreaMovie.initFromMovieFile("Images/Items/Left Area Movie");
	_leftAreaMovie.moveElementTo(kAILeftAreaLeft, kAILeftAreaTop);
	_leftAreaMovie.setDisplayOrder(kAILeftAreaOrder);
	_leftAreaMovie.startDisplaying();
	_leftAreaMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());

	_middleAreaMovie.shareSurface(this);
	_middleAreaMovie.initFromMovieFile("Images/Items/Middle Area Movie");
	_middleAreaMovie.moveElementTo(kAIMiddleAreaLeft, kAIMiddleAreaTop);
	_middleAreaMovie.moveMovieBoxTo(kAIMiddleAreaLeft - kAILeftAreaLeft, 0);
	_middleAreaMovie.setDisplayOrder(kAIMiddleAreaOrder);
	_middleAreaMovie.startDisplaying();
	_middleAreaMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());

	_rightAreaMovie.shareSurface(this);
	_rightAreaMovie.initFromMovieFile("Images/Items/Right Area Movie");
	_rightAreaMovie.moveElementTo(kAIRightAreaLeft, kAIRightAreaTop);
	_rightAreaMovie.moveMovieBoxTo(kAIRightAreaLeft - kAILeftAreaLeft, 0);
	_rightAreaMovie.setDisplayOrder(kAIRightAreaOrder);
	_rightAreaMovie.startDisplaying();
	_rightAreaMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());

	_AIMovie.setDisplayOrder(kAIMovieOrder);
}

void AIArea::setAIVolume(const uint16 volume) {
	_leftAreaMovie.setVolume(volume);
	_middleAreaMovie.setVolume(volume);
	_rightAreaMovie.setVolume(volume);
}

// There are only so many legal combinations of client/area.
// Here is the list of supported pairs:
//      kInventorySignature     kLeftAreaSignature
//      kInventorySignature     kMiddleAreaSignature
//      kBiochipSignature       kMiddleAreaSignature
//      kBiochipSignature       kRightAreaSignature
//      kAISignature            kLeftAreaSignature
// Further, the kAISignature never sets a static frame time in the left area,
// but only plays a sequence.

// If this function is called while a sequence is playing, it will just "remember"
// the time value, so that when the sequence finishes, the new time is asserted.

void AIArea::setAIAreaToTime(const LowerClientSignature client, const LowerAreaSignature area, const TimeValue time) {
	switch (area) {
	case kLeftAreaSignature:
		// Only support kInventorySignature client, since AI never calls SetAIAreaToTime.
		_leftAreaMovie.setSegment(0, _leftAreaMovie.getDuration());

		if (time == 0xffffffff) {
			_leftAreaMovie.hide();
			_leftAreaOwner = kNoClientSignature;
		} else {
			setLeftMovieTime(time);
		}
		break;
	case kMiddleAreaSignature:
		// Only support kInventorySignature and kBiochipSignature clients.
		_middleAreaMovie.stop();
		_middleAreaMovie.setFlags(0);
		_middleAreaMovie.setSegment(0, _middleAreaMovie.getDuration());

		if (time == 0xffffffff) {
			if (client == kInventorySignature) {
				if (_middleBiochipTime != 0xffffffff) {
					setMiddleMovieTime(kBiochipSignature, _middleBiochipTime);
				} else {
					_middleAreaMovie.hide();
					_middleAreaOwner = kNoClientSignature;
				}
			} else {	// client == kBiochipSignature
				if (_middleInventoryTime != 0xffffffff) {
					setMiddleMovieTime(kInventorySignature, _middleInventoryTime);
				} else {
					_middleAreaMovie.hide();
					_middleAreaOwner = kNoClientSignature;
				}
			}
		} else {
			setMiddleMovieTime(client, time);
		}
		break;
	case kRightAreaSignature:
		// Only support kBiochipSignature client.
		_rightAreaMovie.setSegment(0, _rightAreaMovie.getDuration());

		if (time == 0xffffffff) {
			_rightAreaMovie.hide();
			_rightAreaOwner = kNoClientSignature;
		} else {
			setRightMovieTime(time);
		}
		break;
	}
}

// Plays a sequence on an area. When the sequence ends, the previous image
// is restored.
// Also, is input disabled or not?
// Easy answer: yes.

// There are only so many legal combinations of client/area.
// Here is the list of supported pairs:
//     kBiochipSignature        kMiddleAreaSignature
// 	   kBiochipSignature        kRightAreaSignature
// 	   kInventorySignature      kMiddleAreaSignature

void AIArea::playAIAreaSequence(const LowerClientSignature, const LowerAreaSignature area, const TimeValue start, const TimeValue stop) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	lockAIOut();

	switch (area) {
	case kLeftAreaSignature:
		break;
	case kMiddleAreaSignature:
		if (_middleAreaOwner == kInventorySignature)
			_middleInventoryTime = _middleAreaMovie.getTime();
		else if (_middleAreaOwner == kBiochipSignature)
			_middleBiochipTime = _middleAreaMovie.getTime();

		_middleAreaMovie.stop();
		_middleAreaMovie.setFlags(0);
		_middleAreaMovie.setSegment(start, stop);
		_middleAreaMovie.setTime(start);
		_middleAreaMovie.show();
		_middleAreaMovie.start();
		vm->_cursor->hide();

		while (_middleAreaMovie.isRunning()) {
			vm->checkCallBacks();
			vm->refreshDisplay();
			g_system->delayMillis(10);
		}

		_middleAreaMovie.stop();
		vm->_cursor->hideUntilMoved();

		if (_middleAreaOwner == kInventorySignature)
			setAIAreaToTime(_middleAreaOwner, kMiddleAreaSignature, _middleInventoryTime);
		else if (_middleAreaOwner == kBiochipSignature)
			setAIAreaToTime(_middleAreaOwner, kMiddleAreaSignature, _middleBiochipTime);
		else
			setAIAreaToTime(_middleAreaOwner, kMiddleAreaSignature, 0xffffffff);
		break;
	case kRightAreaSignature:
		_rightBiochipTime = _rightAreaMovie.getTime();
		_rightAreaMovie.setSegment(start, stop);
		_rightAreaMovie.setTime(start);
		_rightAreaMovie.show();
		_rightAreaMovie.start();
		vm->_cursor->hide();

		while (_rightAreaMovie.isRunning()) {
			vm->checkCallBacks();
			vm->refreshDisplay();
			g_system->delayMillis(10);
		}

		_rightAreaMovie.stop();
		vm->_cursor->hideUntilMoved();
		setAIAreaToTime(_rightAreaOwner, kRightAreaSignature, _rightBiochipTime);
		break;
	}

	unlockAI();
}

bool AIArea::playAIMovie(const LowerAreaSignature area, const Common::String &movieName, bool keepLastFrame, const InputBits interruptFilter) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	lockAIOut();

	InputDevice.waitInput(interruptFilter);
	if (_AIMovie.isMovieValid())
		_AIMovie.releaseMovie();

	_AIMovie.shareSurface(this);
	_AIMovie.initFromMovieFile(movieName);

	if (area == kLeftAreaSignature) {
		_AIMovie.moveElementTo(kAILeftAreaLeft, kAILeftAreaTop);
		_leftAreaMovie.hide();
	} else {
		_AIMovie.moveElementTo(kAIRightAreaLeft, kAIRightAreaTop);
		_AIMovie.moveMovieBoxTo(kAIRightAreaLeft - kAILeftAreaLeft, 0);
		_rightAreaMovie.hide();
	}

	_AIMovie.setTime(0);
	_AIMovie.startDisplaying();
	_AIMovie.show();
	_AIMovie.redrawMovieWorld();
	_AIMovie.setVolume(vm->getSoundFXLevel());
	_AIMovie.start();
	vm->_cursor->hide();

	bool result = true;
	bool saveAllowed = vm->swapSaveAllowed(false);
	bool openAllowed = vm->swapLoadAllowed(false);

	while (_AIMovie.isRunning()) {
		Input input;
		InputDevice.getInput(input, interruptFilter);

		if (input.anyInput() || vm->shouldQuit() || vm->saveRequested() || vm->loadRequested()) {
			result = false;
			break;
		}

		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	_AIMovie.stop();

	vm->swapSaveAllowed(saveAllowed);
	vm->swapLoadAllowed(openAllowed);

	// This used to keep the last frame up even if the movie was interrupted.
	// However, this only occurs in the recalibration, where interruption means skip the
	// whole thing, so skipping causes the AI to go away even when keepLastFrame is true.

	if (!(result && keepLastFrame)) {
		_AIMovie.stopDisplaying();
		_AIMovie.releaseMovie();

		if (area == kLeftAreaSignature) {
			_leftAreaMovie.setTime(_leftInventoryTime);
			_leftAreaMovie.show();
			_leftAreaMovie.redrawMovieWorld();
		} else {
			_rightAreaMovie.setTime(_rightBiochipTime);
			_rightAreaMovie.show();
			_rightAreaMovie.redrawMovieWorld();
		}
	}

	vm->_cursor->hideUntilMoved();
	unlockAI();
	return result;
}

// Only implemented for kMiddleAreaSignature, kInventorySignature
void AIArea::loopAIAreaSequence(const LowerClientSignature owner, const LowerAreaSignature area, const TimeValue start, const TimeValue stop) {
	if (area == kMiddleAreaSignature && owner == kInventorySignature && owner == _middleAreaOwner) {
		_middleAreaMovie.stop();
		_middleAreaMovie.setFlags(0);
		_middleAreaMovie.setSegment(start, stop);
		_middleAreaMovie.setFlags(kLoopTimeBase);
		_middleAreaMovie.setTime(start);
		_middleAreaMovie.show();
		_middleAreaMovie.start();
	}
}

// Only called by kInventorySignature.
void AIArea::setLeftMovieTime(const TimeValue time) {
	if (!_AIMovie.isSurfaceValid()) {
		_leftAreaMovie.setTime(time);
		_leftAreaMovie.show();
		_leftAreaMovie.redrawMovieWorld();
	}

	_leftAreaOwner = kInventorySignature;
	_leftInventoryTime = time;
}

void AIArea::setMiddleMovieTime(const LowerClientSignature client, const TimeValue time) {
	if (client == kInventorySignature) {
		_middleInventoryTime = time;
		if (_middleAreaOwner == kBiochipSignature) {
			BiochipItem *currentBiochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();
			if (currentBiochip && currentBiochip->isSelected())
				currentBiochip->giveUpSharedArea();
		}
	} else {
		_middleBiochipTime = time;
		if (_middleAreaOwner == kInventorySignature) {
			InventoryItem *currentItem = ((PegasusEngine *)g_engine)->getCurrentInventoryItem();
			if (currentItem && currentItem->isSelected())
				currentItem->giveUpSharedArea();
		}
	}

	_middleAreaMovie.setSegment(0, _middleAreaMovie.getDuration());
	_middleAreaMovie.stop();
	_middleAreaMovie.setFlags(0);
	_middleAreaMovie.setTime(time);
	_middleAreaMovie.show();
	_middleAreaMovie.redrawMovieWorld();
	_middleAreaOwner = client;
}

// Only called by kBiochipSignature.
void AIArea::setRightMovieTime(const TimeValue time) {
	if (!_AIMovie.isSurfaceValid()) {
		// Can't do it when the AI movie is up...
		_rightAreaMovie.setTime(time);
		_rightAreaMovie.show();
		_rightAreaMovie.redrawMovieWorld();
	}

	_rightAreaOwner = kBiochipSignature;
	_rightBiochipTime = time;
}

void AIArea::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (JMPPPInput::isToggleAIMiddleInput(input))
		toggleMiddleAreaOwner();
	else
		InputHandler::handleInput(input, cursorSpot);
}

void AIArea::toggleMiddleAreaOwner() {
	if (_middleAreaOwner == kInventorySignature) {
		BiochipItem *currentBiochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();
		if (currentBiochip) {
			setMiddleMovieTime(kBiochipSignature, currentBiochip->getSharedAreaTime());
			currentBiochip->takeSharedArea();
		}
	} else if (_middleAreaOwner == kBiochipSignature) {
		InventoryItem *currentItem = ((PegasusEngine *)g_engine)->getCurrentInventoryItem();
		if (currentItem) {
			setMiddleMovieTime(kInventorySignature, currentItem->getSharedAreaTime());
			currentItem->takeSharedArea();
		}
	}
}

void AIArea::activateHotspots() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	if (_middleAreaOwner == kBiochipSignature) {
		BiochipItem *currentBiochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();
		if (currentBiochip)
			switch (currentBiochip->getObjectID()) {
			case kAIBiochip:
				((AIChip *)currentBiochip)->activateAIHotspots();
				break;
			case kPegasusBiochip:
				if (!vm->isDemo())
					((PegasusChip *)currentBiochip)->activatePegasusHotspots();
				break;
			case kOpticalBiochip:
				((OpticalChip *)currentBiochip)->activateOpticalHotspots();
				break;
			}
	} else if (_middleAreaOwner == kInventorySignature) {
		InventoryItem *currentItem = ((PegasusEngine *)g_engine)->getCurrentInventoryItem();
		if (currentItem && currentItem->getObjectID() == kAirMask)
			((AirMask *)currentItem)->activateAirMaskHotspots();
	}

	InputHandler::activateHotspots();
}

void AIArea::clickInHotspot(const Input &input, const Hotspot *hotspot) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	bool handled = false;

	if (_middleAreaOwner == kBiochipSignature) {
		BiochipItem *currentBiochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();

		if (currentBiochip) {
			switch (currentBiochip->getObjectID()) {
			case kAIBiochip:
				if ((hotspot->getHotspotFlags() & kAIBiochipSpotFlag) != 0) {
					((AIChip *)currentBiochip)->clickInAIHotspot(hotspot->getObjectID());
					handled = true;
				}
				break;
			case kPegasusBiochip:
				if (!vm->isDemo() && ((hotspot->getHotspotFlags() & kPegasusBiochipSpotFlag) != 0)) {
					((PegasusChip *)currentBiochip)->clickInPegasusHotspot();
					handled = true;
				}
				break;
			case kOpticalBiochip:
				if ((hotspot->getHotspotFlags() & kOpticalBiochipSpotFlag) != 0) {
					((OpticalChip *)currentBiochip)->clickInOpticalHotspot(hotspot->getObjectID());
					handled = true;
				}
				break;
			}
		}
	} else if (_middleAreaOwner == kInventorySignature) {
		InventoryItem *currentItem = ((PegasusEngine *)g_engine)->getCurrentInventoryItem();

		if (currentItem) {
			switch (currentItem->getObjectID()) {
			case kAirMask:
				if ((hotspot->getHotspotFlags() & kAirMaskSpotFlag) != 0) {
					((AirMask *)currentItem)->clickInAirMaskHotspot();
					handled = true;
				}
				break;
			}
		}
	}

	if (!handled)
		InputHandler::clickInHotspot(input, hotspot);
}

void AIArea::lockAIOut() {
	if (_lockCount == 0)
		stopIdling();

	_lockCount++;
}

void AIArea::unlockAI() {
	if (_lockCount > 0) {
		_lockCount--;
		if (_lockCount == 0)
			startIdling();
	}
}

void AIArea::forceAIUnlocked() {
	if (_lockCount > 0) {
		_lockCount = 1;
		unlockAI();
	}
}

void AIArea::checkRules() {
	if (_lockCount == 0 && ((PegasusEngine *)g_engine)->playerAlive())
		for (AIRuleList::iterator it = _AIRules.begin(); it != _AIRules.end(); it++)
			if ((*it)->fireRule())
				break;
}

void AIArea::useIdleTime() {
	checkRules();
}

void AIArea::addAIRule(AIRule *rule) {
	_AIRules.push_back(rule);
}

void AIArea::removeAllRules() {
	for (AIRuleList::iterator it = _AIRules.begin(); it != _AIRules.end(); it++)
		delete *it;

	_AIRules.clear();
}

void AIArea::checkMiddleArea() {
	BiochipItem *currentBiochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();

	if (currentBiochip) {
		if (_middleAreaOwner == kBiochipSignature) {
			switch (currentBiochip->getObjectID()) {
			case kAIBiochip:
				((AIChip *)currentBiochip)->setUpAIChip();
				break;
			case kPegasusBiochip:
				((PegasusChip *)currentBiochip)->setUpPegasusChip();
				break;
			}
		} else {
			switch (currentBiochip->getObjectID()) {
			case kAIBiochip:
				((AIChip *)currentBiochip)->setUpAIChipRude();
				break;
			case kPegasusBiochip:
				((PegasusChip *)currentBiochip)->setUpPegasusChipRude();
				break;
			}
		}
	}
}

TimeValue AIArea::getBigInfoTime() {
	if (_middleAreaOwner == kInventorySignature) {
		InventoryItem *currentItem = ((PegasusEngine *)g_engine)->getCurrentInventoryItem();
		return currentItem->getInfoLeftTime();
	} else if (_middleAreaOwner == kBiochipSignature) {
		BiochipItem *currentBiochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();
		return currentBiochip->getInfoLeftTime();
	}

	return 0xffffffff;
}

void AIArea::getSmallInfoSegment(TimeValue &start, TimeValue &stop) {
	if (_middleAreaOwner == kInventorySignature) {
		InventoryItem *currentItem = ((PegasusEngine *)g_engine)->getCurrentInventoryItem();
		currentItem->getInfoRightTimes(start, stop);
	} else if (_middleAreaOwner == kBiochipSignature) {
		BiochipItem *currentBiochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();
		currentBiochip->getInfoRightTimes(start, stop);
	} else {
		start = 0xffffffff;
		stop = 0xffffffff;
	}
}

LowerClientSignature AIArea::getMiddleAreaOwner() {
	return _middleAreaOwner;
}

} // End of namespace Pegasus
