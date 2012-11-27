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

#include "engines/wintermute/ad/ad_scale_level.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdScaleLevel, false)

//////////////////////////////////////////////////////////////////////////
AdScaleLevel::AdScaleLevel(BaseGame *inGame) : BaseObject(inGame) {
	_posY = 0;
	_scale = 100;
}


//////////////////////////////////////////////////////////////////////////
AdScaleLevel::~AdScaleLevel() {

}


//////////////////////////////////////////////////////////////////////////
bool AdScaleLevel::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "AdScaleLevel::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing SCALE_LEVEL file '%s'", filename);
	}


	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(SCALE_LEVEL)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(Y)
TOKEN_DEF(SCALE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdScaleLevel::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(SCALE_LEVEL)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(SCALE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_SCALE_LEVEL) {
			_gameRef->LOG(0, "'SCALE_LEVEL' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_Y:
			parser.scanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_SCALE: {
			int i;
			parser.scanStr((char *)params, "%d", &i);
			_scale = (float)i;
		}
		break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in SCALE_LEVEL definition");
		return STATUS_FAILED;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScaleLevel::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "SCALE_LEVEL {\n");
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);
	buffer->putTextIndent(indent + 2, "SCALE=%d\n", (int)_scale);
	BaseClass::saveAsText(buffer, indent + 2);
	buffer->putTextIndent(indent, "}\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScaleLevel::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_scale));

	return STATUS_OK;
}

} // end of namespace Wintermute
