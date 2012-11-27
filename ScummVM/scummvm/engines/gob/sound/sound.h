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

#ifndef GOB_SOUND_SOUND_H
#define GOB_SOUND_SOUND_H

#include "gob/sound/sounddesc.h"
#include "gob/sound/bgatmosphere.h"

namespace Gob {

class GobEngine;
class PCSpeaker;
class SoundBlaster;
class ADLPlayer;
class MUSPlayer;
class Infogrames;
class Protracker;
class CDROM;

class Sound {
public:
	static const int kSoundsCount = 60;

	Sound(GobEngine *vm);
	~Sound();

	static void convToSigned(byte *buffer, int length);

	// Samples
	SoundDesc *sampleGetBySlot(int slot);
	const SoundDesc *sampleGetBySlot(int slot) const;
	int sampleGetNextFreeSlot() const;

	bool sampleLoad(SoundDesc *sndDesc, SoundType type, const char *fileName);
	void sampleFree(SoundDesc *sndDesc, bool noteAdLib = false, int index = -1);


	// SoundBlaster
	void blasterPlay(SoundDesc *sndDesc, int16 repCount,
			int16 frequency, int16 fadeLength = 0);
	void blasterStop(int16 fadeLength, SoundDesc *sndDesc = 0);

	void blasterPlayComposition(const int16 *composition, int16 freqVal,
			SoundDesc *sndDescs = 0, int8 sndCount = kSoundsCount);
	void blasterStopComposition();
	void blasterRepeatComposition(int32 repCount);

	char blasterPlayingSound() const;

	void blasterSetRepeating(int32 repCount);
	void blasterWaitEndPlay(bool interruptible = false, bool stopComp = true);


	// PCSpeaker
	void speakerOn(int16 frequency, int32 length = -1);
	void speakerOff();
	void speakerOnUpdate(uint32 millis);


	// AdLib
	bool adlibLoadADL(const char *fileName);
	bool adlibLoadADL(byte *data, uint32 size, int index = -1);
	bool adlibLoadMDY(const char *fileName);
	bool adlibLoadTBR(const char *fileName);
	void adlibUnload();

	void adlibPlayTrack(const char *trackname);
	void adlibPlayBgMusic();

	void adlibPlay();
	void adlibStop();

	bool adlibIsPlaying() const;

	int adlibGetIndex() const;
	int32 adlibGetRepeating() const;

	void adlibSetRepeating(int32 repCount);


	// Infogrames
	bool infogramesLoadInstruments(const char *fileName);
	bool infogramesLoadSong(const char *fileName);

	void infogramesPlay();
	void infogramesStop();


	// Protracker
	bool protrackerPlay(const char *fileName);
	void protrackerStop();


	// CD-ROM
	void cdLoadLIC(const Common::String &fname);
	void cdUnloadLIC();

	void cdPlayBgMusic();
	void cdPlayMultMusic();

	void cdPlay(const Common::String &);
	void cdStop();

	bool cdIsPlaying() const;

	int32 cdGetTrackPos(const char *keyTrack = 0) const;
	const char *cdGetCurrentTrack() const;

	void cdTest(int trySubst, const char *label);


	// Background Atmosphere
	void bgPlay(const char *file, SoundType type);
	void bgPlay(const char *base, const char *ext, SoundType type, int count);
	void bgStop();

	void bgSetPlayMode(BackgroundAtmosphere::PlayMode mode);

	void bgShade();
	void bgUnshade();

private:
	GobEngine *_vm;

	bool _hasAdLib;
	bool _hasAdLibBg;

	SoundDesc _sounds[kSoundsCount];

	// Speaker
	PCSpeaker *_pcspeaker;

	// PCM based
	SoundBlaster *_blaster;
	BackgroundAtmosphere *_bgatmos;

	// AdLib
	MUSPlayer *_mdyPlayer;
	ADLPlayer *_adlPlayer;

	// Amiga Paula
	Infogrames *_infogrames;
	Protracker *_protracker;

	// Audio CD
	CDROM *_cdrom;

	void createMDYPlayer();
	void createADLPlayer();
};

} // End of namespace Gob

#endif // GOB_SOUND_SOUND_H
