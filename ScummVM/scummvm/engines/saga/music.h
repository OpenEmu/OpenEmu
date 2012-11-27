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

// Music class

#ifndef SAGA_MUSIC_H
#define SAGA_MUSIC_H

#include "audio/midiplayer.h"
#include "audio/midiparser.h"
#include "audio/mixer.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/flac.h"
#include "common/mutex.h"

namespace Saga {

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 0x0001,
	MUSIC_DEFAULT = 0xffff
};

class MusicDriver : public Audio::MidiPlayer {
public:
	MusicDriver();

	void play(SagaEngine *vm, ByteArray *buffer, bool loop);
	virtual void pause();
	virtual void resume();

	bool isAdlib() const { return _driverType == MT_ADLIB; }

	// FIXME
	bool isPlaying() const { return _parser && _parser->isPlaying(); }

	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);
	virtual void metaEvent(byte type, byte *data, uint16 length);

protected:
	MusicType _driverType;
	bool _isGM;
};

class Music {
public:

	Music(SagaEngine *vm, Audio::Mixer *mixer);
	~Music();
	bool isPlaying();
	bool hasDigitalMusic() { return _digitalMusic; }

	void play(uint32 resourceId, MusicFlags flags = MUSIC_DEFAULT);
	void pause();
	void resume();
	void stop();

	void setVolume(int volume, int time = 1);
	int getVolume() { return _currentVolume; }

	Common::Array<int32> _songTable;

private:
	SagaEngine *_vm;
	Audio::Mixer *_mixer;

	MusicDriver *_player;
	Audio::SoundHandle _musicHandle;
	uint32 _trackNumber;

	int _targetVolume;
	int _currentVolume;
	int _currentVolumePercent;
	bool _digitalMusic;

	ResourceContext *_musicContext;
	ResourceContext *_digitalMusicContext;


	static void musicVolumeGaugeCallback(void *refCon);
	static void onTimer(void *refCon);
	void musicVolumeGauge();
	ByteArray *_currentMusicBuffer;
	ByteArray _musicBuffer[2];
};

} // End of namespace Saga

#endif
