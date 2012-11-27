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

#ifndef TOWNS_PC98_AUDIODRIVER_H
#define TOWNS_PC98_AUDIODRIVER_H

#include "audio/softsynth/fmtowns_pc98/towns_pc98_fmsynth.h"

class TownsPC98_MusicChannel;
class TownsPC98_MusicChannelSSG;
class TownsPC98_SfxChannel;
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
class TownsPC98_MusicChannelPCM;
#endif

class TownsPC98_AudioDriver : public TownsPC98_FmSynth {
friend class TownsPC98_MusicChannel;
friend class TownsPC98_MusicChannelSSG;
friend class TownsPC98_SfxChannel;
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
friend class TownsPC98_MusicChannelPCM;
#endif
public:
	TownsPC98_AudioDriver(Audio::Mixer *mixer, EmuType type);
	~TownsPC98_AudioDriver();

	void loadMusicData(uint8 *data, bool loadPaused = false);
	void loadSoundEffectData(uint8 *data, uint8 trackNum);
	bool init();
	void reset();

	void fadeStep();

	void pause();
	void cont();

	bool looping();
	bool musicPlaying();

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

private:
	void timerCallbackA();
	void timerCallbackB();

	void startSoundEffect();

	void setMusicTempo(uint8 tempo);
	void setSfxTempo(uint16 tempo);

	TownsPC98_MusicChannel **_channels;
	TownsPC98_MusicChannelSSG **_ssgChannels;
	TownsPC98_SfxChannel **_sfxChannels;
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	TownsPC98_MusicChannelPCM *_rhythmChannel;
#endif

	const uint8 *_opnCarrier;
	const uint8 *_opnFreqTable;
	const uint8 *_opnFreqTableSSG;
	const uint8 *_opnFxCmdLen;
	const uint8 *_opnLvlPresets;

	uint8 *_musicBuffer;
	uint8 *_sfxBuffer;
	uint8 *_trackPtr;
	uint8 *_patches;
	uint8 *_ssgPatches;

	uint8 _updateChannelsFlag;
	uint8 _updateSSGFlag;
	uint8 _updateRhythmFlag;
	uint8 _updateSfxFlag;
	uint8 _finishedChannelsFlag;
	uint8 _finishedSSGFlag;
	uint8 _finishedRhythmFlag;
	uint8 _finishedSfxFlag;

	bool _musicPlaying;
	bool _sfxPlaying;
	uint8 _fading;
	uint8 _looping;
	uint32 _musicTickCounter;

	int _sfxOffs;
	uint8 *_sfxData;
	uint16 _sfxOffsets[2];

	uint16 _musicVolume;
	uint16 _sfxVolume;

	static const uint8 _drvTables[];

	bool _ready;
};

#endif
