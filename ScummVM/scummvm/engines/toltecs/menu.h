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
 *
 */

#ifndef TOLTECS_MENU_H
#define TOLTECS_MENU_H

#include "common/array.h"
#include "common/str-array.h"

namespace Toltecs {

enum ItemID {
	kItemIdNone,
	// Main menu
	kItemIdSave,
	kItemIdLoad,
	kItemIdToggleText,
	kItemIdToggleVoices,
	kItemIdVolumesMenu,
	kItemIdPlay,
	kItemIdQuit,
	// Volumes menu
	kItemIdMasterUp,
	kItemIdVoicesUp,
	kItemIdMusicUp,
	kItemIdSoundFXUp,
	kItemIdBackgroundUp,
	kItemIdMasterDown,
	kItemIdVoicesDown,
	kItemIdMusicDown,
	kItemIdSoundFXDown,
	kItemIdBackgroundDown,
	kItemIdMaster,
	kItemIdVoices,
	kItemIdMusic,
	kItemIdSoundFX,
	kItemIdBackground,
	kItemIdDone,
	kItemIdCancel,
	// Save/load menu
	kItemIdSavegameUp,
	kItemIdSavegameDown,
	kItemIdSavegame1,
	kItemIdSavegame2,
	kItemIdSavegame3,
	kItemIdSavegame4,
	kItemIdSavegame5,
	kItemIdSavegame6,
	kItemIdSavegame7,
	// TODO
	kMenuIdDummy
};

class MenuSystem {

public:
	MenuSystem(ToltecsEngine *vm);
	~MenuSystem();

	int run(MenuID menuId);
	void update();
	void handleEvents();

protected:

	struct Item {
		Common::Rect rect;
		ItemID id;
		Common::String caption;
		byte defaultColor, activeColor;
		int x, y, w;
		uint fontNum;
	};

	struct SavegameItem {
		int _slotNum;
		Common::String _description;
		SavegameItem()
			: _slotNum(-1), _description("") {}
		SavegameItem(int slotNum, Common::String description)
			: _slotNum(slotNum), _description(description) {}
	};

	ToltecsEngine *_vm;
	Graphics::Surface *_background;

	bool _running;
	MenuID _currMenuID, _newMenuID;
	ItemID _currItemID;
	int _top;
	int _savegameListTopIndex;
	bool _editingDescription;
	ItemID _editingDescriptionID;
	Item *_editingDescriptionItem;
	bool _needRedraw;

	Common::Array<Item> _items;
	Common::Array<SavegameItem> _savegames;

	void addClickTextItem(ItemID id, int x, int y, int w, uint fontNum, const char *caption, byte defaultColor, byte activeColor);

	void drawItem(ItemID itemID, bool active);
	void handleMouseMove(int x, int y);
	void handleMouseClick(int x, int y);
	void handleKeyDown(const Common::KeyState& kbd);

	ItemID findItemAt(int x, int y);
	Item *getItem(ItemID id);
	void setItemCaption(Item *item, const char *caption);

	void initMenu(MenuID menuID);

	void enterItem(ItemID id);
	void leaveItem(ItemID id);
	void clickItem(ItemID id);

	void restoreRect(int x, int y, int w, int h);
	void shadeRect(int x, int y, int w, int h, byte color1, byte color2);
	void drawString(int16 x, int16 y, int w, uint fontNum, byte color, const char *text);

	SavegameItem *getSavegameItemByID(ItemID id);

	int loadSavegamesList();
	void setSavegameCaptions();
	void scrollSavegames(int delta);
	void clickSavegameItem(ItemID id);
	void setCfgText(bool value, bool active);
	void setCfgVoices(bool value, bool active);
	void drawVolumeBar(ItemID itemID);
	void changeVolumeBar(ItemID itemID, int delta);

};

} // End of namespace Toltecs

#endif /* TOLTECS_MENU_H */
