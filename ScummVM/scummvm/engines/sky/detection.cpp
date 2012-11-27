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

#include "sky/control.h"
#include "sky/sky.h"

#include "base/plugins.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "engines/metaengine.h"
#include "common/system.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "engines/metaengine.h"

static const PlainGameDescriptor skySetting =
	{"sky", "Beneath a Steel Sky" };

static const ExtraGuiOption skyExtraGuiOption = {
	_s("Floppy intro"),
	_s("Use the floppy version's intro (CD version only)"),
	"alt_intro",
	false
};

struct SkyVersion {
	int dinnerTableEntries;
	int dataDiskSize;
	const char *extraDesc;
	int version;
	const char *guioptions;
};

// TODO: Would be nice if Disk::determineGameVersion() used this table, too.
static const SkyVersion skyVersions[] = {
	{  232, -1, "floppy demo", 272, GUIO1(GUIO_NOSPEECH) }, // German
	{  243, -1, "pc gamer demo", 109, GUIO1(GUIO_NOSPEECH) },
	{  247, -1, "floppy demo", 267, GUIO1(GUIO_NOSPEECH) }, // English
	{ 1404, -1, "floppy", 288, GUIO1(GUIO_NOSPEECH) },
	{ 1413, -1, "floppy", 303, GUIO1(GUIO_NOSPEECH) },
	{ 1445, 8830435, "floppy", 348, GUIO1(GUIO_NOSPEECH) },
	{ 1445, -1, "floppy", 331, GUIO1(GUIO_NOSPEECH) },
	{ 1711, -1, "cd demo", 365, GUIO0() },
	{ 5099, -1, "cd", 368, GUIO0() },
	{ 5097, -1, "cd", 372, GUIO0() },
	{ 0, 0, 0, 0, 0 }
};

class SkyMetaEngine : public MetaEngine {
public:
	virtual const char *getName() const;
	virtual const char *getOriginalCopyright() const;

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual GameList getSupportedGames() const;
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const;
	virtual GameDescriptor findGame(const char *gameid) const;
	virtual GameList detectGames(const Common::FSList &fslist) const;

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;

	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

const char *SkyMetaEngine::getName() const {
	return "Sky";
}

const char *SkyMetaEngine::getOriginalCopyright() const {
	return "Beneath a Steel Sky (C) Revolution";
}

bool SkyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Sky::SkyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

GameList SkyMetaEngine::getSupportedGames() const {
	GameList games;
	games.push_back(skySetting);
	return games;
}

const ExtraGuiOptions SkyMetaEngine::getExtraGuiOptions(const Common::String &target) const {
	Common::String guiOptions;
	ExtraGuiOptions options;

	if (target.empty()) {
		options.push_back(skyExtraGuiOption);
		return options;
	}

	if (ConfMan.hasKey("guioptions", target)) {
		guiOptions = ConfMan.get("guioptions", target);
		guiOptions = parseGameGUIOptions(guiOptions);
	}

	if (!guiOptions.contains(GUIO_NOSPEECH))
		options.push_back(skyExtraGuiOption);
	return options;
}

GameDescriptor SkyMetaEngine::findGame(const char *gameid) const {
	if (0 == scumm_stricmp(gameid, skySetting.gameid))
		return skySetting;
	return GameDescriptor();
}

GameList SkyMetaEngine::detectGames(const Common::FSList &fslist) const {
	GameList detectedGames;
	bool hasSkyDsk = false;
	bool hasSkyDnr = false;
	int dinnerTableEntries = -1;
	int dataDiskSize = -1;

	// Iterate over all files in the given directory
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			const char *fileName = file->getName().c_str();

			if (0 == scumm_stricmp("sky.dsk", fileName)) {
				Common::File dataDisk;
				if (dataDisk.open(*file)) {
					hasSkyDsk = true;
					dataDiskSize = dataDisk.size();
				}
			}

			if (0 == scumm_stricmp("sky.dnr", fileName)) {
				Common::File dinner;
				if (dinner.open(*file)) {
					hasSkyDnr = true;
					dinnerTableEntries = dinner.readUint32LE();
				}
			}
		}
	}

	if (hasSkyDsk && hasSkyDnr) {
		// Match found, add to list of candidates, then abort inner loop.
		// The game detector uses US English by default. We want British
		// English to match the recorded voices better.
		GameDescriptor dg(skySetting.gameid, skySetting.description, Common::UNK_LANG, Common::kPlatformUnknown);
		const SkyVersion *sv = skyVersions;
		while (sv->dinnerTableEntries) {
			if (dinnerTableEntries == sv->dinnerTableEntries &&
				(sv->dataDiskSize == dataDiskSize || sv->dataDiskSize == -1)) {
				dg.updateDesc(Common::String::format("v0.0%d %s", sv->version, sv->extraDesc).c_str());
				dg.setGUIOptions(sv->guioptions);
				break;
			}
			++sv;
		}
		detectedGames.push_back(dg);
	}

	return detectedGames;
}

Common::Error SkyMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);
	*engine = new Sky::SkyEngine(syst);
	return Common::kNoError;
}

SaveStateList SkyMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;

	// Load the descriptions
	Common::StringArray savenames;
	savenames.resize(MAX_SAVE_GAMES+1);

	Common::InSaveFile *inf;
	inf = saveFileMan->openForLoading("SKY-VM.SAV");
	if (inf != NULL) {
		char *tmpBuf =  new char[MAX_SAVE_GAMES * MAX_TEXT_LEN];
		char *tmpPtr = tmpBuf;
		inf->read(tmpBuf, MAX_SAVE_GAMES * MAX_TEXT_LEN);
		for (int i = 0; i < MAX_SAVE_GAMES; ++i) {
			savenames[i] = tmpPtr;
			tmpPtr += savenames[i].size() + 1;
		}
		delete inf;
		delete[] tmpBuf;
	}

	// Find all saves
	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles("SKY-VM.???");
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	// Slot 0 is the autosave, if it exists.
	// TODO: Check for the existence of the autosave -- but this require us
	// to know which SKY variant we are looking at.
	saveList.insert_at(0, SaveStateDescriptor(0, "*AUTOSAVE*"));

	// Prepare the list of savestates by looping over all matching savefiles
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Extract the extension
		Common::String ext = file->c_str() + file->size() - 3;
		ext.toUppercase();
		if (Common::isDigit(ext[0]) && Common::isDigit(ext[1]) && Common::isDigit(ext[2])) {
			int slotNum = atoi(ext.c_str());
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				saveList.push_back(SaveStateDescriptor(slotNum+1, savenames[slotNum]));
				delete in;
			}
		}
	}

	return saveList;
}

int SkyMetaEngine::getMaximumSaveSlot() const { return MAX_SAVE_GAMES; }

void SkyMetaEngine::removeSaveState(const char *target, int slot) const {
	if (slot == 0)	// do not delete the auto save
		return;

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	char fName[20];
	sprintf(fName,"SKY-VM.%03d", slot - 1);
	saveFileMan->removeSavefile(fName);

	// Load current save game descriptions
	Common::StringArray savenames;
	savenames.resize(MAX_SAVE_GAMES+1);
	Common::InSaveFile *inf;
	inf = saveFileMan->openForLoading("SKY-VM.SAV");
	if (inf != NULL) {
		char *tmpBuf =  new char[MAX_SAVE_GAMES * MAX_TEXT_LEN];
		char *tmpPtr = tmpBuf;
		inf->read(tmpBuf, MAX_SAVE_GAMES * MAX_TEXT_LEN);
		for (int i = 0; i < MAX_SAVE_GAMES; ++i) {
			savenames[i] = tmpPtr;
			tmpPtr += savenames[i].size() + 1;
		}
		delete inf;
		delete[] tmpBuf;
	}

	// Update the save game description at the given slot
	savenames[slot - 1] = "";

	// Save the updated descriptions
	Common::OutSaveFile *outf;

	outf = saveFileMan->openForSaving("SKY-VM.SAV");
	bool ioFailed = true;
	if (outf) {
		for (uint16 cnt = 0; cnt < MAX_SAVE_GAMES; cnt++) {
			outf->write(savenames[cnt].c_str(), savenames[cnt].size() + 1);
		}
		outf->finalize();
		if (!outf->err())
			ioFailed = false;
		delete outf;
	}
	if (ioFailed)
		warning("Unable to store Savegame names to file SKY-VM.SAV. (%s)", saveFileMan->popErrorDesc().c_str());
}

#if PLUGIN_ENABLED_DYNAMIC(SKY)
	REGISTER_PLUGIN_DYNAMIC(SKY, PLUGIN_TYPE_ENGINE, SkyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SKY, PLUGIN_TYPE_ENGINE, SkyMetaEngine);
#endif

namespace Sky {
Common::Error SkyEngine::loadGameState(int slot) {
	uint16 result = _skyControl->quickXRestore(slot);
	return (result == GAME_RESTORED) ? Common::kNoError : Common::kUnknownError;
}

Common::Error SkyEngine::saveGameState(int slot, const Common::String &desc) {
	if (slot == 0)
		return Common::kWritePermissionDenied;	// we can't overwrite the auto save

	// Set the save slot and save the game
	_skyControl->_selectedGame = slot - 1;
	if (_skyControl->saveGameToFile(false) != GAME_SAVED)
		return Common::kWritePermissionDenied;

	// Load current save game descriptions
	Common::StringArray saveGameTexts;
	saveGameTexts.resize(MAX_SAVE_GAMES+1);
	_skyControl->loadDescriptions(saveGameTexts);

	// Update the save game description at the given slot
	saveGameTexts[slot - 1] = desc;
	// Save the updated descriptions
	_skyControl->saveDescriptions(saveGameTexts);

	return Common::kNoError;
}

bool SkyEngine::canLoadGameStateCurrently() {
	return _systemVars.pastIntro && _skyControl->loadSaveAllowed();
}

bool SkyEngine::canSaveGameStateCurrently() {
	return _systemVars.pastIntro && _skyControl->loadSaveAllowed();
}

} // End of namespace Sky
