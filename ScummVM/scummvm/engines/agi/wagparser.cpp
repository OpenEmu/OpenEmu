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

#include "common/file.h"
#include "common/util.h"
#include "common/fs.h"
#include "common/debug.h"
#include "common/textconsole.h"

#include "agi/wagparser.h"

namespace Agi {

WagProperty::WagProperty() {
	setDefaults();
}

WagProperty::~WagProperty() {
	deleteData();
}

WagProperty::WagProperty(const WagProperty &other) {
	deepCopy(other);
}

WagProperty &WagProperty::operator=(const WagProperty &other) {
	if (&other != this) deepCopy(other); // Don't do self-assignment
	return *this;
}

void WagProperty::deepCopy(const WagProperty &other) {
	_readOk   = other._readOk;
	_propCode = other._propCode;
	_propType = other._propType;
	_propNum  = other._propNum;
	_propSize = other._propSize;

	deleteData(); // Delete old data (If any) and set _propData to NULL
	if (other._propData != NULL) {
		_propData = new char[other._propSize + 1UL]; // Allocate space for property's data plus trailing zero
		memcpy(_propData, other._propData, other._propSize + 1UL); // Copy the whole thing
	}
}

bool WagProperty::read(Common::SeekableReadStream &stream) {
	// First read the property's header
	_propCode = (enum WagPropertyCode) stream.readByte();
	_propType = (enum WagPropertyType) stream.readByte();
	_propNum  = stream.readByte();
	_propSize = stream.readUint16LE();

	if (stream.eos() || stream.err()) { // Check that we got the whole header
		_readOk = false;
		return _readOk;
	}

	// Then read the property's data
	deleteData(); // Delete old data (If any)
	_propData = new char[_propSize + 1UL]; // Allocate space for property's data plus trailing zero
	uint32 readBytes = stream.read(_propData, _propSize); // Read the data in
	_propData[_propSize] = 0; // Set the trailing zero for easy C-style string access

	_readOk = (_propData != NULL && readBytes == _propSize); // Check that we got the whole data
	return _readOk;
}

void WagProperty::clear() {
	deleteData();
	setDefaults();
}

void WagProperty::setDefaults() {
	_readOk   = false;
	_propCode = PC_UNDEFINED;
	_propType = PT_UNDEFINED;
	_propNum  = 0;
	_propSize = 0;
	_propData = NULL;
}

void WagProperty::deleteData() {
	delete _propData;
	_propData = NULL;
}

WagFileParser::WagFileParser() :
	_parsedOk(false) {
}

WagFileParser::~WagFileParser() {
}

bool WagFileParser::checkAgiVersionProperty(const WagProperty &version) const {
	if (version.getCode() == WagProperty::PC_INTVERSION && // Must be AGI interpreter version property
		version.getSize() >= 3 && // Need at least three characters for a version number like "X.Y"
		Common::isDigit(version.getData()[0]) && // And the first character must be a digit
		(version.getData()[1] == ',' || version.getData()[1] == '.')) { // And the second a comma or a period

		for (int i = 2; i < version.getSize(); i++) // And the rest must all be digits
			if (!Common::isDigit(version.getData()[i]))
				return false; // Bail out if found a non-digit after the decimal point

		return true;
	} else // Didn't pass the preliminary test so fails
		return false;
}

uint16 WagFileParser::convertToAgiVersionNumber(const WagProperty &version) {
	// Examples of the conversion: "2.44" -> 0x2440, "2.917" -> 0x2917, "3.002086" -> 0x3086.
	if (checkAgiVersionProperty(version)) { // Check that the string is a valid AGI interpreter version string
		// Convert first ascii digit to an integer and put it in the fourth nibble (Bits 12...15) of the version number
		// and at the same time set all other nibbles to zero.
		uint16 agiVerNum = ((uint16) (version.getData()[0] - '0')) << (3 * 4);

		// Convert at most three least significant digits of the version number's minor part
		// (i.e. the part after the decimal point) and put them in order to the third, second
		// and the first nibble of the version number. Just to clarify version.getSize() - 2
		// is the number of digits after the decimal point.
		int32 digitCount = MIN<int32>(3, ((int32) version.getSize()) - 2); // How many digits left to convert
		for (int i = 0; i < digitCount; i++)
			agiVerNum |= ((uint16) (version.getData()[version.getSize() - digitCount + i] - '0')) << ((2 - i) * 4);

		debug(3, "WagFileParser: Converted AGI version from string %s to number 0x%x", version.getData(), agiVerNum);
		return agiVerNum;
	} else // Not a valid AGI interpreter version string
		return 0; // Can't convert, so failure
}

bool WagFileParser::checkWagVersion(Common::SeekableReadStream &stream) {
	if (stream.size() >= WINAGI_VERSION_LENGTH) { // Stream has space to contain the WinAGI version string
		// Read the last WINAGI_VERSION_LENGTH bytes of the stream and make a string out of it
		char str[WINAGI_VERSION_LENGTH+1]; // Allocate space for the trailing zero also
		uint32 oldStreamPos = stream.pos(); // Save the old stream position
		stream.seek(stream.size() - WINAGI_VERSION_LENGTH);
		uint32 readBytes = stream.read(str, WINAGI_VERSION_LENGTH);
		stream.seek(oldStreamPos); // Seek back to the old stream position
		str[readBytes] = 0; // Set the trailing zero to finish the C-style string
		if (readBytes != WINAGI_VERSION_LENGTH) { // Check that we got the whole version string
			debug(3, "WagFileParser::checkWagVersion: Error reading WAG file version from stream");
			return false;
		}
		debug(3, "WagFileParser::checkWagVersion: Read WinAGI version string (\"%s\")", str);

		// Check that the WinAGI version string is one of the two version strings
		// WinAGI 1.1.21 recognizes as acceptable in the end of a *.wag file.
		// Note that they are all of length 16 and are padded with spaces to be that long.
		return scumm_stricmp(str, "WINAGI v1.0     ") == 0 ||
			scumm_stricmp(str, "1.0 BETA        ") == 0;
	} else { // Stream is too small to contain the WinAGI version string
		debug(3, "WagFileParser::checkWagVersion: Stream is too small to contain a valid WAG file");
		return false;
	}
}

bool WagFileParser::parse(const Common::FSNode &node) {
	WagProperty property; // Temporary property used for reading
	Common::SeekableReadStream *stream = NULL; // The file stream

	_parsedOk = false; // We haven't parsed the file yet

	stream = node.createReadStream(); // Open the file
	if (stream) { // Check that opening the file was succesful
		if (checkWagVersion(*stream)) { // Check that WinAGI version string is valid
			// It seems we've got a valid *.wag file so let's parse its properties from the start.
			stream->seek(0); // Rewind the stream
			if (!_propList.empty()) _propList.clear(); // Clear out old properties (If any)

			do { // Parse the properties
				if (property.read(*stream)) { // Read the property and check it was read ok
					_propList.push_back(property); // Add read property to properties list
					debug(4, "WagFileParser::parse: Read property with code %d, type %d, number %d, size %d, data \"%s\"",
						property.getCode(), property.getType(), property.getNumber(), property.getSize(), property.getData());
				} else // Reading failed, let's bail out
					break;
			} while (!endOfProperties(*stream)); // Loop until the end of properties

			// File was parsed successfully only if we got to the end of properties
			// and all the properties were read successfully (Also the last).
			_parsedOk = endOfProperties(*stream) && property.readOk();

			if (!_parsedOk) // Error parsing stream
				warning("Error parsing WAG file (%s). WAG file ignored", node.getPath().c_str());
		} else // Invalid WinAGI version string or it couldn't be read
			warning("Invalid WAG file (%s) version or error reading it. WAG file ignored", node.getPath().c_str());
	} else // Couldn't open file
		warning("Couldn't open WAG file (%s). WAG file ignored", node.getPath().c_str());

	delete stream;
	return _parsedOk;
}

const WagProperty *WagFileParser::getProperty(const WagProperty::WagPropertyCode code) const {
	for (PropertyList::const_iterator iter = _propList.begin(); iter != _propList.end(); ++iter)
		if (iter->getCode() == code) return iter;
	return NULL;
}

bool WagFileParser::endOfProperties(const Common::SeekableReadStream &stream) const {
	return stream.pos() >= (stream.size() - WINAGI_VERSION_LENGTH);
}

} // End of namespace Agi
