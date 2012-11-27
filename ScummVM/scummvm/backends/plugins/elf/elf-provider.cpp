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

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

#ifdef ELF_LOADER_CXA_ATEXIT
#include <cxxabi.h>
#endif

#include "backends/plugins/elf/elf-provider.h"
#include "backends/plugins/dynamic-plugin.h"
#include "backends/plugins/elf/memory-manager.h"

#include "common/debug.h"
#include "common/fs.h"

/* Note about ELF_LOADER_CXA_ATEXIT:
 *
 * consider the code:
 *
 * class Foobar {
 *   const char *work() {
 *     static String foo = "bar";
 *     return s.c_str();
 *   }
 * }
 *
 * When instantiating Foobar and calling work() for the first time the String
 * foo will be constructed. GCC automatically registers its destruction via
 * either atexit() or __cxa_atexit(). Only the latter will add information
 * about which DSO did the construction (Using &__dso_handle).
 *
 * __cxa_atexit allows plugins to reference C++ ABI symbols in the main
 * executable without code duplication (No need to link the plugin against
 * libstdc++), since we can distinguish which registered exit functions belong
 * to a specific DSO. When unloading a plugin, we just use the C++ ABI call
 * __cxa_finalize(&__dso_handle) to call all destructors of only that DSO.
 *
 * Prerequisites:
 * - The used libc needs to support __cxa_atexit
 * - -fuse-cxa-atexit in CXXFLAGS
 * - Every plugin needs its own hidden __dso_handle symbol
 *   This is automatically done via REGISTER_PLUGIN_DYNAMIC, see base/plugins.h
 *
 * When __cxa_atexit can not be used, each plugin needs to link against
 * libstdc++ to embed its own set of C++ ABI symbols. When not doing so,
 * registered destructors of already unloaded plugins will crash the
 * application upon returning from main().
 *
 * See "3.3.5 DSO Object Destruction API" of the C++ ABI
 */

DynamicPlugin::VoidFunc ELFPlugin::findSymbol(const char *symbol) {
	void *func = 0;

	if (_dlHandle)
		func = _dlHandle->symbol(symbol);

	if (!func) {
		if (!_dlHandle)
			warning("elfloader: Failed loading symbol '%s' from plugin '%s' (Handle is NULL)", symbol, _filename.c_str());
		else
			warning("elfloader: Failed loading symbol '%s' from plugin '%s'", symbol, _filename.c_str());
	}

	// FIXME HACK: This is a HACK to circumvent a clash between the ISO C++
	// standard and POSIX: ISO C++ disallows casting between function pointers
	// and data pointers, but dlsym always returns a void pointer. For details,
	// see e.g. <http://www.trilithium.com/johan/2004/12/problem-with-dlsym/>.
	assert(sizeof(VoidFunc) == sizeof(func));
	VoidFunc tmp;
	memcpy(&tmp, &func, sizeof(VoidFunc));
	return tmp;
}

 /**
  * Test the size of the plugin.
  */
void ELFPlugin::trackSize() {
	// All we need to do is create our object, track its size, then delete it
	DLObject *obj = makeDLObject();

	obj->trackSize(_filename.c_str());
	delete obj;
}

bool ELFPlugin::loadPlugin() {
	assert(!_dlHandle);

	DLObject *obj = makeDLObject();
	if (obj->open(_filename.c_str())) {
		_dlHandle = obj;
	} else {
		delete obj;
		_dlHandle = 0;
	}

	if (!_dlHandle) {
		warning("elfloader: Failed loading plugin '%s'", _filename.c_str());
		return false;
	}

	CharFunc buildDateFunc = (CharFunc)findSymbol("PLUGIN_getBuildDate");
	if (!buildDateFunc) {
		unloadPlugin();
		warning("elfloader: plugin '%s' is missing symbols", _filename.c_str());
		return false;
	}

	if (strncmp(gScummVMPluginBuildDate, buildDateFunc(), strlen(gScummVMPluginBuildDate))) {
		unloadPlugin();
		warning("elfloader: plugin '%s' has a different build date", _filename.c_str());
		return false;
	}

	bool ret = DynamicPlugin::loadPlugin();

#ifdef ELF_LOADER_CXA_ATEXIT
	if (ret) {
		// FIXME HACK: Reverse HACK of findSymbol() :P
		VoidFunc tmp;
		tmp = findSymbol("__dso_handle");
		memcpy(&_dso_handle, &tmp, sizeof(VoidFunc));
		debug(2, "elfloader: __dso_handle is %p", _dso_handle);
	}
#endif

	_dlHandle->discardSymtab();

	return ret;
}

void ELFPlugin::unloadPlugin() {
	DynamicPlugin::unloadPlugin();

	if (_dlHandle) {
#ifdef ELF_LOADER_CXA_ATEXIT
		if (_dso_handle) {
			debug(2, "elfloader: calling __cxa_finalize");
			__cxxabiv1::__cxa_finalize(_dso_handle);
			_dso_handle = 0;
		}
#endif

		if (!_dlHandle->close())
			warning("elfloader: Failed unloading plugin '%s'", _filename.c_str());

		delete _dlHandle;
		_dlHandle = 0;
	}
}

 /**
  * We override this function in FilePluginProvider to allow the single
  * plugin method to create a non-fragmenting memory allocation. We take
  * the plugins found and tell the memory manager to allocate space for
  * them.
  */
PluginList ELFPluginProvider::getPlugins() {
	PluginList pl = FilePluginProvider::getPlugins();

#if defined(UNCACHED_PLUGINS) && !defined(ELF_NO_MEM_MANAGER)
	// This static downcast is safe because all of the plugins must
	// be ELF plugins
	for (PluginList::iterator p = pl.begin(); p != pl.end(); ++p) {
		(static_cast<ELFPlugin *>(*p))->trackSize();
	}

	// The Memory Manager should now allocate space based on the information
	// it collected
	ELFMemMan.allocateHeap();
#endif

	return pl;
}

bool ELFPluginProvider::isPluginFilename(const Common::FSNode &node) const {
	// Check the plugin suffix
	Common::String filename = node.getName();

	if (!filename.hasSuffix(".PLG") && !filename.hasSuffix(".plg") &&
			!filename.hasSuffix(".PLUGIN") && !filename.hasSuffix(".plugin"))
		return false;

	return true;
}

#endif // defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)
