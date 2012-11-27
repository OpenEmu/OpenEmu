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

#ifndef BACKENDS_GRAPHICS_OPENGL_H
#define BACKENDS_GRAPHICS_OPENGL_H

#include "backends/graphics/opengl/gltexture.h"
#include "backends/graphics/graphics.h"
#include "common/array.h"
#include "common/rect.h"
#include "graphics/font.h"
#include "graphics/pixelformat.h"

// Uncomment this to enable the 'on screen display' code.
#define USE_OSD	1

namespace OpenGL {
// The OpenGL GFX modes. They have to be inside the OpenGL namespace so they
// do not clash with the SDL GFX modes.
enum {
	GFX_NORMAL = 0,
	GFX_CONSERVE = 1,
	GFX_ORIGINAL = 2
};

}

/**
 * OpenGL graphics manager. This is an abstract class, it does not do the
 * window and OpenGL context initialization.
 * Derived classes should at least override internUpdateScreen for doing
 * the buffers swap, and implement loadGFXMode for handling the window/context if
 * needed. If USE_RGB_COLOR is enabled, getSupportedFormats must be implemented.
 */
class OpenGLGraphicsManager : public GraphicsManager {
public:
	OpenGLGraphicsManager();
	virtual ~OpenGLGraphicsManager();

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
	virtual Graphics::PixelFormat getScreenFormat() const;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const = 0;
#endif
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL);
	virtual int getScreenChangeID() const;

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
	virtual void setFocusRectangle(const Common::Rect &rect);
	virtual void clearFocusRectangle();

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual Graphics::PixelFormat getOverlayFormat() const;
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();

	virtual bool showMouse(bool visible);
	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	virtual void displayMessageOnOSD(const char *msg);
protected:
	/**
	 * Setup OpenGL settings
	 */
	virtual void initGL();

	/**
	 * Creates and refreshs OpenGL textures.
	 */
	virtual void loadTextures();

	//
	// GFX and video
	//
	enum {
		kTransactionNone = 0,
		kTransactionActive = 1,
		kTransactionRollback = 2
	};

	struct TransactionDetails {
		bool sizeChanged;
		bool needRefresh;
		bool needUpdatescreen;
		bool filterChanged;
#ifdef USE_RGB_COLOR
		bool formatChanged;
#endif
	};
	TransactionDetails _transactionDetails;
	int _transactionMode;

	struct VideoState {
		bool setup;

		bool fullscreen;

		int mode;
		int scaleFactor;
		bool antialiasing;
		bool aspectRatioCorrection;

		int screenWidth, screenHeight;
		int overlayWidth, overlayHeight;
		int hardwareWidth, hardwareHeight;
#ifdef USE_RGB_COLOR
		Graphics::PixelFormat format;
#endif
	};
	VideoState _videoMode, _oldVideoMode;

	/**
	 * Sets the OpenGL texture format for the given pixel format. If format is not support will raise an error.
	 */
	virtual void getGLPixelFormat(Graphics::PixelFormat pixelFormat, byte &bpp, GLenum &intFormat, GLenum &glFormat, GLenum &type);

	virtual void internUpdateScreen();
	virtual bool loadGFXMode();
	virtual void unloadGFXMode();

	/**
	 * Setup the fullscreen mode state.
	 */
	void setFullscreenMode(bool enable);

	/**
	 * Query the fullscreen state.
	 */
	inline bool getFullscreenMode() const { return _videoMode.fullscreen; }

	/**
	 * Set the scale factor.
	 *
	 * This can only be used in a GFX transaction.
	 *
	 * @param newScale New scale factor.
	 */
	void setScale(int newScale);

	/**
	 * Query the scale factor.
	 */
	inline int getScale() const { return _videoMode.scaleFactor; }

	/**
	 * Toggle the antialiasing state of the current video mode.
	 *
	 * This can only be used in a GFX transaction.
	 */
	void toggleAntialiasing();

	/**
	 * Query the antialiasing state.
	 */
	inline bool getAntialiasingState() const { return _videoMode.antialiasing; }

	// Drawing coordinates for the current display mode and scale
	int _displayX;
	int _displayY;
	int _displayWidth;
	int _displayHeight;

	virtual const char *getCurrentModeName();

	virtual void calculateDisplaySize(int &width, int &height);
	virtual void refreshDisplaySize();

	uint getAspectRatio() const;

	void setFormatIsBGR(bool isBGR) { _formatBGR = isBGR; }
	bool _formatBGR;

	//
	// Game screen
	//
	GLTexture *_gameTexture;
	Graphics::Surface _screenData;
	int _screenChangeCount;
	bool _screenNeedsRedraw;
	Common::Rect _screenDirtyRect;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _screenFormat;
#endif
	byte *_gamePalette;

	virtual void refreshGameScreen();

	// Shake mode
	int _shakePos;

	//
	// Overlay
	//
	GLTexture *_overlayTexture;
	Graphics::Surface _overlayData;
	Graphics::PixelFormat _overlayFormat;
	bool _overlayVisible;
	bool _overlayNeedsRedraw;
	Common::Rect _overlayDirtyRect;

	virtual void refreshOverlay();

	//
	// Mouse
	//
	struct MousePos {
		// The mouse position in hardware screen coordinates.
		int16 x, y;

		// The size and hotspot of the original cursor image.
		int16 w, h;
		int16 hotX, hotY;

		// The size and hotspot of the scaled cursor, in real coordinates.
		int16 rW, rH;
		int16 rHotX, rHotY;

		// The size and hotspot of the scaled cursor, in game coordinates.
		int16 vW, vH;
		int16 vHotX, vHotY;

		MousePos() : x(0), y(0), w(0), h(0), hotX(0), hotY(0),
					rW(0), rH(0), rHotX(0), rHotY(0), vW(0), vH(0),
					vHotX(0), vHotY(0)	{}
	};

	GLTexture *_cursorTexture;
	Graphics::Surface _cursorData;
	Graphics::PixelFormat _cursorFormat;
	byte *_cursorPalette;
	bool _cursorPaletteDisabled;
	MousePos _cursorState;
	bool _cursorVisible;
	uint32 _cursorKeyColor;
	bool _cursorDontScale;
	bool _cursorNeedsRedraw;

	/**
	 * Set up the mouse position for graphics output.
	 *
	 * @param x X coordinate in native coordinates.
	 * @param y Y coordinate in native coordinates.
	 */
	void setMousePosition(int x, int y) { _cursorState.x = x; _cursorState.y = y; }

	virtual void refreshCursor();
	virtual void refreshCursorScale();

	/**
	 * Set up the mouse position for the (event) system.
	 *
	 * @param x X coordinate in native coordinates.
	 * @param y Y coordinate in native coordinates.
	 */
	virtual void setInternalMousePosition(int x, int y) = 0;

	/**
	 * Adjusts hardware screen coordinates to either overlay or game screen
	 * coordinates depending on whether the overlay is visible or not.
	 *
	 * @param x X coordinate of the mouse position.
	 * @param y Y coordinate of the mouse position.
	 */
	virtual void adjustMousePosition(int16 &x, int16 &y);

	//
	// Misc
	//
	virtual bool saveScreenshot(const char *filename);

#ifdef USE_OSD
	/**
	 * Returns the font used for on screen display
	 */
	virtual const Graphics::Font *getFontOSD();

	/**
	 * Update the OSD texture / surface.
	 */
	void updateOSD();

	/**
	 * The OSD contents.
	 */
	Common::Array<Common::String> _osdLines;

	GLTexture *_osdTexture;
	Graphics::Surface _osdSurface;
	uint8 _osdAlpha;
	uint32 _osdFadeStartTime;
	bool _requireOSDUpdate;
	enum {
		kOSDFadeOutDelay = 2 * 1000,
		kOSDFadeOutDuration = 500,
		kOSDInitialAlpha = 80
	};
#endif
};

#endif
