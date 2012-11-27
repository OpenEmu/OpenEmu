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

#include "gob/dbase.h"

namespace Gob {

dBase::dBase() : _recordData(0) {
	clear();
}

dBase::~dBase() {
	clear();
}

bool dBase::load(Common::SeekableReadStream &stream) {
	clear();

	uint32 startPos = stream.pos();

	_version = stream.readByte();
	if ((_version != 0x03) && (_version != 0x83))
		// Unsupported version
		return false;

	// TODO: Add support for memo files. A memo file is an external data file
	//       .DBT, segmented into "blocks". Each memo field in a record is an
	//       index this file.
	_hasMemo = (_version & 0x80) != 0;

	_lastUpdate.tm_year = stream.readByte();
	_lastUpdate.tm_mon  = stream.readByte() - 1;
	_lastUpdate.tm_mday = stream.readByte();
	_lastUpdate.tm_hour = 0;
	_lastUpdate.tm_min  = 0;
	_lastUpdate.tm_sec  = 0;

	uint32 recordCount = stream.readUint32LE();
	uint32 headerSize  = stream.readUint16LE();
	uint32 recordSize  = stream.readUint16LE();

	stream.skip(20); // Reserved

	// Read all field descriptions, 0x0D is the end marker
	uint32 fieldsLength = 0;
	while (!stream.eos() && !stream.err() && (stream.readByte() != 0x0D)) {
		Field field;

		stream.seek(-1, SEEK_CUR);

		field.name = readString(stream, 11);
		field.type = (Type) stream.readByte();

		stream.skip(4); // Field data address

		field.size     = stream.readByte();
		field.decimals = stream.readByte();

		fieldsLength += field.size;

		stream.skip(14); // Reserved and/or useless for us

		_fields.push_back(field);
	}

	if (stream.eos() || stream.err())
		return false;

	if ((stream.pos() - startPos) != headerSize)
		// Corrupted file / unknown format
		return false;

	if (recordSize != (fieldsLength + 1))
		// Corrupted file / unknown format
		return false;

	_recordData = new byte[recordSize * recordCount];
	if (stream.read(_recordData, recordSize * recordCount) != (recordSize * recordCount))
		return false;

	if (stream.readByte() != 0x1A)
		// Missing end marker
		return false;

	uint32 fieldCount = _fields.size();

	// Create the records array
	_records.resize(recordCount);
	for (uint32 i = 0; i < recordCount; i++) {
		Record &record = _records[i];
		const byte *data = _recordData + i * recordSize;

		char status = *data++;
		if ((status != ' ') && (status != '*'))
			// Corrupted file / unknown format
			return false;

		record.deleted = status == '*';

		record.fields.resize(fieldCount);
		for (uint32 j = 0; j < fieldCount; j++) {
			record.fields[j] = data;
			data += _fields[j].size;
		}
	}

	return true;
}

void dBase::clear() {
	memset(&_lastUpdate, 0, sizeof(_lastUpdate));

	_version = 0;
	_hasMemo = false;

	_fields.clear();
	_records.clear();

	delete[] _recordData;
	_recordData = 0;
}

byte dBase::getVersion() const {
	return _version;
}

TimeDate dBase::getLastUpdate() const {
	return _lastUpdate;
}

const Common::Array<dBase::Field> &dBase::getFields() const {
	return _fields;
}

const Common::Array<dBase::Record> &dBase::getRecords() const {
	return _records;
}

Common::String dBase::getString(const Record &record, int field) const {
	assert(_fields[field].type == kTypeString);

	uint32 fieldLength = stringLength(record.fields[field], _fields[field].size);
	return Common::String((const char *) record.fields[field], fieldLength);
}

// String fields are padded with spaces. This finds the real length.
inline uint32 dBase::stringLength(const byte *data, uint32 max) {
	while (max-- > 0)
		if ((data[max] != 0x20) && (data[max] != 0x00))
			return max + 1;

	return 0;
}

// Read a constant-length string out of a stream.
inline Common::String dBase::readString(Common::SeekableReadStream &stream, int n) {
	Common::String str;

	char c;
	while (n-- > 0) {
		if ((c = stream.readByte()) == '\0')
			break;

		str += c;
	}

	if (n > 0)
		stream.skip(n);

	return str;
}

} // End of namespace Gob
