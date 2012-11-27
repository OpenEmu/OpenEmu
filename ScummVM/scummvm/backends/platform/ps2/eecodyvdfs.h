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

#ifndef EECDVDFS_H
#define EECDVDFS_H

#include <tamtypes.h>
#include "iop/CoDyVDfs/common/codyvdirx.h"

struct CdClock {
	u8 stat;
	u8 second;
	u8 minute;
	u8 hour;
	u8 padding;
	u8 day;
	u8 month;
	u8 year;
} __attribute__((packed));

#ifdef __cplusplus
extern "C" {
#endif
    int initCdvdFs(void);
	void readRTC(struct CdClock *dest);
	int driveStop(void);
	int driveStandby(void);
#ifdef __cplusplus
}
#endif

#endif
