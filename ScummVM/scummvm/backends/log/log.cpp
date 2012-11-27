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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "backends/log/log.h"

#include "common/stream.h"
#include "common/str.h"
#include "common/system.h"

#include "base/version.h"

namespace Backends {
namespace Log {

Log::Log(OSystem *system)
    : _system(system), _stream(0), _startOfLine(true) {
	assert(system);
}

void Log::open(Common::WriteStream *stream) {
	// Close the previous log
	close();

	_stream = stream;

	// Output information about the ScummVM version at the start of the log
	// file
	print(gScummVMFullVersion);
	print("\n");
	print(gScummVMFeatures);
	print("\n");
	print("--- Log opened.\n");
	_startOfLine = true;
}

void Log::close() {
	if (_stream) {
		// Output a message to indicate that the log was closed successfully
		print("--- Log closed successfully.\n");

		delete _stream;
		_stream = 0;
	}
}

void Log::print(const char *message, const bool printTime) {
	if (!_stream)
		return;

	while (*message) {
		if (_startOfLine) {
			_startOfLine = false;
			if (printTime)
				printTimeStamp();
		}

		const char *msgStart = message;
		// scan for end of line/string
		while (*message && *message != '\n')
			++message;

		if (*message == '\n') {
			++message;
			_startOfLine = true;
		}

		// TODO: It might be wise to check for write errors and/or incomplete
		// writes here, since losing certain bits of the log is not nice.
		_stream->write(msgStart, message - msgStart);
	}

	_stream->flush();
}

void Log::printTimeStamp() {
	TimeDate date;
	_system->getTimeAndDate(date);

	_stream->writeString(Common::String::format("[%d-%02d-%02d %02d:%02d:%02d] ",
	                     date.tm_year + 1900, date.tm_mon, date.tm_mday,
	                     date.tm_hour, date.tm_min, date.tm_sec));
}

} // End of namespace Log
} // End of namespace Backends
