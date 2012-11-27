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

#include "cine/cine.h"
#include "cine/bg.h"
#include "cine/bg_list.h"
#include "cine/various.h"
#include "cine/pal.h"

#include "common/endian.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"

namespace Cine {

byte *collisionPage;
FWRenderer *renderer = NULL;

// Constants related to kLowPalFormat
#define kLowPalBytesPerColor 2
#define kLowPalNumColors 16
#define kLowPalNumBytes ((kLowPalNumColors) * (kLowPalBytesPerColor))

/** Low resolution (9-bit) color format used in Cine's 16-color modes. */
#define kLowPalFormat Graphics::PixelFormat(kLowPalBytesPerColor, 3, 3, 3, 0, 8, 4, 0, 0)


// Constants related to kHighPalFormat
#define kHighPalBytesPerColor 3
#define kHighPalNumColors 256
#define kHighPalNumBytes ((kHighPalNumColors) * (kHighPalBytesPerColor))

/** High resolution (24-bit) color format used in Cine's 256-color modes. */
#define kHighPalFormat Graphics::PixelFormat(kHighPalBytesPerColor, 8, 8, 8, 0, 0, 8, 16, 0)

static const byte mouseCursorNormal[] = {
	0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70, 0x00,
	0x78, 0x00, 0x7C, 0x00, 0x7E, 0x00, 0x7F, 0x00,
	0x7F, 0x80, 0x7C, 0x00, 0x6C, 0x00, 0x46, 0x00,
	0x06, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
	0xC0, 0x00, 0xE0, 0x00, 0xF0, 0x00, 0xF8, 0x00,
	0xFC, 0x00, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x80,
	0xFF, 0xC0, 0xFF, 0xC0, 0xFE, 0x00, 0xFF, 0x00,
	0xCF, 0x00, 0x07, 0x80, 0x07, 0x80, 0x03, 0x80
};

static const byte mouseCursorDisk[] = {
	0x7F, 0xFC, 0x9F, 0x12, 0x9F, 0x12, 0x9F, 0x12,
	0x9F, 0x12, 0x9F, 0xE2, 0x80, 0x02, 0x9F, 0xF2,
	0xA0, 0x0A, 0xA0, 0x0A, 0xA0, 0x0A, 0xA0, 0x0A,
	0xA0, 0x0A, 0xA0, 0x0A, 0x7F, 0xFC, 0x00, 0x00,
	0x7F, 0xFC, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0x7F, 0xFC, 0x00, 0x00
};

static const byte mouseCursorCross[] = {
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x7C, 0x7C,
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80,
	0x03, 0x80, 0x03, 0x80, 0xFF, 0xFE, 0xFE, 0xFE,
	0xFF, 0xFE, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80,
	0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x00, 0x00
};

static const struct MouseCursor {
	int hotspotX;
	int hotspotY;
	const byte *bitmap;
} mouseCursors[] = {
	{ 1, 1, mouseCursorNormal },
	{ 0, 0, mouseCursorDisk },
	{ 7, 7, mouseCursorCross }
};

static const byte cursorPalette[] = {
	0, 0, 0, 0xff,
	0xff, 0xff, 0xff, 0xff
};

/**
 * Initialize renderer
 */
FWRenderer::FWRenderer() : _background(NULL), _backupPal(), _cmd(""),
	_cmdY(0), _messageBg(0), _backBuffer(new byte[_screenSize]),
	_activePal(), _changePal(0), _showCollisionPage(false) {

	assert(_backBuffer);

	memset(_backBuffer, 0, _screenSize);
	memset(_bgName, 0, sizeof(_bgName));
}


/**
 * Destroy renderer
 */
FWRenderer::~FWRenderer() {
	delete[] _background;
	delete[] _backBuffer;

	clearMenuStack();
}

bool FWRenderer::initialize() {
	_activePal = Palette(kLowPalFormat, kLowPalNumColors);
	return true;
}


/**
 * Reset renderer state
 */
void FWRenderer::clear() {
	delete[] _background;

	_background = NULL;
	_backupPal.clear();
	_activePal.clear();

	memset(_backBuffer, 0, _screenSize);

	_cmd = "";
	_cmdY = 0;
	_messageBg = 0;
	_changePal = 0;
	_showCollisionPage = false;
}

/**
 * Draw 1bpp sprite using selected color
 * @param obj Object info
 * @param fillColor Sprite color
 */
void FWRenderer::fillSprite(const ObjectStruct &obj, uint8 color) {
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	gfxFillSprite(data, width, height, _backBuffer, x, y, color);
}

/**
 * Draw 1bpp sprite using selected color on background
 * @param obj Object info
 * @param fillColor Sprite color
 */
void FWRenderer::incrustMask(const BGIncrust &incrust, uint8 color) {
	const ObjectStruct &obj = g_cine->_objectTable[incrust.objIdx];
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	gfxFillSprite(data, width, height, _background, x, y, color);
}

/**
 * Draw color sprite using with external mask
 * @param obj Object info
 * @param mask External mask
 */
void FWRenderer::drawMaskedSprite(const ObjectStruct &obj, const byte *mask) {
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	assert(mask);

	drawSpriteRaw(data, mask, width, height, _backBuffer, x, y);
}

/**
 * Draw color sprite
 * @param obj Object info
 */
void FWRenderer::drawSprite(const ObjectStruct &obj) {
	const byte *mask = g_cine->_animDataTable[obj.frame].mask();
	drawMaskedSprite(obj, mask);
}

/**
 * Draw color sprite on background
 * @param obj Object info
 */
void FWRenderer::incrustSprite(const BGIncrust &incrust) {
	const ObjectStruct &obj = g_cine->_objectTable[incrust.objIdx];

	const byte *data = g_cine->_animDataTable[obj.frame].data();
	const byte *mask = g_cine->_animDataTable[obj.frame].mask();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	// There was an assert(mask) here before but it made savegame loading
	// in Future Wars sometimes fail the assertion (e.g. see bug #2055912).
	// Not drawing sprites that have no mask seems to work, but not sure
	// if this is really a correct way to fix this.
	if (mask) {
		drawSpriteRaw(data, mask, width, height, _background, x, y);
	} else { // mask == NULL
		warning("FWRenderer::incrustSprite: Skipping maskless sprite (frame=%d)", obj.frame);
	}
}

/**
 * Draw command box on screen
 */
void FWRenderer::drawCommand() {
	unsigned int i;
	int x = 10, y = _cmdY;

	if (disableSystemMenu == 0) {
		drawPlainBox(x, y, 301, 11, 0);
		drawBorder(x - 1, y - 1, 302, 12, 2);

		x += 2;
		y += 2;

		for (i = 0; i < _cmd.size(); i++) {
			x = drawChar(_cmd[i], x, y);
		}
	}
}

void FWRenderer::drawString(const char *string, byte param) {
	int width;

	width = getStringWidth(string) + 10;
	width = width > 300 ? 300 : width;

	drawMessage(string, (320 - width) / 2, 80, width, 4);

	blit();
}

/**
 * Draw message in a box
 * @param str Message to draw
 * @param x Top left message box corner coordinate
 * @param y Top left message box corner coordinate
 * @param width Message box width
 * @param color Message box background color (Or if negative draws only the text)
 * @note Negative colors are used in Operation Stealth's timed cutscenes
 * (e.g. when first meeting The Movement for the Liberation of Santa Paragua).
 */
void FWRenderer::drawMessage(const char *str, int x, int y, int width, int color) {
	int i, tx, ty, tw;
	int line = 0, words = 0, cw = 0;
	int space = 0, extraSpace = 0;

	const bool isAmiga = (g_cine->getPlatform() == Common::kPlatformAmiga);

	if (color >= 0) {
		if (isAmiga)
			drawTransparentBox(x, y, width, 4);
		else
			drawPlainBox(x, y, width, 4, color);
	}
	tx = x + 4;
	ty = str[0] ? y - 5 : y + 4;
	tw = width - 8;

	for (i = 0; str[i]; i++, line--) {
		// Fit line of text into textbox
		if (!line) {
			while (str[i] == ' ')
				i++;
			line = fitLine(str + i, tw, words, cw);

			if (str[i + line] != '\0' && str[i + line] != 0x7C && words) {
				space = (tw - cw) / words;
				extraSpace = (tw - cw) % words;
			} else {
				space = 5;
				extraSpace = 0;
			}

			ty += 9;
			if (color >= 0) {
				if (isAmiga)
					drawTransparentBox(x, ty, width, 9);
				else
					drawPlainBox(x, ty, width, 9, color);
			}
			tx = x + 4;
		}

		// draw characters
		if (str[i] == ' ') {
			tx += space + extraSpace;

			if (extraSpace) {
				extraSpace = 0;
			}
		} else {
			tx = drawChar(str[i], tx, ty);
		}
	}

	ty += 9;
	if (color >= 0) {
		if (isAmiga)
			drawTransparentBox(x, ty, width, 4);
		else
			drawPlainBox(x, ty, width, 4, color);
		drawDoubleBorder(x, y, width, ty - y + 4, isAmiga ? 18 : 2);
	}
}

/**
 * Draw rectangle on screen
 * @param x Top left corner coordinate
 * @param y Top left corner coordinate
 * @param width Rectangle width (Negative values draw the box horizontally flipped)
 * @param height Rectangle height (Negative values draw the box vertically flipped)
 * @param color Fill color
 * @note An on-screen rectangle's drawn width is always at least one.
 * @note An on-screen rectangle's drawn height is always at least one.
 */
void FWRenderer::drawPlainBox(int x, int y, int width, int height, byte color) {
	// Make width's and height's absolute values at least one
	// which forces this function to always draw something if the
	// drawing position is inside screen bounds. This fixes at least
	// the showing of the oxygen gauge meter in Operation Stealth's
	// first arcade sequence where this function is called with a
	// height of zero.
	if (width == 0) {
		width = 1;
	}
	if (height == 0) {
		height = 1;
	}

	// Handle horizontally flipped boxes
	if (width < 0) {
		width = ABS(width);
		x -= width;
	}

	// Handle vertically flipped boxes
	if (height < 0) {
		height = ABS(height);
		y -= height;
	}

	// Clip the rectangle to screen dimensions
	Common::Rect boxRect(x, y, x + width, y + height);
	Common::Rect screenRect(320, 200);
	boxRect.clip(screenRect);

	byte *dest = _backBuffer + boxRect.top * 320 + boxRect.left;
	for (int i = 0; i < boxRect.height(); i++) {
		memset(dest + i * 320, color, boxRect.width());
	}
}

void FWRenderer::drawTransparentBox(int x, int y, int width, int height) {
	// Handle horizontally flipped boxes
	if (width < 0) {
		width = ABS(width);
		x -= width;
	}

	// Handle vertically flipped boxes
	if (height < 0) {
		height = ABS(height);
		y -= height;
	}

	// Clip the rectangle to screen dimensions
	Common::Rect boxRect(x, y, x + width, y + height);
	Common::Rect screenRect(320, 200);
	boxRect.clip(screenRect);

	byte *dest = _backBuffer + boxRect.top * 320 + boxRect.left;
	const int lineAdd = 320 - boxRect.width();
	for (int i = 0; i < boxRect.height(); ++i) {
		for (int j = 0; j < boxRect.width(); ++j, ++dest) {
			if (*dest < 16)
				*dest += 16;
		}
		dest += lineAdd;
	}
}

/**
 * Draw empty rectangle
 * @param x Top left corner coordinate
 * @param y Top left corner coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color Line color
 */
void FWRenderer::drawBorder(int x, int y, int width, int height, byte color) {
	drawLine(x, y, width, 1, color);
	drawLine(x, y + height, width, 1, color);
	drawLine(x, y, 1, height, color);
	drawLine(x + width, y, 1, height + 1, color);
}

/**
 * Draw empty 2 color rectangle (inner line color is black)
 * @param x Top left corner coordinate
 * @param y Top left corner coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color Outter line color
 */
void FWRenderer::drawDoubleBorder(int x, int y, int width, int height, byte color) {
	drawBorder(x + 1, y + 1, width - 2, height - 2, 0);
	drawBorder(x, y, width, height, color);
}

/**
 * Draw text character on screen
 * @param character Character to draw
 * @param x Character coordinate
 * @param y Character coordinate
 */
int FWRenderer::drawChar(char character, int x, int y) {
	int width, idx;

	if (character == ' ') {
		x += 5;
	} else if ((width = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterWidth)) {
		idx = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterIdx;
		drawSpriteRaw(g_cine->_textHandler.textTable[idx][FONT_DATA], g_cine->_textHandler.textTable[idx][FONT_MASK], FONT_WIDTH, FONT_HEIGHT, _backBuffer, x, y);
		x += width + 1;
	}

	return x;
}

/**
 * Clears the character glyph to black
 * This function is called "undrawChar", because the original only applies
 * this drawing after the original glyph has been drawn.
 * Possible TODO: Find a better name.
 * @param character Character to undraw
 * @param x Character coordinate
 * @param y Character coordinate
 */
int FWRenderer::undrawChar(char character, int x, int y) {
	int width, idx;

	if (character == ' ') {
		x += 5;
	} else if ((width = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterWidth)) {
		idx = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterIdx;
		const byte *sprite = g_cine->_textHandler.textTable[idx][FONT_DATA];
		for (uint i = 0; i < FONT_HEIGHT; ++i) {
			byte *dst = _backBuffer + (y + i) * 320 + x;
			for (uint j = 0; j < FONT_WIDTH; ++j, ++dst) {
				// The original does this based on whether bit 1 of the pixel
				// is set. Since that's the only bit ever set in (FW) this
				// check should be fine.
				// TODO: Check how Operation Stealth Amiga works
				if (*sprite++) {
					*dst = 0;
				}
			}
		}
		x += width + 1;
	}

	return x;
}

int FWRenderer::getStringWidth(const char *str) {
	const char *p = str;
	int width = 0;

	while (*p) {
		if (*p == ' ')
			width += 5;
		else
			width += g_cine->_textHandler.fontParamTable[(unsigned char)*p].characterWidth;
		p++;
	}

	return width;
}

/**
 * Draw Line
 * @param x Line end coordinate
 * @param y Line end coordinate
 * @param width Horizontal line length
 * @param height Vertical line length
 * @param color Line color
 * @note Either width or height must be equal to 1
 */
void FWRenderer::drawLine(int x, int y, int width, int height, byte color) {
	// this line is a special case of rectangle ;-)
	drawPlainBox(x, y, width, height, color);
}

/**
 * Hide invisible parts of the sprite
 * @param[in,out] mask Mask to be updated
 * @param it Overlay info from overlayList
 */
void FWRenderer::remaskSprite(byte *mask, Common::List<overlay>::iterator it) {
	AnimData &sprite = g_cine->_animDataTable[g_cine->_objectTable[it->objIdx].frame];
	int x, y, width, height, idx;
	int mx, my, mw, mh;

	x = g_cine->_objectTable[it->objIdx].x;
	y = g_cine->_objectTable[it->objIdx].y;
	width = sprite._realWidth;
	height = sprite._height;

	for (++it; it != g_cine->_overlayList.end(); ++it) {
		if (it->type != 5) {
			continue;
		}

		idx = ABS(g_cine->_objectTable[it->objIdx].frame);
		mx = g_cine->_objectTable[it->objIdx].x;
		my = g_cine->_objectTable[it->objIdx].y;
		mw = g_cine->_animDataTable[idx]._realWidth;
		mh = g_cine->_animDataTable[idx]._height;

		gfxUpdateSpriteMask(mask, x, y, width, height, g_cine->_animDataTable[idx].data(), mx, my, mw, mh);
	}
}

/**
 * Draw background to backbuffer
 */
void FWRenderer::drawBackground() {
	assert(_background);
	memcpy(_backBuffer, _background, _screenSize);
}

/**
 * Draw one overlay
 * @param it Overlay info
 */
void FWRenderer::renderOverlay(const Common::List<overlay>::iterator &it) {
	int idx, len, width;
	ObjectStruct *obj;
	AnimData *sprite;
	byte *mask;

	switch (it->type) {
	// color sprite
	case 0:
		if (g_cine->_objectTable[it->objIdx].frame < 0) {
			return;
		}
		sprite = &g_cine->_animDataTable[g_cine->_objectTable[it->objIdx].frame];
		len = sprite->_realWidth * sprite->_height;
		mask = new byte[len];
		memcpy(mask, sprite->mask(), len);
		remaskSprite(mask, it);
		drawMaskedSprite(g_cine->_objectTable[it->objIdx], mask);
		delete[] mask;
		break;

	// game message
	case 2:
		if (it->objIdx >= g_cine->_messageTable.size()) {
			return;
		}

		_messageLen += g_cine->_messageTable[it->objIdx].size();
		drawMessage(g_cine->_messageTable[it->objIdx].c_str(), it->x, it->y, it->width, it->color);
		waitForPlayerClick = 1;
		break;

	// action failure message
	case 3:
		idx = it->objIdx * 4 + g_cine->_rnd.getRandomNumber(3);
		len = strlen(failureMessages[idx]);
		_messageLen += len;
		width = 6 * len + 20;
		width = width > 300 ? 300 : width;

		drawMessage(failureMessages[idx], (320 - width) / 2, 80, width, 4);
		waitForPlayerClick = 1;
		break;

	// bitmap
	case 4:
		assert(it->objIdx < NUM_MAX_OBJECT);
		obj = &g_cine->_objectTable[it->objIdx];

		if (obj->frame < 0) {
			return;
		}

		if (!g_cine->_animDataTable[obj->frame].data()) {
			return;
		}

		fillSprite(*obj);
		break;
	}
}

/**
 * Draw overlays
 */
void FWRenderer::drawOverlays() {
	Common::List<overlay>::iterator it;

	for (it = g_cine->_overlayList.begin(); it != g_cine->_overlayList.end(); ++it) {
		renderOverlay(it);
	}
}

/**
 * Draw another frame
 */
void FWRenderer::drawFrame() {
	drawBackground();
	drawOverlays();

	if (!_cmd.empty()) {
		drawCommand();
	}

	if (_changePal) {
		refreshPalette();
	}

	const int menus = _menuStack.size();
	for (int i = 0; i < menus; ++i)
		_menuStack[i]->drawMenu(*this, (i == menus - 1));

	blit();
}

/**
 * Turn on or off the showing of the collision page.
 * If turned on the blitting routine shows the collision page instead of the back buffer.
 * @note Useful for debugging collision page related problems.
 */
void FWRenderer::showCollisionPage(bool state) {
	_showCollisionPage = state;
}

/**
 * Update screen
 */
void FWRenderer::blit() {
	// Show the back buffer or the collision page. Normally the back
	// buffer but showing the collision page is useful for debugging.
	byte *source = (_showCollisionPage ? collisionPage : _backBuffer);
	g_system->copyRectToScreen(source, 320, 0, 0, 320, 200);
}

/**
 * Set player command string
 * @param cmd New command string
 */
void FWRenderer::setCommand(Common::String cmd) {
	_cmd = cmd;
}

/**
 * Refresh current palette
 */
void FWRenderer::refreshPalette() {
	assert(_activePal.isValid() && !_activePal.empty());
	_activePal.setGlobalOSystemPalette();
	_changePal = 0;
}

/**
 * Load palette of current background
 */
void FWRenderer::reloadPalette() {
	assert(_backupPal.isValid() && !_backupPal.empty());
	_activePal = _backupPal;
	_changePal = 1;
}

/**
 * Load background into renderer
 * @param bg Raw background data
 * @todo Combine with OSRenderer's version of loadBg16
 */
void FWRenderer::loadBg16(const byte *bg, const char *name, unsigned int idx) {
	assert(idx == 0);

	if (!_background) {
		_background = new byte[_screenSize];
	}

	assert(_background);

	strcpy(_bgName, name);

	// Load the 16 color palette
	_backupPal.load(bg, kLowPalNumBytes, kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);

	// Jump over the palette data to the background data
	bg += kLowPalNumBytes;

	gfxConvertSpriteToRaw(_background, bg, 160, 200);
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadCt16(const byte *ct, const char *name) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadBg256(const byte *bg, const char *name, unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadCt256(const byte *ct, const char *name) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::selectBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::selectScrollBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::setScroll(unsigned int shift) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Future Wars has no scrolling backgrounds so scroll value is always zero.
 */
uint FWRenderer::getScroll() const {
	return 0;
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::removeBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

void FWRenderer::saveBgNames(Common::OutSaveFile &fHandle) {
	fHandle.write(_bgName, 13);
}

const char *FWRenderer::getBgName(uint idx) const {
	assert(idx == 0);
	return _bgName;
}

/**
 * Restore active and backup palette from save
 * @param fHandle Savefile open for reading
 */
void FWRenderer::restorePalette(Common::SeekableReadStream &fHandle, int version) {
	byte buf[kLowPalNumBytes];

	// Load the active 16 color palette from file
	fHandle.read(buf, kLowPalNumBytes);
	_activePal.load(buf, sizeof(buf), kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);

	// Load the backup 16 color palette from file
	fHandle.read(buf, kLowPalNumBytes);
	_backupPal.load(buf, sizeof(buf), kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);

	_changePal = 1;
}

/**
 * Write active and backup palette to save
 * @param fHandle Savefile open for writing
 */
void FWRenderer::savePalette(Common::OutSaveFile &fHandle) {
	byte buf[kLowPalNumBytes];

	// Make sure the active palette has the correct format and color count
	assert(_activePal.colorFormat() == kLowPalFormat);
	assert(_activePal.colorCount() == kLowPalNumColors);

	// Make sure the backup palette has the correct format and color count
	assert(_backupPal.colorFormat() == kLowPalFormat);
	assert(_backupPal.colorCount() == kLowPalNumColors);

	// Write the active palette to the file
	_activePal.save(buf, sizeof(buf), CINE_BIG_ENDIAN);
	fHandle.write(buf, kLowPalNumBytes);

	// Write the backup palette to the file
	_backupPal.save(buf, sizeof(buf), CINE_BIG_ENDIAN);
	fHandle.write(buf, kLowPalNumBytes);
}

/**
 * Write active and backup palette to save
 * @param fHandle Savefile open for writing
 * @todo Add support for saving the palette in the 16 color version of Operation Stealth.
 *       Possibly combine with FWRenderer's savePalette-method?
 */
void OSRenderer::savePalette(Common::OutSaveFile &fHandle) {
	byte buf[kHighPalNumBytes];

	// We can have 16 color palette in many cases
	fHandle.writeUint16LE(_activePal.colorCount());

	// Write the active 256 color palette.
	_activePal.save(buf, sizeof(buf), CINE_LITTLE_ENDIAN);
	fHandle.write(buf, kHighPalNumBytes);

	// Write the active 256 color palette a second time.
	// FIXME: The backup 256 color palette should be saved here instead of the active one.
	fHandle.write(buf, kHighPalNumBytes);
}

/**
 * Restore active and backup palette from save
 * @param fHandle Savefile open for reading
 */
void OSRenderer::restorePalette(Common::SeekableReadStream &fHandle, int version) {
	byte buf[kHighPalNumBytes];
	uint colorCount = (version > 0) ? fHandle.readUint16LE() : kHighPalNumBytes;

	fHandle.read(buf, kHighPalNumBytes);

	if (colorCount == kHighPalNumColors) {
		// Load the active 256 color palette from file
		_activePal.load(buf, sizeof(buf), kHighPalFormat, kHighPalNumColors, CINE_LITTLE_ENDIAN);
	} else {
		_activePal.load(buf, sizeof(buf), kLowPalFormat, kLowPalNumColors, CINE_LITTLE_ENDIAN);
	}

	// Jump over the backup 256 color palette.
	// FIXME: Load the backup 256 color palette and use it properly.
	fHandle.seek(kHighPalNumBytes, SEEK_CUR);

	_changePal = 1;
}

/**
 * Rotate active palette
 * @param a First color to rotate
 * @param b Last color to rotate
 * @param c Possibly rotation step, must be 0 or 1 at the moment
 */
void FWRenderer::rotatePalette(int a, int b, int c) {
	_activePal.rotateRight(a, b, c);
	refreshPalette();
}

/**
 * Copy part of backup palette to active palette and transform
 * @param first First color to transform
 * @param last Last color to transform
 * @param r Red channel transformation
 * @param g Green channel transformation
 * @param b Blue channel transformation
 */
void FWRenderer::transformPalette(int first, int last, int r, int g, int b) {
	if (!_activePal.isValid() || _activePal.empty()) {
		_activePal = Cine::Palette(kLowPalFormat, kLowPalNumColors);
	}

	_backupPal.saturatedAddColor(_activePal, first, last, r, g, b);
	refreshPalette();
}

/**
 * Fade to black
 * \bug Operation Stealth sometimes seems to fade to black using
 * transformPalette resulting in double fadeout
 */
void FWRenderer::fadeToBlack() {
	assert(_activePal.isValid() && !_activePal.empty());

	for (int i = 0; i < 8; i++) {
		// Fade out the whole palette by 1/7th
		// (Operation Stealth used 36 / 252, which is 1 / 7. Future Wars used 1 / 7 directly).
		_activePal.saturatedAddNormalizedGray(_activePal, 0, _activePal.colorCount() - 1, -1, 7);

		refreshPalette();
		g_system->updateScreen();
		g_system->delayMillis(50);
	}
}

// Menu implementation

void FWRenderer::pushMenu(Menu *menu) {
	_menuStack.push(menu);
}

Menu *FWRenderer::popMenu() {
	if (_menuStack.empty())
		return 0;

	Menu *menu = _menuStack.top();
	_menuStack.pop();
	return menu;
}

void FWRenderer::clearMenuStack() {
	Menu *menu = 0;
	while ((menu = popMenu()) != 0)
		delete menu;
}

SelectionMenu::SelectionMenu(Common::Point p, int width, Common::StringArray elements)
	: Menu(kSelectionMenu), _pos(p), _width(width), _elements(elements), _selection(-1) {
}

void SelectionMenu::setSelection(int selection) {
	if (selection >= getElementCount() || selection < -1) {
		warning("Invalid selection %d", selection);
		selection = -1;
	}

	_selection = selection;
}

void SelectionMenu::drawMenu(FWRenderer &r, bool top) {
	const int height = getElementCount() * 9 + 10;
	int x = _pos.x;
	int y = _pos.y;

	if (x + _width > 319)
		x = 319 - _width;

	if (y + height > 199)
		y = 199 - height;

	const bool isAmiga = (g_cine->getPlatform() == Common::kPlatformAmiga);

	if (isAmiga) {
		r.drawTransparentBox(x, y, _width, height);
		r.drawDoubleBorder(x, y, _width, height, 18);
	} else {
		r.drawPlainBox(x, y, _width, height, r._messageBg);
		r.drawDoubleBorder(x, y, _width, height, 2);
	}

	int lineY = y + 4;
	int charX;

	const int elemCount = getElementCount();
	for (int i = 0; i < elemCount; ++i, lineY += 9) {
		charX = x + 4;

		if (i == _selection) {
			int color;

			if (isAmiga) {
				if (top) {
					color = 2;
				} else {
					color = 18;
				}
			} else {
				color = 0;
			}

			r.drawPlainBox(x + 2, lineY - 1, _width - 3, 9, color);
		}

		const int size = _elements[i].size();
		for (int j = 0; j < size; ++j) {
			if (isAmiga && i == _selection) {
				charX = r.undrawChar(_elements[i][j], charX, lineY);
			} else {
				charX = r.drawChar(_elements[i][j], charX, lineY);
			}
		}
	}
}

TextInputMenu::TextInputMenu(Common::Point p, int width, const char *info)
	: Menu(kTextInputMenu), _pos(p), _width(width), _info(info), _input(), _cursor(0) {
}

void TextInputMenu::setInput(const char *input, int cursor) {
	_input = input;
	_cursor = cursor;
}

void TextInputMenu::drawMenu(FWRenderer &r, bool top) {
	const int x = _pos.x;
	const int y = _pos.y;

	int i, tx, ty, tw;
	int line = 0, words = 0, cw = 0;
	int space = 0, extraSpace = 0;

	const bool isAmiga = (g_cine->getPlatform() == Common::kPlatformAmiga);

	if (isAmiga)
		r.drawTransparentBox(x, y, _width, 4);
	else
		r.drawPlainBox(x, y, _width, 4, r._messageBg);
	tx = x + 4;
	ty = _info[0] ? y - 5 : y + 4;
	tw = _width - 8;

	const int infoSize = _info.size();

	// input box info message
	for (i = 0; i < infoSize; i++, line--) {
		// fit line of text
		if (!line) {
			line = fitLine(_info.c_str() + i, tw, words, cw);

			if (i + line < infoSize && words) {
				space = (tw - cw) / words;
				extraSpace = (tw - cw) % words;
			} else {
				space = 5;
				extraSpace = 0;
			}

			ty += 9;
			if (isAmiga)
				r.drawTransparentBox(x, ty, _width, 9);
			else
				r.drawPlainBox(x, ty, _width, 9, r._messageBg);
			tx = x + 4;
		}

		// draw characters
		if (_info[i] == ' ') {
			tx += space + extraSpace;

			if (extraSpace) {
				extraSpace = 0;
			}
		} else {
			tx = r.drawChar(_info[i], tx, ty);
		}
	}

	// input area background
	ty += 9;
	if (isAmiga)
		r.drawTransparentBox(x, ty, _width, 9);
	else
		r.drawPlainBox(x, ty, _width, 9, r._messageBg);
	r.drawPlainBox(x + 16, ty - 1, _width - 32, 9, 0);
	tx = x + 20;

	// text in input area
	const int inputSize = _input.size();
	for (i = 0; i < inputSize; i++) {
		tx = r.drawChar(_input[i], tx, ty);

		if (_cursor == i + 2) {
			r.drawLine(tx, ty - 1, 1, 9, 2);
		}
	}

	if (_input.empty() || _cursor == 1) {
		r.drawLine(x + 20, ty - 1, 1, 9, 2);
	}

	ty += 9;
	if (isAmiga)
		r.drawTransparentBox(x, ty, _width, 4);
	else
		r.drawPlainBox(x, ty, _width, 4, r._messageBg);
	r.drawDoubleBorder(x, y, _width, ty - y + 4, isAmiga ? 18 : 2);
}

// -------------------

/**
 * Initialize Operation Stealth renderer
 */
OSRenderer::OSRenderer() : FWRenderer(), _bgTable(), _currentBg(0), _scrollBg(0),
	_bgShift(0) {

	_bgTable.resize(9); // Resize the background table to its required size
}

/**
 * Destroy Operation Stealth renderer
 */
OSRenderer::~OSRenderer() {
	for (uint i = 0; i < _bgTable.size(); i++) {
		_bgTable[i].clear();
	}
}

bool OSRenderer::initialize() {
	_activePal = Palette(kHighPalFormat, kHighPalNumColors);
	return true;
}

/**
 * Reset Operation Stealth renderer state
 */
void OSRenderer::clear() {
	for (uint i = 0; i < _bgTable.size(); i++) {
		_bgTable[i].clear();
	}

	_currentBg = 0;
	_scrollBg = 0;
	_bgShift = 0;

	FWRenderer::clear();
}

/**
 * Draw 1bpp sprite using selected color on backgrounds
 * @param obj Object info
 * @param fillColor Sprite color
 */
void OSRenderer::incrustMask(const BGIncrust &incrust, uint8 color) {
	const ObjectStruct &obj = g_cine->_objectTable[incrust.objIdx];
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	if (_bgTable[_currentBg].bg) {
		gfxFillSprite(data, width, height, _bgTable[_currentBg].bg, x, y, color);
	}
}

/**
 * Draw color sprite
 * @param obj Object info
 */
void OSRenderer::drawSprite(const ObjectStruct &obj) {
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height, transColor;

	x = obj.x;
	y = obj.y;
	transColor = obj.part;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	drawSpriteRaw2(data, transColor, width, height, _backBuffer, x, y);
}

/**
 * Draw color sprite
 * @param obj Object info
 */
void OSRenderer::incrustSprite(const BGIncrust &incrust) {
	const ObjectStruct &obj = g_cine->_objectTable[incrust.objIdx];
	const byte *data = g_cine->_animDataTable[incrust.frame].data();
	int x, y, width, height, transColor;

	x = incrust.x;
	y = incrust.y;
	transColor = obj.part;
	width = g_cine->_animDataTable[incrust.frame]._realWidth;
	height = g_cine->_animDataTable[incrust.frame]._height;

	if (_bgTable[_currentBg].bg) {
		drawSpriteRaw2(data, transColor, width, height, _bgTable[_currentBg].bg, x, y);
	}
}

/**
 * Draw text character on screen
 * @param character Character to draw
 * @param x Character coordinate
 * @param y Character coordinate
 */
int OSRenderer::drawChar(char character, int x, int y) {
	int width, idx;

	if (character == ' ') {
		x += 5;
	} else if ((width = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterWidth)) {
		idx = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterIdx;
		drawSpriteRaw2(g_cine->_textHandler.textTable[idx][FONT_DATA], 0, FONT_WIDTH, FONT_HEIGHT, _backBuffer, x, y);
		x += width + 1;
	}

	return x;
}

/**
 * Draw background to backbuffer
 */
void OSRenderer::drawBackground() {
	byte *main;

	main = _bgTable[_currentBg].bg;
	assert(main);

	if (!_bgShift) {
		memcpy(_backBuffer, main, _screenSize);
	} else {
		byte *scroll = _bgTable[_scrollBg].bg;
		int mainShift = _bgShift * _screenWidth;
		int mainSize = _screenSize - mainShift;

		assert(scroll);

		if (mainSize > 0) { // Just a precaution
			memcpy(_backBuffer, main + mainShift, mainSize);
		}
		if (mainShift > 0) { // Just a precaution
			memcpy(_backBuffer + mainSize, scroll, mainShift);
		}
	}
}

/**
 * Draw one overlay
 * @param it Overlay info
 * @todo Add handling of type 22 overlays
 */
void OSRenderer::renderOverlay(const Common::List<overlay>::iterator &it) {
	int len, idx, width, height;
	ObjectStruct *obj;
	AnimData *sprite;
	byte color;

	switch (it->type) {
	// color sprite
	case 0:
		if (g_cine->_objectTable[it->objIdx].frame < 0) {
			break;
		}
		sprite = &g_cine->_animDataTable[g_cine->_objectTable[it->objIdx].frame];
		drawSprite(&(*it), sprite->data(), sprite->_realWidth, sprite->_height, _backBuffer, g_cine->_objectTable[it->objIdx].x, g_cine->_objectTable[it->objIdx].y, g_cine->_objectTable[it->objIdx].part, sprite->_bpp);
		break;

	// game message
	case 2:
		if (it->objIdx >= g_cine->_messageTable.size()) {
			return;
		}

		_messageLen += g_cine->_messageTable[it->objIdx].size();
		drawMessage(g_cine->_messageTable[it->objIdx].c_str(), it->x, it->y, it->width, it->color);
		if (it->color >= 0) { // This test isn't in Future Wars's implementation
			waitForPlayerClick = 1;
		}
		break;

	// action failure message
	case 3:
		idx = it->objIdx * 4 + g_cine->_rnd.getRandomNumber(3);
		len = strlen(failureMessages[idx]);
		_messageLen += len;
		width = 6 * len + 20;
		width = width > 300 ? 300 : width;

		// The used color here differs from Future Wars
		drawMessage(failureMessages[idx], (320 - width) / 2, 80, width, _messageBg);
		waitForPlayerClick = 1;
		break;

	// bitmap
	case 4:
		if (g_cine->_objectTable[it->objIdx].frame >= 0) {
			FWRenderer::renderOverlay(it);
		}
		break;

	// masked background
	case 20:
		assert(it->objIdx < NUM_MAX_OBJECT);
		var5 = it->x; // A global variable updated here!
		obj = &g_cine->_objectTable[it->objIdx];
		sprite = &g_cine->_animDataTable[obj->frame];

		if (obj->frame < 0 || it->x < 0 || it->x > 8 || !_bgTable[it->x].bg || sprite->_bpp != 1) {
			break;
		}

		maskBgOverlay(_bgTable[it->x].bg, sprite->data(), sprite->_realWidth, sprite->_height, _backBuffer, obj->x, obj->y);
		break;

	case 22:
		// TODO: Check it this implementation really works correctly (Some things might be wrong, needs testing).
		assert(it->objIdx < NUM_MAX_OBJECT);
		obj = &g_cine->_objectTable[it->objIdx];
		color = obj->part & 0x0F;
		width = obj->frame;
		height = obj->costume;
		drawPlainBox(obj->x, obj->y, width, height, color);
		debug(5, "renderOverlay: type=%d, x=%d, y=%d, width=%d, height=%d, color=%d",
			  it->type, obj->x, obj->y, width, height, color);
		break;

	// something else
	default:
		FWRenderer::renderOverlay(it);
		break;
	}
}

/**
 * Load palette of current background
 */
void OSRenderer::reloadPalette() {
	// selected background in plane takeoff scene has swapped colors 12
	// and 14, shift background has it right
	palBg *bg = _bgShift ? &_bgTable[_scrollBg] : &_bgTable[_currentBg];

	assert(bg->pal.isValid() && !(bg->pal.empty()));

	_activePal = bg->pal;
	_changePal = 1;
}

/**
 * Copy part of backup palette to active palette and transform
 * @param first First color to transform
 * @param last Last color to transform
 * @param r Red channel transformation
 * @param g Green channel transformation
 * @param b Blue channel transformation
 */
void OSRenderer::transformPalette(int first, int last, int r, int g, int b) {
	palBg *bg = _bgShift ? &_bgTable[_scrollBg] : &_bgTable[_currentBg];

	// Initialize active palette to current background's palette format and size if they differ
	if (_activePal.colorFormat() != bg->pal.colorFormat() || _activePal.colorCount() != bg->pal.colorCount()) {
		_activePal = Cine::Palette(bg->pal.colorFormat(), bg->pal.colorCount());
	}

	bg->pal.saturatedAddColor(_activePal, first, last, r, g, b, kLowPalFormat);
	refreshPalette();
}

/**
 * Load 16 color background into renderer
 * @param bg Raw background data
 * @param name Background filename
 * @param pos Background index
 * @todo Combine with FWRenderer's version of loadBg16
 */
void OSRenderer::loadBg16(const byte *bg, const char *name, unsigned int idx) {
	assert(idx < 9);

	if (!_bgTable[idx].bg) {
		_bgTable[idx].bg = new byte[_screenSize];
	}

	assert(_bgTable[idx].bg);

	strcpy(_bgTable[idx].name, name);

	// Load the 16 color palette
	_bgTable[idx].pal.load(bg, kLowPalNumBytes, kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);

	// Jump over the palette data to the background data
	bg += kLowPalNumBytes;

	gfxConvertSpriteToRaw(_bgTable[idx].bg, bg, 160, 200);
}

/**
 * Load 16 color CT data as background into renderer
 * @param ct Raw CT data
 * @param name Background filename
 */
void OSRenderer::loadCt16(const byte *ct, const char *name) {
	// Make the 9th background point directly to the collision page
	// and load the picture into it.
	_bgTable[kCollisionPageBgIdxAlias].bg = collisionPage;
	loadBg16(ct, name, kCollisionPageBgIdxAlias);
}

/**
 * Load 256 color background into renderer
 * @param bg Raw background data
 * @param name Background filename
 * @param pos Background index
 */
void OSRenderer::loadBg256(const byte *bg, const char *name, unsigned int idx) {
	assert(idx < 9);

	if (!_bgTable[idx].bg) {
		_bgTable[idx].bg = new byte[_screenSize];
	}

	assert(_bgTable[idx].bg);

	strcpy(_bgTable[idx].name, name);
	_bgTable[idx].pal.load(bg, kHighPalNumBytes, kHighPalFormat, kHighPalNumColors, CINE_LITTLE_ENDIAN);
	memcpy(_bgTable[idx].bg, bg + kHighPalNumBytes, _screenSize);
}

/**
 * Load 256 color CT data as background into renderer
 * @param ct Raw CT data
 * @param name Background filename
 */
void OSRenderer::loadCt256(const byte *ct, const char *name) {
	// Make the 9th background point directly to the collision page
	// and load the picture into it.
	_bgTable[kCollisionPageBgIdxAlias].bg = collisionPage;
	loadBg256(ct, name, kCollisionPageBgIdxAlias);
}

/**
 * Select active background and load its palette
 * @param idx Background index
 */
void OSRenderer::selectBg(unsigned int idx) {
	assert(idx < 9);

	if (_bgTable[idx].bg) {
		assert(_bgTable[idx].pal.isValid() && !(_bgTable[idx].pal.empty()));
		_currentBg = idx;
	} else
		warning("OSRenderer::selectBg(%d) - attempt to select null background", idx);
	reloadPalette();
}

/**
 * Select scroll background
 * @param idx Scroll background index
 */
void OSRenderer::selectScrollBg(unsigned int idx) {
	assert(idx < 9);

	if (_bgTable[idx].bg) {
		_scrollBg = idx;
	}
	reloadPalette();
}

/**
 * Set background scroll
 * @param shift Background scroll in pixels
 */
void OSRenderer::setScroll(unsigned int shift) {
	assert(shift <= 200);

	_bgShift = shift;
}

/**
 * Get background scroll
 * @return Background scroll in pixels
 */
uint OSRenderer::getScroll() const {
	return _bgShift;
}

/**
 * Unload background from renderer
 * @param idx Background to unload
 */
void OSRenderer::removeBg(unsigned int idx) {
	assert(idx > 0 && idx < 9);

	if (_currentBg == idx) {
		_currentBg = 0;
	}

	if (_scrollBg == idx) {
		_scrollBg = 0;
	}

	_bgTable[idx].clear();
}

void OSRenderer::saveBgNames(Common::OutSaveFile &fHandle) {
	for (int i = 0; i < 8; i++) {
		fHandle.write(_bgTable[i].name, 13);
	}
}

const char *OSRenderer::getBgName(uint idx) const {
	assert(idx < 9);
	return _bgTable[idx].name;
}

void setMouseCursor(int cursor) {
	static int currentMouseCursor = -1;
	assert(cursor >= 0 && cursor < 3);
	if (currentMouseCursor != cursor) {
		byte mouseCursor[16 * 16];
		const MouseCursor *mc = &mouseCursors[cursor];
		const byte *src = mc->bitmap;
		for (int i = 0; i < 32; ++i) {
			int offs = i * 8;
			for (byte mask = 0x80; mask != 0; mask >>= 1) {
				if (src[0] & mask) {
					mouseCursor[offs] = 1;
				} else if (src[32] & mask) {
					mouseCursor[offs] = 0;
				} else {
					mouseCursor[offs] = 0xFF;
				}
				++offs;
			}
			++src;
		}
		CursorMan.replaceCursor(mouseCursor, 16, 16, mc->hotspotX, mc->hotspotY, 0xFF);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		currentMouseCursor = cursor;
	}
}

void gfxFillSprite(const byte *spritePtr, uint16 width, uint16 height, byte *page, int16 x, int16 y, uint8 fillColor) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width; j++) {
			if (x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200 && !*spritePtr) {
				*destPtr = fillColor;
			}

			destPtr++;
			spritePtr++;
		}
	}
}

void gfxDrawMaskedSprite(const byte *spritePtr, const byte *maskPtr, uint16 width, uint16 height, byte *page, int16 x, int16 y) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width; j++) {
			if (x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200 && *maskPtr == 0) {
				*destPtr = *spritePtr;
			}
			++destPtr;
			++spritePtr;
			++maskPtr;
		}
	}
}

void gfxUpdateSpriteMask(byte *destMask, int16 x, int16 y, int16 width, int16 height, const byte *srcMask, int16 xm, int16 ym, int16 maskWidth, int16 maskHeight) {
	int16 i, j, d, spritePitch, maskPitch;

	spritePitch = width;
	maskPitch = maskWidth;

	// crop update area to overlapping parts of masks
	if (y > ym) {
		d = y - ym;
		srcMask += d * maskPitch;
		maskHeight -= d;
	} else if (y < ym) {
		d = ym - y;
		destMask += d * spritePitch;
		height -= d;
	}

	if (x > xm) {
		d = x - xm;
		srcMask += d;
		maskWidth -= d;
	} else if (x < xm) {
		d = xm - x;
		destMask += d;
		width -= d;
	}

	// update mask
	for (j = 0; j < MIN(maskHeight, height); ++j) {
		for (i = 0; i < MIN(maskWidth, width); ++i) {
			destMask[i] |= srcMask[i] ^ 1;
		}
		destMask += spritePitch;
		srcMask += maskPitch;
	}
}

void gfxUpdateIncrustMask(byte *destMask, int16 x, int16 y, int16 width, int16 height, const byte *srcMask, int16 xm, int16 ym, int16 maskWidth, int16 maskHeight) {
	int16 i, j, d, spritePitch, maskPitch;

	spritePitch = width;
	maskPitch = maskWidth;

	// crop update area to overlapping parts of masks
	if (y > ym) {
		d = y - ym;
		srcMask += d * maskPitch;
		maskHeight -= d;
	} else if (y < ym) {
		d = ym - y > height ? height : ym - y;
		memset(destMask, 1, d * spritePitch);
		destMask += d * spritePitch;
		height -= d;
	}

	if (x > xm) {
		d = x - xm;
		xm = x;
		srcMask += d;
		maskWidth -= d;
	}

	d = xm - x;
	maskWidth += d;

	// update mask
	for (j = 0; j < MIN(maskHeight, height); ++j) {
		for (i = 0; i < width; ++i) {
			destMask[i] |= i < d || i >= maskWidth ? 1 : srcMask[i - d];
		}
		destMask += spritePitch;
		srcMask += maskPitch;
	}

	if (j < height) {
		memset(destMask, 1, (height - j) * spritePitch);
	}
}

void gfxDrawLine(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page) {
	if (x1 == x2) {
		if (y1 > y2) {
			SWAP(y1, y2);
		}
		while (y1 <= y2) {
			*(page + (y1 * 320 + x1)) = color;
			y1++;
		}
	} else {
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		while (x1 <= x2) {
			*(page + (y1 * 320 + x1)) = color;
			x1++;
		}
	}

}

void gfxDrawPlainBoxRaw(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page) {
	int16 t;

	if (x1 > x2) {
		SWAP(x1, x2);
	}

	if (y1 > y2) {
		SWAP(y1, y2);
	}

	t = x1;
	while (y1 <= y2) {
		x1 = t;
		while (x1 <= x2) {
			*(page + y1 * 320 + x1) = color;
			x1++;
		}
		y1++;
	}
}

int16 gfxGetBit(int16 x, int16 y, const byte *ptr, int16 width) {
	const byte *ptrToData = (ptr) + y * width + x;

	if (x > width) {
		return 0;
	}

	if (*ptrToData) {
		return 0;
	}

	return 1;
}

void gfxResetRawPage(byte *pageRaw) {
	memset(pageRaw, 0, 320 * 200);
}

void gfxConvertSpriteToRaw(byte *dst, const byte *src, uint16 w, uint16 h) {
	// Output is 4 bits per pixel.
	// Pixels are in 16 pixel chunks (8 bytes of source per 16 pixels of output).
	// The source data is interleaved so that
	// 1st big-endian 16-bit value contains all bit position 0 values for 16 pixels,
	// 2nd big-endian 16-bit value contains all bit position 1 values for 16 pixels,
	// 3rd big-endian 16-bit value contains all bit position 2 values for 16 pixels,
	// 4th big-endian 16-bit value contains all bit position 3 values for 16 pixels.
	// 1st pixel's bits are in the 16th bits,
	// 2nd pixel's bits are in the 15th bits,
	// 3rd pixel's bits are in the 14th bits etc.
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w / 8; ++x) {
			for (int bit = 0; bit < 16; ++bit) {
				uint8 color = 0;
				for (int p = 0; p < 4; ++p) {
					if (READ_BE_UINT16(src + p * 2) & (1 << (15 - bit))) {
						color |= 1 << p;
					}
				}
				*dst++ = color;
			}
			src += 8;
		}
	}
}

void drawSpriteRaw(const byte *spritePtr, const byte *maskPtr, int16 width, int16 height, byte *page, int16 x, int16 y) {
	int16 i, j;

	// FIXME: Is it a bug if maskPtr == NULL?
	if (!maskPtr)
		warning("drawSpriteRaw: maskPtr == NULL");

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width; j++) {
			if ((!maskPtr || !(*maskPtr)) && x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200) {
				*(destPtr++) = *(spritePtr++);
			} else {
				destPtr++;
				spritePtr++;
			}

			if (maskPtr)
				maskPtr++;
		}
	}
}

void OSRenderer::drawSprite(overlay *overlayPtr, const byte *spritePtr, int16 width, int16 height, byte *page, int16 x, int16 y, byte transparentColor, byte bpp) {
	byte *pMask = NULL;

	// draw the mask based on next objects in the list
	Common::List<overlay>::iterator it;
	for (it = g_cine->_overlayList.begin(); it != g_cine->_overlayList.end(); ++it) {
		if (&(*it) == overlayPtr) {
			break;
		}
	}

	while (it != g_cine->_overlayList.end()) {
		overlay *pCurrentOverlay = &(*it);
		if ((pCurrentOverlay->type == 5) || ((pCurrentOverlay->type == 21) && (pCurrentOverlay->x == overlayPtr->objIdx))) {
			AnimData *sprite = &g_cine->_animDataTable[g_cine->_objectTable[it->objIdx].frame];

			if (pMask == NULL) {
				pMask = new byte[width * height];

				for (int i = 0; i < height; i++) {
					for (int j = 0; j < width; j++) {
						byte spriteColor = spritePtr[width * i + j];
						pMask[width * i + j] = spriteColor;
					}
				}
			}

			for (int i = 0; i < sprite->_realWidth; i++) {
				for (int j = 0; j < sprite->_height; j++) {
					int inMaskX = (g_cine->_objectTable[it->objIdx].x + i) - x;
					int inMaskY = (g_cine->_objectTable[it->objIdx].y + j) - y;

					if (inMaskX >= 0 && inMaskX < width) {
						if (inMaskY >= 0 && inMaskY < height) {
							if (sprite->_bpp == 1) {
								if (!sprite->getColor(i, j)) {
									pMask[inMaskY * width + inMaskX] = page[x + y * 320 + inMaskX + inMaskY * 320];
								}
							}
						}
					}
				}
			}
		}
		it++;
	}

	// now, draw with the mask we created
	if (pMask) {
		spritePtr = pMask;
	}

	// ignore transparent color in 1bpp
	if (bpp == 1) {
		transparentColor = 1;
	}

	{
		for (int i = 0; i < height; i++) {
			byte *destPtr = page + x + y * 320;
			destPtr += i * 320;

			for (int j = 0; j < width; j++) {
				byte color = *(spritePtr++);
				if ((transparentColor != color) && x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200) {
					*(destPtr++) = color;
				} else {
					destPtr++;
				}
			}
		}
	}

	delete[] pMask;
}

void drawSpriteRaw2(const byte *spritePtr, byte transColor, int16 width, int16 height, byte *page, int16 x, int16 y) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width; j++) {
			if ((*spritePtr != transColor) && (x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200)) {
				*destPtr = *spritePtr;
			}
			destPtr++;
			spritePtr++;
		}
	}
}

void maskBgOverlay(const byte *bgPtr, const byte *maskPtr, int16 width, int16 height,
				   byte *page, int16 x, int16 y) {
	int16 i, j, tmpWidth, tmpHeight;
	Common::List<BGIncrust>::iterator it;
	byte *mask;
	const byte *backup = maskPtr;

	// background pass
	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		const byte *srcPtr = bgPtr + x + y * 320;
		destPtr += i * 320;
		srcPtr += i * 320;

		for (j = 0; j < width; j++) {
			if ((!maskPtr || !(*maskPtr)) && (x + j >= 0
					&& x + j < 320 && i + y >= 0 && i + y < 200)) {
				*destPtr = *srcPtr;
			}

			destPtr++;
			srcPtr++;
			maskPtr++;
		}
	}

	maskPtr = backup;

	// incrust pass
	for (it = g_cine->_bgIncrustList.begin(); it != g_cine->_bgIncrustList.end(); ++it) {
		tmpWidth = g_cine->_animDataTable[it->frame]._realWidth;
		tmpHeight = g_cine->_animDataTable[it->frame]._height;
		mask = (byte *)malloc(tmpWidth * tmpHeight);

		if (it->param == 0) {
			generateMask(g_cine->_animDataTable[it->frame].data(), mask, tmpWidth * tmpHeight, it->part);
			gfxUpdateIncrustMask(mask, it->x, it->y, tmpWidth, tmpHeight, maskPtr, x, y, width, height);
			gfxDrawMaskedSprite(g_cine->_animDataTable[it->frame].data(), mask, tmpWidth, tmpHeight, page, it->x, it->y);
		} else {
			memcpy(mask, g_cine->_animDataTable[it->frame].data(), tmpWidth * tmpHeight);
			gfxUpdateIncrustMask(mask, it->x, it->y, tmpWidth, tmpHeight, maskPtr, x, y, width, height);
			gfxFillSprite(mask, tmpWidth, tmpHeight, page, it->x, it->y);
		}

		free(mask);
	}
}

} // End of namespace Cine
