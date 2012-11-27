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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <romfs.h>

#include <malloc.h> // Required for memalign

#include "osys_n64.h"
#include "pakfs_save_manager.h"
#include "framfs_save_manager.h"
#include "backends/fs/n64/n64-fs-factory.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"

typedef unsigned long long uint64;

extern uint8 _romfs; // Defined by linker (used to calculate position of romfs image)

inline uint16 colRGB888toBGR555(byte r, byte g, byte b);

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{ "320x240 (PAL) fix overscan", "340x240 PAL",  OVERS_PAL_340X240   },
	{ "320x240 (PAL) overscan", "320x240 PAL",  NORM_PAL_320X240   },
	{ "320x240 (MPAL) fix overscan", "340x240 MPAL", OVERS_MPAL_340X240  },
	{ "320x240 (MPAL) overscan", "320x240 MPAL", NORM_MPAL_320X240  },
	{ "340x240 (NTSC) fix overscan", "340x240 NTSC", OVERS_NTSC_340X240 },
	{ "320x240 (NTSC) overscan", "320x240 NTSC", NORM_NTSC_320X240  },
	{ 0, 0, 0 }
};

OSystem_N64::OSystem_N64() {
	// Enable Mips interrupts
	set_MI_interrupt(1);

	// Initialize display: NTSC 340x240 (16 bit)
	initDisplay(NTSC_340X240_16BIT);

	// Prepare virtual text layer for debugging purposes
	initTextLayer();

	// Init PI interface
	PI_Init();

	// Screen size
	_screenWidth = 320;
	_screenHeight = 240;

	// Game screen size
	_gameHeight = 320;
	_gameWidth = 240;

	// Overlay size
	_overlayWidth = 320;
	_overlayHeight = 240;

	// Framebuffer width
	_frameBufferWidth = 340;

	// Pixels to skip
	_offscrPixels = 16;

	// Video clock
	_viClockRate = VI_NTSC_CLOCK;

	// Max FPS
	_maxFps = N64_NTSC_FPS;

	_disableFpsLimit = false;

	_overlayVisible = false;

	_shakeOffset = 0;

	// Allocate memory for offscreen buffers
	_offscreen_hic = (uint16 *)memalign(8, _screenWidth * _screenHeight * 2);
	_offscreen_pal = (uint8 *)memalign(8, _screenWidth * _screenHeight);
	_overlayBuffer = (uint16 *)memalign(8, _overlayWidth * _overlayHeight * sizeof(uint16));

	_cursor_pal = NULL;
	_cursor_hic = NULL;

	_cursorWidth = 0;
	_cursorHeight = 0;
	_cursorKeycolor = -1;
	_mouseHotspotX = _mouseHotspotY = -1;

	// Clean offscreen buffers
	memset(_offscreen_hic, 0, _screenWidth * _screenHeight * 2);
	memset(_offscreen_pal, 0, _screenWidth * _screenHeight);
	memset(_overlayBuffer, 0, _overlayWidth * _overlayHeight * sizeof(uint16));

	// Default graphic mode
	_graphicMode = OVERS_NTSC_340X240;

	// Clear palette array
	_screenPalette = (uint16 *)memalign(8, 256 * sizeof(uint16));
#ifndef N64_EXTREME_MEMORY_SAVING
	_screenExactPalette = (uint8 *)memalign(8, 256 * 3);
	memset(_screenExactPalette, 0, 256 * 3);
#endif
	memset(_screenPalette, 0, 256 * sizeof(uint16));
	memset(_cursorPalette, 0, 256 * sizeof(uint16));

	_dirtyPalette = false;
	_cursorPaletteDisabled = false;

	_audioEnabled = false;

	// Initialize ROMFS access interface
	initRomFSmanager((uint8 *)(((uint32)&_romfs + (uint32)0xc00) | (uint32)0xB0000000));

	_mouseVisible = false;

	_mouseX = _overlayWidth  / 2;
	_mouseY = _overlayHeight / 2;
	_tempMouseX = _mouseX;
	_tempMouseY = _mouseY;
	_mouseMaxX = _overlayWidth;
	_mouseMaxY = _overlayHeight;

	_mixer = 0;

	_dirtyOffscreen = false;

	detectControllers();

	_controllerHasRumble = (identifyPak(_controllerPort) == 2);

	_fsFactory = new N64FilesystemFactory();

	// Register vblank callback (this MUST be done at the END of init).
	registerVIhandler(vblCallback);
}

OSystem_N64::~OSystem_N64() {
	delete _mixer;
}

void OSystem_N64::initBackend() {
	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_high_quality", false);
	ConfMan.setBool("FM_medium_quality", true);
	ConfMan.set("gui_theme", "modern"); // In case of modern theme being present, use it.

	FRAM_Init();

	if (FRAM_Detect()) { // Use FlashRAM
		initFramFS();
		_savefileManager = new FRAMSaveManager();
	} else { // Use PakFS
		// Init Controller Pak
		initPakFs();

		// Use the first controller pak found
		uint8 ctrl_num;
		for (ctrl_num = 0; ctrl_num < 4; ctrl_num++) {
			int8 pak_type = identifyPak(ctrl_num);
			if (pak_type == 1) {
				loadPakData(ctrl_num);
				break;
			}
		}

		_savefileManager = new PAKSaveManager();
	}

	_timerManager = new DefaultTimerManager();

	setTimerCallback(&timer_handler, 10);

	setupMixer();

	EventsBaseBackend::initBackend();
}

bool OSystem_N64::hasFeature(Feature f) {
	return (f == kFeatureCursorPalette);
}

void OSystem_N64::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureCursorPalette) {
		_cursorPaletteDisabled = !enable;

		// Rebuild cursor hicolor buffer
		rebuildOffscreenMouseBuffer();

		_dirtyOffscreen = true;
	}
}

bool OSystem_N64::getFeatureState(Feature f) {
	if (f == kFeatureCursorPalette)
		return !_cursorPaletteDisabled;
	return false;
}

const OSystem::GraphicsMode* OSystem_N64::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_N64::getDefaultGraphicsMode() const {
	return OVERS_NTSC_340X240;
}

bool OSystem_N64::setGraphicsMode(const char *mode) {
	int i = 0;
	while (s_supportedGraphicsModes[i].name) {
		if (!strcmpi(s_supportedGraphicsModes[i].name, mode)) {
			_graphicMode = s_supportedGraphicsModes[i].id;

			switchGraphicModeId(_graphicMode);

			return true;
		}
		i++;
	}

	return true;
}

bool OSystem_N64::setGraphicsMode(int mode) {
	_graphicMode = mode;
	switchGraphicModeId(_graphicMode);

	return true;
}

void OSystem_N64::switchGraphicModeId(int mode) {
	switch (mode) {
	case NORM_PAL_320X240:
		disableAudioPlayback();
		_viClockRate = VI_PAL_CLOCK;
		_maxFps = N64_PAL_FPS;
		initDisplay(PAL_320X240_16BIT);
		_frameBufferWidth = 320;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 0;
		_graphicMode = NORM_PAL_320X240;
		enableAudioPlayback();
		break;

	case OVERS_PAL_340X240:
		disableAudioPlayback();
		_viClockRate = VI_PAL_CLOCK;
		_maxFps = N64_PAL_FPS;
		initDisplay(PAL_340X240_16BIT);
		_frameBufferWidth = 340;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 16;
		_graphicMode = OVERS_PAL_340X240;
		enableAudioPlayback();
		break;

	case NORM_MPAL_320X240:
		disableAudioPlayback();
		_viClockRate = VI_MPAL_CLOCK;
		_maxFps = N64_NTSC_FPS;
		initDisplay(MPAL_320X240_16BIT);
		_frameBufferWidth = 320;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 0;
		_graphicMode = NORM_MPAL_320X240;
		enableAudioPlayback();
		break;

	case OVERS_MPAL_340X240:
		disableAudioPlayback();
		_viClockRate = VI_MPAL_CLOCK;
		_maxFps = N64_NTSC_FPS;
		initDisplay(MPAL_340X240_16BIT);
		_frameBufferWidth = 340;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 16;
		_graphicMode = OVERS_MPAL_340X240;
		enableAudioPlayback();
		break;

	case NORM_NTSC_320X240:
		disableAudioPlayback();
		_viClockRate = VI_NTSC_CLOCK;
		_maxFps = N64_NTSC_FPS;
		initDisplay(NTSC_320X240_16BIT);
		_frameBufferWidth = 320;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 0;
		_graphicMode = NORM_NTSC_320X240;
		enableAudioPlayback();
		break;

	case OVERS_NTSC_340X240:
	default:
		disableAudioPlayback();
		_viClockRate = VI_NTSC_CLOCK;
		_maxFps = N64_NTSC_FPS;
		initDisplay(NTSC_340X240_16BIT);
		_frameBufferWidth = 340;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 16;
		_graphicMode = OVERS_NTSC_340X240;
		enableAudioPlayback();
		break;
	}
}

int OSystem_N64::getGraphicsMode() const {
	return _graphicMode;
}

void OSystem_N64::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	_gameWidth = width;
	_gameHeight = height;

	if (_gameWidth > _screenWidth)
		_gameWidth = _screenWidth;
	if (_gameHeight > _screenHeight)
		_gameHeight = _screenHeight;

	_mouseMaxX = _gameWidth;
	_mouseMaxY = _gameHeight;
}

int16 OSystem_N64::getHeight() {
	return _screenHeight;
}

int16 OSystem_N64::getWidth() {
	return _screenWidth;
}

void OSystem_N64::setPalette(const byte *colors, uint start, uint num) {
#ifndef N64_EXTREME_MEMORY_SAVING
	memcpy(_screenExactPalette + start * 3, colors, num * 3);
#endif

	for (uint i = 0; i < num; ++i) {
		_screenPalette[start + i] = colRGB888toBGR555(colors[2], colors[1], colors[0]);
		colors += 3;
	}

	// If cursor uses the game palette, we need to rebuild the hicolor buffer
	if (_cursorPaletteDisabled)
		rebuildOffscreenMouseBuffer();

	_dirtyPalette = true;
	_dirtyOffscreen = true;
}

void OSystem_N64::rebuildOffscreenGameBuffer(void) {
	// Regenerate hi-color offscreen buffer
	uint64 four_col_hi;
	uint32 four_col_pal;

	for (int h = 0; h < _gameHeight; h++) {
		for (int w = 0; w < _gameWidth; w += 4) {
			four_col_pal = *(uint32 *)(_offscreen_pal + ((h * _screenWidth) + w));

			four_col_hi = 0;
			four_col_hi |= (uint64)_screenPalette[((four_col_pal >> 24) & 0xFF)] << 48;
			four_col_hi |= (uint64)_screenPalette[((four_col_pal >> 16) & 0xFF)] << 32;
			four_col_hi |= (uint64)_screenPalette[((four_col_pal >>  8) & 0xFF)] << 16;
			four_col_hi |= (uint64)_screenPalette[((four_col_pal >>  0) & 0xFF)] <<  0;

			// Save the converted pixels into hicolor buffer
			*(uint64 *)((_offscreen_hic) + (h * _screenWidth) + w) = four_col_hi;
		}
	}
}

void OSystem_N64::rebuildOffscreenMouseBuffer(void) {
	uint16 width, height;
	uint16 *_pal_src = _cursorPaletteDisabled ? _screenPalette : _cursorPalette;

	for (height = 0; height < _cursorHeight; height++) {
		for (width = 0; width < _cursorWidth; width++) {
			uint8 pix = _cursor_pal[(_cursorWidth * height) + width];

			// Enable alpha bit in cursor buffer if pixel should be invisible
			_cursor_hic[(_cursorWidth * height) + width] = (pix != _cursorKeycolor) ? _pal_src[pix] : 0x0001;
		}
	}
}

void OSystem_N64::grabPalette(byte *colors, uint start, uint num) {
#ifdef N64_EXTREME_MEMORY_SAVING  // This way loses precisions
	uint32 i;
	uint16 color;

	for (i = start; i < start + num; i++) {
		color = _screenPalette[i];

		// Color format on the n64 is RGB - 1555
		*colors++ = ((color & 0x1F) << 3);
		*colors++ = (((color >> 5) & 0x1F) << 3);
		*colors++ = (((color >> 10) & 0x1F) << 3);
	}
#else
	memcpy(colors, _screenExactPalette + start * 3, num * 3);
#endif

	return;
}

void OSystem_N64::setCursorPalette(const byte *colors, uint start, uint num) {
	for (uint i = 0; i < num; ++i) {
		_cursorPalette[start + i] = colRGB888toBGR555(colors[2], colors[1], colors[0]);
		colors += 3;
	}

	_cursorPaletteDisabled = false;

	// Rebuild cursor hicolor buffer
	rebuildOffscreenMouseBuffer();

	_dirtyOffscreen = true;
}

void OSystem_N64::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	//Clip the coordinates
	const byte *src = (const byte *)buf;
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	uint8 *dst_pal = _offscreen_pal + ((y * _screenWidth) + x);
	uint16 *dst_hicol = _offscreen_hic + ((y * _screenWidth) + x);

	do {
		for (int hor = 0; hor < w; hor++) {
			if (dst_pal[hor] != src[hor]) {
				uint16 color = _screenPalette[src[hor]];
				dst_hicol[hor] = color;  // Save image converted to 16-bit
				dst_pal[hor] = src[hor]; // Save palettized display
			}
		}

		src += pitch;
		dst_pal += _screenWidth;
		dst_hicol += _screenWidth;
	} while (--h);

	_dirtyOffscreen = true;

	return;
}

void OSystem_N64::updateScreen() {
#ifdef LIMIT_FPS
	static uint32 _lastScreenUpdate = 0;
	if (!_disableFpsLimit) {
		uint32 now = getMillis();
		if (now - _lastScreenUpdate < 1000 / _maxFps)
			return;

		_lastScreenUpdate = now;
	}
#endif

	// Check if audio buffer needs refill
	// Done here because this gets called regularly
	refillAudioBuffers();

	if (!_dirtyOffscreen && !_dirtyPalette) return; // The offscreen is clean

	uint8 skip_lines = (_screenHeight - _gameHeight) / 4;
	uint8 skip_pixels = (_screenWidth - _gameWidth) / 2; // Center horizontally the image
	skip_pixels -= (skip_pixels % 8); // To keep aligned memory access

	if (_dirtyPalette)
		rebuildOffscreenGameBuffer();

	// Obtain the framebuffer
	while (!(_dc = lockDisplay()));

	uint16 *overlay_framebuffer = (uint16 *)_dc->conf.framebuffer; // Current screen framebuffer
	uint16 *game_framebuffer = overlay_framebuffer + (_frameBufferWidth * skip_lines * 2); // Skip some lines to center the image vertically

	uint16 currentHeight, currentWidth;
	uint16 *tmpDst;
	uint16 *tmpSrc;

	// Copy the game buffer to screen
	if (!_overlayVisible) {
		tmpDst = game_framebuffer;
		tmpSrc = _offscreen_hic + (_shakeOffset * _screenWidth);
		for (currentHeight = _shakeOffset; currentHeight < _gameHeight; currentHeight++) {
			uint64 *game_dest = (uint64 *)(tmpDst + skip_pixels + _offscrPixels);
			uint64 *game_src = (uint64 *)tmpSrc;

			// With uint64 we copy 4 pixels at a time
			for (currentWidth = 0; currentWidth < _gameWidth; currentWidth += 4) {
				*game_dest++ = *game_src++;
			}
			tmpDst += _frameBufferWidth;
			tmpSrc += _screenWidth;
		}

		uint16 _clearLines = _shakeOffset; // When shaking we must take care of remaining lines to clear
		while (_clearLines--) {
			memset(tmpDst + skip_pixels + _offscrPixels, 0, _screenWidth * 2);
			tmpDst += _frameBufferWidth;
		}
	} else { // If the overlay is enabled, draw it on top of game screen
		tmpDst = overlay_framebuffer;
		tmpSrc = _overlayBuffer;
		for (currentHeight = 0; currentHeight < _overlayHeight; currentHeight++) {
			uint64 *over_dest = (uint64 *)(tmpDst + _offscrPixels);
			uint64 *over_src = (uint64 *)tmpSrc;

			// Copy 4 pixels at a time
			for (currentWidth = 0; currentWidth < _overlayWidth; currentWidth += 4) {
				*over_dest++ = *over_src++;
			}
			tmpDst += _frameBufferWidth;
			tmpSrc += _overlayWidth;
		}
	}

	// Draw mouse cursor
	if ((_mouseVisible || _overlayVisible) && _cursorHeight > 0 && _cursorWidth > 0) {
		uint16 *mouse_framebuffer;
		uint16 horiz_pix_skip;

		if (_overlayVisible) {
			mouse_framebuffer = overlay_framebuffer;
			horiz_pix_skip = 0;
		} else {
			mouse_framebuffer = game_framebuffer;
			horiz_pix_skip = skip_pixels;
		}

		for (uint h = 0; h < _cursorHeight; h++) {
			for (uint w = 0; w < _cursorWidth; w++) {
				int posX = (_mouseX - _mouseHotspotX) + w;
				int posY = (_mouseY - _mouseHotspotY) + h;

				// Draw pixel
				if ((posY >= 0) && (posY < _mouseMaxY) && (posX >= 0) && (posX < _mouseMaxX)) {
					uint16 cursor_pixel_hic = _cursor_hic[(h * _cursorWidth) + w];

					if (!(cursor_pixel_hic & 0x00001))
						mouse_framebuffer[(posY * _frameBufferWidth) + (posX + _offscrPixels + horiz_pix_skip)] = cursor_pixel_hic;
				}
			}
		}
	}

#ifndef _ENABLE_DEBUG_
	showDisplay(_dc);
#else
	showDisplayAndText(_dc);
#endif

	_dc = NULL;
	_dirtyOffscreen = false;
	_dirtyPalette = false;

	return;
}

Graphics::Surface *OSystem_N64::lockScreen() {
	_framebuffer.pixels = _offscreen_pal;
	_framebuffer.w = _gameWidth;
	_framebuffer.h = _gameHeight;
	_framebuffer.pitch = _screenWidth;
	_framebuffer.format = Graphics::PixelFormat::createFormatCLUT8();

	return &_framebuffer;
}

void OSystem_N64::unlockScreen() {
	_dirtyPalette = true;
	_dirtyOffscreen = true;
}

void OSystem_N64::setShakePos(int shakeOffset) {

	// If a rumble pak is plugged in and screen shakes, rumble!
	if (shakeOffset && _controllerHasRumble) rumblePakEnable(1, _controllerPort);
	else if (!shakeOffset && _controllerHasRumble) rumblePakEnable(0, _controllerPort);

	_shakeOffset = shakeOffset;
	_dirtyOffscreen = true;

	return;
}

void OSystem_N64::showOverlay() {
	// Change min/max mouse coords
	_mouseMaxX = _overlayWidth;
	_mouseMaxY = _overlayHeight;

	// Relocate the mouse cursor given the new limitations
	warpMouse(_mouseX, _mouseY);

	_overlayVisible = true;
	_dirtyOffscreen = true;
}

void OSystem_N64::hideOverlay() {
	// Change min/max mouse coords
	_mouseMaxX = _gameWidth;
	_mouseMaxY = _gameHeight;

	// Relocate the mouse cursor given the new limitations
	warpMouse(_mouseX, _mouseY);

	_overlayVisible = false;

	// Clear double buffered display
	clearAllVideoBuffers();

	_dirtyOffscreen = true;

	// Force TWO screen updates (because of double buffered display).
	// This way games which won't automatically update the screen
	// when overlay is disabled, won't show a black screen. (eg. Lure)
	_disableFpsLimit = true;
	updateScreen();
	updateScreen();
	_disableFpsLimit = false;
}

void OSystem_N64::clearOverlay() {
	memset(_overlayBuffer, 0, _overlayWidth * _overlayHeight * sizeof(uint16));

	uint8 skip_lines = (_screenHeight - _gameHeight) / 4;
	uint8 skip_pixels = (_screenWidth - _gameWidth) / 2; // Center horizontally the image

	uint16 *tmpDst = _overlayBuffer + (_overlayWidth * skip_lines * 2);
	uint16 *tmpSrc = _offscreen_hic + (_shakeOffset * _screenWidth);
	for (uint16 currentHeight = _shakeOffset; currentHeight < _gameHeight; currentHeight++) {
		memcpy((tmpDst + skip_pixels), tmpSrc, _gameWidth * 2);
		tmpDst += _overlayWidth;
		tmpSrc += _screenWidth;
	}

	_dirtyOffscreen = true;
}

void OSystem_N64::grabOverlay(void *buf, int pitch) {
	int h = _overlayHeight;
	uint16 *src = _overlayBuffer;
	byte *dst = (byte *)buf;

	do {
		memcpy(dst, src, _overlayWidth * sizeof(uint16));
		src += _overlayWidth;
		dst += pitch;
	} while (--h);
}

void OSystem_N64::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	const byte *src = (const byte *)buf;
	//Clip the coordinates
	if (x < 0) {
		w += x;
		src -= x * sizeof(uint16);
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > _overlayWidth - x) {
		w = _overlayWidth - x;
	}

	if (h > _overlayHeight - y) {
		h = _overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;


	uint16 *dst = _overlayBuffer + (y * _overlayWidth + x);

	if (_overlayWidth == (uint16)w && (uint16)pitch == _overlayWidth * sizeof(uint16)) {
		memcpy(dst, src, h * pitch);
	} else {
		do {
			memcpy(dst, src, w * sizeof(uint16));
			src += pitch;
			dst += _overlayWidth;
		} while (--h);
	}

	_dirtyOffscreen = true;

	return;
}

int16 OSystem_N64::getOverlayHeight() {
	return _overlayHeight;
}

int16 OSystem_N64::getOverlayWidth() {
	return _overlayWidth;
}


bool OSystem_N64::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;

	_dirtyOffscreen = true;

	return last;
}

void OSystem_N64::warpMouse(int x, int y) {

	if (x < 0)
		_mouseX = 0;
	else if (x >= _mouseMaxX)
		_mouseX = _mouseMaxX - 1;
	else
		_mouseX = x;

	if (y < 0)
		_mouseY = 0;
	else if (y >= _mouseMaxY)
		_mouseY = _mouseMaxY - 1;
	else
		_mouseY = y;

	_dirtyOffscreen = true;
}

void OSystem_N64::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	if (!w || !h) return;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	if (_cursor_pal && ((w != _cursorWidth) || (h != _cursorHeight))) {
		free(_cursor_pal);
		free(_cursor_hic);
		_cursor_pal = NULL;
		_cursor_hic = NULL;
	}

	if (!_cursor_pal) {
		_cursor_pal = (uint8 *)malloc(w * h);
		_cursor_hic = (uint16 *)malloc(w * h * sizeof(uint16));
	}

	_cursorWidth = w;
	_cursorHeight = h;

	memcpy(_cursor_pal, buf, w * h); // Copy the palettized cursor

	_cursorKeycolor = keycolor & 0xFF;

	// Regenerate cursor hicolor buffer
	rebuildOffscreenMouseBuffer();

	_dirtyOffscreen = true;

	return;
}

uint32 OSystem_N64::getMillis() {
	return getMilliTick();
}

void OSystem_N64::delayMillis(uint msecs) {
	// In some cases a game might hang waiting for audio being
	// played. This is a workaround for all the situations i
	// found (kyra 1 & 2 DOS).

	uint32 oldTime = getMilliTick();
	refillAudioBuffers();
	uint32 pastMillis = (getMilliTick() - oldTime);

	if (pastMillis >= msecs) {
		return;
	} else {
		delay(msecs - pastMillis);
	}
}

// As we don't have multi-threading, no need for mutexes
OSystem::MutexRef OSystem_N64::createMutex(void) {
	return NULL;
}

void OSystem_N64::lockMutex(MutexRef mutex) {
	return;
}

void OSystem_N64::unlockMutex(MutexRef mutex) {
	return;
}

void OSystem_N64::deleteMutex(MutexRef mutex) {
	return;
}

void OSystem_N64::quit() {
	// Not much to do...
	return;
}

Audio::Mixer *OSystem_N64::getMixer() {
	assert(_mixer);
	return _mixer;
}

void OSystem_N64::getTimeAndDate(TimeDate &t) const {
	// No RTC inside the N64, read mips timer to simulate
	// passing of time, not a perfect solution, but can't think
	// of anything better.

	uint32 now = getMilliTick();

	t.tm_sec  = (now / 1000) % 60;
	t.tm_min  = ((now / 1000) / 60) % 60;
	t.tm_hour = (((now / 1000) / 60) / 60) % 24;
	t.tm_mday = 1;
	t.tm_mon  = 0;
	t.tm_year = 110;
	t.tm_wday = 0;

	return;
}

void OSystem_N64::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);
}

void OSystem_N64::setTimerCallback(TimerProc callback, int interval) {
	assert (interval > 0);

	if (callback != NULL) {
		_timerCallbackTimer = interval;
		_timerCallbackNext = getMillis() + interval;
		_timerCallback = callback;
	} else
		_timerCallback = NULL;
}

void OSystem_N64::setupMixer(void) {
	_mixer = new Audio::MixerImpl(this, DEFAULT_SOUND_SAMPLE_RATE);
	_mixer->setReady(false);

	enableAudioPlayback();
}

/* Check all controller ports for a compatible input adapter. */
void OSystem_N64::detectControllers(void) {
	controller_data_status *ctrl_status = (controller_data_status *)memalign(8, sizeof(controller_data_status));
	controller_Read_Status(ctrl_status);

	_controllerPort = -1; // Default no controller
	_mousePort = -1; // Default no mouse
	for (int8 ctrl_port = 3; ctrl_port >= 0; ctrl_port--) {
		// Found a standard pad, use this by default.
		if (ctrl_status->c[ctrl_port].type == CTRL_PAD_STANDARD) {
			_controllerPort = ctrl_port;
		} else if (ctrl_status->c[ctrl_port].type == CTRL_N64_MOUSE) {
			_mousePort = ctrl_port;
		}
	}

	free(ctrl_status);
}

inline uint16 colRGB888toBGR555(byte r, byte g, byte b) {
	return ((r >> 3) << 1) | ((g >> 3) << 6) | ((b >> 3) << 11);
}
