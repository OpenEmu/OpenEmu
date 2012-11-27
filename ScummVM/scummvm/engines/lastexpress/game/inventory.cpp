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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lastexpress/game/inventory.h"

#include "lastexpress/data/cursor.h"
#include "lastexpress/data/scene.h"
#include "lastexpress/data/snd.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/menu/menu.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"


namespace LastExpress {

Inventory::Inventory(LastExpressEngine *engine) : _engine(engine), _selectedItem(kItemNone), _highlightedItemIndex(0), _itemsShown(0),
	_showingHourGlass(false), _blinkingDirection(1), _blinkingBrightness(0),
	_useMagnifier(false), _portraitHighlighted(false), _isOpened(false), _eggHightlighted(false), _itemScene(NULL) {

	//_inventoryRect = Common::Rect(0, 0, 32, 32);
	_menuEggRect = Common::Rect(608, 448, 640, 480);
	_selectedItemRect = Common::Rect(44, 0, 76, 32);

	init();
}

Inventory::~Inventory() {
	_itemScene = NULL;

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Inventory handling
//////////////////////////////////////////////////////////////////////////

// Initialize inventory contents
void Inventory::init() {
	// ID
	_entries[kItemMatchBox].cursor = kCursorMatchBox;
	_entries[kItemTelegram].cursor = kCursorTelegram;
	_entries[kItemPassengerList].cursor = kCursorPassengerList;
	_entries[kItemArticle].cursor = kCursorArticle;
	_entries[kItemScarf].cursor = kCursorScarf;
	_entries[kItemPaper].cursor = kCursorPaper;
	_entries[kItemParchemin].cursor = kCursorParchemin;
	_entries[kItemMatch].cursor = kCursorMatch;
	_entries[kItemWhistle].cursor = kCursorWhistle;
	_entries[kItemKey].cursor = kCursorKey;
	_entries[kItemBomb].cursor = kCursorBomb;
	_entries[kItemFirebird].cursor = kCursorFirebird;
	_entries[kItemBriefcase].cursor = kCursorBriefcase;
	_entries[kItemCorpse].cursor = kCursorCorpse;

	// Selectable
	_entries[kItemMatchBox].isSelectable = true;
	_entries[kItemMatch].isSelectable = true;
	_entries[kItemTelegram].isSelectable = true;
	_entries[kItemWhistle].isSelectable = true;
	_entries[kItemKey].isSelectable = true;
	_entries[kItemFirebird].isSelectable = true;
	_entries[kItemBriefcase].isSelectable = true;
	_entries[kItemCorpse].isSelectable = true;
	_entries[kItemPassengerList].isSelectable = true;

	// Auto selection
	_entries[kItem2].manualSelect = false;
	_entries[kItem3].manualSelect = false;
	_entries[kItem5].manualSelect = false;
	_entries[kItem7].manualSelect = false;
	_entries[kItem9].manualSelect = false;
	_entries[kItem11].manualSelect = false;
	_entries[kItemBeetle].manualSelect = false;
	_entries[kItem17].manualSelect = false;
	_entries[kItemFirebird].manualSelect = false;
	_entries[kItemBriefcase].manualSelect = false;
	_entries[kItemCorpse].manualSelect = false;
	_entries[kItemGreenJacket].manualSelect = false;
	_entries[kItem22].manualSelect = false;

	// Scene
	_entries[kItemMatchBox].scene = kSceneMatchbox;
	_entries[kItemTelegram].scene = kSceneTelegram;
	_entries[kItemPassengerList].scene = kScenePassengerList;
	_entries[kItemScarf].scene = kSceneScarf;
	_entries[kItemParchemin].scene = kSceneParchemin;
	_entries[kItemArticle].scene = kSceneArticle;
	_entries[kItemPaper].scene = kScenePaper;
	_entries[kItemFirebird].scene = kSceneFirebird;
	_entries[kItemBriefcase].scene = kSceneBriefcase;

	// Has item
	_entries[kItemTelegram].isPresent = true;
	_entries[kItemArticle].isPresent = true;

	_selectedItem = kItemNone;
}

void Inventory::handleMouseEvent(const Common::Event &ev) {
	_useMagnifier = false;

	// Egg (menu)
	if (!_menuEggRect.contains(ev.mouse)) {
		// Remove highlight if needed
		if (_eggHightlighted) {
			if (!getGlobalTimer()) {
				drawItem((CursorStyle)(getMenu()->getGameId() + 39), 608, 448, 1);
				askForRedraw();
			}

			_eggHightlighted = false;
		}
	} else {
		// Highlight menu
		if (!_eggHightlighted) {
			if (!getGlobalTimer()) {
				drawItem((CursorStyle)(getMenu()->getGameId() + 39), 608, 448);
				askForRedraw();
			}

			_eggHightlighted = true;
		}

		// If clicked, show the menu
		if (ev.type == Common::EVENT_LBUTTONDOWN) {
			_eggHightlighted = false;
			_portraitHighlighted = false;
			_isOpened = false;

			getSound()->playSoundWithSubtitles("LIB039.SND", kFlagMenuClock, kEntityPlayer);

			getMenu()->show(true, kSavegameTypeIndex, 0);

		} else if (ev.type == Common::EVENT_RBUTTONDOWN && getGlobalTimer()) {
			if (getSoundQueue()->isBuffered("TIMER"))
				getSoundQueue()->removeFromQueue("TIMER");

			setGlobalTimer(900);
		}
	}

	// Selected item
	if (ev.mouse.x >= 32) {
		if (_isOpened) {
			// If we are currently inside inventory with the mouse pressed
			if (getFlags()->mouseLeftPressed) {
				if (_highlightedItemIndex)
					drawHighlight(_highlightedItemIndex, true);
			} else {
				// The user released the mouse button, we need to update the inventory state (clear highlight and items)
				drawItem((CursorStyle)getProgress().portrait, 0, 0, 1);
				_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(0, 44, 32, (int16)(40 * _itemsShown + 40)));
				_isOpened = false;
				askForRedraw();

				drawSelectedItem();

				// Load backup scene
				if (getState()->sceneUseBackup) {
					SceneIndex scene = kSceneNone;
					if (getState()->sceneBackup2) {
						scene = getState()->sceneBackup2;
						getState()->sceneBackup2 = kSceneNone;

						// Load our scene
						getScenes()->loadScene(scene);
					} else if (!getEvent(kEventKronosBringFirebird) && !getProgress().isEggOpen) {
						getState()->sceneUseBackup = false;
						scene = getState()->sceneBackup;

						if (getEntities()->getPosition(getScenes()->get(scene)->car, getScenes()->get(scene)->position))
							scene = getScenes()->processIndex(getState()->sceneBackup);

						// Load our scene
						getScenes()->loadScene(scene);
					}
				}
			}
		} else {
			if (_portraitHighlighted) {
				drawItem((CursorStyle)getProgress().portrait, 0, 0, 1);
				askForRedraw();
				_portraitHighlighted = false;
			}

			// Magnifier
			if (_selectedItem != kItemNone
			 && get(_selectedItem)->scene != kSceneNone
			 && _selectedItemRect.contains(ev.mouse)) {

				if (!getState()->sceneUseBackup || (getState()->sceneBackup2 && getFirstExaminableItem() == _selectedItem))
					_useMagnifier = true;

				if (getFlags()->mouseLeftPressed) {
					if (getState()->sceneUseBackup) {
						if (getState()->sceneBackup2 && getFirstExaminableItem() == _selectedItem) {
							SceneIndex sceneIndex = getState()->sceneBackup2;
							getState()->sceneBackup2 = kSceneNone;

							getScenes()->loadScene(sceneIndex);
						}
					} else {
						getState()->sceneBackup = getState()->scene;
						getState()->sceneUseBackup = true;

						getScenes()->loadScene(get(_selectedItem)->scene);
					}
				}
			}
		}

		return;
	}

	// Opened inventory
	if (ev.mouse.y >= 32) {
		// Draw portrait (darkened) if the inventory is closed (and we are not on top of it)
		if (!_isOpened) {
			if (_portraitHighlighted) {
				drawItem((CursorStyle)getProgress().portrait, 0, 0, 1);
				askForRedraw();
				_portraitHighlighted = false;
			}

			return;
		}

		// Change item highlight on list
		if (getFlags()->mouseLeftPressed) {
			uint32 index = (uint16)ev.mouse.y / 40;

			if (_highlightedItemIndex && _highlightedItemIndex != index)
				drawHighlight(_highlightedItemIndex, true);

			if (index && index <= _itemsShown && index != _highlightedItemIndex)
				drawHighlight(index, false);

			return;
		}

		// User released the mouse button, check if we were on a valid item
		uint32 index = _highlightedItemIndex ? getItemIndex(_highlightedItemIndex) : 0;

		// Reset items and portrait
		drawItem((CursorStyle)getProgress().portrait, 0, 0, 1);
		_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(0, 44, 32, (int16)(40 * _itemsShown + 40)));
		askForRedraw();
		_highlightedItemIndex = 0;
		_itemsShown = 0;

		// Load the scene if an item has been selected
		if (index) {
			InventoryEntry entry = _entries[index];

			// If a scene is affected to the item
			if (entry.scene) {
				if (getState()->sceneUseBackup) {
					if (getFirstExaminableItem() && !getState()->sceneBackup2)
						getState()->sceneBackup2 = getState()->scene;
				} else {
					getState()->sceneUseBackup = true;
					getState()->sceneBackup = getState()->scene;
				}

				getScenes()->loadScene(entry.scene);
			}

			if (entry.field_2)
				selectItem((InventoryItem)index);
			else
				drawSelectedItem();

			// Set inventory as closed (will cause a cleanup on next call)
			_isOpened = false;

			return;
		}

		// Draw the currently selected item
		drawSelectedItem();

		// Stop processing if we are not looking at an item already
		if (!getState()->sceneUseBackup) {
			_isOpened = false;
			return;
		}

		SceneIndex scene = kSceneNone;
		if (getState()->sceneBackup2) {
			scene = getState()->sceneBackup2;
			getState()->sceneBackup2 = kSceneNone;
		} else {
			if (getEvent(kEventKronosBringFirebird) || getProgress().isEggOpen) {
				_isOpened = false;
				return;
			}

			getState()->sceneUseBackup = false;
			scene = getState()->sceneBackup;

			if (getEntities()->getPosition(getScenes()->get(scene)->car, getScenes()->get(scene)->position))
				scene = getScenes()->processIndex(getState()->sceneBackup);
		}

		getScenes()->loadScene(scene);

		_isOpened = false;

		return;
	}

	//
	if (!getProgress().field_84
	 && getEntityData(kEntityPlayer)->location != kLocationOutsideTrain
	 && getProgress().field_18 != 4
	 && (_selectedItem == kItemNone || get(_selectedItem)->manualSelect || getState()->sceneUseBackup)) {

		// Draw inventory contents when clicking on portrait
		if (ev.type == Common::EVENT_LBUTTONDOWN) {
			open();
			return;
		}

		if (!_portraitHighlighted && !_isOpened) {
			drawItem((CursorStyle)getProgress().portrait, 0, 0);
			_portraitHighlighted = true;
		} else if (!_isOpened || getFlags()->mouseLeftPressed) {
			// Do nothing
		} else if (_isOpened) {
			close();

			// Select item
			if (_selectedItem == kItemNone || get(_selectedItem)->manualSelect) {
				_selectedItem = getFirstExaminableItem();

				if (_selectedItem != kItemNone)
					drawItem(get(_selectedItem)->cursor, 44, 0);
			}

			// Restore scene
			if (getState()->sceneBackup) {

				if (getState()->sceneBackup2) {
					SceneIndex backup = getState()->sceneBackup2;
					getState()->sceneBackup2 = kSceneNone;

					getScenes()->loadScene(backup);
				} else if (!getEvent(kEventKronosBringFirebird)) {
					if (!getProgress().isEggOpen) {
						getState()->sceneBackup = kSceneNone;

						Scene *backup = getScenes()->get(getState()->sceneBackup);
						if (getEntities()->getPosition(backup->car, backup->position))
							getScenes()->loadScene(getScenes()->processIndex(getState()->sceneBackup));
						else
							getScenes()->loadScene(getState()->sceneBackup);
					}
				}
			}

			_portraitHighlighted = true;
		}

		// Draw highlighted item
		if (_highlightedItemIndex)
			drawHighlight(_highlightedItemIndex, true);
	}
}

//////////////////////////////////////////////////////////////////////////
// UI
//////////////////////////////////////////////////////////////////////////
void Inventory::show() {
	clearBg(GraphicsManager::kBackgroundInventory);
	askForRedraw();

	// Show portrait (first draw, cannot be highlighted)
	drawItem((CursorStyle)getProgress().portrait, 0, 0, 1);

	// Show selected item
	if (_selectedItem != kItemNone)
		drawItem(get(_selectedItem)->cursor, 44, 0);

	drawEgg();
}

void Inventory::setPortrait(InventoryItem item) const {
	getProgress().portrait = item;
	drawItem((CursorStyle)getProgress().portrait, 0, 0);
}

void Inventory::showHourGlass() const {
	if (!getMenu()->isShown())
		drawItem(kCursorHourGlass, 608, 448);

	getFlags()->shouldRedraw = false;

	askForRedraw();

	getFlags()->shouldDrawEggOrHourGlass = true;
}

//////////////////////////////////////////////////////////////////////////
// Items
//////////////////////////////////////////////////////////////////////////
Inventory::InventoryEntry *Inventory::get(InventoryItem item) {
	if (item >= kPortraitOriginal)
		error("[Inventory::get] Invalid inventory item");

	return &_entries[item];
}

void Inventory::addItem(InventoryItem item) {
	if (item >= kPortraitOriginal)
		return;

	get(item)->isPresent = true;
	get(item)->location = kObjectLocationNone;

	// Auto-select item if necessary
	if (get(item)->cursor && !get(item)->manualSelect) {
		_selectedItem = item;
		drawItem(get(_selectedItem)->cursor, 44, 0);
		askForRedraw();
	}
}

void Inventory::removeItem(InventoryItem item, ObjectLocation newLocation) {
	if (item >= kPortraitOriginal)
		return;

	get(item)->isPresent = false;
	get(item)->location = newLocation;

	if (get(item)->cursor == get(_selectedItem)->cursor) {
		_selectedItem = kItemNone;
		_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(44, 0, 44 + 32, 32));
		askForRedraw();
	}
}

bool Inventory::hasItem(InventoryItem item) {
	if (get(item)->isPresent && item < kPortraitOriginal)
		return true;

	return false;
}

void Inventory::selectItem(InventoryItem item) {
	_selectedItem = item;

	drawItem(get(_selectedItem)->cursor, 44, 0);
	askForRedraw();
}

void Inventory::unselectItem() {
	_selectedItem = kItemNone;

	_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(44, 0, 44 + 32, 32));
	askForRedraw();
}

void Inventory::setLocationAndProcess(InventoryItem item, ObjectLocation location) {
	if (item >= kPortraitOriginal)
		return;

	if (get(item)->location == location)
		return;

	get(item)->location = location;

	if (isItemSceneParameter(item) && !getFlags()->flag_0)
		getScenes()->processScene();
}

//////////////////////////////////////////////////////////////////////////
// Serializable
//////////////////////////////////////////////////////////////////////////
void Inventory::saveLoadWithSerializer(Common::Serializer &s) {
	for (uint i = 0; i < ARRAYSIZE(_entries); i++)
		_entries[i].saveLoadWithSerializer(s);
}

void Inventory::saveSelectedItem(Common::Serializer &s) {
	s.syncAsUint32LE(_selectedItem);
}

//////////////////////////////////////////////////////////////////////////
// toString
//////////////////////////////////////////////////////////////////////////
Common::String Inventory::toString() {
	Common::String ret = "";

	for (int i = 0; i < kPortraitOriginal; i++)
		ret += Common::String::format("%d : %s\n", i, _entries[i].toString().c_str());

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////
InventoryItem Inventory::getFirstExaminableItem() const {

	int index = 0;
	InventoryEntry entry = _entries[index];
	while (!entry.isPresent || !entry.cursor || entry.manualSelect) {
		index++;
		entry = _entries[index];

		if (index >= kPortraitOriginal)
			return kItemNone;
	}

	return (InventoryItem)index;
}

bool Inventory::isItemSceneParameter(InventoryItem item) const {
	Scene *scene = getScenes()->get(getState()->scene);

	switch(scene->type) {
	default:
		return false;

	case Scene::kTypeItem:
		if (scene->param1 == item)
			return true;
		break;

	case Scene::kTypeItem2:
		if (scene->param1 == item || scene->param2 == item)
			return true;
		break;

	case Scene::kTypeObjectItem:
		if (scene->param2 == item)
			return true;
		break;

	case Scene::kTypeItem3:
		if (scene->param1 == item || scene->param2 == item || scene->param3 == item)
			return true;
		break;

	case Scene::kTypeCompartmentsItem:
		if (scene->param2 == item)
			return true;
		break;
	}

	return false;
}

// Examine an inventory item
void Inventory::examine(InventoryItem item) {
	SceneIndex index = get(item)->scene;
	if (!index)
		return;

	/*if (!getState()->sceneUseBackup ||
		(getState()->sceneBackup2 && getFirstExaminableItem() == _selectedItem))
		flag = 1;*/

	if (!getState()->sceneUseBackup) {
		getState()->sceneBackup = getState()->scene;
		getState()->sceneUseBackup = true;

		getScenes()->loadScene(index);
	} else {

		if (!getState()->sceneBackup2)
			return;

		if (getFirstExaminableItem() == _selectedItem) {
			index = getState()->sceneBackup2;
			getState()->sceneBackup2 = kSceneNone;
			getScenes()->loadScene(index);
		}
	}
}

void Inventory::drawEgg() const {
	if (!getMenu()->isShown())
		drawItem((CursorStyle)(getMenu()->getGameId() + 39), 608, 448, _eggHightlighted ? 0 : 1);

	getFlags()->shouldDrawEggOrHourGlass = false;
}

// Blinking egg: we need to blink the egg for delta time, with the blinking getting faster until it's always lit.
void Inventory::drawBlinkingEgg(uint ticks) {
	uint globalTimer = (uint)getGlobalTimer();
	uint timerValue = (getProgress().jacket == kJacketGreen) ? 450 : 225;

	if (globalTimer == timerValue || globalTimer == 900) {
		_blinkingBrightness = 0;
		_blinkingDirection = 1;
	}

	globalTimer = globalTimer <= ticks ? 0 : globalTimer - ticks;
	setGlobalTimer(globalTimer);

	if (getFlags()->flag_0
	|| (globalTimer % 5) == 0
	|| (globalTimer <= 500 && (globalTimer % ((globalTimer + 100) / 100)) == 0))
		blinkEgg();

	if (globalTimer < 90) {
		if ((globalTimer + ticks) >= 90)
			getSound()->playSoundWithSubtitles("TIMER", (SoundFlag)(kFlagType13|kFlagDefault), kEntityPlayer);

		if (!getSoundQueue()->isBuffered("TIMER"))
			setGlobalTimer(0);
	}

	if (globalTimer == 0) {
		drawItem((CursorStyle)(getMenu()->getGameId() + 39), 608, 448, _menuEggRect.contains(getCoords()) ? 1 : -1);

		askForRedraw();

		getSaveLoad()->saveGame(kSavegameTypeAuto, kEntityChapters, 0);
	}
}

void Inventory::blinkEgg() {
	drawItem((CursorStyle)(getMenu()->getGameId() + 39), 608, 448, (_blinkingBrightness == 0) ? -1 : (int16)_blinkingBrightness);

	askForRedraw();

	_blinkingBrightness += _blinkingDirection;
	if (_blinkingBrightness == 0 || _blinkingBrightness == 3)
		_blinkingDirection = -_blinkingDirection;
}

void Inventory::drawItem(CursorStyle id, uint16 x, uint16 y, int16 brightnessIndex) const {
	Icon icon(id);
	icon.setPosition(x, y);

	if (brightnessIndex != -1)
		icon.setBrightness(brightnessIndex);

	_engine->getGraphicsManager()->draw(&icon, GraphicsManager::kBackgroundInventory);
}

void Inventory::drawSelectedItem() {
	// Draw the selected item if any
	if (!_selectedItem || get(_selectedItem)->manualSelect) {
		_selectedItem = getFirstExaminableItem();

		if (_selectedItem) {
			drawItem(get(_selectedItem)->cursor, 44, 0);
		} else {
			clearSelectedItem();
		}
		askForRedraw();
	}
}

void Inventory::clearSelectedItem() const {
	_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(44, 0, 44 + 32, 32));
}

// Open inventory: show portrait selected and draw contents
void Inventory::open() {
	_portraitHighlighted = false;
	_isOpened = true;

	// Draw highlighted portrait
	drawItem((CursorStyle)(getProgress().portrait + 1), 0, 0);

	// Draw at most 11 items in the inventory
	_itemsShown = 0;
	for (int i = 1; i < ARRAYSIZE(_entries); i++) {
		if (!_entries[i].isPresent)
			continue;

		if (!_entries[i].manualSelect)
			continue;

		if (_itemsShown < 11) {
			drawItem(_entries[i].cursor, 0, 40 * _itemsShown + 44, 1);
			++_itemsShown;
		}
	}

	askForRedraw();
}

// Close inventory: clear items and reset icon
void Inventory::close() {
	_isOpened = false;

	// Fallback to unselected state
	drawItem((CursorStyle)getProgress().portrait, 0, 0);

	// Erase rectangle for inventory items shown
	_engine->getGraphicsManager()->clear(GraphicsManager::kBackgroundInventory, Common::Rect(0, 44, 32, (int16)(44 + 40 * _itemsShown)));

	_itemsShown = 0;

	askForRedraw();
}

void Inventory::drawHighlight(uint32 currentIndex, bool reset) {
	uint32 index = getItemIndex(currentIndex);

	if (index) {
		drawItem(_entries[index].cursor, 0, 40 * currentIndex + 4, reset ? 1 : -1);
		_highlightedItemIndex = reset ? 0 : currentIndex;
		askForRedraw();
	}
}

uint32 Inventory::getItemIndex(uint32 currentIndex) const {
	uint32 count = 0;

	for (uint32 i = 1; i < ARRAYSIZE(_entries); i++) {
		if (!_entries[i].isPresent)
			continue;

		if (!_entries[i].manualSelect)
			continue;

		if (count < 11) {
			++count;

			if (count == currentIndex)
				return i;
		}
	}

	return 0;
}

} // End of namespace LastExpress
