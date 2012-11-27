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

#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/ui/ui_text.h"
#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/base_string_table.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/platform_osystem.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(UIText, false)

//////////////////////////////////////////////////////////////////////////
UIText::UIText(BaseGame *inGame) : UIObject(inGame) {
	_textAlign = TAL_LEFT;
	_verticalAlign = VAL_CENTER;
	_type = UI_STATIC;
	_canFocus = false;
}


//////////////////////////////////////////////////////////////////////////
UIText::~UIText() {

}


//////////////////////////////////////////////////////////////////////////
bool UIText::display(int offsetX, int offsetY) {
	if (!_visible) {
		return STATUS_OK;
	}


	BaseFont *font = _font;
	if (!font) {
		font = _gameRef->_systemFont;
	}

	if (_back) {
		_back->display(offsetX + _posX, offsetY + _posY, _width, _height);
	}
	if (_image) {
		_image->draw(offsetX + _posX, offsetY + _posY, NULL);
	}

	if (font && _text) {
		int textOffset;
		switch (_verticalAlign) {
		case VAL_TOP:
			textOffset = 0;
			break;
		case VAL_BOTTOM:
			textOffset = _height - font->getTextHeight((byte *)_text, _width);
			break;
		default:
			textOffset = (_height - font->getTextHeight((byte *)_text, _width)) / 2;
		}
		font->drawText((byte *)_text, offsetX + _posX, offsetY + _posY + textOffset, _width, _textAlign, _height);
	}

	//_gameRef->_renderer->_rectList.add(new BaseActiveRect(_gameRef,  this, NULL, OffsetX + _posX, OffsetY + _posY, _width, _height, 100, 100, false));

	return STATUS_OK;
}



//////////////////////////////////////////////////////////////////////////
bool UIText::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "UIText::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing STATIC file '%s'", filename);
	}

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(STATIC)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(BACK)
TOKEN_DEF(IMAGE)
TOKEN_DEF(FONT)
TOKEN_DEF(TEXT_ALIGN)
TOKEN_DEF(VERTICAL_ALIGN)
TOKEN_DEF(TEXT)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(CURSOR)
TOKEN_DEF(NAME)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PARENT_NOTIFY)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool UIText::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(STATIC)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(BACK)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TEXT_ALIGN)
	TOKEN_TABLE(VERTICAL_ALIGN)
	TOKEN_TABLE(TEXT)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PARENT_NOTIFY)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_STATIC) {
			_gameRef->LOG(0, "'STATIC' keyword expected.");
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

		case TOKEN_BACK:
			delete _back;
			_back = new UITiledImage(_gameRef);
			if (!_back || DID_FAIL(_back->loadFile((char *)params))) {
				delete _back;
				_back = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			delete _image;
			_image = new BaseSprite(_gameRef);
			if (!_image || DID_FAIL(_image->loadFile((char *)params))) {
				delete _image;
				_image = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (_font) {
				_gameRef->_fontStorage->removeFont(_font);
			}
			_font = _gameRef->_fontStorage->addFont((char *)params);
			if (!_font) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_TEXT:
			setText((char *)params);
			_gameRef->_stringTable->expand(&_text);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) {
				_textAlign = TAL_LEFT;
			} else if (scumm_stricmp((char *)params, "right") == 0) {
				_textAlign = TAL_RIGHT;
			} else {
				_textAlign = TAL_CENTER;
			}
			break;

		case TOKEN_VERTICAL_ALIGN:
			if (scumm_stricmp((char *)params, "top") == 0) {
				_verticalAlign = VAL_TOP;
			} else if (scumm_stricmp((char *)params, "bottom") == 0) {
				_verticalAlign = VAL_BOTTOM;
			} else {
				_verticalAlign = VAL_CENTER;
			}
			break;

		case TOKEN_X:
			parser.scanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.scanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_WIDTH:
			parser.scanStr((char *)params, "%d", &_width);
			break;

		case TOKEN_HEIGHT:
			parser.scanStr((char *)params, "%d", &_height);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new BaseSprite(_gameRef);
			if (!_cursor || DID_FAIL(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.scanStr((char *)params, "%b", &_parentNotify);
			break;

		case TOKEN_DISABLED:
			parser.scanStr((char *)params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.scanStr((char *)params, "%b", &_visible);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in STATIC definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading STATIC definition");
		return STATUS_FAILED;
	}

	correctSize();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UIText::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "STATIC\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	buffer->putTextIndent(indent + 2, "\n");

	if (_back && _back->getFilename()) {
		buffer->putTextIndent(indent + 2, "BACK=\"%s\"\n", _back->getFilename());
	}

	if (_image && _image->getFilename()) {
		buffer->putTextIndent(indent + 2, "IMAGE=\"%s\"\n", _image->getFilename());
	}

	if (_font && _font->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->getFilename());
	}

	if (_cursor && _cursor->getFilename()) {
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->getFilename());
	}

	if (_text) {
		buffer->putTextIndent(indent + 2, "TEXT=\"%s\"\n", _text);
	}

	switch (_textAlign) {
	case TAL_LEFT:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "center");
		break;
	default:
		error("UIText::SaveAsText - Unhandled enum");
		break;
	}

	switch (_verticalAlign) {
	case VAL_TOP:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "top");
		break;
	case VAL_BOTTOM:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "bottom");
		break;
	case VAL_CENTER:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "center");
		break;
	default:
		error("UIText::SaveAsText - Unhandled enum value: NUM_VERTICAL_ALIGN");
	}

	buffer->putTextIndent(indent + 2, "\n");

	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);
	buffer->putTextIndent(indent + 2, "WIDTH=%d\n", _width);
	buffer->putTextIndent(indent + 2, "HEIGHT=%d\n", _height);

	buffer->putTextIndent(indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");

	buffer->putTextIndent(indent + 2, "\n");

	// scripts
	for (uint32 i = 0; i < _scripts.size(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool UIText::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SizeToFit
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SizeToFit") == 0) {
		stack->correctParams(0);
		sizeToFit();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HeightToFit
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HeightToFit") == 0) {
		stack->correctParams(0);
		if (_font && _text) {
			_height = _font->getTextHeight((byte *)_text, _width);
		}
		stack->pushNULL();
		return STATUS_OK;
	} else {
		return UIObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *UIText::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("static");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TextAlign") {
		_scValue->setInt(_textAlign);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// VerticalAlign
	//////////////////////////////////////////////////////////////////////////
	else if (name == "VerticalAlign") {
		_scValue->setInt(_verticalAlign);
		return _scValue;
	} else {
		return UIObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIText::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "TextAlign") == 0) {
		int i = value->getInt();
		if (i < 0 || i >= NUM_TEXT_ALIGN) {
			i = 0;
		}
		_textAlign = (TTextAlign)i;
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// VerticalAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VerticalAlign") == 0) {
		int i = value->getInt();
		if (i < 0 || i >= NUM_VERTICAL_ALIGN) {
			i = 0;
		}
		_verticalAlign = (TVerticalAlign)i;
		return STATUS_OK;
	} else {
		return UIObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *UIText::scToString() {
	return "[static]";
}



//////////////////////////////////////////////////////////////////////////
bool UIText::persist(BasePersistenceManager *persistMgr) {

	UIObject::persist(persistMgr);
	persistMgr->transfer(TMEMBER_INT(_textAlign));
	persistMgr->transfer(TMEMBER_INT(_verticalAlign));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIText::sizeToFit() {
	if (_font && _text) {
		_width = _font->getTextWidth((byte *)_text);
		_height = _font->getTextHeight((byte *)_text, _width);
	}
	return STATUS_OK;
}

} // end of namespace Wintermute
