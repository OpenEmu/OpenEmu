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

#if !defined(BACKEND_SAVES_DEFAULT_H) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
#define BACKEND_SAVES_DEFAULT_H

#include "common/scummsys.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/fs.h"

/**
 * Provides a default savefile manager implementation for common platforms.
 */
class DefaultSaveFileManager : public Common::SaveFileManager {
public:
	DefaultSaveFileManager();
	DefaultSaveFileManager(const Common::String &defaultSavepath);

	virtual Common::StringArray listSavefiles(const Common::String &pattern);
	virtual Common::InSaveFile *openForLoading(const Common::String &filename);
	virtual Common::OutSaveFile *openForSaving(const Common::String &filename, bool compress = true);
	virtual bool removeSavefile(const Common::String &filename);

protected:
	/**
	 * Get the path to the savegame directory.
	 * Should only be used internally since some platforms
	 * might implement savefiles in a completely different way.
	 */
	virtual Common::String getSavePath() const;

	/**
	 * Checks the given path for read access, existence, etc.
	 * Sets the internal error and error message accordingly.
	 */
	virtual void checkPath(const Common::FSNode &dir);
};

#endif
