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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/ps2/systemps2.h"

void OSystem_PS2::initMutexes(void) {
	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_mutexSema = CreateSema(&newSema);
	for (int i = 0; i < MAX_MUTEXES; i++) {
		_mutex[i].sema = -1;
		_mutex[i].count = _mutex[i].owner = 0;
	}
}

OSystem::MutexRef OSystem_PS2::createMutex(void) {
	WaitSema(_mutexSema);
	Ps2Mutex *mutex = NULL;
	for (int i = 0; i < MAX_MUTEXES; i++)
		if (_mutex[i].sema < 0) {
			mutex = _mutex + i;
			break;
		}
	if (mutex) {
		ee_sema_t newSema;
		newSema.init_count = 1;
		newSema.max_count = 1;
		mutex->sema = CreateSema(&newSema);
		mutex->owner = mutex->count = 0;
	} else
		printf("OSystem_PS2::createMutex: ran out of Mutex slots!\n");
	SignalSema(_mutexSema);
	return (OSystem::MutexRef)mutex;
}

void OSystem_PS2::lockMutex(MutexRef mutex) {
	WaitSema(_mutexSema);
	Ps2Mutex *sysMutex = (Ps2Mutex *)mutex;
	int tid = GetThreadId();

	assert(tid != 0);
	if (sysMutex->owner && (sysMutex->owner == tid))
		sysMutex->count++;
	else {
		SignalSema(_mutexSema);
		WaitSema(sysMutex->sema);
		WaitSema(_mutexSema);
		sysMutex->owner = tid;
		sysMutex->count = 0;
	}
	SignalSema(_mutexSema);
}

void OSystem_PS2::unlockMutex(MutexRef mutex) {
	WaitSema(_mutexSema);
	Ps2Mutex *sysMutex = (Ps2Mutex *)mutex;
	int tid = GetThreadId();

	if (sysMutex->owner && sysMutex->count && (sysMutex->owner == tid))
		sysMutex->count--;
	else {
		assert(sysMutex->count == 0);
		SignalSema(sysMutex->sema);
		sysMutex->owner = 0;
	}
	SignalSema(_mutexSema);
}

void OSystem_PS2::deleteMutex(MutexRef mutex) {
	WaitSema(_mutexSema);
	Ps2Mutex *sysMutex = (Ps2Mutex *)mutex;
	if (sysMutex->owner || sysMutex->count)
		printf("WARNING: Deleting LOCKED mutex!\n");
	DeleteSema(sysMutex->sema);
	sysMutex->sema = -1;
	SignalSema(_mutexSema);
}
