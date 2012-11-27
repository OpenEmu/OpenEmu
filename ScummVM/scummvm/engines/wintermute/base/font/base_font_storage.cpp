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

#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/base_game.h"
#include "common/str.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(BaseFontStorage, true)

//////////////////////////////////////////////////////////////////////////
BaseFontStorage::BaseFontStorage(BaseGame *inGame) : BaseClass(inGame) {
}

//////////////////////////////////////////////////////////////////////////
BaseFontStorage::~BaseFontStorage() {
	cleanup(true);
}

//////////////////////////////////////////////////////////////////////////
bool BaseFontStorage::cleanup(bool warn) {
	for (uint32 i = 0; i < _fonts.size(); i++) {
		if (warn) {
			_gameRef->LOG(0, "Removing orphan font '%s'", _fonts[i]->getFilename());
		}
		delete _fonts[i];
	}
	_fonts.clear();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFontStorage::initLoop() {
	for (uint32 i = 0; i < _fonts.size(); i++) {
		_fonts[i]->initLoop();
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
BaseFont *BaseFontStorage::addFont(const Common::String &filename) {
	if (!filename.size()) {
		return NULL;
	}

	for (uint32 i = 0; i < _fonts.size(); i++) {
		if (scumm_stricmp(_fonts[i]->getFilename(), filename.c_str()) == 0) {
			_fonts[i]->_refCount++;
			return _fonts[i];
		}
	}

	/*
	BaseFont* font = new BaseFont(_gameRef);
	if (!font) return NULL;

	if (DID_FAIL(font->loadFile(filename))) {
	    delete font;
	    return NULL;
	}
	else {
	    font->_refCount = 1;
	    _fonts.add(font);
	    return font;
	}
	*/
	BaseFont *font = BaseFont::createFromFile(_gameRef,  filename);
	if (font) {
		font->_refCount = 1;
		_fonts.add(font);
	}
	return font;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFontStorage::removeFont(BaseFont *font) {
	if (!font) {
		return STATUS_FAILED;
	}

	for (uint32 i = 0; i < _fonts.size(); i++) {
		if (_fonts[i] == font) {
			_fonts[i]->_refCount--;
			if (_fonts[i]->_refCount <= 0) {
				delete _fonts[i];
				_fonts.remove_at(i);
			}
			break;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFontStorage::persist(BasePersistenceManager *persistMgr) {

	if (!persistMgr->getIsSaving()) {
		cleanup(false);
	}

	persistMgr->transfer(TMEMBER(_gameRef));
	_fonts.persist(persistMgr);

	return STATUS_OK;
}

} // end of namespace Wintermute
