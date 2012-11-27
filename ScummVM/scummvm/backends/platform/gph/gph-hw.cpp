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
 * GP2X Wiz: Hardware Stuff.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/gph/gph-hw.h"

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>

namespace WIZ_HW {

enum {
	VOLUME_NOCHG            = 0,
	VOLUME_DOWN             = 1,
	VOLUME_UP               = 2,
	VOLUME_CHANGE_RATE      = 8,
	VOLUME_MIN              = 0,
	VOLUME_INITIAL          = 60,
	VOLUME_MAX              = 100
};

int volumeLevel = VOLUME_INITIAL;

void deviceInit() {
}

void deviceDeinit() {
}

void mixerMoveVolume(int direction) {
	if (volumeLevel <= 10) {
		if (direction == VOLUME_UP)   volumeLevel += VOLUME_CHANGE_RATE / 2;
		if (direction == VOLUME_DOWN) volumeLevel -= VOLUME_CHANGE_RATE / 2;
	} else {
		if (direction == VOLUME_UP)   volumeLevel += VOLUME_CHANGE_RATE;
		if (direction == VOLUME_DOWN) volumeLevel -= VOLUME_CHANGE_RATE;
	}

	if (volumeLevel < VOLUME_MIN) volumeLevel = VOLUME_MIN;
	if (volumeLevel > VOLUME_MAX) volumeLevel = VOLUME_MAX;

	unsigned long soundDev = open("/dev/mixer", O_RDWR);

	if (soundDev) {
		int vol = ((volumeLevel << 8) | volumeLevel);
		ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
		close(soundDev);
	}
}

} /* namespace WIZ_HW */

namespace GPH {

enum {
	/* Touchscreen TapMode */
	TAPMODE_LEFT        = 0,
	TAPMODE_RIGHT       = 1,
	TAPMODE_HOVER       = 2
};

int tapmodeLevel = TAPMODE_LEFT;

void ToggleTapMode() {
	if (tapmodeLevel == TAPMODE_LEFT) {
		tapmodeLevel = TAPMODE_RIGHT;
	} else if (tapmodeLevel == TAPMODE_RIGHT) {
		tapmodeLevel = TAPMODE_HOVER;
	} else if (tapmodeLevel == TAPMODE_HOVER) {
		tapmodeLevel = TAPMODE_LEFT;
	} else {
		tapmodeLevel = TAPMODE_LEFT;
	}
}

} /* namespace GPH */
