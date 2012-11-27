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

#ifndef TINSEL_TOKEN_H
#define TINSEL_TOKEN_H

#include "tinsel/dw.h"

namespace Tinsel {

// Fixed tokens

enum {
	TOKEN_CONTROL = 0,
	TOKEN_LEAD, // = TOKEN_CONTROL + 1
	TOKEN_LEFT_BUT = TOKEN_LEAD + MAX_MOVERS,

	NUMTOKENS // = TOKEN_LEFT_BUT + 1
};

// Token functions

void GetControlToken();
void FreeControlToken();

void GetToken(int which);
void FreeToken(int which);

void FreeAllTokens();
bool TestToken(int which);


} // End of namespace Tinsel

#endif		// TINSEL_TOKEN_H
