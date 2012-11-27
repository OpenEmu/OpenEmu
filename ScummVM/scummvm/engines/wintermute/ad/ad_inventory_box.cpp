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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_inventory_box.h"
#include "engines/wintermute/ad/ad_inventory.h"
#include "engines/wintermute/ad/ad_item.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/ui/ui_button.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"
#include "common/rect.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdInventoryBox, false)

//////////////////////////////////////////////////////////////////////////
AdInventoryBox::AdInventoryBox(BaseGame *inGame) : BaseObject(inGame) {
	_itemsArea.setEmpty();
	_scrollOffset = 0;
	_spacing = 0;
	_itemWidth = _itemHeight = 50;
	_scrollBy = 1;

	_window = NULL;
	_closeButton = NULL;

	_hideSelected = false;

	_visible = false;
	_exclusive = false;
}


//////////////////////////////////////////////////////////////////////////
AdInventoryBox::~AdInventoryBox() {
	_gameRef->unregisterObject(_window);
	_window = NULL;

	delete _closeButton;
	_closeButton = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdInventoryBox::listen(BaseScriptHolder *param1, uint32 param2) {
	UIObject *obj = (UIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->getName(), "close") == 0) {
			_visible = false;
		} else if (scumm_stricmp(obj->getName(), "prev") == 0) {
			_scrollOffset -= _scrollBy;
			_scrollOffset = MAX(_scrollOffset, 0);
		} else if (scumm_stricmp(obj->getName(), "next") == 0) {
			_scrollOffset += _scrollBy;
		} else {
			return BaseObject::listen(param1, param2);
		}
		break;
	default:
		error("AdInventoryBox::Listen - Unhandled enum");
		break;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdInventoryBox::display() {
	AdGame *adGame = (AdGame *)_gameRef;

	if (!_visible) {
		return STATUS_OK;
	}

	int itemsX, itemsY;
	itemsX = (int)floor((float)((_itemsArea.right - _itemsArea.left + _spacing) / (_itemWidth + _spacing)));
	itemsY = (int)floor((float)((_itemsArea.bottom - _itemsArea.top + _spacing) / (_itemHeight + _spacing)));

	if (_window) {
		_window->enableWidget("prev", _scrollOffset > 0);
		_window->enableWidget("next", _scrollOffset + itemsX * itemsY < (int32)adGame->_inventoryOwner->getInventory()->_takenItems.size());
	}


	if (_closeButton) {
		_closeButton->_posX = _closeButton->_posY = 0;
		_closeButton->_width = _gameRef->_renderer->_width;
		_closeButton->_height = _gameRef->_renderer->_height;

		_closeButton->display();
	}


	// display window
	Rect32 rect = _itemsArea;
	if (_window) {
		rect.offsetRect(_window->_posX, _window->_posY);
		_window->display();
	}

	// display items
	if (_window && _window->_alphaColor != 0) {
		_gameRef->_renderer->_forceAlphaColor = _window->_alphaColor;
	}
	int yyy = rect.top;
	for (int j = 0; j < itemsY; j++) {
		int xxx = rect.left;
		for (int i = 0; i < itemsX; i++) {
			int itemIndex = _scrollOffset + j * itemsX + i;
			if (itemIndex >= 0 && itemIndex < (int32)adGame->_inventoryOwner->getInventory()->_takenItems.size()) {
				AdItem *item = adGame->_inventoryOwner->getInventory()->_takenItems[itemIndex];
				if (item != ((AdGame *)_gameRef)->_selectedItem || !_hideSelected) {
					item->update();
					item->display(xxx, yyy);
				}
			}

			xxx += (_itemWidth + _spacing);
		}
		yyy += (_itemHeight + _spacing);
	}
	if (_window && _window->_alphaColor != 0) {
		_gameRef->_renderer->_forceAlphaColor = 0;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdInventoryBox::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "AdInventoryBox::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing INVENTORY_BOX file '%s'", filename);
	}


	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(INVENTORY_BOX)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(WINDOW)
TOKEN_DEF(EXCLUSIVE)
TOKEN_DEF(ALWAYS_VISIBLE)
TOKEN_DEF(AREA)
TOKEN_DEF(SPACING)
TOKEN_DEF(ITEM_WIDTH)
TOKEN_DEF(ITEM_HEIGHT)
TOKEN_DEF(SCROLL_BY)
TOKEN_DEF(NAME)
TOKEN_DEF(CAPTION)
TOKEN_DEF(HIDE_SELECTED)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdInventoryBox::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(INVENTORY_BOX)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(WINDOW)
	TOKEN_TABLE(EXCLUSIVE)
	TOKEN_TABLE(ALWAYS_VISIBLE)
	TOKEN_TABLE(AREA)
	TOKEN_TABLE(SPACING)
	TOKEN_TABLE(ITEM_WIDTH)
	TOKEN_TABLE(ITEM_HEIGHT)
	TOKEN_TABLE(SCROLL_BY)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(HIDE_SELECTED)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	BaseParser parser;
	bool alwaysVisible = false;

	_exclusive = false;
	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_INVENTORY_BOX) {
			_gameRef->LOG(0, "'INVENTORY_BOX' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while (cmd > 0 && (cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_WINDOW:
			delete _window;
			_window = new UIWindow(_gameRef);
			if (!_window || DID_FAIL(_window->loadBuffer(params, false))) {
				delete _window;
				_window = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				_gameRef->registerObject(_window);
			}
			break;

		case TOKEN_AREA:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &_itemsArea.left, &_itemsArea.top, &_itemsArea.right, &_itemsArea.bottom);
			break;

		case TOKEN_EXCLUSIVE:
			parser.scanStr((char *)params, "%b", &_exclusive);
			break;

		case TOKEN_HIDE_SELECTED:
			parser.scanStr((char *)params, "%b", &_hideSelected);
			break;

		case TOKEN_ALWAYS_VISIBLE:
			parser.scanStr((char *)params, "%b", &alwaysVisible);
			break;

		case TOKEN_SPACING:
			parser.scanStr((char *)params, "%d", &_spacing);
			break;

		case TOKEN_ITEM_WIDTH:
			parser.scanStr((char *)params, "%d", &_itemWidth);
			break;

		case TOKEN_ITEM_HEIGHT:
			parser.scanStr((char *)params, "%d", &_itemHeight);
			break;

		case TOKEN_SCROLL_BY:
			parser.scanStr((char *)params, "%d", &_scrollBy);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in INVENTORY_BOX definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading INVENTORY_BOX definition");
		return STATUS_FAILED;
	}

	if (_exclusive) {
		delete _closeButton;
		_closeButton = new UIButton(_gameRef);
		if (_closeButton) {
			_closeButton->setName("close");
			_closeButton->setListener(this, _closeButton, 0);
			_closeButton->_parent = _window;
		}
	}

	_visible = alwaysVisible;

	if (_window) {
		for (uint32 i = 0; i < _window->_widgets.size(); i++) {
			if (!_window->_widgets[i]->_listenerObject) {
				_window->_widgets[i]->setListener(this, _window->_widgets[i], 0);
			}
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdInventoryBox::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "INVENTORY_BOX\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	buffer->putTextIndent(indent + 2, "AREA { %d, %d, %d, %d }\n", _itemsArea.left, _itemsArea.top, _itemsArea.right, _itemsArea.bottom);

	buffer->putTextIndent(indent + 2, "EXCLUSIVE=%s\n", _exclusive ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "HIDE_SELECTED=%s\n", _hideSelected ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "ALWAYS_VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "SPACING=%d\n", _spacing);
	buffer->putTextIndent(indent + 2, "ITEM_WIDTH=%d\n", _itemWidth);
	buffer->putTextIndent(indent + 2, "ITEM_HEIGHT=%d\n", _itemHeight);
	buffer->putTextIndent(indent + 2, "SCROLL_BY=%d\n", _scrollBy);

	buffer->putTextIndent(indent + 2, "\n");

	// window
	if (_window) {
		_window->saveAsText(buffer, indent + 2);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdInventoryBox::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_closeButton));
	persistMgr->transfer(TMEMBER(_hideSelected));
	persistMgr->transfer(TMEMBER(_itemHeight));
	persistMgr->transfer(TMEMBER(_itemsArea));
	persistMgr->transfer(TMEMBER(_itemWidth));
	persistMgr->transfer(TMEMBER(_scrollBy));
	persistMgr->transfer(TMEMBER(_scrollOffset));
	persistMgr->transfer(TMEMBER(_spacing));
	persistMgr->transfer(TMEMBER(_visible));
	persistMgr->transfer(TMEMBER(_window));
	persistMgr->transfer(TMEMBER(_exclusive));

	return STATUS_OK;
}

} // end of namespace Wintermute
