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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * LGPL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef KYRA_SOUND_ADLIB_H
#define KYRA_SOUND_ADLIB_H

#include "kyra/sound.h"

#include "common/mutex.h"

namespace Kyra {
class AdLibDriver;

/**
 * AdLib implementation of the sound output device.
 *
 * It uses a special sound file format special to
 * Dune II, Kyrandia 1 and 2. While Dune II and
 * Kyrandia 1 are using exact the same format, the
 * one of Kyrandia 2 slightly differs.
 *
 * See AdLibDriver for more information.
 * @see AdLibDriver
 */
class SoundAdLibPC : public Sound {
public:
	SoundAdLibPC(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundAdLibPC();

	virtual kType getMusicType() const { return kAdLib; }

	virtual bool init();
	virtual void process();

	virtual void updateVolumeSettings();

	virtual void initAudioResourceInfo(int set, void *info);
	virtual void selectAudioResourceSet(int set);
	virtual bool hasSoundFile(uint file) const;
	virtual void loadSoundFile(uint file);
	virtual void loadSoundFile(Common::String file);

	virtual void playTrack(uint8 track);
	virtual void haltTrack();
	virtual bool isPlaying() const;

	virtual void playSoundEffect(uint8 track, uint8 volume = 0xff);

	virtual void beginFadeOut();

	virtual int checkTrigger();
	virtual void resetTrigger();
private:
	void internalLoadFile(Common::String file);

	void play(uint8 track, uint8 volume);

	const SoundResourceInfo_PC *res() const {return _resInfo[_currentResourceSet]; }
	SoundResourceInfo_PC *_resInfo[3];
	int _currentResourceSet;

	AdLibDriver *_driver;

	int _version;
	uint8 _trackEntries[500];
	uint8 *_soundDataPtr;
	int _sfxPlayingSound;

	Common::String _soundFileLoaded;

	int _numSoundTriggers;
	const int *_soundTriggers;

	static const int _kyra1NumSoundTriggers;
	static const int _kyra1SoundTriggers[];
};

} // End of namespace Kyra

#endif
