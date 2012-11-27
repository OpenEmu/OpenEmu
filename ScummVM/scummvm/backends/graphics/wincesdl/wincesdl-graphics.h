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

#ifndef BACKENDS_GRAPHICS_WINCE_SDL_H
#define BACKENDS_GRAPHICS_WINCE_SDL_H

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#include "backends/platform/wince/CEgui/CEGUI.h"

// Internal GUI names
#define NAME_MAIN_PANEL         "MainPanel"
#define NAME_PANEL_KEYBOARD     "Keyboard"
#define NAME_ITEM_OPTIONS       "Options"
#define NAME_ITEM_SKIP          "Skip"
#define NAME_ITEM_SOUND         "Sound"
#define NAME_ITEM_ORIENTATION   "Orientation"
#define NAME_ITEM_BINDKEYS      "Bindkeys"

#define TOTAL_ZONES 3

extern bool _hasSmartphoneResolution;

class WINCESdlGraphicsManager : public SurfaceSdlGraphicsManager {
public:
	WINCESdlGraphicsManager(SdlEventSource *sdlEventSource);

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL);

	bool hasFeature(OSystem::Feature f);
	void setFeatureState(OSystem::Feature f, bool enable);
	bool getFeatureState(OSystem::Feature f);

	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	bool loadGFXMode();
	void unloadGFXMode();
	bool hotswapGFXMode();

	void update_game_settings();

	// Overloaded from SDL backend (toolbar handling)
	void drawMouse();
	// Overloaded from SDL backend (new scaler handling)
	void addDirtyRect(int x, int y, int w, int h, bool mouseRect = false);
	// Overloaded from SDL backend (new scaler handling)
	void warpMouse(int x, int y);

	// Update the dirty areas of the screen
	void internUpdateScreen();
	bool saveScreenshot(const char *filename);

	// Overloaded from SDL_Common (FIXME)
	void internDrawMouse();
	void undrawMouse();
	bool showMouse(bool visible);
	void setMouseCursor(const void *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format); // overloaded by CE backend
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h); // overloaded by CE backend (FIXME)
	Graphics::Surface *lockScreen();
	void unlockScreen();
	void blitCursor();
	void showOverlay();
	void hideOverlay();
	void setMousePos(int x, int y);

	// GUI and action stuff
	void swap_panel_visibility();
	void swap_panel();
	void swap_smartphone_keyboard();
	void swap_zoom_up();
	void swap_zoom_down();
	void swap_mouse_visibility();
	void init_panel();
	void reset_panel();
	void swap_freeLook();
	bool getFreeLookState();

//#ifdef WIN32_PLATFORM_WFSP
	void move_cursor_up();
	void move_cursor_down();
	void move_cursor_left();
	void move_cursor_right();

	void switch_zone();

	void add_right_click(bool pushed);
	void add_left_click(bool pushed);

	void initZones();
	void smartphone_rotate_display();
//#endif

	bool hasPocketPCResolution();
	bool hasDesktopResolution();
	bool hasSquareQVGAResolution();
	bool hasWideResolution() const;

	bool _panelInitialized; // only initialize the toolbar once
	bool _noDoubleTapRMB;   // disable double tap -> rmb click
	bool _noDoubleTapPT;    // disable double tap for toolbar toggling

	CEGUI::ToolbarHandler _toolbarHandler;

	bool _toolbarHighDrawn;     // cache toolbar 640x80
	int _newOrientation;        // new orientation
	int _orientationLandscape;  // current orientation

	int _scaleFactorXm;     // scaler X *
	int _scaleFactorXd;     // scaler X /
	int _scaleFactorYm;     // scaler Y *
	int _scaleFactorYd;     // scaler Y /

	bool _hasfocus;         // scummvm has the top window

	MousePos _mouseCurState;

	bool _zoomUp;           // zooming up mode
	bool _zoomDown;         // zooming down mode

	bool _usesEmulatedMouse;    // emulated mousemove ever been used in this session

	int _mouseXZone[TOTAL_ZONES];
	int _mouseYZone[TOTAL_ZONES];
	int _currentZone;

	// Smartphone specific variables
	int _lastKeyPressed;        // last key pressed
	int _keyRepeat;             // number of time the last key was repeated
	int _keyRepeatTime;         // elapsed time since the key was pressed
	int _keyRepeatTrigger;      // minimum time to consider the key was repeated

	struct zoneDesc {
		int x;
		int y;
		int width;
		int height;
	};

	static zoneDesc _zones[TOTAL_ZONES];

	virtual void transformMouseCoordinates(Common::Point &point);

private:
	bool update_scalers();
	void drawToolbarMouse(SDL_Surface *surf, bool draw);
	void retrieve_mouse_location(int &x, int &y);

	void create_toolbar();
	bool _panelVisible;         // panel visibility
	bool _panelStateForced;     // panel visibility forced by external call
	String _saveActiveToolbar;  // save active toolbar when forced

	bool _canBeAspectScaled;    // game screen size allows for aspect scaling

	SDL_Rect _dirtyRectOut[NUM_DIRTY_RECT];
	bool _scalersChanged;

	bool isOzone();

	bool _saveToolbarState;     // save visibility when forced
	bool _saveToolbarZoom;      // save visibility when zooming

	SDL_Surface *_toolbarLow;   // toolbar 320x40
	SDL_Surface *_toolbarHigh;  // toolbar 640x80

	// Mouse
	int _mouseHotspotX, _mouseHotspotY;
	byte *_mouseBackupOld;
	uint16 *_mouseBackupToolbar;
	uint16 _mouseBackupDim;

	bool _forceHideMouse;       // force invisible mouse cursor
	bool _freeLook;             // freeLook mode (do not send mouse button events)

	// Smartphone specific variables
	void loadDeviceConfigurationElement(Common::String element, int &value, int defaultValue);
	int _repeatX;               // repeat trigger for left and right cursor moves
	int _repeatY;               // repeat trigger for up and down cursor moves
	int _stepX1;                // offset for left and right cursor moves (slowest)
	int _stepX2;                // offset for left and right cursor moves (faster)
	int _stepX3;                // offset for left and right cursor moves (fastest)
	int _stepY1;                // offset for up and down cursor moves (slowest)
	int _stepY2;                // offset for up and down cursor moves (faster)
	int _stepY3;                // offset for up and down cursor moves (fastest)
};

#endif /* BACKENDS_GRAPHICS_WINCE_SDL_H */
