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

#ifndef GOB_DBASE_H
#define GOB_DBASE_H

#include "common/system.h"
#include "common/util.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/array.h"

namespace Gob {

/**
 * A class for reading dBase files.
 *
 * Only dBase III files supported for now, and only field type
 * string is actually useful. Further missing is reading of MDX
 * index files and support for the external "Memo" data file.
 */
class dBase {
public:
	enum Type {
		kTypeString = 0x43, // 'C'
		kTypeDate   = 0x44, // 'D'
		kTypeBool   = 0x4C, // 'L'
		kTypeMemo   = 0x4D, // 'M'
		kTypeNumber = 0x4E  // 'N'
	};

	/** A field description. */
	struct Field {
		Common::String name; ///< Name of the field.

		Type  type;     ///< Type of the field.
		uint8 size;     ///< Size of raw field data in bytes.
		uint8 decimals; ///< Number of decimals the field holds.
	};

	/** A record. */
	struct Record {
		bool deleted; ///< Has this record been deleted?
		Common::Array<const byte *> fields; ///< Raw field data.
	};

	dBase();
	~dBase();

	bool load(Common::SeekableReadStream &stream);
	void clear();

	byte getVersion() const;

	/** Return the date the database was last updated. */
	TimeDate getLastUpdate() const;

	const Common::Array<Field>  &getFields()  const;
	const Common::Array<Record> &getRecords() const;

	/** Extract a string out of raw field data. */
	Common::String getString(const Record &record, int field) const;

private:
	byte _version;
	bool _hasMemo;

	TimeDate _lastUpdate;

	Common::Array<Field>  _fields;
	Common::Array<Record> _records;

	byte *_recordData;

	static inline uint32 stringLength(const byte *data, uint32 max);
	static inline Common::String readString(Common::SeekableReadStream &stream, int n);
};

} // End of namespace Gob

#endif // GOB_DBASE_H
