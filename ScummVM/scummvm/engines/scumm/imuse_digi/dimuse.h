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
 */

#if !defined(SCUMM_IMUSE_DIGI_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"
#include "scumm/music.h"
#include "scumm/sound.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Scumm {

enum {
	MAX_DIGITAL_TRACKS = 8,
	MAX_DIGITAL_FADETRACKS = 8
};

struct imuseDigTable;
struct imuseComiTable;
class Serializer;
class ScummEngine_v7;
struct Track;

class IMuseDigital : public MusicEngine {
private:

	int _callbackFps;		// value how many times callback needs to be called per second

	struct TriggerParams {
		char marker[10];
		int fadeOutDelay;
		char filename[13];
		int soundId;
		int hookId;
		int volume;
	};

	TriggerParams _triggerParams;
	bool _triggerUsed;

	Track *_track[MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS];

	Common::Mutex _mutex;
	ScummEngine_v7 *_vm;
	Audio::Mixer *_mixer;
	ImuseDigiSndMgr *_sound;

	char *_audioNames;		// filenames of sound SFX used in FT
	int32 _numAudioNames;	// number of above filenames

	bool _pause;			// flag mean that iMuse callback should be idle

	int32 _attributes[188];	// internal attributes for each music file to store and check later
	int32 _nextSeqToPlay;	// id of sequence type of music needed played
	int32 _curMusicState;	// current or previous id of music
	int32 _curMusicSeq;		// current or previous id of sequence music
	int32 _curMusicCue;		// current cue for current music. used in FT
	int _stopingSequence;
	bool _radioChatterSFX;

	static void timer_handler(void *refConf);
	void callback();
	void switchToNextRegion(Track *track);
	int allocSlot(int priority);
	void startSound(int soundId, const char *soundName, int soundType, int volGroupId, Audio::AudioStream *input, int hookId, int volume, int priority, Track *otherTrack);
	void selectVolumeGroup(int soundId, int volGroupId);

	int32 getPosInMs(int soundId);
	void getLipSync(int soundId, int syncId, int32 msPos, int32 &width, int32 &height);

	int getSoundIdByName(const char *soundName);
	void fadeOutMusic(int fadeDelay);
	void fadeOutMusicAndStartNew(int fadeDelay, const char *filename, int soundId);
	void setTrigger(TriggerParams *trigger);
	void setHookIdForMusic(int hookId);
	Track *cloneToFadeOutTrack(Track *track, int fadeDelay);

	void setFtMusicState(int stateId);
	void setFtMusicSequence(int seqId);
	void setFtMusicCuePoint(int cueId);
	void playFtMusic(const char *songName, int opcode, int volume);

	void setComiMusicState(int stateId);
	void setComiMusicSequence(int seqId);
	void playComiMusic(const char *songName, const imuseComiTable *table, int attribPos, bool sequence);

	void setDigMusicState(int stateId);
	void setDigMusicSequence(int seqId);
	void playDigMusic(const char *songName, const imuseDigTable *table, int attribPos, bool sequence);

	void flushTrack(Track *track);

public:
	IMuseDigital(ScummEngine_v7 *scumm, Audio::Mixer *mixer, int fps);
	virtual ~IMuseDigital();

	void setAudioNames(int32 num, char *names);

	void startVoice(int soundId, Audio::AudioStream *input);
	void startVoice(int soundId, const char *soundName);
	void startMusic(int soundId, int volume);
	void startMusic(const char *soundName, int soundId, int hookId, int volume);
	void startMusicWithOtherPos(const char *soundName, int soundId, int hookId, int volume, Track *otherTrack);
	void startSfx(int soundId, int priority);
	void startSound(int sound)
		{ error("IMuseDigital::startSound(int) should be never called"); }

	void saveOrLoad(Serializer *ser);
	void resetState();
	void setRadioChatterSFX(bool state) {
		_radioChatterSFX = state;
	}

	void setPriority(int soundId, int priority);
	void setVolume(int soundId, int volume);
	void setPan(int soundId, int pan);
	void setFade(int soundId, int destVolume, int delay60HzTicks);
	int getCurMusicSoundId();
	void setHookId(int soundId, int hookId);
	void setMusicVolume(int vol) {}
	void stopSound(int sound);
	void stopAllSounds();
	void pause(bool pause);
	void parseScriptCmds(int cmd, int soundId, int sub_cmd, int d, int e, int f, int g, int h);
	void refreshScripts();
	void flushTracks();
	int getSoundStatus(int sound) const;
	int32 getCurMusicPosInMs();
	int32 getCurVoiceLipSyncWidth();
	int32 getCurVoiceLipSyncHeight();
	int32 getCurMusicLipSyncWidth(int syncId);
	int32 getCurMusicLipSyncHeight(int syncId);
};

} // End of namespace Scumm

#endif
