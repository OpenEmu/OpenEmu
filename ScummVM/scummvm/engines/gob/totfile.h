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

#ifndef GOB_TOTFILE_H
#define GOB_TOTFILE_H

#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Gob {

class TOTFile {
public:
	enum Function {
		kFunctionStart  =  0,
		kFunctionCenter = 13
	};

	struct Properties {
		uint8  versionMajor;
		uint8  versionMinor;
		uint32 variablesCount;
		uint32 textsOffset;
		uint32 resourcesOffset;
		uint16 animDataSize;
		uint8  imFileNumber;
		uint8  exFileNumber;
		uint8  communHandling;
		uint16 functions[14];
		uint32 scriptEnd;
		uint32 textsSize;
		uint32 resourcesSize;
	};

	TOTFile(GobEngine *vm);
	~TOTFile();

	bool load(const Common::String &fileName);
	void unload();

	Common::SeekableReadStream *getStream() const;
	bool getProperties(Properties &props) const;

	static Common::String createFileName(const Common::String &base, bool &isLOM);
	static Common::String getFileBase(const Common::String &fileName);

private:
	GobEngine *_vm;

	Common::SeekableReadStream *_stream;

	byte _header[128];
};

} // End of namespace Gob

#endif // GOB_TOTFILE_H
