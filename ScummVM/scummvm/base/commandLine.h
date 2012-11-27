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

#ifndef COMMON_COMMAND_LINE_H
#define COMMON_COMMAND_LINE_H

#include "common/hash-str.h"

namespace Common {
class Error;
class String;
}

namespace Base {

/**
 * Register various defaults with the ConfigManager.
 */
void registerDefaults();

/**
 * Parse the command line for options and a command; the options
 * are stored in the map 'settings, the command (if any) is returned.
 */
Common::String parseCommandLine(Common::StringMap &settings, int argc, const char * const *argv);

/**
 * Process the command line options and arguments.
 * Returns true if everything was handled and ScummVM should quit
 * (e.g. because "--help" was specified, and handled).
 *
 * @param[in] command	the command as returned by parseCommandLine
 * @param[in] settings	the settings as returned by parseCommandLine
 * @param[out] err		indicates whether any error occurred, and which
 * @return true if the command was completely processed and ScummVM should quit, false otherwise
 */
bool processSettings(Common::String &command, Common::StringMap &settings, Common::Error &err);

} // End of namespace Base

#endif
