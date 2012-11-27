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

#include "engines/advancedDetector.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_persistence_manager.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/util.h"

#include "engines/metaengine.h"

#include "engines/wintermute/detection_tables.h"

namespace Wintermute {

/**
 * The fallback game descriptor used by the Wintermute engine's fallbackDetector.
 * Contents of this struct are overwritten by the fallbackDetector. (logic copied partially
 * from the SCI-engine).
 */
static ADGameDescription s_fallbackDesc = {
	"",
	"",
	AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformWindows,
	ADGF_UNSTABLE,
	GUIO0()
};
static char s_fallbackGameIdBuf[256];

static const char *directoryGlobs[] = {
	"language", // To detect the various languages
	0
};

class WintermuteMetaEngine : public AdvancedMetaEngine {
public:
	WintermuteMetaEngine() : AdvancedMetaEngine(Wintermute::gameDescriptions, sizeof(ADGameDescription), Wintermute::wintermuteGames) {
		_singleid = "wintermute";
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}
	virtual const char *getName() const {
		return "Wintermute";
	}

	virtual const char *getOriginalCopyright() const {
		return "Copyright (c) 2011 Jan Nedoma";
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		// Set some defaults
		s_fallbackDesc.extra = "";
		s_fallbackDesc.language = Common::UNK_LANG;
		s_fallbackDesc.flags = ADGF_UNSTABLE;
		s_fallbackDesc.platform = Common::kPlatformWindows; // default to Windows
		s_fallbackDesc.gameid = "wintermute";
		s_fallbackDesc.guioptions = GUIO0();

		if (allFiles.contains("data.dcp")) {
			Common::String name, caption;
			if (WintermuteEngine::getGameInfo(fslist, name, caption)) {
				for (uint32 i = 0; i < name.size(); i++) {
					// Replace spaces (and other non-alphanumerics) with underscores
					if (!Common::isAlnum(name[(int32)i])) {
						name.setChar('_', (uint32)i);
					}
				}
				// Prefix to avoid collisions with actually known games
				name = "wmefan-" + name;
				Common::strlcpy(s_fallbackGameIdBuf, name.c_str(), sizeof(s_fallbackGameIdBuf) - 1);
				s_fallbackDesc.gameid = s_fallbackGameIdBuf;
				if (caption != name) {
					caption += " (fangame) ";
					char *offset = s_fallbackGameIdBuf + name.size() + 1;
					uint32 remainingLength = (sizeof(s_fallbackGameIdBuf) - 1) - (name.size() + 1);
					Common::strlcpy(offset, caption.c_str(), remainingLength);
					s_fallbackDesc.extra = offset;
					s_fallbackDesc.flags |= ADGF_USEEXTRAASTITLE;
				}
				return &s_fallbackDesc;
			} // Fall through to return 0;
		}
		return 0;
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		assert(syst);
		assert(engine);

		*engine = new Wintermute::WintermuteEngine(syst, desc);
		return true;
	}

	bool hasFeature(MetaEngineFeature f) const {
		switch (f) {
		case MetaEngine::kSupportsListSaves:
			return true;
		case MetaEngine::kSupportsLoadingDuringStartup:
			return true;
		case MetaEngine::kSupportsDeleteSave:
			return true;
		case MetaEngine::kSavesSupportCreationDate:
			return true;
		case MetaEngine::kSavesSupportMetaInfo:
			return true;
		case MetaEngine::kSavesSupportThumbnail:
			return true;
		default:
			return false;
		}
	}

	SaveStateList listSaves(const char *target) const {
		SaveStateList saves;
		Wintermute::BasePersistenceManager pm(target, true);
		for (int i = 0; i < getMaximumSaveSlot(); i++) {
			if (pm.getSaveExists(i)) {
				SaveStateDescriptor desc;
				pm.getSaveStateDesc(i, desc);
				saves.push_back(desc);
			}
		}
		return saves;
	}

	int getMaximumSaveSlot() const {
		return 100;
	}

	void removeSaveState(const char *target, int slot) const {
		Wintermute::BasePersistenceManager pm(target, true);
		pm.deleteSaveSlot(slot);
	}

	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const {
		Wintermute::BasePersistenceManager pm(target, true);
		SaveStateDescriptor retVal;
		retVal.setDescription("Invalid savegame");
		pm.getSaveStateDesc(slot, retVal);
		return retVal;
	}
};

} // end of namespace Wintermute

#if PLUGIN_ENABLED_DYNAMIC(WINTERMUTE)
REGISTER_PLUGIN_DYNAMIC(WINTERMUTE, PLUGIN_TYPE_ENGINE, Wintermute::WintermuteMetaEngine);
#else
REGISTER_PLUGIN_STATIC(WINTERMUTE, PLUGIN_TYPE_ENGINE, Wintermute::WintermuteMetaEngine);
#endif
