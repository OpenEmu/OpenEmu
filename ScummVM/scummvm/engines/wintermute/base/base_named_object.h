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

#ifndef WINTERMUTE_BASE_NAMED_OBJECT_H
#define WINTERMUTE_BASE_NAMED_OBJECT_H


#include "engines/wintermute/base/base.h"

namespace Wintermute {

class BaseNamedObject : public BaseClass {
	char *_name;
public:
	BaseNamedObject(BaseGame *inGame);
	BaseNamedObject();
	virtual ~BaseNamedObject(void);
	BaseNamedObject(TDynamicConstructor, TDynamicConstructor);

	const char *getName() { return _name; }
	void setName(const char *name);
};

} // end of namespace Wintermute

#endif
