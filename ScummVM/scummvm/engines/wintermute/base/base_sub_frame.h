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

#ifndef WINTERMUTE_BASE_SUBFRAME_H
#define WINTERMUTE_BASE_SUBFRAME_H


#include "engines/wintermute/base/base.h"
#include "engines/wintermute/base/base_scriptable.h"

namespace Wintermute {
class BaseObject;
class BaseSurface;
class BaseSubFrame : public BaseScriptable {
public:
	bool _mirrorX;
	bool _mirrorY;
	bool _decoration;
	bool setSurface(const Common::String &filename, bool defaultCK = true, byte ckRed = 0, byte ckGreen = 0, byte ckBlue = 0, int lifeTime = -1, bool keepLoaded = false);
	bool setSurfaceSimple();
	DECLARE_PERSISTENT(BaseSubFrame, BaseScriptable)
	void setDefaultRect();
	uint32 _transparent;
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) { return saveAsText(buffer, indent, true); }
	bool saveAsText(BaseDynamicBuffer *buffer, int indent, bool complete);
	bool _editorSelected;
	BaseSubFrame(BaseGame *inGame);
	virtual ~BaseSubFrame();
	bool loadBuffer(byte *buffer, int lifeTime, bool keepLoaded);
	bool draw(int x, int y, BaseObject *registerOwner = NULL, float zoomX = 100, float zoomY = 100, bool precise = true, uint32 alpha = 0xFFFFFFFF, float rotate = 0.0f, TSpriteBlendMode blendMode = BLEND_NORMAL);
	bool getBoundingRect(Rect32 *rect, int x, int y, float scaleX = 100, float scaleY = 100);
	const char* getSurfaceFilename();

	int _hotspotX;
	int _hotspotY;
	uint32 _alpha;
	// These two setters and getters are rather usefull, as they allow _rect to be lazily defined
	// Thus we don't need to load the actual graphics before the rect is actually needed.
	Rect32 getRect();
	void setRect(Rect32 rect);
private:
	bool _wantsDefaultRect;
	Rect32 _rect;
	char *_surfaceFilename;
public:
	bool _cKDefault;
	byte _cKRed;
	byte _cKGreen;
	byte _cKBlue;
	int _lifeTime;
	bool _keepLoaded;

	bool _2DOnly;
	bool _3DOnly;

	BaseSurface *_surface;

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();

};

} // end of namespace Wintermute

#endif
