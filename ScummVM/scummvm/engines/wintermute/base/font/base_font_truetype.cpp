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

#include "engines/wintermute/base/font/base_font_truetype.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/wintermute.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"
#include <limits.h>

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseFontTT, false)

//////////////////////////////////////////////////////////////////////////
BaseFontTT::BaseFontTT(BaseGame *inGame) : BaseFont(inGame) {
	_fontHeight = 12;
	_isBold = _isItalic = _isUnderline = _isStriked = false;

	_fontFile = NULL;
	_font = NULL;
	_fallbackFont = NULL;
	_deletableFont = NULL;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		_cachedTexts[i] = NULL;
	}

	_lineHeight = 0;
	_maxCharWidth = _maxCharHeight = 0;
}

//////////////////////////////////////////////////////////////////////////
BaseFontTT::~BaseFontTT(void) {
	clearCache();

	for (uint32 i = 0; i < _layers.size(); i++) {
		delete _layers[i];
	}
	_layers.clear();

	delete[] _fontFile;
	_fontFile = NULL;

	delete _deletableFont;
	_font = NULL;
}


//////////////////////////////////////////////////////////////////////////
void BaseFontTT::clearCache() {
	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (_cachedTexts[i]) {
			delete _cachedTexts[i];
		}
		_cachedTexts[i] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void BaseFontTT::initLoop() {
	// we need more aggressive cache management on iOS not to waste too much memory on fonts
	if (_gameRef->_constrainedMemory) {
		// purge all cached images not used in the last frame
		for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
			if (_cachedTexts[i] == NULL) {
				continue;
			}

			if (!_cachedTexts[i]->_marked) {
				delete _cachedTexts[i];
				_cachedTexts[i] = NULL;
			} else {
				_cachedTexts[i]->_marked = false;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getTextWidth(byte *text, int maxLength) {
	WideString textStr;

	if (_gameRef->_textEncoding == TEXT_UTF8) {
		textStr = StringUtil::utf8ToWide((char *)text);
	} else {
		textStr = StringUtil::ansiToWide((char *)text);
	}

	if (maxLength >= 0 && textStr.size() > (uint32)maxLength) {
		textStr = Common::String(textStr.c_str(), (uint32)maxLength);
	}
	//text = text.substr(0, MaxLength); // TODO: Remove

	int textWidth, textHeight;
	measureText(textStr, -1, -1, textWidth, textHeight);

	return textWidth;
}

//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getTextHeight(byte *text, int width) {
	WideString textStr;

	if (_gameRef->_textEncoding == TEXT_UTF8) {
		textStr = StringUtil::utf8ToWide((char *)text);
	} else {
		textStr = StringUtil::ansiToWide((char *)text);
	}


	int textWidth, textHeight;
	measureText(textStr, width, -1, textWidth, textHeight);

	return textHeight;
}


//////////////////////////////////////////////////////////////////////////
void BaseFontTT::drawText(const byte *text, int x, int y, int width, TTextAlign align, int maxHeight, int maxLength) {
	if (text == NULL || strcmp((const char *)text, "") == 0) {
		return;
	}

	WideString textStr = (const char *)text;

	// TODO: Why do we still insist on Widestrings everywhere?
	/*  if (_gameRef->_textEncoding == TEXT_UTF8) text = StringUtil::Utf8ToWide((char *)Text);
	        else text = StringUtil::AnsiToWide((char *)Text);*/
	// HACK: J.U.L.I.A. uses CP1252, we need to fix that,
	// And we still don't have any UTF8-support.
	if (_gameRef->_textEncoding != TEXT_UTF8) {
		textStr = StringUtil::ansiToWide((char *)text);
	}

	if (maxLength >= 0 && textStr.size() > (uint32)maxLength) {
		textStr = Common::String(textStr.c_str(), (uint32)maxLength);
	}
	//text = text.substr(0, MaxLength); // TODO: Remove

	BaseRenderer *renderer = _gameRef->_renderer;

	// find cached surface, if exists
	int minPriority = INT_MAX;
	int minIndex = -1;
	BaseSurface *surface = NULL;
	int textOffset = 0;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (_cachedTexts[i] == NULL) {
			minPriority = 0;
			minIndex = i;
		} else {
			if (_cachedTexts[i]->_text == textStr && _cachedTexts[i]->_align == align && _cachedTexts[i]->_width == width && _cachedTexts[i]->_maxHeight == maxHeight && _cachedTexts[i]->_maxLength == maxLength) {
				surface = _cachedTexts[i]->_surface;
				textOffset = _cachedTexts[i]->_textOffset;
				_cachedTexts[i]->_priority++;
				_cachedTexts[i]->_marked = true;
				break;
			} else {
				if (_cachedTexts[i]->_priority < minPriority) {
					minPriority = _cachedTexts[i]->_priority;
					minIndex = i;
				}
			}
		}
	}

	// not found, create one
	if (!surface) {
		debugC(kWintermuteDebugFont, "Draw text: %s", text);
		surface = renderTextToTexture(textStr, width, align, maxHeight, textOffset);
		if (surface) {
			// write surface to cache
			if (_cachedTexts[minIndex] != NULL) {
				delete _cachedTexts[minIndex];
			}
			_cachedTexts[minIndex] = new BaseCachedTTFontText;

			_cachedTexts[minIndex]->_surface = surface;
			_cachedTexts[minIndex]->_align = align;
			_cachedTexts[minIndex]->_width = width;
			_cachedTexts[minIndex]->_maxHeight = maxHeight;
			_cachedTexts[minIndex]->_maxLength = maxLength;
			_cachedTexts[minIndex]->_priority = 1;
			_cachedTexts[minIndex]->_text = textStr;
			_cachedTexts[minIndex]->_textOffset = textOffset;
			_cachedTexts[minIndex]->_marked = true;
		}
	}


	// and paint it
	if (surface) {
		Rect32 rc;
		BasePlatform::setRect(&rc, 0, 0, surface->getWidth(), surface->getHeight());
		for (uint32 i = 0; i < _layers.size(); i++) {
			uint32 color = _layers[i]->_color;
			uint32 origForceAlpha = renderer->_forceAlphaColor;
			if (renderer->_forceAlphaColor != 0) {
				color = BYTETORGBA(RGBCOLGetR(color), RGBCOLGetG(color), RGBCOLGetB(color), RGBCOLGetA(renderer->_forceAlphaColor));
				renderer->_forceAlphaColor = 0;
			}
			surface->displayTransOffset(x, y - textOffset, rc, color, BLEND_NORMAL, false, false, _layers[i]->_offsetX, _layers[i]->_offsetY);

			renderer->_forceAlphaColor = origForceAlpha;
		}
	}


}

//////////////////////////////////////////////////////////////////////////
BaseSurface *BaseFontTT::renderTextToTexture(const WideString &text, int width, TTextAlign align, int maxHeight, int &textOffset) {
	//TextLineList lines;
	// TODO: Use WideString-conversion here.
	//WrapText(text, width, maxHeight, lines);
	Common::Array<Common::String> lines;
	_font->wordWrapText(text, width, lines);

	while (maxHeight > 0 && lines.size() * _lineHeight > maxHeight) {
		lines.pop_back();
	}
	if (lines.size() == 0) {
		return NULL;
	}

	Graphics::TextAlign alignment = Graphics::kTextAlignInvalid;
	if (align == TAL_LEFT) {
		alignment = Graphics::kTextAlignLeft;
	} else if (align == TAL_CENTER) {
		alignment = Graphics::kTextAlignCenter;
	} else if (align == TAL_RIGHT) {
		alignment = Graphics::kTextAlignRight;
	}

	debugC(kWintermuteDebugFont, "%s %d %d %d %d", text.c_str(), RGBCOLGetR(_layers[0]->_color), RGBCOLGetG(_layers[0]->_color), RGBCOLGetB(_layers[0]->_color), RGBCOLGetA(_layers[0]->_color));
//	void drawString(Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true) const;
	Graphics::Surface *surface = new Graphics::Surface();
	if (_deletableFont) { // We actually have a TTF
		surface->create((uint16)width, (uint16)(_lineHeight * lines.size()), Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
	} else { // We are using a fallback, they can't do 32bpp
		surface->create((uint16)width, (uint16)(_lineHeight * lines.size()), Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	}
	uint32 useColor = 0xffffffff;
	Common::Array<Common::String>::iterator it;
	int heightOffset = 0;
	for (it = lines.begin(); it != lines.end(); ++it) {
		_font->drawString(surface, *it, 0, heightOffset, width, useColor, alignment);
		heightOffset += (int)_lineHeight;
	}

	BaseSurface *retSurface = _gameRef->_renderer->createSurface();
	Graphics::Surface *convertedSurface = surface->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
	retSurface->putSurface(*convertedSurface, true);
	convertedSurface->free();
	surface->free();
	delete surface;
	delete convertedSurface;
	return retSurface;
	// TODO: _isUnderline, _isBold, _isItalic, _isStriked
}


//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getLetterHeight() {
	return (int)getLineHeight();
}


//////////////////////////////////////////////////////////////////////
bool BaseFontTT::loadFile(const Common::String &filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "BaseFontTT::LoadFile failed for file '%s'", filename.c_str());
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename.c_str());

	if (DID_FAIL(ret = loadBuffer(buffer))) {
		_gameRef->LOG(0, "Error parsing TTFONT file '%s'", filename.c_str());
	}

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(TTFONT)
TOKEN_DEF(SIZE)
TOKEN_DEF(FACE)
TOKEN_DEF(FILENAME)
TOKEN_DEF(BOLD)
TOKEN_DEF(ITALIC)
TOKEN_DEF(UNDERLINE)
TOKEN_DEF(STRIKE)
TOKEN_DEF(CHARSET)
TOKEN_DEF(COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(LAYER)
TOKEN_DEF(OFFSET_X)
TOKEN_DEF(OFFSET_Y)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
bool BaseFontTT::loadBuffer(byte *buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(TTFONT)
	TOKEN_TABLE(SIZE)
	TOKEN_TABLE(FACE)
	TOKEN_TABLE(FILENAME)
	TOKEN_TABLE(BOLD)
	TOKEN_TABLE(ITALIC)
	TOKEN_TABLE(UNDERLINE)
	TOKEN_TABLE(STRIKE)
	TOKEN_TABLE(CHARSET)
	TOKEN_TABLE(COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(LAYER)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;

	if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_TTFONT) {
		_gameRef->LOG(0, "'TTFONT' keyword expected.");
		return STATUS_FAILED;
	}
	buffer = (byte *)params;

	uint32 baseColor = 0x00000000;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_SIZE:
			parser.scanStr(params, "%d", &_fontHeight);
			break;

		case TOKEN_FACE:
			// we don't need this anymore
			break;

		case TOKEN_FILENAME:
			BaseUtils::setString(&_fontFile, params);
			break;

		case TOKEN_BOLD:
			parser.scanStr(params, "%b", &_isBold);
			break;

		case TOKEN_ITALIC:
			parser.scanStr(params, "%b", &_isItalic);
			break;

		case TOKEN_UNDERLINE:
			parser.scanStr(params, "%b", &_isUnderline);
			break;

		case TOKEN_STRIKE:
			parser.scanStr(params, "%b", &_isStriked);
			break;

		case TOKEN_CHARSET:
			// we don't need this anymore
			break;

		case TOKEN_COLOR: {
			int r, g, b;
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			baseColor = BYTETORGBA(r, g, b, RGBCOLGetA(baseColor));
		}
		break;

		case TOKEN_ALPHA: {
			int a;
			parser.scanStr(params, "%d", &a);
			baseColor = BYTETORGBA(RGBCOLGetR(baseColor), RGBCOLGetG(baseColor), RGBCOLGetB(baseColor), a);
		}
		break;

		case TOKEN_LAYER: {
			BaseTTFontLayer *layer = new BaseTTFontLayer;
			if (layer && DID_SUCCEED(parseLayer(layer, (byte *)params))) {
				_layers.add(layer);
			} else {
				delete layer;
				layer = NULL;
				cmd = PARSERR_TOKENNOTFOUND;
			}
		}
		break;

		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in TTFONT definition");
		return STATUS_FAILED;
	}

	// create at least one layer
	if (_layers.size() == 0) {
		BaseTTFontLayer *layer = new BaseTTFontLayer;
		layer->_color = baseColor;
		_layers.add(layer);
	}

	if (!_fontFile) {
		BaseUtils::setString(&_fontFile, "arial.ttf");
	}

	return initFont();
}


//////////////////////////////////////////////////////////////////////////
bool BaseFontTT::parseLayer(BaseTTFontLayer *layer, byte *buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(OFFSET_X)
	TOKEN_TABLE(OFFSET_Y)
	TOKEN_TABLE(COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_OFFSET_X:
			parser.scanStr(params, "%d", &layer->_offsetX);
			break;

		case TOKEN_OFFSET_Y:
			parser.scanStr(params, "%d", &layer->_offsetY);
			break;

		case TOKEN_COLOR: {
			int r, g, b;
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			layer->_color = BYTETORGBA(r, g, b, RGBCOLGetA(layer->_color));
		}
		break;

		case TOKEN_ALPHA: {
			int a;
			parser.scanStr(params, "%d", &a);
			layer->_color = BYTETORGBA(RGBCOLGetR(layer->_color), RGBCOLGetG(layer->_color), RGBCOLGetB(layer->_color), a);
		}
		break;
		}
	}
	if (cmd != PARSERR_EOF) {
		return STATUS_FAILED;
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseFontTT::persist(BasePersistenceManager *persistMgr) {
	BaseFont::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_isBold));
	persistMgr->transfer(TMEMBER(_isItalic));
	persistMgr->transfer(TMEMBER(_isUnderline));
	persistMgr->transfer(TMEMBER(_isStriked));
	persistMgr->transfer(TMEMBER(_fontHeight));
	persistMgr->transfer(TMEMBER(_fontFile));


	// persist layers
	int numLayers;
	if (persistMgr->getIsSaving()) {
		numLayers = _layers.size();
		persistMgr->transfer(TMEMBER(numLayers));
		for (int i = 0; i < numLayers; i++) {
			_layers[i]->persist(persistMgr);
		}
	} else {
		numLayers = _layers.size();
		persistMgr->transfer(TMEMBER(numLayers));
		for (int i = 0; i < numLayers; i++) {
			BaseTTFontLayer *layer = new BaseTTFontLayer;
			layer->persist(persistMgr);
			_layers.add(layer);
		}
	}

	if (!persistMgr->getIsSaving()) {
		for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
			_cachedTexts[i] = NULL;
		}
		_fallbackFont = _font = _deletableFont = NULL;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void BaseFontTT::afterLoad() {
	initFont();
}

//////////////////////////////////////////////////////////////////////////
bool BaseFontTT::initFont() {
	if (!_fontFile) {
		return STATUS_FAILED;
	}

	Common::SeekableReadStream *file = BaseFileManager::getEngineInstance()->openFile(_fontFile);
	if (!file) {
		//TODO: Try to fallback from Arial to FreeSans
		/*
		// the requested font file is not in wme file space; try loading a system font
		AnsiString fontFileName = PathUtil::combine(BasePlatform::getSystemFontPath(), PathUtil::getFileName(_fontFile));
		file = BaseFileManager::getEngineInstance()->openFile(fontFileName.c_str(), false);
		if (!file) {
		    _gameRef->LOG(0, "Error loading TrueType font '%s'", _fontFile);
		    //return STATUS_FAILED;
		}*/
	}

	if (file) {
#ifdef USE_FREETYPE2
		_deletableFont = Graphics::loadTTFFont(*file, 96, _fontHeight); // Use the same dpi as WME (96 vs 72).
		_font = _deletableFont;
#endif
	}
	if (!_font) {
		_font = _fallbackFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		warning("BaseFontTT::InitFont - Couldn't load font: %s", _fontFile);
	}
	_lineHeight = _font->getFontHeight();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseFontTT::measureText(const WideString &text, int maxWidth, int maxHeight, int &textWidth, int &textHeight) {
	//TextLineList lines;

	if (maxWidth >= 0) {
		Common::Array<Common::String> lines;
		_font->wordWrapText(text, maxWidth, lines);
		Common::Array<Common::String>::iterator it;
		textWidth = 0;
		for (it = lines.begin(); it != lines.end(); ++it) {
			textWidth = MAX(textWidth, _font->getStringWidth(*it));
		}

		//WrapText(text, maxWidth, maxHeight, lines);

		textHeight = (int)(lines.size() * getLineHeight());
	} else {
		textWidth = _font->getStringWidth(text);
		textHeight = _fontHeight;
	}
	/*
	    TextLineList::iterator it;
	    for (it = lines.begin(); it != lines.end(); ++it) {
	        TextLine *line = (*it);
	        textWidth = MAX(textWidth, line->GetWidth());
	        delete line;
	        line = NULL;
	    }*/
}

} // end of namespace Wintermute
