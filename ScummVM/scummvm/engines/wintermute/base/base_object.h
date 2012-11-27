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

#ifndef WINTERMUTE_BASE_OBJECT_H
#define WINTERMUTE_BASE_OBJECT_H


#include "engines/wintermute/base/base_script_holder.h"
#include "engines/wintermute/persistent.h"
#include "common/events.h"

namespace Wintermute {

class BaseSprite;
class BaseSound;
class BaseSurface;
class BaseScriptHolder;
class ScValue;
class ScStack;
class ScScript;
class BaseObject : public BaseScriptHolder {
protected:
	bool _autoSoundPanning;
	uint32 _sFXStart;
	bool setSFXTime(uint32 time);
	bool setSFXVolume(int volume);
	bool resumeSFX();
	bool pauseSFX();
	bool stopSFX(bool deleteSound = true);
	bool playSFX(const char *filename, bool looping = false, bool playNow = true, const char *eventName = NULL, uint32 loopStart = 0);
	BaseSound *_sFX;
	TSFXType _sFXType;
	float _sFXParam1;
	float _sFXParam2;
	float _sFXParam3;
	float _sFXParam4;
	float _relativeRotate;
	bool _rotateValid;
	float _rotate;
	void setSoundEvent(const char *eventName);
	bool _rotatable;
	float _scaleX;
	float _scaleY;
	float _relativeScale;
	bool _editorSelected;
	bool _editorAlwaysRegister;
	bool _ready;
	Rect32 _rect;
	bool _rectSet;
	int _iD;
	char *_soundEvent;
public:
	TSpriteBlendMode _blendMode;
	virtual bool afterMove();
	float _scale;
	uint32 _alphaColor;
	virtual bool isReady();
	virtual bool getExtendedFlag(const char *flagName);
	virtual bool resetSoundPan();
	virtual bool updateSounds();
	bool updateOneSound(BaseSound *sound);
	int _sFXVolume;

	virtual bool handleMouseWheel(int delta);
	virtual bool handleMouse(TMouseEvent event, TMouseButton button);
	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	virtual int getHeight();
	bool setCursor(const char *filename);
	bool setActiveCursor(const char *filename);
	bool cleanup();
	const char *getCaption(int caseVal = 1);
	void setCaption(const char *caption, int caseVal = 1);

	bool _editorOnly;
	bool _is3D;

	DECLARE_PERSISTENT(BaseObject, BaseScriptHolder)
	virtual bool showCursor();
	BaseSprite *_cursor;
	bool _sharedCursors;
	BaseSprite *_activeCursor;
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent);
	virtual bool listen(BaseScriptHolder *param1, uint32 param2);

	bool _movable;
	bool _zoomable;
	bool _shadowable;
	int _posY;
	int _posX;
	bool _registrable;
	char *_caption[7];
	bool _saveState;

	BaseObject(BaseGame *inGame);
	virtual ~BaseObject();
	// base
	virtual bool update()  {
		return STATUS_FAILED;
	};
	virtual bool display() {
		return STATUS_FAILED;
	};
	virtual bool invalidateDeviceObjects()  {
		return STATUS_OK;
	};
	virtual bool restoreDeviceObjects()     {
		return STATUS_OK;
	};
	bool _nonIntMouseEvents;


public:
	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();
};

} // end of namespace Wintermute

#endif
