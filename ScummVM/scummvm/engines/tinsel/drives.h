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
 * CD/drive handling functions
 */

#ifndef TINSEL_DRIVES_H
#define TINSEL_DRIVES_H

#include "common/coroutines.h"
#include "common/stream.h"
#include "tinsel/dw.h"

namespace Tinsel {

// flags2
#define fCd1	0x00000001L
#define fCd2	0x00000002L
#define fCd3	0x00000004L
#define fCd4	0x00000008L
#define fCd5	0x00000010L
#define fCd6	0x00000020L
#define fCd7	0x00000040L
#define fCd8	0x00000080L

#define fAllCds	(fCd1|fCd2|fCd3|fCd4|fCd5|fCd6|fCd7|fCd8)

void DoCdChange();

void CdCD(CORO_PARAM);

int GetCurrentCD();

void SetCD(int flags);

int GetCD(int flags);

void SetNextCD(int cdNumber);

bool GotoCD();

class TinselFile : public Common::SeekableReadStream, public Common::ReadStreamEndian {
private:
	static bool _warningShown;
	Common::SeekableReadStream *_stream;
	bool openInternal(const Common::String &filename);
public:
	TinselFile();
	~TinselFile();
	bool open(const Common::String &filename);
	void close();
	char getCdNumber();

	bool err() const;
	void clearErr();

	bool eos() const;
	uint32 read(void *dataPtr, uint32 dataSize);

	int32 pos() const;
	int32 size() const;
	bool seek(int32 offset, int whence = SEEK_SET);
};


} // End of namespace Tinsel

#endif /* TINSEL_DRIVES_H */
