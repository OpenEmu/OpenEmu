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

#include "engines/wintermute/ad/ad_response_context.h"
#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdResponseContext, false)

//////////////////////////////////////////////////////////////////////////
AdResponseContext::AdResponseContext(BaseGame *inGame) : BaseClass(inGame) {
	_id = 0;
	_context = NULL;
}


//////////////////////////////////////////////////////////////////////////
AdResponseContext::~AdResponseContext() {
	delete[] _context;
	_context = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseContext::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transfer(TMEMBER(_gameRef));
	persistMgr->transfer(TMEMBER(_context));
	persistMgr->transfer(TMEMBER(_id));

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void AdResponseContext::setContext(const char *context) {
	delete[] _context;
	_context = NULL;
	if (context) {
		_context = new char [strlen(context) + 1];
		if (_context) {
			strcpy(_context, context);
		}
	}
}

} // end of namespace Wintermute
