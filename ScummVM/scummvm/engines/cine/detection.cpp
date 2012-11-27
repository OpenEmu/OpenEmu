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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "engines/obsolete.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "cine/cine.h"
#include "cine/various.h"

namespace Cine {

struct CINEGameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

int CineEngine::getGameType() const { return _gameDescription->gameType; }
uint32 CineEngine::getFeatures() const { return _gameDescription->features; }
Common::Language CineEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform CineEngine::getPlatform() const { return _gameDescription->desc.platform; }

} // End of namespace Cine

static const PlainGameDescriptor cineGames[] = {
	{"cine", "Cinematique evo.1 engine game"},
	{"fw", "Future Wars"},
	{"os", "Operation Stealth"},
	{0, 0}
};

static const Engines::ObsoleteGameID obsoleteGameIDsTable[] = {
	{"fw", "cine", Common::kPlatformUnknown},
	{"os", "cine", Common::kPlatformUnknown},
	{0, 0, Common::kPlatformUnknown}
};

#include "cine/detection_tables.h"

class CineMetaEngine : public AdvancedMetaEngine {
public:
	CineMetaEngine() : AdvancedMetaEngine(Cine::gameDescriptions, sizeof(Cine::CINEGameDescription), cineGames) {
		_singleid = "cine";
		_guioptions = GUIO1(GUIO_NOSPEECH);
	}

	virtual GameDescriptor findGame(const char *gameid) const {
		return Engines::findGameID(gameid, _gameids, obsoleteGameIDsTable);
	}

	virtual const char *getName() const {
		return "Cine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Future Wars & Operation Stealth (C) Delphine Software";
	}

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const {
		Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);
		return AdvancedMetaEngine::createInstance(syst, engine);
	}
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool CineMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Cine::CineEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool CineMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Cine::CINEGameDescription *gd = (const Cine::CINEGameDescription *)desc;
	if (gd) {
		*engine = new Cine::CineEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList CineMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;

	Common::String pattern = target;
	pattern += ".?";
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());
	Common::StringArray::const_iterator file;

	Common::String filename = target;
	filename += ".dir";
	Common::InSaveFile *in = saveFileMan->openForLoading(filename);
	if (in) {
		typedef char CommandeType[20];
		CommandeType saveNames[10];

		// Initialize all savegames' descriptions to empty strings
		// so that if the savegames' descriptions can only be partially read from file
		// then the missing ones are correctly set to empty strings.
		memset(saveNames, 0, sizeof(saveNames));

		in->read(saveNames, 10 * 20);
		CommandeType saveDesc;

		for (file = filenames.begin(); file != filenames.end(); ++file) {
			// Jump over savegame files that don't end with a digit (e.g. "fw.3" is ok, "fw.a" is not).
			if (!Common::isDigit(file->lastChar()))
				continue;

			// Obtain the last digit of the filename, since they correspond to the save slot
			int slotNum = atoi(file->c_str() + file->size() - 1);

			// Copy the savegame description making sure it ends with a trailing zero
			strncpy(saveDesc, saveNames[slotNum], 20);
			saveDesc[sizeof(CommandeType) - 1] = 0;

			saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
		}
	}

	delete in;

	return saveList;
}

int CineMetaEngine::getMaximumSaveSlot() const { return 9; }

void CineMetaEngine::removeSaveState(const char *target, int slot) const {
	// Load savegame descriptions from index file
	typedef char CommandeType[20];
	CommandeType saveNames[10];

	// Initialize all savegames' descriptions to empty strings
	// so that if the savegames' descriptions can only be partially read from file
	// then the missing ones are correctly set to empty strings.
	memset(saveNames, 0, sizeof(saveNames));

	Common::InSaveFile *in;
	in = g_system->getSavefileManager()->openForLoading(Common::String::format("%s.dir", target));

	if (!in)
		return;

	in->read(saveNames, 10 * 20);
	delete in;

	// Set description for selected slot
	char slotName[20];
	slotName[0] = 0;
	strncpy(saveNames[slot], slotName, 20);

	// Update savegame descriptions
	Common::String indexFile = Common::String::format("%s.dir", target);
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(indexFile);
	if (!out) {
		warning("Unable to open file %s for saving", indexFile.c_str());
		return;
	}

	out->write(saveNames, 10 * 20);
	delete out;

	// Delete save file
	char saveFileName[256];
	sprintf(saveFileName, "%s.%1d", target, slot);

	g_system->getSavefileManager()->removeSavefile(saveFileName);
}

#if PLUGIN_ENABLED_DYNAMIC(CINE)
	REGISTER_PLUGIN_DYNAMIC(CINE, PLUGIN_TYPE_ENGINE, CineMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CINE, PLUGIN_TYPE_ENGINE, CineMetaEngine);
#endif

namespace Cine {

Common::Error CineEngine::loadGameState(int slot) {
	char saveNameBuffer[256];
	sprintf(saveNameBuffer, "%s.%1d", _targetName.c_str(), slot);
	bool gameLoaded = makeLoad(saveNameBuffer);

	return gameLoaded ? Common::kNoError : Common::kUnknownError;
}

Common::Error CineEngine::saveGameState(int slot, const Common::String &desc) {
	// Load savegame descriptions from index file
	loadSaveDirectory();

	// Set description for selected slot making sure it ends with a trailing zero
	strncpy(currentSaveName[slot], desc.c_str(), 20);
	currentSaveName[slot][sizeof(CommandeType) - 1] = 0;

	// Update savegame descriptions
	Common::String indexFile = _targetName + ".dir";

	Common::OutSaveFile *fHandle = _saveFileMan->openForSaving(indexFile);
	if (!fHandle) {
		warning("Unable to open file %s for saving", indexFile.c_str());
		return Common::kUnknownError;
	}

	fHandle->write(currentSaveName, 10 * 20);
	delete fHandle;

	// Save game
	char saveFileName[256];
	sprintf(saveFileName, "%s.%1d", _targetName.c_str(), slot);
	makeSave(saveFileName);

	checkDataDisk(-1);

	return Common::kNoError;
}

bool CineEngine::canLoadGameStateCurrently() {
	return (!disableSystemMenu && !inMenu);
}

bool CineEngine::canSaveGameStateCurrently() {
	return (allowPlayerInput && !disableSystemMenu && !inMenu);
}

} // End of namespace Cine
