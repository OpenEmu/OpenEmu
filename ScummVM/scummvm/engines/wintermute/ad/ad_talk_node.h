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

#ifndef WINTERMUTE_ADTALKNODE_H
#define WINTERMUTE_ADTALKNODE_H

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/base/base.h"

namespace Wintermute {
class AdSpriteSet;
class BaseSprite;
class AdTalkNode : public BaseClass {
public:
	char *_spriteSetFilename;
	AdSpriteSet *_spriteSet;
	BaseSprite *getSprite(TDirection dir);
	bool isInTimeInterval(uint32 time, TDirection dir);
	bool loadSprite();
	DECLARE_PERSISTENT(AdTalkNode, BaseClass)

	AdTalkNode(BaseGame *inGame);
	virtual ~AdTalkNode();
	bool loadBuffer(byte *buffer, bool complete = true);
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent = 0);
	char *_spriteFilename;
	BaseSprite *_sprite;
	uint32 _startTime;
	uint32 _endTime;
	bool _playToEnd;
	bool _preCache;
	char *_comment;

};

} // end of namespace Wintermute

#endif
