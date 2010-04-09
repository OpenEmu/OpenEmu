/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "state_osd_elements.h"
#include "bitmap_font.h"
#include "statesaver.h"
#include <fstream>
#include <cstring>

using namespace BitmapFont;

static const char stateLoadedTxt[] = { S,t,a,t,e,SPC,N0,SPC,l,o,a,d,e,d,0 };
static const char stateSavedTxt[] = { S,t,a,t,e,SPC,N0,SPC,s,a,v,e,d,0 };
static const unsigned stateLoadedTxtWidth = getWidth(stateLoadedTxt);
static const unsigned stateSavedTxtWidth = getWidth(stateSavedTxt);

class ShadedTextOsdElment : public OsdElement {
	struct ShadeFill {
		void operator()(Gambatte::uint_least32_t *dest, const unsigned pitch) {
			dest[2] = dest[1] = dest[0] = 0x000000ul;
			dest += pitch;
			dest[2] = dest[0] = 0x000000ul;
			dest += pitch;
			dest[2] = dest[1] = dest[0] = 0x000000ul;
		}
	};
	
	Gambatte::uint_least32_t *const pixels;
	unsigned life;
	
public:
	ShadedTextOsdElment(unsigned w, const char *txt);
	~ShadedTextOsdElment();
	const Gambatte::uint_least32_t* update();
};

ShadedTextOsdElment::ShadedTextOsdElment(unsigned width, const char *txt) :
OsdElement(MAX_WIDTH, 144 - HEIGHT - HEIGHT, width + 2, HEIGHT + 2, THREE_FOURTHS),
pixels(new Gambatte::uint_least32_t[w() * h()]),
life(4 * 60) {
	std::memset(pixels, 0xFF, w() * h() * sizeof(Gambatte::uint_least32_t));
	
	/*print(pixels + 0 * w() + 0, w(), 0x000000ul, txt);
	print(pixels + 0 * w() + 1, w(), 0x000000ul, txt);
	print(pixels + 0 * w() + 2, w(), 0x000000ul, txt);
	print(pixels + 1 * w() + 0, w(), 0x000000ul, txt);
	print(pixels + 1 * w() + 2, w(), 0x000000ul, txt);
	print(pixels + 2 * w() + 0, w(), 0x000000ul, txt);
	print(pixels + 2 * w() + 1, w(), 0x000000ul, txt);
	print(pixels + 2 * w() + 2, w(), 0x000000ul, txt);
	print(pixels + 1 * w() + 1, w(), 0xE0E0E0ul, txt);*/
	
	print(pixels, w(), ShadeFill(), txt);
	print(pixels + 1 * w() + 1, w(), 0xE0E0E0ul, txt);
}

ShadedTextOsdElment::~ShadedTextOsdElment() {
	delete []pixels;
}

const Gambatte::uint_least32_t* ShadedTextOsdElment::update() {
	if (life--)
		return pixels;
	
	return 0;
}

/*class FramedTextOsdElment : public OsdElement {
	Gambatte::uint_least32_t *const pixels;
	unsigned life;
	
public:
	FramedTextOsdElment(unsigned w, const char *txt);
	~FramedTextOsdElment();
	const Gambatte::uint_least32_t* update();
};

FramedTextOsdElment::FramedTextOsdElment(unsigned width, const char *txt) :
OsdElement(NUMBER_WIDTH, 144 - HEIGHT * 2 - HEIGHT / 2, width + NUMBER_WIDTH * 2, HEIGHT * 2),
pixels(new Gambatte::uint_least32_t[w() * h()]),
life(4 * 60) {
	std::memset(pixels, 0x00, w() * h() * sizeof(Gambatte::uint_least32_t));
	print(pixels + (w() - width) / 2 + ((h() - HEIGHT) / 2) * w(), w(), 0xA0A0A0ul, txt);
}

FramedTextOsdElment::~FramedTextOsdElment() {
	delete []pixels;
}

const Gambatte::uint_least32_t* FramedTextOsdElment::update() {
	if (life--)
		return pixels;
	
	return 0;
}*/

std::auto_ptr<OsdElement> newStateLoadedOsdElement(unsigned stateNo) {
	char txt[sizeof(stateLoadedTxt)];
	
	std::memcpy(txt, stateLoadedTxt, sizeof(stateLoadedTxt));
	utoa(stateNo, txt + 6);
	
	return std::auto_ptr<OsdElement>(new ShadedTextOsdElment(stateLoadedTxtWidth, txt));
}

std::auto_ptr<OsdElement> newStateSavedOsdElement(unsigned stateNo) {
	char txt[sizeof(stateSavedTxt)];
	
	std::memcpy(txt, stateSavedTxt, sizeof(stateSavedTxt));
	utoa(stateNo, txt + 6);
	
	return std::auto_ptr<OsdElement>(new ShadedTextOsdElment(stateSavedTxtWidth, txt));
}

class SaveStateOsdElement : public OsdElement {
	Gambatte::uint_least32_t pixels[StateSaver::SS_WIDTH * StateSaver::SS_HEIGHT];
	unsigned life;
	
public:
	SaveStateOsdElement(const char *fileName, unsigned stateNo);
	const Gambatte::uint_least32_t* update();
};

SaveStateOsdElement::SaveStateOsdElement(const char *fileName, unsigned stateNo) :
OsdElement((stateNo ? stateNo - 1 : 9) * ((160 - StateSaver::SS_WIDTH) / 10) + ((160 - StateSaver::SS_WIDTH) / 10) / 2, 4, StateSaver::SS_WIDTH, StateSaver::SS_HEIGHT),
life(4 * 60) {
	std::ifstream file(fileName, std::ios_base::binary);
	
	if (file.is_open()) {
		file.ignore(5);
		file.read(reinterpret_cast<char*>(pixels), sizeof(pixels));
	} else {
		std::memset(pixels, 0, sizeof(pixels));
		
		{
			using namespace BitmapFont;
			
			static const char txt[] = { E,m,p,t,BitmapFont::y,0 };
			
			print(pixels + 3 + (StateSaver::SS_HEIGHT / 2 - BitmapFont::HEIGHT / 2) * StateSaver::SS_WIDTH, StateSaver::SS_WIDTH, 0x808080ul, txt);
		}
	}
}

const Gambatte::uint_least32_t* SaveStateOsdElement::update() {
	if (life--)
		return pixels;
	
	return 0;
}

std::auto_ptr<OsdElement> newSaveStateOsdElement(const char *fileName, unsigned stateNo) {
	return std::auto_ptr<OsdElement>(new SaveStateOsdElement(fileName, stateNo));
}
