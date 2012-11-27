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

// stdafx.h : Includedatei für Standardsystem-Includedateien,
// oder projektspezifische Includedateien, die häufig benutzt, aber
// in unregelmäßigen Abständen geändert werden.
//
#ifndef SKYCPT_STDAFX_H
#define SKYCPT_STDAFX_H

#ifdef _MSC_VER
#pragma once

#include <iostream>
#include <tchar.h>
#endif

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct CptObj {
	uint16 *data;
	uint32 len;
	char *dbgName;
	uint16 type;
	//uint16 id;
};

#endif // __STDAFX_H__
