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

#include "agi/agi.h"

namespace Agi {

int AgiBase::getflag(int n) {
	uint8 *set = (uint8 *)&_game.flags;

	set += n >> 3;
	return (*set & (1 << (n & 0x07))) != 0;
}

void AgiBase::setflag(int n, int v) {
	uint8 *set = (uint8 *)&_game.flags;

	set += n >> 3;
	if (v)
		*set |= 1 << (n & 0x07);	// set bit
	else
		*set &= ~(1 << (n & 0x07));	// clear bit
}

void AgiBase::flipflag(int n) {
	uint8 *set = (uint8 *)&_game.flags;

	set += n >> 3;
	*set ^= 1 << (n & 0x07);	// flip bit
}

void AgiEngine::setvar(int var, int val) {
	_game.vars[var] = val;

	if (var == vVolume) {
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, val * 17);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, val * 17);
	}
}

int AgiEngine::getvar(int var) {
	return _game.vars[var];
}

void AgiEngine::decrypt(uint8 *mem, int len) {
	const uint8 *key;
	int i;

	key = (getFeatures() & GF_AGDS) ? (const uint8 *)CRYPT_KEY_AGDS
	                   : (const uint8 *)CRYPT_KEY_SIERRA;

	for (i = 0; i < len; i++)
		*(mem + i) ^= *(key + (i % 11));
}

} // End of namespace Agi
