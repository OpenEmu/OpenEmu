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

#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/saveload.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_game.h" // Temporary
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "common/savefile.h"
#include "common/config-manager.h"

namespace Wintermute {

bool SaveLoad::loadGame(const Common::String &filename, BaseGame *gameRef) {
	gameRef->LOG(0, "Loading game '%s'...", filename.c_str());

	bool ret;

	gameRef->_renderer->initSaveLoad(false);

	gameRef->_loadInProgress = true;
	BasePersistenceManager *pm = new BasePersistenceManager();
	if (DID_SUCCEED(ret = pm->initLoad(filename))) {
		//if (DID_SUCCEED(ret = cleanup())) {
		if (DID_SUCCEED(ret = SystemClassRegistry::getInstance()->loadTable(gameRef,  pm))) {
			if (DID_SUCCEED(ret = SystemClassRegistry::getInstance()->loadInstances(gameRef,  pm))) {
				// Restore random-seed:
				BaseEngine::instance().getRandomSource()->setSeed(pm->getDWORD());

				// data initialization after load
				SaveLoad::initAfterLoad();

				gameRef->applyEvent("AfterLoad", true);

				gameRef->displayContent(true, false);
				//_renderer->flip();
			}
		}
	}

	delete pm;
	gameRef->_loadInProgress = false;

	gameRef->_renderer->endSaveLoad();

	//_gameRef->LOG(0, "Load end %d", BaseUtils::GetUsedMemMB());
	// AdGame:
	if (DID_SUCCEED(ret)) {
		SystemClassRegistry::getInstance()->enumInstances(SaveLoad::afterLoadRegion, "AdRegion", NULL);
	}
	return ret;
}

bool SaveLoad::saveGame(int slot, const char *desc, bool quickSave, BaseGame *gameRef) {
	Common::String filename = SaveLoad::getSaveSlotFilename(slot);

	gameRef->LOG(0, "Saving game '%s'...", filename.c_str());

	gameRef->applyEvent("BeforeSave", true);

	bool ret;

	BasePersistenceManager *pm = new BasePersistenceManager();
	if (DID_SUCCEED(ret = pm->initSave(desc))) {
		gameRef->_renderer->initSaveLoad(true, quickSave); // TODO: The original code inited the indicator before the conditionals
		if (DID_SUCCEED(ret = SystemClassRegistry::getInstance()->saveTable(gameRef,  pm, quickSave))) {
			if (DID_SUCCEED(ret = SystemClassRegistry::getInstance()->saveInstances(gameRef,  pm, quickSave))) {
				pm->putDWORD(BaseEngine::instance().getRandomSource()->getSeed());
				if (DID_SUCCEED(ret = pm->saveFile(filename))) {
					ConfMan.setInt("most_recent_saveslot", slot);
				}
			}
		}
	}

	delete pm;

	gameRef->_renderer->endSaveLoad();

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool SaveLoad::initAfterLoad() {
	SystemClassRegistry::getInstance()->enumInstances(afterLoadRegion,   "BaseRegion",   NULL);
	SystemClassRegistry::getInstance()->enumInstances(afterLoadSubFrame, "BaseSubFrame", NULL);
	SystemClassRegistry::getInstance()->enumInstances(afterLoadSound,    "BaseSound",    NULL);
	SystemClassRegistry::getInstance()->enumInstances(afterLoadFont,     "BaseFontTT",   NULL);
	SystemClassRegistry::getInstance()->enumInstances(afterLoadScript,   "ScScript",  NULL);
	// AdGame:
	SystemClassRegistry::getInstance()->enumInstances(afterLoadScene,   "AdScene",   NULL);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void SaveLoad::afterLoadScene(void *scene, void *data) {
	((AdScene *)scene)->afterLoad();
}

//////////////////////////////////////////////////////////////////////////
void SaveLoad::afterLoadRegion(void *region, void *data) {
	((BaseRegion *)region)->createRegion();
}


//////////////////////////////////////////////////////////////////////////
void SaveLoad::afterLoadSubFrame(void *subframe, void *data) {
	((BaseSubFrame *)subframe)->setSurfaceSimple();
}


//////////////////////////////////////////////////////////////////////////
void SaveLoad::afterLoadSound(void *sound, void *data) {
	((BaseSound *)sound)->setSoundSimple();
}

//////////////////////////////////////////////////////////////////////////
void SaveLoad::afterLoadFont(void *font, void *data) {
	((BaseFont *)font)->afterLoad();
}

//////////////////////////////////////////////////////////////////////////
void SaveLoad::afterLoadScript(void *script, void *data) {
	((ScScript *)script)->afterLoad();
}

Common::String SaveLoad::getSaveSlotFilename(int slot) {
	BasePersistenceManager *pm = new BasePersistenceManager();
	Common::String filename = pm->getFilenameForSlot(slot);
	delete pm;
	debugC(kWintermuteDebugSaveGame, "getSaveSlotFileName(%d) = %s", slot, filename.c_str());
	return filename;
}

bool SaveLoad::getSaveSlotDescription(int slot, char *buffer) {
	buffer[0] = '\0';

	Common::String filename = getSaveSlotFilename(slot);
	BasePersistenceManager *pm = new BasePersistenceManager();
	if (!pm) {
		return false;
	}

	if (!(pm->initLoad(filename))) {
		delete pm;
		return false;
	}

	strcpy(buffer, pm->_savedDescription);
	delete pm;

	return true;
}

bool SaveLoad::isSaveSlotUsed(int slot) {
	Common::String filename = getSaveSlotFilename(slot);
	BasePersistenceManager *pm = new BasePersistenceManager();
	bool ret = pm->getSaveExists(slot);
	delete pm;
	return ret;
}

bool SaveLoad::emptySaveSlot(int slot) {
	Common::String filename = getSaveSlotFilename(slot);
	BasePersistenceManager *pm = new BasePersistenceManager();
	((WintermuteEngine *)g_engine)->getSaveFileMan()->removeSavefile(pm->getFilenameForSlot(slot));
	delete pm;
	return true;
}


} // end of namespace Wintermute
