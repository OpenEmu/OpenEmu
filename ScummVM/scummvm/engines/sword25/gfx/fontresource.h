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

#ifndef SWORD25_FONTRESOURCE_H
#define SWORD25_FONTRESOURCE_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/xmlparser.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/resource.h"

namespace Sword25 {

class Kernel;

class FontResource : public Resource, Common::XMLParser {
public:
	/**
	    @brief Erzeugt eine neues Exemplar von BS_FontResource
	    @param pKernel ein Pointer auf den Kernel
	    @param FileName der Dateiname der zu ladenen Resource
	    @remark Wenn der Konstruktor erfolgreich ausgeführt werden konnte gibt die Methode IsValid true zurück.
	*/
	FontResource(Kernel *pKernel, const Common::String &fileName);

	/**
	    @brief Gibt true zurück, wenn das Objekt korrekt initialisiert wurde.

	    Diese Methode kann dazu benutzt werden um festzustellen, ob der Konstruktor erfolgreich ausgeführt wurde.
	*/
	bool isValid() const {
		return _valid;
	}

	/**
	    @brief Gibt die Zeilenhöhe des Fonts in Pixeln zurück.

	    Die Zeilenhöhe ist der Wert, der zur Y-Koordinate addiert wird, wenn ein Zeilenumbruch auftritt.
	*/
	int getLineHeight() const {
		return _lineHeight;
	}

	/**
	    @brief Gibt den Buchstabenabstand der Fonts in Pixeln zurück.

	    Der Buchstabenabstand ist der Wert, der zwischen zwei Buchstaben freigelassen wird.
	*/
	int getGapWidth() const {
		return _gapWidth;
	}

	/**
	    @brief Gibt das Bounding-Rect eines Zeichens auf der Charactermap zurück.
	    @param Character der ASCII-Code des Zeichens
	    @return Das Bounding-Rect des übergebenen Zeichens auf der Charactermap.
	*/
	const Common::Rect &getCharacterRect(int character) const {
		assert(character >= 0 && character < 256);
		return _characterRects[character];
	}

	/**
	    @brief Gibt den Dateinamen der Charactermap zurück.
	*/
	const Common::String &getCharactermapFileName() const {
		return _bitmapFileName;
	}

private:
	Kernel *_pKernel;
	bool _valid;
	Common::String _bitmapFileName;
	int _lineHeight;
	int _gapWidth;
	Common::Rect _characterRects[256];

	// Parser
	CUSTOM_XML_PARSER(FontResource) {
		XML_KEY(font)
			XML_PROP(bitmap, true)
			XML_PROP(lineheight, false)
			XML_PROP(gap, false)

			XML_KEY(character)
				XML_PROP(code, true)
				XML_PROP(left, true)
				XML_PROP(top, true)
				XML_PROP(right, true)
				XML_PROP(bottom, true)
			KEY_END()
		KEY_END()
	} PARSER_END()

	// Parser callback methods
	bool parserCallback_font(ParserNode *node);
	bool parserCallback_character(ParserNode *node);
};

} // End of namespace Sword25

#endif
