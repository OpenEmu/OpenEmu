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

#include "gob/gob.h"
#include "gob/dataio.h"

#include "gob/detection/tables.h"

class GobMetaEngine : public AdvancedMetaEngine {
public:
	GobMetaEngine();

	virtual GameDescriptor findGame(const char *gameid) const;

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;

	virtual const char *getName() const;
	virtual const char *getOriginalCopyright() const;

	virtual bool hasFeature(MetaEngineFeature f) const;

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;

private:
	/**
	 * Inspect the game archives to detect which Once Upon A Time game this is.
	 */
	static const Gob::GOBGameDescription *detectOnceUponATime(const Common::FSList &fslist);
};

GobMetaEngine::GobMetaEngine() :
	AdvancedMetaEngine(Gob::gameDescriptions, sizeof(Gob::GOBGameDescription), gobGames) {

	_singleid   = "gob";
	_guioptions = GUIO1(GUIO_NOLAUNCHLOAD);
}

GameDescriptor GobMetaEngine::findGame(const char *gameid) const {
	return Engines::findGameID(gameid, _gameids, obsoleteGameIDsTable);
}

const ADGameDescription *GobMetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	ADFilePropertiesMap filesProps;

	const Gob::GOBGameDescription *game;
	game = (const Gob::GOBGameDescription *)detectGameFilebased(allFiles, fslist, Gob::fileBased, &filesProps);
	if (!game)
		return 0;

	if (game->gameType == Gob::kGameTypeOnceUponATime) {
		game = detectOnceUponATime(fslist);
		if (!game)
			return 0;
	}

	reportUnknown(fslist.begin()->getParent(), filesProps);
	return (const ADGameDescription *)game;
}

const Gob::GOBGameDescription *GobMetaEngine::detectOnceUponATime(const Common::FSList &fslist) {
	// Add the game path to the search manager
	SearchMan.clear();
	SearchMan.addDirectory(fslist.begin()->getParent().getPath(), fslist.begin()->getParent());

	// Open the archives
	Gob::DataIO dataIO;
	if (!dataIO.openArchive("stk1.stk", true) ||
	    !dataIO.openArchive("stk2.stk", true) ||
	    !dataIO.openArchive("stk3.stk", true)) {

		SearchMan.clear();
		return 0;
	}

	Gob::OnceUponATime gameType         = Gob::kOnceUponATimeInvalid;
	Gob::OnceUponATimePlatform platform = Gob::kOnceUponATimePlatformInvalid;

	// If these animal files are present, it's Abracadabra
	if (dataIO.hasFile("arai.anm") &&
	    dataIO.hasFile("crab.anm") &&
	    dataIO.hasFile("crap.anm") &&
	    dataIO.hasFile("drag.anm") &&
	    dataIO.hasFile("guep.anm") &&
	    dataIO.hasFile("loup.anm") &&
	    dataIO.hasFile("mous.anm") &&
	    dataIO.hasFile("rhin.anm") &&
	    dataIO.hasFile("saut.anm") &&
	    dataIO.hasFile("scor.anm"))
		gameType = Gob::kOnceUponATimeAbracadabra;

	// If these animal files are present, it's Baba Yaga
	if (dataIO.hasFile("abei.anm") &&
	    dataIO.hasFile("arai.anm") &&
	    dataIO.hasFile("drag.anm") &&
	    dataIO.hasFile("fauc.anm") &&
	    dataIO.hasFile("gren.anm") &&
	    dataIO.hasFile("rena.anm") &&
	    dataIO.hasFile("sang.anm") &&
	    dataIO.hasFile("serp.anm") &&
	    dataIO.hasFile("tort.anm") &&
	    dataIO.hasFile("vaut.anm"))
		gameType = Gob::kOnceUponATimeBabaYaga;

	// Detect the platform by endianness and existence of a MOD file
	Common::SeekableReadStream *villeDEC = dataIO.getFile("ville.dec");
	if (villeDEC && (villeDEC->size() > 6)) {
		byte data[6];

		if (villeDEC->read(data, 6) == 6) {
			if        (!memcmp(data, "\000\000\000\001\000\007", 6)) {
				// Big endian -> Amiga or Atari ST

				if (dataIO.hasFile("mod.babayaga"))
					platform = Gob::kOnceUponATimePlatformAmiga;
				else
					platform = Gob::kOnceUponATimePlatformAtariST;

			} else if (!memcmp(data, "\000\000\001\000\007\000", 6))
				// Little endian -> DOS
				platform = Gob::kOnceUponATimePlatformDOS;
		}

		delete villeDEC;
	}

	SearchMan.clear();

	if ((gameType == Gob::kOnceUponATimeInvalid) || (platform == Gob::kOnceUponATimePlatformInvalid)) {
		warning("GobMetaEngine::detectOnceUponATime(): Detection failed (%d, %d)",
		        (int) gameType, (int) platform);
		return 0;
	}

	return &Gob::fallbackOnceUpon[gameType][platform];
}

const char *GobMetaEngine::getName() const {
	return "Gob";
}

const char *GobMetaEngine::getOriginalCopyright() const {
	return "Goblins Games (C) Coktel Vision";
}

bool GobMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Gob::GobEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

Common::Error GobMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);
	return AdvancedMetaEngine::createInstance(syst, engine);
}

bool GobMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Gob::GOBGameDescription *gd = (const Gob::GOBGameDescription *)desc;
	if (gd) {
		*engine = new Gob::GobEngine(syst);
		((Gob::GobEngine *)*engine)->initGame(gd);
	}
	return gd != 0;
}


#if PLUGIN_ENABLED_DYNAMIC(GOB)
	REGISTER_PLUGIN_DYNAMIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#endif

namespace Gob {

void GobEngine::initGame(const GOBGameDescription *gd) {
	if (gd->startTotBase == 0)
		_startTot = "intro.tot";
	else
		_startTot = gd->startTotBase;

	if (gd->startStkBase == 0)
		_startStk = "intro.stk";
	else
		_startStk = gd->startStkBase;

	_demoIndex = gd->demoIndex;

	_gameType = gd->gameType;
	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;
}

} // End of namespace Gob
