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

#ifndef SCI_SOUNDCMD_H
#define SCI_SOUNDCMD_H

#include "common/list.h"
#include "audio/mididrv.h"	// for MusicType
#include "sci/engine/state.h"

namespace Sci {

class Console;
class SciMusic;
class SoundCommandParser;
class MusicEntry;
//typedef void (SoundCommandParser::*SoundCommand)(reg_t obj, int16 value);

//struct MusicEntryCommand {
//	MusicEntryCommand(const char *d, SoundCommand c) : sndCmd(c), desc(d) {}
//	SoundCommand sndCmd;
//	const char *desc;
//};

class SoundCommandParser {
public:
	SoundCommandParser(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, AudioPlayer *audio, SciVersion soundVersion);
	~SoundCommandParser();

	//reg_t parseCommand(int argc, reg_t *argv, reg_t acc);

	// Functions used for game state loading
	void clearPlayList();
	void syncPlayList(Common::Serializer &s);
	void reconstructPlayList();

	// Functions used for the ScummVM menus
	void setMasterVolume(int vol);
	void pauseAll(bool pause);

	// Debug console functions
	void startNewSound(int number);
	void stopAllSounds();
	void printPlayList(Console *con);
	void printSongInfo(reg_t obj, Console *con);

	void processPlaySound(reg_t obj);
	void processStopSound(reg_t obj, bool sampleFinishedPlaying);
	void initSoundResource(MusicEntry *newSound);

	MusicType getMusicType() const;

	/**
	 * Synchronizes the current state of the music list to the rest of the engine, so that
	 * the changes that the sound thread makes to the music are registered with the engine
	 * scripts. In SCI0, we invoke this from kAnimate (which is called very often). SCI01
	 * and later have a specific callback function, cmdUpdateCues, which is called regularly
	 * by the engine scripts themselves, so the engine itself polls for changes to the music
	 */
	void updateSci0Cues();

	reg_t kDoSoundInit(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundPlay(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundRestore(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundMute(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundPause(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundResumeAfterRestore(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundStop(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundStopAll(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundDispose(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundMasterVolume(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundFade(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundGetPolyphony(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundUpdate(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundUpdateCues(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundSendMidi(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundGlobalReverb(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundSetHold(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundDummy(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundGetAudioCapability(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundSetVolume(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundSetPriority(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundSetLoop(int argc, reg_t *argv, reg_t acc);
	reg_t kDoSoundSuspend(int argc, reg_t *argv, reg_t acc);

private:
	//typedef Common::Array<MusicEntryCommand *> SoundCommandContainer;
	//SoundCommandContainer _soundCommands;
	ResourceManager *_resMan;
	SegManager *_segMan;
	Kernel *_kernel;
	SciMusic *_music;
	AudioPlayer *_audio;
	SciVersion _soundVersion;
	// If true and an alternative digital sound effect exists, the digital
	// sound effect is preferred instead
	bool _useDigitalSFX;

	void processInitSound(reg_t obj);
	void processDisposeSound(reg_t obj);
	void processUpdateCues(reg_t obj);
	int getSoundResourceId(reg_t obj);
};

} // End of namespace Sci

#endif // SCI_SOUNDCMD_H
