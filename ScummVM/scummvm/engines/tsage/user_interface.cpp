/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "tsage/user_interface.h"
#include "tsage/core.h"
#include "tsage/tsage.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

void StripProxy::process(Event &event) {
	if (_action)
		_action->process(event);
}

/*--------------------------------------------------------------------------*/

void UIElement::synchronize(Serializer &s) {
	BackgroundSceneObject::synchronize(s);
	s.syncAsSint16LE(_field88);
	s.syncAsSint16LE(_enabled);
	s.syncAsSint16LE(_frameNum);
}

void UIElement::setup(int visage, int stripNum, int frameNum, int posX, int posY, int priority) {
	_field88 = 0;
	_frameNum = frameNum;
	_enabled = true;

	SceneObject::setup(visage, stripNum, frameNum, posX, posY, priority);
}

void UIElement::setEnabled(bool flag) {
	if (_enabled != flag) {
		_enabled = flag;
		setFrame(_enabled ? _frameNum : _frameNum + 2);
	}
}

/*--------------------------------------------------------------------------*/

void UIQuestion::process(Event &event) {
	if (event.eventType == EVENT_BUTTON_DOWN) {
		CursorType currentCursor = GLOBALS._events.getCursor();
		GLOBALS._events.hideCursor();
		showDescription(currentCursor);

		event.handled = true;
	}
}

void UIQuestion::showDescription(CursorType cursor) {
	switch (g_vm->getGameID()) {
	case GType_BlueForce:
		if (cursor == INV_FOREST_RAP) {
			// Forest rap item has a graphical display
			showItem(5, 1, 1);
		} else {
			// Display object description
			SceneItem::display2(9001, (int)cursor);
		}
		break;
	case GType_Ringworld2:
		if ((cursor == R2_COM_SCANNER) || (cursor == R2_COM_SCANNER_2)) {
			// Show communicator
			warning("TODO: Communicator");
		} else {
			// Show object description
			SceneItem::display2(3, (int)cursor);
		}
		break;
	default:
		break;
	}
}

void UIQuestion::setEnabled(bool flag) {
	if (_enabled != flag) {
		UIElement::setEnabled(flag);
		T2_GLOBALS._uiElements.draw();
	}
}

void UIQuestion::showItem(int resNum, int rlbNum, int frameNum) {
	GfxDialog::setPalette();

	// Get the item to display
	GfxSurface objImage = surfaceFromRes(resNum, rlbNum, frameNum);
	Rect imgRect;
	imgRect.resize(objImage, 0, 0, 100);
	imgRect.center(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	// Save the area behind where the image will be displayed
	GfxSurface *savedArea = surfaceGetArea(GLOBALS.gfxManager().getSurface(), imgRect);

	// Draw the image
	GLOBALS.gfxManager().copyFrom(objImage, imgRect);

	// Wait for a press
	GLOBALS._events.waitForPress();

	// Restore the old area
	GLOBALS.gfxManager().copyFrom(*savedArea, imgRect);
	delete savedArea;
}

/*--------------------------------------------------------------------------*/

void UIScore::postInit(SceneObjectList *OwnerList) {
	int xp = 266;
	_digit3.setup(1, 6, 1, xp, 180, 255);
	_digit3.reposition();
	xp += 7;
	_digit2.setup(1, 6, 1, xp, 180, 255);
	_digit2.reposition();
	xp += 7;
	_digit1.setup(1, 6, 1, xp, 180, 255);
	_digit1.reposition();
	xp += 7;
	_digit0.setup(1, 6, 1, xp, 180, 255);
	_digit0.reposition();
}

void UIScore::draw() {
	_digit3.draw();
	_digit2.draw();
	_digit1.draw();
	_digit0.draw();
}

void UIScore::updateScore() {
	int score = T2_GLOBALS._uiElements._scoreValue;

	_digit3.setFrame(score / 1000 + 1); score %= 1000;
	_digit2.setFrame(score / 100 + 1); score %= 100;
	_digit1.setFrame(score / 10 + 1); score %= 10;
	_digit0.setFrame(score + 1);
}

/*--------------------------------------------------------------------------*/

UIInventorySlot::UIInventorySlot(): UIElement() {
	_objIndex = 0;
	_object = NULL;
}

void UIInventorySlot::synchronize(Serializer &s) {
	UIElement::synchronize(s);
	s.syncAsSint16LE(_objIndex);
	SYNC_POINTER(_object);
}

void UIInventorySlot::process(Event &event) {
	if (event.eventType == EVENT_BUTTON_DOWN) {
		event.handled = true;

		// Check if game has a select item handler, and if so, give it a chance to check
		// whether something special happens when the item is selected
		if (!T2_GLOBALS._onSelectItem || !T2_GLOBALS._onSelectItem((CursorType)_objIndex))
			_object->setCursor();
	}
}

/*--------------------------------------------------------------------------*/

UIInventoryScroll::UIInventoryScroll() {
	_isLeft = false;
}

void UIInventoryScroll::synchronize(Serializer &s) {
	UIElement::synchronize(s);
	s.syncAsSint16LE(_isLeft);
}

void UIInventoryScroll::process(Event &event) {
	switch (event.eventType) {
	case EVENT_BUTTON_DOWN:
		// Draw the button as selected
		toggle(true);

		// Wait for the mouse to be released
		BF_GLOBALS._events.waitForPress(EVENT_BUTTON_UP);

		// Restore unselected version
		toggle(false);

		// Scroll the inventory as necessary
		T2_GLOBALS._uiElements.scrollInventory(_isLeft);
		event.handled = true;
		break;
	default:
		break;
	}
}

void UIInventoryScroll::toggle(bool pressed) {
	if (_enabled) {
		setFrame(pressed ? (_frameNum + 1) : _frameNum);
		T2_GLOBALS._uiElements.draw();
	}
}

/*--------------------------------------------------------------------------*/

UICollection::UICollection(): EventHandler() {
	_clearScreen = false;
	_visible = false;
	_cursorChanged = false;
}

void UICollection::setup(const Common::Point &pt) {
	_position = pt;
	_bounds.left = _bounds.right = pt.x;
	_bounds.top = _bounds.bottom = pt.y;
}

void UICollection::hide() {
	erase();
	_visible = false;
}

void UICollection::show() {
	_visible = true;
	draw();
}

void UICollection::erase() {
	if (_clearScreen) {
		Rect tempRect(0, UI_INTERFACE_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
		GLOBALS._screenSurface.fillRect(tempRect, 0);
		GLOBALS._sceneManager._scene->_backSurface.fillRect(tempRect, 0);
		_clearScreen = false;
	}
}

void UICollection::resetClear() {
	_clearScreen = false;
}

void UICollection::draw() {
	if (_visible) {
		// Temporarily reset the sceneBounds when drawing UI elements to force them on-screen
		Rect savedBounds = g_globals->_sceneManager._scene->_sceneBounds;
		g_globals->_sceneManager._scene->_sceneBounds = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		// Draw the elements onto the background
		for (uint idx = 0; idx < _objList.size(); ++idx)
			_objList[idx]->draw();

		// Draw the resulting UI onto the screen
		GLOBALS._screenSurface.copyFrom(GLOBALS._sceneManager._scene->_backSurface,
			Rect(0, UI_INTERFACE_Y, SCREEN_WIDTH, SCREEN_HEIGHT),
			Rect(0, UI_INTERFACE_Y, SCREEN_WIDTH, SCREEN_HEIGHT));

		_clearScreen = 1;
		g_globals->_sceneManager._scene->_sceneBounds = savedBounds;
	}
}

/*--------------------------------------------------------------------------*/

UIElements::UIElements(): UICollection() {
	if (g_vm->getGameID() == GType_Ringworld2)
		_cursorVisage.setVisage(5, 1);
	else
		_cursorVisage.setVisage(1, 5);
	g_saver->addLoadNotifier(&UIElements::loadNotifierProc);
}

void UIElements::synchronize(Serializer &s) {
	UICollection::synchronize(s);

	s.syncAsSint16LE(_slotStart);
	s.syncAsSint16LE(_scoreValue);
	s.syncAsByte(_active);

	int count = _itemList.size();
	s.syncAsSint16LE(count);
	if (s.isLoading()) {
		// Load in item list
		_itemList.clear();

		for (int idx = 0; idx < count; ++idx) {
			int itemId;
			s.syncAsSint16LE(itemId);
			_itemList.push_back(itemId);
		}
	} else {
		// Save item list
		for (int idx = 0; idx < count; ++idx) {
			int itemId = _itemList[idx];
			s.syncAsSint16LE(itemId);
		}
	}
}

void UIElements::process(Event &event) {
	if (_clearScreen && GLOBALS._player._enabled &&
			((g_vm->getGameID() != GType_BlueForce) || (GLOBALS._sceneManager._sceneNumber != 50))) {
		if (_bounds.contains(event.mousePos)) {
			// Cursor inside UI area
			if (!_cursorChanged) {
				if (GLOBALS._events.isInventoryIcon()) {
					// Inventory icon being displayed, so leave alone
				} else {
					// Change to the inventory use cursor
					int cursorId = (g_vm->getGameID() == GType_Ringworld2) ? 11 : 6;
					GfxSurface surface = _cursorVisage.getFrame(cursorId);
					GLOBALS._events.setCursor(surface);
				}
				_cursorChanged = true;
			}

			// Pass event to any element that the cursor falls on
			for (int idx = (int)_objList.size() - 1; idx >= 0; --idx) {
				if (_objList[idx]->_bounds.contains(event.mousePos) && _objList[idx]->_enabled) {
					_objList[idx]->process(event);
					if (event.handled)
						break;
				}
			}

			if (event.eventType == EVENT_BUTTON_DOWN)
				event.handled = true;

		} else if (_cursorChanged) {
			// Cursor outside UI area, so reset as necessary
			GLOBALS._events.setCursor(GLOBALS._events.getCursor());
			_cursorChanged = false;
/*
			SceneExt *scene = (SceneExt *)GLOBALS._sceneManager._scene;
			if (scene->_focusObject) {
				GfxSurface surface = _cursorVisage.getFrame(7);
				GLOBALS._events.setCursor(surface);
			}
*/
		}
	}
}

void UIElements::setup(const Common::Point &pt) {
	_slotStart = 0;
	_itemList.clear();
	_scoreValue = 0;
	_active = true;
	UICollection::setup(pt);
	hide();

	_background.setup(1, 3, 1, 0, 0, 255);
	add(&_background);

	// Set up the inventory slots
	int xp = 0;
	for (int idx = 0; idx < 4; ++idx) {
		UIElement *item = NULL;
		switch (idx) {
		case 0:
			item = &_slot1;
			break;
		case 1:
			item = &_slot2;
			break;
		case 2:
			item = &_slot3;
			break;
		case 3:
			item = &_slot4;
			break;
		}

		xp = idx * 63 + 2;
		if (g_vm->getGameID() == GType_BlueForce) {
			item->setup(9, 1, idx, xp, 4, 255);
		} else {
			item->setup(7, 1, idx, xp, 4, 255);
		}
		add(item);
	}

	// Setup bottom-right hand buttons
	xp += 62;
	int yp = (g_vm->getGameID() == GType_BlueForce) ? 16 : 17;
	_question.setup(1, 4, 7, xp, yp, 255);
	_question.setEnabled(false);
	add(&_question);

	xp += 21;
	_scrollLeft.setup(1, 4, 1, xp, yp, 255);
	add(&_scrollLeft);
	_scrollLeft._isLeft = true;

	xp += 22;
	_scrollRight.setup(1, 4, 4, xp, yp, 255);
	add(&_scrollRight);
	_scrollRight._isLeft = false;

	switch (g_vm->getGameID()) {
	case GType_BlueForce:
		// Set up the score
		_score.postInit();
		add(&_score);
		break;
	case GType_Ringworld2:
		// Set up the character display
		_character.setup(1, 5, R2_GLOBALS._player._characterIndex, 285, 11, 255);
		add(&_character);
		break;
	default:
		break;
	}

	// Set interface area
	_bounds = Rect(0, UI_INTERFACE_Y - 1, SCREEN_WIDTH, SCREEN_HEIGHT);

	updateInventory();
}

void UIElements::add(UIElement *obj) {
	// Add object
	assert(_objList.size() < 12);
	_objList.push_back(obj);

	obj->setPosition(Common::Point(_bounds.left + obj->_position.x, _bounds.top + obj->_position.y));
	obj->reposition();

	GfxSurface s = obj->getFrame();
	s.draw(obj->_position);
}

/**
 * Handles updating the visual inventory in the user interface
 */
void UIElements::updateInventory() {
	switch (g_vm->getGameID()) {
	case GType_BlueForce:
		// Update the score
		_score.updateScore();
		break;
	case GType_Ringworld2:
		_character.setFrame(R2_GLOBALS._player._characterIndex);
		break;
	}

	updateInvList();

	// Enable scroll buttons if the player has more than four items
	if (_itemList.size() > 4) {
		_scrollLeft.setEnabled(true);
		_scrollRight.setEnabled(true);
	} else {
		_scrollLeft.setEnabled(false);
		_scrollRight.setEnabled(false);
	}

	// Handle cropping the slots start within inventory
	int lastPage  = (_itemList.size() - 1) / 4 + 1;
	if (_slotStart < 0)
		_slotStart = lastPage - 1;
	else if (_slotStart > (lastPage - 1))
		_slotStart = 0;

	// Handle refreshing slot graphics
	UIInventorySlot *slotList[4] = { &_slot1, &_slot2, &_slot3, &_slot4 };

	// Loop through the inventory objects
	SynchronizedList<InvObject *>::iterator i;
	int objIndex = 0;
	for (i = GLOBALS._inventory->_itemList.begin(); i != GLOBALS._inventory->_itemList.end(); ++i, ++objIndex) {
		InvObject *obj = *i;

		// Check whether the object is in any of the four inventory slots
		for (int slotIndex = 0; slotIndex < 4; ++slotIndex) {
			int idx = _slotStart * 4 + slotIndex;
			int objectIdx = (idx < (int)_itemList.size()) ? _itemList[idx] : 0;

			if (objectIdx == objIndex) {
				UIInventorySlot *slot = slotList[slotIndex];

				slot->_objIndex = objIndex;
				slot->_object = obj;
				slot->setVisage(obj->_visage);
				slot->setStrip(obj->_strip);
				slot->setFrame(obj->_frame);

				slot->reposition();
			}
		}
	}

	// Refresh the display if necessary
	if (_active)
		draw();
}

/**
 * Update the list of the indexes of items in the player's inventory
 */
void UIElements::updateInvList() {
	// Update the index list of items in the player's inventory
	_itemList.clear();

	SynchronizedList<InvObject *>::iterator i;
	int itemIndex = 0;
	for (i = GLOBALS._inventory->_itemList.begin(); i != GLOBALS._inventory->_itemList.end(); ++i, ++itemIndex) {
		InvObject *invObject = *i;
		if (invObject->inInventory())
			_itemList.push_back(itemIndex);
	}
}

/**
 * Set the game score
 */
void UIElements::addScore(int amount) {
	_scoreValue += amount;
	T2_GLOBALS._inventorySound.play(0);
	updateInventory();
}

/*
 * Scroll the inventory slots
 */
void UIElements::scrollInventory(bool isLeft) {
	if (isLeft)
		--_slotStart;
	else
		++_slotStart;

	updateInventory();
}

void UIElements::loadNotifierProc(bool postFlag) {
	if (postFlag && T2_GLOBALS._uiElements._active)
		T2_GLOBALS._uiElements.show();
}

} // End of namespace TsAGE
