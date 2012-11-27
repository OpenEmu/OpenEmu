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


// This section can only be included once
#ifndef PSP_TRACE_H
#define PSP_TRACE_H

#include "common/str.h"

#define __PSP_PRINT_TO_FILE_AND_SCREEN__

/* Choose to print to file/screen/both */
#ifdef __PSP_PRINT_TO_FILE__
	#define __PSP_PRINT__(format,...)			PspDebugTrace(false, format, ## __VA_ARGS__)
#elif defined(__PSP_PRINT_TO_FILE_AND_SCREEN__)
	#define __PSP_PRINT__(format,...)			PspDebugTrace(true, format, ## __VA_ARGS__)
#else /* default - print to screen */
	#define __PSP_PRINT__(format,...)			fprintf(stderr, format, ## __VA_ARGS__)
#endif /* PSP_PRINT_TO_FILE/SCREEN */

/* Error function - always print to file as well */
#define PSP_ERROR(format,...)					PspDebugTrace(true, "Error in %s: " format, __PRETTY_FUNCTION__, ## __VA_ARGS__)

/* Do the indent */
#define __PSP_INDENT__							for(int _i=psp_debug_indent; _i>0; _i--) \
													__PSP_PRINT__( "   ")

/* always print */
#define PSP_INFO_PRINT(format,...)				__PSP_PRINT__(format, ## __VA_ARGS__)
/* always print, with indent */
#define PSP_INFO_PRINT_INDENT(format,...)		{ __PSP_INDENT__; \
												__PSP_PRINT__(format, ## __VA_ARGS__); }

void PspDebugTrace(bool alsoToScreen, const char *format, ...);
void mipsBacktrace(uint32 levels, void **addresses);

extern int psp_debug_indent;

// We use this class to print out function calls on the stack in an easy way.
//

class PSPStackDebugFuncs {
    Common::String _name;

public:
	PSPStackDebugFuncs(const char *name) : _name(name) {
		PSP_INFO_PRINT_INDENT("++ %s\n", _name.c_str());
		psp_debug_indent++;
    }

	~PSPStackDebugFuncs() {
		psp_debug_indent--;
		if (psp_debug_indent < 0)
			PSP_ERROR("debug indent < 0\n");
		PSP_INFO_PRINT_INDENT("-- %s\n", _name.c_str());
	}
};

#endif /* PSP_TRACE_H */




// From here on, we allow multiple redefinitions

// Use these defines for debugging

//#define __PSP_PRINT_TO_FILE__
//#define __PSP_PRINT_TO_FILE_AND_SCREEN__
//#define __PSP_DEBUG_FUNCS__	/* can put this locally too */
//#define __PSP_DEBUG_PRINT__

#undef PSP_DEBUG_PRINT
#undef PSP_DEBUG_PRINT_FUNC
#undef PSP_DEBUG_PRINT_SAMELN
#undef PSP_DEBUG_DO
#undef DEBUG_ENTER_FUNC
#undef DEBUG_EXIT_FUNC

#ifdef __PSP_DEBUG_PRINT__
/* printf with indents */
#define PSP_DEBUG_PRINT_SAMELN(format,...)	__PSP_PRINT__(format, ## __VA_ARGS__)
#define PSP_DEBUG_PRINT(format,...)			PSP_INFO_PRINT_INDENT(format, ## __VA_ARGS__)
#define PSP_DEBUG_PRINT_FUNC(format,...)	{ __PSP_INDENT__; \
												__PSP_PRINT__("In %s: " format, __PRETTY_FUNCTION__, ## __VA_ARGS__); }
#define PSP_DEBUG_DO(x)						(x)

#else	/* no debug print */
	#define PSP_DEBUG_PRINT_SAMELN(format,...)
	#define PSP_DEBUG_PRINT(format,...)
	#define PSP_DEBUG_PRINT_FUNC(format,...)
	#define PSP_DEBUG_DO(x)
#endif /* __PSP_DEBUG_PRINT__ */

/* We don't need anything but this line at the beginning of each function to debug function calls */
/* Debugging function calls */
#ifdef __PSP_DEBUG_FUNCS__
	#define DEBUG_ENTER_FUNC()		volatile PSPStackDebugFuncs __foo(__PRETTY_FUNCTION__)
#else /* Don't debug function calls */
	#define DEBUG_ENTER_FUNC()
#endif /* __PSP_DEBUG_FUNCS__ */

// Undef the main defines for next time
#undef __PSP_PRINT_TO_FILE__
#undef __PSP_PRINT_TO_FILE_AND_SCREEN__
#undef __PSP_DEBUG_FUNCS__
#undef __PSP_DEBUG_PRINT__
