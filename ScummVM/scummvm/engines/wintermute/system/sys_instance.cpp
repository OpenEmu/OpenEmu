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

#include "engines/wintermute/system/sys_instance.h"
#include "engines/wintermute/system/sys_class_registry.h"
#include "engines/wintermute/system/sys_class.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
SystemInstance::SystemInstance(void *instance, int id, SystemClass *sysClass) {
	_instance = instance;
	_id = id;
	_savedID = -1;
	_class = sysClass;

	_used = false;
}

//////////////////////////////////////////////////////////////////////////
SystemInstance::~SystemInstance() {
}

} // end of namespace Wintermute
