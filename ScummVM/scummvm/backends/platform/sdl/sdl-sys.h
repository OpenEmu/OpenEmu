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

#ifndef BACKEND_SDL_SYS_H
#define BACKEND_SDL_SYS_H

// The purpose of this header is to include the SDL headers in a uniform
// fashion, even on the Symbian port.
// Moreover, it contains a workaround for the fact that SDL_rwops.h uses
// a FILE pointer in one place, which conflicts with common/forbidden.h.
// The SDL 1.3 headers also include strings.h

#include "common/scummsys.h"

// Remove FILE override from common/forbidden.h, and replace
// it with an alternate slightly less unfriendly override.
#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_FILE)
#undef FILE
typedef struct { int FAKE; } FAKE_FILE;
#define FILE FAKE_FILE
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strcasecmp)
#undef strcasecmp
#define strcasecmp FAKE_strcasecmp
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strncasecmp)
#undef strncasecmp
#define strncasecmp FAKE_strncasecmp
#endif

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

// Finally forbid FILE again (if it was forbidden to start with)
#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_FILE)
#undef FILE
#define FILE	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strcasecmp)
#undef strcasecmp
#define strcasecmp     FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strncasecmp)
#undef strncasecmp
#define strncasecmp    FORBIDDEN_SYMBOL_REPLACEMENT
#endif


#endif
