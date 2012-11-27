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

#include "pegasus/pegasus.h"
#include "pegasus/transition.h"
#include "pegasus/items/inventorypicture.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/inventory/inventoryitem.h"

namespace Pegasus {

InventoryPicture::InventoryPicture(const DisplayElementID id, InputHandler *nextHandler, Inventory *inventory) :
		InputHandler(nextHandler), Picture(id), _panelMovie(kNoDisplayElement){
	_inventory = inventory;
	_lastReferenceCount = 0xffffffff;

	if (_inventory->getNumItems() > 0) {
		_currentItemIndex = 0;
		_currentItem = (Item *)_inventory->getItemAt(0);
	} else {
		_currentItemIndex = -1;
		_currentItem = 0;
	}

	_active = false;
	_shouldDrawHighlight = true;
	_itemsPerRow = 1;
	_numberOfRows = 1;
	_itemWidth = 0;
	_itemHeight = 0;
	_itemX = 0;
	_itemY = 0;
}

void InventoryPicture::initInventoryImage(Transition *transition) {
	initFromPICTFile(_pictName, true);
	_panelMovie.shareSurface(this);
	_panelMovie.initFromMovieFile(_movieName);
	_panelMovie.getBounds(_highlightBounds);
	_panelMovie.setTriggeredElement(transition);
	_highlightImage.initFromPICTFile(_highlightName, true);
}

void InventoryPicture::throwAwayInventoryImage() {
	if (isSurfaceValid()) {
		_panelMovie.releaseMovie();
		_highlightImage.deallocateSurface();
		deallocateSurface();
	}
}

void InventoryPicture::getItemXY(uint32 index, CoordType &x, CoordType &y) {
	x = (index % _itemsPerRow) * _itemWidth + _itemX;
	y = (index / _itemsPerRow) * _itemHeight + _itemY;
}

void InventoryPicture::drawItemHighlight(const Common::Rect &r) {
	if (_highlightImage.isSurfaceValid()) {
		Common::Rect r2 = _highlightBounds;
		Common::Rect bounds;
		getBounds(bounds);

		r2.translate(bounds.left, bounds.top);
		r2 = r2.findIntersectingRect(r);
		if (!r2.isEmpty()) {
			Common::Rect r1 = r2;
			r1.translate(-bounds.left - _highlightBounds.left, -bounds.top - _highlightBounds.top);
			_highlightImage.drawImage(r1, r2);
		}
	}
}

void InventoryPicture::draw(const Common::Rect &r) {
	Picture::draw(r);
	if (_inventory->getNumItems() != 0 && _shouldDrawHighlight)
		drawItemHighlight(r);
}

// Assumes index >= 0.
void InventoryPicture::setCurrentItemIndex(int32 index) {
	if (index >= _inventory->getNumItems())
		index = _inventory->getNumItems() - 1;

	Item *currentItem = 0;
	if (index >= 0)
		currentItem = (Item *)_inventory->getItemAt(index);

	if (currentItem != _currentItem) {
		if (_currentItem) {
			if (_currentItem->isSelected())
				_currentItem->deselect();

			if (_active)
				unhighlightCurrentItem();
		}

		_currentItemIndex = index;
		_currentItem = currentItem;
		if (_currentItem) {
			_currentItem->select();

			if (_active)
				highlightCurrentItem();
		}

		if (_active)
			triggerRedraw();
	}
}

void InventoryPicture::setCurrentItemID(ItemID id) {
	int32 index = _inventory->findIndexOf(id);
	if (index >= 0)
		setCurrentItemIndex(index);
}

InventoryResult InventoryPicture::addInventoryItem(Item *item) {
	InventoryResult result = _inventory->addItem(item);

	if (result == kInventoryOK)
		setCurrentItemIndex(_inventory->findIndexOf(item));

	return result;
}

InventoryResult InventoryPicture::removeInventoryItem(Item *item) {
	InventoryResult result = _inventory->removeItem(item);

	if (result == kInventoryOK)
		setCurrentItemIndex(getCurrentItemIndex());

	return result;
}

void InventoryPicture::removeAllItems() {
	_inventory->removeAllItems();
	setCurrentItemIndex(0);
}

bool InventoryPicture::itemInInventory(Item *item) {
	return _inventory->itemInInventory(item);
}

bool InventoryPicture::itemInInventory(const ItemID id) {
	return _inventory->itemInInventory(id);
}

void InventoryPicture::panelUp() {
	allowInput(true);
}

// Must ensure that the picture matches the _inventory member variable.
void InventoryPicture::activateInventoryPicture() {
	if (_active)
		return;

	allowInput(false);

	if (_lastReferenceCount != _inventory->getReferenceCount()) {
		uint32 numItems = _inventory->getNumItems();

		CoordType x, y;
		getItemXY(0, x, y);
		_panelMovie.moveMovieBoxTo(x, y);
		_panelMovie.show();

		for (uint32 i = 0; i < numItems; i++) {
			Item *item = (Item *)_inventory->getItemAt(i);
			if (item == _currentItem)
				item->select();

			getItemXY(i, x, y);
			_panelMovie.moveMovieBoxTo(x, y);
			_panelMovie.setTime(getItemPanelTime(item));
			_panelMovie.redrawMovieWorld();
		}

		uint32 numSlots = _itemsPerRow * _numberOfRows;

		for (uint32 i = numItems; i < numSlots; i++) {
			getItemXY(i, x, y);
			_panelMovie.moveMovieBoxTo(x, y);
			_panelMovie.setTime(0);
			_panelMovie.redrawMovieWorld();
		}

		_lastReferenceCount = _inventory->getReferenceCount();
	}

	show(); // *** Do we really need this?
	if (_currentItem)
		highlightCurrentItem();

	_active = true;
}

void InventoryPicture::deactivateInventoryPicture() {
	if (!_active)
		return;

	_active = false;
	allowInput(false);
	_panelMovie.hide();
	hide();

	if (_inventory->getNumItems() != 0)
		if (!_currentItem->isActive())
			_currentItem->activate();
}

void InventoryPicture::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (_active) {
		if (input.upButtonDown()) {
			if (_currentItemIndex - _itemsPerRow >= 0)
				setCurrentItemIndex(_currentItemIndex - _itemsPerRow);
		} else if (input.downButtonDown()) {
			if (_currentItemIndex + _itemsPerRow < _inventory->getNumItems())
				setCurrentItemIndex(_currentItemIndex + _itemsPerRow);
		} else if (input.leftButtonDown()) {
			if ((_currentItemIndex % _itemsPerRow) != 0)
				setCurrentItemIndex(_currentItemIndex - 1);
		} else if (input.rightButtonDown()) {
			if (((_currentItemIndex + 1) % _itemsPerRow) != 0 && _currentItemIndex + 1 < _inventory->getNumItems())
				setCurrentItemIndex(_currentItemIndex + 1);
		}
	}

	InputHandler::handleInput(input, cursorSpot);
}

void InventoryPicture::highlightCurrentItem() {
	CoordType x, y;
	getItemXY(_currentItemIndex, x, y);
	_highlightBounds.moveTo(x, y);
}

InventoryItemsPicture::InventoryItemsPicture(const DisplayElementID id, InputHandler *nextHandler, Inventory *inventory) :
		InventoryPicture(id, nextHandler, inventory) {
	_pictName = "Images/Items/Inventory/Inventory Panel";
	_movieName = "Images/Items/Inventory/Inventory Panel Movie";
	_highlightName = "Images/Items/Inventory/Inventory Hilite";

	_itemsPerRow = 3;
	_numberOfRows = 3;
	_itemWidth = 88;
	_itemHeight = 64;
	_itemX = 8;
	_itemY = 26;
	_isLooping = true;
}

void InventoryItemsPicture::loopCurrentItem() {
	if (_isLooping) {
		CoordType x, y;
		getItemXY(_currentItemIndex, x, y);
		_panelMovie.moveMovieBoxTo(x, y);
		_highlightBounds.moveTo(x, y);

		TimeValue start, stop;
		((InventoryItem *)_currentItem)->getPanelTimes(start, stop);
		_panelMovie.stop();
		_panelMovie.setFlags(0);
		_panelMovie.setSegment(start, stop);
		_panelMovie.setFlags(kLoopTimeBase);
		_panelMovie.setTime(((InventoryItem *)_currentItem)->getAnimationTime());
		_panelMovie.start();
	}
}

void InventoryItemsPicture::highlightCurrentItem() {
	InventoryPicture::highlightCurrentItem();
	loopCurrentItem();
}

void InventoryItemsPicture::unhighlightCurrentItem() {
	InventoryPicture::unhighlightCurrentItem();
	_panelMovie.stop();
	_panelMovie.setFlags(0);
	((InventoryItem *)_currentItem)->setAnimationTime(_panelMovie.getTime());
}

TimeValue InventoryItemsPicture::getItemPanelTime(Item *item) {
	TimeValue start, stop;
	((InventoryItem *)item)->getPanelTimes(start, stop);
	((InventoryItem *)item)->setAnimationTime(start);
	return start;
}

void InventoryItemsPicture::deactivateInventoryPicture() {
	if (_active) {
		InventoryPicture::deactivateInventoryPicture();
		_panelMovie.stop();
		_panelMovie.setFlags(0);
		_panelMovie.setSegment(0, _panelMovie.getDuration());
		_isLooping = true;
	}
}

void InventoryItemsPicture::playEndMessage(DisplayElement *pushElement) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	Movie endMessage(0);

	_shouldDrawHighlight = false;
	endMessage.shareSurface(this);
	endMessage.initFromMovieFile("Images/Caldoria/A56 Congrats");
	endMessage.moveMovieBoxTo(kFinalMessageLeft - kInventoryPushLeft, kFinalMessageTop - kInventoryPushTop);
	endMessage.setTriggeredElement(pushElement);
	endMessage.start();

	while (endMessage.isRunning()) {
		vm->checkCallBacks();
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	endMessage.stop();
}

BiochipPicture::BiochipPicture(const DisplayElementID id, InputHandler *nextHandler, Inventory *inventory) :
		InventoryPicture(id, nextHandler, inventory) {
	_pictName = "Images/Items/Biochips/Biochip Panel";
	_movieName = "Images/Items/Biochips/Biochip Panel Movie";
	_highlightName = "Images/Items/Biochips/BioChip Hilite";

	_itemsPerRow = 4;
	_numberOfRows = 2;
	_itemWidth = 46;
	_itemHeight = 46;
	_itemX = 4;
	_itemY = 24;
}

void BiochipPicture::unhighlightCurrentItem() {
	InventoryPicture::unhighlightCurrentItem();
	CoordType x, y;
	getItemXY(_currentItemIndex, x, y);
	_panelMovie.show();
	_panelMovie.moveMovieBoxTo(x, y);
	_panelMovie.setTime(getItemPanelTime(_currentItem));
	_panelMovie.redrawMovieWorld();
}

TimeValue BiochipPicture::getItemPanelTime(Item *item) {
	return ((BiochipItem *)item)->getPanelTime();
}

} // End of namespace Pegasus
