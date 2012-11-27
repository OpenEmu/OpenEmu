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

#if defined(DYNAMIC_MODULES) && defined(__PLAYSTATION2__)

#include "backends/plugins/ps2/ps2-provider.h"
#include "backends/plugins/elf/mips-loader.h"

class PS2DLObject : public MIPSDLObject {
protected:

	virtual void flushDataCache(void *, uint32) const {
		FlushCache(0);
		FlushCache(2);
	}
};

Plugin *PS2PluginProvider::createPlugin(const Common::FSNode &node) const {
	return new TemplatedELFPlugin<PS2DLObject>(node.getPath());
}

#endif // defined(DYNAMIC_MODULES) && defined(__PLAYSTATION2__)
