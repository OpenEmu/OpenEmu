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

#ifndef CINE_GFX_H
#define CINE_GFX_H

#include "common/noncopyable.h"
#include "common/rect.h"
#include "common/stack.h"
#include "cine/object.h"
#include "cine/bg_list.h"

namespace Cine {

extern byte *collisionPage;
static const int kCollisionPageBgIdxAlias = 8;

/**
 * Background with palette
 */
struct palBg {
	byte *bg; ///< Background data
	Cine::Palette pal; ///< Background color palette
	char name[15]; ///< Background filename

	/** @brief Default constructor. */
	palBg() : bg(NULL), pal(), name() {
		// Make sure the name is empty (Maybe this is not needed?)
		memset(this->name, 0, sizeof(this->name));
	}

	/** @brief Clears the struct (Releases allocated memory etc). */
	void clear() {
		// In Operation Stealth the 9th background is sometimes aliased to
		// the collision page so we should take care not to double delete it
		// (The collision page is deleted elsewhere).
		if (this->bg != collisionPage) {
			delete[] this->bg;
		}
		this->bg = NULL;
		this->pal.clear();
		memset(this->name, 0, sizeof(this->name));
	}
};

class FWRenderer;

class Menu {
public:
	enum Type {
		kSelectionMenu,
		kTextInputMenu
	};

	Menu(Type t) : _type(t) {}
	virtual ~Menu() {}

	Type getType() const { return _type; }

	virtual void drawMenu(FWRenderer &r, bool top) = 0;
private:
	const Type _type;
};

class SelectionMenu : public Menu {
public:
	SelectionMenu(Common::Point p, int width, Common::StringArray elements);

	int getElementCount() const { return _elements.size(); }

	void setSelection(int selection);

	void drawMenu(FWRenderer &r, bool top);
private:
	const Common::Point _pos;
	const int _width;
	const Common::StringArray _elements;

	int _selection;
};

class TextInputMenu : public Menu {
public:
	TextInputMenu(Common::Point p, int width, const char *info);

	void setInput(const char *input, int cursor);

	void drawMenu(FWRenderer &r, bool top);
private:
	const Common::Point _pos;
	const int _width;
	const Common::String _info;

	Common::String _input;
	int _cursor;
};

/**
 * Future Wars renderer
 *
 * Screen backbuffer is not cleared between frames.
 */
class FWRenderer : public Common::NonCopyable {
	// TODO: Consider getting rid of this
	friend class SelectionMenu;
	friend class TextInputMenu;
private:
	byte *_background; ///< Current background
	char _bgName[13]; ///< Background filename

	Common::String _cmd; ///< Player command string

protected:
	static const int _screenSize = 320 * 200; ///< Screen size
	static const int _screenWidth = 320; ///< Screen width
	static const int _screenHeight = 200; ///< Screen height

	byte *_backBuffer; ///< Screen backbuffer
	Cine::Palette _backupPal; ///< The backup color palette
	Cine::Palette _activePal; ///< The active color palette
	Common::Stack<Menu *> _menuStack; ///< All displayed menus
	int _changePal; ///< Load active palette to video backend on next frame
	bool _showCollisionPage; ///< Should we show the collision page instead of the back buffer? Used for debugging.

	void fillSprite(const ObjectStruct &obj, uint8 color = 0);
	void drawMaskedSprite(const ObjectStruct &obj, const byte *mask);
	virtual void drawSprite(const ObjectStruct &obj);

	void drawCommand();
	void drawMessage(const char *str, int x, int y, int width, int color);
	void drawPlainBox(int x, int y, int width, int height, byte color);
	void drawTransparentBox(int x, int y, int width, int height);
	void drawBorder(int x, int y, int width, int height, byte color);
	void drawDoubleBorder(int x, int y, int width, int height, byte color);
	virtual int drawChar(char character, int x, int y);
	virtual int undrawChar(char character, int x, int y);
	void drawLine(int x, int y, int width, int height, byte color);
	void remaskSprite(byte *mask, Common::List<overlay>::iterator it);
	virtual void drawBackground();

	virtual void renderOverlay(const Common::List<overlay>::iterator &it);
	void drawOverlays();

	void blit();

public:
	uint16 _messageBg; ///< Message box background color
	uint16 _cmdY; ///< Player command string position on screen

	FWRenderer();
	virtual ~FWRenderer();

	virtual bool initialize();

	/** Test if renderer is ready to draw */
	virtual bool ready() { return _background != NULL; }

	virtual void clear();

	void drawFrame();
	void setCommand(Common::String cmd);

	virtual void incrustMask(const BGIncrust &incrust, uint8 color = 0);
	virtual void incrustSprite(const BGIncrust &incrust);

	virtual void loadBg16(const byte *bg, const char *name, unsigned int idx = 0);
	virtual void loadCt16(const byte *ct, const char *name);
	virtual void loadBg256(const byte *bg, const char *name, unsigned int idx = 0);
	virtual void loadCt256(const byte *ct, const char *name);
	virtual void selectBg(unsigned int idx);
	virtual void selectScrollBg(unsigned int idx);
	virtual void setScroll(unsigned int shift);
	virtual uint getScroll() const;
	virtual void removeBg(unsigned int idx);
	virtual void saveBgNames(Common::OutSaveFile &fHandle);
	virtual const char *getBgName(uint idx = 0) const;

	virtual void refreshPalette();
	virtual void reloadPalette();
	virtual void restorePalette(Common::SeekableReadStream &fHandle, int version);
	virtual void savePalette(Common::OutSaveFile &fHandle);
	virtual void rotatePalette(int a, int b, int c);
	virtual void transformPalette(int first, int last, int r, int g, int b);

	void pushMenu(Menu *menu);
	Menu *popMenu();
	void clearMenuStack();

	virtual void fadeToBlack();
	void showCollisionPage(bool state);

	void drawString(const char *string, byte param);
	int getStringWidth(const char *str);
};

/**
 * Operation Stealth renderer
 */
class OSRenderer : public FWRenderer {
private:
	Common::Array<palBg> _bgTable; ///< Table of backgrounds loaded into renderer (Maximum is 9)
	unsigned int _currentBg; ///< Current background
	unsigned int _scrollBg; ///< Current scroll background
	unsigned int _bgShift; ///< Background shift

protected:

	void drawSprite(const ObjectStruct &obj);
	void drawSprite(overlay *overlayPtr, const byte *spritePtr, int16 width, int16 height, byte *page, int16 x, int16 y, byte transparentColor, byte bpp);
	int drawChar(char character, int x, int y);
	void drawBackground();
	void renderOverlay(const Common::List<overlay>::iterator &it);

public:
	OSRenderer();
	~OSRenderer();

	bool initialize();

	/** Test if renderer is ready to draw */
	bool ready() { return _bgTable[_currentBg].bg != NULL; }

	void clear();

	void incrustMask(const BGIncrust &incrust, uint8 color = 0);
	void incrustSprite(const BGIncrust &incrust);

	void loadBg16(const byte *bg, const char *name, unsigned int idx = 0);
	void loadCt16(const byte *ct, const char *name);
	void loadBg256(const byte *bg, const char *name, unsigned int idx = 0);
	void loadCt256(const byte *ct, const char *name);
	void selectBg(unsigned int idx);
	void selectScrollBg(unsigned int idx);
	void setScroll(unsigned int shift);
	uint getScroll() const;
	void removeBg(unsigned int idx);
	void saveBgNames(Common::OutSaveFile &fHandle);
	const char *getBgName(uint idx = 0) const;

	void reloadPalette();
	void restorePalette(Common::SeekableReadStream &fHandle, int version);
	void savePalette(Common::OutSaveFile &fHandle);
	void transformPalette(int first, int last, int r, int g, int b);

};

void gfxDrawSprite(byte *src4, uint16 sw, uint16 sh, byte *dst4, int16 sx, int16 sy);

extern FWRenderer *renderer;

void setMouseCursor(int cursor);
void gfxCopyPage(byte *source, byte *dest);

void transformPaletteRange(byte startColor, byte numColor, int8 r, int8 g, int8 b);
void gfxFlipPage();

void gfxDrawMaskedSprite(const byte *ptr, const byte *msk, uint16 width, uint16 height, byte *page, int16 x, int16 y);
void gfxFillSprite(const byte *src4, uint16 sw, uint16 sh, byte *dst4, int16 sx, int16 sy, uint8 fillColor = 0);

void gfxUpdateSpriteMask(byte *destMask, int16 x, int16 y, int16 width, int16 height, const byte *maskPtr, int16 xm, int16 ym, int16 maskWidth, int16 maskHeight);

void gfxDrawLine(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page);
void gfxDrawPlainBox(int16 x1, int16 y1, int16 x2, int16 y2, byte color);

void gfxResetPage(byte *pagePtr);

int16 gfxGetBit(int16 x, int16 y, const byte *ptr, int16 width);
byte gfxGetColor(int16 x, int16 y, const byte *ptr, int16 width);

void gfxResetRawPage(byte *pageRaw);
void gfxConvertSpriteToRaw(byte *dst, const byte *src, uint16 w, uint16 h);
void gfxCopyRawPage(byte *source, byte *dest);
void gfxFlipRawPage(byte *frontBuffer);
void drawSpriteRaw(const byte *spritePtr, const byte *maskPtr, int16 width, int16 height, byte *page, int16 x, int16 y);
void gfxDrawPlainBoxRaw(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page);
void drawSpriteRaw2(const byte *spritePtr, byte transColor, int16 width, int16 height, byte *page, int16 x, int16 y);
void maskBgOverlay(const byte *spritePtr, const byte *maskPtr, int16 width, int16 height, byte *page, int16 x, int16 y);

void fadeFromBlack();
void fadeToBlack();

// wtf?!
//void gfxDrawMaskedSprite(byte *param1, byte *param2, byte *param3, byte *param4, int16 param5);
void gfxWaitVBL();
void gfxRedrawMouseCursor();

void blitScreen(byte *frontBuffer, byte *backbuffer);
void blitRawScreen(byte *frontBuffer);
void flip();

} // End of namespace Cine

#endif
