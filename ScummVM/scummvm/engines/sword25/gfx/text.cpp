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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// TODO:
// Entweder Fontfile absolut abspeichern, oder Verzeichniswechseln verbieten
// Eine relative Fontfile-Angabe könnte verwandt werden nachdem das Verzeichnis bereits gewechselt wurde und die Datei würde nicht mehr gefunden

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/resmanager.h"	// for PRECACHE_RESOURCES
#include "sword25/gfx/fontresource.h"
#include "sword25/gfx/bitmapresource.h"

#include "sword25/gfx/text.h"

namespace Sword25 {

namespace {
const uint AUTO_WRAP_THRESHOLD_DEFAULT = 300;
}

Text::Text(RenderObjectPtr<RenderObject> parentPtr) :
	RenderObject(parentPtr, RenderObject::TYPE_TEXT),
	_modulationColor(0xffffffff),
	_autoWrap(false),
	_autoWrapThreshold(AUTO_WRAP_THRESHOLD_DEFAULT) {

}

Text::Text(InputPersistenceBlock &reader, RenderObjectPtr<RenderObject> parentPtr, uint handle) :
		RenderObject(parentPtr, TYPE_TEXT, handle),
		// Temporarily set fields prior to unpersisting actual values
		_modulationColor(0xffffffff),
		_autoWrap(false),
		_autoWrapThreshold(AUTO_WRAP_THRESHOLD_DEFAULT) {

	// Unpersist the fields
	_initSuccess = unpersist(reader);
}

bool Text::setFont(const Common::String &font) {
	// Load font

#ifdef PRECACHE_RESOURCES
	if (getResourceManager()->precacheResource(font)) {
		_font = font;
		updateFormat();
		forceRefresh();
		return true;
	} else {
		error("Could not precache font \"%s\". Font probably does not exist.", font.c_str());
		return false;
	}
#else
	getResourceManager()->requestResource(font);
	_font = font;
	updateFormat();
	forceRefresh();
	return true;
#endif

}

void Text::setText(const Common::String &text) {
	_text = text;
	updateFormat();
	forceRefresh();
}

void Text::setColor(uint modulationColor) {
	uint newModulationColor = (modulationColor & 0x00ffffff) | (_modulationColor & 0xff000000);
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

void Text::setAlpha(int alpha) {
	assert(alpha >= 0 && alpha < 256);
	uint newModulationColor = (_modulationColor & 0x00ffffff) | alpha << 24;
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

void Text::setAutoWrap(bool autoWrap) {
	if (autoWrap != _autoWrap) {
		_autoWrap = autoWrap;
		updateFormat();
		forceRefresh();
	}
}

void Text::setAutoWrapThreshold(uint autoWrapThreshold) {
	if (autoWrapThreshold != _autoWrapThreshold) {
		_autoWrapThreshold = autoWrapThreshold;
		updateFormat();
		forceRefresh();
	}
}

bool Text::doRender() {
	// Font-Resource locken.
	FontResource *fontPtr = lockFontResource();
	if (!fontPtr)
		return false;

	// Charactermap-Resource locken.
	ResourceManager *rmPtr = getResourceManager();
	BitmapResource *charMapPtr;
	{
		Resource *pResource = rmPtr->requestResource(fontPtr->getCharactermapFileName());
		if (!pResource) {
			warning("Could not request resource \"%s\".", fontPtr->getCharactermapFileName().c_str());
			return false;
		}
		if (pResource->getType() != Resource::TYPE_BITMAP) {
			error("Requested resource \"%s\" is not a bitmap.", fontPtr->getCharactermapFileName().c_str());
			return false;
		}

		charMapPtr = static_cast<BitmapResource *>(pResource);
	}

	// Framebufferobjekt holen.
	GraphicEngine *gfxPtr = Kernel::getInstance()->getGfx();
	assert(gfxPtr);

	bool result = true;
	Common::Array<Line>::iterator iter = _lines.begin();
	for (; iter != _lines.end(); ++iter) {
		// Feststellen, ob überhaupt Buchstaben der aktuellen Zeile vom Update betroffen sind.
		Common::Rect checkRect = (*iter).bbox;
		checkRect.translate(_absoluteX, _absoluteY);

		// Jeden Buchstaben einzeln Rendern.
		int curX = _absoluteX + (*iter).bbox.left;
		int curY = _absoluteY + (*iter).bbox.top;
		for (uint i = 0; i < (*iter).text.size(); ++i) {
			Common::Rect curRect = fontPtr->getCharacterRect((byte)(*iter).text[i]);

			Common::Rect renderRect(curX, curY, curX + curRect.width(), curY + curRect.height());
			renderRect.translate(curRect.left - curX, curRect.top - curY);
			result = charMapPtr->blit(curX, curY, Image::FLIP_NONE, &renderRect, _modulationColor);
			if (!result)
				break;

			curX += curRect.width() + fontPtr->getGapWidth();
		}
	}

	// Charactermap-Resource freigeben.
	charMapPtr->release();

	// Font-Resource freigeben.
	fontPtr->release();

	return result;
}

ResourceManager *Text::getResourceManager() {
	// Pointer auf den Resource-Manager holen.
	return Kernel::getInstance()->getResourceManager();
}

FontResource *Text::lockFontResource() {
	ResourceManager *rmPtr = getResourceManager();

	// Font-Resource locken.
	FontResource *fontPtr;
	{
		Resource *resourcePtr = rmPtr->requestResource(_font);
		if (!resourcePtr) {
			warning("Could not request resource \"%s\".", _font.c_str());
			return NULL;
		}
		if (resourcePtr->getType() != Resource::TYPE_FONT) {
			error("Requested resource \"%s\" is not a font.", _font.c_str());
			return NULL;
		}

		fontPtr = static_cast<FontResource *>(resourcePtr);
	}

	return fontPtr;
}

void Text::updateFormat() {
	FontResource *fontPtr = lockFontResource();
	assert(fontPtr);

	updateMetrics(*fontPtr);

	_lines.resize(1);
	if (_autoWrap && (uint) _width >= _autoWrapThreshold && _text.size() >= 2) {
		_width = 0;
		uint curLineWidth = 0;
		uint curLineHeight = 0;
		uint curLine = 0;
		uint tempLineWidth = 0;
		uint lastSpace = 0; // we need at least 1 space character to start a new line...
		_lines[0].text = "";
		for (uint i = 0; i < _text.size(); ++i) {
			uint j;
			tempLineWidth = 0;
			lastSpace = 0;
			for (j = i; j < _text.size(); ++j) {
				if ((byte)_text[j] == ' ')
					lastSpace = j;

				const Common::Rect &curCharRect = fontPtr->getCharacterRect((byte)_text[j]);
				tempLineWidth += curCharRect.width();
				tempLineWidth += fontPtr->getGapWidth();

				if ((tempLineWidth >= _autoWrapThreshold) && (lastSpace > 0))
					break;
			}

			if (j == _text.size()) // everything in 1 line.
				lastSpace = _text.size();

			curLineWidth = 0;
			curLineHeight = 0;
			for (j = i; j < lastSpace; ++j) {
				_lines[curLine].text += _text[j];

				const Common::Rect &curCharRect = fontPtr->getCharacterRect((byte)_text[j]);
				curLineWidth += curCharRect.width();
				curLineWidth += fontPtr->getGapWidth();
				if ((uint)curCharRect.height() > curLineHeight)
					curLineHeight = curCharRect.height();
			}

			_lines[curLine].bbox.right = curLineWidth;
			_lines[curLine].bbox.bottom = curLineHeight;
			if ((uint)_width < curLineWidth)
				_width = curLineWidth;

			if (lastSpace < _text.size()) {
				++curLine;
				assert(curLine == _lines.size());
				_lines.resize(curLine + 1);
				_lines[curLine].text = "";
			}

			i = lastSpace;
		}

		// Bounding-Box der einzelnen Zeilen relativ zur ersten festlegen (vor allem zentrieren).
		_height = 0;
		Common::Array<Line>::iterator iter = _lines.begin();
		for (; iter != _lines.end(); ++iter) {
			Common::Rect &bbox = (*iter).bbox;
			bbox.left = (_width - bbox.right) / 2;
			bbox.right = bbox.left + bbox.right;
			bbox.top = (iter - _lines.begin()) * fontPtr->getLineHeight();
			bbox.bottom = bbox.top + bbox.bottom;
			_height += bbox.height();
		}
	} else {
		// Keine automatische Formatierung, also wird der gesamte Text in nur eine Zeile kopiert.
		_lines[0].text = _text;
		_lines[0].bbox = Common::Rect(0, 0, _width, _height);
	}

	fontPtr->release();
}

void Text::updateMetrics(FontResource &fontResource) {
	_width = 0;
	_height = 0;

	for (uint i = 0; i < _text.size(); ++i) {
		const Common::Rect &curRect = fontResource.getCharacterRect((byte)_text[i]);
		_width += curRect.width();
		if (i != _text.size() - 1)
			_width += fontResource.getGapWidth();
		if (_height < curRect.height())
			_height = curRect.height();
	}
}

bool Text::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	result &= RenderObject::persist(writer);

	writer.write(_modulationColor);
	writer.writeString(_font);
	writer.writeString(_text);
	writer.write(_autoWrap);
	writer.write(_autoWrapThreshold);

	result &= RenderObject::persistChildren(writer);

	return result;
}

bool Text::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	result &= RenderObject::unpersist(reader);

	// Farbe und Alpha einlesen.
	reader.read(_modulationColor);

	// Beim Laden der anderen Member werden die Set-Methoden benutzt statt der tatsächlichen Member.
	// So wird das Layout automatisch aktualisiert und auch alle anderen notwendigen Methoden ausgeführt.

	Common::String font;
	reader.readString(font);
	setFont(font);

	Common::String text;
	reader.readString(text);
	setText(text);

	bool autoWrap;
	reader.read(autoWrap);
	setAutoWrap(autoWrap);

	uint autoWrapThreshold;
	reader.read(autoWrapThreshold);
	setAutoWrapThreshold(autoWrapThreshold);

	result &= RenderObject::unpersistChildren(reader);

	return reader.isGood() && result;
}

} // End of namespace Sword25
