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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/ui/ui_object.h"
#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/font/base_font_storage.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(UIObject, false)

//////////////////////////////////////////////////////////////////////////
UIObject::UIObject(BaseGame *inGame) : BaseObject(inGame) {
	_back = NULL;
	_image = NULL;
	_font = NULL;
	_text = NULL;
	_sharedFonts = _sharedImages = false;

	_width = _height = 0;

	_listenerObject = NULL;
	_listenerParamObject = NULL;
	_listenerParamDWORD = 0;

	_disable = false;
	_visible = true;

	_type = UI_UNKNOWN;
	_parent = NULL;

	_parentNotify = false;

	_focusedWidget = NULL;

	_canFocus = false;
	_nonIntMouseEvents = true;
}


//////////////////////////////////////////////////////////////////////////
UIObject::~UIObject() {
	if (!_gameRef->_loadInProgress) {
		SystemClassRegistry::getInstance()->enumInstances(BaseGame::invalidateValues, "ScValue", (void *)this);
	}

	if (_back) {
		delete _back;
	}
	if (_font && !_sharedFonts) {
		_gameRef->_fontStorage->removeFont(_font);
	}

	if (_image && !_sharedImages) {
		delete _image;
	}

	if (_text) {
		delete[] _text;
	}

	_focusedWidget = NULL; // ref only
}


//////////////////////////////////////////////////////////////////////////
void UIObject::setText(const char *text) {
	if (_text) {
		delete[] _text;
	}
	_text = new char [strlen(text) + 1];
	if (_text) {
		strcpy(_text, text);
		for (uint32 i = 0; i < strlen(_text); i++) {
			if (_text[i] == '|') {
				_text[i] = '\n';
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::display(int offsetX, int offsetY) {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void UIObject::setListener(BaseScriptHolder *object, BaseScriptHolder *listenerObject, uint32 listenerParam) {
	_listenerObject = object;
	_listenerParamObject = listenerObject;
	_listenerParamDWORD = listenerParam;
}


//////////////////////////////////////////////////////////////////////////
void UIObject::correctSize() {
	Rect32 rect;

	if (_width <= 0) {
		if (_image) {
			_image->getBoundingRect(&rect, 0, 0);
			_width = rect.right - rect.left;
		} else {
			_width = 100;
		}
	}

	if (_height <= 0) {
		if (_image) {
			_image->getBoundingRect(&rect, 0, 0);
			_height = rect.bottom - rect.top;
		}
	}

	if (_back) {
		_back->correctSize(&_width, &_height);
	}
}



//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool UIObject::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetFont") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		if (_font) {
			_gameRef->_fontStorage->removeFont(_font);
		}
		if (val->isNULL()) {
			_font = NULL;
			stack->pushBool(true);
		} else {
			_font = _gameRef->_fontStorage->addFont(val->getString());
			stack->pushBool(_font != NULL);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetImage") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		/* const char *filename = */ val->getString();

		delete _image;
		_image = NULL;
		if (val->isNULL()) {
			stack->pushBool(true);
			return STATUS_OK;
		}

		_image = new BaseSprite(_gameRef);
		if (!_image || DID_FAIL(_image->loadFile(val->getString()))) {
			delete _image;
			_image = NULL;
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetImage") == 0) {
		stack->correctParams(0);
		if (!_image || !_image->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_image->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetImageObject") == 0) {
		stack->correctParams(0);
		if (!_image) {
			stack->pushNULL();
		} else {
			stack->pushNative(_image, true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focus
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Focus") == 0) {
		stack->correctParams(0);
		focus();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveAfter / MoveBefore
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveAfter") == 0 || strcmp(name, "MoveBefore") == 0) {
		stack->correctParams(1);

		if (_parent && _parent->_type == UI_WINDOW) {
			UIWindow *win = (UIWindow *)_parent;

			uint32 i;
			bool found = false;
			ScValue *val = stack->pop();
			// find directly
			if (val->isNative()) {
				UIObject *widget = (UIObject *)val->getNative();
				for (i = 0; i < win->_widgets.size(); i++) {
					if (win->_widgets[i] == widget) {
						found = true;
						break;
					}
				}
			}
			// find by name
			else {
				const char *findName = val->getString();
				for (i = 0; i < win->_widgets.size(); i++) {
					if (scumm_stricmp(win->_widgets[i]->getName(), findName) == 0) {
						found = true;
						break;
					}
				}
			}

			if (found) {
				bool done = false;
				for (uint32 j = 0; j < win->_widgets.size(); j++) {
					if (win->_widgets[j] == this) {
						if (strcmp(name, "MoveAfter") == 0) {
							i++;
						}
						if (j >= i) {
							j++;
						}

						win->_widgets.insert_at(i, this);
						win->_widgets.remove_at(j);

						done = true;
						stack->pushBool(true);
						break;
					}
				}
				if (!done) {
					stack->pushBool(false);
				}
			} else {
				stack->pushBool(false);
			}

		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveToBottom
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveToBottom") == 0) {
		stack->correctParams(0);

		if (_parent && _parent->_type == UI_WINDOW) {
			UIWindow *win = (UIWindow *)_parent;
			for (uint32 i = 0; i < win->_widgets.size(); i++) {
				if (win->_widgets[i] == this) {
					win->_widgets.remove_at(i);
					win->_widgets.insert_at(0, this);
					break;
				}
			}
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveToTop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveToTop") == 0) {
		stack->correctParams(0);

		if (_parent && _parent->_type == UI_WINDOW) {
			UIWindow *win = (UIWindow *)_parent;
			for (uint32 i = 0; i < win->_widgets.size(); i++) {
				if (win->_widgets[i] == this) {
					win->_widgets.remove_at(i);
					win->_widgets.add(this);
					break;
				}
			}
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	} else {
		return BaseObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *UIObject::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("ui_object");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Name") {
		_scValue->setString(getName());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Parent (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Parent") {
		_scValue->setNative(_parent, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParentNotify
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ParentNotify") {
		_scValue->setBool(_parentNotify);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Width") {
		_scValue->setInt(_width);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Height") {
		_scValue->setInt(_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Visible
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Visible") {
		_scValue->setBool(_visible);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Disabled
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Disabled") {
		_scValue->setBool(_disable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Text") {
		_scValue->setString(_text);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NextSibling (RO) / PrevSibling (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NextSibling" || name == "PrevSibling") {
		_scValue->setNULL();
		if (_parent && _parent->_type == UI_WINDOW) {
			UIWindow *win = (UIWindow *)_parent;
			for (uint32 i = 0; i < win->_widgets.size(); i++) {
				if (win->_widgets[i] == this) {
					if (name == "NextSibling") {
						if (i < win->_widgets.size() - 1) {
							_scValue->setNative(win->_widgets[i + 1], true);
						}
					} else {
						if (i > 0) {
							_scValue->setNative(win->_widgets[i - 1], true);
						}
					}
					break;
				}
			}
		}
		return _scValue;
	} else {
		return BaseObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParentNotify
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ParentNotify") == 0) {
		_parentNotify = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_width = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_height = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Visible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Visible") == 0) {
		_visible = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Disabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Disabled") == 0) {
		_disable = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Text") == 0) {
		setText(value->getString());
		return STATUS_OK;
	} else {
		return BaseObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *UIObject::scToString() {
	return "[ui_object]";
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::isFocused() {
	if (!_gameRef->_focusedWindow) {
		return false;
	}
	if (_gameRef->_focusedWindow == this) {
		return true;
	}

	UIObject *obj = _gameRef->_focusedWindow;
	while (obj) {
		if (obj == this) {
			return true;
		} else {
			obj = obj->_focusedWidget;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::handleMouse(TMouseEvent event, TMouseButton button) {
	// handle focus change
	if (event == MOUSE_CLICK && button == MOUSE_BUTTON_LEFT) {
		focus();
	}
	return BaseObject::handleMouse(event, button);
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::focus() {
	UIObject *obj = this;
	bool disabled = false;
	while (obj) {
		if (obj->_disable && obj->_type == UI_WINDOW) {
			disabled = true;
			break;
		}
		obj = obj->_parent;
	}
	if (!disabled) {
		obj = this;
		while (obj) {
			if (obj->_parent) {
				if (!obj->_disable && obj->_canFocus) {
					obj->_parent->_focusedWidget = obj;
				}
			} else {
				if (obj->_type == UI_WINDOW) {
					_gameRef->focusWindow((UIWindow *)obj);
				}
			}

			obj = obj->_parent;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::getTotalOffset(int *offsetX, int *offsetY) {
	int offX = 0, offY = 0;

	UIObject *obj = _parent;
	while (obj) {
		offX += obj->_posX;
		offY += obj->_posY;

		obj = obj->_parent;
	}
	if (offsetX) {
		*offsetX = offX;
	}
	if (offsetY) {
		*offsetY = offY;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_back));
	persistMgr->transfer(TMEMBER(_canFocus));
	persistMgr->transfer(TMEMBER(_disable));
	persistMgr->transfer(TMEMBER(_focusedWidget));
	persistMgr->transfer(TMEMBER(_font));
	persistMgr->transfer(TMEMBER(_height));
	persistMgr->transfer(TMEMBER(_image));
	persistMgr->transfer(TMEMBER(_listenerObject));
	persistMgr->transfer(TMEMBER(_listenerParamObject));
	persistMgr->transfer(TMEMBER(_listenerParamDWORD));
	persistMgr->transfer(TMEMBER(_parent));
	persistMgr->transfer(TMEMBER(_parentNotify));
	persistMgr->transfer(TMEMBER(_sharedFonts));
	persistMgr->transfer(TMEMBER(_sharedImages));
	persistMgr->transfer(TMEMBER(_text));
	persistMgr->transfer(TMEMBER_INT(_type));
	persistMgr->transfer(TMEMBER(_visible));
	persistMgr->transfer(TMEMBER(_width));

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UIObject::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	return STATUS_FAILED;
}

} // end of namespace Wintermute
