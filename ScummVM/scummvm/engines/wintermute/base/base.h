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

#ifndef WINTERMUTE_BASE_BASE_H
#define WINTERMUTE_BASE_BASE_H

#include "engines/wintermute/wintypes.h"
#include "engines/wintermute/dctypes.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Wintermute {

class BaseGame;
class BaseDynamicBuffer;

class BaseClass {
public:
	bool _persistable;
	bool setEditorProp(const Common::String &propName, const Common::String &propValue);
	Common::String getEditorProp(const Common::String &propName, const Common::String &initVal = NULL);
	BaseClass(TDynamicConstructor, TDynamicConstructor) {}
	bool parseEditorProperty(byte *buffer, bool complete = true);
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent = 0);
	BaseClass();
	BaseClass(BaseGame *GameOwner);
	virtual ~BaseClass();
	BaseGame *_gameRef;
protected:
	Common::HashMap<Common::String, Common::String> _editorProps;
	Common::HashMap<Common::String, Common::String>::iterator _editorPropsIter;
};

} // end of namespace Wintermute

#endif
