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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"
#include "sword25/script/luacallback.h"
#include "sword25/math/vertex.h"

#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/renderobject.h"
#include "sword25/gfx/bitmap.h"
#include "sword25/gfx/animation.h"
#include "sword25/gfx/panel.h"
#include "sword25/gfx/text.h"
#include "sword25/gfx/animationtemplate.h"
#include "sword25/gfx/animationtemplateregistry.h"

namespace Sword25 {

static bool animationDeleteCallback(uint Data);
static bool animationActionCallback(uint Data);
static bool animationLoopPointCallback(uint Data);

namespace {
class ActionCallback : public LuaCallback {
public:
	ActionCallback(lua_State *L) : LuaCallback(L) {}

	Common::String Action;

protected:
	virtual int PreFunctionInvokation(lua_State *L) {
		lua_pushstring(L, Action.c_str());
		return 1;
	}
};

static LuaCallback *loopPointCallbackPtr = 0;	// FIXME: should be turned into GraphicEngine member var
static ActionCallback *actionCallbackPtr = 0;	// FIXME: should be turned into GraphicEngine member var
}

// Die Strings werden als #defines definiert um Stringkomposition zur Compilezeit zu ermöglichen.
#define RENDEROBJECT_CLASS_NAME "Gfx.RenderObject"
#define BITMAP_CLASS_NAME "Gfx.Bitmap"
#define PANEL_CLASS_NAME "Gfx.Panel"
#define TEXT_CLASS_NAME "Gfx.Text"
#define ANIMATION_CLASS_NAME "Gfx.Animation"
#define ANIMATION_TEMPLATE_CLASS_NAME "Gfx.AnimationTemplate"
static const char *GFX_LIBRARY_NAME = "Gfx";

static void newUintUserData(lua_State *L, uint value) {
	void *userData = lua_newuserdata(L, sizeof(value));
	memcpy(userData, &value, sizeof(value));
}

static AnimationTemplate *checkAnimationTemplate(lua_State *L, int idx = 1) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.AnimationTemplate
	uint animationTemplateHandle = *reinterpret_cast<uint *>(LuaBindhelper::my_checkudata(L, idx, ANIMATION_TEMPLATE_CLASS_NAME));
	if (animationTemplateHandle != 0) {
		AnimationTemplate *animationTemplatePtr = AnimationTemplateRegistry::instance().resolveHandle(animationTemplateHandle);
		if (!animationTemplatePtr)
			luaL_error(L, "The animation template with the handle %d does no longer exist.", animationTemplateHandle);
		return animationTemplatePtr;
	} else {
		luaL_argcheck(L, 0, idx, "'" ANIMATION_TEMPLATE_CLASS_NAME "' expected");
		return 0;
	}
}


static int newAnimationTemplate(lua_State *L) {
	uint animationTemplateHandle = AnimationTemplate::create(luaL_checkstring(L, 1));
	AnimationTemplate *animationTemplatePtr = AnimationTemplateRegistry::instance().resolveHandle(animationTemplateHandle);
	if (animationTemplatePtr && animationTemplatePtr->isValid()) {
		newUintUserData(L, animationTemplateHandle);
		//luaL_getmetatable(L, ANIMATION_TEMPLATE_CLASS_NAME);
		LuaBindhelper::getMetatable(L, ANIMATION_TEMPLATE_CLASS_NAME);
		assert(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

static int at_addFrame(lua_State *L) {
	AnimationTemplate *pAT = checkAnimationTemplate(L);
	pAT->addFrame(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

static int at_setFrame(lua_State *L) {
	AnimationTemplate *pAT = checkAnimationTemplate(L);
	pAT->setFrame(static_cast<int>(luaL_checknumber(L, 2)), static_cast<int>(luaL_checknumber(L, 3)));
	return 0;
}

static bool animationTypeStringToNumber(const char *typeString, Animation::ANIMATION_TYPES &result) {
	if (strcmp(typeString, "jojo") == 0) {
		result = Animation::AT_JOJO;
		return true;
	} else if (strcmp(typeString, "loop") == 0) {
		result = Animation::AT_LOOP;
		return true;
	} else if (strcmp(typeString, "oneshot") == 0) {
		result = Animation::AT_ONESHOT;
		return true;
	} else
		return false;
}

static int at_setAnimationType(lua_State *L) {
	AnimationTemplate *pAT = checkAnimationTemplate(L);
	Animation::ANIMATION_TYPES animationType;
	if (animationTypeStringToNumber(luaL_checkstring(L, 2), animationType)) {
		pAT->setAnimationType(animationType);
	} else {
		luaL_argcheck(L, 0, 2, "Invalid animation type");
	}

	return 0;
}

static int at_setFPS(lua_State *L) {
	AnimationTemplate *pAT = checkAnimationTemplate(L);
	pAT->setFPS(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

static int at_finalize(lua_State *L) {
	AnimationTemplate *pAT = checkAnimationTemplate(L);
	delete pAT;
	return 0;
}

static const luaL_reg ANIMATION_TEMPLATE_METHODS[] = {
	{"AddFrame", at_addFrame},
	{"SetFrame", at_setFrame},
	{"SetAnimationType", at_setAnimationType},
	{"SetFPS", at_setFPS},
	{"__gc", at_finalize},
	{0, 0}
};

static GraphicEngine *getGE() {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	GraphicEngine *pGE = pKernel->getGfx();
	assert(pGE);
	return pGE;
}

static int init(lua_State *L) {
	GraphicEngine *pGE = getGE();

	switch (lua_gettop(L)) {
	case 0:
		lua_pushbooleancpp(L, pGE->init());
		break;
	case 1:
		lua_pushbooleancpp(L, pGE->init(static_cast<int>(luaL_checknumber(L, 1))));
		break;
	case 2:
		lua_pushbooleancpp(L, pGE->init(static_cast<int>(luaL_checknumber(L, 1)), static_cast<int>(luaL_checknumber(L, 2))));
		break;
	case 3:
		lua_pushbooleancpp(L, pGE->init(static_cast<int>(luaL_checknumber(L, 1)), static_cast<int>(luaL_checknumber(L, 2)),
		                                static_cast<int>(luaL_checknumber(L, 3))));
		break;
	case 4:
	default:
		lua_pushbooleancpp(L, pGE->init(static_cast<int>(luaL_checknumber(L, 1)), static_cast<int>(luaL_checknumber(L, 2)),
		                                static_cast<int>(luaL_checknumber(L, 3)), static_cast<int>(luaL_checknumber(L, 4))));
		break;
	}


#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Main-Panel zum Gfx-Modul hinzufügen
	RenderObjectPtr<Panel> mainPanelPtr(getGE()->getMainPanel());
	assert(mainPanelPtr.isValid());

	lua_pushstring(L, GFX_LIBRARY_NAME);
	lua_gettable(L, LUA_GLOBALSINDEX);
	assert(!lua_isnil(L, -1));

	newUintUserData(L, mainPanelPtr->getHandle());
	assert(!lua_isnil(L, -1));
	// luaL_getmetatable(L, PANEL_CLASS_NAME);
	LuaBindhelper::getMetatable(L, PANEL_CLASS_NAME);
	assert(!lua_isnil(L, -1));
	lua_setmetatable(L, -2);

	lua_pushstring(L, "MainPanel");
	lua_insert(L, -2);
	lua_settable(L, -3);

	lua_pop(L, 1);

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(L));
#endif

	return 1;
}

static int startFrame(lua_State *L) {
	GraphicEngine *pGE = getGE();

	if (lua_gettop(L) == 0)
		lua_pushbooleancpp(L, pGE->startFrame());
	else
		lua_pushbooleancpp(L, pGE->startFrame(lua_tobooleancpp(L, 1)));

	return 1;
}

static int endFrame(lua_State *L) {
	GraphicEngine *pGE = getGE();

	lua_pushbooleancpp(L, pGE->endFrame());

	return 1;
}

static int getBitDepth(lua_State *L) {
	GraphicEngine *pGE = getGE();

	lua_pushnumber(L, pGE->getBitDepth());

	return 1;
}

static int setVsync(lua_State *L) {
	GraphicEngine *pGE = getGE();

	pGE->setVsync(lua_tobooleancpp(L, 1));

	return 0;
}

static int isVsync(lua_State *L) {
	GraphicEngine *pGE = getGE();

	lua_pushbooleancpp(L, pGE->getVsync());

	return 1;
}

static int getLastFrameDuration(lua_State *L) {
	GraphicEngine *pGE = getGE();

	lua_pushnumber(L, pGE->getLastFrameDuration());

	return 1;
}

static int stopMainTimer(lua_State *L) {
	GraphicEngine *pGE = getGE();
	pGE->stopMainTimer();
	return 0;
}

static int resumeMainTimer(lua_State *L) {
	GraphicEngine *pGE = getGE();
	pGE->resumeMainTimer();
	return 0;
}

static int getSecondaryFrameDuration(lua_State *L) {
	GraphicEngine *pGE = getGE();

	lua_pushnumber(L, pGE->getSecondaryFrameDuration());

	return 1;
}

static int saveThumbnailScreenshot(lua_State *L) {
	GraphicEngine *pGE = getGE();
	lua_pushbooleancpp(L, pGE->saveThumbnailScreenshot(luaL_checkstring(L, 1)));
	return 1;
}

// Marks a function that should never be used
static int dummyFuncError(lua_State *L) {
	error("Dummy function invoked by LUA");
	return 1;
}

static const luaL_reg GFX_FUNCTIONS[] = {
	{"Init", init},
	{"StartFrame", startFrame},
	{"EndFrame", endFrame},
	{"DrawDebugLine", dummyFuncError},
	{"SetVsync", setVsync},
	{"GetDisplayWidth", dummyFuncError},
	{"GetDisplayHeight", dummyFuncError},
	{"GetBitDepth", getBitDepth},
	{"IsVsync", isVsync},
	{"IsWindowed", dummyFuncError},
	{"GetFPSCount", dummyFuncError},
	{"GetLastFrameDuration", getLastFrameDuration},
	{"StopMainTimer", stopMainTimer},
	{"ResumeMainTimer", resumeMainTimer},
	{"GetSecondaryFrameDuration", getSecondaryFrameDuration},
	{"SaveScreenshot", dummyFuncError},
	{"NewAnimationTemplate", newAnimationTemplate},
	{"GetRepaintedPixels", dummyFuncError},
	{"SaveThumbnailScreenshot", saveThumbnailScreenshot},
	{0, 0}
};

static RenderObjectPtr<RenderObject> checkRenderObject(lua_State *L, bool errorIfRemoved = true) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable einer Klasse haben, die von Gfx.RenderObject "erbt".
	uint *userDataPtr;
	if ((userDataPtr = (uint *)LuaBindhelper::my_checkudata(L, 1, BITMAP_CLASS_NAME)) != 0 ||
	        (userDataPtr = (uint *)LuaBindhelper::my_checkudata(L, 1, ANIMATION_CLASS_NAME)) != 0 ||
	        (userDataPtr = (uint *)LuaBindhelper::my_checkudata(L, 1, PANEL_CLASS_NAME)) != 0 ||
	        (userDataPtr = (uint *)LuaBindhelper::my_checkudata(L, 1, TEXT_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> roPtr(*userDataPtr);
		if (roPtr.isValid())
			return roPtr;
		else {
			if (errorIfRemoved)
				luaL_error(L, "The renderobject with the handle %d does no longer exist.", *userDataPtr);
		}
	} else {
		luaL_argcheck(L, 0, 1, "'" RENDEROBJECT_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<RenderObject>();
}

static int ro_setPos(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	Vertex pos;
	Vertex::luaVertexToVertex(L, 2, pos);
	roPtr->setPos(pos.x, pos.y);
	return 0;
}

static int ro_setX(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	roPtr->setX(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

static int ro_setY(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	roPtr->setY(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

static int ro_setZ(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	roPtr->setZ(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

static int ro_setVisible(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	roPtr->setVisible(lua_tobooleancpp(L, 2));
	return 0;
}

static int ro_getX(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	lua_pushnumber(L, roPtr->getX());

	return 1;
}

static int ro_getY(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	lua_pushnumber(L, roPtr->getY());

	return 1;
}

static int ro_getZ(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	lua_pushnumber(L, roPtr->getZ());

	return 1;
}

static int ro_getAbsoluteX(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	lua_pushnumber(L, roPtr->getAbsoluteX());

	return 1;
}

static int ro_getAbsoluteY(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	lua_pushnumber(L, roPtr->getAbsoluteY());

	return 1;
}

static int ro_getWidth(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	lua_pushnumber(L, roPtr->getWidth());

	return 1;
}

static int ro_getHeight(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	lua_pushnumber(L, roPtr->getHeight());

	return 1;
}

static int ro_isVisible(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	lua_pushbooleancpp(L, roPtr->isVisible());

	return 1;
}

static int ro_addPanel(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	RenderObjectPtr<Panel> panelPtr = roPtr->addPanel(static_cast<int>(luaL_checknumber(L, 2)),
	                                        static_cast<int>(luaL_checknumber(L, 3)),
	                                        GraphicEngine::luaColorToARGBColor(L, 4));
	if (panelPtr.isValid()) {
		newUintUserData(L, panelPtr->getHandle());
		// luaL_getmetatable(L, PANEL_CLASS_NAME);
		LuaBindhelper::getMetatable(L, PANEL_CLASS_NAME);
		assert(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	} else
		lua_pushnil(L);

	return 1;
}

static int ro_addBitmap(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	RenderObjectPtr<Bitmap> bitmaPtr = roPtr->addBitmap(luaL_checkstring(L, 2));
	if (bitmaPtr.isValid()) {
		newUintUserData(L, bitmaPtr->getHandle());
		// luaL_getmetatable(L, BITMAP_CLASS_NAME);
		LuaBindhelper::getMetatable(L, BITMAP_CLASS_NAME);
		assert(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	} else
		lua_pushnil(L);

	return 1;
}

static int ro_addText(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());

	RenderObjectPtr<Text> textPtr;
	if (lua_gettop(L) >= 3)
		textPtr = roPtr->addText(luaL_checkstring(L, 2), luaL_checkstring(L, 3));
	else
		textPtr = roPtr->addText(luaL_checkstring(L, 2));

	if (textPtr.isValid()) {
		newUintUserData(L, textPtr->getHandle());
		// luaL_getmetatable(L, TEXT_CLASS_NAME);
		LuaBindhelper::getMetatable(L, TEXT_CLASS_NAME);
		assert(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	} else
		lua_pushnil(L);

	return 1;
}

static int ro_addAnimation(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());

	RenderObjectPtr<Animation> animationPtr;
	if (lua_type(L, 2) == LUA_TUSERDATA)
		animationPtr = roPtr->addAnimation(*checkAnimationTemplate(L, 2));
	else
		animationPtr = roPtr->addAnimation(luaL_checkstring(L, 2));

	if (animationPtr.isValid()) {
		newUintUserData(L, animationPtr->getHandle());
		// luaL_getmetatable(L, ANIMATION_CLASS_NAME);
		LuaBindhelper::getMetatable(L, ANIMATION_CLASS_NAME);
		assert(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);

		// Alle Animationscallbacks registrieren.
		animationPtr->setCallbacks();
	} else
		lua_pushnil(L);

	return 1;
}

void Animation::setCallbacks() {
	_actionCallback = animationActionCallback;
	_loopPointCallback = animationLoopPointCallback;
	_deleteCallback = animationDeleteCallback;
}

static const luaL_reg RENDEROBJECT_METHODS[] = {
	{"AddAnimation", ro_addAnimation},
	{"AddText", ro_addText},
	{"AddBitmap", ro_addBitmap},
	{"AddPanel", ro_addPanel},
	{"SetPos", ro_setPos},
	{"SetX", ro_setX},
	{"SetY", ro_setY},
	{"SetZ", ro_setZ},
	{"SetVisible", ro_setVisible},
	{"GetX", ro_getX},
	{"GetY", ro_getY},
	{"GetZ", ro_getZ},
	{"GetAbsoluteX", ro_getAbsoluteX},
	{"GetAbsoluteY", ro_getAbsoluteY},
	{"GetWidth", ro_getWidth},
	{"GetHeight", ro_getHeight},
	{"IsVisible", ro_isVisible},
	{0, 0}
};

static RenderObjectPtr<Panel> checkPanel(lua_State *L) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Panel
	uint *userDataPtr;
	if ((userDataPtr = (uint *)LuaBindhelper::my_checkudata(L, 1, PANEL_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> roPtr(*userDataPtr);
		if (roPtr.isValid())
			return roPtr->toPanel();
		else
			luaL_error(L, "The panel with the handle %d does no longer exist.", *userDataPtr);
	} else {
		luaL_argcheck(L, 0, 1, "'" PANEL_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<Panel>();
}

static int p_getColor(lua_State *L) {
	RenderObjectPtr<Panel> PanelPtr = checkPanel(L);
	assert(PanelPtr.isValid());
	GraphicEngine::ARGBColorToLuaColor(L, PanelPtr->getColor());

	return 1;
}

static int p_setColor(lua_State *L) {
	RenderObjectPtr<Panel> PanelPtr = checkPanel(L);
	assert(PanelPtr.isValid());
	PanelPtr->setColor(GraphicEngine::luaColorToARGBColor(L, 2));
	return 0;
}

static int p_remove(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	roPtr.erase();
	return 0;
}

static const luaL_reg PANEL_METHODS[] = {
	{"GetColor", p_getColor},
	{"SetColor", p_setColor},
	{"Remove", p_remove},
	{0, 0}
};

static RenderObjectPtr<Bitmap> checkBitmap(lua_State *L) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Bitmap
	uint *userDataPtr;
	if ((userDataPtr = (uint *)LuaBindhelper::my_checkudata(L, 1, BITMAP_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> roPtr(*userDataPtr);
		if (roPtr.isValid())
			return roPtr->toBitmap();
		else
			luaL_error(L, "The bitmap with the handle %d does no longer exist.", *userDataPtr);
	} else {
		luaL_argcheck(L, 0, 1, "'" BITMAP_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<Bitmap>();
}

static int b_setAlpha(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	bitmapPtr->setAlpha(static_cast<uint>(luaL_checknumber(L, 2)));
	return 0;
}

static int b_setTintColor(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	bitmapPtr->setModulationColor(GraphicEngine::luaColorToARGBColor(L, 2));
	return 0;
}

static int b_setScaleFactor(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	bitmapPtr->setScaleFactor(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

static int b_setScaleFactorX(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	bitmapPtr->setScaleFactorX(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

static int b_setScaleFactorY(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	bitmapPtr->setScaleFactorY(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

static int b_setFlipH(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	bitmapPtr->setFlipH(lua_tobooleancpp(L, 2));
	return 0;
}

static int b_setFlipV(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	bitmapPtr->setFlipV(lua_tobooleancpp(L, 2));
	return 0;
}

static int b_getAlpha(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	lua_pushnumber(L, bitmapPtr->getAlpha());
	return 1;
}

static int b_getTintColor(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	GraphicEngine::ARGBColorToLuaColor(L, bitmapPtr->getModulationColor());
	return 1;
}

static int b_getScaleFactorX(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	lua_pushnumber(L, bitmapPtr->getScaleFactorX());
	return 1;
}

static int b_getScaleFactorY(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	lua_pushnumber(L, bitmapPtr->getScaleFactorY());
	return 1;
}

static int b_isFlipH(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	lua_pushbooleancpp(L, bitmapPtr->isFlipH());
	return 1;
}

static int b_isFlipV(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	lua_pushbooleancpp(L, bitmapPtr->isFlipV());
	return 1;
}

static int b_getPixel(lua_State *L) {
	RenderObjectPtr<Bitmap> bitmapPtr = checkBitmap(L);
	assert(bitmapPtr.isValid());
	Vertex Pos;
	Vertex::luaVertexToVertex(L, 2, Pos);
	GraphicEngine::ARGBColorToLuaColor(L, bitmapPtr->getPixel(Pos.x, Pos.y));
	return 1;
}

static int b_remove(lua_State *L) {
	RenderObjectPtr<RenderObject> roPtr = checkRenderObject(L);
	assert(roPtr.isValid());
	roPtr.erase();
	return 0;
}

static const luaL_reg BITMAP_METHODS[] = {
	{"SetAlpha", b_setAlpha},
	{"SetTintColor", b_setTintColor},
	{"SetScaleFactor", b_setScaleFactor},
	{"SetScaleFactorX", b_setScaleFactorX},
	{"SetScaleFactorY", b_setScaleFactorY},
	{"SetFlipH", b_setFlipH},
	{"SetFlipV", b_setFlipV},
	{"GetAlpha", b_getAlpha},
	{"GetTintColor", b_getTintColor},
	{"GetScaleFactorX", b_getScaleFactorX},
	{"GetScaleFactorY", b_getScaleFactorY},
	{"IsFlipH", b_isFlipH},
	{"IsFlipV", b_isFlipV},
	{"GetPixel", b_getPixel},
	{"IsScalingAllowed", dummyFuncError},
	{"IsAlphaAllowed", dummyFuncError},
	{"IsTintingAllowed", dummyFuncError},
	{"Remove", b_remove},
	{0, 0}
};

static RenderObjectPtr<Animation> checkAnimation(lua_State *L) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Animation
	uint *userDataPtr;
	if ((userDataPtr = (uint *)LuaBindhelper::my_checkudata(L, 1, ANIMATION_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> roPtr(*userDataPtr);
		if (roPtr.isValid())
			return roPtr->toAnimation();
		else
			luaL_error(L, "The animation with the handle %d does no longer exist.", *userDataPtr);
	} else {
		luaL_argcheck(L, 0, 1, "'" ANIMATION_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<Animation>();
}

static int a_play(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->play();
	return 0;
}

static int a_pause(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->pause();
	return 0;
}

static int a_stop(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->stop();
	return 0;
}

static int a_setFrame(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->setFrame(static_cast<uint>(luaL_checknumber(L, 2)));
	return 0;
}

static int a_setAlpha(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->setAlpha(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

static int a_setTintColor(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->setModulationColor(GraphicEngine::luaColorToARGBColor(L, 2));
	return 0;
}

static int a_setScaleFactor(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->setScaleFactor(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

static int a_setScaleFactorX(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->setScaleFactorX(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

static int a_setScaleFactorY(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr->setScaleFactorY(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

static int a_getScaleFactorX(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushnumber(L, animationPtr->getScaleFactorX());
	return 1;
}

static int a_getScaleFactorY(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushnumber(L, animationPtr->getScaleFactorY());
	return 1;
}

static int a_getAnimationType(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	switch (animationPtr->getAnimationType()) {
	case Animation::AT_JOJO:
		lua_pushstring(L, "jojo");
		break;
	case Animation::AT_LOOP:
		lua_pushstring(L, "loop");
		break;
	case Animation::AT_ONESHOT:
		lua_pushstring(L, "oneshot");
		break;
	default:
		assert(false);
	}
	return 1;
}

static int a_getFPS(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushnumber(L, animationPtr->getFPS());
	return 1;
}

static int a_getFrameCount(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushnumber(L, animationPtr->getFrameCount());
	return 1;
}

static int a_isScalingAllowed(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushbooleancpp(L, animationPtr->isScalingAllowed());
	return 1;
}

static int a_isAlphaAllowed(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushbooleancpp(L, animationPtr->isAlphaAllowed());
	return 1;
}

static int a_isTintingAllowed(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushbooleancpp(L, animationPtr->isColorModulationAllowed());
	return 1;
}

static int a_getCurrentFrame(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushnumber(L, animationPtr->getCurrentFrame());
	return 1;
}

static int a_getCurrentAction(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushstring(L, animationPtr->getCurrentAction().c_str());
	return 1;
}

static int a_isPlaying(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	lua_pushbooleancpp(L, animationPtr->isRunning());
	return 1;
}

static bool animationLoopPointCallback(uint handle) {
	lua_State *L = static_cast<lua_State *>(Kernel::getInstance()->getScript()->getScriptObject());
	loopPointCallbackPtr->invokeCallbackFunctions(L, handle);

	return true;
}

static int a_registerLoopPointCallback(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	loopPointCallbackPtr->registerCallbackFunction(L, animationPtr->getHandle());

	return 0;
}

static int a_unregisterLoopPointCallback(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	loopPointCallbackPtr->unregisterCallbackFunction(L, animationPtr->getHandle());

	return 0;
}

static bool animationActionCallback(uint Handle) {
	RenderObjectPtr<Animation> animationPtr(Handle);
	if (animationPtr.isValid()) {
		actionCallbackPtr->Action = animationPtr->getCurrentAction();
		lua_State *L = static_cast<lua_State *>(Kernel::getInstance()->getScript()->getScriptObject());
		actionCallbackPtr->invokeCallbackFunctions(L, animationPtr->getHandle());
	}

	return true;
}

static int a_registerActionCallback(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	actionCallbackPtr->registerCallbackFunction(L, animationPtr->getHandle());

	return 0;
}

static int a_unregisterActionCallback(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	actionCallbackPtr->unregisterCallbackFunction(L, animationPtr->getHandle());

	return 0;
}

static bool animationDeleteCallback(uint Handle) {
	lua_State *L = static_cast<lua_State *>(Kernel::getInstance()->getScript()->getScriptObject());
	loopPointCallbackPtr->removeAllObjectCallbacks(L, Handle);

	return true;
}

static int a_remove(lua_State *L) {
	RenderObjectPtr<Animation> animationPtr = checkAnimation(L);
	assert(animationPtr.isValid());
	animationPtr.erase();
	return 0;
}

static const luaL_reg ANIMATION_METHODS[] = {
	{"Play", a_play},
	{"Pause", a_pause},
	{"Stop", a_stop},
	{"SetFrame", a_setFrame},
	{"SetAlpha", a_setAlpha},
	{"SetTintColor", a_setTintColor},
	{"SetScaleFactor", a_setScaleFactor},
	{"SetScaleFactorX", a_setScaleFactorX},
	{"SetScaleFactorY", a_setScaleFactorY},
	{"GetScaleFactorX", a_getScaleFactorX},
	{"GetScaleFactorY", a_getScaleFactorY},
	{"GetAnimationType", a_getAnimationType},
	{"GetFPS", a_getFPS},
	{"GetFrameCount", a_getFrameCount},
	{"IsScalingAllowed", a_isScalingAllowed},
	{"IsAlphaAllowed", a_isAlphaAllowed},
	{"IsTintingAllowed", a_isTintingAllowed},
	{"GetCurrentFrame", a_getCurrentFrame},
	{"GetCurrentAction", a_getCurrentAction},
	{"IsPlaying", a_isPlaying},
	{"RegisterLoopPointCallback", a_registerLoopPointCallback},
	{"UnregisterLoopPointCallback", a_unregisterLoopPointCallback},
	{"RegisterActionCallback", a_registerActionCallback},
	{"UnregisterActionCallback", a_unregisterActionCallback},
	{"Remove", a_remove},
	{0, 0}
};

static RenderObjectPtr<Text> checkText(lua_State *L) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Text
	uint *userDataPtr;
	if ((userDataPtr = (uint *)LuaBindhelper::my_checkudata(L, 1, TEXT_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> roPtr(*userDataPtr);
		if (roPtr.isValid())
			return roPtr->toText();
		else
			luaL_error(L, "The text with the handle %d does no longer exist.", *userDataPtr);
	} else {
		luaL_argcheck(L, 0, 1, "'" TEXT_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<Text>();
}

static int t_setFont(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	textPtr->setFont(luaL_checkstring(L, 2));
	return 0;
}

static int t_setText(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	textPtr->setText(luaL_checkstring(L, 2));
	return 0;
}

static int t_setAlpha(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	textPtr->setAlpha(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

static int t_setColor(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	textPtr->setColor(GraphicEngine::luaColorToARGBColor(L, 2));
	return 0;
}

static int t_setAutoWrap(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	textPtr->setAutoWrap(lua_tobooleancpp(L, 2));
	return 0;
}

static int t_setAutoWrapThreshold(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	textPtr->setAutoWrapThreshold(static_cast<uint>(luaL_checknumber(L, 2)));
	return 0;
}

static int t_getText(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	lua_pushstring(L, textPtr->getText().c_str());
	return 1;
}

static int t_getFont(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	lua_pushstring(L, textPtr->getFont().c_str());
	return 1;
}

static int t_getAlpha(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	lua_pushnumber(L, textPtr->getAlpha());
	return 1;
}

static int t_getColor(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	lua_pushnumber(L, textPtr->getColor());
	return 1;
}

static int t_isAutoWrap(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	lua_pushbooleancpp(L, textPtr->isAutoWrapActive());
	return 1;
}

static int t_getAutoWrapThreshold(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	lua_pushnumber(L, textPtr->getAutoWrapThreshold());
	return 1;
}

static int t_remove(lua_State *L) {
	RenderObjectPtr<Text> textPtr = checkText(L);
	assert(textPtr.isValid());
	textPtr.erase();
	return 0;
}

static const luaL_reg TEXT_METHODS[] = {
	{"SetFont", t_setFont},
	{"SetText", t_setText},
	{"SetAlpha", t_setAlpha},
	{"SetColor", t_setColor},
	{"SetAutoWrap", t_setAutoWrap},
	{"SetAutoWrapThreshold", t_setAutoWrapThreshold},
	{"GetText", t_getText},
	{"GetFont", t_getFont},
	{"GetAlpha", t_getAlpha},
	{"GetColor", t_getColor},
	{"IsAutoWrap", t_isAutoWrap},
	{"GetAutoWrapThreshold", t_getAutoWrapThreshold},
	{"Remove", t_remove},
	{0, 0}
};

bool GraphicEngine::registerScriptBindings() {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	ScriptEngine *pScript = pKernel->getScript();
	assert(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->getScriptObject());
	assert(L);

	if (!LuaBindhelper::addMethodsToClass(L, BITMAP_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, ANIMATION_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, PANEL_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, TEXT_CLASS_NAME, RENDEROBJECT_METHODS)) return false;

	if (!LuaBindhelper::addMethodsToClass(L, PANEL_CLASS_NAME, PANEL_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, BITMAP_CLASS_NAME, BITMAP_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, TEXT_CLASS_NAME, TEXT_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, ANIMATION_CLASS_NAME, ANIMATION_METHODS)) return false;

	if (!LuaBindhelper::addMethodsToClass(L, ANIMATION_TEMPLATE_CLASS_NAME, ANIMATION_TEMPLATE_METHODS)) return false;

	if (!LuaBindhelper::addFunctionsToLib(L, GFX_LIBRARY_NAME, GFX_FUNCTIONS)) return false;

	assert(loopPointCallbackPtr == 0);
	loopPointCallbackPtr = new LuaCallback(L);

	assert(actionCallbackPtr == 0);
	actionCallbackPtr = new ActionCallback(L);

	return true;
}

void GraphicEngine::unregisterScriptBindings() {
	delete loopPointCallbackPtr;
	loopPointCallbackPtr = 0;

	delete actionCallbackPtr;
	actionCallbackPtr = 0;
}

} // End of namespace Sword25
