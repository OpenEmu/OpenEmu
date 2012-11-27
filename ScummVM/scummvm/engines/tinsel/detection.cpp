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
#include "common/file.h"
#include "common/md5.h"
#include "common/savefile.h"

#include "tinsel/bmv.h"
#include "tinsel/cursor.h"
#include "tinsel/tinsel.h"
#include "tinsel/savescn.h"	// needed by TinselMetaEngine::listSaves

namespace Tinsel {

struct TinselGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 TinselEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 TinselEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Language TinselEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform TinselEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 TinselEngine::getVersion() const {
	return _gameDescription->version;
}

bool TinselEngine::getIsADGFDemo() const {
	return (bool)(_gameDescription->desc.flags & ADGF_DEMO);
}

bool TinselEngine::isCD() const {
	return (bool)(_gameDescription->desc.flags & ADGF_CD);
}

} // End of namespace Tinsel

static const PlainGameDescriptor tinselGames[] = {
	{"tinsel", "Tinsel engine game"},
	{"dw", "Discworld"},
	{"dw2", "Discworld 2: Missing Presumed ...!?"},
	{0, 0}
};

#include "tinsel/detection_tables.h"

class TinselMetaEngine : public AdvancedMetaEngine {
public:
	TinselMetaEngine() : AdvancedMetaEngine(Tinsel::gameDescriptions, sizeof(Tinsel::TinselGameDescription), tinselGames) {
		_singleid = "tinsel";
	}

	virtual const char *getName() const {
		return "Tinsel";
	}

	virtual const char *getOriginalCopyright() const {
		return "Tinsel (C) Psygnosis";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool TinselMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Tinsel::TinselEngine::hasFeature(EngineFeature f) const {
	return
#if 0
		// FIXME: It is possible to return to the launcher from tinsel.
		// But then any attempt to re-enter the engine will lead to
		// a crash or at least seriously broken behavior.
		//
		// This is because the Tinsel engine makes use of tons of
		// global variables (static and non-static) which are never
		// explicitly re-initialized when the engine is started
		// for a second time.
		(f == kSupportsRTL) ||
#endif
		(f == kSupportsLoadingDuringRuntime);
}

namespace Tinsel {
extern int getList(Common::SaveFileManager *saveFileMan, const Common::String &target);
}

SaveStateList TinselMetaEngine::listSaves(const char *target) const {
	Common::String pattern = target;
	pattern = pattern + ".???";
	Common::StringArray files = g_system->getSavefileManager()->listSavefiles(pattern);
	sort(files.begin(), files.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = files.begin(); file != files.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		const Common::String &fname = *file;
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fname);
		if (in) {
			in->readUint32LE();		// skip id
			in->readUint32LE();		// skip size
			in->readUint32LE();		// skip version
			char saveDesc[Tinsel::SG_DESC_LEN];
			in->read(saveDesc, sizeof(saveDesc));

			saveDesc[Tinsel::SG_DESC_LEN - 1] = 0;

			saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
			delete in;
		}
	}

	return saveList;
}

bool TinselMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Tinsel::TinselGameDescription *gd = (const Tinsel::TinselGameDescription *)desc;
	if (gd) {
		*engine = new Tinsel::TinselEngine(syst, gd);
	}
	return gd != 0;
}

struct SizeMD5 {
	int size;
	Common::String md5;
};
typedef Common::HashMap<Common::String, SizeMD5, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SizeMD5Map;
typedef Common::HashMap<Common::String, Common::FSNode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
typedef Common::Array<const ADGameDescription *> ADGameDescList;

/**
 * Fallback detection scans the list of Discworld 2 targets to see if it can detect an installation
 * where the files haven't been renamed (i.e. don't have the '1' just before the extension)
 */
const ADGameDescription *TinselMetaEngine::fallbackDetect(const FileMap &allFilesXXX, const Common::FSList &fslist) const {
	Common::String extra;
	FileMap allFiles;
	SizeMD5Map filesSizeMD5;

	const ADGameFileDescription *fileDesc;
	const Tinsel::TinselGameDescription *g;

	if (fslist.empty())
		return NULL;

	// TODO: The following code is essentially a slightly modified copy of the
	// complete code of function detectGame() in engines/advancedDetector.cpp.
	// That quite some hefty and undesirable code duplication. Its only purpose
	// seems to be to treat filenames of the form "foo1.ext" as "foo.ext".
	// It would be nice to avoid this code duplication.

	// First we compose a hashmap of all files in fslist.
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			if (!scumm_stricmp(file->getName().c_str(), "dw2")) {
				// Probably Discworld 2 subfolder on CD, so add it's contents as well
				Common::FSList files;
				if (file->getChildren(files, Common::FSNode::kListAll)) {
					Common::FSList::const_iterator file2;
					for (file2 = files.begin(); file2 != files.end(); ++file2) {
						if (file2->isDirectory())
							continue;

						Common::String fname = file2->getName();
						allFiles[fname] = *file2;
					}
				}
			}
			continue;
		}

		Common::String tstr = file->getName();

		allFiles[tstr] = *file;	// Record the presence of this file
	}

	// Check which files are included in some dw2 ADGameDescription *and* present
	// in fslist without a '1' suffix character. Compute MD5s and file sizes for these files.
	for (g = &Tinsel::gameDescriptions[0]; g->desc.gameid != 0; ++g) {
		if (strcmp(g->desc.gameid, "dw2") != 0)
			continue;

		for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++) {
			// Get the next filename, stripping off any '1' suffix character
			char tempFilename[50];
			strcpy(tempFilename, fileDesc->fileName);
			char *pOne = strchr(tempFilename, '1');
			if (pOne) {
				do {
					*pOne = *(pOne + 1);
					pOne++;
				} while (*pOne);
			}

			Common::String fname(tempFilename);
			if (allFiles.contains(fname) && !filesSizeMD5.contains(fname)) {
				SizeMD5 tmp;
				Common::File testFile;

				if (testFile.open(allFiles[fname])) {
					tmp.size = (int32)testFile.size();
					tmp.md5 = computeStreamMD5AsString(testFile, _md5Bytes);
				} else {
					tmp.size = -1;
				}

				filesSizeMD5[fname] = tmp;
			}
		}
	}

	ADGameDescList matched;
	int maxFilesMatched = 0;

	// MD5 based matching
	for (g = &Tinsel::gameDescriptions[0]; g->desc.gameid != 0; ++g) {
		if (strcmp(g->desc.gameid, "dw2") != 0)
			continue;

		bool fileMissing = false;

		// Try to match all files for this game
		for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++) {
			// Get the next filename, stripping off any '1' suffix character
			char tempFilename[50];
			strcpy(tempFilename, fileDesc->fileName);
			char *pOne = strchr(tempFilename, '1');
			if (pOne) {
				do {
					*pOne = *(pOne + 1);
					pOne++;
				} while (*pOne);
			}

			Common::String tstr(tempFilename);

			if (!filesSizeMD5.contains(tstr)) {
				fileMissing = true;
				break;
			}

			if (fileDesc->md5 != NULL && fileDesc->md5 != filesSizeMD5[tstr].md5) {
				fileMissing = true;
				break;
			}

			if (fileDesc->fileSize != -1 && fileDesc->fileSize != filesSizeMD5[tstr].size) {
				fileMissing = true;
				break;
			}
		}

		if (!fileMissing) {
			// Count the number of matching files. Then, only keep those
			// entries which match a maximal amount of files.
			int curFilesMatched = 0;
			for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++)
				curFilesMatched++;

			if (curFilesMatched > maxFilesMatched) {
				maxFilesMatched = curFilesMatched;

				matched.clear();	// Remove any prior, lower ranked matches.
				matched.push_back((const ADGameDescription *)g);
			} else if (curFilesMatched == maxFilesMatched) {
				matched.push_back((const ADGameDescription *)g);
			}
		}
	}

	// We didn't find a match
	if (matched.empty())
		return NULL;

	return *matched.begin();
}

int TinselMetaEngine::getMaximumSaveSlot() const { return 99; }

void TinselMetaEngine::removeSaveState(const char *target, int slot) const {
	Tinsel::setNeedLoad();
	// Same issue here as with loadGameState(): we need the physical savegame
	// slot. Refer to bug #3387551.
	int listSlot = -1;
	const int numStates = Tinsel::getList(g_system->getSavefileManager(), target);
	for (int i = 0; i < numStates; ++i) {
		const char *fileName = Tinsel::ListEntry(i, Tinsel::LE_NAME);
		const int saveSlot = atoi(fileName + strlen(fileName) - 3);

		if (saveSlot == slot) {
			listSlot = i;
			break;
		}
	}

	g_system->getSavefileManager()->removeSavefile(Tinsel::ListEntry(listSlot, Tinsel::LE_NAME));
	Tinsel::setNeedLoad();
	Tinsel::getList(g_system->getSavefileManager(), target);
}

#if PLUGIN_ENABLED_DYNAMIC(TINSEL)
	REGISTER_PLUGIN_DYNAMIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#endif

namespace Tinsel {

Common::Error TinselEngine::loadGameState(int slot) {
	// FIXME: Hopefully this is only used when loading games via
	// the launcher, since we do a hacky savegame slot to savelist
	// entry mapping here.
	//
	// You might wonder why is needed and here is the answer:
	// The save/load dialog of the GMM operates with the physical
	// savegame slots, while Tinsel internally uses entry numbers in
	// a savelist (which is sorted latest to first). Now to allow
	// proper loading of (especially Discworld2) saves we need to
	// get a savelist entry number instead of the physical slot.
	//
	// There are different possible solutions:
	//
	// One way to fix this would be to pass the filename instead of
	// the savelist entry number to RestoreGame, though it could make
	// problems how DW2 handles CD switches. Normally DW2 would pass
	// '-2' as slot when it changes CDs.
	//
	// Another way would be to convert all of Tinsel to use physical
	// slot numbers instead of savelist entry numbers for loading.
	// This would also allow '-2' as slot for CD changes without
	// any major hackery.

	int listSlot = -1;
	const int numStates = Tinsel::getList();
	for (int i = 0; i < numStates; ++i) {
		const char *fileName = Tinsel::ListEntry(i, Tinsel::LE_NAME);
		const int saveSlot = atoi(fileName + strlen(fileName) - 3);

		if (saveSlot == slot) {
			listSlot = i;
			break;
		}
	}

	if (listSlot == -1)
		return Common::kUnknownError;	// TODO: proper error code

	RestoreGame(listSlot);
	return Common::kNoError;	// TODO: return success/failure
}

#if 0
Common::Error TinselEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String saveName = _vm->getSavegameFilename((int16)(slot + 1));
	char saveDesc[SG_DESC_LEN];
	Common::strlcpy(saveDesc, desc, SG_DESC_LEN);
	SaveGame((char *)saveName.c_str(), saveDesc);
	ProcessSRQueue();			// This shouldn't be needed, but for some reason it is...
	return Common::kNoError;	// TODO: return success/failure
}
#endif

bool TinselEngine::canLoadGameStateCurrently() { return !_bmv->MoviePlaying(); }

#if 0
bool TinselEngine::canSaveGameStateCurrently() { return isCursorShown(); }
#endif

} // End of namespace Tinsel
