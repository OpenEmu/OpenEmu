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

// Disable symbol overrides for FILE as that is used in FLAC headers
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "cdaudio.h"
#include "backends/fs/ds/ds-fs.h"
#include "common/config-manager.h"
#include "dsmain.h"
#include "NDS/scummvm_ipc.h"

#define WAV_FORMAT_IMA_ADPCM 0x14
#define BUFFER_SIZE 8192
#define BUFFER_CHUNK_SIZE (BUFFER_SIZE >> 2)

namespace DS {
namespace CD {

struct WaveHeader {

	char		riff[4];		// 'RIFF'
	u32			size;			// Size of the file
	char		wave[4];		// 'WAVE'

	// fmt chunk
	char		fmt[4];			// 'fmt '
	u32			fmtSize;		// Chunk size
	u16			fmtFormatTag;	// Format of this file
	u16			fmtChannels;	// Num channels
	u32			fmtSamPerSec;	// Samples per second
	u32			fmtBytesPerSec; // Bytes per second
	u16			fmtBlockAlign;	// Block alignment
	u16			fmtBitsPerSam;	// Bits per sample

	u16			fmtExtraData;	// Number of extra fmt bytes
	u16			fmtExtra;		// Samples per block (only for IMA-ADPCM files)
} __attribute__ ((packed));

struct chunkHeader {
	char 		name[4];
	u32			size;
} __attribute__ ((packed));

struct Header {
	s16 		firstSample;
	char		stepTableIndex;
	char		reserved;
} __attribute__ ((packed));

struct decoderFormat {
	s16 initial;
	unsigned char tableIndex;
	unsigned char test;
	unsigned char	sample[1024];
} __attribute__ ((packed));

static bool s_started = false;
static bool s_active = false;
static WaveHeader waveHeader;
static Header blockHeader;
static FILE *s_file;
static int fillPos;
static bool isPlayingFlag = false;

static s16 *audioBuffer;
static u32 sampleNum;
static s16 *decompressionBuffer;
static int s_numLoops;
static int blockCount;
static int dataChunkStart;
static int blocksLeft;
static bool trackStartsAt2 = false;


// These are from Microsoft's document on DVI ADPCM
static const int stepTab[ 89 ] = {
7, 8, 9, 10, 11, 12, 13, 14,
16, 17, 19, 21, 23, 25, 28, 31,
34, 37, 41, 45, 50, 55, 60, 66,
73, 80, 88, 97, 107, 118, 130, 143,
157, 173, 190, 209, 230, 253, 279, 307,
337, 371, 408, 449, 494, 544, 598, 658,
724, 796, 876, 963, 1060, 1166, 1282, 1411,
1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
32767 };

static const int indexTab[ 16 ] = { -1, -1, -1, -1, 2, 4, 6, 8,
-1, -1, -1, -1, 2, 4, 6, 8 };

void playNextBlock();
void decompressBlock();


void allocBuffers() {

}

void setActive(bool active) {
	s_active = active;
}

bool getActive() {
	return s_active;
}

void playTrack(int track, int numLoops, int startFrame, int duration) {
	Common::String path = ConfMan.get("path");

	if (isPlayingFlag) {
		stopTrack();
	}

	if (trackStartsAt2) {
		track++;
	}



	char str[100];

	if (path.lastChar() != '/')
		path += '/';

	Common::String fname;

	sprintf(str, "track%d.wav", track);
	fname = path + str;
	s_file = DS::std_fopen(fname.c_str(), "rb");

	if (!s_file) {
		sprintf(str, "track%02d.wav", track);
		fname = path + str;
		s_file = DS::std_fopen(fname.c_str(), "rb");
	}

	if (!s_file) {
		consolePrintf("Failed to open %s!\n", path.c_str());
		return;
	}


	DS::std_fread(&waveHeader, sizeof(waveHeader), 1, s_file);

	consolePrintf("File: %s\n", fname.c_str());

	consolePrintf("Playing track %d\n", track);
	consolePrintf("Format: %d\n", waveHeader.fmtFormatTag);
	consolePrintf("Rate  : %d\n", waveHeader.fmtSamPerSec);
	consolePrintf("Bits  : %d\n", waveHeader.fmtBitsPerSam);
	consolePrintf("BlkSz : %d\n", waveHeader.fmtExtra);

	if ((waveHeader.fmtFormatTag != 17) && (waveHeader.fmtFormatTag != 20)) {
		consolePrintf("Wave file is in the wrong format!  You must use IMA-ADPCM 4-bit mono.\n");
		DS::std_fclose(s_file);
		return;
	}

	for (int r = 0; r < 8; r++) {
		IPC->adpcm.buffer[r] = (u8 * volatile) (decoderFormat *) malloc(waveHeader.fmtBlockAlign);
		IPC->adpcm.filled[r] = false;
		IPC->adpcm.arm7Dirty[r] = false;
	}

	// Skip chunks until we reach the data chunk
	chunkHeader chunk;
	DS::std_fread(&chunk, sizeof(chunkHeader), 1, s_file);

	while (!((chunk.name[0] == 'd') && (chunk.name[1] == 'a') && (chunk.name[2] == 't') && (chunk.name[3] == 'a'))) {
		DS::std_fseek(s_file, chunk.size, SEEK_CUR);
		DS::std_fread(&chunk, sizeof(chunkHeader), 1, s_file);
	}

	dataChunkStart = DS::std_ftell(s_file);


	sampleNum = 0;
	blockCount = 0;

	IPC->streamFillNeeded[0] = true;
	IPC->streamFillNeeded[1] = true;
	IPC->streamFillNeeded[2] = true;
	IPC->streamFillNeeded[3] = true;
	if (!s_started) {
		fillPos = 0;
		audioBuffer = (s16 *) malloc(BUFFER_SIZE * 2);
		decompressionBuffer = (s16 *) malloc(waveHeader.fmtExtra * 2);
		s_started = true;
//		consolePrintf("****Starting buffer*****\n");
		memset(audioBuffer, 0, BUFFER_SIZE * 2);
		memset(decompressionBuffer, 0, waveHeader.fmtExtra * 2);
		DS::playSound(audioBuffer, BUFFER_SIZE * 2, false, false, waveHeader.fmtSamPerSec);

	}
	fillPos = (IPC->streamPlayingSection + 1) & 3;
	isPlayingFlag = true;


	// Startframe is a 75Hz timer.  Dunno why, since nothing else
	// seems to run at that rate.
	int tenths = (startFrame * 10) / 75;

	// Seek to the nearest block start to the start time
	int samples = (tenths * waveHeader.fmtSamPerSec) / 10;
	int block = samples / waveHeader.fmtExtra;


	if (duration == 0) {
		blocksLeft = 0;
	} else {
		blocksLeft = ((((duration * 100) / 75) * (waveHeader.fmtSamPerSec)) / (waveHeader.fmtExtra) / 100) + 10;
	}
//	consolePrintf("Playing %d blocks (%d)\n\n", blocksLeft, duration);

	// No need to seek if we're starting from the beginning
	if (block != 0) {
		DS::std_fseek(s_file, dataChunkStart + block * waveHeader.fmtBlockAlign, SEEK_SET);
//		consolePrintf("Startframe: %d  msec: %d (%d,%d)\n", startFrame, tenthssec, samples, block);
	}


	//decompressBlock();
	playNextBlock();
	s_numLoops = numLoops;
}

void update() {
	playNextBlock();
}

#ifdef ARM_ADPCM
// FIXME: This code, as well as the source file adpcm_arm.s, are
// apparently unused. Maybe that is a mistake? Or maybe there is a bug
// in ARM_adpcm (then this should be reported and fixed). Or maybe there
// are other good reasons to prefer the C code, but then this as well as
// the assembler source file should be removed.
extern "C" void ARM_adpcm(int *block, int len, int stepTableIndex,
                          int firstSample, s16 *decompressionBuffer);
#endif

void decompressBlock() {
	int block[2048];
	bool loop = false;

	blockCount++;

	if (blockCount < 10) return;


	do {
		DS::std_fread(&blockHeader, sizeof(blockHeader), 1, s_file);

		DS::std_fread(&block[0], waveHeader.fmtBlockAlign - sizeof(blockHeader), 1, s_file);

		if (DS::std_feof(s_file)) {
			// Reached end of file, so loop


			if ((s_numLoops == -1) || (s_numLoops > 1)) {
				// Seek file to first packet
				if (s_numLoops != -1) {
					s_numLoops--;
				}
				DS::std_fseek(s_file, dataChunkStart, SEEK_SET);
				loop = true;
			} else {
				// Fill decompression buffer with zeros to prevent glitching
				for (int r = 0; r < waveHeader.fmtExtra; r++) {
					decompressionBuffer[r] = 0;
				}
//				consolePrintf("Stopping music\n");
				stopTrack();
				return;
			}

		} else {
			loop = false;
		}

	} while (loop);


	if (blocksLeft > 0) {
		blocksLeft--;
	//	consolePrintf("%d ", blocksLeft);
		if (blocksLeft == 0) {
			stopTrack();
			return;
		}
	}

#ifdef ARM_ADPCM
	ARM_adpcm(block, waveHeader.fmtExtra,
	          blockHeader.stepTableIndex,
	          blockHeader.firstSample,
	          decompressionBuffer);
#else
	// First sample is in header
	decompressionBuffer[0] = blockHeader.firstSample;

	// Set up initial table indeces
	int stepTableIndex = blockHeader.stepTableIndex;
	int prevSample = blockHeader.firstSample;

//	consolePrintf("Decompressing block step=%d fs=%d\n", stepTableIndex, prevSample);

	for (int r = 0; r < waveHeader.fmtExtra - 1; r++) {

		int word = block[r >> 3];
		int offset = 0;

		switch (7 - (r & 0x0007)) {
		case 0:
			offset = (word & 0xF0000000) >> 28;
			break;
		case 1:
			offset = (word & 0x0F000000) >> 24;
			break;
		case 2:
			offset = (word & 0x00F00000) >> 20;
			break;
		case 3:
			offset = (word & 0x000F0000) >> 16;
			break;
		case 4:
			offset = (word & 0x0000F000) >> 12;
			break;
		case 5:
			offset = (word & 0x00000F00) >> 8;
			break;
		case 6:
			offset = (word & 0x000000F0) >> 4;
			break;
		case 7:
			offset = (word & 0x0000000F);
			break;
		}

		int diff = 0;

		if (offset & 4) {
			diff = diff + stepTab[stepTableIndex];
		}

		if (offset & 2) {
			diff = diff + (stepTab[stepTableIndex] >> 1);
		}

		if (offset & 1) {
			diff = diff + (stepTab[stepTableIndex] >> 2);
		}

		diff = diff + (stepTab[stepTableIndex] >> 3);

		if (offset & 8) {
			diff = -diff;
		}

		int newSample = prevSample + diff;

		if (newSample > 32767) newSample = 32767;
		if (newSample < -32768) newSample = -32768;

		decompressionBuffer[r + 1] = newSample;

		prevSample = newSample;

		stepTableIndex += indexTab[offset];

		if (stepTableIndex > 88) stepTableIndex = 88;
		if (stepTableIndex < 0) stepTableIndex = 0;


	}
#endif
}

void playNextBlock() {
	if (!isPlayingFlag)
		return;
	int lastBlockId = -1;

	while (IPC->adpcm.semaphore);		// Wait for buffer to become free if needed
	IPC->adpcm.semaphore = true;		// Lock the buffer structure to prevent clashing with the ARM7
//	DC_FlushAll();

	//-8644, 25088
	for (int block = fillPos + 1; block < fillPos + 4; block++) {

		int blockId = block & 3;

		if (IPC->streamFillNeeded[blockId]) {

			IPC->streamFillNeeded[blockId] = false;
//			DC_FlushAll();

/*			if (!(REG_KEYINPUT & KEY_R)) {
				//consolePrintf("Align: %d First: %d  Step:%d  Res:%d\n", waveHeader.fmtBlockAlign, blockHeader.firstSample, blockHeader.stepTableIndex, blockHeader.reserved);
				consolePrintf("Filling buffer %d\n", blockId);
			}*/
			for (int r = blockId * BUFFER_CHUNK_SIZE; r < (blockId + 1) * BUFFER_CHUNK_SIZE; r++) {
				if (isPlayingFlag) {
					audioBuffer[r] = decompressionBuffer[sampleNum++];
					if (sampleNum >= waveHeader.fmtExtra) {
						decompressBlock();
						sampleNum = 0;
					}
				}
			}

			lastBlockId = blockId;
			IPC->streamFillNeeded[blockId] = false;
//			DC_FlushAll();

		}
	}



	if (lastBlockId != -1) {
		fillPos = lastBlockId;
/*		if (!(REG_KEYINPUT & KEY_R)) {
			consolePrintf("Frame fill done\n");
		}*/
	}
	IPC->adpcm.semaphore = false;		// Release the buffer structure
//	DC_FlushAll();
}

void stopTrack() {
	if (!isPlayingFlag)
		return;

	DS::std_fclose(s_file);

	isPlayingFlag = false;

	for (int r = 0; r < BUFFER_SIZE; r++) {
		audioBuffer[r] = 0;
	}

	for (int r = 0; r < waveHeader.fmtExtra; r++) {
		decompressionBuffer[r] = 0;
	}
//	DS::stopSound(1);

//	free(audioBuffer);
//	free(decompressionBuffer);

	DC_FlushAll();
}

bool trackExists(int num) {
	Common::String path;
	char fname[128];
	FILE *file;

	sprintf(fname, "track%d.wav", num);

	path = ConfMan.get("path");
	if (path.lastChar() != '/')
		path += '/';
	path += fname;

	consolePrintf("Looking for %s...", path.c_str());

	file = DS::std_fopen(path.c_str(), "r");
	if (file) {
		consolePrintf("Success!\n");
		setActive(true);
		DS::std_fclose(file);
		return true;
	}

	sprintf(fname, "track%02d.wav", num);

	path = ConfMan.get("path");
	if (path.lastChar() != '/')
		path += '/';
	path += fname;

	consolePrintf("Looking for %s...", path.c_str());

	file = DS::std_fopen(path.c_str(), "r");
	if (file) {
		consolePrintf("Success!\n");
		setActive(true);
		DS::std_fclose(file);
		return true;
	}

	setActive(false);
	consolePrintf("Failed!\n");
	return false;
}

bool checkCD() {
	// Need to check whethe CD audio files are present - do this by trying to open Track1.wav.
	consolePrintf("Attempted to open cd drive\n");

	if (trackExists(1)) {
		trackStartsAt2 = false;
		return true;
	} else if (trackExists(2)) {
		trackStartsAt2 = true;
		return true;
	} else {
		return false;
	}
}

bool isPlaying() {
	return isPlayingFlag;
}

}
}	// End of namespace DS
