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

#ifndef BACKENDS_LOG_LOG_H
#define BACKENDS_LOG_LOG_H

#include "common/scummsys.h"

class OSystem;

namespace Common {
class WriteStream;
} // End of namespace Common

namespace Backends {
namespace Log {

/**
 * Log file writer.
 *
 * This can be used by the backends to implement file logging functionality.
 */
class Log {
public:
	/**
	 * Constructor for the logger object.
	 *
	 * @param system The OSystem instance to use. Must be non-null.
	 */
	Log(OSystem *system);
	~Log() { close(); }

	/**
	 * Opens a new log file.
	 *
	 * The previous log, which was handled by this logger, will be closed
	 * before the new stream is associated.
	 *
	 * The current implemention will always call flush after data is written
	 * to the log file. It might thus be wise to pass an unbuffered write
	 * stream here to avoid unnecessary overhead.
	 * @see Common::WriteStream::flush
	 *
	 * Calling open with stream being 0 is valid and will result in the same
	 * behavior as calling close, but it may have additional overhead.
	 * @see close
	 *
	 * This function will output information about the ScummVM version and
	 * the features built into ScummVM automatically. It will also add a short
	 * notice to indicate that the log was opened successfully.
	 *
	 * @param stream Stream where to output the log contents.
	 *               Note that the stream will be deleted by the logger.
	 */
	void open(Common::WriteStream *stream);

	/**
	 * Closes the current log file.
	 *
	 * This function will output a line saying that the log was closed
	 * successfully. This can be used to check whether a log is incomplete
	 * because of whatever reasons.
	 */
	void close();

	/**
	 * Prints a message to the log stream.
	 *
	 * This has optional support to output a timestamp on every new line.
	 * The timestamp will look like: "[YYYY-MM-DD HH:MM:SS] ".
	 * Printing of a timestamp is done by default.
	 *
	 * It might be noteworthy that this function does not append a new line
	 * to the given message.
	 *
	 * In case no stream is associated with this logger, this function will
	 * quit immediatly.
	 *
	 * @param message            The message to write.
	 * @param printTimeOnNewline Whether to print a timestamp on the start of
	 *                           a new line.
	 */
	void print(const char *message, const bool printTimeOnNewline = true);
private:
	/**
	 * Prints a time stamp in the form: "[YYYY-MM-DD HH:MM:SS] ".
	 */
	void printTimeStamp();

	/**
	 * The OSystem instance used to query data like the time.
	 */
	OSystem *_system;

	/**
	 * Where to write the output too.
	 */
	Common::WriteStream *_stream;

	/**
	 * Whether we are at the start of a line.
	 */
	bool _startOfLine;
};

} // End of namespace Log
} // End of namespace Backends

#endif
