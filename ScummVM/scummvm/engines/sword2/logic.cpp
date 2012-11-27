/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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


#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/sound.h"

namespace Sword2 {

Logic::Logic(Sword2Engine *vm) :
	_vm(vm), _kills(0), _currentRunList(0), _moviePlayer(0),
	_smackerLeadIn(0), _smackerLeadOut(0), _sequenceTextLines(0),
	_speechTime(0), _animId(0), _speechAnimType(0), _leftClickDelay(0),
	_rightClickDelay(0), _officialTextNumber(0), _speechTextBlocNo(0) {

	_scriptVars = NULL;
	memset(_eventList, 0, sizeof(_eventList));
	memset(_syncList, 0, sizeof(_syncList));
	_router = new Router(_vm);

	_cycleSkip = false;
	_speechRunning = false;

	setupOpcodes();
}

Logic::~Logic() {
	delete _router;
}

/**
 * Do one cycle of the current session.
 */

int Logic::processSession() {
	// might change during the session, so take a copy here
	uint32 run_list = _currentRunList;

	_pc = 0;	// first object in list

	// by minusing the pc we can cause an immediate cessation of logic
	// processing on the current list

	while (_pc != 0xffffffff) {
		byte *game_object_list, *head, *raw_script_ad, *raw_data_ad;
		uint32 level, ret, script, id;

		game_object_list = _vm->_resman->openResource(run_list) + ResHeader::size();

		assert(_vm->_resman->fetchType(run_list) == RUN_LIST);

		// read the next id
		id = READ_LE_UINT32(game_object_list + 4 * _pc);
		_pc++;

		writeVar(ID, id);

		_vm->_resman->closeResource(run_list);

		if (!id) {
			// End of list - end the session naturally
			return 0;
		}

		assert(_vm->_resman->fetchType(id) == GAME_OBJECT);

		head = _vm->_resman->openResource(id);
		_curObjectHub.setAddress(head + ResHeader::size());

		level = _curObjectHub.getLogicLevel();

		debug(5, "Level %d id(%d) pc(%d)",
			level,
			_curObjectHub.getScriptId(level),
			_curObjectHub.getScriptPc(level));

		// Do the logic for this object. We keep going until a function
		// says to stop - remember, system operations are run via
		// function calls to drivers now.

		do {
			// There is a distinction between running one of our
			// own scripts and that of another object.

			level = _curObjectHub.getLogicLevel();
			script = _curObjectHub.getScriptId(level);

			if (script / SIZE == readVar(ID)) {
				// It's our own script

				debug(5, "Run script %d pc=%d",
					script / SIZE,
					_curObjectHub.getScriptPc(level));

				// This is the script data. Script and data
				// object are the same.

				raw_script_ad = head;

				ret = runScript2(raw_script_ad, raw_script_ad, _curObjectHub.getScriptPcPtr(level));
			} else {
				// We're running the script of another game
				// object - get our data object address.

				uint8 type = _vm->_resman->fetchType(script / SIZE);

				assert(type == GAME_OBJECT || type == SCREEN_MANAGER);

				raw_script_ad = _vm->_resman->openResource(script / SIZE);
				raw_data_ad = head;

				ret = runScript2(raw_script_ad, raw_data_ad, _curObjectHub.getScriptPcPtr(level));

				_vm->_resman->closeResource(script / SIZE);

				// reset to us for service script
				raw_script_ad = raw_data_ad;
			}

			if (ret == 1) {
				level = _curObjectHub.getLogicLevel();

				// The script finished - drop down a level
				if (level) {
					_curObjectHub.setLogicLevel(level - 1);
				} else {
					// Hmmm, level 0 terminated :-| Let's
					// be different this time and simply
					// let it restart next go :-)

					// Note that this really does happen a
					// lot, so don't make it a warning.

					debug(5, "object %d script 0 terminated", id);

					// reset to rerun, drop out for a cycle
					_curObjectHub.setScriptPc(level, _curObjectHub.getScriptId(level) & 0xffff);
					ret = 0;
				}
			} else if (ret > 2) {
				error("processSession: illegal script return type %d", ret);
			}

			// if ret == 2 then we simply go around again - a new
			// script or subroutine will kick in and run

			// keep processing scripts until 0 for quit is returned
		} while (ret);

		// Any post logic system requests to go here

		// Clear any syncs that were waiting for this character - it
		// has used them or now looses them

		clearSyncs(readVar(ID));

		if (_pc != 0xffffffff) {
			// The session is still valid so run the graphics/mouse
			// service script
			runScript(raw_script_ad, raw_script_ad, 0);
		}

		// and that's it so close the object resource

		_vm->_resman->closeResource(readVar(ID));
	}

	// Leaving a room so remove all ids that must reboot correctly. Then
	// restart the loop.

	for (uint32 i = 0; i < _kills; i++)
		_vm->_resman->remove(_objectKillList[i]);

	resetKillList();
	return 1;
}

/**
 * Bring an immediate halt to the session and cause a new one to start without
 * a screen update.
 */

void Logic::expressChangeSession(uint32 sesh_id) {
	// Set new session and force the old one to quit.
	_currentRunList = sesh_id;
	_pc = 0xffffffff;

	// Reset now in case we double-clicked an exit prior to changing screen
	writeVar(EXIT_FADING, 0);

	// We're trashing the list - presumably to change room. In theory,
	// sync waiting in the list could be left behind and never removed -
	// so we trash the lot
	memset(_syncList, 0, sizeof(_syncList));

	// Various clean-ups
	_router->clearWalkGridList();
	_vm->_sound->clearFxQueue(false);
	_router->freeAllRouteMem();
}

/**
 * @return The private _currentRunList variable.
 */

uint32 Logic::getRunList() {
	return _currentRunList;
}

/**
 * Move the current object up a level. Called by fnGosub command. Remember:
 * only the logic object has access to _curObjectHub.
 */

void Logic::logicUp(uint32 new_script) {
	debug(5, "new pc = %d", new_script & 0xffff);

	// going up a level - and we'll keep going this cycle
	_curObjectHub.setLogicLevel(_curObjectHub.getLogicLevel() + 1);

	assert(_curObjectHub.getLogicLevel() < 3);	// Can be 0, 1, 2
	logicReplace(new_script);
}

/**
 * Force the level to one.
 */

void Logic::logicOne(uint32 new_script) {
	_curObjectHub.setLogicLevel(1);
	logicReplace(new_script);
}

/**
 * Change current logic. Script must quit with a TERMINATE directive, which
 * does not write to &pc
 */

void Logic::logicReplace(uint32 new_script) {
	uint32 level = _curObjectHub.getLogicLevel();

	_curObjectHub.setScriptId(level, new_script);
	_curObjectHub.setScriptPc(level, new_script & 0xffff);
}

void Logic::resetKillList() {
	_kills = 0;
}

/**
 * Read current location number from script vars
 */

uint32 Logic::getLocationNum() {
	return readVar(LOCATION);
}

} // End of namespace Sword2
