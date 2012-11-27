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

#ifndef WINTERMUTE_ADACTOR_H
#define WINTERMUTE_ADACTOR_H


#include "engines/wintermute/dctypes.h"    // Added by ClassView
#include "engines/wintermute/ad/ad_types.h"    // Added by ClassView
#include "engines/wintermute/ad/ad_talk_holder.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/base/base_point.h" // Added by ClassView
#include "engines/wintermute/persistent.h"
#include "common/str.h"

namespace Wintermute {

class AdSpriteSet;
class AdPath;
class BaseSprite;
class AdActor : public AdTalkHolder {
public:
	TDirection angleToDirection(int angle);
	DECLARE_PERSISTENT(AdActor, AdTalkHolder)
	virtual int getHeight();
	BaseSprite *getTalkStance(const char *stance);
	virtual void goTo(int x, int y, TDirection afterWalkDir = DI_NONE);
	BasePoint *_targetPoint;
	virtual bool update();
	virtual bool display();
	virtual void turnTo(TDirection dir);
	AdActor(BaseGame *inGame/*=NULL*/);
	virtual ~AdActor();
	bool loadFile(const char *filename);
	bool loadBuffer(byte *buffer, bool complete = true);


private:
	TDirection _targetDir;
	TDirection _afterWalkDir;

	AdPath *_path;
	AdSpriteSet *_walkSprite;
	AdSpriteSet *_standSprite;
	AdSpriteSet *_turnLeftSprite;
	AdSpriteSet *_turnRightSprite;
	BaseArray<AdSpriteSet *> _talkSprites;
	BaseArray<AdSpriteSet *> _talkSpritesEx;
	TDirection _dir;
	// new anim system
	Common::String _talkAnimName;
	Common::String _idleAnimName;
	Common::String _walkAnimName;
	Common::String _turnLeftAnimName;
	Common::String _turnRightAnimName;
	BaseArray<AdSpriteSet *> _anims;
	virtual bool playAnim(const char *filename);
	AdSpriteSet *getAnimByName(const Common::String &animName);

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();

	bool setDefaultAnimNames();
	BaseSprite *getTalkStanceOld(const char *stance);
	bool mergeAnims(const char *animsFilename);
	BaseSprite *_animSprite2;

	void initLine(BasePoint startPt, BasePoint endPt);
	void getNextStep();
	void followPath();
	double _pFStepX;
	double _pFStepY;
	double _pFX;
	double _pFY;
	int _pFCount;
};

} // end of namespace Wintermute

#endif
