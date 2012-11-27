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

#ifndef SWORD25_BITMAP_H
#define SWORD25_BITMAP_H

#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobject.h"

namespace Sword25 {

class Bitmap : public RenderObject {
protected:
	Bitmap(RenderObjectPtr<RenderObject> parentPtr, TYPES type, uint handle = 0);

public:

	virtual ~Bitmap();

	/**
	    @brief Setzt den Alphawert des Bitmaps.
	    @param Alpha der neue Alphawert der Bitmaps (0 = keine Deckung, 255 = volle Deckung).
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsAlphaAllowed() true zurückgibt.
	*/
	void setAlpha(int alpha);

	/**
	    @brief Setzt die Modulationfarbe der Bitmaps.
	    @param Color eine 24-Bit Farbe, die die Modulationsfarbe des Bitmaps festlegt.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsColorModulationAllowed() true zurückgibt.
	*/
	void setModulationColor(uint modulationColor);

	/**
	    @brief Setzt den Skalierungsfaktor des Bitmaps.
	    @param ScaleFactor der Faktor um den das Bitmap in beide Richtungen gestreckt werden soll.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void setScaleFactor(float scaleFactor);

	/**
	    @brief Setzt den Skalierungsfaktor der Bitmap auf der X-Achse.
	    @param ScaleFactor der Faktor um den die Bitmap in Richtungen der X-Achse gestreckt werden soll. Dieser Wert muss positiv sein.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void setScaleFactorX(float scaleFactorX);

	/**
	    @brief Setzt den Skalierungsfaktor der Bitmap auf der Y-Achse.
	    @param ScaleFactor der Faktor um den die Bitmap in Richtungen der Y-Achse gestreckt werden soll. Dieser Wert muss positiv sein.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void setScaleFactorY(float scaleFactorY);

	/**
	    @brief Legt fest, ob das Bild an der X-Achse gespiegelt werden soll.
	*/
	void setFlipH(bool flipH);

	/**
	    @brief Legt fest, ob das Bild an der Y-Achse gespiegelt werden soll.
	*/
	void setFlipV(bool flipV);

	/**
	    @brief Gibt den aktuellen Alphawert des Bildes zurück.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsAlphaAllowed() true zurückgibt.
	*/
	int getAlpha() {
		return _modulationColor >> 24;
	}

	/**
	    @brief Gibt die aktuelle 24bit RGB Modulationsfarde des Bildes zurück.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsColorModulationAllowed() true zurückgibt.
	*/
	int getModulationColor() {
		return _modulationColor & 0x00ffffff;
	}

	/**
	    @brief Gibt den Skalierungsfakter des Bitmaps auf der X-Achse zurück.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	float getScaleFactorX() const {
		return _scaleFactorX;
	}

	/**
	    @brief Gibt den Skalierungsfakter des Bitmaps auf der Y-Achse zurück.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	float getScaleFactorY() const {
		return _scaleFactorY;
	}

	/**
	    @brief Gibt zurück, ob das Bild an der X-Achse gespiegelt angezeigt wird.
	*/
	bool isFlipH() {
		return _flipH;
	}

	/**
	    @brief Gibt zurück, ob das Bild an der Y-Achse gespiegelt angezeigt wird.
	*/
	bool isFlipV() {
		return _flipV;
	}

	// -----------------------------------------------------------------------------
	// Die folgenden Methoden müssen alle BS_Bitmap-Klassen implementieren
	// -----------------------------------------------------------------------------

	/**
	    @brief Liest einen Pixel des Bildes.
	    @param X die X-Koordinate des Pixels.
	    @param Y die Y-Koordinate des Pixels
	    @return Gibt den 32-Bit Farbwert des Pixels an der übergebenen Koordinate zurück.
	    @remark Diese Methode sollte auf keine Fall benutzt werden um größere Teile des Bildes zu lesen, da sie sehr langsam ist. Sie ist
	            eher dafür gedacht einzelne Pixel des Bildes auszulesen.
	*/
	virtual uint getPixel(int x, int y) const = 0;

	/**
	    @brief Füllt den Inhalt des Bildes mit Pixeldaten.
	    @param Pixeldata ein Vector der die Pixeldaten enthält. Sie müssen in dem Farbformat des Bildes vorliegen und es müssen genügend Daten
	           vorhanden sein, um das ganze Bild zu füllen.
	    @param Offset der Offset in Byte im Pixeldata-Vector an dem sich der erste zu schreibende Pixel befindet.<br>
	                  Der Standardwert ist 0.
	    @param Stride der Abstand in Byte zwischen dem Zeilenende und dem Beginn einer neuen Zeile im Pixeldata-Vector.<br>
	                  Der Standardwert ist 0.
	    @return Gibt false zurück, falls der Aufruf fehlgeschlagen ist.
	    @remark Ein Aufruf dieser Methode ist nur erlaubt, wenn IsSetContentAllowed() true zurückgibt.
	*/
	virtual bool    setContent(const byte *pixeldata, uint size, uint offset = 0, uint stride = 0) = 0;

	virtual bool    isScalingAllowed() const = 0;
	virtual bool    isAlphaAllowed() const = 0;
	virtual bool    isColorModulationAllowed() const = 0;
	virtual bool    isSetContentAllowed() const = 0;

	virtual bool    persist(OutputPersistenceBlock &writer);
	virtual bool    unpersist(InputPersistenceBlock &reader);

protected:
	bool  _flipH;
	bool  _flipV;
	float _scaleFactorX;
	float _scaleFactorY;
	uint  _modulationColor;
	int   _originalWidth;
	int   _originalHeight;
};

} // End of namespace Sword25

#endif
