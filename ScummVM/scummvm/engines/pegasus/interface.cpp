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

#include "pegasus/compass.h"
#include "pegasus/energymonitor.h"
#include "pegasus/interface.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/inventory/inventoryitem.h"

namespace Pegasus {

Interface *g_interface = 0;

Interface::Interface() : InputHandler(0), _interfaceNotification(kInterfaceNotificationID, (NotificationManager *)((PegasusEngine *)g_engine)),
			_currentItemSpot(kCurrentItemSpotID), _currentBiochipSpot(kCurrentBiochipSpotID),
			_background1(kInterface1ID), _background2(kInterface2ID), _background3(kInterface3ID),
			_background4(kInterface4ID), _datePicture(kDateID), _inventoryPush(kInventoryPushID),
			_inventoryLid(kInventoryLidID, kNoDisplayElement),
			_inventoryPanel(kNoDisplayElement, (InputHandler *)((PegasusEngine *)g_engine), ((PegasusEngine *)g_engine)->getItemsInventory()),
			_biochipPush(kBiochipPushID), _biochipLid(kBiochipLidID, kNoDisplayElement),
			_biochipPanel(kNoDisplayElement, (InputHandler *)((PegasusEngine *)g_engine), ((PegasusEngine *)g_engine)->getBiochipsInventory()) {
	g_energyMonitor = 0;
	_previousHandler = 0;
	_inventoryRaised = false;
	_biochipRaised = false;
	_playingEndMessage = false;
	g_interface = this;
}

Interface::~Interface() {
	throwAwayInterface();
	g_interface = 0;
}

void Interface::throwAwayInterface() {
	g_allHotspots.removeOneHotspot(kCurrentItemSpotID);
	g_allHotspots.removeOneHotspot(kCurrentBiochipSpotID);

	throwAwayBackground();
	throwAwayDateMonitor();
	throwAwayEnergyMonitor();
	throwAwayAIArea();
	throwAwayCompass();
	throwAwayNotifications();
	throwAwayInventoryPanel();
	throwAwayBiochipPanel();
}

void Interface::validateBackground() {
	if (!_background1.isSurfaceValid()) {
		_background1.initFromPICTFile("Images/Interface/3DInterface Left");
		_background2.initFromPICTFile("Images/Interface/3DInterface Top");
		_background3.initFromPICTFile("Images/Interface/3DInterface Right");
		_background4.initFromPICTFile("Images/Interface/3DInterface Bottom");

		_background1.setDisplayOrder(kBackground1Order);
		_background1.startDisplaying();
		_background1.moveElementTo(kBackground1Left, kBackground1Top);

		_background2.setDisplayOrder(kBackground2Order);
		_background2.startDisplaying();
		_background2.moveElementTo(kBackground2Left, kBackground2Top);

		_background3.setDisplayOrder(kBackground2Order);
		_background3.startDisplaying();
		_background3.moveElementTo(kBackground3Left, kBackground3Top);

		_background4.setDisplayOrder(kBackground4Order);
		_background4.startDisplaying();
		_background4.moveElementTo(kBackground4Left, kBackground4Top);

		_background1.show();
		_background2.show();
		_background3.show();
		_background4.show();
	}
}

void Interface::throwAwayBackground() {
	_background1.stopDisplaying();
	_background1.deallocateSurface();
	_background2.stopDisplaying();
	_background2.deallocateSurface();
	_background3.stopDisplaying();
	_background3.deallocateSurface();
	_background4.stopDisplaying();
	_background4.deallocateSurface();
}

void Interface::validateDateMonitor() {
	if (!_datePicture.isSurfaceValid()) {
		_datePicture.setDisplayOrder(kDateOrder);
		_datePicture.startDisplaying();
		_datePicture.moveElementTo(kDateLeft, kDateTop);
		_datePicture.show();
	}
}

void Interface::throwAwayDateMonitor() {
	_datePicture.stopDisplaying();
	_datePicture.deallocateSurface();
}

void Interface::setDate(const uint16 dateResID) {
	validateDateMonitor();
	_datePicture.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, dateResID);
	_datePicture.triggerRedraw();
}

void Interface::validateCompass() {
	if (!g_compass) {
		new Compass();
		g_compass->initCompass();
		g_compass->setDisplayOrder(kCompassOrder);
		g_compass->startDisplaying();
		g_compass->moveElementTo(kCompassLeft, kCompassTop);
		g_compass->show();
	}
}

void Interface::throwAwayCompass() {
	delete g_compass;
}

void Interface::validateNotifications() {
	_interfaceNotification.notifyMe(this, kInterfaceNotificationFlags, kInterfaceNotificationFlags);
	_inventoryLidCallBack.setNotification(&_interfaceNotification);
	_inventoryPushCallBack.setNotification(&_interfaceNotification);
	_biochipLidCallBack.setNotification(&_interfaceNotification);
	_biochipPushCallBack.setNotification(&_interfaceNotification);
}

void Interface::throwAwayNotifications() {
	_interfaceNotification.cancelNotification(this);
}

void Interface::validateAIArea() {
	if (!g_AIArea) {
		new AIArea((InputHandler *)((PegasusEngine *)g_engine));
		if (g_AIArea)
			g_AIArea->initAIArea();
	}
}

void Interface::throwAwayAIArea() {
	delete g_AIArea;
}

void Interface::validateInventoryPanel() {
	if (!_inventoryPanel.isSurfaceValid()) {
		_inventoryPanel.initInventoryImage(&_inventoryPush);
		_inventoryPanel.moveElementTo(kInventoryPushLeft, kInventoryPushTop);
		_inventoryPush.setSlideDirection(kSlideUpMask);
		_inventoryPush.setInAndOutElements(&_inventoryPanel, 0);
		_inventoryPush.setDisplayOrder(kInventoryPushOrder);
		_inventoryPush.startDisplaying();

		_inventoryLid.useFileName("Images/Lids/Inventory Lid Sequence");
		_inventoryLid.useTransparent(true);
		_inventoryLid.openFrameSequence();
		_inventoryLid.moveElementTo(kInventoryLidLeft, kInventoryLidTop);
		_inventoryLid.setDisplayOrder(kInventoryLidOrder);
		_inventoryLid.startDisplaying();

		_inventoryPushCallBack.initCallBack(&_inventoryPush, kCallBackAtExtremes);
		_inventoryLidCallBack.initCallBack(&_inventoryLid, kCallBackAtExtremes);

		_inventoryUp = false;
		_inventoryRaised = false;

		Item *item = getCurrentInventoryItem();
		if (item)
			item->select();
	}
}

void Interface::throwAwayInventoryPanel() {
	_inventoryPanel.stopDisplaying();
	_inventoryPanel.throwAwayInventoryImage();
	_inventoryPush.stopDisplaying();
	_inventoryLid.stopDisplaying();
	_inventoryLid.closeFrameSequence();
	_inventoryPushCallBack.releaseCallBack();
	_inventoryLidCallBack.releaseCallBack();

	Item *item = getCurrentInventoryItem();
	if (item)
		item->deselect();

	_inventoryUp = false;
	_inventoryRaised = false;
}

void Interface::validateBiochipPanel() {
	if (!_biochipPanel.isSurfaceValid()) {
		_biochipPanel.initInventoryImage(&_biochipPush);
		_biochipPanel.moveElementTo(kBiochipPushLeft, kBiochipPushTop);
		_biochipPush.setSlideDirection(kSlideUpMask);
		_biochipPush.setInAndOutElements(&_biochipPanel, 0);
		_biochipPush.setDisplayOrder(kBiochipPushOrder);
		_biochipPush.startDisplaying();

		_biochipLid.useFileName("Images/Lids/Biochip Lid Sequence");
		_biochipLid.useTransparent(true);
		_biochipLid.openFrameSequence();
		_biochipLid.moveElementTo(kBiochipLidLeft, kBiochipLidTop);
		_biochipLid.setDisplayOrder(kBiochipLidOrder);
		_biochipLid.startDisplaying();

		_biochipPushCallBack.initCallBack(&_biochipPush, kCallBackAtExtremes);
		_biochipLidCallBack.initCallBack(&_biochipLid, kCallBackAtExtremes);

		_biochipUp = false;
		_biochipRaised = false;

		Item *item = getCurrentBiochip();
		if (item)
			item->select();
	}
}

void Interface::throwAwayBiochipPanel() {
	_biochipPanel.stopDisplaying();
	_biochipPanel.throwAwayInventoryImage();
	_biochipPush.stopDisplaying();
	_biochipLid.stopDisplaying();
	_biochipLid.closeFrameSequence();
	_biochipPushCallBack.releaseCallBack();
	_biochipLidCallBack.releaseCallBack();

	Item *item = getCurrentBiochip();
	if (item)
		item->deselect();

	_biochipUp = false;
	_biochipRaised = false;
}

void Interface::validateEnergyMonitor() {
	if (!g_energyMonitor)
		new EnergyMonitor();
}

void Interface::throwAwayEnergyMonitor() {
	delete g_energyMonitor;
}

void Interface::createInterface() {
	validateBackground();
	validateDateMonitor();
	validateCompass();
	validateNotifications();
	validateAIArea();
	validateBiochipPanel();
	validateInventoryPanel();
	validateEnergyMonitor();

	if (!g_allHotspots.findHotspotByID(kCurrentItemSpotID)) {
		_currentItemSpot.setArea(Common::Rect(76, 334, 172, 430));
		_currentItemSpot.setHotspotFlags(kShellSpotFlag);
		_currentItemSpot.setActive();
		g_allHotspots.push_back(&_currentItemSpot);
	}

	if (!g_allHotspots.findHotspotByID(kCurrentBiochipSpotID)) {
		_currentBiochipSpot.setArea(Common::Rect(364, 334, 460, 430));
		_currentBiochipSpot.setHotspotFlags(kShellSpotFlag);
		_currentBiochipSpot.setActive();
		g_allHotspots.push_back(&_currentBiochipSpot);
	}
}

InventoryResult Interface::addInventoryItem(InventoryItem *item) {
	return _inventoryPanel.addInventoryItem(item);
}

InventoryResult Interface::removeInventoryItem(InventoryItem *item) {
	return _inventoryPanel.removeInventoryItem(item);
}

void Interface::removeAllItemsFromInventory() {
	_inventoryPanel.removeAllItems();
}

InventoryItem *Interface::getCurrentInventoryItem() {
	return (InventoryItem *)_inventoryPanel.getCurrentItem();
}

void Interface::setCurrentInventoryItem(InventoryItem *item) {
	setCurrentInventoryItemID(item->getObjectID());
}

void Interface::setCurrentInventoryItemID(ItemID id) {
	_inventoryPanel.setCurrentItemID(id);
}

InventoryResult Interface::addBiochip(BiochipItem *item) {
	return _biochipPanel.addInventoryItem(item);
}

void Interface::removeAllItemsFromBiochips() {
	_biochipPanel.removeAllItems();
}

BiochipItem *Interface::getCurrentBiochip() {
	return (BiochipItem *)_biochipPanel.getCurrentItem();
}

void Interface::setCurrentBiochip(BiochipItem *item) {
	setCurrentBiochipID(item->getObjectID());
}

void Interface::setCurrentBiochipID(ItemID id) {
	_biochipPanel.setCurrentItemID(id);
}

void Interface::receiveNotification(Notification *notification, const NotificationFlags flags) {
	if (notification == &_interfaceNotification) {
		switch (flags) {
		case kInventoryLidOpenFlag:
			inventoryLidOpen(true);
			break;
		case kInventoryLidClosedFlag:
			inventoryLidClosed();
			break;
		case kInventoryDrawerUpFlag:
			inventoryDrawerUp();
			break;
		case kInventoryDrawerDownFlag:
			inventoryDrawerDown(true);
			break;
		case kBiochipLidOpenFlag:
			biochipLidOpen(true);
			break;
		case kBiochipLidClosedFlag:
			biochipLidClosed();
			break;
		case kBiochipDrawerUpFlag:
			biochipDrawerUp();
			break;
		case kBiochipDrawerDownFlag:
			biochipDrawerDown(true);
			break;
		}
	}
}

void Interface::raiseInventoryDrawer(const bool doCallBacks) {
	if (!_biochipUp)
		_previousHandler = InputHandler::getCurrentHandler();

	InputHandler::setInputHandler(&_inventoryPanel);
	_inventoryUp = true;
	_inventoryPanel.activateInventoryPicture();

	if (doCallBacks) {
		_inventoryLidCallBack.setCallBackFlag(kInventoryLidOpenFlag);
		_inventoryLidCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	}

	_inventoryLid.show();
	_inventoryPush.show();
	_inventoryLid.start();
}

void Interface::playEndMessage() {
	raiseInventoryDrawerForMessage();
	_playingEndMessage = true;
	_inventoryPanel.playEndMessage(&_inventoryPush);
	lowerInventoryDrawerForMessage();
	_playingEndMessage = false;
}

void Interface::raiseInventoryDrawerForMessage() {
	_inventoryPanel.disableLooping();
	raiseInventoryDrawerSync();
}

void Interface::lowerInventoryDrawerForMessage() {
	lowerInventoryDrawerSync();
}

void Interface::inventoryLidOpen(const bool doCallBacks) {
	_inventoryLid.stop();

	if (doCallBacks) {
		_inventoryPushCallBack.setCallBackFlag(kInventoryDrawerUpFlag);
		_inventoryPushCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	}

	FaderMoveSpec moveSpec;
	moveSpec.makeTwoKnotFaderSpec(60, 0, 0, 15, 1000);
	_inventoryPush.startFader(moveSpec);
}

void Interface::inventoryDrawerUp() {
	_inventoryPush.stopFader();
	_inventoryPanel.panelUp();
	_inventoryRaised = true;
}

bool Interface::isInventoryUp() {
	return _inventoryRaised;
}

bool Interface::isInventoryDown() {
	return !_inventoryUp;
}

void Interface::lowerInventoryDrawer(const bool doCallBacks) {
	if (_inventoryRaised) {
		_inventoryRaised = false;

		if (!_playingEndMessage)
			_inventoryPanel.deactivateInventoryPicture();

		if (doCallBacks) {
			_inventoryPushCallBack.setCallBackFlag(kInventoryDrawerDownFlag);
			_inventoryPushCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
		}

		FaderMoveSpec moveSpec;
		moveSpec.makeTwoKnotFaderSpec(60, 0, 1000, 15, 0);
		_inventoryPush.startFader(moveSpec);
	}
}

void Interface::inventoryDrawerDown(const bool doCallBacks) {
	_inventoryPush.stopFader();

	if (doCallBacks) {
		_inventoryLidCallBack.setCallBackFlag(kInventoryLidClosedFlag);
		_inventoryLidCallBack.scheduleCallBack(kTriggerAtStart, 0, 0);
	}

	_inventoryLid.setRate(-1);
}

void Interface::inventoryLidClosed() {
	_inventoryLid.stop();

	if (!_biochipUp)
		InputHandler::setInputHandler(_previousHandler);

	_inventoryLid.hide();
	_inventoryPush.hide();
	_inventoryUp = false;
}

void Interface::raiseBiochipDrawer(const bool doCallBacks) {
	if (!_inventoryUp)
		_previousHandler = InputHandler::getCurrentHandler();

	InputHandler::setInputHandler(&_biochipPanel);
	_biochipUp = true;
	_biochipPanel.activateInventoryPicture();

	if (doCallBacks) {
		_biochipLidCallBack.setCallBackFlag(kBiochipLidOpenFlag);
		_biochipLidCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	}

	_biochipLid.show();
	_biochipPush.show();
	_biochipLid.start();
}

void Interface::biochipLidOpen(const bool doCallBacks) {
	_biochipLid.stop();

	if (doCallBacks) {
		_biochipPushCallBack.setCallBackFlag(kBiochipDrawerUpFlag);
		_biochipPushCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	}

	FaderMoveSpec moveSpec;
	moveSpec.makeTwoKnotFaderSpec(60, 0, 0, 9, 1000);
	_biochipPush.startFader(moveSpec);
}

void Interface::biochipDrawerUp() {
	_biochipPush.stopFader();
	_biochipPanel.panelUp();
	_biochipRaised = true;
}

void Interface::lowerBiochipDrawer(const bool doCallBacks) {
	if (_biochipRaised) {
		_biochipRaised = false;
		_biochipPanel.deactivateInventoryPicture();

		if (doCallBacks) {
			_biochipPushCallBack.setCallBackFlag(kBiochipDrawerDownFlag);
			_biochipPushCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
		}

		FaderMoveSpec moveSpec;
		moveSpec.makeTwoKnotFaderSpec(60, 0, 1000, 9, 0);
		_biochipPush.startFader(moveSpec);
	}
}

void Interface::biochipDrawerDown(const bool doCallBacks) {
	_biochipPush.stopFader();

	if (doCallBacks) {
		_biochipLidCallBack.setCallBackFlag(kBiochipLidClosedFlag);
		_biochipLidCallBack.scheduleCallBack(kTriggerAtStart, 0, 0);
	}

	_biochipLid.setRate(-1);
}

void Interface::biochipLidClosed() {
	_biochipLid.stop();

	if (!_inventoryUp)
		InputHandler::setInputHandler(_previousHandler);

	_biochipLid.hide();
	_biochipPush.hide();
	_biochipUp = false;
}

void Interface::calibrateCompass() {
	uint32 currentValue = g_compass->getFaderValue();
	FaderMoveSpec compassMove;
	compassMove.makeTwoKnotFaderSpec(15, 0, currentValue, 30, currentValue + 360);

	g_compass->startFader(compassMove);

	PegasusEngine *vm = (PegasusEngine *)g_engine;

	while (g_compass->isFading()) {
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	g_compass->setFaderValue(currentValue);
}

void Interface::calibrateEnergyBar() {
	g_energyMonitor->calibrateEnergyBar();
}

void Interface::raiseInventoryDrawerSync() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	raiseInventoryDrawer(false);

	while (_inventoryLid.isRunning()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	inventoryLidOpen(false);

	while (_inventoryPush.isFading()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	inventoryDrawerUp();
}

void Interface::lowerInventoryDrawerSync() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	lowerInventoryDrawer(false);

	while (_inventoryPush.isFading()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	inventoryDrawerDown(false);

	while (_inventoryLid.isRunning()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	inventoryLidClosed();
}

void Interface::raiseBiochipDrawerSync() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	raiseBiochipDrawer(false);

	while (_biochipLid.isRunning()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	biochipLidOpen(false);

	while (_biochipPush.isFading()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	biochipDrawerUp();
}

void Interface::lowerBiochipDrawerSync() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	lowerBiochipDrawer(false);

	while (_biochipPush.isFading()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	biochipDrawerDown(false);

	while (_biochipLid.isRunning()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	vm->refreshDisplay();
	biochipLidClosed();
}

} // End of namespace Pegasus
