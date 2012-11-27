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

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include "eecodyvdfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SifRpcClientData_t cd0;
static unsigned sbuff[64] __attribute__((aligned (64)));

int driveStopped;

int initCdvdFs(void) {
	int res;
	while(((res = SifBindRpc(&cd0, CDVDFS_IRX_ID, 0)) >= 0) && (cd0.server == NULL))
		nopdelay();
	driveStopped = 0;
	return res;
}

void readRTC(struct CdClock *dest) {
	SifCallRpc(&cd0, READ_RTC, 0, (void *)sbuff, 0, (void *)sbuff, 8, 0, 0);
	memcpy(dest, sbuff, 8);
}

int driveStop(void) {
	if (driveStopped)
		return 0;
	SifCallRpc(&cd0, DRIVE_STOP, 0, (void *)sbuff, 0, (void *)sbuff, 4, 0, 0);
	if (*(int *)sbuff == 0)
		driveStopped = 1;
	return *(int *)sbuff;
}

int driveStandby(void) {
	if (!driveStopped)
		return 0;
	SifCallRpc(&cd0, DRIVE_STANDBY, 0, (void *)sbuff, 0, (void *)sbuff, 4, 0, 0);
	if (*(int *)sbuff == 0)
		driveStopped = 0;
	return *(int *)sbuff;
}
