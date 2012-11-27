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

#include "common/file.h"
#include "common/timer.h"
#include "common/util.h"
#include "common/system.h"

#include "graphics/palette.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxPalette::GfxPalette(ResourceManager *resMan, GfxScreen *screen)
	: _resMan(resMan), _screen(screen) {
	int16 color;

	_sysPalette.timestamp = 0;
	for (color = 0; color < 256; color++) {
		_sysPalette.colors[color].used = 0;
		_sysPalette.colors[color].r = 0;
		_sysPalette.colors[color].g = 0;
		_sysPalette.colors[color].b = 0;
		_sysPalette.intensity[color] = 100;
		_sysPalette.mapping[color] = color;
	}
	// Black and white are hardcoded
	_sysPalette.colors[0].used = 1;
	_sysPalette.colors[255].used = 1;
	_sysPalette.colors[255].r = 255;
	_sysPalette.colors[255].g = 255;
	_sysPalette.colors[255].b = 255;

	_sysPaletteChanged = false;

	// Quest for Glory 3 demo, Eco Quest 1 demo, Laura Bow 2 demo, Police Quest
	// 1 vga and all Nick's Picks all use the older palette format and thus are
	// not using the SCI1.1 palette merging (copying over all the colors) but
	// the real merging done in earlier games. If we use the copying over, we
	// will get issues because some views have marked all colors as being used
	// and those will overwrite the current palette in that case
	if (getSciVersion() < SCI_VERSION_1_1)
		_useMerging = true;
	else if (getSciVersion() == SCI_VERSION_1_1)
		// there are some games that use inbetween SCI1.1 interpreter, so we have
		// to detect if the current game is merging or copying
		_useMerging = _resMan->detectPaletteMergingSci11();
	else	// SCI32
		_useMerging = false;

	palVaryInit();

	_macClut = 0;
	loadMacIconBarPalette();

#ifdef ENABLE_SCI32
	_clutTable = 0;
#endif

	switch (_resMan->getViewType()) {
	case kViewEga:
		_totalScreenColors = 16;
		break;
	case kViewAmiga:
		_totalScreenColors = 32;
		break;
	case kViewAmiga64:
		_totalScreenColors = 64;
		break;
	case kViewVga:
	case kViewVga11:
			_totalScreenColors = 256;
		break;
	default:
		error("GfxPalette: Unknown view type");
	}

	_remapOn = false;
	resetRemapping();
}

GfxPalette::~GfxPalette() {
	if (_palVaryResourceId != -1)
		palVaryRemoveTimer();

	delete[] _macClut;

#ifdef ENABLE_SCI32
	unloadClut();
#endif
}

bool GfxPalette::isMerging() {
	return _useMerging;
}

// meant to get called only once during init of engine
void GfxPalette::setDefault() {
	if (_resMan->getViewType() == kViewEga)
		setEGA();
	else if (_resMan->getViewType() == kViewAmiga || _resMan->getViewType() == kViewAmiga64)
		setAmiga();
	else
		kernelSetFromResource(999, true);
}

#define SCI_PAL_FORMAT_CONSTANT 1
#define SCI_PAL_FORMAT_VARIABLE 0

void GfxPalette::createFromData(byte *data, int bytesLeft, Palette *paletteOut) {
	int palFormat = 0;
	int palOffset = 0;
	int palColorStart = 0;
	int palColorCount = 0;
	int colorNo = 0;

	memset(paletteOut, 0, sizeof(Palette));

	// Setup 1:1 mapping
	for (colorNo = 0; colorNo < 256; colorNo++) {
		paletteOut->mapping[colorNo] = colorNo;
	}

	if (bytesLeft < 37) {
		// This happens when loading palette of picture 0 in sq5 - the resource is broken and doesn't contain a full
		//  palette
		debugC(kDebugLevelResMan, "GfxPalette::createFromData() - not enough bytes in resource (%d), expected palette header", bytesLeft);
		return;
	}

	// palette formats in here are not really version exclusive, we can not use sci-version to differentiate between them
	//  they were just called that way, because they started appearing in sci1.1 for example
	if ((data[0] == 0 && data[1] == 1) || (data[0] == 0 && data[1] == 0 && READ_SCI11ENDIAN_UINT16(data + 29) == 0)) {
		// SCI0/SCI1 palette
		palFormat = SCI_PAL_FORMAT_VARIABLE; // CONSTANT;
		palOffset = 260;
		palColorStart = 0; palColorCount = 256;
		//memcpy(&paletteOut->mapping, data, 256);
	} else {
		// SCI1.1 palette
		palFormat = data[32];
		palOffset = 37;
		palColorStart = data[25];
		palColorCount = READ_SCI11ENDIAN_UINT16(data + 29);
	}

	switch (palFormat) {
		case SCI_PAL_FORMAT_CONSTANT:
			// Check, if enough bytes left
			if (bytesLeft < palOffset + (3 * palColorCount)) {
				warning("GfxPalette::createFromData() - not enough bytes in resource, expected palette colors");
				return;
			}

			for (colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
				paletteOut->colors[colorNo].used = 1;
				paletteOut->colors[colorNo].r = data[palOffset++];
				paletteOut->colors[colorNo].g = data[palOffset++];
				paletteOut->colors[colorNo].b = data[palOffset++];
			}
			break;
		case SCI_PAL_FORMAT_VARIABLE:
			if (bytesLeft < palOffset + (4 * palColorCount)) {
				warning("GfxPalette::createFromData() - not enough bytes in resource, expected palette colors");
				return;
			}

			for (colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
				paletteOut->colors[colorNo].used = data[palOffset++];
				paletteOut->colors[colorNo].r = data[palOffset++];
				paletteOut->colors[colorNo].g = data[palOffset++];
				paletteOut->colors[colorNo].b = data[palOffset++];
			}
			break;
	}
}

// Will try to set amiga palette by using "spal" file. If not found, we return false
bool GfxPalette::setAmiga() {
	Common::File file;

	if (file.open("spal")) {
		for (int curColor = 0; curColor < 32; curColor++) {
			byte byte1 = file.readByte();
			byte byte2 = file.readByte();

			if (file.eos())
				error("Amiga palette file ends prematurely");

			_sysPalette.colors[curColor].used = 1;
			_sysPalette.colors[curColor].r = (byte1 & 0x0F) * 0x11;
			_sysPalette.colors[curColor].g = ((byte2 & 0xF0) >> 4) * 0x11;
			_sysPalette.colors[curColor].b = (byte2 & 0x0F) * 0x11;

			if (_totalScreenColors == 64) {
				// Set the associated color from the Amiga halfbrite colors
				_sysPalette.colors[curColor + 32].used = 1;
				_sysPalette.colors[curColor + 32].r = _sysPalette.colors[curColor].r >> 1;
				_sysPalette.colors[curColor + 32].g = _sysPalette.colors[curColor].g >> 1;
				_sysPalette.colors[curColor + 32].b = _sysPalette.colors[curColor].b >> 1;
			}
		}

		// Directly set the palette, because setOnScreen() wont do a thing for amiga
		copySysPaletteToScreen();
		return true;
	}

	return false;
}

// Called from picture class, some amiga sci1 games set half of the palette
void GfxPalette::modifyAmigaPalette(byte *data) {
	int16 curPos = 0;

	for (int curColor = 0; curColor < 16; curColor++) {
		byte byte1 = data[curPos++];
		byte byte2 = data[curPos++];
		_sysPalette.colors[curColor].r = (byte1 & 0x0F) * 0x11;
		_sysPalette.colors[curColor].g = ((byte2 & 0xF0) >> 4) * 0x11;
		_sysPalette.colors[curColor].b = (byte2 & 0x0F) * 0x11;

		if (_totalScreenColors == 64) {
			// Set the associated color from the Amiga halfbrite colors
			_sysPalette.colors[curColor + 32].r = _sysPalette.colors[curColor].r >> 1;
			_sysPalette.colors[curColor + 32].g = _sysPalette.colors[curColor].g >> 1;
			_sysPalette.colors[curColor + 32].b = _sysPalette.colors[curColor].b >> 1;
		}
	}

	copySysPaletteToScreen();
}

static byte blendColors(byte c1, byte c2) {
	// linear
	// return (c1/2+c2/2)+((c1&1)+(c2&1))/2;

	// gamma 2.2
	double t = (pow(c1/255.0, 2.2/1.0) * 255.0) +
	           (pow(c2/255.0, 2.2/1.0) * 255.0);
	return (byte)(0.5 + (pow(0.5*t/255.0, 1.0/2.2) * 255.0));
}

void GfxPalette::setEGA() {
	int curColor;
	byte color1, color2;

	_sysPalette.colors[1].r  = 0x000; _sysPalette.colors[1].g  = 0x000; _sysPalette.colors[1].b  = 0x0AA;
	_sysPalette.colors[2].r  = 0x000; _sysPalette.colors[2].g  = 0x0AA; _sysPalette.colors[2].b  = 0x000;
	_sysPalette.colors[3].r  = 0x000; _sysPalette.colors[3].g  = 0x0AA; _sysPalette.colors[3].b  = 0x0AA;
	_sysPalette.colors[4].r  = 0x0AA; _sysPalette.colors[4].g  = 0x000; _sysPalette.colors[4].b  = 0x000;
	_sysPalette.colors[5].r  = 0x0AA; _sysPalette.colors[5].g  = 0x000; _sysPalette.colors[5].b  = 0x0AA;
	_sysPalette.colors[6].r  = 0x0AA; _sysPalette.colors[6].g  = 0x055; _sysPalette.colors[6].b  = 0x000;
	_sysPalette.colors[7].r  = 0x0AA; _sysPalette.colors[7].g  = 0x0AA; _sysPalette.colors[7].b  = 0x0AA;
	_sysPalette.colors[8].r  = 0x055; _sysPalette.colors[8].g  = 0x055; _sysPalette.colors[8].b  = 0x055;
	_sysPalette.colors[9].r  = 0x055; _sysPalette.colors[9].g  = 0x055; _sysPalette.colors[9].b  = 0x0FF;
	_sysPalette.colors[10].r = 0x055; _sysPalette.colors[10].g = 0x0FF; _sysPalette.colors[10].b = 0x055;
	_sysPalette.colors[11].r = 0x055; _sysPalette.colors[11].g = 0x0FF; _sysPalette.colors[11].b = 0x0FF;
	_sysPalette.colors[12].r = 0x0FF; _sysPalette.colors[12].g = 0x055; _sysPalette.colors[12].b = 0x055;
	_sysPalette.colors[13].r = 0x0FF; _sysPalette.colors[13].g = 0x055; _sysPalette.colors[13].b = 0x0FF;
	_sysPalette.colors[14].r = 0x0FF; _sysPalette.colors[14].g = 0x0FF; _sysPalette.colors[14].b = 0x055;
	_sysPalette.colors[15].r = 0x0FF; _sysPalette.colors[15].g = 0x0FF; _sysPalette.colors[15].b = 0x0FF;
	for (curColor = 0; curColor <= 15; curColor++) {
		_sysPalette.colors[curColor].used = 1;
	}
	// Now setting colors 16-254 to the correct mix colors that occur when not doing a dithering run on
	//  finished pictures
	for (curColor = 0x10; curColor <= 0xFE; curColor++) {
		_sysPalette.colors[curColor].used = 1;
		color1 = curColor & 0x0F; color2 = curColor >> 4;

		_sysPalette.colors[curColor].r = blendColors(_sysPalette.colors[color1].r, _sysPalette.colors[color2].r);
		_sysPalette.colors[curColor].g = blendColors(_sysPalette.colors[color1].g, _sysPalette.colors[color2].g);
		_sysPalette.colors[curColor].b = blendColors(_sysPalette.colors[color1].b, _sysPalette.colors[color2].b);
	}
	_sysPalette.timestamp = 1;
	setOnScreen();
}

void GfxPalette::set(Palette *newPalette, bool force, bool forceRealMerge) {
	uint32 systime = _sysPalette.timestamp;

	if (force || newPalette->timestamp != systime) {
		// SCI1.1+ doesnt do real merging anymore, but simply copying over the used colors from other palettes
		//  There are some games with inbetween SCI1.1 interpreters, use real merging for them (e.g. laura bow 2 demo)
		if ((forceRealMerge) || (_useMerging))
			_sysPaletteChanged |= merge(newPalette, force, forceRealMerge);
		else
			_sysPaletteChanged |= insert(newPalette, &_sysPalette);

		// Adjust timestamp on newPalette, so it wont get merged/inserted w/o need
		newPalette->timestamp = _sysPalette.timestamp;

		bool updatePalette = _sysPaletteChanged && _screen->_picNotValid == 0;

		if (_palVaryResourceId != -1) {
			// Pal-vary currently active, we don't set at any time, but also insert into origin palette
			insert(newPalette, &_palVaryOriginPalette);
			palVaryProcess(0, updatePalette);
			return;
		}

		if (updatePalette) {
			setOnScreen();
			_sysPaletteChanged = false;
		}
	}
}

byte GfxPalette::remapColor(byte remappedColor, byte screenColor) {
	assert(_remapOn);
	if (_remappingType[remappedColor] == kRemappingByRange)
		return _remappingByRange[screenColor];
	else if (_remappingType[remappedColor] == kRemappingByPercent)
		return _remappingByPercent[screenColor];
	else
		error("remapColor(): Color %d isn't remapped", remappedColor);

	return 0;	// should never reach here
}

void GfxPalette::resetRemapping() {
	_remapOn = false;
	_remappingPercentToSet = 0;

	for (int i = 0; i < 256; i++) {
		_remappingType[i] = kRemappingNone;
		_remappingByPercent[i] = i;
		_remappingByRange[i] = i;
	}
}

void GfxPalette::setRemappingPercent(byte color, byte percent) {
	_remapOn = true;

	// We need to defer the setup of the remapping table every time the screen
	// palette is changed, so that kernelFindColor() can find the correct
	// colors. Set it once here, in case the palette stays the same and update
	// it on each palette change by copySysPaletteToScreen().
	_remappingPercentToSet = percent;

	for (int i = 0; i < 256; i++) {
		byte r = _sysPalette.colors[i].r * _remappingPercentToSet / 100;
		byte g = _sysPalette.colors[i].g * _remappingPercentToSet / 100;
		byte b = _sysPalette.colors[i].b * _remappingPercentToSet / 100;
		_remappingByPercent[i] = kernelFindColor(r, g, b);
	}

	_remappingType[color] = kRemappingByPercent;
}

void GfxPalette::setRemappingPercentGray(byte color, byte percent) {
	_remapOn = true;

	// We need to defer the setup of the remapping table every time the screen
	// palette is changed, so that kernelFindColor() can find the correct
	// colors. Set it once here, in case the palette stays the same and update
	// it on each palette change by copySysPaletteToScreen().
	_remappingPercentToSet = percent;

	// Note: This is not what the original does, but the results are the same visually
	for (int i = 0; i < 256; i++) {
		byte rComponent = _sysPalette.colors[i].r * _remappingPercentToSet * 0.30 / 100;
		byte gComponent = _sysPalette.colors[i].g * _remappingPercentToSet * 0.59 / 100;
		byte bComponent = _sysPalette.colors[i].b * _remappingPercentToSet * 0.11 / 100;
		byte luminosity = rComponent + gComponent + bComponent;
		_remappingByPercent[i] = kernelFindColor(luminosity, luminosity, luminosity);
	}

	_remappingType[color] = kRemappingByPercent;
}

void GfxPalette::setRemappingRange(byte color, byte from, byte to, byte base) {
	_remapOn = true;

	for (int i = from; i <= to; i++) {
		_remappingByRange[i] = i + base;
	}

	_remappingType[color] = kRemappingByRange;
}

bool GfxPalette::insert(Palette *newPalette, Palette *destPalette) {
	bool paletteChanged = false;

	for (int i = 1; i < 255; i++) {
		if (newPalette->colors[i].used) {
			if ((newPalette->colors[i].r != destPalette->colors[i].r) ||
				(newPalette->colors[i].g != destPalette->colors[i].g) ||
				(newPalette->colors[i].b != destPalette->colors[i].b)) {
				destPalette->colors[i].r = newPalette->colors[i].r;
				destPalette->colors[i].g = newPalette->colors[i].g;
				destPalette->colors[i].b = newPalette->colors[i].b;
				paletteChanged = true;
			}
			destPalette->colors[i].used = newPalette->colors[i].used;
			newPalette->mapping[i] = i;
		}
	}

	// We don't update the timestamp for SCI1.1, it's only updated on kDrawPic calls
	return paletteChanged;
}

bool GfxPalette::merge(Palette *newPalette, bool force, bool forceRealMerge) {
	uint16 res;
	bool paletteChanged = false;

	for (int i = 1; i < 255; i++) {
		// skip unused colors
		if (!newPalette->colors[i].used)
			continue;

		// forced palette merging or dest color is not used yet
		if (force || (!_sysPalette.colors[i].used)) {
			_sysPalette.colors[i].used = newPalette->colors[i].used;
			if ((newPalette->colors[i].r != _sysPalette.colors[i].r) ||
				(newPalette->colors[i].g != _sysPalette.colors[i].g) ||
				(newPalette->colors[i].b != _sysPalette.colors[i].b)) {
				_sysPalette.colors[i].r = newPalette->colors[i].r;
				_sysPalette.colors[i].g = newPalette->colors[i].g;
				_sysPalette.colors[i].b = newPalette->colors[i].b;
				paletteChanged = true;
			}
			newPalette->mapping[i] = i;
			continue;
		}

		// is the same color already at the same position? -> match it directly w/o lookup
		//  this fixes games like lsl1demo/sq5 where the same rgb color exists multiple times and where we would
		//  otherwise match the wrong one (which would result into the pixels affected (or not) by palette changes)
		if ((_sysPalette.colors[i].r == newPalette->colors[i].r) &&
			(_sysPalette.colors[i].g == newPalette->colors[i].g) &&
			(_sysPalette.colors[i].b == newPalette->colors[i].b)) {
			newPalette->mapping[i] = i;
			continue;
		}

		// check if exact color could be matched
		res = matchColor(newPalette->colors[i].r, newPalette->colors[i].g, newPalette->colors[i].b);
		if (res & 0x8000) { // exact match was found
			newPalette->mapping[i] = res & 0xFF;
			continue;
		}

		int j = 1;

		// no exact match - see if there is an unused color
		for (; j < 256; j++) {
			if (!_sysPalette.colors[j].used) {
				_sysPalette.colors[j].used = newPalette->colors[i].used;
				_sysPalette.colors[j].r = newPalette->colors[i].r;
				_sysPalette.colors[j].g = newPalette->colors[i].g;
				_sysPalette.colors[j].b = newPalette->colors[i].b;
				newPalette->mapping[i] = j;
				paletteChanged = true;
				break;
			}
		}

		// if still no luck - set an approximate color
		if (j == 256) {
			newPalette->mapping[i] = res & 0xFF;
			_sysPalette.colors[res & 0xFF].used |= 0x10;
		}
	}

	if (!forceRealMerge)
		_sysPalette.timestamp = g_system->getMillis() * 60 / 1000;

	return paletteChanged;
}

// This is called for SCI1.1, when kDrawPic got done. We update sysPalette timestamp this way for SCI1.1 and also load
//  target-palette, if palvary is active
void GfxPalette::drewPicture(GuiResourceId pictureId) {
	if (!_useMerging) // Don't do this on inbetween SCI1.1 games
		_sysPalette.timestamp++;

	if (_palVaryResourceId != -1) {
		if (g_sci->getEngineState()->gameIsRestarting == 0) // only if not restored nor restarted
			palVaryLoadTargetPalette(pictureId);
	}
}

uint16 GfxPalette::matchColor(byte r, byte g, byte b) {
	byte found = 0xFF;
	int diff = 0x2FFFF, cdiff;
	int16 dr,dg,db;

	for (int i = 1; i < 255; i++) {
		if ((!_sysPalette.colors[i].used))
			continue;
		dr = _sysPalette.colors[i].r - r;
		dg = _sysPalette.colors[i].g - g;
		db = _sysPalette.colors[i].b - b;
//		minimum squares match
		cdiff = (dr*dr) + (dg*dg) + (db*db);
//		minimum sum match (Sierra's)
//		cdiff = ABS(dr) + ABS(dg) + ABS(db);
		if (cdiff < diff) {
			if (cdiff == 0)
				return i | 0x8000; // setting this flag to indicate exact match
			found = i;
			diff = cdiff;
		}
	}
	return found;
}

void GfxPalette::getSys(Palette *pal) {
	if (pal != &_sysPalette)
		memcpy(pal, &_sysPalette,sizeof(Palette));
}

void GfxPalette::setOnScreen() {
	copySysPaletteToScreen();
}

static byte convertMacGammaToSCIGamma(int comp) {
	return (byte)sqrt(comp * 255.0f);
}

void GfxPalette::copySysPaletteToScreen() {
	// just copy palette to system
	byte bpal[3 * 256];

	// Get current palette, update it and put back
	g_system->getPaletteManager()->grabPalette(bpal, 0, 256);

	for (int16 i = 0; i < 256; i++) {
		if (colorIsFromMacClut(i)) {
			// If we've got a Mac CLUT, override the SCI palette with its non-black colors
			bpal[i * 3    ] = convertMacGammaToSCIGamma(_macClut[i * 3    ]);
			bpal[i * 3 + 1] = convertMacGammaToSCIGamma(_macClut[i * 3 + 1]);
			bpal[i * 3 + 2] = convertMacGammaToSCIGamma(_macClut[i * 3 + 2]);
		} else if (_sysPalette.colors[i].used != 0) {
			// Otherwise, copy to the screen
			bpal[i * 3    ] = CLIP(_sysPalette.colors[i].r * _sysPalette.intensity[i] / 100, 0, 255);
			bpal[i * 3 + 1] = CLIP(_sysPalette.colors[i].g * _sysPalette.intensity[i] / 100, 0, 255);
			bpal[i * 3 + 2] = CLIP(_sysPalette.colors[i].b * _sysPalette.intensity[i] / 100, 0, 255);
		}
	}

	// Check if we need to reset remapping by percent with the new colors.
	if (_remappingPercentToSet) {
		for (int i = 0; i < 256; i++) {
			byte r = _sysPalette.colors[i].r * _remappingPercentToSet / 100;
			byte g = _sysPalette.colors[i].g * _remappingPercentToSet / 100;
			byte b = _sysPalette.colors[i].b * _remappingPercentToSet / 100;
			_remappingByPercent[i] = kernelFindColor(r, g, b);
		}
	}

	g_system->getPaletteManager()->setPalette(bpal, 0, 256);
}

bool GfxPalette::kernelSetFromResource(GuiResourceId resourceId, bool force) {
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);
	Palette palette;

	if (palResource) {
		createFromData(palResource->data, palResource->size, &palette);
		set(&palette, force);
		return true;
	}

	return false;
}

void GfxPalette::kernelSetFlag(uint16 fromColor, uint16 toColor, uint16 flag) {
	uint16 colorNr;
	for (colorNr = fromColor; colorNr < toColor; colorNr++) {
		_sysPalette.colors[colorNr].used |= flag;
	}
}

void GfxPalette::kernelUnsetFlag(uint16 fromColor, uint16 toColor, uint16 flag) {
	uint16 colorNr;
	for (colorNr = fromColor; colorNr < toColor; colorNr++) {
		_sysPalette.colors[colorNr].used &= ~flag;
	}
}

void GfxPalette::kernelSetIntensity(uint16 fromColor, uint16 toColor, uint16 intensity, bool setPalette) {
	memset(&_sysPalette.intensity[0] + fromColor, intensity, toColor - fromColor);
	if (setPalette) {
		setOnScreen();
		EngineState *state = g_sci->getEngineState();
		// Call speed throttler from here as well just in case we need it
		//  At least in kq6 intro the scripts call us in a tight loop for fadein/fadeout
		state->speedThrottler(30);
		state->_throttleTrigger = true;
	}
}

int16 GfxPalette::kernelFindColor(uint16 r, uint16 g, uint16 b) {
	return matchColor(r, g, b) & 0xFF;
}

// Returns true, if palette got changed
bool GfxPalette::kernelAnimate(byte fromColor, byte toColor, int speed) {
	Color col;
	//byte colorNr;
	int16 colorCount;
	uint32 now = g_system->getMillis() * 60 / 1000;

	// search for sheduled animations with the same 'from' value
	// schedule animation...
	int scheduleCount = _schedules.size();
	int scheduleNr;
	for (scheduleNr = 0; scheduleNr < scheduleCount; scheduleNr++) {
		if (_schedules[scheduleNr].from == fromColor)
			break;
	}
	if (scheduleNr == scheduleCount) {
		// adding a new schedule
		PalSchedule newSchedule;
		newSchedule.from = fromColor;
		newSchedule.schedule = now + ABS(speed);
		_schedules.push_back(newSchedule);
		scheduleCount++;
	}

	g_sci->getEngineState()->_throttleTrigger = true;

	for (scheduleNr = 0; scheduleNr < scheduleCount; scheduleNr++) {
		if (_schedules[scheduleNr].from == fromColor) {
			if (_schedules[scheduleNr].schedule <= now) {
				if (speed > 0) {
					// TODO: Not really sure about this, sierra sci seems to do exactly this here
					col = _sysPalette.colors[fromColor];
					if (fromColor < toColor) {
						colorCount = toColor - fromColor - 1;
						memmove(&_sysPalette.colors[fromColor], &_sysPalette.colors[fromColor + 1], colorCount * sizeof(Color));
					}
					_sysPalette.colors[toColor - 1] = col;
				} else {
					col = _sysPalette.colors[toColor - 1];
					if (fromColor < toColor) {
						colorCount = toColor - fromColor - 1;
						memmove(&_sysPalette.colors[fromColor + 1], &_sysPalette.colors[fromColor], colorCount * sizeof(Color));
					}
					_sysPalette.colors[fromColor] = col;
				}
				// removing schedule
				_schedules[scheduleNr].schedule = now + ABS(speed);
				// TODO: Not sure when sierra actually removes a schedule
				//_schedules.remove_at(scheduleNr);
				return true;
			}
			return false;
		}
	}
	return false;
}

void GfxPalette::kernelAnimateSet() {
	setOnScreen();
}

reg_t GfxPalette::kernelSave() {
	SegManager *segMan = g_sci->getEngineState()->_segMan;
	reg_t memoryId = segMan->allocateHunkEntry("kPalette(save)", 1024);
	byte *memoryPtr = segMan->getHunkPointer(memoryId);
	if (memoryPtr) {
		for (int colorNr = 0; colorNr < 256; colorNr++) {
			*memoryPtr++ = _sysPalette.colors[colorNr].used;
			*memoryPtr++ = _sysPalette.colors[colorNr].r;
			*memoryPtr++ = _sysPalette.colors[colorNr].g;
			*memoryPtr++ = _sysPalette.colors[colorNr].b;
		}
	}
	return memoryId;
}

void GfxPalette::kernelRestore(reg_t memoryHandle) {
	SegManager *segMan = g_sci->getEngineState()->_segMan;
	if (!memoryHandle.isNull()) {
		byte *memoryPtr = segMan->getHunkPointer(memoryHandle);
		if (!memoryPtr)
			error("Bad handle used for kPalette(restore)");

		Palette restoredPalette;

		restoredPalette.timestamp = 0;
		for (int colorNr = 0; colorNr < 256; colorNr++) {
			restoredPalette.colors[colorNr].used = *memoryPtr++;
			restoredPalette.colors[colorNr].r = *memoryPtr++;
			restoredPalette.colors[colorNr].g = *memoryPtr++;
			restoredPalette.colors[colorNr].b = *memoryPtr++;
		}

		set(&restoredPalette, true);
	}
}

void GfxPalette::kernelAssertPalette(GuiResourceId resourceId) {
	// Sometimes invalid viewIds are asked for, ignore those (e.g. qfg1vga)
	//if (!_resMan->testResource(ResourceId(kResourceTypeView, resourceId)))
	//	return;
	// maybe we took the wrong parameter before, if this causes invalid view again, enable to commented out code again

	GfxView *view = g_sci->_gfxCache->getView(resourceId);
	Palette *viewPalette = view->getPalette();
	if (viewPalette) {
		// merge/insert this palette
		set(viewPalette, true);
	}
}

void GfxPalette::kernelSyncScreenPalette() {
	// just copy palette to system
	byte bpal[3 * 256];

	// Get current palette, update it and put back
	g_system->getPaletteManager()->grabPalette(bpal, 0, 256);
	for (int16 i = 1; i < 255; i++) {
		_sysPalette.colors[i].r = bpal[i * 3];
		_sysPalette.colors[i].g = bpal[i * 3 + 1];
		_sysPalette.colors[i].b = bpal[i * 3 + 2];
	}
}

// palVary
//  init - only does, if palVaryOn == false
//         target, start, new palette allocation
//         palVaryOn = true
//         palDirection = 1
//         palStop = 64
//         palTime = from caller
//         copy resource palette to target
//         init target palette (used = 1 on all colors, color 0 = RGB 0, 0, 0 color 255 = RGB 0xFF, 0xFF, 0xFF
//         copy sysPalette to startPalette
//         init new palette like target palette
//         palPercent = 1
//         do various things
//         return 1
//  deinit - unloads target palette, kills timer hook, disables palVaryOn
//  pause - counts up or down, if counter != 0 -> signal wont get counted up by timer
//           will only count down to 0
//
// Restarting game
//         palVary = false
//         palPercent = 0
//         call palVary deinit
//
// Saving/restoring
//         need to save start and target-palette, when palVaryOn = true

void GfxPalette::palVaryInit() {
	_palVaryResourceId = -1;
	_palVaryPaused = 0;
	_palVarySignal = 0;
	_palVaryStep = 0;
	_palVaryStepStop = 0;
	_palVaryDirection = 0;
	_palVaryTicks = 0;
}

bool GfxPalette::palVaryLoadTargetPalette(GuiResourceId resourceId) {
	_palVaryResourceId = (resourceId != 65535) ? resourceId : -1;
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);
	if (palResource) {
		// Load and initialize destination palette
		createFromData(palResource->data, palResource->size, &_palVaryTargetPalette);
		return true;
	}
	return false;
}

void GfxPalette::palVaryInstallTimer() {
	// Remove any possible leftover palVary timer callbacks.
	// This happens for example in QFG1VGA, when sleeping at Erana's place
	// (bug #3439240) - the nighttime to daytime effect clashes with the
	// scene transition effect, as we load scene images too quickly for
	// the SCI scripts in that case (also refer to kernelPalVaryInit).
	palVaryRemoveTimer();

	int16 ticks = _palVaryTicks > 0 ? _palVaryTicks : 1;
	// Call signal increase every [ticks]
	g_sci->getTimerManager()->installTimerProc(&palVaryCallback, 1000000 / 60 * ticks, this, "sciPalette");
}

void GfxPalette::palVaryRemoveTimer() {
	g_sci->getTimerManager()->removeTimerProc(&palVaryCallback);
}

bool GfxPalette::kernelPalVaryInit(GuiResourceId resourceId, uint16 ticks, uint16 stepStop, uint16 direction) {
	if (_palVaryResourceId != -1)	// another palvary is taking place, return
		return false;

	if (palVaryLoadTargetPalette(resourceId)) {
		// Save current palette
		memcpy(&_palVaryOriginPalette, &_sysPalette, sizeof(Palette));

		_palVarySignal = 0;
		_palVaryTicks = ticks;
		_palVaryStep = 1;
		_palVaryStepStop = stepStop;
		_palVaryDirection = direction;
		// if no ticks are given, jump directly to destination
		if (!_palVaryTicks) {
			_palVaryDirection = stepStop;
			// sierra sci set the timer to 1 tick instead of calling it directly
			//  we have to change this to prevent a race condition to happen in
			//  at least freddy pharkas during nighttime. In that case kPalVary is
			//  called right before a transition and because we load pictures much
			//  faster, the 1 tick won't pass sometimes resulting in the palette
			//  being daytime instead of nighttime during the transition.
			palVaryProcess(1, true);
		} else {
			palVaryInstallTimer();
		}
		return true;
	}
	return false;
}

int16 GfxPalette::kernelPalVaryReverse(int16 ticks, uint16 stepStop, int16 direction) {
	if (_palVaryResourceId == -1)
		return 0;

	if (_palVaryStep > 64)
		_palVaryStep = 64;
	if (ticks != -1)
		_palVaryTicks = ticks;
	_palVaryStepStop = stepStop;
	_palVaryDirection = direction != -1 ? -direction : -_palVaryDirection;

	if (!_palVaryTicks) {
		_palVaryDirection = _palVaryStepStop - _palVaryStep;
		// ffs. see palVaryInit right above, we fix the code here as well
		//  just in case
		palVaryProcess(1, true);
	} else {
		palVaryInstallTimer();
	}
	return kernelPalVaryGetCurrentStep();
}

int16 GfxPalette::kernelPalVaryGetCurrentStep() {
	if (_palVaryDirection >= 0)
		return _palVaryStep;
	return -_palVaryStep;
}

int16 GfxPalette::kernelPalVaryChangeTarget(GuiResourceId resourceId) {
	if (_palVaryResourceId != -1) {
		Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);
		if (palResource) {
			Palette insertPalette;
			createFromData(palResource->data, palResource->size, &insertPalette);
			// insert new palette into target
			insert(&insertPalette, &_palVaryTargetPalette);
			// update palette and set on screen
			palVaryProcess(0, true);
		}
	}
	return kernelPalVaryGetCurrentStep();
}

void GfxPalette::kernelPalVaryChangeTicks(uint16 ticks) {
	_palVaryTicks = ticks;
	if (_palVaryStep - _palVaryStepStop) {
		palVaryRemoveTimer();
		palVaryInstallTimer();
	}
}

void GfxPalette::kernelPalVaryPause(bool pause) {
	if (_palVaryResourceId == -1)
		return;
	// this call is actually counting states, so calling this 3 times with true will require calling it later
	// 3 times with false to actually remove pause
	if (pause) {
		_palVaryPaused++;
	} else {
		if (_palVaryPaused)
			_palVaryPaused--;
	}
}

void GfxPalette::kernelPalVaryDeinit() {
	palVaryRemoveTimer();

	_palVaryResourceId = -1;	// invalidate the target palette
}

void GfxPalette::palVaryCallback(void *refCon) {
	((GfxPalette *)refCon)->palVaryIncreaseSignal();
}

void GfxPalette::palVaryIncreaseSignal() {
	if (!_palVaryPaused)
		_palVarySignal++;
}

// Actually do the pal vary processing
void GfxPalette::palVaryUpdate() {
	if (_palVarySignal) {
		palVaryProcess(_palVarySignal, true);
		_palVarySignal = 0;
	}
}

void GfxPalette::palVaryPrepareForTransition() {
	if (_palVaryResourceId != -1) {
		// Before doing transitions, we have to prepare palette
		palVaryProcess(0, false);
	}
}

// Processes pal vary updates
void GfxPalette::palVaryProcess(int signal, bool setPalette) {
	int16 stepChange = signal * _palVaryDirection;

	_palVaryStep += stepChange;
	if (stepChange > 0) {
		if (_palVaryStep > _palVaryStepStop)
			_palVaryStep = _palVaryStepStop;
	} else {
		if (_palVaryStep < _palVaryStepStop) {
			if (signal)
				_palVaryStep = _palVaryStepStop;
		}
	}

	// We don't need updates anymore, if we reached end-position
	if (_palVaryStep == _palVaryStepStop)
		palVaryRemoveTimer();
	if (_palVaryStep == 0)
		_palVaryResourceId = -1;

	// Calculate inbetween palette
	Sci::Color inbetween;
	int16 color;
	for (int colorNr = 1; colorNr < 255; colorNr++) {
		inbetween.used = _sysPalette.colors[colorNr].used;
		color = _palVaryTargetPalette.colors[colorNr].r - _palVaryOriginPalette.colors[colorNr].r;
		inbetween.r = ((color * _palVaryStep) / 64) + _palVaryOriginPalette.colors[colorNr].r;
		color = _palVaryTargetPalette.colors[colorNr].g - _palVaryOriginPalette.colors[colorNr].g;
		inbetween.g = ((color * _palVaryStep) / 64) + _palVaryOriginPalette.colors[colorNr].g;
		color = _palVaryTargetPalette.colors[colorNr].b - _palVaryOriginPalette.colors[colorNr].b;
		inbetween.b = ((color * _palVaryStep) / 64) + _palVaryOriginPalette.colors[colorNr].b;

		if (memcmp(&inbetween, &_sysPalette.colors[colorNr], sizeof(Sci::Color))) {
			_sysPalette.colors[colorNr] = inbetween;
			_sysPaletteChanged = true;
		}
	}

	if ((_sysPaletteChanged) && (setPalette) && (_screen->_picNotValid == 0)) {
		setOnScreen();
		_sysPaletteChanged = false;
	}
}

static inline uint getMacColorDiff(byte r1, byte g1, byte b1, byte r2, byte g2, byte b2) {
	// Use the difference of the top 4 bits and add together the differences
	return ABS((r2 & 0xf0) - (r1 & 0xf0)) + ABS((g2 & 0xf0) - (g1 & 0xf0)) + ABS((b2 & 0xf0) - (b1 & 0xf0));
}

byte GfxPalette::findMacIconBarColor(byte r, byte g, byte b) {
	// Find the best color for use with the Mac icon bar
	// Check white, then the palette colors, and then black

	// Try white first
	byte found = 0xff;
	uint diff = getMacColorDiff(r, g, b, 0xff, 0xff, 0xff);

	if (diff == 0)
		return found;

	// Go through the main colors of the CLUT
	for (uint16 i = 1; i < 255; i++) {
		if (!colorIsFromMacClut(i))
			continue;

		uint cdiff = getMacColorDiff(r, g, b, _macClut[i * 3], _macClut[i * 3 + 1], _macClut[i * 3 + 2]);

		if (cdiff == 0)
			return i;
		else if (cdiff < diff) {
			found = i;
			diff = cdiff;
		}
	}

	// Also check black here
	if (getMacColorDiff(r, g, b, 0, 0, 0) < diff)
		return 0;

	return found;
}

void GfxPalette::loadMacIconBarPalette() {
	if (!g_sci->hasMacIconBar())
		return;

	Common::SeekableReadStream *clutStream = g_sci->getMacExecutable()->getResource(MKTAG('c','l','u','t'), 150);

	if (!clutStream)
		error("Could not find clut 150 for the Mac icon bar");

	clutStream->readUint32BE(); // seed
	clutStream->readUint16BE(); // flags
	uint16 colorCount = clutStream->readUint16BE() + 1;
	assert(colorCount == 256);

	_macClut = new byte[256 * 3];

	for (uint16 i = 0; i < colorCount; i++) {
		clutStream->readUint16BE();
		_macClut[i * 3    ] = clutStream->readUint16BE() >> 8;
		_macClut[i * 3 + 1] = clutStream->readUint16BE() >> 8;
		_macClut[i * 3 + 2] = clutStream->readUint16BE() >> 8;
	}

	// Adjust bounds on the KQ6 palette
	// We don't use all of it for the icon bar
	if (g_sci->getGameId() == GID_KQ6)
		memset(_macClut + 32 * 3, 0, (256 - 32) * 3);

	// Force black/white
	_macClut[0x00 * 3    ] = 0;
	_macClut[0x00 * 3 + 1] = 0;
	_macClut[0x00 * 3 + 2] = 0;
	_macClut[0xff * 3    ] = 0xff;
	_macClut[0xff * 3 + 1] = 0xff;
	_macClut[0xff * 3 + 2] = 0xff;

	delete clutStream;
}

bool GfxPalette::colorIsFromMacClut(byte index) {
	return index != 0 && _macClut && (_macClut[index * 3] != 0 || _macClut[index * 3 + 1] != 0 || _macClut[index * 3 + 2] != 0);
}

#ifdef ENABLE_SCI32

bool GfxPalette::loadClut(uint16 clutId) {
	// loadClut() will load a color lookup table from a clu file and set
	// the palette found in the file. This is to be used with Phantasmagoria 2.

	unloadClut();

	Common::String filename = Common::String::format("%d.clu", clutId);
	Common::File clut;

	if (!clut.open(filename) || clut.size() != 0x10000 + 236 * 3)
		return false;

	// Read in the lookup table
	// It maps each RGB565 color to a palette index
	_clutTable = new byte[0x10000];
	clut.read(_clutTable, 0x10000);

	Palette pal;
	memset(&pal, 0, sizeof(Palette));

	// Setup 1:1 mapping
	for (int i = 0; i < 256; i++) {
		pal.mapping[i] = i;
	}

	// Now load in the palette
	for (int i = 1; i <= 236; i++) {
		pal.colors[i].used = 1;
		pal.colors[i].r = clut.readByte();
		pal.colors[i].g = clut.readByte();
		pal.colors[i].b = clut.readByte();
	}

	set(&pal, true);
	setOnScreen();
	return true;
}

byte GfxPalette::matchClutColor(uint16 color) {
	// Match a color in RGB565 format to a palette index based on the loaded CLUT
	assert(_clutTable);
	return _clutTable[color];
}

void GfxPalette::unloadClut() {
	// This will only unload the actual table, but not reset any palette
	delete[] _clutTable;
	_clutTable = 0;
}

#endif

} // End of namespace Sci
