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

#ifndef BACKENDS_GRAPHICS_SURFACESDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_SURFACESDL_GRAPHICS_H

#include "backends/graphics/graphics.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include "graphics/pixelformat.h"
#include "graphics/scaler.h"
#include "common/events.h"
#include "common/system.h"

#include "backends/events/sdl/sdl-events.h"

#include "backends/platform/sdl/sdl-sys.h"

#ifndef RELEASE_BUILD
// Define this to allow for focus rectangle debugging
#define USE_SDL_DEBUG_FOCUSRECT
#endif

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
// Uncomment this to enable the 'on screen display' code.
#define USE_OSD	1
#endif

enum {
	GFX_NORMAL = 0,
	GFX_DOUBLESIZE = 1,
	GFX_TRIPLESIZE = 2,
	GFX_2XSAI = 3,
	GFX_SUPER2XSAI = 4,
	GFX_SUPEREAGLE = 5,
	GFX_ADVMAME2X = 6,
	GFX_ADVMAME3X = 7,
	GFX_HQ2X = 8,
	GFX_HQ3X = 9,
	GFX_TV2X = 10,
	GFX_DOTMATRIX = 11
};


class AspectRatio {
	int _kw, _kh;
public:
	AspectRatio() { _kw = _kh = 0; }
	AspectRatio(int w, int h);

	bool isAuto() const { return (_kw | _kh) == 0; }

	int kw() const { return _kw; }
	int kh() const { return _kh; }
};

/**
 * SDL graphics manager
 */
class SurfaceSdlGraphicsManager : public GraphicsManager, public SdlGraphicsManager, public Common::EventObserver {
public:
	SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource);
	virtual ~SurfaceSdlGraphicsManager();

	virtual void initEventObserver();

	virtual bool hasFeature(OSystem::Feature f);
	virtual void setFeatureState(OSystem::Feature f, bool enable);
	virtual bool getFeatureState(OSystem::Feature f);

	static const OSystem::GraphicsMode *supportedGraphicsModes();
	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual void resetGraphicsScale();
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const { return _screenFormat; }
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif
	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL);
	virtual int getScreenChangeID() const { return _screenChangeCount; }

	virtual void beginGFXTransaction();
	virtual OSystem::TransactionError endGFXTransaction();

	virtual int16 getHeight();
	virtual int16 getWidth();

protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);

public:
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void fillScreen(uint32 col);
	virtual void updateScreen();
	virtual void setShakePos(int shakeOffset);
	virtual void setFocusRectangle(const Common::Rect& rect);
	virtual void clearFocusRectangle();

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual Graphics::PixelFormat getOverlayFormat() const { return _overlayFormat; }
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight() { return _videoMode.overlayHeight; }
	virtual int16 getOverlayWidth() { return _videoMode.overlayWidth; }

	virtual bool showMouse(bool visible);
	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);

#ifdef USE_OSD
	virtual void displayMessageOnOSD(const char *msg);
#endif

	// Override from Common::EventObserver
	bool notifyEvent(const Common::Event &event);

	// SdlGraphicsManager interface
	virtual void notifyVideoExpose();
	virtual void transformMouseCoordinates(Common::Point &point);
	virtual void notifyMousePos(Common::Point mouse);

protected:
#ifdef USE_OSD
	/** Surface containing the OSD message */
	SDL_Surface *_osdSurface;
	/** Transparency level of the OSD */
	uint8 _osdAlpha;
	/** When to start the fade out */
	uint32 _osdFadeStartTime;
	/** Enum with OSD options */
	enum {
		kOSDFadeOutDelay = 2 * 1000,	/** < Delay before the OSD is faded out (in milliseconds) */
		kOSDFadeOutDuration = 500,		/** < Duration of the OSD fade out (in milliseconds) */
		kOSDColorKey = 1,				/** < Transparent color key */
		kOSDInitialAlpha = 80			/** < Initial alpha level, in percent */
	};
#endif

	/** Hardware screen */
	SDL_Surface *_hwscreen;

	/** Unseen game screen */
	SDL_Surface *_screen;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _screenFormat;
	Graphics::PixelFormat _cursorFormat;
	Common::List<Graphics::PixelFormat> _supportedFormats;

	/**
	 * Update the list of supported pixel formats.
	 * This method is invoked by loadGFXMode().
	 */
	void detectSupportedFormats();
#endif

	/** Temporary screen (for scalers) */
	SDL_Surface *_tmpscreen;
	/** Temporary screen (for scalers) */
	SDL_Surface *_tmpscreen2;

	SDL_Surface *_overlayscreen;
	bool _overlayVisible;
	Graphics::PixelFormat _overlayFormat;

	enum {
		kTransactionNone = 0,
		kTransactionActive = 1,
		kTransactionRollback = 2
	};

	struct TransactionDetails {
		bool sizeChanged;
		bool needHotswap;
		bool needUpdatescreen;
		bool normal1xScaler;
#ifdef USE_RGB_COLOR
		bool formatChanged;
#endif
	};
	TransactionDetails _transactionDetails;

	struct VideoState {
		bool setup;

		bool fullscreen;
		bool aspectRatioCorrection;
		AspectRatio desiredAspectRatio;

		int mode;
		int scaleFactor;

		int screenWidth, screenHeight;
		int overlayWidth, overlayHeight;
		int hardwareWidth, hardwareHeight;
#ifdef USE_RGB_COLOR
		Graphics::PixelFormat format;
#endif
	};
	VideoState _videoMode, _oldVideoMode;

	/** Force full redraw on next updateScreen */
	bool _forceFull;

	ScalerProc *_scalerProc;
	int _scalerType;
	int _transactionMode;

	bool _screenIsLocked;
	Graphics::Surface _framebuffer;

	int _screenChangeCount;

	enum {
		NUM_DIRTY_RECT = 100,
		MAX_SCALING = 3
	};

	// Dirty rect management
	SDL_Rect _dirtyRectList[NUM_DIRTY_RECT];
	int _numDirtyRects;

	struct MousePos {
		// The mouse position, using either virtual (game) or real
		// (overlay) coordinates.
		int16 x, y;

		// The size and hotspot of the original cursor image.
		int16 w, h;
		int16 hotX, hotY;

		// The size and hotspot of the pre-scaled cursor image, in real
		// coordinates.
		int16 rW, rH;
		int16 rHotX, rHotY;

		// The size and hotspot of the pre-scaled cursor image, in game
		// coordinates.
		int16 vW, vH;
		int16 vHotX, vHotY;

		MousePos() : x(0), y(0), w(0), h(0), hotX(0), hotY(0),
					rW(0), rH(0), rHotX(0), rHotY(0), vW(0), vH(0),
					vHotX(0), vHotY(0)
			{ }
	};

	bool _mouseVisible;
	bool _mouseNeedsRedraw;
	byte *_mouseData;
	SDL_Rect _mouseBackup;
	MousePos _mouseCurState;
#ifdef USE_RGB_COLOR
	uint32 _mouseKeyColor;
#else
	byte _mouseKeyColor;
#endif
	bool _cursorDontScale;
	bool _cursorPaletteDisabled;
	SDL_Surface *_mouseOrigSurface;
	SDL_Surface *_mouseSurface;
	enum {
		kMouseColorKey = 1
	};

	// Shake mode
	int _currentShakePos;
	int _newShakePos;

	// Palette data
	SDL_Color *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;

	// Cursor palette data
	SDL_Color *_cursorPalette;

	/**
	 * Mutex which prevents multiple threads from interfering with each other
	 * when accessing the screen.
	 */
	OSystem::MutexRef _graphicsMutex;

#ifdef USE_SDL_DEBUG_FOCUSRECT
	bool _enableFocusRectDebugCode;
	bool _enableFocusRect;
	Common::Rect _focusRect;
#endif

	virtual void addDirtyRect(int x, int y, int w, int h, bool realCoordinates = false);

	virtual void drawMouse();
	virtual void undrawMouse();
	virtual void blitCursor();

	virtual void internUpdateScreen();

	virtual bool loadGFXMode();
	virtual void unloadGFXMode();
	virtual bool hotswapGFXMode();

	virtual void setFullscreenMode(bool enable);
	virtual void setAspectRatioCorrection(bool enable);

	virtual int effectiveScreenHeight() const;

	virtual void setGraphicsModeIntern();

	virtual bool handleScalerHotkeys(Common::KeyCode key);
	virtual bool isScalerHotkey(const Common::Event &event);
	virtual void setMousePos(int x, int y);
	virtual void toggleFullScreen();
	virtual bool saveScreenshot(const char *filename);
};

#endif
