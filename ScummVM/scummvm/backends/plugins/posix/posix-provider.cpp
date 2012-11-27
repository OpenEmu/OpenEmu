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

#if defined(DYNAMIC_MODULES) && defined(POSIX)

#include "backends/plugins/posix/posix-provider.h"
#include "backends/plugins/dynamic-plugin.h"
#include "common/fs.h"

#include <dlfcn.h>


class POSIXPlugin : public DynamicPlugin {
protected:
	void *_dlHandle;

	virtual VoidFunc findSymbol(const char *symbol) {
		void *func = dlsym(_dlHandle, symbol);
		if (!func)
			warning("Failed loading symbol '%s' from plugin '%s' (%s)", symbol, _filename.c_str(), dlerror());

		// FIXME HACK: This is a HACK to circumvent a clash between the ISO C++
		// standard and POSIX: ISO C++ disallows casting between function pointers
		// and data pointers, but dlsym always returns a void pointer. For details,
		// see e.g. <http://www.trilithium.com/johan/2004/12/problem-with-dlsym/>.
		assert(sizeof(VoidFunc) == sizeof(func));
		VoidFunc tmp;
		memcpy(&tmp, &func, sizeof(VoidFunc));
		return tmp;
	}

public:
	POSIXPlugin(const Common::String &filename)
		: DynamicPlugin(filename), _dlHandle(0) {}

	bool loadPlugin() {
		assert(!_dlHandle);
		_dlHandle = dlopen(_filename.c_str(), RTLD_LAZY);

		if (!_dlHandle) {
			warning("Failed loading plugin '%s' (%s)", _filename.c_str(), dlerror());
			return false;
		}

		return DynamicPlugin::loadPlugin();
	}

	void unloadPlugin() {
		DynamicPlugin::unloadPlugin();
		if (_dlHandle) {
			if (dlclose(_dlHandle) != 0)
				warning("Failed unloading plugin '%s' (%s)", _filename.c_str(), dlerror());
			_dlHandle = 0;
		}
	}
};


Plugin *POSIXPluginProvider::createPlugin(const Common::FSNode &node) const {
	return new POSIXPlugin(node.getPath());
}


#endif // defined(DYNAMIC_MODULES) && defined(POSIX)
