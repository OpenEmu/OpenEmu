
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

#include "common/system.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "graphics/palette.h"
#include "graphics/primitives.h"
#include "engines/util.h"

#include "parallaction/input.h"
#include "parallaction/parallaction.h"


namespace Parallaction {

// this is the size of the receiving buffer for unpacked frames,
// since BRA uses some insanely big animations (the largest is
// part0/ani/dino.ani).
#define MAXIMUM_UNPACKED_BITMAP_SIZE	641*401


#define	LABEL_TRANSPARENT_COLOR 0xFF

void halfbritePixel(int x, int y, int color, void *data) {
	Graphics::Surface *surf = (Graphics::Surface *)data;
	byte *pixel = (byte *)surf->getBasePtr(x, y);
	*pixel &= ~0x20;
}

void drawCircleLine(int xCenter, int yCenter, int x, int y, int color, void (*plotProc)(int, int, int, void *), void *data){
	Graphics::drawLine(xCenter + x, yCenter + y, xCenter - x, yCenter + y, color, plotProc, data);
	Graphics::drawLine(xCenter + x, yCenter - y, xCenter - x, yCenter - y, color, plotProc, data);
	Graphics::drawLine(xCenter + y, yCenter + x, xCenter - y, yCenter + x, color, plotProc, data);
	Graphics::drawLine(xCenter + y, yCenter - x, xCenter - y, yCenter - x, color, plotProc, data);
}

void drawCircle(int xCenter, int yCenter, int radius, int color, void (*plotProc)(int, int, int, void *), void *data) {
	int x = 0;
	int y = radius;
	int p = 1 - radius;

	/* Plot first set of points */
	drawCircleLine(xCenter, yCenter, x, y, color, plotProc, data);

	while (x < y) {
		x++;
		if (p < 0)
			p += 2*x + 1;
		else {
			y--;
			p += 2 * (x-y) + 1;
		}
		drawCircleLine(xCenter, yCenter, x, y, color, plotProc, data);
	}
}




Palette::Palette() {

	int gameType = g_vm->getGameType();

	if (gameType == GType_Nippon) {
		_colors = 32;
		_hb = (g_vm->getPlatform() == Common::kPlatformAmiga);
	} else
	if (gameType == GType_BRA) {
		_colors = 256;
		_hb = false;
	} else
		error("can't create palette for id = '%i'", gameType);

	_size = _colors * 3;

	makeBlack();
}

Palette::Palette(const Palette &pal) {
	clone(pal);
}

void Palette::clone(const Palette &pal) {
	_colors = pal._colors;
	_hb = pal._hb;
	_size = pal._size;
	memcpy(_data, pal._data, _size);
}


void Palette::makeBlack() {
	memset(_data, 0, _size);
}

void Palette::setEntry(uint index, int red, int green, int blue) {
	assert(index < _colors);

	if (red >= 0)
		_data[index*3] = red & 0xFF;

	if (green >= 0)
		_data[index*3+1] = green & 0xFF;

	if (blue >= 0)
		_data[index*3+2] = blue & 0xFF;
}

void Palette::getEntry(uint index, int &red, int &green, int &blue) {
	assert(index < _colors);
	red   = _data[index*3];
	green = _data[index*3+1];
	blue  = _data[index*3+2];
}

void Palette::makeGrayscale() {
	byte v;
	for (uint16 i = 0; i < _colors; i++) {
		v = MAX(_data[i*3+1], _data[i*3+2]);
		v = MAX(v, _data[i*3]);
		setEntry(i, v, v, v);
	}
}

void Palette::fadeTo(const Palette& target, uint step) {

	if (step == 0)
		return;

	for (uint16 i = 0; i < _size; i++) {
		if (_data[i] == target._data[i]) continue;

		if (_data[i] < target._data[i])
			_data[i] = CLIP(_data[i] + (int)step, (int)0, (int)target._data[i]);
		else
			_data[i] = CLIP(_data[i] - (int)step, (int)target._data[i], (int)255);
	}

	return;
}

uint Palette::fillRGB(byte *rgb) {

	byte r, g, b;
	byte *hbPal = rgb + _colors * 3;

	for (uint32 i = 0; i < _colors; i++) {
		r = (_data[i*3]   << 2) | (_data[i*3]   >> 4);
		g = (_data[i*3+1] << 2) | (_data[i*3+1] >> 4);
		b = (_data[i*3+2] << 2) | (_data[i*3+2] >> 4);

		rgb[i*3]   = r;
		rgb[i*3+1] = g;
		rgb[i*3+2] = b;

		if (_hb) {
			hbPal[i*3]   = r >> 1;
			hbPal[i*3+1] = g >> 1;
			hbPal[i*3+2] = b >> 1;
		}

	}

	return ((_hb) ? 2 : 1) * _colors;
}

void Palette::rotate(uint first, uint last, bool forward) {

	byte tmp[3];

	if (forward) {					// forward

		tmp[0] = _data[first * 3];
		tmp[1] = _data[first * 3 + 1];
		tmp[2] = _data[first * 3 + 2];

		memmove(_data+first*3, _data+(first+1)*3, (last - first)*3);

		_data[last * 3]	 = tmp[0];
		_data[last * 3 + 1] = tmp[1];
		_data[last * 3 + 2] = tmp[2];

	} else {											// backward

		tmp[0] = _data[last * 3];
		tmp[1] = _data[last * 3 + 1];
		tmp[2] = _data[last * 3 + 2];

		memmove(_data+(first+1)*3, _data+first*3, (last - first)*3);

		_data[first * 3]	  = tmp[0];
		_data[first * 3 + 1] = tmp[1];
		_data[first * 3 + 2] = tmp[2];

	}

}



void Gfx::setPalette(Palette pal) {
	byte sysPal[256*3];

	uint n = pal.fillRGB(sysPal);
	_vm->_system->getPaletteManager()->setPalette(sysPal, 0, n);
}

void Gfx::setBlackPalette() {
	Palette pal;
	setPalette(pal);
}




void Gfx::animatePalette() {

	// avoid forcing setPalette when not needed
	bool done = false;

	PaletteFxRange *range;
	for (uint16 i = 0; i < 4; i++) {
		range = &_backgroundInfo->ranges[i];

		if ((range->_flags & 1) == 0) continue;		// animated palette
		range->_timer += range->_step * 2;	// update timer

		if (range->_timer < 0x4000) continue;		// check timeout

		range->_timer = 0;							// reset timer

		_palette.rotate(range->_first, range->_last, (range->_flags & 2) != 0);

		done = true;
	}

	if (done) {
		setPalette(_palette);
	}

	return;
}





void Gfx::setHalfbriteMode(bool enable) {
	if (_vm->getPlatform() != Common::kPlatformAmiga) return;
	if (enable == _halfbrite) return;

	_halfbrite = !_halfbrite;
}

#define HALFBRITE_CIRCLE_RADIUS		48
void Gfx::setProjectorPos(int x, int y) {
	_hbCircleRadius = HALFBRITE_CIRCLE_RADIUS;
	_hbCirclePos.x = x + _hbCircleRadius;
	_hbCirclePos.y = y + _hbCircleRadius;
}

void Gfx::setProjectorProgram(int16 *data) {
	if (_nextProjectorPos == 0) {
		_nextProjectorPos = data;
	}
}

void Gfx::drawInventory() {
/*
	if ((_engineFlags & kEngineInventory) == 0) {
		return;
	}
*/
	if (_vm->_input->_inputMode != Input::kInputModeInventory) {
		return;
	}

	Common::Rect r;
	_vm->_inventoryRenderer->getRect(r);
	byte *data = _vm->_inventoryRenderer->getData();

	copyRectToScreen(data, r.width(), r.left, r.top, r.width(), r.height());
}

void Gfx::drawList(Graphics::Surface &surface, GfxObjArray &list) {
	if (list.size() == 0) {
		return;
	}

	for (uint i = 0; i < list.size(); i++) {
		drawGfxObject(list[i], surface);
	}
}

void Gfx::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	if (_doubleBuffering) {
		if (_overlayMode) {
			x += _scrollPosX;
			y += _scrollPosY;
		}

		byte *dst = (byte *)_backBuffer.getBasePtr(x, y);
		for (int i = 0; i < h; i++) {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _backBuffer.pitch;
		}
	} else {
		_vm->_system->copyRectToScreen(buf, pitch, x, y, w, h);
	}
}

void Gfx::clearScreen() {
	if (_doubleBuffering) {
		if (_backBuffer.pixels) {
			Common::Rect r(_backBuffer.w, _backBuffer.h);
			_backBuffer.fillRect(r, 0);
		}
	} else {
		_vm->_system->fillScreen(0);
	}
}

Graphics::Surface *Gfx::lockScreen() {
	if (_doubleBuffering) {
		return &_backBuffer;
	}
	return _vm->_system->lockScreen();
}

void Gfx::unlockScreen() {
	if (_doubleBuffering) {
		return;
	}
	_vm->_system->unlockScreen();
}

void Gfx::updateScreenIntern() {
	if (_doubleBuffering) {
		byte *data = (byte *)_backBuffer.getBasePtr(_scrollPosX, _scrollPosY);
		_vm->_system->copyRectToScreen(data, _backBuffer.pitch, 0, 0, _vm->_screenWidth, _vm->_screenHeight);
	}

	_vm->_system->updateScreen();
}

void Gfx::getScrollPos(Common::Point &p) {
	p.x = _scrollPosX;
	p.y = _scrollPosY;
}

void Gfx::setScrollPosX(int scrollX) {
	_scrollPosX = CLIP(scrollX, _minScrollX, _maxScrollX);
}

void Gfx::setScrollPosY(int scrollY) {
	_scrollPosY = CLIP(scrollY, _minScrollY, _maxScrollY);
}

void Gfx::initiateScroll(int deltaX, int deltaY) {
	if (deltaX != 0) {
		_requestedHScrollDir = deltaX > 0 ? 1 : -1;
		deltaX *= _requestedHScrollDir;
		_requestedHScrollSteps = ((deltaX+31)/32) / _requestedHScrollDir;
	}

	if (deltaY != 0) {
		_requestedVScrollDir = deltaY > 0 ? 1 : -1;
		deltaY *= _requestedVScrollDir;
		_requestedVScrollSteps = ((deltaY+7)/8) / _requestedVScrollDir;
	}
}

void Gfx::scroll() {
	int32 x = _scrollPosX, y = _scrollPosY;

	if (_requestedHScrollSteps) {
		x += 32*_requestedHScrollDir;	// scroll 32 pixels at a time
		_requestedHScrollSteps--;
	}

	if (_requestedVScrollSteps) {
		y += 8*_requestedVScrollDir;	// scroll 8 pixel at a time
		_requestedVScrollSteps--;
	}

	setScrollPosX(x);
	setScrollPosY(y);
}

void Gfx::beginFrame() {
	resetSceneDrawList();
	scroll();
}

void Gfx::updateScreen() {

	// the scene is calculated in game coordinates, so no translation
	// is needed
	_overlayMode = false;

	bool skipBackground = (_backgroundInfo->bg.pixels == 0);	// don't render frame if background is missing

	if (!skipBackground) {
		// background may not cover the whole screen, so adjust bulk update size
		uint w = _backgroundInfo->width;
		uint h = _backgroundInfo->height;
		byte *backgroundData = (byte *)_backgroundInfo->bg.getBasePtr(0, 0);
		uint16 backgroundPitch = _backgroundInfo->bg.pitch;
		copyRectToScreen(backgroundData, backgroundPitch, _backgroundInfo->_x, _backgroundInfo->_y, w, h);
	}

	sortScene();
	Graphics::Surface *surf = lockScreen();
		// draws animations frames and other game items
		drawList(*surf, _sceneObjects);

		// special effects
		applyHalfbriteEffect_NS(*surf);

		// draws inventory, labels and dialogue items
		drawOverlay(*surf);
	unlockScreen();

	updateScreenIntern();
}

void Gfx::applyHalfbriteEffect_NS(Graphics::Surface &surf) {
	if (!_halfbrite) {
		return;
	}

	byte *buf = (byte *)surf.pixels;
	for (int i = 0; i < surf.w*surf.h; i++) {
		*buf++ |= 0x20;
	}

	if (_nextProjectorPos) {
		int16 x = *_nextProjectorPos;
		int16 y = *(_nextProjectorPos + 1);
		if (x != -1 && y != -1) {
			_nextProjectorPos += 2;
			setProjectorPos(x, y);
		}
	}
	if (_hbCircleRadius > 0) {
		drawCircle(_hbCirclePos.x, _hbCirclePos.y, _hbCircleRadius, 0, &halfbritePixel, &surf);
	}
}

void Gfx::drawOverlay(Graphics::Surface &surf) {
	// the following items are handled in screen coordinates, so they need translation before
	// being drawn
	_overlayMode = true;

	drawInventory();

	updateFloatingLabel();

	drawList(surf, _items);
	drawList(surf, _balloons);
	drawList(surf, _labels);
}

//
//	graphic primitives
//


void Gfx::patchBackground(Graphics::Surface &surf, int16 x, int16 y, bool mask) {

	Common::Rect r(surf.w, surf.h);
	r.moveTo(x, y);

	uint16 z = (mask) ? _backgroundInfo->getMaskLayer(y) : LAYER_FOREGROUND;
	blt(r, (byte *)surf.pixels, &_backgroundInfo->bg, z, 100, 0);
}

void Gfx::fillBackground(const Common::Rect& r, byte color) {
	_backgroundInfo->bg.fillRect(r, color);
}

void Gfx::invertBackground(const Common::Rect& r) {

	byte *d = (byte *)_backgroundInfo->bg.getBasePtr(r.left, r.top);

	for (int i = 0; i < r.height(); i++) {
		for (int j = 0; j < r.width(); j++) {
			*d ^= 0x1F;
			d++;
		}

		d += (_backgroundInfo->bg.pitch - r.width());
	}

}





void setupLabelSurface(Graphics::Surface &surf, uint w, uint h) {
	surf.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	surf.fillRect(Common::Rect(w,h), LABEL_TRANSPARENT_COLOR);
}

GfxObj *Gfx::renderFloatingLabel(Font *font, char *text) {

	Graphics::Surface *cnv = new Graphics::Surface;

	uint w, h;
	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		w = font->getStringWidth(text) + 16;
		h = font->height() + 2;

		setupLabelSurface(*cnv, w, h);

		font->setColor((_gameType == GType_BRA) ? 0 : 7);
		font->drawString((byte *)cnv->pixels + 1, cnv->w, text);
		font->drawString((byte *)cnv->pixels + 1 + cnv->w * 2, cnv->w, text);
		font->drawString((byte *)cnv->pixels + cnv->w, cnv->w, text);
		font->drawString((byte *)cnv->pixels + 2 + cnv->w, cnv->w, text);
		font->setColor((_gameType == GType_BRA) ? 11 : 1);
		font->drawString((byte *)cnv->pixels + 1 + cnv->w, cnv->w, text);
	} else {
		w = font->getStringWidth(text);
		h = font->height();

		setupLabelSurface(*cnv, w, h);

		drawText(font, cnv, 0, 0, text, 0);
	}

	GfxObj *obj = new GfxObj(kGfxObjTypeLabel, new SurfaceToFrames(cnv), "floatingLabel");
	obj->transparentKey = LABEL_TRANSPARENT_COLOR;
	obj->layer = LAYER_FOREGROUND;

	return obj;
}

void Gfx::showFloatingLabel(GfxObj *label) {
	hideFloatingLabel();

	if (label) {
		label->x = -1000;
		label->y = -1000;
		label->setFlags(kGfxObjVisible);

		_floatingLabel = label;
		_labels.push_back(label);
	}
}

void Gfx::hideFloatingLabel() {
	if (_floatingLabel != 0) {
		_floatingLabel->clearFlags(kGfxObjVisible);
	}
	_floatingLabel = 0;
}


void Gfx::updateFloatingLabel() {
	if (_floatingLabel == 0) {
		return;
	}

	struct FloatingLabelTraits {
		Common::Point _offsetWithItem;
		Common::Point _offsetWithoutItem;
		int	_minX;
		int _minY;
		int	_maxX;
		int _maxY;
	} *traits;

	Common::Rect r;
	_floatingLabel->getRect(0, r);

	FloatingLabelTraits traits_NS = {
		Common::Point(16 - r.width()/2, 34),
		Common::Point(8 - r.width()/2, 21),
		0, 0, _vm->_screenWidth - r.width(), 190
	};

	// FIXME: _maxY for BRA is not constant (390), but depends on _vm->_subtitleY
	FloatingLabelTraits traits_BR = {
		Common::Point(34 - r.width()/2, 70),
		Common::Point(16 - r.width()/2, 37),
		0, 0, _vm->_screenWidth - r.width(), 390
	};

	if (_gameType == GType_Nippon) {
		traits = &traits_NS;
	} else {
		traits = &traits_BR;
	}

	Common::Point	cursor;
	_vm->_input->getCursorPos(cursor);
	Common::Point offset = (_vm->_input->_activeItem._id) ? traits->_offsetWithItem : traits->_offsetWithoutItem;

	_floatingLabel->x = CLIP(cursor.x + offset.x, traits->_minX, traits->_maxX);
	_floatingLabel->y = CLIP(cursor.y + offset.y, traits->_minY, traits->_maxY);
}




GfxObj *Gfx::createLabel(Font *font, const char *text, byte color) {
	Graphics::Surface *cnv = new Graphics::Surface;

	uint w, h;

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		w = font->getStringWidth(text) + 2;
		h = font->height() + 2;

		setupLabelSurface(*cnv, w, h);

		drawText(font, cnv, 0, 2, text, 0);
		drawText(font, cnv, 2, 0, text, color);
	} else {
		w = font->getStringWidth(text);
		h = font->height();

		setupLabelSurface(*cnv, w, h);

		drawText(font, cnv, 0, 0, text, color);
	}

	GfxObj *obj = new GfxObj(kGfxObjTypeLabel, new SurfaceToFrames(cnv), "label");
	obj->transparentKey = LABEL_TRANSPARENT_COLOR;
	obj->layer = LAYER_FOREGROUND;

	return obj;
}

void Gfx::showLabel(GfxObj *label, int16 x, int16 y) {
	if (!label) {
		return;
	}

	label->setFlags(kGfxObjVisible);

	Common::Rect r;
	label->getRect(0, r);

	if (x == CENTER_LABEL_HORIZONTAL) {
		x = CLIP<int16>((_backgroundInfo->width - r.width()) / 2, 0, _backgroundInfo->width/2);
	}

	if (y == CENTER_LABEL_VERTICAL) {
		y = CLIP<int16>((_vm->_screenHeight - r.height()) / 2, 0, _vm->_screenHeight/2);
	}

	label->x = x;
	label->y = y;

	_labels.push_back(label);
}

void Gfx::hideLabel(GfxObj *label) {
	if (label) {
		label->clearFlags(kGfxObjVisible);
		unregisterLabel(label);
	}
}

void Gfx::freeLabels() {
	_labels.clear();
	_floatingLabel = 0;
}

void Gfx::unregisterLabel(GfxObj *label) {
	for (uint i = 0; i < _labels.size(); i++) {
		if (_labels[i] == label) {
			_labels.remove_at(i);
			break;
		}
	}
}


void Gfx::copyRect(const Common::Rect &r, Graphics::Surface &src, Graphics::Surface &dst) {

	byte *s = (byte *)src.getBasePtr(r.left, r.top);
	byte *d = (byte *)dst.getBasePtr(0, 0);

	for (uint16 i = 0; i < r.height(); i++) {
		memcpy(d, s, r.width());

		s += src.pitch;
		d += dst.pitch;
	}

	return;
}

void Gfx::grabBackground(const Common::Rect& r, Graphics::Surface &dst) {
	copyRect(r, _backgroundInfo->bg, dst);
}


Gfx::Gfx(Parallaction* vm) :
	_vm(vm), _disk(vm->_disk), _backgroundInfo(0),
	_scrollPosX(0), _scrollPosY(0),_minScrollX(0), _maxScrollX(0),
	_minScrollY(0), _maxScrollY(0),
	_requestedHScrollSteps(0), _requestedVScrollSteps(0),
	_requestedHScrollDir(0), _requestedVScrollDir(0) {

	_gameType = _vm->getGameType();
	_doubleBuffering = _gameType != GType_Nippon;

	initGraphics(_vm->_screenWidth, _vm->_screenHeight, _gameType == GType_BRA);

	setPalette(_palette);

	_floatingLabel = 0;

	_backgroundInfo = 0;

	_halfbrite = false;
	_nextProjectorPos = 0;
	_hbCircleRadius = 0;

	_unpackedBitmap = new byte[MAXIMUM_UNPACKED_BITMAP_SIZE];
	assert(_unpackedBitmap);

	if ((_gameType == GType_BRA) && (_vm->getPlatform() == Common::kPlatformPC)) {
	// this loads the backup palette needed by the PC version of BRA (see setBackground()).
		BackgroundInfo	paletteInfo;
		_disk->loadSlide(paletteInfo, "pointer");
		_backupPal.clone(paletteInfo.palette);
	}

	resetSceneDrawList();

	return;
}

Gfx::~Gfx() {

	_backBuffer.free();

	delete _backgroundInfo;

	freeLabels();

	delete[] _unpackedBitmap;

	return;
}



int Gfx::setItem(GfxObj* frames, uint16 x, uint16 y, byte transparentColor) {
	int id = _items.size();

	frames->x = x;
	frames->y = y;
	frames->transparentKey = transparentColor;
	frames->layer = LAYER_FOREGROUND;
	frames->setFlags(kGfxObjVisible);

	_items.insert_at(id, frames);

	setItemFrame(id, 0);

	return id;
}

void Gfx::setItemFrame(uint item, uint16 f) {
	_items[item]->frame = f;
}


GfxObj* Gfx::registerBalloon(Frames *frames, const char *text) {

	GfxObj *obj = new GfxObj(kGfxObjTypeBalloon, frames, text);

	obj->layer = LAYER_FOREGROUND;
	obj->frame = 0;
	obj->setFlags(kGfxObjVisible);

	_balloons.push_back(obj);

	return obj;
}

void Gfx::freeDialogueObjects() {
	_items.clear();

	_vm->_balloonMan->reset();

	for (uint i = 0; i < _balloons.size(); i++) {
		delete _balloons[i];
	}
	_balloons.clear();
}

void Gfx::setBackground(uint type, BackgroundInfo *info) {
	if (!info) {
		warning("Gfx::setBackground() called with an null BackgroundInfo");
		return;
	}

	_hbCircleRadius = 0;
	_nextProjectorPos = 0;

	delete _backgroundInfo;
	_backgroundInfo = info;

	if (type == kBackgroundLocation) {
		// The PC version of BRA needs the entries 20-31 of the palette to be constant, but
		// the background resource files are screwed up. The right colors come from an unused
		// bitmap (pointer.bmp). Nothing is known about the Amiga version so far.
		if ((_gameType == GType_BRA) && (_vm->getPlatform() == Common::kPlatformPC)) {
			int r, g, b;
			for (uint i = 16; i < 32; i++) {
				_backupPal.getEntry(i, r, g, b);
				_backgroundInfo->palette.setEntry(i, r, g, b);
			}
		}

		setPalette(_backgroundInfo->palette);
		_palette.clone(_backgroundInfo->palette);
	} else {
		for (uint i = 0; i < 6; i++)
			_backgroundInfo->ranges[i]._flags = 0;	// disable palette cycling for slides
		setPalette(_backgroundInfo->palette);
	}

	_backgroundInfo->finalizeMask();
	_backgroundInfo->finalizePath();

	if (_gameType == GType_BRA) {
		int width = CLIP(info->width, (int)_vm->_screenWidth, info->width);
		int height = CLIP(info->height, (int)_vm->_screenHeight, info->height);

		if (width != _backBuffer.w || height != _backBuffer.h) {
			_backBuffer.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		}
	}

	_minScrollX = 0;
	_maxScrollX = MAX<int>(0, _backgroundInfo->width - _vm->_screenWidth);
	_minScrollY = 0;
	_maxScrollY = MAX<int>(0, _backgroundInfo->height - _vm->_screenHeight);
}


BackgroundInfo::BackgroundInfo() : _x(0), _y(0), width(0), height(0), _mask(0), _path(0) {
	layers[0] = layers[1] = layers[2] = layers[3] = 0;
	memset(ranges, 0, sizeof(ranges));
}

BackgroundInfo::~BackgroundInfo() {
	bg.free();
	clearMaskData();
	clearPathData();
}

bool BackgroundInfo::hasMask() {
	return _mask != 0;
}

void BackgroundInfo::clearMaskData() {
	// free mask data
	MaskPatches::iterator it = _maskPatches.begin();
	for ( ; it != _maskPatches.end(); ++it) {
		delete *it;
	}
	_maskPatches.clear();
	delete _mask;
	_mask = 0;
	_maskBackup.free();
}

void BackgroundInfo::finalizeMask() {
	if (_mask) {
		if (_maskPatches.size() > 0) {
			// since no more patches can be added after finalization,
			// avoid creating the backup if there is none
			_maskBackup.clone(*_mask);
		}
	} else {
		clearMaskData();
	}
}

uint BackgroundInfo::addMaskPatch(MaskBuffer *patch) {
	uint id = _maskPatches.size();
	_maskPatches.push_back(patch);
	return id;
}

void BackgroundInfo::toggleMaskPatch(uint id, int x, int y, bool apply) {
	if (!hasMask()) {
		return;
	}
	if (id >= _maskPatches.size()) {
		return;
	}
	MaskBuffer *patch = _maskPatches[id];
	if (apply) {
		_mask->bltOr(x, y, *patch, 0, 0, patch->w, patch->h);
	} else {
		_mask->bltCopy(x, y, _maskBackup, x, y, patch->w, patch->h);
	}
}

uint16 BackgroundInfo::getMaskLayer(uint16 z) const {
	for (uint16 i = 0; i < 3; i++) {
		if (layers[i+1] > z) return i;
	}
	return LAYER_FOREGROUND;
}

void BackgroundInfo::setPaletteRange(int index, const PaletteFxRange& range) {
	assert(index < 6);
	memcpy(&ranges[index], &range, sizeof(PaletteFxRange));
}

bool BackgroundInfo::hasPath() {
	return _path != 0;
}

void BackgroundInfo::clearPathData() {
	// free mask data
	PathPatches::iterator it = _pathPatches.begin();
	for ( ; it != _pathPatches.end(); ++it) {
		delete *it;
	}
	_pathPatches.clear();
	delete _path;
	_path = 0;
	_pathBackup.free();
}

void BackgroundInfo::finalizePath() {
	if (_path) {
		if (_pathPatches.size() > 0) {
			// since no more patches can be added after finalization,
			// avoid creating the backup if there is none
			_pathBackup.clone(*_path);
		}
	} else {
		clearPathData();
	}
}

uint BackgroundInfo::addPathPatch(PathBuffer *patch) {
	uint id = _pathPatches.size();
	_pathPatches.push_back(patch);
	return id;
}

void BackgroundInfo::togglePathPatch(uint id, int x, int y, bool apply) {
	if (!hasPath()) {
		return;
	}
	if (id >= _pathPatches.size()) {
		return;
	}
	PathBuffer *patch = _pathPatches[id];
	if (apply) {
		_path->bltCopy(x, y, *patch, 0, 0, patch->w, patch->h);
	} else {
		_path->bltCopy(x, y, _pathBackup, x, y, patch->w, patch->h);
	}
}

MaskBuffer::MaskBuffer() : w(0), internalWidth(0), h(0), size(0), data(0), bigEndian(true) {
}

MaskBuffer::~MaskBuffer() {
	free();
}

byte* MaskBuffer::getPtr(uint16 x, uint16 y) const {
	return data + (x >> 2) + y * internalWidth;
}

void MaskBuffer::clone(const MaskBuffer &buf) {
	if (!buf.data)
		return;

	create(buf.w, buf.h);
	bigEndian = buf.bigEndian;
	memcpy(data, buf.data, size);
}

void MaskBuffer::create(uint16 width, uint16 height) {
	free();

	w = width;
	internalWidth = w >> 2;
	h = height;
	size = (internalWidth * h);
	data = (byte *)calloc(size, 1);
}

void MaskBuffer::free() {
	::free(data);
	data = 0;
	w = 0;
	h = 0;
	internalWidth = 0;
	size = 0;
}

byte MaskBuffer::getValue(uint16 x, uint16 y) const {
	byte m = data[(x >> 2) + y * internalWidth];
	uint n;
	if (bigEndian) {
		n = (x & 3) << 1;
	} else {
		n = (3 - (x & 3)) << 1;
	}
	return (m >> n) & 3;
}

void MaskBuffer::bltOr(uint16 dx, uint16 dy, const MaskBuffer &src, uint16 sx, uint16 sy, uint width, uint height) {
	assert((width <= w) && (width <= src.w) && (height <= h) && (height <= src.h));

	byte *s = src.getPtr(sx, sy);
	byte *d = getPtr(dx, dy);

	// this code assumes buffers are aligned on 4-pixels boundaries, as the original does
	uint16 linewidth = width >> 2;
	for (uint16 i = 0; i < height; i++) {
		for (uint16 j = 0; j < linewidth; j++) {
			*d++ |= *s++;
		}
		d += internalWidth - linewidth;
		s += src.internalWidth - linewidth;
	}
}

void MaskBuffer::bltCopy(uint16 dx, uint16 dy, const MaskBuffer &src, uint16 sx, uint16 sy, uint width, uint height) {
	assert((width <= w) && (width <= src.w) && (height <= h) && (height <= src.h));

	byte *s = src.getPtr(sx, sy);
	byte *d = getPtr(dx, dy);

	// this code assumes buffers are aligned on 4-pixels boundaries, as the original does
	for (uint16 i = 0; i < height; i++) {
		memcpy(d, s, (width >> 2));
		d += internalWidth;
		s += src.internalWidth;
	}
}



PathBuffer::PathBuffer() : w(0), internalWidth(0), h(0), size(0), data(0), bigEndian(true) {
}

PathBuffer::~PathBuffer() {
	free();
}

void PathBuffer::clone(const PathBuffer &buf) {
	if (!buf.data)
		return;

	create(buf.w, buf.h);
	bigEndian = buf.bigEndian;
	memcpy(data, buf.data, size);
}

void PathBuffer::create(uint16 width, uint16 height) {
	free();

	w = width;
	internalWidth = w >> 3;
	h = height;
	size = (internalWidth * h);
	data = (byte *)calloc(size, 1);
}

void PathBuffer::free() {
	::free(data);
	data = 0;
	w = 0;
	h = 0;
	internalWidth = 0;
	size = 0;
}

byte PathBuffer::getValue(uint16 x, uint16 y) const {
	byte m = 0;
	if (data) {
		uint index = (x >> 3) + y * internalWidth;
		if (index < size)
			m = data[index];
		else
			warning("PathBuffer::getValue(x: %d, y: %d) outside of data buffer of size %d", x, y, size);
	} else
		warning("PathBuffer::getValue() attempted to use NULL data buffer");
	uint bit = bigEndian ? (x & 7) : (7 - (x & 7));
	return ((1 << bit) & m) >> bit;
}

byte* PathBuffer::getPtr(uint16 x, uint16 y) const {
	return data + (x >> 3) + y * internalWidth;
}

void PathBuffer::bltCopy(uint16 dx, uint16 dy, const PathBuffer &src, uint16 sx, uint16 sy, uint width, uint height) {
	assert((width <= w) && (width <= src.w) && (height <= h) && (height <= src.h));

	byte *s = src.getPtr(sx, sy);
	byte *d = getPtr(dx, dy);

	// this code assumes buffers are aligned on 4-pixels boundaries, as the original does
	for (uint16 i = 0; i < height; i++) {
		memcpy(d, s, (width >> 3));
		d += internalWidth;
		s += src.internalWidth;
	}
}

} // namespace Parallaction
