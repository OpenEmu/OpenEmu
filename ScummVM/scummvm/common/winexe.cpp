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

#include "common/str.h"
#include "common/winexe.h"

namespace Common {

WinResourceID &WinResourceID::operator=(const String &x) {
	_name = x;
	_idType = kIDTypeString;
	return *this;
}

WinResourceID &WinResourceID::operator=(uint32 x) {
	_id = x;
	_idType = kIDTypeNumerical;
	return *this;
}

bool WinResourceID::operator==(const String &x) const {
	return _idType == kIDTypeString && _name.equalsIgnoreCase(x);
}

bool WinResourceID::operator==(const uint32 &x) const {
	return _idType == kIDTypeNumerical && _id == x;
}

bool WinResourceID::operator==(const WinResourceID &x) const {
	if (_idType != x._idType)
		return false;
	if (_idType == kIDTypeString)
		return _name.equalsIgnoreCase(x._name);
	if (_idType == kIDTypeNumerical)
		return _id == x._id;
	return true;
}

String WinResourceID::getString() const {
	if (_idType != kIDTypeString)
		return "";

	return _name;
}

uint32 WinResourceID::getID() const {
	if (_idType != kIDTypeNumerical)
		return 0xffffffff;

	return _id;
}

String WinResourceID::toString() const {
	if (_idType == kIDTypeString)
		return _name;
	else if (_idType == kIDTypeNumerical)
		return String::format("%08x", _id);

	return "";
}

} // End of namespace Common
