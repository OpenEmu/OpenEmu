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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "teenagent/console.h"
#include "teenagent/teenagent.h"

namespace TeenAgent {

Console::Console(TeenAgentEngine *engine) : _engine(engine) {
	DCmd_Register("enable_object",		WRAP_METHOD(Console, enableObject));
	DCmd_Register("disable_object",		WRAP_METHOD(Console, enableObject));
	DCmd_Register("set_ons",			WRAP_METHOD(Console, setOns));
	DCmd_Register("set_music",			WRAP_METHOD(Console, setMusic));
	DCmd_Register("animation",			WRAP_METHOD(Console, playAnimation));
	DCmd_Register("actor_animation",	WRAP_METHOD(Console, playActorAnimation));
	DCmd_Register("call",				WRAP_METHOD(Console, call));
}

bool Console::enableObject(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("usage: %s object_id [scene_id]\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	if (id < 0) {
		DebugPrintf("object id %d is invalid\n", id);
		return true;
	}

	int scene_id = 0;
	if (argc > 2) {
		scene_id = atoi(argv[2]);
		if (scene_id < 0) {
			DebugPrintf("scene id %d is invalid\n", scene_id);
			return true;
		}
	}

	if (strcmp(argv[0], "disable_object") == 0)
		_engine->disableObject(id, scene_id);
	else
		_engine->enableObject(id, scene_id);

	return true;
}

bool Console::setOns(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("usage: %s index(0-3) value [scene_id]\n", argv[0]);
		return true;
	}

	int index = atoi(argv[1]);
	if (index < 0 || index > 3) {
		DebugPrintf("index %d is invalid\n", index);
		return true;
	}

	int value = 0;
	value = atoi(argv[2]);
	if (value < 0) {
		DebugPrintf("invalid value\n");
		return true;
	}

	int scene_id = 0;
	if (argc > 3) {
		scene_id = atoi(argv[3]);
		if (scene_id < 0) {
			DebugPrintf("scene id %d is invalid\n", scene_id);
			return true;
		}
	}

	_engine->setOns(index, value, scene_id);
	return true;
}

bool Console::setMusic(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("usage: %s index(1-11)\n", argv[0]);
		return true;
	}

	int index = atoi(argv[1]);
	if (index <= 0 || index > 11) {
		DebugPrintf("invalid value\n");
		return true;
	}

	_engine->setMusic(index);
	return true;
}

bool Console::playAnimation(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("usage: %s id slot(0-3)\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	int slot = atoi(argv[2]);
	if (id < 0 || slot < 0 || slot > 3) {
		DebugPrintf("invalid slot or animation id\n");
		return true;
	}

	_engine->playAnimation(id, slot);
	return true;
}

bool Console::playActorAnimation(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("usage: %s id\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	if (id < 0) {
		DebugPrintf("invalid animation id\n");
		return true;
	}

	_engine->playActorAnimation(id);
	return true;
}

bool Console::call(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("usage: %s 0xHEXADDR\n", argv[0]);
		return true;
	}

	uint addr;
	if (sscanf(argv[1], "0x%x", &addr) != 1) {
		DebugPrintf("invalid address\n");
		return true;
	}

	if (!_engine->processCallback(addr))
		DebugPrintf("calling callback %04x failed\n", addr);

	return true;
}

}
