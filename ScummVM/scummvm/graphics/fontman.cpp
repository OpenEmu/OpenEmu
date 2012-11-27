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
 */

#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/bdf.h"

#include "common/translation.h"

namespace Common {
DECLARE_SINGLETON(Graphics::FontManager);
}

namespace Graphics {

FORWARD_DECLARE_FONT(g_sysfont);
FORWARD_DECLARE_FONT(g_sysfont_big);
FORWARD_DECLARE_FONT(g_consolefont);

FontManager::FontManager() {
	// This assert should *never* trigger, because
	// FontManager is a singleton, thus there is only
	// one instance of it per time. (g_sysfont gets
	// reset to 0 in the desctructor of this class).
	assert(g_sysfont == 0);
	INIT_FONT(g_sysfont);
	INIT_FONT(g_sysfont_big);
	INIT_FONT(g_consolefont);
}

FontManager::~FontManager() {
	for (uint i = 0; i < _ownedFonts.size(); ++i) {
		const Font *font = _ownedFonts[i];
		if (font == g_sysfont || font == g_sysfont_big || font == g_consolefont)
			continue;
		delete font;
	}

	delete g_sysfont;
	g_sysfont = 0;
	delete g_sysfont_big;
	g_sysfont_big = 0;
	delete g_consolefont;
	g_consolefont = 0;
}

const struct {
	const char *name;
	FontManager::FontUsage id;
} builtinFontNames[] = {
	{ "builtinConsole", FontManager::kConsoleFont },
	{ "fixed5x8.bdf", FontManager::kConsoleFont },
	{ "fixed5x8-iso-8859-1.bdf", FontManager::kConsoleFont },
	{ "fixed5x8-ascii.bdf", FontManager::kConsoleFont },
	{ "clR6x12.bdf", FontManager::kGUIFont },
	{ "clR6x12-iso-8859-1.bdf", FontManager::kGUIFont },
	{ "clR6x12-ascii.bdf", FontManager::kGUIFont },
	{ "helvB12.bdf", FontManager::kBigGUIFont },
	{ "helvB12-iso-8859-1.bdf", FontManager::kBigGUIFont },
	{ "helvB12-ascii.bdf", FontManager::kBigGUIFont },
	{ 0, FontManager::kConsoleFont }
};

bool FontManager::setLocalizedFont(const Common::String &name) {
	Common::String lowercaseName = name;
	lowercaseName.toLowercase();

	// We only update the localized font in case the name is properly assigned
	// to a font.
	if (_fontMap.contains(lowercaseName) && _fontMap.getVal(lowercaseName) != 0) {
		_localizedFontName = lowercaseName;
		return true;
	} else {
		return false;
	}
}

bool FontManager::assignFontToName(const Common::String &name, const Font *font) {
	Common::String lowercaseName = name;
	lowercaseName.toLowercase();
	_fontMap[lowercaseName] = font;
	if (Common::find(_ownedFonts.begin(), _ownedFonts.end(), font) == _ownedFonts.end())
		_ownedFonts.push_back(font);
	return true;
}

bool FontManager::setFont(FontUsage usage, const BdfFont *font) {
	switch (usage) {
	case kConsoleFont:
		delete g_consolefont;
		g_consolefont = font;
		break;
	case kGUIFont:
		delete g_sysfont;
		g_sysfont = font;
		break;
	case kBigGUIFont:
		delete g_sysfont_big;
		g_sysfont_big = font;
		break;
	default:
		return false;
	}
	return true;
}

void FontManager::removeFontName(const Common::String &name) {
	Common::String lowercaseName = name;
	lowercaseName.toLowercase();
	if (!_fontMap.contains(lowercaseName))
		return;

	const Font *font = _fontMap[lowercaseName];
	_fontMap.erase(lowercaseName);

	// Check if we still have a copy of this font in the map.
	bool stillHasFont = false;
	for (Common::HashMap<Common::String, const Font *>::iterator i = _fontMap.begin(); i != _fontMap.end(); ++i) {
		if (i->_value != font)
			continue;
		stillHasFont = true;
		break;
	}

	if (!stillHasFont) {
		// We don't have a copy of the font, so remove it from our list and delete it.
		stillHasFont = true;
		for (uint i = 0; i < _ownedFonts.size(); ++i) {
			if (_ownedFonts[i] != font)
				continue;
			stillHasFont = false;
			_ownedFonts.remove_at(i);
			break;
		}
		assert(!stillHasFont);
		delete font;
	}

	// In case the current localized font is removed, we fall back to the
	// default font again.
	if (_localizedFontName == lowercaseName)
		_localizedFontName.clear();
}

const Font *FontManager::getFontByName(const Common::String &name) const {
	for (int i = 0; builtinFontNames[i].name; i++)
		if (!scumm_stricmp(name.c_str(), builtinFontNames[i].name))
			return getFontByUsage(builtinFontNames[i].id);

	Common::String lowercaseName = name;
	lowercaseName.toLowercase();
	if (!_fontMap.contains(lowercaseName))
		return 0;
	return _fontMap[lowercaseName];
}

const Font *FontManager::getFontByUsage(FontUsage usage) const {
	switch (usage) {
	case kConsoleFont:
		return g_consolefont;
	case kGUIFont:
		return g_sysfont;
	case kBigGUIFont:
		return g_sysfont_big;
	case kLocalizedFont:
		// By default use the big font as localized font
		if (_localizedFontName.empty())
			return g_sysfont_big;
		else
			return _fontMap[_localizedFontName];
	}

	return 0;
}

Common::String FontManager::genLocalizedFontFilename(const Common::String &filename) const {
#ifndef USE_TRANSLATION
	return filename;
#else
	// We will transform the font filename in the following way:
	//   name.bdf
	//  will become:
	//   name-charset.bdf
	// Note that name should not contain any dot here!

	// In the first step we look for the dot. In case there is none we will
	// return the normal filename.
	Common::String::const_iterator dot = Common::find(filename.begin(), filename.end(), '.');
	if (dot == filename.end())
		return filename;

	// Put the translated font filename string back together.
	Common::String result(filename.begin(), dot);
	result += '-';
	result += TransMan.getCurrentCharset();
	result += dot;

	return result;
#endif
}

} // End of namespace Graphics
