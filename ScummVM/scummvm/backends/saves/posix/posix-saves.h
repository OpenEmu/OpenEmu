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

#if !defined(BACKEND_POSIX_SAVES_H) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
#define BACKEND_POSIX_SAVES_H

#include "backends/saves/default/default-saves.h"

#if defined(POSIX) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
/**
 * Customization of the DefaultSaveFileManager for POSIX platforms.
 * The only two differences are that the default constructor sets
 * up the savepath based on HOME, and that checkPath tries to
 * create the savedir, if missing, via the mkdir() syscall.
 */
class POSIXSaveFileManager : public DefaultSaveFileManager {
public:
	POSIXSaveFileManager();

protected:
	/**
	 * Checks the given path for read access, existence, etc.
	 * In addition, tries to create a missing savedir, if possible.
	 * Sets the internal error and error message accordingly.
	 */
	virtual void checkPath(const Common::FSNode &dir);
};
#endif

#endif
