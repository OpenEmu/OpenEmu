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

#ifndef SCUMM_PLAYER_V3A_H
#define SCUMM_PLAYER_V3A_H

#include "common/scummsys.h"
#include "scumm/music.h"
#include "scumm/player_mod.h"

class Mixer;

namespace Scumm {

class ScummEngine;

/**
 * Scumm V3 Amiga sound/music driver.
 */
class Player_V3A : public MusicEngine {
public:
	Player_V3A(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_V3A();

	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer();
	virtual int  getSoundStatus(int sound) const;

private:
	enum {
		V3A_MAXMUS = 24,
		V3A_MAXSFX = 16
	};

	struct musChan {
		int id;
		int dur;
	};

	struct sfxChan {
		int id;
		int dur;
		uint32 rate;
		int32 delta;
	};

	struct instData {
		char *_idat[6];
		uint16 _ilen[6];
		char *_ldat[6];
		uint16 _llen[6];
		uint16 _oct[6];
		int16 _pitadjust;
	};

	ScummEngine *_vm;
	Player_MOD *_mod;

	musChan _mus[V3A_MAXMUS];
	sfxChan _sfx[V3A_MAXSFX];

	int _curSong;
	uint8 *_songData;
	uint16 _songPtr;
	uint16 _songDelay;
	int _music_timer;
	bool _isinit;

	instData **_wavetable;

	int getMusChan (int id = 0) const;
	int getSfxChan (int id = 0) const;
	static void update_proc(void *param);
	void playMusic();
};

} // End of namespace Scumm

#endif
