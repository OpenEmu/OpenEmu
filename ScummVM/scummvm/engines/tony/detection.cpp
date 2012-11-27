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
 *
 */

#include "base/plugins.h"

#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "tony/tony.h"
#include "tony/game.h"

namespace Tony {

enum {
	GF_COMPRESSED = (1 << 0)
};

struct TonyGameDescription {
	ADGameDescription desc;
};

uint32 TonyEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language TonyEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool TonyEngine::getIsDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

bool TonyEngine::isCompressed() const {
	return _gameDescription->desc.flags & GF_COMPRESSED;
}

} // End of namespace Tony

static const PlainGameDescriptor tonyGames[] = {
	{"tony", "Tony Tough and the Night of Roasted Moths"},
	{0, 0}
};

#include "tony/detection_tables.h"

class TonyMetaEngine : public AdvancedMetaEngine {
public:
	TonyMetaEngine() : AdvancedMetaEngine(Tony::gameDescriptions, sizeof(Tony::TonyGameDescription), tonyGames) {
	}

	virtual const char *getName() const {
		return "Tony Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Tony Engine (C) Protonic Interactive";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool TonyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Tony::TonyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool TonyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Tony::TonyGameDescription *gd = (const Tony::TonyGameDescription *)desc;
	if (gd) {
		*engine = new Tony::TonyEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList TonyMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = "tony.0??";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			byte thumbnailData[160 * 120 * 2];
			Common::String saveName;
			byte difficulty;

			if (Tony::RMOptionScreen::loadThumbnailFromSaveState(slotNum, thumbnailData, saveName, difficulty)) {
				// Add the save name to the savegame list
				saveList.push_back(SaveStateDescriptor(slotNum, saveName));
			}
		}
	}

	return saveList;
}

int TonyMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

void TonyMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Tony::TonyEngine::getSaveStateFileName(slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor TonyMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String saveName;
	byte difficulty;
	byte thumbData[160 * 120 * 2];

	if (Tony::RMOptionScreen::loadThumbnailFromSaveState(slot, thumbData, saveName, difficulty)) {
		// Convert the 565 thumbnail data to the needed overlay format
		Common::MemoryReadStream thumbStream(thumbData, 160 * 120 * 2);
		Graphics::PixelFormat destFormat = g_system->getOverlayFormat();
		Graphics::Surface *to = new Graphics::Surface();
		to->create(160, 120, destFormat);

		OverlayColor *pixels = (OverlayColor *)to->pixels;
		for (int y = 0; y < to->h; ++y) {
			for (int x = 0; x < to->w; ++x) {
				uint8 r, g, b;
				Graphics::colorToRGB<Graphics::ColorMasks<555> >(thumbStream.readUint16LE(), r, g, b);

				// converting to current OSystem Color
				*pixels++ = destFormat.RGBToColor(r, g, b);
			}
		}

		// Create the return descriptor
		SaveStateDescriptor desc(slot, saveName);
		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);
		desc.setThumbnail(to);

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(TONY)
REGISTER_PLUGIN_DYNAMIC(TONY, PLUGIN_TYPE_ENGINE, TonyMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TONY, PLUGIN_TYPE_ENGINE, TonyMetaEngine);
#endif
