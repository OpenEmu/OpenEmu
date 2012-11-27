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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_WINTYPES_H
#define WINTERMUTE_WINTYPES_H

#include "common/scummsys.h"

namespace Wintermute {

#define BYTETORGBA(r,g,b,a) ((uint32)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

#define RGBCOLGetB(rgb)  ((byte )(rgb))
#define RGBCOLGetG(rgb)  ((byte )(((uint16)(rgb)) >> 8))
#define RGBCOLGetR(rgb)  ((byte )((rgb)>>16))
#define RGBCOLGetA(rgb)  ((byte )((rgb)>>24))

#define DID_SUCCEED(hr) ((bool)(hr))
#define DID_FAIL(hr) (!((bool)(hr)))

#define STATUS_OK       (true)
#define STATUS_FAILED   (false)

#define MAX_PATH_LENGTH 512

} // end of namespace Wintermute

#endif
