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

#ifndef PEGASUS_INTERFACE_H
#define PEGASUS_INTERFACE_H

#include "pegasus/hotspot.h"
#include "pegasus/input.h"
#include "pegasus/notification.h"
#include "pegasus/surface.h"
#include "pegasus/transition.h"
#include "pegasus/items/inventorypicture.h"

namespace Pegasus {

class BiochipItem;
class InventoryItem;

class Interface : public InputHandler, public NotificationReceiver {
public:
	Interface();
	virtual ~Interface();

	void createInterface();

	// Recalibration functions...
	void calibrateCompass();
	void calibrateEnergyBar();
	void raiseInventoryDrawerSync();
	void lowerInventoryDrawerSync();
	void raiseBiochipDrawerSync();
	void lowerBiochipDrawerSync();

	void raiseInventoryDrawer(const bool doCallBacks = true);
	void raiseBiochipDrawer(const bool doCallBacks = true);
	void lowerInventoryDrawer(const bool doCallBacks = true);
	void lowerBiochipDrawer(const bool doCallBacks = true);

	void raiseInventoryDrawerForMessage();
	void lowerInventoryDrawerForMessage();
	bool isInventoryUp();
	bool isInventoryDown();

	InventoryResult addInventoryItem(InventoryItem *);
	InventoryResult removeInventoryItem(InventoryItem *);
	void removeAllItemsFromInventory();
	InventoryItem *getCurrentInventoryItem();
	void setCurrentInventoryItem(InventoryItem *);
	void setCurrentInventoryItemID(ItemID);
	InventoryResult addBiochip(BiochipItem *);
	void removeAllItemsFromBiochips();
	BiochipItem *getCurrentBiochip();
	void setCurrentBiochip(BiochipItem *);
	void setCurrentBiochipID(ItemID);

	void setDate(const uint16);

	void playEndMessage();

	void throwAwayInterface();

protected:
	void validateBackground();
	void validateDateMonitor();
	void validateCompass();
	void validateNotifications();
	void validateAIArea();
	void validateInventoryPanel();
	void validateBiochipPanel();
	void validateEnergyMonitor();

	void throwAwayBackground();
	void throwAwayDateMonitor();
	void throwAwayCompass();
	void throwAwayNotifications();
	void throwAwayAIArea();
	void throwAwayInventoryPanel();
	void throwAwayBiochipPanel();
	void throwAwayEnergyMonitor();

	void receiveNotification(Notification *, const NotificationFlags);
	void inventoryLidOpen(const bool doCallBacks);
	void inventoryLidClosed();
	void inventoryDrawerUp();
	void inventoryDrawerDown(const bool doCallBacks);
	void biochipLidOpen(const bool doCallBacks);
	void biochipLidClosed();
	void biochipDrawerUp();
	void biochipDrawerDown(const bool doCallBacks);

	Picture _background1;
	Picture _background2;
	Picture _background3;
	Picture _background4;

	Picture _datePicture;

	InputHandler *_previousHandler;

	Push _inventoryPush;
	SpriteSequence _inventoryLid;
	NotificationCallBack _inventoryPushCallBack;
	NotificationCallBack _inventoryLidCallBack;
	InventoryItemsPicture _inventoryPanel;
	bool _inventoryUp, _inventoryRaised;

	Push _biochipPush;
	SpriteSequence _biochipLid;
	NotificationCallBack _biochipPushCallBack;
	NotificationCallBack _biochipLidCallBack;
	BiochipPicture _biochipPanel;
	bool _biochipUp, _biochipRaised;

	Hotspot _currentItemSpot;
	Hotspot _currentBiochipSpot;

	Notification _interfaceNotification;

	bool _playingEndMessage;
};

extern Interface *g_interface;

} // End of namespace Pegasus

#endif
