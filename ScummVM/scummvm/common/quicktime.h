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
 */

//
// Heavily based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#ifndef COMMON_QUICKTIME_H
#define COMMON_QUICKTIME_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/rational.h"
#include "common/types.h"

namespace Common {
	class MacResManager;

/**
 * Parser for QuickTime/MPEG-4 files.
 *
 * File parser used in engines:
 *  - groovie
 *  - mohawk
 *  - sci
 */
class QuickTimeParser {
public:
	QuickTimeParser();
	virtual ~QuickTimeParser();

	/**
	 * Load a QuickTime file
	 * @param filename	the filename to load
	 */
	bool parseFile(const String &filename);

	/**
	 * Load a QuickTime file from a SeekableReadStream
	 * @param stream	the stream to load
	 * @param disposeFileHandle whether to delete the stream after use
	 */
	bool parseStream(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle = DisposeAfterUse::YES);

	/**
	 * Close a QuickTime file
	 */
	void close();

	/**
	 * Set the beginning offset of the video so we can modify the offsets in the stco
	 * atom of videos inside the Mohawk archives
	 * @param the beginning offset of the video
	 */
	void setChunkBeginOffset(uint32 offset) { _beginOffset = offset; }

	/** Find out if this parser has an open file handle */
	bool isOpen() const { return _fd != 0; }

protected:
	// This is the file handle from which data is read from. It can be the actual file handle or a decompressed stream.
	SeekableReadStream *_fd;

	struct TimeToSampleEntry {
		int count;
		int duration;
	};

	struct SampleToChunkEntry {
		uint32 first;
		uint32 count;
		uint32 id;
	};

	struct EditListEntry {
		uint32 trackDuration;
		uint32 timeOffset;
		int32 mediaTime;
		Rational mediaRate;
	};

	struct Track;

	class SampleDesc {
	public:
		SampleDesc(Track *parentTrack, uint32 codecTag);
		virtual ~SampleDesc() {}

		uint32 getCodecTag() const { return _codecTag; }

	protected:
		Track *_parentTrack;
		uint32 _codecTag;
	};

	enum CodecType {
		CODEC_TYPE_MOV_OTHER,
		CODEC_TYPE_VIDEO,
		CODEC_TYPE_AUDIO
	};

	struct Track {
		Track();
		~Track();

		uint32 chunkCount;
		uint32 *chunkOffsets;
		int timeToSampleCount;
		TimeToSampleEntry *timeToSample;
		uint32 sampleToChunkCount;
		SampleToChunkEntry *sampleToChunk;
		uint32 sampleSize;
		uint32 sampleCount;
		uint32 *sampleSizes;
		uint32 keyframeCount;
		uint32 *keyframes;
		int32 timeScale;

		uint16 width;
		uint16 height;
		CodecType codecType;

		Array<SampleDesc *> sampleDescs;

		uint32 editCount;
		EditListEntry *editList;

		SeekableReadStream *extraData;

		uint32 frameCount;
		uint32 duration;
		uint32 mediaDuration;
		uint32 startTime;
		Rational scaleFactorX;
		Rational scaleFactorY;

		byte objectTypeMP4;
	};

	virtual SampleDesc *readSampleDesc(Track *track, uint32 format) = 0;

	uint32 _timeScale;
	uint32 _duration;
	Rational _scaleFactorX;
	Rational _scaleFactorY;
	Array<Track *> _tracks;

	void init();

private:
	struct Atom {
		uint32 type;
		uint32 offset;
		uint32 size;
	};

	struct ParseTable {
		int (QuickTimeParser::*func)(Atom atom);
		uint32 type;
	};

	DisposeAfterUse::Flag _disposeFileHandle;
	const ParseTable *_parseTable;
	uint32 _beginOffset;
	MacResManager *_resFork;
	bool _foundMOOV;

	void initParseTable();

	int readDefault(Atom atom);
	int readLeaf(Atom atom);
	int readELST(Atom atom);
	int readHDLR(Atom atom);
	int readMDHD(Atom atom);
	int readMOOV(Atom atom);
	int readMVHD(Atom atom);
	int readTKHD(Atom atom);
	int readTRAK(Atom atom);
	int readSTCO(Atom atom);
	int readSTSC(Atom atom);
	int readSTSD(Atom atom);
	int readSTSS(Atom atom);
	int readSTSZ(Atom atom);
	int readSTTS(Atom atom);
	int readCMOV(Atom atom);
	int readWAVE(Atom atom);
	int readESDS(Atom atom);
	int readSMI(Atom atom);
};

} // End of namespace Common

#endif
