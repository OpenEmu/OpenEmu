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

#ifndef WINTERMUTE_ADOBJECT_H
#define WINTERMUTE_ADOBJECT_H

#include "engines/wintermute/ad/ad_types.h"
#include "engines/wintermute/base/base_object.h"

namespace Wintermute {

class AdWaypointGroup;
class AdRegion;
class AdSentence;
class BaseFont;
class BaseRegion;
class AdInventory;
class PartEmitter;

#define MAX_NUM_REGIONS 10

class AdObject : public BaseObject {
public:
	PartEmitter *_partEmitter;
	virtual PartEmitter *createParticleEmitter(bool followParent = false, int offsetX = 0, int offsetY = 0);
	virtual bool updatePartEmitter();
	bool _partFollowParent;
	int _partOffsetX;
	int _partOffsetY;

	bool invalidateCurrRegions();
	bool _subtitlesModRelative;
	bool _subtitlesModXCenter;
	int _subtitlesModX;
	int _subtitlesModY;
	int _subtitlesWidth;
	AdRegion *_stickRegion;
	bool _sceneIndependent;
	bool _ignoreItems;
	bool updateBlockRegion();
	bool _forcedTalkAnimUsed;
	char *_forcedTalkAnimName;
	virtual bool getExtendedFlag(const char *flagName);
	virtual bool resetSoundPan();
	virtual bool updateSounds();
	bool reset();
	DECLARE_PERSISTENT(AdObject, BaseObject)
	virtual void talk(const char *text, const char *sound = NULL, uint32 duration = 0, const char *stances = NULL, TTextAlign align = TAL_CENTER);
	virtual int getHeight();
	AdSentence *_sentence;
	bool setFont(const char *filename);
	virtual bool update();
	virtual bool display();
	bool _drawn;
	bool _active;
	virtual bool playAnim(const char *filename);
	BaseSprite *_animSprite;
	BaseSprite *_currentSprite;
	TObjectState _state;
	TObjectState _nextState;
	TObjectType _type;
	AdObject(BaseGame *inGame);
	virtual ~AdObject();
	BaseFont *_font;
	BaseSprite *_tempSprite2;
	BaseRegion *_blockRegion;
	AdWaypointGroup *_wptGroup;
	BaseRegion *_currentBlockRegion;
	AdWaypointGroup *_currentWptGroup;
	AdInventory *getInventory();

	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent);

	virtual bool afterMove();
	AdRegion *_currentRegions[MAX_NUM_REGIONS];

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();

	BaseArray<AdObject *> _attachmentsPre;
	BaseArray<AdObject *> _attachmentsPost;

	bool updateSpriteAttachments();
	bool displaySpriteAttachments(bool preDisplay);
	AdObject *_registerAlias;
private:
	bool displaySpriteAttachment(AdObject *attachment);
	AdInventory *_inventory;

protected:
	bool getScale(float *scaleX, float *scaleY);
};

} // end of namespace Wintermute

#endif
