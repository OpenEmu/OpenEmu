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

#include <sifman.h>
#include <sifrpc.h>
#include <thbase.h>
#include <stdio.h>
#include "codyvdfs.h"
#include "../common/codyvdirx.h"

struct t_SifRpcDataQueue qd;
struct t_SifRpcServerData sd0;
static uint8 rpcBuffer[64];

void *rpcServer(int func, void *data, int size);
void rpcThread(void *param);

int initRpc(void) {
	iop_thread_t thread;
	int tid;

	thread.attr = TH_C;
	thread.thread = rpcThread;
	thread.priority = 40;
	thread.stacksize = 0x1000;
	thread.attr = 0;

	tid = CreateThread(&thread);
	if (tid >= 0)
		StartThread(tid, 0);
	else {
		printf("Unable to start RPC Thread!\n");
		return -1;
	}
	return 0;
}

void rpcThread(void *param) {
	SifInitRpc(0);
	SifSetRpcQueue(&qd, GetThreadId());
	SifRegisterRpc(&sd0, CDVDFS_IRX_ID, rpcServer, (void *)rpcBuffer, 0, 0, &qd);
	SifRpcLoop(&qd);
}

void *rpcReadClock(void *data) {
	CdReadClock((cd_clock_t *)data);
	return data;
}

void *driveStop(void *data) {
	if (CdStop() == 1) {
		if (CdSync(0) == 0) {
			*(int*)data = CdGetError();
		} else
			*(int*)data = -0x100;
	} else
		*(int*)data = -0x101;
	return data;
}

void *driveStandby(void *data) {
	int type;
	if (CdStandby() == 1) {
		if (CdSync(0) == 0) {
			*(int*)data = CdGetError();
		} else
			*(int*)data = -0x100;
	} else
		*(int*)data = -0x101;

	do {	// wait until drive detected disc type
		type = CdGetDiskType();
		if (DISC_NOT_READY(type))
			DelayThread(10 * 1000);
	} while (DISC_NOT_READY(type));
	printf("Standby: Disc type: %02X\n", type);

	return data;
}

void *rpcServer(int func, void *data, int size) {
	switch (func) {
		case READ_RTC:
			return rpcReadClock(data);
		case DRIVE_STOP:
			return driveStop(data);
		case DRIVE_STANDBY:
			return driveStandby(data);
		default:
			printf("Unknown RPC command %d\n", func);
			return NULL;
	}
	return NULL;
}
