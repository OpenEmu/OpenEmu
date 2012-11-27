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

#include "common/error.h"

#include "common/translation.h"

namespace Common {

/**
 * Maps an error code to equivalent string description.
 *
 * @param errorCode error code to be converted
 * @return a pointer to string description of the error
 */
static String errorToString(ErrorCode errorCode) {
	switch (errorCode) {
	case kNoError:
		return _s("No error");
	case kNoGameDataFoundError:
		return _s("Game data not found");
	case kUnsupportedGameidError:
		return _s("Game id not supported");
	case kUnsupportedColorMode:
		return _s("Unsupported color mode");

	case kReadPermissionDenied:
		return _s("Read permission denied");
	case kWritePermissionDenied:
		return _s("Write permission denied");

	case kPathDoesNotExist:
		return _s("Path does not exist");
	case kPathNotDirectory:
		return _s("Path not a directory");
	case kPathNotFile:
		return _s("Path not a file");

	case kCreatingFileFailed:
		return _s("Cannot create file");
	case kReadingFailed:
		return _s("Reading data failed");
	case kWritingFailed:
		return _s("Writing data failed");

	case kEnginePluginNotFound:
		return _s("Could not find suitable engine plugin");
	case kEnginePluginNotSupportSaves:
		return _s("Engine plugin does not support save states");

	case kUserCanceled:
		return _s("User canceled");

	case kUnknownError:
	default:
		return _s("Unknown error");
	}
}

Error::Error(ErrorCode code)
	: _code(code), _desc(errorToString(code)) {
}

Error::Error(ErrorCode code, const String &desc)
	: _code(code), _desc(errorToString(code) + " (" + desc + ")") {
}


} // End of namespace Common
