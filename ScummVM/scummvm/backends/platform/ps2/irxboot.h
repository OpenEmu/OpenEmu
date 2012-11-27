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

#ifndef __IRXBOOT_H__
#define __IRXBOOT_H__

#include "common/scummsys.h"

enum IrxFlags {
	BIOS = 0,
	SYSTEM = 1,
	USB = 2,
	HDD = 3,
	NET = 4,
	TYPEMASK = 7,

	OPTIONAL = 8,
	DEPENDANCY = 16,
	NOT_HOST = 32
};

enum IrxPurpose {
	NOTHING,
	HDD_DRIVER,
	USB_DRIVER,
	MOUSE_DRIVER,
	KBD_DRIVER,
	MASS_DRIVER,
	NET_DRIVER
};

enum IrxLocation {
	IRX_BUFFER,
	IRX_FILE
};

/*
enum BootDevice {
	HOST = 0,
	CDROM,
	MASS,
	OTHER
};
*/

struct IrxFile {
	const char *name;
	int flags;
	IrxPurpose purpose;
	const char *args;
	int argSize;
};

struct IrxReference {
	IrxFile *fileRef;
	IrxLocation loc;
	char *path;
	void *buffer;
	uint32 size;
	uint32 argSize;
	const char *args;
	int errorCode;
};

int loadIrxModules(int device, const char *irxPath, IrxReference **modules);

#endif // __IRXBOOT_H__
