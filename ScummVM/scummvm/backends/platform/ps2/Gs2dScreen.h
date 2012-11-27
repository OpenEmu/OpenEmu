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

#ifndef __GS2DSCREEN_H__
#define __GS2DSCREEN_H__

#include "sysdefs.h"
#include "backends/base-backend.h"

#include "backends/platform/ps2/DmaPipe.h"
#include "graphics/surface.h"


enum TVMode {
	TV_DONT_CARE = 0,
	TV_PAL,
	TV_NTSC
};

enum GsInterlace {
	GS_NON_INTERLACED = 0,
	GS_INTERLACED
};


class Gs2dScreen {
public:
	Gs2dScreen(uint16 width, uint16 height, TVMode tvMode);
	~Gs2dScreen(void);
	void newScreenSize(uint16 width, uint16 height);
	uint8 tvMode(void);
	uint16 getWidth(void);
	uint16 getHeight(void);

	void copyPrintfOverlay(const uint8* buf);
	void clearPrintfOverlay(void);

	Graphics::Surface *lockScreen();
	void unlockScreen();

	void copyScreenRect(const uint8 *buf, int pitch, int x, int y, int w, int h);
	void setPalette(const uint8 *pal, uint8 start, uint16 num);
	void updateScreen(void);
	void grabPalette(uint8 *pal, uint8 start, uint16 num);
	//- overlay routines
	void copyOverlayRect(const byte *buf, uint16 pitch, uint16 x, uint16 y, uint16 w, uint16 h);
	void grabOverlay(byte *buf, uint16 pitch);
	void clearOverlay(void);
	void showOverlay(void);
	void hideOverlay(void);
	Graphics::PixelFormat getOverlayFormat(void);
	int16 getOverlayWidth(void);
	int16 getOverlayHeight(void);

	//- mouse routines
	void setMouseOverlay(const uint8 *buf, uint16 width, uint16 height, uint16 hotSpotX, uint16 hotSpotY, uint8 transpCol);
	void showMouse(bool show);
	void setMouseXy(int16 x, int16 y);
	void setShakePos(int shake);

	void animThread(void);
	void wantAnim(bool runIt);

	void quit(void);

private:
	void uploadToVram(void);
	void createAnimTextures(void);

	DmaPipe *_dmaPipe;
	uint8 _tvMode;
	uint16 _tvWidth, _tvHeight;
	GsVertex _blitCoords[2];
	TexVertex _texCoords[2];

	uint8  _curDrawBuf;
	uint32 _frameBufPtr[2]; //
	uint32 _clutPtrs[3];    //   vram pointers
	uint32 _texPtrs[4];     //

	Graphics::Surface _framebuffer;

	/* TODO : check if we do need this */
    struct VideoState {
        bool setup;

        bool fullscreen;
        bool aspectRatio;

        int mode;
        int scaleFactor;

        int screenWidth, screenHeight;
        int overlayWidth, overlayHeight;
    };

	VideoState _videoMode;
	/* */

	uint16 _width, _height, _pitch;
	int16  _mouseX, _mouseY, _hotSpotX, _hotSpotY;
	uint32 _mouseScaleX, _mouseScaleY;
	uint8  _mTraCol;

	int _shakePos;

	bool _showMouse, _showOverlay, _screenChanged, _overlayChanged, _clutChanged;
	uint16 *_overlayBuf;
	uint8 *_screenBuf;
	uint32 *_clut;
	Graphics::PixelFormat _overlayFormat;

	int _screenSema;
	int _vblankStartId, _vblankEndId, _dmacId, _animTid;
	void *_animStack;
	volatile bool _systemQuit;

	static const uint32 _binaryClut[16];
	static const uint8  _binaryData[4 * 14 * 2];
	static const uint16 _binaryPattern[16];
};

#endif // __GS2DSCREEN_H__
