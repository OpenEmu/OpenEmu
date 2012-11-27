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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/kyra_rpg.h"
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

void KyraRpgEngine::enableSysTimer(int sysTimer) {
	if (sysTimer != 2)
		return;

	for (int i = 0; i < getNumClock2Timers(); i++)
		_timer->pauseSingleTimer(getClock2Timer(i), false);
}

void KyraRpgEngine::disableSysTimer(int sysTimer) {
	if (sysTimer != 2)
		return;

	for (int i = 0; i < getNumClock2Timers(); i++)
		_timer->pauseSingleTimer(getClock2Timer(i), true);
}

void KyraRpgEngine::enableTimer(int id) {
	_timer->enable(id);
	_timer->setCountdown(id, _timer->getDelay(id));
}

void KyraRpgEngine::timerProcessDoors(int timerNum) {
	for (int i = 0; i < 3; i++) {
		uint16 b = _openDoorState[i].block;
		if (!b)
			continue;

		int v = _openDoorState[i].state;
		int c = _openDoorState[i].wall;

		_levelBlockProperties[b].walls[c] += v;
		_levelBlockProperties[b].walls[c ^ 2] += v;

		int snd = 3;
		int flg = _wllWallFlags[_levelBlockProperties[b].walls[c]];
		if (flg & 0x20)
			snd = 5;
		else if (v == -1)
			snd = 4;

		if (_flags.gameID == GI_LOL) {
			if (!(_updateFlags & 1)) {
				snd_processEnvironmentalSoundEffect(snd + 28, b);
				if (!checkSceneUpdateNeed(b))
					updateEnvironmentalSfx(0);
			}
		} else {
			checkSceneUpdateNeed(b);
			updateEnvironmentalSfx(snd);
		}

		if (flg & 0x30)
			_openDoorState[i].block = 0;
	}
}

} // namespace Kyra

#endif
