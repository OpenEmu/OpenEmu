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

#ifndef WINTERMUTE_PARTFORCE_H
#define WINTERMUTE_PARTFORCE_H


#include "engines/wintermute/base/base.h"
#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/math/vector2.h"

namespace Wintermute {

class PartForce : public BaseNamedObject {
public:
	enum TForceType {
	    FORCE_POINT, FORCE_GLOBAL
	};

	PartForce(BaseGame *inGame);
	virtual ~PartForce(void);

	Vector2 _pos;
	Vector2 _direction;
	TForceType _type;

	bool persist(BasePersistenceManager *PersistMgr);
};

} // end of namespace Wintermute

#endif
