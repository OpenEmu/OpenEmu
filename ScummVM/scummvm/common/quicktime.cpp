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

#include "common/debug.h"
#include "common/endian.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/quicktime.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/zlib.h"

namespace Common {

////////////////////////////////////////////
// QuickTimeParser
////////////////////////////////////////////

QuickTimeParser::QuickTimeParser() {
	_beginOffset = 0;
	_fd = 0;
	_scaleFactorX = 1;
	_scaleFactorY = 1;
	_resFork = new MacResManager();
	_disposeFileHandle = DisposeAfterUse::YES;

	initParseTable();
}

QuickTimeParser::~QuickTimeParser() {
	close();
	delete _resFork;
}

bool QuickTimeParser::parseFile(const String &filename) {
	if (!_resFork->open(filename) || !_resFork->hasDataFork())
		return false;

	_foundMOOV = false;
	_disposeFileHandle = DisposeAfterUse::YES;

	Atom atom = { 0, 0, 0 };

	if (_resFork->hasResFork()) {
		// Search for a 'moov' resource
		MacResIDArray idArray = _resFork->getResIDArray(MKTAG('m', 'o', 'o', 'v'));

		if (!idArray.empty())
			_fd = _resFork->getResource(MKTAG('m', 'o', 'o', 'v'), idArray[0]);

		if (_fd) {
			atom.size = _fd->size();
			if (readDefault(atom) < 0 || !_foundMOOV)
				return false;
		}

		delete _fd;
	}

	_fd = _resFork->getDataFork();
	atom.size = _fd->size();

	if (readDefault(atom) < 0 || !_foundMOOV)
		return false;

	init();
	return true;
}

bool QuickTimeParser::parseStream(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
	_fd = stream;
	_foundMOOV = false;
	_disposeFileHandle = disposeFileHandle;

	Atom atom = { 0, 0, 0xffffffff };

	if (readDefault(atom) < 0 || !_foundMOOV) {
		close();
		return false;
	}

	init();
	return true;
}

void QuickTimeParser::init() {
	for (uint32 i = 0; i < _tracks.size(); i++) {
		// Remove unknown/unhandled tracks
		if (_tracks[i]->codecType == CODEC_TYPE_MOV_OTHER) {
			delete _tracks[i];
			_tracks.remove_at(i);
			i--;
		} else {
			// If this track doesn't have a declared scale, use the movie scale
			if (_tracks[i]->timeScale == 0)
				_tracks[i]->timeScale = _timeScale;

			// If this track doesn't have an edit list (like in MPEG-4 files),
			// fake an entry of one edit that takes up the entire sample
			if (_tracks[i]->editCount == 0) {
				_tracks[i]->editCount = 1;
				_tracks[i]->editList = new EditListEntry[1];
				_tracks[i]->editList[0].trackDuration = _tracks[i]->duration;
				_tracks[i]->editList[0].timeOffset = 0;
				_tracks[i]->editList[0].mediaTime = 0;
				_tracks[i]->editList[0].mediaRate = 1;
			}
		}
	}
}

void QuickTimeParser::initParseTable() {
	static const ParseTable p[] = {
		{ &QuickTimeParser::readDefault, MKTAG('d', 'i', 'n', 'f') },
		{ &QuickTimeParser::readLeaf,    MKTAG('d', 'r', 'e', 'f') },
		{ &QuickTimeParser::readDefault, MKTAG('e', 'd', 't', 's') },
		{ &QuickTimeParser::readELST,    MKTAG('e', 'l', 's', 't') },
		{ &QuickTimeParser::readHDLR,    MKTAG('h', 'd', 'l', 'r') },
		{ &QuickTimeParser::readLeaf,    MKTAG('m', 'd', 'a', 't') },
		{ &QuickTimeParser::readMDHD,    MKTAG('m', 'd', 'h', 'd') },
		{ &QuickTimeParser::readDefault, MKTAG('m', 'd', 'i', 'a') },
		{ &QuickTimeParser::readDefault, MKTAG('m', 'i', 'n', 'f') },
		{ &QuickTimeParser::readMOOV,    MKTAG('m', 'o', 'o', 'v') },
		{ &QuickTimeParser::readMVHD,    MKTAG('m', 'v', 'h', 'd') },
		{ &QuickTimeParser::readLeaf,    MKTAG('s', 'm', 'h', 'd') },
		{ &QuickTimeParser::readDefault, MKTAG('s', 't', 'b', 'l') },
		{ &QuickTimeParser::readSTCO,    MKTAG('s', 't', 'c', 'o') },
		{ &QuickTimeParser::readSTSC,    MKTAG('s', 't', 's', 'c') },
		{ &QuickTimeParser::readSTSD,    MKTAG('s', 't', 's', 'd') },
		{ &QuickTimeParser::readSTSS,    MKTAG('s', 't', 's', 's') },
		{ &QuickTimeParser::readSTSZ,    MKTAG('s', 't', 's', 'z') },
		{ &QuickTimeParser::readSTTS,    MKTAG('s', 't', 't', 's') },
		{ &QuickTimeParser::readTKHD,    MKTAG('t', 'k', 'h', 'd') },
		{ &QuickTimeParser::readTRAK,    MKTAG('t', 'r', 'a', 'k') },
		{ &QuickTimeParser::readLeaf,    MKTAG('u', 'd', 't', 'a') },
		{ &QuickTimeParser::readLeaf,    MKTAG('v', 'm', 'h', 'd') },
		{ &QuickTimeParser::readCMOV,    MKTAG('c', 'm', 'o', 'v') },
		{ &QuickTimeParser::readWAVE,    MKTAG('w', 'a', 'v', 'e') },
		{ &QuickTimeParser::readESDS,    MKTAG('e', 's', 'd', 's') },
		{ &QuickTimeParser::readSMI,     MKTAG('S', 'M', 'I', ' ') },
		{ 0, 0 }
	};

	_parseTable = p;
}

int QuickTimeParser::readDefault(Atom atom) {
	uint32 total_size = 0;
	Atom a;
	int err = 0;

	a.offset = atom.offset;

	while(((total_size + 8) < atom.size) && !_fd->eos() && _fd->pos() < _fd->size() && !err) {
		a.size = atom.size;
		a.type = 0;

		if (atom.size >= 8) {
			a.size = _fd->readUint32BE();
			a.type = _fd->readUint32BE();

			// Some QuickTime videos with resource forks have mdat chunks
			// that are of size 0. Adjust it so it's the correct size.
			if (a.type == MKTAG('m', 'd', 'a', 't') && a.size == 0)
				a.size = _fd->size();
		}

		total_size += 8;
		a.offset += 8;
		debug(4, "type: %08x  %.4s  sz: %x %x %x", a.type, tag2str(a.type), a.size, atom.size, total_size);

		if (a.size == 1) { // 64 bit extended size
			warning("64 bit extended size is not supported in QuickTime");
			return -1;
		}

		if (a.size == 0) {
			a.size = atom.size - total_size;
			if (a.size <= 8)
				break;
		}

		uint32 i = 0;

		for (; _parseTable[i].type != 0 && _parseTable[i].type != a.type; i++)
			; // Empty

		if (a.size < 8)
			break;

		a.size -= 8;

		if (a.size + (uint32)_fd->pos() > (uint32)_fd->size()) {
			_fd->seek(_fd->size());
			debug(0, "Skipping junk found at the end of the QuickTime file");
			return 0;
		} else if (_parseTable[i].type == 0) { // skip leaf atom data
			debug(0, ">>> Skipped [%s]", tag2str(a.type));

			_fd->seek(a.size, SEEK_CUR);
		} else {
			uint32 start_pos = _fd->pos();
			err = (this->*_parseTable[i].func)(a);

			uint32 left = a.size - _fd->pos() + start_pos;

			if (left > 0) // skip garbage at atom end
				_fd->seek(left, SEEK_CUR);
		}

		a.offset += a.size;
		total_size += a.size;
	}

	if (!err && total_size < atom.size)
		_fd->seek(atom.size - total_size, SEEK_SET);

	return err;
}

int QuickTimeParser::readLeaf(Atom atom) {
	if (atom.size > 1)
		_fd->seek(atom.size, SEEK_SET);

	return 0;
}

int QuickTimeParser::readMOOV(Atom atom) {
	if (readDefault(atom) < 0)
		return -1;

	// We parsed the 'moov' atom, so we don't need anything else
	_foundMOOV = true;
	return 1;
}

int QuickTimeParser::readCMOV(Atom atom) {
#ifdef USE_ZLIB
	// Read in the dcom atom
	_fd->readUint32BE();
	if (_fd->readUint32BE() != MKTAG('d', 'c', 'o', 'm'))
		return -1;
	if (_fd->readUint32BE() != MKTAG('z', 'l', 'i', 'b')) {
		warning("Unknown cmov compression type");
		return -1;
	}

	// Read in the cmvd atom
	uint32 compressedSize = _fd->readUint32BE() - 12;
	if (_fd->readUint32BE() != MKTAG('c', 'm', 'v', 'd'))
		return -1;
	uint32 uncompressedSize = _fd->readUint32BE();

	// Read in data
	byte *compressedData = (byte *)malloc(compressedSize);
	_fd->read(compressedData, compressedSize);

	// Create uncompressed stream
	byte *uncompressedData = (byte *)malloc(uncompressedSize);

	// Uncompress the data
	unsigned long dstLen = uncompressedSize;
	if (!uncompress(uncompressedData, &dstLen, compressedData, compressedSize)) {
		warning ("Could not uncompress cmov chunk");
		free(compressedData);
		free(uncompressedData);
		return -1;
	}

	// Load data into a new MemoryReadStream and assign _fd to be that
	SeekableReadStream *oldStream = _fd;
	_fd = new MemoryReadStream(uncompressedData, uncompressedSize, DisposeAfterUse::YES);

	// Read the contents of the uncompressed data
	Atom a = { MKTAG('m', 'o', 'o', 'v'), 0, uncompressedSize };
	int err = readDefault(a);

	// Assign the file handle back to the original handle
	free(compressedData);
	delete _fd;
	_fd = oldStream;

	return err;
#else
	warning ("zlib not found, cannot read QuickTime cmov atom");
	return -1;
#endif
}

int QuickTimeParser::readMVHD(Atom atom) {
	byte version = _fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	if (version == 1) {
		warning("QuickTime version 1");
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	_timeScale = _fd->readUint32BE(); // time scale
	debug(0, "time scale = %i\n", _timeScale);

	// duration
	_duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE();
	_fd->readUint32BE(); // preferred scale

	_fd->readUint16BE(); // preferred volume

	_fd->seek(10, SEEK_CUR); // reserved

	// We only need two values from the movie display matrix. Most of the values are just
	// skipped. xMod and yMod are 16:16 fixed point numbers, the last part of the 3x3 matrix
	// is 2:30.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	_scaleFactorX = Rational(0x10000, xMod);
	_scaleFactorY = Rational(0x10000, yMod);

	_scaleFactorX.debugPrint(1, "readMVHD(): scaleFactorX =");
	_scaleFactorY.debugPrint(1, "readMVHD(): scaleFactorY =");

	_fd->readUint32BE(); // preview time
	_fd->readUint32BE(); // preview duration
	_fd->readUint32BE(); // poster time
	_fd->readUint32BE(); // selection time
	_fd->readUint32BE(); // selection duration
	_fd->readUint32BE(); // current time
	_fd->readUint32BE(); // next track ID

	return 0;
}

int QuickTimeParser::readTRAK(Atom atom) {
	Track *track = new Track();

	if (!track)
		return -1;

	track->codecType = CODEC_TYPE_MOV_OTHER;
	track->startTime = 0; // XXX: check
	_tracks.push_back(track);

	return readDefault(atom);
}

int QuickTimeParser::readTKHD(Atom atom) {
	Track *track = _tracks.back();
	byte version = _fd->readByte();

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags
	//
	//MOV_TRACK_ENABLED 0x0001
	//MOV_TRACK_IN_MOVIE 0x0002
	//MOV_TRACK_IN_PREVIEW 0x0004
	//MOV_TRACK_IN_POSTER 0x0008
	//

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	/* track->id = */_fd->readUint32BE(); // track id (NOT 0 !)
	_fd->readUint32BE(); // reserved
	track->duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // highlevel (considering edits) duration in movie timebase
	_fd->readUint32BE(); // reserved
	_fd->readUint32BE(); // reserved

	_fd->readUint16BE(); // layer
	_fd->readUint16BE(); // alternate group
	_fd->readUint16BE(); // volume
	_fd->readUint16BE(); // reserved

	// We only need the two values from the displacement matrix for a track.
	// See readMVHD() for more information.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	track->scaleFactorX = Rational(0x10000, xMod);
	track->scaleFactorY = Rational(0x10000, yMod);

	track->scaleFactorX.debugPrint(1, "readTKHD(): scaleFactorX =");
	track->scaleFactorY.debugPrint(1, "readTKHD(): scaleFactorY =");

	// these are fixed-point, 16:16
	//_fd->readUint32BE() >> 16; // track width
	//_fd->readUint32BE() >> 16; // track height

	return 0;
}

// edit list atom
int QuickTimeParser::readELST(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->editCount = _fd->readUint32BE();
	track->editList = new EditListEntry[track->editCount];

	debug(2, "Track %d edit list count: %d", _tracks.size() - 1, track->editCount);

	uint32 offset = 0;

	for (uint32 i = 0; i < track->editCount; i++){
		track->editList[i].trackDuration = _fd->readUint32BE();
		track->editList[i].mediaTime = _fd->readSint32BE();
		track->editList[i].mediaRate = Rational(_fd->readUint32BE(), 0x10000);
		track->editList[i].timeOffset = offset;
		debugN(3, "\tDuration = %d (Offset = %d), Media Time = %d, ", track->editList[i].trackDuration, offset, track->editList[i].mediaTime);
		track->editList[i].mediaRate.debugPrint(3, "Media Rate =");
		offset += track->editList[i].trackDuration;
	}

	return 0;
}

int QuickTimeParser::readHDLR(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	// component type
	uint32 ctype = _fd->readUint32BE();
	uint32 type = _fd->readUint32BE(); // component subtype

	debug(0, "ctype= %s (0x%08lx)", tag2str(ctype), (long)ctype);
	debug(0, "stype= %s", tag2str(type));

	if (ctype == MKTAG('m', 'h', 'l', 'r')) // MOV
		debug(0, "MOV detected");
	else if (ctype == 0)
		debug(0, "MPEG-4 detected");

	if (type == MKTAG('v', 'i', 'd', 'e'))
		track->codecType = CODEC_TYPE_VIDEO;
	else if (type == MKTAG('s', 'o', 'u', 'n'))
		track->codecType = CODEC_TYPE_AUDIO;

	_fd->readUint32BE(); // component manufacture
	_fd->readUint32BE(); // component flags
	_fd->readUint32BE(); // component flags mask

	if (atom.size <= 24)
		return 0; // nothing left to read

	// .mov: PASCAL string
	byte len = _fd->readByte();
	_fd->seek(len, SEEK_CUR);

	_fd->seek(atom.size - (_fd->pos() - atom.offset), SEEK_CUR);

	return 0;
}

int QuickTimeParser::readMDHD(Atom atom) {
	Track *track = _tracks.back();
	byte version = _fd->readByte();

	if (version > 1)
		return 1; // unsupported

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	track->timeScale = _fd->readUint32BE();
	track->mediaDuration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // duration

	_fd->readUint16BE(); // language
	_fd->readUint16BE(); // quality

	return 0;
}

int QuickTimeParser::readSTSD(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	uint32 entryCount = _fd->readUint32BE();
	track->sampleDescs.resize(entryCount);

	for (uint32 i = 0; i < entryCount; i++) { // Parsing Sample description table
		Atom a = { 0, 0, 0 };
		uint32 start_pos = _fd->pos();
		int size = _fd->readUint32BE(); // size
		uint32 format = _fd->readUint32BE(); // data format

		_fd->readUint32BE(); // reserved
		_fd->readUint16BE(); // reserved
		_fd->readUint16BE(); // index

		track->sampleDescs[i] = readSampleDesc(track, format);

		debug(0, "size=%d 4CC= %s codec_type=%d", size, tag2str(format), track->codecType);

		if (!track->sampleDescs[i]) {
			// other codec type, just skip (rtp, mp4s, tmcd ...)
			_fd->seek(size - (_fd->pos() - start_pos), SEEK_CUR);
		}

		// this will read extra atoms at the end (wave, alac, damr, avcC, SMI ...)
		a.size = size - (_fd->pos() - start_pos);
		if (a.size > 8)
			readDefault(a);
		else if (a.size > 0)
			_fd->seek(a.size, SEEK_CUR);
	}

	return 0;
}

int QuickTimeParser::readSTSC(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->sampleToChunkCount = _fd->readUint32BE();

	debug(0, "track[%i].stsc.entries = %i", _tracks.size() - 1, track->sampleToChunkCount);

	track->sampleToChunk = new SampleToChunkEntry[track->sampleToChunkCount];

	if (!track->sampleToChunk)
		return -1;

	for (uint32 i = 0; i < track->sampleToChunkCount; i++) {
		track->sampleToChunk[i].first = _fd->readUint32BE() - 1;
		track->sampleToChunk[i].count = _fd->readUint32BE();
		track->sampleToChunk[i].id = _fd->readUint32BE();
		//warning("Sample to Chunk[%d]: First = %d, Count = %d", i, track->sampleToChunk[i].first, track->sampleToChunk[i].count);
	}

	return 0;
}

int QuickTimeParser::readSTSS(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->keyframeCount = _fd->readUint32BE();

	debug(0, "keyframeCount = %d", track->keyframeCount);

	track->keyframes = new uint32[track->keyframeCount];

	if (!track->keyframes)
		return -1;

	for (uint32 i = 0; i < track->keyframeCount; i++) {
		track->keyframes[i] = _fd->readUint32BE() - 1; // Adjust here, the frames are based on 1
		debug(6, "keyframes[%d] = %d", i, track->keyframes[i]);

	}
	return 0;
}

int QuickTimeParser::readSTSZ(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->sampleSize = _fd->readUint32BE();
	track->sampleCount = _fd->readUint32BE();

	debug(5, "sampleSize = %d sampleCount = %d", track->sampleSize, track->sampleCount);

	if (track->sampleSize)
		return 0; // there isn't any table following

	track->sampleSizes = new uint32[track->sampleCount];

	if (!track->sampleSizes)
		return -1;

	for(uint32 i = 0; i < track->sampleCount; i++) {
		track->sampleSizes[i] = _fd->readUint32BE();
		debug(6, "sampleSizes[%d] = %d", i, track->sampleSizes[i]);
	}

	return 0;
}

int QuickTimeParser::readSTTS(Atom atom) {
	Track *track = _tracks.back();
	uint32 totalSampleCount = 0;

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->timeToSampleCount = _fd->readUint32BE();
	track->timeToSample = new TimeToSampleEntry[track->timeToSampleCount];

	debug(0, "track[%d].stts.entries = %d", _tracks.size() - 1, track->timeToSampleCount);

	for (int32 i = 0; i < track->timeToSampleCount; i++) {
		track->timeToSample[i].count = _fd->readUint32BE();
		track->timeToSample[i].duration = _fd->readUint32BE();

		debug(1, "\tCount = %d, Duration = %d", track->timeToSample[i].count, track->timeToSample[i].duration);

		totalSampleCount += track->timeToSample[i].count;
	}

	track->frameCount = totalSampleCount;
	return 0;
}

int QuickTimeParser::readSTCO(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->chunkCount = _fd->readUint32BE();
	track->chunkOffsets = new uint32[track->chunkCount];

	if (!track->chunkOffsets)
		return -1;

	for (uint32 i = 0; i < track->chunkCount; i++) {
		// WORKAROUND/HACK: The offsets in Riven videos (ones inside the Mohawk archives themselves)
		// have offsets relative to the archive and not the video. This is quite nasty. We subtract
		// the initial offset of the stream to get the correct value inside of the stream.
		track->chunkOffsets[i] = _fd->readUint32BE() - _beginOffset;
	}

	return 0;
}

int QuickTimeParser::readWAVE(Atom atom) {
	if (_tracks.empty())
		return 0;

	Track *track = _tracks.back();

	if (atom.size > (1 << 30))
		return -1;

	if (track->sampleDescs[0]->getCodecTag() == MKTAG('Q', 'D', 'M', '2')) // Read extra data for QDM2
		track->extraData = _fd->readStream(atom.size);
	else if (atom.size > 8)
		return readDefault(atom);
	else
		_fd->skip(atom.size);

	return 0;
}

enum {
	kMP4IODescTag          = 2,
	kMP4ESDescTag          = 3,
	kMP4DecConfigDescTag   = 4,
	kMP4DecSpecificDescTag = 5
};

static int readMP4DescLength(SeekableReadStream *stream) {
	int length = 0;
	int count = 4;

	while (count--) {
		byte c = stream->readByte();
		length = (length << 7) | (c & 0x7f);

		if (!(c & 0x80))
			break;
	}

	return length;
}

static void readMP4Desc(SeekableReadStream *stream, byte &tag, int &length) {
	tag = stream->readByte();
	length = readMP4DescLength(stream);
}

int QuickTimeParser::readESDS(Atom atom) {
	if (_tracks.empty())
		return 0;

	Track *track = _tracks.back();

	_fd->readUint32BE(); // version + flags

	byte tag;
	int length;

	readMP4Desc(_fd, tag, length);
	_fd->readUint16BE(); // id
	if (tag == kMP4ESDescTag)
		_fd->readByte(); // priority

	// Check if we've got the Config MPEG-4 header
	readMP4Desc(_fd, tag, length);
	if (tag != kMP4DecConfigDescTag)
		return 0;

	track->objectTypeMP4 = _fd->readByte();
	_fd->readByte();                      // stream type
	_fd->readUint16BE(); _fd->readByte(); // buffer size
	_fd->readUint32BE();                  // max bitrate
	_fd->readUint32BE();                  // avg bitrate

	// Check if we've got the Specific MPEG-4 header
	readMP4Desc(_fd, tag, length);
	if (tag != kMP4DecSpecificDescTag)
		return 0;

	track->extraData = _fd->readStream(length);

	debug(0, "MPEG-4 object type = %02x", track->objectTypeMP4);
	return 0;
}

int QuickTimeParser::readSMI(Atom atom) {
	if (_tracks.empty())
		return 0;

	Track *track = _tracks.back();

	// This atom just contains SVQ3 extra data
	track->extraData = _fd->readStream(atom.size);

	return 0;
}

void QuickTimeParser::close() {
	for (uint32 i = 0; i < _tracks.size(); i++)
		delete _tracks[i];

	_tracks.clear();

	if (_disposeFileHandle == DisposeAfterUse::YES)
		delete _fd;

	_fd = 0;
}

QuickTimeParser::SampleDesc::SampleDesc(Track *parentTrack, uint32 codecTag) {
	_parentTrack = parentTrack;
	_codecTag = codecTag;
}

QuickTimeParser::Track::Track() {
	chunkCount = 0;
	chunkOffsets = 0;
	timeToSampleCount = 0;
	timeToSample = 0;
	sampleToChunkCount = 0;
	sampleToChunk = 0;
	sampleSize = 0;
	sampleCount = 0;
	sampleSizes = 0;
	keyframeCount = 0;
	keyframes = 0;
	timeScale = 0;
	width = 0;
	height = 0;
	codecType = CODEC_TYPE_MOV_OTHER;
	editCount = 0;
	editList = 0;
	extraData = 0;
	frameCount = 0;
	duration = 0;
	startTime = 0;
	objectTypeMP4 = 0;
	mediaDuration = 0;
}

QuickTimeParser::Track::~Track() {
	delete[] chunkOffsets;
	delete[] timeToSample;
	delete[] sampleToChunk;
	delete[] sampleSizes;
	delete[] keyframes;
	delete[] editList;
	delete extraData;

	for (uint32 i = 0; i < sampleDescs.size(); i++)
		delete sampleDescs[i];
}

} // End of namespace Video
