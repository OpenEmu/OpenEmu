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

#ifndef SCUMM_SMUSH_CHANNEL_H
#define SCUMM_SMUSH_CHANNEL_H

#include "common/util.h"

namespace Common {
class SeekableReadStream;
}

namespace Scumm {

class SmushChannel {
protected:
	int32 _track;				///< the track number
	byte *_tbuffer;	///< data temporary buffer
	int32 _tbufferSize;			///< temporary buffer size
	byte *_sbuffer;	///< sound buffer
	int32 _sbufferSize;			///< sound buffer size

	int32 _dataSize;			///< remaining size of sound data in the iMUS buffer

	bool _inData;

	int32 _volume;
	int32 _pan;

	void processBuffer();

	virtual bool handleSubTags(int32 &offset) = 0;

public:
	SmushChannel(int32 track);
	virtual ~SmushChannel();
	virtual bool appendData(Common::SeekableReadStream &b, int32 size) = 0;
	virtual bool setParameters(int32, int32, int32, int32, int32) = 0;
	virtual bool checkParameters(int32, int32, int32, int32, int32) = 0;
	virtual bool isTerminated() const = 0;
	virtual byte *getSoundData() = 0;
	virtual int32 getRate() = 0;
	virtual bool getParameters(bool &stereo, bool &is_16bit, int32 &vol, int32 &pan) = 0;

	int32 getAvailableSoundDataSize() const { return _sbufferSize; }
	int32 getTrackIdentifier() const { return _track; }
};

class SaudChannel : public SmushChannel {
private:
	int32 _nbframes;
	bool _markReached;
	int32 _flags;
	int32 _index;
	bool _keepSize;

protected:
	bool handleSubTags(int32 &offset);

public:
	SaudChannel(int32 track);
	bool isTerminated() const;
	bool setParameters(int32 duration, int32 flags, int32 vol1, int32 vol2, int32 index);
	bool checkParameters(int32 index, int32 duration, int32 flags, int32 vol1, int32 vol2);
	bool appendData(Common::SeekableReadStream &b, int32 size);
	byte *getSoundData();
	int32 getRate() { return 22050; }
	bool getParameters(bool &stereo, bool &is_16bit, int32 &vol, int32 &pan) {
		stereo = false;
		is_16bit = false;
		vol = _volume;
		pan = _pan;
		return true;
	}
};

class ImuseChannel : public SmushChannel {
private:
	int32 _srbufferSize;

	int32 _bitsize;			///< the bitsize of the original data
	int32 _rate;				///< the sampling rate of the original data
	int32 _channels;			///< the number of channels of the original data

protected:
	void decode();
	bool handleMap(byte *data);
	bool handleSubTags(int32 &offset);

public:
	ImuseChannel(int32 track);
	bool isTerminated() const;
	bool setParameters(int32 nbframes, int32 size, int32 track_flags, int32 unk1, int32);
	bool checkParameters(int32 index, int32 nbframes, int32 size, int32 track_flags, int32 unk1);
	bool appendData(Common::SeekableReadStream &b, int32 size);
	byte *getSoundData();
	int32 getRate() { return _rate; }
	bool getParameters(bool &stereo, bool &is_16bit, int32 &vol, int32 &pan) {
		stereo = (_channels == 2);
		is_16bit = (_bitsize > 8);
		vol = _volume;
		pan = _pan;
		return true;
	}
};

} // End of namespace Scumm

#endif
