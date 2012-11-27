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

#include "common/func.h"
#include "common/debug.h"
#include "common/config-manager.h"

#ifdef DYNAMIC_MODULES
#include "common/fs.h"
#endif

// Plugin versioning

int pluginTypeVersions[PLUGIN_TYPE_MAX] = {
	PLUGIN_TYPE_ENGINE_VERSION,
	PLUGIN_TYPE_MUSIC_VERSION,
};


// Abstract plugins

PluginType Plugin::getType() const {
	return _type;
}

const char *Plugin::getName() const {
	return _pluginObject->getName();
}

class StaticPlugin : public Plugin {
public:
	StaticPlugin(PluginObject *pluginobject, PluginType type) {
		assert(pluginobject);
		assert(type < PLUGIN_TYPE_MAX);
		_pluginObject = pluginobject;
		_type = type;
	}

	~StaticPlugin() {
		delete _pluginObject;
	}

	virtual bool loadPlugin()		{ return true; }
	virtual void unloadPlugin()		{}
};

class StaticPluginProvider : public PluginProvider {
public:
	StaticPluginProvider() {
	}

	~StaticPluginProvider() {
	}

	virtual PluginList getPlugins() {
		PluginList pl;

		#define LINK_PLUGIN(ID) \
			extern PluginType g_##ID##_type; \
			extern PluginObject *g_##ID##_getObject(); \
			pl.push_back(new StaticPlugin(g_##ID##_getObject(), g_##ID##_type));

		// "Loader" for the static plugins.
		// Iterate over all registered (static) plugins and load them.

		// Engine plugins
		#include "engines/plugins_table.h"

		// Music plugins
		// TODO: Use defines to disable or enable each MIDI driver as a
		// static/dynamic plugin, like it's done for the engines
		LINK_PLUGIN(AUTO)
		LINK_PLUGIN(NULL)
		#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
		LINK_PLUGIN(WINDOWS)
		#endif
		#if defined(USE_ALSA)
		LINK_PLUGIN(ALSA)
		#endif
		#if defined(USE_SEQ_MIDI)
		LINK_PLUGIN(SEQ)
		#endif
		#if defined(USE_SNDIO)
		LINK_PLUGIN(SNDIO)
		#endif
		#if defined(__MINT__)
		LINK_PLUGIN(STMIDI)
		#endif
		#if defined(IRIX)
		LINK_PLUGIN(DMEDIA)
		#endif
		#if defined(__amigaos4__)
		LINK_PLUGIN(CAMD)
		#endif
		#if defined(MACOSX)
		LINK_PLUGIN(COREAUDIO)
		LINK_PLUGIN(COREMIDI)
		#endif
		#ifdef USE_FLUIDSYNTH
		LINK_PLUGIN(FLUIDSYNTH)
		#endif
		#ifdef USE_MT32EMU
		LINK_PLUGIN(MT32)
		#endif
		#if defined(__ANDROID__)
		LINK_PLUGIN(EAS)
		#endif
		LINK_PLUGIN(ADLIB)
		LINK_PLUGIN(PCSPK)
		LINK_PLUGIN(PCJR)
		LINK_PLUGIN(CMS)
		#ifndef DISABLE_SID
		LINK_PLUGIN(C64)
		#endif
		LINK_PLUGIN(AMIGA)
		LINK_PLUGIN(APPLEIIGS)
		LINK_PLUGIN(TOWNS)
		LINK_PLUGIN(PC98)
		#if defined(USE_TIMIDITY)
		LINK_PLUGIN(TIMIDITY)
		#endif

		return pl;
	}
};

#ifdef DYNAMIC_MODULES

PluginList FilePluginProvider::getPlugins() {
	PluginList pl;

	// Prepare the list of directories to search
	Common::FSList pluginDirs;

	// Add the default directories
	pluginDirs.push_back(Common::FSNode("."));
	pluginDirs.push_back(Common::FSNode("plugins"));

	// Add the provider's custom directories
	addCustomDirectories(pluginDirs);

	// Add the user specified directory
	Common::String pluginsPath(ConfMan.get("pluginspath"));
	if (!pluginsPath.empty())
		pluginDirs.push_back(Common::FSNode(pluginsPath));

	Common::FSList::const_iterator dir;
	for (dir = pluginDirs.begin(); dir != pluginDirs.end(); ++dir) {
		// Load all plugins.
		// Scan for all plugins in this directory
		Common::FSList files;
		if (!dir->getChildren(files, Common::FSNode::kListFilesOnly)) {
			debug(1, "Couldn't open plugin directory '%s'", dir->getPath().c_str());
			continue;
		} else {
			debug(1, "Reading plugins from plugin directory '%s'", dir->getPath().c_str());
		}

		for (Common::FSList::const_iterator i = files.begin(); i != files.end(); ++i) {
			if (isPluginFilename(*i)) {
				pl.push_back(createPlugin(*i));
			}
		}
	}

	return pl;
}

bool FilePluginProvider::isPluginFilename(const Common::FSNode &node) const {
	Common::String filename = node.getName();

#ifdef PLUGIN_PREFIX
	// Check the plugin prefix
	if (!filename.hasPrefix(PLUGIN_PREFIX))
		return false;
#endif

#ifdef PLUGIN_SUFFIX
	// Check the plugin suffix
	if (!filename.hasSuffix(PLUGIN_SUFFIX))
		return false;
#endif

	return true;
}

void FilePluginProvider::addCustomDirectories(Common::FSList &dirs) const {
#ifdef PLUGIN_DIRECTORY
	dirs.push_back(Common::FSNode(PLUGIN_DIRECTORY));
#endif
}

#endif // DYNAMIC_MODULES

#pragma mark -

PluginManager *PluginManager::_instance = NULL;

PluginManager &PluginManager::instance() {
	if (_instance)
		return *_instance;

#if defined(UNCACHED_PLUGINS) && defined(DYNAMIC_MODULES)
		_instance = new PluginManagerUncached();
#else
		_instance = new PluginManager();
#endif
	return *_instance;
}

PluginManager::PluginManager() {
	// Always add the static plugin provider.
	addPluginProvider(new StaticPluginProvider());
}

PluginManager::~PluginManager() {
	// Explicitly unload all loaded plugins
	unloadAllPlugins();

	// Delete the plugin providers
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		delete *pp;
	}
}

void PluginManager::addPluginProvider(PluginProvider *pp) {
	_providers.push_back(pp);
}

/**
 * This should only be called once by main()
 **/
void PluginManagerUncached::init() {
	unloadAllPlugins();
	_allEnginePlugins.clear();

	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false);	// empty the engine plugins

	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		PluginList pl((*pp)->getPlugins());

		for (PluginList::iterator p = pl.begin(); p != pl.end(); ++p) {
			// This is a 'hack' based on the assumption that we have no sound
			// file plugins. Currently this is the case. If it changes, we
			// should find a fast way of detecting whether a plugin is a
			// music or an engine plugin.
			if ((*pp)->isFilePluginProvider()) {
				_allEnginePlugins.push_back(*p);
			} else if ((*p)->loadPlugin()) { // and this is the proper method
				if ((*p)->getType() == PLUGIN_TYPE_ENGINE) {
					(*p)->unloadPlugin();
					_allEnginePlugins.push_back(*p);
				} else {	// add non-engine plugins to the 'in-memory' list
							// these won't ever get unloaded
					addToPluginsInMemList(*p);
				}
			}
 		}
 	}
}

/**
 * Try to load the plugin by searching in the ConfigManager for a matching
 * gameId under the domain 'plugin_files'.
 **/
bool PluginManagerUncached::loadPluginFromGameId(const Common::String &gameId) {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain("plugin_files");

	if (domain) {
		if (domain->contains(gameId)) {
			Common::String filename = (*domain)[gameId];

		    if (loadPluginByFileName(filename)) {
				return true;
			}
		}
	}
	return false;
}

/**
 * Load a plugin with a filename taken from ConfigManager.
 **/
bool PluginManagerUncached::loadPluginByFileName(const Common::String &filename) {
	if (filename.empty())
		return false;

	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false);

	PluginList::iterator i;
	for (i = _allEnginePlugins.begin(); i != _allEnginePlugins.end(); ++i) {
		if (Common::String((*i)->getFileName()) == filename && (*i)->loadPlugin()) {
			addToPluginsInMemList(*i);
			_currentPlugin = i;
			return true;
		}
	}
	return false;
}

/**
 * Update the config manager with a plugin file name that we found can handle
 * the game.
 **/
void PluginManagerUncached::updateConfigWithFileName(const Common::String &gameId) {
	// Check if we have a filename for the current plugin
	if ((*_currentPlugin)->getFileName()) {
		if (!ConfMan.hasMiscDomain("plugin_files"))
			ConfMan.addMiscDomain("plugin_files");

		Common::ConfigManager::Domain *domain = ConfMan.getDomain("plugin_files");
		assert(domain);
		(*domain)[gameId] = (*_currentPlugin)->getFileName();

		ConfMan.flushToDisk();
	}
}

void PluginManagerUncached::loadFirstPlugin() {
	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false);

	// let's try to find one we can load
	for (_currentPlugin = _allEnginePlugins.begin(); _currentPlugin != _allEnginePlugins.end(); ++_currentPlugin) {
		if ((*_currentPlugin)->loadPlugin()) {
			addToPluginsInMemList(*_currentPlugin);
			break;
		}
	}
}

bool PluginManagerUncached::loadNextPlugin() {
	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false);

	for (++_currentPlugin; _currentPlugin != _allEnginePlugins.end(); ++_currentPlugin) {
		if ((*_currentPlugin)->loadPlugin()) {
			addToPluginsInMemList(*_currentPlugin);
			return true;
		}
	}
	return false;	// no more in list
}

/**
 * Used by only the cached plugin manager. The uncached manager can only have
 * one plugin in memory at a time.
 **/
void PluginManager::loadAllPlugins() {
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		PluginList pl((*pp)->getPlugins());
		Common::for_each(pl.begin(), pl.end(), Common::bind1st(Common::mem_fun(&PluginManager::tryLoadPlugin), this));
	}
}

void PluginManager::unloadAllPlugins() {
	for (int i = 0; i < PLUGIN_TYPE_MAX; i++)
		unloadPluginsExcept((PluginType)i, NULL);
}

void PluginManager::unloadPluginsExcept(PluginType type, const Plugin *plugin, bool deletePlugin /*=true*/) {
	Plugin *found = NULL;
	for (PluginList::iterator p = _pluginsInMem[type].begin(); p != _pluginsInMem[type].end(); ++p) {
		if (*p == plugin) {
			found = *p;
		} else {
			(*p)->unloadPlugin();
			if (deletePlugin)
				delete *p;
		}
	}
	_pluginsInMem[type].clear();
	if (found != NULL) {
		_pluginsInMem[type].push_back(found);
	}
}

/*
 * Used only by the cached plugin manager since it deletes the plugin.
 */
bool PluginManager::tryLoadPlugin(Plugin *plugin) {
	assert(plugin);
	// Try to load the plugin
	if (plugin->loadPlugin()) {
		addToPluginsInMemList(plugin);
		return true;
	} else {
		// Failed to load the plugin
		delete plugin;
		return false;
	}
}

/**
 * Add to the list of plugins loaded in memory.
 */
void PluginManager::addToPluginsInMemList(Plugin *plugin) {
	bool found = false;
	// The plugin is valid, see if it provides the same module as an
	// already loaded one and should replace it.

	PluginList::iterator pl = _pluginsInMem[plugin->getType()].begin();
	while (!found && pl != _pluginsInMem[plugin->getType()].end()) {
		if (!strcmp(plugin->getName(), (*pl)->getName())) {
			// Found a duplicated module. Replace the old one.
			found = true;
			delete *pl;
			*pl = plugin;
			debug(1, "Replaced the duplicated plugin: '%s'", plugin->getName());
		}
		pl++;
	}

	if (!found) {
		// If it provides a new module, just add it to the list of known plugins in memory.
		_pluginsInMem[plugin->getType()].push_back(plugin);
	}
}

// Engine plugins

#include "engines/metaengine.h"

namespace Common {
DECLARE_SINGLETON(EngineManager);
}

/**
 * This function works for both cached and uncached PluginManagers.
 * For the cached version, most of the logic here will short circuit.
 *
 * For the uncached version, we first try to find the plugin using the gameId
 * and only if we can't find it there, we loop through the plugins.
 **/
GameDescriptor EngineManager::findGame(const Common::String &gameName, const EnginePlugin **plugin) const {
	GameDescriptor result;

	// First look for the game using the plugins in memory. This is critical
	// for calls coming from inside games
	result = findGameInLoadedPlugins(gameName, plugin);
	if (!result.gameid().empty()) {
		return result;
	}

	// Now look for the game using the gameId. This is much faster than scanning plugin
	// by plugin
	if (PluginMan.loadPluginFromGameId(gameName))  {
		result = findGameInLoadedPlugins(gameName, plugin);
		if (!result.gameid().empty()) {
			return result;
		}
	}

	// We failed to find it using the gameid. Scan the list of plugins
	PluginMan.loadFirstPlugin();
	do {
		result = findGameInLoadedPlugins(gameName, plugin);
		if (!result.gameid().empty()) {
			// Update with new plugin file name
			PluginMan.updateConfigWithFileName(gameName);
			break;
		}
	} while (PluginMan.loadNextPlugin());

	return result;
}

/**
 * Find the game within the plugins loaded in memory
 **/
GameDescriptor EngineManager::findGameInLoadedPlugins(const Common::String &gameName, const EnginePlugin **plugin) const {
	// Find the GameDescriptor for this target
	const EnginePlugin::List &plugins = getPlugins();
	GameDescriptor result;

	if (plugin)
		*plugin = 0;

	EnginePlugin::List::const_iterator iter;

	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		result = (**iter)->findGame(gameName.c_str());
		if (!result.gameid().empty()) {
			if (plugin)
				*plugin = *iter;
			return result;
		}
	}
	return result;
}

GameList EngineManager::detectGames(const Common::FSList &fslist) const {
	GameList candidates;
	EnginePlugin::List plugins;
	EnginePlugin::List::const_iterator iter;
	PluginManager::instance().loadFirstPlugin();
	do {
		plugins = getPlugins();
		// Iterate over all known games and for each check if it might be
		// the game in the presented directory.
		for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
			candidates.push_back((**iter)->detectGames(fslist));
		}
	} while (PluginManager::instance().loadNextPlugin());
	return candidates;
}

const EnginePlugin::List &EngineManager::getPlugins() const {
	return (const EnginePlugin::List &)PluginManager::instance().getPlugins(PLUGIN_TYPE_ENGINE);
}


// Music plugins

#include "audio/musicplugin.h"

namespace Common {
DECLARE_SINGLETON(MusicManager);
}

const MusicPlugin::List &MusicManager::getPlugins() const {
	return (const MusicPlugin::List &)PluginManager::instance().getPlugins(PLUGIN_TYPE_MUSIC);
}
