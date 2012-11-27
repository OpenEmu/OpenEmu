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

#ifndef BACKENDS_PLUGINS_ELF_PROVIDER_H
#define BACKENDS_PLUGINS_ELF_PROVIDER_H

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

#include "backends/plugins/elf/elf-loader.h"

#include "common/fs.h"

/**
 * ELFPlugin
 *
 * Objects of this class are returned when the PluginManager calls
 * getPlugins() on an ELFPluginProvider. An intermediary class for
 * dealing with plugin files, ELFPlugin is responsible for creating/destroying
 * a DLObject that handles the opening/loading/unloading of the plugin file whose
 * path in the target backend's file system is "_filename".
 */
class ELFPlugin : public DynamicPlugin {
protected:
	typedef const char *(*CharFunc)();

	DLObject *_dlHandle;
	void *_dso_handle;

	virtual VoidFunc findSymbol(const char *symbol);

public:
	ELFPlugin(const Common::String &filename) :
		DynamicPlugin(filename),
		_dlHandle(0),
		_dso_handle(0) {
	}

	virtual ~ELFPlugin() {
		if (_dlHandle)
			unloadPlugin();
	}

	virtual DLObject *makeDLObject() = 0;

	virtual bool loadPlugin();
	virtual void unloadPlugin();
	void trackSize();
};

template<class T>
class TemplatedELFPlugin : public ELFPlugin {
public:
	TemplatedELFPlugin(const Common::String &filename) :
		ELFPlugin(filename) {
	}

	virtual DLObject *makeDLObject() {
		return new T();
	}
};


class ELFPluginProvider : public FilePluginProvider {
protected:
	virtual Plugin *createPlugin(const Common::FSNode &node) const = 0;
	virtual PluginList getPlugins();

	bool isPluginFilename(const Common::FSNode &node) const;
};

#endif // defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

#endif /* BACKENDS_PLUGINS_ELF_PROVIDER_H */
