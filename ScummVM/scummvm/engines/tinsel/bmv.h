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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Plays films within a scene, takes into account the actor in each 'column'.
 */

#ifndef TINSEL_BMV_H
#define TINSEL_BMV_H

#include "common/coroutines.h"
#include "common/file.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "tinsel/object.h"
#include "tinsel/palette.h"

namespace Tinsel {


class BMVPlayer {

	bool bOldAudio;

	/// Set when a movie is on
	bool bMovieOn;

	/// Set to kill one off
	bool bAbort;

	/// For escaping out of movies
	int bmvEscape;

	/// Movie file pointer
	Common::File stream;

	/// Movie file name
	char szMovieFile[14];

	/// Pointers to buffers
	byte *bigBuffer;

	/// Next data to use to extract a frame
	int nextUseOffset;

	/// Next data to use to extract sound data
	int nextSoundOffset;

	/// When above offset gets to what this is set at, rewind
	int wrapUseOffset;

	/// The offset of the most future packet
	int mostFutureOffset;

	/// The current frame
	int currentFrame;
	int currentSoundFrame;

	/// Number of packets currently in RAM
	int numAdvancePackets;

	/// Next slot that will be read from disc
	int nextReadSlot;

	/// Set when the whole file has been read
	bool bFileEnd;

	/// Palette
	COLORREF moviePal[256];

	int blobsInBuffer;

	struct {
		POBJECT	pText;
		int	dieFrame;
	} texts[2];

	COLORREF talkColor;

	int bigProblemCount;

	bool bIsText;

	int movieTick;
	int startTick;
	uint32 nextMovieTime;

	uint16 Au_Prev1;
	uint16 Au_Prev2;
	byte *ScreenBeg;
	byte *screenBuffer;

	bool audioStarted;

	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	int nextMaintain;
public:
	BMVPlayer();

	void PlayBMV(CORO_PARAM, SCNHANDLE hFileStem, int myEscape);
	void FinishBMV();
	void CopyMovieToScreen();
	void FettleBMV();

	bool MoviePlaying();

	int32 MovieAudioLag();

	uint32 NextMovieTime();

	void AbortMovie();

private:
	void InitBMV(byte *memoryBuffer);
	void PrepAudio(const byte *sourceData, int blobCount, byte *destPtr);
	void MoviePalette(int paletteOffset);
	void InitializeMovieSound();
	void StartMovieSound();
	void FinishMovieSound();
	void MovieAudio(int audioOffset, int blobs);
	void FettleMovieText();
	void BmvDrawText(bool bDraw);
	void MovieText(CORO_PARAM, int stringId, int x, int y, int fontId, COLORREF *pTalkColor, int duration);
	int MovieCommand(char cmd, int commandOffset);
	int FollowingPacket(int thisPacket, bool bReallyImportant);
	void LoadSlots(int number);
	void InitializeBMV();
	bool MaintainBuffer();
	bool DoBMVFrame();
	bool DoSoundFrame();
};


} // End of namespace Tinsel

#endif
