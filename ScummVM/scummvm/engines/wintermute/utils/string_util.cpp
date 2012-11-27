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

#include "common/tokenizer.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/utils/convert_utf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
bool StringUtil::compareNoCase(const AnsiString &str1, const AnsiString &str2) {
	return (str1.compareToIgnoreCase(str2) == 0);
}

//////////////////////////////////////////////////////////////////////////
/*bool StringUtil::CompareNoCase(const WideString &str1, const WideString &str2) {
    WideString str1lc = str1;
    WideString str2lc = str2;

    ToLowerCase(str1lc);
    ToLowerCase(str2lc);

    return (str1lc == str2lc);
}*/

//////////////////////////////////////////////////////////////////////////
WideString StringUtil::utf8ToWide(const Utf8String &Utf8Str) {
	error("StringUtil::Utf8ToWide - WideString not supported yet");
	/*  size_t WideSize = Utf8Str.size();

	    if (sizeof(wchar_t) == 2) {
	        wchar_t *WideStringNative = new wchar_t[WideSize + 1];

	        const UTF8 *SourceStart = reinterpret_cast<const UTF8 *>(Utf8Str.c_str());
	        const UTF8 *SourceEnd = SourceStart + WideSize;

	        UTF16 *TargetStart = reinterpret_cast<UTF16 *>(WideStringNative);
	        UTF16 *TargetEnd = TargetStart + WideSize + 1;

	        ConversionResult res = ConvertUTF8toUTF16(&SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
	        if (res != conversionOK) {
	            delete[] WideStringNative;
	            return L"";
	        }
	        *TargetStart = 0;
	        WideString ResultString(WideStringNative);
	        delete[] WideStringNative;

	        return ResultString;
	    } else if (sizeof(wchar_t) == 4) {
	        wchar_t *WideStringNative = new wchar_t[WideSize + 1];

	        const UTF8 *SourceStart = reinterpret_cast<const UTF8 *>(Utf8Str.c_str());
	        const UTF8 *SourceEnd = SourceStart + WideSize;

	        UTF32 *TargetStart = reinterpret_cast<UTF32 *>(WideStringNative);
	        UTF32 *TargetEnd = TargetStart + WideSize;

	        ConversionResult res = ConvertUTF8toUTF32(&SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
	        if (res != conversionOK) {
	            delete[] WideStringNative;
	            return L"";
	        }
	        *TargetStart = 0;
	        WideString ResultString(WideStringNative);
	        delete[] WideStringNative;

	        return ResultString;
	    } else {
	        return L"";
	    }*/
	return "";
}

//////////////////////////////////////////////////////////////////////////
Utf8String StringUtil::wideToUtf8(const WideString &WideStr) {
	error("StringUtil::wideToUtf8 - Widestring not supported yet");
	/*  size_t WideSize = WideStr.length();

	    if (sizeof(wchar_t) == 2) {
	        size_t utf8Size = 3 * WideSize + 1;
	        char *utf8StringNative = new char[Utf8Size];

	        const UTF16 *SourceStart = reinterpret_cast<const UTF16 *>(WideStr.c_str());
	        const UTF16 *SourceEnd = SourceStart + WideSize;

	        UTF8 *TargetStart = reinterpret_cast<UTF8 *>(Utf8StringNative);
	        UTF8 *TargetEnd = TargetStart + Utf8Size;

	        ConversionResult res = ConvertUTF16toUTF8(&SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
	        if (res != conversionOK) {
	            delete[] Utf8StringNative;
	            return (Utf8String)"";
	        }
	        *TargetStart = 0;
	        Utf8String ResultString(Utf8StringNative);
	        delete[] Utf8StringNative;
	        return ResultString;
	    } else if (sizeof(wchar_t) == 4) {
	        size_t utf8Size = 4 * WideSize + 1;
	        char *utf8StringNative = new char[Utf8Size];

	        const UTF32 *SourceStart = reinterpret_cast<const UTF32 *>(WideStr.c_str());
	        const UTF32 *SourceEnd = SourceStart + WideSize;

	        UTF8 *TargetStart = reinterpret_cast<UTF8 *>(Utf8StringNative);
	        UTF8 *TargetEnd = TargetStart + Utf8Size;

	        ConversionResult res = ConvertUTF32toUTF8(&SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
	        if (res != conversionOK) {
	            delete[] Utf8StringNative;
	            return (Utf8String)"";
	        }
	        *TargetStart = 0;
	        Utf8String ResultString(Utf8StringNative);
	        delete[] Utf8StringNative;
	        return ResultString;
	    } else {
	        return (Utf8String)"";
	    }*/
	return "";
}

// Currently this only does Ansi->ISO 8859, and only for carets.
char simpleAnsiToWide(const AnsiString &str, uint32 &offset) {
	byte c = str[offset];

	if (c == 146) {
		offset++;
		return 39; // Replace right-quote with apostrophe
	} else {
		offset++;
		return c;
	}
}

//////////////////////////////////////////////////////////////////////////
WideString StringUtil::ansiToWide(const AnsiString &str) {
	// TODO: This function gets called a lot, so warnings like these drown out the usefull information
	Common::String converted = "";
	uint32 index = 0;
	while (index != str.size()) {
	    converted += simpleAnsiToWide(str, index);
	}
	// using default os locale!

	/*  setlocale(LC_CTYPE, "");
	    size_t wideSize = mbstowcs(NULL, str.c_str(), 0) + 1;
	    wchar_t *wstr = new wchar_t[WideSize];
	    mbstowcs(wstr, str.c_str(), WideSize);
	    WideString ResultString(wstr);
	    delete[] wstr;
	    return ResultString;*/
	return WideString(converted);
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::wideToAnsi(const WideString &wstr) {
	// using default os locale!
	// TODO: This function gets called a lot, so warnings like these drown out the usefull information
	/*  setlocale(LC_CTYPE, "");
	    size_t wideSize = wcstombs(NULL, wstr.c_str(), 0) + 1;
	    char *str = new char[WideSize];
	    wcstombs(str, wstr.c_str(), WideSize);
	    AnsiString ResultString(str);
	    delete[] str;
	    return ResultString;*/
	return AnsiString(wstr);
}

//////////////////////////////////////////////////////////////////////////
bool StringUtil::isUtf8BOM(const byte *buffer, uint32 bufferSize) {
	if (bufferSize > 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
		return true;
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
int StringUtil::indexOf(const WideString &str, const WideString &toFind, size_t startFrom) {
	const char *index = strstr(str.c_str(), toFind.c_str());
	if (index == NULL) {
		return -1;
	} else {
		return index - str.c_str();
	}
}

Common::String StringUtil::encodeSetting(const Common::String &str) {
	if (str.contains('=')) {
		error("Setting contains '='");
	}
	return str;
}

Common::String StringUtil::decodeSetting(const Common::String &str) {
	return str;
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::toString(int val) {
	return Common::String::format("%d", val);
}


} // end of namespace Wintermute
