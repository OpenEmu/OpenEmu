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

// #define __USE_LIBMC__
#include "backends/fs/ps2/ps2-fs-factory.h"

enum PS2Device {
	CD_DEV = 0,
	HD_DEV,
	USB_DEV,
	HOST_DEV,
	MC_DEV,
	ERR_DEV = -1
};

inline PS2Device _getDev(const char *s) {
#if 0
	if (s==NULL || strlen(s) < 3)
		return -1;
#endif

	if (s[0] == 'c')
		return CD_DEV;
	else if (s[0] == 'p')
		return HD_DEV;
	else if (strncmp(s, "ma", 2) == 0)
		return USB_DEV;
	else if (s[0] == 'h')
		return HOST_DEV;
	else if (strncmp(s, "mc", 2) == 0)
		return MC_DEV;
	else
		return ERR_DEV; // -1;
}

inline PS2Device _getDev(Common::String& cs) {
	const char *s = cs.c_str();
	return _getDev(s);
}

inline PS2Device _getDev(Common::FSNode& n) {
	const char *s = n.getPath().c_str();
	return _getDev(s);
}
