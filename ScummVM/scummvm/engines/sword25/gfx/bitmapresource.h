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

#ifndef SWORD25_BITMAP_RESOURCE_H
#define SWORD25_BITMAP_RESOURCE_H

#include "sword25/kernel/common.h"
#include "sword25/kernel/resource.h"
#include "sword25/gfx/image/image.h"

namespace Sword25 {

class BitmapResource : public Resource {
public:
	/**
	    @brief Die möglichen Flippingparameter für die Blit-Methode.
	*/
	enum FLIP_FLAGS {
		/// Das Bild wird nicht gespiegelt.
		FLIP_NONE = 0,
		/// Das Bild wird an der horizontalen Achse gespiegelt.
		FLIP_H = 1,
		/// Das Bild wird an der vertikalen Achse gespiegelt.
		FLIP_V = 2,
		/// Das Bild wird an der horizontalen und vertikalen Achse gespiegelt.
		FLIP_HV = FLIP_H | FLIP_V,
		/// Das Bild wird an der horizontalen und vertikalen Achse gespiegelt.
		FLIP_VH = FLIP_H | FLIP_V
	};

	BitmapResource(const Common::String &filename, Image *pImage) :
					_pImage(pImage), Resource(filename, Resource::TYPE_BITMAP) {}
	virtual ~BitmapResource() { delete _pImage; }

	/**
	    @brief Gibt zurück, ob das Objekt einen gültigen Zustand hat.
	*/
	bool isValid() const {
		return (_pImage != 0);
	}

	/**
	    @brief Gibt die Breite des Bitmaps zurück.
	*/
	int getWidth() const {
		assert(_pImage);
		return _pImage->getWidth();
	}

	/**
	    @brief Gibt die Höhe des Bitmaps zurück.
	*/
	int getHeight() const {
		assert(_pImage);
		return _pImage->getHeight();
	}

	/**
	    @brief Rendert das Bild in den Framebuffer.
	    @param PosX die Position auf der X-Achse im Zielbild in Pixeln, an der das Bild gerendert werden soll.<br>
	                Der Standardwert ist 0.
	    @param PosY die Position auf der Y-Achse im Zielbild in Pixeln, an der das Bild gerendert werden soll.<br>
	                Der Standardwert ist 0.
	    @param Flipping gibt an, wie das Bild gespiegelt werden soll.<br>
	                    Der Standardwert ist BS_Image::FLIP_NONE (keine Spiegelung)
	    @param pSrcPartRect Pointer auf ein Common::Rect, welches den Ausschnitt des Quellbildes spezifiziert, der gerendert
	                        werden soll oder NULL, falls das gesamte Bild gerendert werden soll.<br>
	                        Dieser Ausschnitt bezieht sich auf das ungespiegelte und unskalierte Bild.<br>
	                        Der Standardwert ist NULL.
	    @param Color ein ARGB Farbwert, der die Parameter für die Farbmodulation und fürs Alphablending festlegt.<br>
	                 Die Alpha-Komponente der Farbe bestimmt den Alphablending Parameter (0 = keine Deckung, 255 = volle Deckung).<br>
	                 Die Farbkomponenten geben die Farbe für die Farbmodulation an.<br>
	                 Der Standardwert is BS_ARGB(255, 255, 255, 255) (volle Deckung, keine Farbmodulation).
	                 Zum Erzeugen des Farbwertes können die Makros BS_RGB und BS_ARGB benutzt werden.
	    @param Width gibt die Ausgabebreite des Bildausschnittes an.
	                 Falls diese von der Breite des Bildausschnittes abweicht wird
	                 das Bild entsprechend Skaliert.<br>
	                 Der Wert -1 gibt an, dass das Bild nicht Skaliert werden soll.<br>
	                 Der Standardwert ist -1.
	    @param Width gibt die Ausgabehöhe des Bildausschnittes an.
	                 Falls diese von der Höhe des Bildauschnittes abweicht, wird
	                 das Bild entsprechend Skaliert.<br>
	                 Der Wert -1 gibt an, dass das Bild nicht Skaliert werden soll.<br>
	                 Der Standardwert ist -1.
	    @return Gibt false zurück, falls das Rendern fehlgeschlagen ist.
	    @remark Er werden nicht alle Blitting-Operationen von allen BS_Image-Klassen unterstützt.<br>
	            Mehr Informationen gibt es in der Klassenbeschreibung von BS_Image und durch folgende Methoden:
	            - IsBlitTarget()
	            - IsScalingAllowed()
	            - IsFillingAllowed()
	            - IsAlphaAllowed()
	            - IsColorModulationAllowed()
	*/
	bool blit(int posX = 0, int posY = 0,
	          int flipping = FLIP_NONE,
	          Common::Rect *pSrcPartRect = NULL,
	          uint color = BS_ARGB(255, 255, 255, 255),
	          int width = -1, int height = -1) {
		assert(_pImage);
		return _pImage->blit(posX, posY, flipping, pSrcPartRect, color, width, height);
	}

	/**
	    @brief Füllt einen Rechteckigen Bereich des Bildes mit einer Farbe.
	    @param pFillRect Pointer auf ein Common::Rect, welches den Ausschnitt des Bildes spezifiziert, der gefüllt
	                      werden soll oder NULL, falls das gesamte Bild gefüllt werden soll.<br>
	                      Der Standardwert ist NULL.
	    @param Color der 32 Bit Farbwert mit dem der Bildbereich gefüllt werden soll.
	    @remark Ein Aufruf dieser Methode ist nur gestattet, wenn IsFillingAllowed() true zurückgibt.
	    @remark Es ist möglich über die Methode transparente Rechtecke darzustellen, indem man eine Farbe mit einem Alphawert ungleich
	            255 angibt.
	    @remark Unabhängig vom Farbformat des Bildes muss ein 32 Bit Farbwert angegeben werden. Zur Erzeugung, können die Makros
	            BS_RGB und BS_ARGB benutzt werden.
	    @remark Falls das Rechteck nicht völlig innerhalb des Bildschirms ist, wird es automatisch zurechtgestutzt.
	*/
	bool fill(const Common::Rect *pFillRect = 0, uint color = BS_RGB(0, 0, 0)) {
		assert(_pImage);
		return _pImage->fill(pFillRect, color);
	}

	/**
	    @brief Liest einen Pixel des Bildes.
	    @param X die X-Koordinate des Pixels.
	    @param Y die Y-Koordinate des Pixels
	    @return Gibt den 32-Bit Farbwert des Pixels an der übergebenen Koordinate zurück.
	    @remark Diese Methode sollte auf keine Fall benutzt werden um größere Teile des Bildes zu lesen, da sie sehr langsam ist. Sie ist
	            eher dafür gedacht einzelne Pixel des Bildes auszulesen.
	*/
	uint getPixel(int x, int y) const {
		return _pImage->getPixel(x, y);
	}

	//@{
	/** @name Auskunfts-Methoden */

	/**
	    @brief Überprüft, ob das BS_Image ein Zielbild für einen Blit-Aufruf sein kann.
	    @return Gibt false zurück, falls ein Blit-Aufruf mit diesem Objekt als Ziel nicht gestattet ist.
	*/
	bool isBlitTarget() {
		assert(_pImage);
		return _pImage->isBlitTarget();
	}

	/**
	    @brief Gibt true zurück, falls das BS_Image bei einem Aufruf von Blit() skaliert dargestellt werden kann.
	*/
	bool isScalingAllowed() {
		assert(_pImage);
		return _pImage->isScalingAllowed();
	}

	/**
	    @brief Gibt true zurück, wenn das BS_Image mit einem Aufruf von Fill() gefüllt werden kann.
	*/
	bool isFillingAllowed() {
		assert(_pImage);
		return _pImage->isFillingAllowed();
	}

	/**
	    @brief Gibt true zurück, wenn das BS_Image bei einem Aufruf von Blit() mit einem Alphawert dargestellt werden kann.
	*/
	bool isAlphaAllowed() {
		assert(_pImage);
		return _pImage->isAlphaAllowed();
	}

	/**
	    @brief Gibt true zurück, wenn das BS_Image bei einem Aufruf von Blit() mit Farbmodulation dargestellt werden kann.
	*/
	bool isColorModulationAllowed() {
		assert(_pImage);
		return _pImage->isColorModulationAllowed();
	}

private:
	Image *_pImage;
};

} // End of namespace Sword25

#endif
