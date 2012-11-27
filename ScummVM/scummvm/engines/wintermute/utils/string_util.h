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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_STRINGUTIL_H
#define WINTERMUTE_STRINGUTIL_H

#include "engines/wintermute/dctypes.h"

namespace Wintermute {

class StringUtil {
public:
	static bool compareNoCase(const AnsiString &str1, const AnsiString &str2);
	//static bool compareNoCase(const WideString &str1, const WideString &str2);
	static WideString utf8ToWide(const Utf8String &Utf8Str);
	static Utf8String wideToUtf8(const WideString &WideStr);
	static WideString ansiToWide(const AnsiString &str);
	static AnsiString wideToAnsi(const WideString &str);

	static bool isUtf8BOM(const byte *buffer, uint32 bufferSize);
	static int indexOf(const WideString &str, const WideString &toFind, size_t startFrom);

	static Common::String encodeSetting(const Common::String &str);
	static Common::String decodeSetting(const Common::String &str);

	static AnsiString toString(int val);
};

} // end of namespace Wintermute

#endif
