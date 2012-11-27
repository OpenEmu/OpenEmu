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

// ---------------------------------------------------------------------------
// SAVE_REST.CPP	save, restore & restart functions
//
// James 05feb97
//
// "Jesus Saves", but could he Restore or Restart? He can now...
//
// ---------------------------------------------------------------------------


#include "common/memstream.h"
#include "common/savefile.h"
#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/object.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/saveload.h"
#include "sword2/screen.h"
#include "sword2/sound.h"

namespace Sword2 {

Common::String Sword2Engine::getSaveFileName(uint16 slotNo) {
	return Common::String::format("%s.%.3d", _targetName.c_str(), slotNo);
}

/**
 * Calculate size of required savegame buffer. A savegame consists of a header
 * and the global variables.
 */

uint32 Sword2Engine::findBufferSize() {
	return 212 + _resman->fetchLen(1);
}

/**
 * Save the game.
 */

uint32 Sword2Engine::saveGame(uint16 slotNo, const byte *desc) {
	char description[SAVE_DESCRIPTION_LEN];
	uint32 bufferSize = findBufferSize();
	byte *saveBuffer = (byte *)malloc(bufferSize);
	ScreenInfo *screenInfo = _screen->getScreenInfo();

	memset(description, 0, sizeof(description));
	strncpy(description, (const char *)desc, SAVE_DESCRIPTION_LEN - 1);

	Common::MemoryWriteStream writeS(saveBuffer, bufferSize);

	byte *globalVars = _resman->openResource(1);
	byte *objectHub = _resman->openResource(CUR_PLAYER_ID) + ResHeader::size();

	// Script no. 7 - 'george_savedata_request' calls fnPassPlayerSaveData
	_logic->runResScript(CUR_PLAYER_ID, 7);

	writeS.writeUint32LE(0);	// Checksum
	writeS.write(description, SAVE_DESCRIPTION_LEN);
	writeS.writeUint32LE(_resman->fetchLen(1));
	writeS.writeUint32LE(screenInfo->background_layer_id);
	writeS.writeUint32LE(_logic->getRunList());
	writeS.writeUint32LE(screenInfo->feet_x);
	writeS.writeUint32LE(screenInfo->feet_y);
	writeS.writeUint32LE(_sound->getLoopingMusicId());
	writeS.write(objectHub, ObjectHub::size());
	writeS.write(_logic->_saveLogic, ObjectLogic::size());
	writeS.write(_logic->_saveGraphic, ObjectGraphic::size());
	writeS.write(_logic->_saveMega, ObjectMega::size());
	writeS.write(globalVars, _resman->fetchLen(1));

	WRITE_LE_UINT32(saveBuffer, calcChecksum(saveBuffer + 4, bufferSize - 4));

	_resman->closeResource(CUR_PLAYER_ID);
	_resman->closeResource(1);

	uint32 errorCode = saveData(slotNo, saveBuffer, bufferSize);

	free(saveBuffer);

	if (errorCode != SR_OK) {
		uint32 textId;

		switch (errorCode) {
		case SR_ERR_FILEOPEN:
			textId = TEXT_SAVE_CANT_OPEN;
			break;
		default:
			textId = TEXT_SAVE_FAILED;
			break;
		}

		_screen->displayMsg(fetchTextLine(_resman->openResource(textId / SIZE), textId & 0xffff) + 2, 0);
	}

	return errorCode;
}

uint32 Sword2Engine::saveData(uint16 slotNo, byte *buffer, uint32 bufferSize) {
	Common::String saveFileName = getSaveFileName(slotNo);

	Common::OutSaveFile *out;

	if (!(out = _saveFileMan->openForSaving(saveFileName))) {
		return SR_ERR_FILEOPEN;
	}

	out->write(buffer, bufferSize);
	out->finalize();

	if (!out->err()) {
		delete out;
		return SR_OK;
	}

	delete out;
	return SR_ERR_WRITEFAIL;
}

/**
 * Restore the game.
 */

uint32 Sword2Engine::restoreGame(uint16 slotNo) {
	uint32 bufferSize = findBufferSize();
	byte *saveBufferMem = (byte *)malloc(bufferSize);

	uint32 errorCode = restoreData(slotNo, saveBufferMem, bufferSize);

	// If it was read in successfully, then restore the game from the
	// buffer & free the buffer. Note that restoreFromBuffer() frees the
	// buffer in order to clear it from memory before loading in the new
	// screen and runlist, so we only need to free it in case of failure.

	if (errorCode == SR_OK)
		errorCode = restoreFromBuffer(saveBufferMem, bufferSize);
	else
		free(saveBufferMem);

	if (errorCode != SR_OK) {
		uint32 textId;

		switch (errorCode) {
		case SR_ERR_FILEOPEN:
			textId = TEXT_RESTORE_CANT_OPEN;
			break;
		case SR_ERR_INCOMPATIBLE:
			textId = TEXT_RESTORE_INCOMPATIBLE;
			break;
		default:
			textId = TEXT_RESTORE_FAILED;
			break;
		}

		_screen->displayMsg(fetchTextLine(_resman->openResource(textId / SIZE), textId & 0xffff) + 2, 0);
	} else {
		// Prime system with a game cycle

		// Reset the graphic 'BuildUnit' list before a new logic list
		// (see fnRegisterFrame)
		_screen->resetRenderLists();

		// Reset the mouse hot-spot list. See fnRegisterMouse()
		// and fnRegisterFrame()
		_mouse->resetMouseList();

		if (_logic->processSession())
			error("restore 1st cycle failed??");
	}

	// Force the game engine to pick a cursor. This appears to be needed
	// when using the -x command-line option to restore a game.
	_mouse->setMouseTouching(1);
	return errorCode;
}

uint32 Sword2Engine::restoreData(uint16 slotNo, byte *buffer, uint32 bufferSize) {
	Common::String saveFileName = getSaveFileName(slotNo);

	Common::InSaveFile *in;

	if (!(in = _saveFileMan->openForLoading(saveFileName))) {
		// error: couldn't open file
		return SR_ERR_FILEOPEN;
	}

	// Read savegame into the buffer
	uint32 itemsRead = in->read(buffer, bufferSize);

	delete in;

	if (itemsRead != bufferSize) {
		// We didn't get all of it. At the moment we have no way of
		// knowing why, so assume that it's an incompatible savegame.

		return SR_ERR_INCOMPATIBLE;
	}

	return SR_OK;
}

uint32 Sword2Engine::restoreFromBuffer(byte *buffer, uint32 size) {
	Common::MemoryReadStream readS(buffer, size);

	// Calc checksum & check that aginst the value stored in the header

	if (readS.readUint32LE() != calcChecksum(buffer + 4, size - 4)) {
		free(buffer);
		return SR_ERR_INCOMPATIBLE;
	}

	readS.seek(SAVE_DESCRIPTION_LEN, SEEK_CUR);

	// Check savegame against length of current global variables resource
	// This would most probably be trapped by the checksum test anyway,
	// but it doesn't do any harm to check this as well.

	// Historical note: During development, earlier savegames would often
	// be shorter than the current expected length.

	if (readS.readUint32LE() != _resman->fetchLen(1)) {
		free(buffer);
		return SR_ERR_INCOMPATIBLE;
	}

	byte *globalVars = _resman->openResource(1);
	byte *objectHub = _resman->openResource(CUR_PLAYER_ID) + ResHeader::size();

	uint32 screenId = readS.readUint32LE();
	uint32 runListId = readS.readUint32LE();
	uint32 feetX = readS.readUint32LE();
	uint32 feetY = readS.readUint32LE();
	uint32 musicId = readS.readUint32LE();

	// Trash all resources from memory except player object & global vars
	_resman->killAll(false);
	_logic->resetKillList();

	readS.read(objectHub, ObjectHub::size());
	readS.read(_logic->_saveLogic, ObjectLogic::size());
	readS.read(_logic->_saveGraphic, ObjectGraphic::size());
	readS.read(_logic->_saveMega, ObjectMega::size());

	// Fill out the player object structures from the savegame structures.
	// Also run the appropriate scripts to set up George's anim tables and
	// walkdata, and Nico's anim tables.

	// Script no. 8 - 'george_savedata_return' calls fnGetPlayerSaveData
	_logic->runResScript(CUR_PLAYER_ID, 8);

	// Script no. 14 - 'set_up_nico_anim_tables'
	_logic->runResScript(CUR_PLAYER_ID, 14);

	// Which megaset was the player at the time of saving?
	ObjectMega obMega(_logic->_saveMega);

	uint32 scriptNo = 0;

	switch (obMega.getMegasetRes()) {
	case 36:		// GeoMega:
		scriptNo = 9;	// script no.9	- 'player_is_george'
		break;
	case 2003:		// GeoMegaB:
		scriptNo = 13;	// script no.13 - 'player_is_georgeB'
		break;
	case 1366:		// NicMegaA:
		scriptNo = 11;	// script no.11 - 'player_is_nicoA'
		break;
	case 1437:		// NicMegaB:
		scriptNo = 12;	// script no.12 - 'player_is_nicoB'
		break;
	case 1575:		// NicMegaC:
		scriptNo = 10;	// script no.10 - 'player_is_nicoC'
		break;
	}

	_logic->runResScript(CUR_PLAYER_ID, scriptNo);

	// Copy variables from savegame buffer to memory
	readS.read(globalVars, _resman->fetchLen(1));

	_resman->closeResource(CUR_PLAYER_ID);
	_resman->closeResource(1);

	free(buffer);

	int32 pars[2];

	pars[0] = screenId;
	pars[1] = 1;
	_logic->fnInitBackground(pars);

	ScreenInfo *screenInfo = _screen->getScreenInfo();

	// So palette not restored immediately after control panel - we want to
	// fade up instead!
	screenInfo->new_palette = 99;

	// These need setting after the defaults get set in fnInitBackground.
	// Remember that these can change through the game, so need saving &
	// restoring too.

	screenInfo->feet_x = feetX;
	screenInfo->feet_y = feetY;

	// Start the new run list
	_logic->expressChangeSession(runListId);

	// Force in the new scroll position, so unsightly scroll-catch-up does
	// not occur when screen first draws after returning from restore panel

	// Set the screen record of player position - ready for setScrolling()

	screenInfo->player_feet_x = obMega.getFeetX();
	screenInfo->player_feet_y = obMega.getFeetY();

	// if this screen is wide, recompute the scroll offsets now
	if (screenInfo->scroll_flag)
		_screen->setScrolling();

	// Any music required will be started after we've returned from
	// restoreControl() - see systemMenuMouse() in mouse.cpp!

	// Restart any looping music. Originally this was - and still is - done
	// in systemMenuMouse(), but with ScummVM we have other ways of
	// restoring savegames so it's easier to put it here as well.

	if (musicId) {
		pars[0] = musicId;
		pars[1] = FX_LOOP;
		_logic->fnPlayMusic(pars);
	} else
		_logic->fnStopMusic(NULL);

	return SR_OK;
}

/**
 * Get the description of a savegame
 */

uint32 Sword2Engine::getSaveDescription(uint16 slotNo, byte *description) {
	Common::String saveFileName = getSaveFileName(slotNo);

	Common::InSaveFile *in;

	if (!(in = _saveFileMan->openForLoading(saveFileName))) {
		return SR_ERR_FILEOPEN;
	}

	in->readUint32LE();
	in->read(description, SAVE_DESCRIPTION_LEN);

	delete in;
	return SR_OK;
}

bool Sword2Engine::saveExists() {
	Common::String pattern = _targetName + ".???";
	Common::StringArray filenames = _saveFileMan->listSavefiles(pattern);

	return !filenames.empty();
}

bool Sword2Engine::saveExists(uint16 slotNo) {
	Common::String saveFileName = getSaveFileName(slotNo);
	Common::InSaveFile *in;

	if (!(in = _saveFileMan->openForLoading(saveFileName))) {
		return false;
	}

	delete in;
	return true;
}

uint32 Sword2Engine::calcChecksum(byte *buffer, uint32 size) {
	uint32 total = 0;

	for (uint32 pos = 0; pos < size; pos++)
		total += buffer[pos];

	return total;
}

} // End of namespace Sword2
