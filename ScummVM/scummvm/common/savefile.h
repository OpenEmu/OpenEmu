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

#ifndef COMMON_SAVEFILE_H
#define COMMON_SAVEFILE_H

#include "common/noncopyable.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/str-array.h"
#include "common/error.h"

namespace Common {


/**
 * A class which allows game engines to load game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
typedef SeekableReadStream InSaveFile;

/**
 * A class which allows game engines to save game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 */
typedef WriteStream OutSaveFile;


/**
 * The SaveFileManager is serving as a factory for InSaveFile
 * and OutSaveFile objects.
 *
 * Engines and other code should use SaveFiles whenever they need to
 * store data which they need to be able to retrieve again later on --
 * i.e. typically save states, but also configuration files and similar
 * things.
 *
 * While not declared as a singleton, it is effectively used as such,
 * with OSystem::getSavefileManager returning a pointer to the single
 * SaveFileManager instances to be used.
 */
class SaveFileManager : NonCopyable {

protected:
	Error _error;
	String _errorDesc;

	/**
	 * Set some information about the last error which occurred .
	 * @param error Code identifying the last error.
	 * @param errorDesc String describing the last error.
	 */
	virtual void setError(Error error, const String &errorDesc) { _error = error; _errorDesc = errorDesc; }

public:
	virtual ~SaveFileManager() {}

	/**
	 * Clears the last set error code and string.
	 */
	virtual void clearError() { _error = kNoError; _errorDesc.clear(); }

	/**
	 * Returns the last occurred error code. If none occurred, returns kNoError.
	 *
	 * @return A value indicating the type of the last error.
	 */
	virtual Error getError() { return _error; }

	/**
	 * Returns the last occurred error description. If none occurred, returns 0.
	 *
	 * @return A string describing the last error.
	 */
	virtual String getErrorDesc() { return _errorDesc; }

	/**
	 * Returns the last occurred error description. If none occurred, returns 0.
	 * Also clears the last error state and description.
	 *
	 * @return A string describing the last error.
	 */
	virtual String popErrorDesc();

	/**
	 * Open the savefile with the specified name in the given directory for
	 * saving.
	 *
	 * Saved games are compressed by default, and engines are expected to
	 * always write compressed saves.
	 *
	 * A notable exception is if uncompressed files are needed for
	 * compatibility with games not supported by ScummVM, such as character
	 * exports from the Quest for Glory series. QfG5 is a 3D game and won't be
	 * supported by ScummVM.
	 *
	 * @param name		the name of the savefile
	 * @param compress	toggles whether to compress the resulting save file
	 * 					(default) or not.
	 * @return pointer to an OutSaveFile, or NULL if an error occurred.
	 */
	virtual OutSaveFile *openForSaving(const String &name, bool compress = true) = 0;

	/**
	 * Open the file with the specified name in the given directory for loading.
	 * @param name	the name of the savefile
	 * @return pointer to an InSaveFile, or NULL if an error occurred.
	 */
	virtual InSaveFile *openForLoading(const String &name) = 0;

	/**
	 * Removes the given savefile from the system.
	 * @param name the name of the savefile to be removed.
	 * @return true if no error occurred, false otherwise.
	 */
	virtual bool removeSavefile(const String &name) = 0;

	/**
	 * Renames the given savefile.
	 * @param oldName Old name.
	 * @param newName New name.
	 * @return true if no error occurred. false otherwise.
	 */
	virtual bool renameSavefile(const String &oldName, const String &newName);

	/**
	 * Copy the given savefile.
	 * @param oldName Old name.
	 * @param newName New name.
	 * @return true if no error occurred. false otherwise.
	 */
	virtual bool copySavefile(const String &oldName, const String &newName);

	/**
	 * Request a list of available savegames with a given DOS-style pattern,
	 * also known as "glob" in the POSIX world. Refer to the Common::matchString()
	 * function to learn about the precise pattern format.
	 * @param pattern Pattern to match. Wildcards like * or ? are available.
	 * @return list of strings for all present file names.
	 * @see Common::matchString()
	 */
	virtual StringArray listSavefiles(const String &pattern) = 0;
};

} // End of namespace Common

#endif
