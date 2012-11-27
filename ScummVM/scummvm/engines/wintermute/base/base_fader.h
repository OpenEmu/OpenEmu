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

#ifndef WINTERMUTE_BASE_FADER_H
#define WINTERMUTE_BASE_FADER_H


#include "engines/wintermute/base/base_object.h"

namespace Wintermute {

class BaseFader : public BaseObject {
public:
	uint32 getCurrentColor();
	bool fadeOut(uint32 targetColor, uint32 duration, bool system = false);
	bool fadeIn(uint32 sourceColor, uint32 duration, bool system = false);
	bool deactivate();
	bool display();
	bool update();
	DECLARE_PERSISTENT(BaseFader, BaseObject)
	BaseFader(BaseGame *inGame);
	virtual ~BaseFader();
private:
	bool _system;
	bool _active;
	byte _red;
	byte _green;
	byte _blue;
	byte _currentAlpha;
	byte _targetAlpha;
	byte _sourceAlpha;
	uint32 _duration;
	uint32 _startTime;
};

} // end of namespace Wintermute

#endif
