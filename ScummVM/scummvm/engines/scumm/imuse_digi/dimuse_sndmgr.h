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

#ifndef SCUMM_IMUSE_DIGI_SNDMGR_H
#define SCUMM_IMUSE_DIGI_SNDMGR_H


#include "common/scummsys.h"
#include "audio/audiostream.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

class ScummEngine;
class BundleMgr;

class ImuseDigiSndMgr {
public:

#define MAX_IMUSE_SOUNDS 16

#define IMUSE_RESOURCE 1
#define IMUSE_BUNDLE 2

#define IMUSE_VOLGRP_VOICE 1
#define IMUSE_VOLGRP_SFX 2
#define IMUSE_VOLGRP_MUSIC 3

private:
	struct Region {
		int32 offset;		// offset of region
		int32 length;		// length of region
	};

	struct Jump {
		int32 offset;		// jump offset position
		int32 dest;			// jump to dest position
		byte hookId;		// id of hook
		int16 fadeDelay;	// fade delay in ms
	};

	struct Sync {
		int32 size;			// size of sync
		byte *ptr;			// pointer to sync
	};

	struct Marker {
		int32 pos;			// position Markaer in sound data
		int32 length;		// length of marker string
		char *ptr;			// pointer to string
	};

public:

	struct SoundDesc {
		uint16 freq;		// frequency
		byte channels;		// stereo or mono
		byte bits;			// 8, 12, 16

		int numJumps;		// number of Jumps
		Region *region;

		int numRegions;		// number of Regions
		Jump *jump;

		int numSyncs;		// number of Syncs
		Sync *sync;

		int numMarkers;		// number of Markers
		Marker *marker;

		bool endFlag;
		bool inUse;
		byte *allData;
		int32 offsetData;
		byte *resPtr;
		char name[15];
		int16 soundId;
		BundleMgr *bundle;
		int type;
		int volGroupId;
		int disk;
		Audio::SeekableAudioStream *compressedStream;
		bool compressed;
		char lastFileName[24];
	};

private:

	SoundDesc _sounds[MAX_IMUSE_SOUNDS];

	bool checkForProperHandle(SoundDesc *soundDesc);
	SoundDesc *allocSlot();
	void prepareSound(byte *ptr, SoundDesc *sound);
	void prepareSoundFromRMAP(Common::SeekableReadStream *file, SoundDesc *sound, int32 offset, int32 size);

	ScummEngine *_vm;
	byte _disk;
	BundleDirCache *_cacheBundleDir;

	bool openMusicBundle(SoundDesc *sound, int &disk);
	bool openVoiceBundle(SoundDesc *sound, int &disk);

	void countElements(byte *ptr, int &numRegions, int &numJumps, int &numSyncs, int &numMarkers);

public:

	ImuseDigiSndMgr(ScummEngine *scumm);
	~ImuseDigiSndMgr();

	SoundDesc *openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk);
	void closeSound(SoundDesc *soundDesc);
	SoundDesc *cloneSound(SoundDesc *soundDesc);

	bool isSndDataExtComp(SoundDesc *soundDesc);
	int getFreq(SoundDesc *soundDesc);
	int getBits(SoundDesc *soundDesc);
	int getChannels(SoundDesc *soundDesc);
	bool isEndOfRegion(SoundDesc *soundDesc, int region);
	int getNumRegions(SoundDesc *soundDesc);
	int getNumJumps(SoundDesc *soundDesc);
	int getRegionOffset(SoundDesc *soundDesc, int region);
	int getJumpIdByRegionAndHookId(SoundDesc *soundDesc, int region, int hookId);
	bool checkForTriggerByRegionAndMarker(SoundDesc *soundDesc, int region, const char *marker);
	int getRegionIdByJumpId(SoundDesc *soundDesc, int jumpId);
	int getJumpHookId(SoundDesc *soundDesc, int number);
	int getJumpFade(SoundDesc *soundDesc, int number);
	void getSyncSizeAndPtrById(SoundDesc *soundDesc, int number, int32 &sync_size, byte **sync_ptr);

	int32 getDataFromRegion(SoundDesc *soundDesc, int region, byte **buf, int32 offset, int32 size);
};

} // End of namespace Scumm

#endif
