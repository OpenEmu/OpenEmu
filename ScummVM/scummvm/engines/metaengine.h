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

#ifndef ENGINES_METAENGINE_H
#define ENGINES_METAENGINE_H

#include "common/scummsys.h"
#include "common/error.h"
#include "common/array.h"

#include "engines/game.h"
#include "engines/savestate.h"

#include "base/plugins.h"

class Engine;
class OSystem;

namespace Common {
class FSList;
class String;
}

/**
 * Per-game extra GUI options structure.
 * Currently, this can only be used for options with checkboxes.
 */
struct ExtraGuiOption {
	const char *label;          // option label, e.g. "Fullscreen mode"
	const char *tooltip;        // option tooltip (when the mouse hovers above it)
	const char *configOption;   // confMan key, e.g. "fullscreen"
	bool defaultState;          // the detault state of the checkbox (checked or not)
};

typedef Common::Array<ExtraGuiOption> ExtraGuiOptions;

/**
 * A meta engine is essentially a factory for Engine instances with the
 * added ability of listing and detecting supported games.
 * Every engine "plugin" provides a hook to get an instance of a MetaEngine
 * subclass for that "engine plugin". E.g. SCUMM povides ScummMetaEngine.
 * This is then in turn used by the frontend code to detect games,
 * and instantiate actual Engine objects.
 */
class MetaEngine : public PluginObject {
public:
	virtual ~MetaEngine() {}

	/** Returns some copyright information about the original engine. */
	virtual const char *getOriginalCopyright() const = 0;

	/** Returns a list of games supported by this engine. */
	virtual GameList getSupportedGames() const = 0;

	/** Query the engine for a GameDescriptor for the specified gameid, if any. */
	virtual GameDescriptor findGame(const char *gameid) const = 0;

	/**
	 * Runs the engine's game detector on the given list of files, and returns a
	 * (possibly empty) list of games supported by the engine which it was able
	 * to detect amongst the given files.
	 */
	virtual GameList detectGames(const Common::FSList &fslist) const = 0;

	/**
	 * Tries to instantiate an engine instance based on the settings of
	 * the currently active ConfMan target. That is, the MetaEngine should
	 * query the ConfMan singleton for the target, gameid, path etc. data.
	 *
	 * @param syst	Pointer to the global OSystem object
	 * @param engine	Pointer to a pointer which the MetaEngine sets to
	 *					the newly create Engine, or 0 in case of an error
	 * @return		a Common::Error describing the error which occurred, or kNoError
	 */
	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const = 0;

	/**
	 * Return a list of all save states associated with the given target.
	 *
	 * The caller has to ensure that this (Meta)Engine is responsible
	 * for the specified target (by using findGame on it respectively
	 * on the associated gameid from the relevant ConfMan entry, if present).
	 *
	 * The default implementation returns an empty list.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsListSaves feature flag.
	 *
	 * @param target	name of a config manager target
	 * @return			a list of save state descriptors
	 */
	virtual SaveStateList listSaves(const char *target) const {
		return SaveStateList();
	}

	/**
	 * Return a list of extra GUI options for the specified target.
	 * If no target is specified, all of the available custom GUI options are
	 * Returned for the plugin (used to set default values).
	 *
	 * Currently, this only supports options with checkboxes.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @param target    name of a config manager target
	 * @return          a list of extra GUI options for an engine plugin and
	 *                  target
	 */
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const {
		return ExtraGuiOptions();
	}

	/**
	 * Return the maximum save slot that the engine supports.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsListSaves feature flag.
	 *
	 * The default implementation limits the save slots to zero (0).
	 *
	 * @return			maximum save slot number supported
	 */
	virtual int getMaximumSaveSlot() const {
		return 0;
	}

	/**
	 * Remove the specified save state.
	 *
	 * For most engines this just amounts to calling _saveFileMan->removeSaveFile().
	 * Engines which keep an index file will also update it accordingly.
	 *
	 * @note MetaEngines must indicate that this function has been implemented
	 *       via the kSupportsDeleteSave feature flag.
	 *
	 * @param target	name of a config manager target
	 * @param slot		slot number of the save state to be removed
	 */
	virtual void removeSaveState(const char *target, int slot) const {}

	/**
	 * Returns meta infos from the specified save state.
	 *
	 * Depending on the MetaEngineFeatures set this can include
	 * thumbnails, save date / time, play time.
	 *
	 * @param target	name of a config manager target
	 * @param slot		slot number of the save state
	 */
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const {
		return SaveStateDescriptor();
	}

	/** @name MetaEngineFeature flags */
	//@{

	/**
	 * A feature in this context means an ability of the engine which can be
	 * either available or not.
	 */
	enum MetaEngineFeature {
		/**
		 * Listing all Save States for a given target is supported, i.e.,
		 * the listSaves() and getMaximumSaveSlot methods are implemented.
		 * Used for --list-saves support, as well as the GMM load dialog.
		 */
		kSupportsListSaves,

		/**
		 * Loading from the Launcher / command line (-x)
		 */
		kSupportsLoadingDuringStartup,

		/**
		 * Deleting Saves from the Launcher (i.e. implements the
		 * removeSaveState() method)
		 */
		kSupportsDeleteSave,

		/**
		 * Features meta infos for savestates (i.e. implements the
		 * querySaveMetaInfos method properly).
		 *
		 * Engines implementing meta infos always have to provide
		 * the following entries in the save state descriptor queried
		 * by querySaveMetaInfos:
		 * - 'is_deletable', which indicates if a given save is
		 *                   safe for deletion
		 * - 'is_write_protected', which indicates if a given save
		 *                         can be overwritten by the user.
		 *                         (note: of course you do not have to
		 *                         set this, since it defaults to 'false')
		 */
		kSavesSupportMetaInfo,

		/**
		 * Features a thumbnail in savegames (i.e. includes a thumbnail
		 * in savestates returned via querySaveMetaInfo).
		 * This flag may only be set when 'kSavesSupportMetaInfo' is set.
		 */
		kSavesSupportThumbnail,

		/**
		 * Features 'save_date' and 'save_time' entries in the
		 * savestate returned by querySaveMetaInfo. Those values
		 * indicate the date/time the savegame was created.
		 * This flag may only be set when 'kSavesSupportMetaInfo' is set.
		 */
		kSavesSupportCreationDate,

		/**
		 * Features 'play_time' entry in the savestate returned by
		 * querySaveMetaInfo. It indicates how long the user played
		 * the game till the save.
		 * This flag may only be set when 'kSavesSupportMetaInfo' is set.
		 */
		kSavesSupportPlayTime
	};

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 * Used by e.g. the launcher to determine whether to enable the "Load" button.
	 */
	virtual bool hasFeature(MetaEngineFeature f) const {
		return false;
	}

	//@}
};


// Engine plugins

typedef PluginSubclass<MetaEngine> EnginePlugin;

/**
 * Singleton class which manages all Engine plugins.
 */
class EngineManager : public Common::Singleton<EngineManager> {
public:
	GameDescriptor findGameInLoadedPlugins(const Common::String &gameName, const EnginePlugin **plugin = NULL) const;
	GameDescriptor findGame(const Common::String &gameName, const EnginePlugin **plugin = NULL) const;
	GameList detectGames(const Common::FSList &fslist) const;
	const EnginePlugin::List &getPlugins() const;
};

/** Convenience shortcut for accessing the engine manager. */
#define EngineMan EngineManager::instance()

#endif
