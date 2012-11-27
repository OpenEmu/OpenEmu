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
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_exit

#include "common/textconsole.h"
#include "common/system.h"
#include "common/str.h"

namespace Common {

static OutputFormatter s_errorOutputFormatter = 0;

void setErrorOutputFormatter(OutputFormatter f) {
	s_errorOutputFormatter = f;
}

static ErrorHandler s_errorHandler = 0;

void setErrorHandler(ErrorHandler handler) {
	s_errorHandler = handler;
}


}	// End of namespace Common


#ifndef DISABLE_TEXT_CONSOLE

void warning(const char *s, ...) {
	Common::String output;
	va_list va;

	va_start(va, s);
	output = Common::String::vformat(s, va);
	va_end(va);

	output = "WARNING: " + output + "!\n";

	if (g_system)
		g_system->logMessage(LogMessageType::kWarning, output.c_str());
	// TODO: Think of a good fallback in case we do not have
	// any OSystem yet.
}

#endif

void NORETURN_PRE error(const char *s, ...) {
	// We don't use String::vformat here, as that require
	// using the heap, and that might be impossible at this
	// point, e.g. if the error was an "out-of-memory" error.
	char buf_input[STRINGBUFLEN];
	char buf_output[STRINGBUFLEN];
	va_list va;

	// Generate the full error message
	va_start(va, s);
	vsnprintf(buf_input, STRINGBUFLEN, s, va);
	va_end(va);


	// Next, give the active engine (if any) a chance to augment the message
	if (Common::s_errorOutputFormatter) {
		(*Common::s_errorOutputFormatter)(buf_output, buf_input, STRINGBUFLEN);
	} else {
		strncpy(buf_output, buf_input, STRINGBUFLEN);
	}

	buf_output[STRINGBUFLEN - 3] = '\0';
	buf_output[STRINGBUFLEN - 2] = '\0';
	buf_output[STRINGBUFLEN - 1] = '\0';
	strcat(buf_output, "!\n");

	if (g_system)
		g_system->logMessage(LogMessageType::kError, buf_output);
	// TODO: Think of a good fallback in case we do not have
	// any OSystem yet.

	// If there is an error handler, invoke it now
	if (Common::s_errorHandler)
		(*Common::s_errorHandler)(buf_output);

	if (g_system)
		g_system->fatalError();

#if defined(SAMSUNGTV)
	// FIXME
	for (;;) {}
#else
	exit(1);
#endif
}
