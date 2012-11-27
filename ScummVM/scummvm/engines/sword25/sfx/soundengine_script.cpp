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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/sfx/soundengine.h"

namespace Sword25 {

static int init(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	if (lua_gettop(L) == 0)
		lua_pushbooleancpp(L, pSfx->init(44100, 32));
	else if (lua_gettop(L) == 1)
		lua_pushbooleancpp(L, pSfx->init(static_cast<uint>(luaL_checknumber(L, 1)), 32));
	else
		lua_pushbooleancpp(L, pSfx->init(static_cast<uint>(luaL_checknumber(L, 1)), static_cast<uint>(luaL_checknumber(L, 2))));

	return 1;
}

static int update(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->update();

	return 0;
}

static int setVolume(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->setVolume(static_cast<float>(luaL_checknumber(L, 1)),
	                static_cast<SoundEngine::SOUND_TYPES>(static_cast<uint>(luaL_checknumber(L, 2))));

	return 0;
}

static int getVolume(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	lua_pushnumber(L, pSfx->getVolume(static_cast<SoundEngine::SOUND_TYPES>(static_cast<uint>(luaL_checknumber(L, 1)))));

	return 1;
}

static int pauseAll(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->pauseAll();

	return 0;
}

static int resumeAll(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->resumeAll();

	return 0;
}

static int pauseLayer(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->pauseLayer(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

static int resumeLayer(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->resumeLayer(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

static void processPlayParams(lua_State *L, Common::String &fileName, SoundEngine::SOUND_TYPES &type, float &volume, float &pan, bool &loop, int &loopStart, int &loopEnd, uint &layer) {
	fileName = luaL_checkstring(L, 1);

	type = static_cast<SoundEngine::SOUND_TYPES>(static_cast<uint>(luaL_checknumber(L, 2)));

	if (lua_gettop(L) < 3 || lua_isnil(L, 3))
		volume = 1.0f;
	else
		volume = static_cast<float>(luaL_checknumber(L, 3));

	if (lua_gettop(L) < 4 || lua_isnil(L, 4))
		pan = 0.0f;
	else
		pan = static_cast<float>(luaL_checknumber(L, 4));

	if (lua_gettop(L) < 5 || lua_isnil(L, 5))
		loop = false;
	else
		loop = lua_tobooleancpp(L, 5);

	if (lua_gettop(L) < 6 || lua_isnil(L, 6))
		loopStart = -1;
	else
		loopStart = static_cast<int>(luaL_checknumber(L, 6));

	if (lua_gettop(L) < 7 || lua_isnil(L, 7))
		loopEnd = -1;
	else
		loopEnd = static_cast<int>(luaL_checknumber(L, 7));

	if (lua_gettop(L) < 8 || lua_isnil(L, 8))
		layer = 0;
	else
		layer = static_cast<uint>(luaL_checknumber(L, 8));
}

static int playSound(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	Common::String fileName;
	SoundEngine::SOUND_TYPES type;
	float volume;
	float pan;
	bool loop;
	int loopStart;
	int loopEnd;
	uint layer;
	processPlayParams(L, fileName, type, volume, pan, loop, loopStart, loopEnd, layer);

	lua_pushbooleancpp(L, pSfx->playSound(fileName, type, volume, pan, loop, loopStart, loopEnd, layer));

	return 1;
}

static int playSoundEx(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	Common::String fileName;
	SoundEngine::SOUND_TYPES type;
	float volume;
	float pan;
	bool loop;
	int loopStart;
	int loopEnd;
	uint layer;
	processPlayParams(L, fileName, type, volume, pan, loop, loopStart, loopEnd, layer);

	lua_pushnumber(L, pSfx->playSoundEx(fileName, type, volume, pan, loop, loopStart, loopEnd, layer));

	return 1;
}

static int setSoundVolume(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->setSoundVolume(static_cast<uint>(luaL_checknumber(L, 1)), static_cast<float>(luaL_checknumber(L, 2)));

	return 0;
}

static int setSoundPanning(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->setSoundPanning(static_cast<uint>(luaL_checknumber(L, 1)), static_cast<float>(luaL_checknumber(L, 2)));

	return 0;
}

static int pauseSound(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->pauseSound(static_cast<uint>(luaL_checknumber(L, 1)));

	return 0;
}

static int resumeSound(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->resumeSound(static_cast<uint>(luaL_checknumber(L, 1)));

	return 0;
}

static int stopSound(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	pSfx->stopSound(static_cast<uint>(luaL_checknumber(L, 1)));

	return 0;
}

static int isSoundPaused(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	lua_pushbooleancpp(L, pSfx->isSoundPaused(static_cast<uint>(luaL_checknumber(L, 1))));

	return 1;
}

static int isSoundPlaying(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	lua_pushbooleancpp(L, pSfx->isSoundPlaying(static_cast<uint>(luaL_checknumber(L, 1))));

	return 1;
}

static int getSoundVolume(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	lua_pushnumber(L, pSfx->getSoundVolume(static_cast<uint>(luaL_checknumber(L, 1))));

	return 1;
}

static int getSoundPanning(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	SoundEngine *pSfx = pKernel->getSfx();
	assert(pSfx);

	lua_pushnumber(L, pSfx->getSoundPanning(static_cast<uint>(luaL_checknumber(L, 1))));

	return 1;
}

static const char *SFX_LIBRARY_NAME = "Sfx";

static const luaL_reg SFX_FUNCTIONS[] = {
	{"Init", init},
	{"Update", update},
	{"__SetVolume", setVolume},
	{"__GetVolume", getVolume},
	{"PauseAll", pauseAll},
	{"ResumeAll", resumeAll},
	{"PauseLayer", pauseLayer},
	{"ResumeLayer", resumeLayer},
	{"__PlaySound", playSound},
	{"__PlaySoundEx", playSoundEx},
	{"__SetSoundVolume", setSoundVolume},
	{"__SetSoundPanning", setSoundPanning},
	{"__PauseSound", pauseSound},
	{"__ResumeSound", resumeSound},
	{"__StopSound", stopSound},
	{"__IsSoundPaused", isSoundPaused},
	{"__IsSoundPlaying", isSoundPlaying},
	{"__GetSoundVolume", getSoundVolume},
	{"__GetSoundPanning", getSoundPanning},
	{0, 0}
};

static const lua_constant_reg SFX_CONSTANTS[] = {
	{"MUSIC", SoundEngine::MUSIC},
	{"SPEECH", SoundEngine::SPEECH},
	{"SFX", SoundEngine::SFX},
	{0, 0}
};

bool SoundEngine::registerScriptBindings() {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	ScriptEngine *pScript = pKernel->getScript();
	assert(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->getScriptObject());
	assert(L);

	if (!LuaBindhelper::addFunctionsToLib(L, SFX_LIBRARY_NAME, SFX_FUNCTIONS)) return false;
	if (!LuaBindhelper::addConstantsToLib(L, SFX_LIBRARY_NAME, SFX_CONSTANTS)) return false;

	return true;
}

} // End of namespace Sword25
