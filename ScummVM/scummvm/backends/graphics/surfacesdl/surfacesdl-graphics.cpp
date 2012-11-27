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

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "common/config-manager.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/util.h"
#ifdef USE_RGB_COLOR
#include "common/list.h"
#endif
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/scaler/aspect.h"
#include "graphics/surface.h"

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", _s("Normal (no scaling)"), GFX_NORMAL},
#ifdef USE_SCALERS
	{"2x", "2x", GFX_DOUBLESIZE},
	{"3x", "3x", GFX_TRIPLESIZE},
	{"2xsai", "2xSAI", GFX_2XSAI},
	{"super2xsai", "Super2xSAI", GFX_SUPER2XSAI},
	{"supereagle", "SuperEagle", GFX_SUPEREAGLE},
	{"advmame2x", "AdvMAME2x", GFX_ADVMAME2X},
	{"advmame3x", "AdvMAME3x", GFX_ADVMAME3X},
#ifdef USE_HQ_SCALERS
	{"hq2x", "HQ2x", GFX_HQ2X},
	{"hq3x", "HQ3x", GFX_HQ3X},
#endif
	{"tv2x", "TV2x", GFX_TV2X},
	{"dotmatrix", "DotMatrix", GFX_DOTMATRIX},
#endif
	{0, 0, 0}
};

DECLARE_TRANSLATION_ADDITIONAL_CONTEXT("Normal (no scaling)", "lowres")

// Table of the cursor scalers [scaleFactor - 1]
static ScalerProc *scalersMagn[3] = {
#ifdef USE_SCALERS
	Normal1x, AdvMame2x, AdvMame3x
#else // remove dependencies on other scalers
	Normal1x, Normal1x, Normal1x
#endif
};

static const int s_gfxModeSwitchTable[][4] = {
		{ GFX_NORMAL, GFX_DOUBLESIZE, GFX_TRIPLESIZE, -1 },
		{ GFX_NORMAL, GFX_ADVMAME2X, GFX_ADVMAME3X, -1 },
		{ GFX_NORMAL, GFX_HQ2X, GFX_HQ3X, -1 },
		{ GFX_NORMAL, GFX_2XSAI, -1, -1 },
		{ GFX_NORMAL, GFX_SUPER2XSAI, -1, -1 },
		{ GFX_NORMAL, GFX_SUPEREAGLE, -1, -1 },
		{ GFX_NORMAL, GFX_TV2X, -1, -1 },
		{ GFX_NORMAL, GFX_DOTMATRIX, -1, -1 }
	};

#ifdef USE_SCALERS
static int cursorStretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY);
#endif

AspectRatio::AspectRatio(int w, int h) {
	// TODO : Validation and so on...
	// Currently, we just ensure the program don't instantiate non-supported aspect ratios
	_kw = w;
	_kh = h;
}

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__) && defined(USE_SCALERS)
static AspectRatio getDesiredAspectRatio() {
	const size_t AR_COUNT = 4;
	const char *desiredAspectRatioAsStrings[AR_COUNT] = {	"auto",				"4/3",				"16/9",				"16/10" };
	const AspectRatio desiredAspectRatios[AR_COUNT] = {		AspectRatio(0, 0),	AspectRatio(4,3),	AspectRatio(16,9),	AspectRatio(16,10) };

	//TODO : We could parse an arbitrary string, if we code enough proper validation
	Common::String desiredAspectRatio = ConfMan.get("desired_screen_aspect_ratio");

	for (size_t i = 0; i < AR_COUNT; i++) {
		assert(desiredAspectRatioAsStrings[i] != NULL);

		if (!scumm_stricmp(desiredAspectRatio.c_str(), desiredAspectRatioAsStrings[i])) {
			return desiredAspectRatios[i];
		}
	}
	// TODO : Report a warning
	return AspectRatio(0, 0);
}
#endif

SurfaceSdlGraphicsManager::SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource)
	:
	SdlGraphicsManager(sdlEventSource),
#ifdef USE_OSD
	_osdSurface(0), _osdAlpha(SDL_ALPHA_TRANSPARENT), _osdFadeStartTime(0),
#endif
	_hwscreen(0), _screen(0), _tmpscreen(0),
#ifdef USE_RGB_COLOR
	_screenFormat(Graphics::PixelFormat::createFormatCLUT8()),
	_cursorFormat(Graphics::PixelFormat::createFormatCLUT8()),
#endif
	_overlayVisible(false),
	_overlayscreen(0), _tmpscreen2(0),
	_scalerProc(0), _screenChangeCount(0),
	_mouseVisible(false), _mouseNeedsRedraw(false), _mouseData(0), _mouseSurface(0),
	_mouseOrigSurface(0), _cursorDontScale(false), _cursorPaletteDisabled(true),
	_currentShakePos(0), _newShakePos(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0),
	_screenIsLocked(false),
	_graphicsMutex(0),
#ifdef USE_SDL_DEBUG_FOCUSRECT
	_enableFocusRectDebugCode(false), _enableFocusRect(false), _focusRect(),
#endif
	_transactionMode(kTransactionNone) {

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	// This is also called in initSDL(), but initializing graphics
	// may reset it.
	SDL_EnableUNICODE(1);

	// allocate palette storage
	_currentPalette = (SDL_Color *)calloc(sizeof(SDL_Color), 256);
	_cursorPalette = (SDL_Color *)calloc(sizeof(SDL_Color), 256);

	_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;

	memset(&_mouseCurState, 0, sizeof(_mouseCurState));

	_graphicsMutex = g_system->createMutex();

#ifdef USE_SDL_DEBUG_FOCUSRECT
	if (ConfMan.hasKey("use_sdl_debug_focusrect"))
		_enableFocusRectDebugCode = ConfMan.getBool("use_sdl_debug_focusrect");
#endif

	SDL_ShowCursor(SDL_DISABLE);

	memset(&_oldVideoMode, 0, sizeof(_oldVideoMode));
	memset(&_videoMode, 0, sizeof(_videoMode));
	memset(&_transactionDetails, 0, sizeof(_transactionDetails));

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__) && defined(USE_SCALERS)
	_videoMode.mode = GFX_DOUBLESIZE;
	_videoMode.scaleFactor = 2;
	_videoMode.aspectRatioCorrection = ConfMan.getBool("aspect_ratio");
	_videoMode.desiredAspectRatio = getDesiredAspectRatio();
	_scalerProc = Normal2x;
#else // for small screen platforms
	_videoMode.mode = GFX_NORMAL;
	_videoMode.scaleFactor = 1;
	_videoMode.aspectRatioCorrection = false;
	_scalerProc = Normal1x;
#endif
	_scalerType = 0;

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	_videoMode.fullscreen = ConfMan.getBool("fullscreen");
#else
	_videoMode.fullscreen = true;
#endif
}

SurfaceSdlGraphicsManager::~SurfaceSdlGraphicsManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher() != NULL)
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	unloadGFXMode();
	if (_mouseSurface)
		SDL_FreeSurface(_mouseSurface);
	_mouseSurface = 0;
	if (_mouseOrigSurface)
		SDL_FreeSurface(_mouseOrigSurface);
	_mouseOrigSurface = 0;
	g_system->deleteMutex(_graphicsMutex);

	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);
}

void SurfaceSdlGraphicsManager::initEventObserver() {
	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

bool SurfaceSdlGraphicsManager::hasFeature(OSystem::Feature f) {
	return
		(f == OSystem::kFeatureFullscreenMode) ||
		(f == OSystem::kFeatureAspectRatioCorrection) ||
		(f == OSystem::kFeatureCursorPalette) ||
		(f == OSystem::kFeatureIconifyWindow);
}

void SurfaceSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		setFullscreenMode(enable);
		break;
	case OSystem::kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable);
		break;
	case OSystem::kFeatureCursorPalette:
		_cursorPaletteDisabled = !enable;
		blitCursor();
		break;
	case OSystem::kFeatureIconifyWindow:
		if (enable)
			SDL_WM_IconifyWindow();
		break;
	default:
		break;
	}
}

bool SurfaceSdlGraphicsManager::getFeatureState(OSystem::Feature f) {
	// We need to allow this to be called from within a transaction, since we
	// currently use it to retreive the graphics state, when switching from
	// SDL->OpenGL mode for example.
	//assert(_transactionMode == kTransactionNone);

	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return _videoMode.fullscreen;
	case OSystem::kFeatureAspectRatioCorrection:
		return _videoMode.aspectRatioCorrection;
	case OSystem::kFeatureCursorPalette:
		return !_cursorPaletteDisabled;
	default:
		return false;
	}
}

const OSystem::GraphicsMode *SurfaceSdlGraphicsManager::supportedGraphicsModes() {
	return s_supportedGraphicsModes;
}

const OSystem::GraphicsMode *SurfaceSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int SurfaceSdlGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_DOUBLESIZE;
}

void SurfaceSdlGraphicsManager::resetGraphicsScale() {
	setGraphicsMode(s_gfxModeSwitchTable[_scalerType][0]);
}

void SurfaceSdlGraphicsManager::beginGFXTransaction() {
	assert(_transactionMode == kTransactionNone);

	_transactionMode = kTransactionActive;

	_transactionDetails.sizeChanged = false;

	_transactionDetails.needHotswap = false;
	_transactionDetails.needUpdatescreen = false;

	_transactionDetails.normal1xScaler = false;
#ifdef USE_RGB_COLOR
	_transactionDetails.formatChanged = false;
#endif

	_oldVideoMode = _videoMode;
}

OSystem::TransactionError SurfaceSdlGraphicsManager::endGFXTransaction() {
	int errors = OSystem::kTransactionSuccess;

	assert(_transactionMode != kTransactionNone);

	if (_transactionMode == kTransactionRollback) {
		if (_videoMode.fullscreen != _oldVideoMode.fullscreen) {
			errors |= OSystem::kTransactionFullscreenFailed;

			_videoMode.fullscreen = _oldVideoMode.fullscreen;
		} else if (_videoMode.aspectRatioCorrection != _oldVideoMode.aspectRatioCorrection) {
			errors |= OSystem::kTransactionAspectRatioFailed;

			_videoMode.aspectRatioCorrection = _oldVideoMode.aspectRatioCorrection;
		} else if (_videoMode.mode != _oldVideoMode.mode) {
			errors |= OSystem::kTransactionModeSwitchFailed;

			_videoMode.mode = _oldVideoMode.mode;
			_videoMode.scaleFactor = _oldVideoMode.scaleFactor;
#ifdef USE_RGB_COLOR
		} else if (_videoMode.format != _oldVideoMode.format) {
			errors |= OSystem::kTransactionFormatNotSupported;

			_videoMode.format = _oldVideoMode.format;
			_screenFormat = _videoMode.format;
#endif
		} else if (_videoMode.screenWidth != _oldVideoMode.screenWidth || _videoMode.screenHeight != _oldVideoMode.screenHeight) {
			errors |= OSystem::kTransactionSizeChangeFailed;

			_videoMode.screenWidth = _oldVideoMode.screenWidth;
			_videoMode.screenHeight = _oldVideoMode.screenHeight;
			_videoMode.overlayWidth = _oldVideoMode.overlayWidth;
			_videoMode.overlayHeight = _oldVideoMode.overlayHeight;
		}

		if (_videoMode.fullscreen == _oldVideoMode.fullscreen &&
			_videoMode.aspectRatioCorrection == _oldVideoMode.aspectRatioCorrection &&
			_videoMode.mode == _oldVideoMode.mode &&
			_videoMode.screenWidth == _oldVideoMode.screenWidth &&
			_videoMode.screenHeight == _oldVideoMode.screenHeight) {

			// Our new video mode would now be exactly the same as the
			// old one. Since we still can not assume SDL_SetVideoMode
			// to be working fine, we need to invalidate the old video
			// mode, so loadGFXMode would error out properly.
			_oldVideoMode.setup = false;
		}
	}

#ifdef USE_RGB_COLOR
	if (_transactionDetails.sizeChanged || _transactionDetails.formatChanged) {
#else
	if (_transactionDetails.sizeChanged) {
#endif
		unloadGFXMode();
		if (!loadGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			setGraphicsModeIntern();
			clearOverlay();

			_videoMode.setup = true;
			// OSystem_SDL::pollEvent used to update the screen change count,
			// but actually it gives problems when a video mode was changed
			// but OSystem_SDL::pollEvent was not called. This for example
			// caused a crash under certain circumstances when doing an RTL.
			// To fix this issue we update the screen change count right here.
			_screenChangeCount++;
		}
	} else if (_transactionDetails.needHotswap) {
		setGraphicsModeIntern();
		if (!hotswapGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			_videoMode.setup = true;
			// OSystem_SDL::pollEvent used to update the screen change count,
			// but actually it gives problems when a video mode was changed
			// but OSystem_SDL::pollEvent was not called. This for example
			// caused a crash under certain circumstances when doing an RTL.
			// To fix this issue we update the screen change count right here.
			_screenChangeCount++;

			if (_transactionDetails.needUpdatescreen)
				internUpdateScreen();
		}
	} else if (_transactionDetails.needUpdatescreen) {
		setGraphicsModeIntern();
		internUpdateScreen();
	}

	_transactionMode = kTransactionNone;
	return (OSystem::TransactionError)errors;
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> SurfaceSdlGraphicsManager::getSupportedFormats() const {
	assert(!_supportedFormats.empty());
	return _supportedFormats;
}

void SurfaceSdlGraphicsManager::detectSupportedFormats() {

	// Clear old list
	_supportedFormats.clear();

	// Some tables with standard formats that we always list
	// as "supported". If frontend code tries to use one of
	// these, we will perform the necessary format
	// conversion in the background. Of course this incurs a
	// performance hit, but on desktop ports this should not
	// matter. We still push the currently active format to
	// the front, so if frontend code just uses the first
	// available format, it will get one that is "cheap" to
	// use.
	const Graphics::PixelFormat RGBList[] = {
#ifdef USE_RGB_COLOR
		// RGBA8888, ARGB8888, RGB888
		Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0),
		Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24),
		Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0),
#endif
		// RGB565, XRGB1555, RGB555, RGBA4444, ARGB4444
		Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
		Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15),
		Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),
		Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0),
		Graphics::PixelFormat(2, 4, 4, 4, 4, 8, 4, 0, 12)
	};
	const Graphics::PixelFormat BGRList[] = {
#ifdef USE_RGB_COLOR
		// ABGR8888, BGRA8888, BGR888
		Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24),
		Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0),
		Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0),
#endif
		// BGR565, XBGR1555, BGR555, ABGR4444, BGRA4444
		Graphics::PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0),
		Graphics::PixelFormat(2, 5, 5, 5, 1, 0, 5, 10, 15),
		Graphics::PixelFormat(2, 5, 5, 5, 0, 0, 5, 10, 0),
		Graphics::PixelFormat(2, 4, 4, 4, 4, 0, 4, 8, 12),
		Graphics::PixelFormat(2, 4, 4, 4, 4, 4, 8, 12, 0)
	};

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	if (_hwscreen) {
		// Get our currently set hardware format
		format = Graphics::PixelFormat(_hwscreen->format->BytesPerPixel,
			8 - _hwscreen->format->Rloss, 8 - _hwscreen->format->Gloss,
			8 - _hwscreen->format->Bloss, 8 - _hwscreen->format->Aloss,
			_hwscreen->format->Rshift, _hwscreen->format->Gshift,
			_hwscreen->format->Bshift, _hwscreen->format->Ashift);

		// Workaround to SDL not providing an accurate Aloss value on Mac OS X.
		if (_hwscreen->format->Amask == 0)
			format.aLoss = 8;

		// Push it first, as the prefered format.
		_supportedFormats.push_back(format);
	}

	// TODO: prioritize matching alpha masks
	int i;

	// Push some RGB formats
	for (i = 0; i < ARRAYSIZE(RGBList); i++) {
		if (_hwscreen && (RGBList[i].bytesPerPixel > format.bytesPerPixel))
			continue;
		if (RGBList[i] != format)
			_supportedFormats.push_back(RGBList[i]);
	}

	// Push some BGR formats
	for (i = 0; i < ARRAYSIZE(BGRList); i++) {
		if (_hwscreen && (BGRList[i].bytesPerPixel > format.bytesPerPixel))
			continue;
		if (BGRList[i] != format)
			_supportedFormats.push_back(BGRList[i]);
	}

	// Finally, we always supposed 8 bit palette graphics
	_supportedFormats.push_back(Graphics::PixelFormat::createFormatCLUT8());
}
#endif

bool SurfaceSdlGraphicsManager::setGraphicsMode(int mode) {
	Common::StackLock lock(_graphicsMutex);

	assert(_transactionMode == kTransactionActive);

	if (_oldVideoMode.setup && _oldVideoMode.mode == mode)
		return true;

	int newScaleFactor = 1;

	switch (mode) {
	case GFX_NORMAL:
		newScaleFactor = 1;
		break;
#ifdef USE_SCALERS
	case GFX_DOUBLESIZE:
		newScaleFactor = 2;
		break;
	case GFX_TRIPLESIZE:
		newScaleFactor = 3;
		break;

	case GFX_2XSAI:
		newScaleFactor = 2;
		break;
	case GFX_SUPER2XSAI:
		newScaleFactor = 2;
		break;
	case GFX_SUPEREAGLE:
		newScaleFactor = 2;
		break;
	case GFX_ADVMAME2X:
		newScaleFactor = 2;
		break;
	case GFX_ADVMAME3X:
		newScaleFactor = 3;
		break;
#ifdef USE_HQ_SCALERS
	case GFX_HQ2X:
		newScaleFactor = 2;
		break;
	case GFX_HQ3X:
		newScaleFactor = 3;
		break;
#endif
	case GFX_TV2X:
		newScaleFactor = 2;
		break;
	case GFX_DOTMATRIX:
		newScaleFactor = 2;
		break;
#endif // USE_SCALERS

	default:
		warning("unknown gfx mode %d", mode);
		return false;
	}

	_transactionDetails.normal1xScaler = (mode == GFX_NORMAL);
	if (_oldVideoMode.setup && _oldVideoMode.scaleFactor != newScaleFactor)
		_transactionDetails.needHotswap = true;

	_transactionDetails.needUpdatescreen = true;

	_videoMode.mode = mode;
	_videoMode.scaleFactor = newScaleFactor;

	return true;
}

void SurfaceSdlGraphicsManager::setGraphicsModeIntern() {
	Common::StackLock lock(_graphicsMutex);
	ScalerProc *newScalerProc = 0;

	switch (_videoMode.mode) {
	case GFX_NORMAL:
		newScalerProc = Normal1x;
		break;
#ifdef USE_SCALERS
	case GFX_DOUBLESIZE:
		newScalerProc = Normal2x;
		break;
	case GFX_TRIPLESIZE:
		newScalerProc = Normal3x;
		break;

	case GFX_2XSAI:
		newScalerProc = _2xSaI;
		break;
	case GFX_SUPER2XSAI:
		newScalerProc = Super2xSaI;
		break;
	case GFX_SUPEREAGLE:
		newScalerProc = SuperEagle;
		break;
	case GFX_ADVMAME2X:
		newScalerProc = AdvMame2x;
		break;
	case GFX_ADVMAME3X:
		newScalerProc = AdvMame3x;
		break;
#ifdef USE_HQ_SCALERS
	case GFX_HQ2X:
		newScalerProc = HQ2x;
		break;
	case GFX_HQ3X:
		newScalerProc = HQ3x;
		break;
#endif
	case GFX_TV2X:
		newScalerProc = TV2x;
		break;
	case GFX_DOTMATRIX:
		newScalerProc = DotMatrix;
		break;
#endif // USE_SCALERS

	default:
		error("Unknown gfx mode %d", _videoMode.mode);
	}

	_scalerProc = newScalerProc;

	if (_videoMode.mode != GFX_NORMAL) {
		for (int i = 0; i < ARRAYSIZE(s_gfxModeSwitchTable); i++) {
			if (s_gfxModeSwitchTable[i][1] == _videoMode.mode || s_gfxModeSwitchTable[i][2] == _videoMode.mode) {
				_scalerType = i;
				break;
			}
		}
	}

	if (!_screen || !_hwscreen)
		return;

	// Blit everything to the screen
	_forceFull = true;

	// Even if the old and new scale factors are the same, we may have a
	// different scaler for the cursor now.
	blitCursor();
}

int SurfaceSdlGraphicsManager::getGraphicsMode() const {
	assert(_transactionMode == kTransactionNone);
	return _videoMode.mode;
}

void SurfaceSdlGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	assert(_transactionMode == kTransactionActive);

#ifdef USE_RGB_COLOR
	//avoid redundant format changes
	Graphics::PixelFormat newFormat;
	if (!format)
		newFormat = Graphics::PixelFormat::createFormatCLUT8();
	else
		newFormat = *format;

	assert(newFormat.bytesPerPixel > 0);

	if (newFormat != _videoMode.format) {
		_videoMode.format = newFormat;
		_transactionDetails.formatChanged = true;
		_screenFormat = newFormat;
	}
#endif

	// Avoid redundant res changes
	if ((int)w == _videoMode.screenWidth && (int)h == _videoMode.screenHeight)
		return;

	_videoMode.screenWidth = w;
	_videoMode.screenHeight = h;

	_transactionDetails.sizeChanged = true;
}

int SurfaceSdlGraphicsManager::effectiveScreenHeight() const {
	return _videoMode.scaleFactor *
				(_videoMode.aspectRatioCorrection
					? real2Aspect(_videoMode.screenHeight)
					: _videoMode.screenHeight);
}

static void fixupResolutionForAspectRatio(AspectRatio desiredAspectRatio, int &width, int &height) {
	assert(&width != &height);

	if (desiredAspectRatio.isAuto())
		return;

	int kw = desiredAspectRatio.kw();
	int kh = desiredAspectRatio.kh();

	const int w = width;
	const int h = height;

	SDL_Rect const* const*availableModes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_SWSURFACE); //TODO : Maybe specify a pixel format
	assert(availableModes);

	const SDL_Rect *bestMode = NULL;
	uint bestMetric = (uint)-1; // Metric is wasted space
	while (const SDL_Rect *mode = *availableModes++) {
		if (mode->w < w)
			continue;
		if (mode->h < h)
			continue;
		if (mode->h * kw != mode->w * kh)
			continue;

		uint metric = mode->w * mode->h - w * h;
		if (metric > bestMetric)
			continue;

		bestMetric = metric;
		bestMode = mode;
	}

	if (!bestMode) {
		warning("Unable to enforce the desired aspect ratio");
		return;
	}
	width = bestMode->w;
	height = bestMode->h;
}

bool SurfaceSdlGraphicsManager::loadGFXMode() {
	_forceFull = true;

#if !defined(__MAEMO__) && !defined(DINGUX) && !defined(GPH_DEVICE) && !defined(LINUXMOTO) && !defined(OPENPANDORA)
	_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

	if (_videoMode.screenHeight != 200 && _videoMode.screenHeight != 400)
		_videoMode.aspectRatioCorrection = false;

	if (_videoMode.aspectRatioCorrection)
		_videoMode.overlayHeight = real2Aspect(_videoMode.overlayHeight);

	_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.hardwareHeight = effectiveScreenHeight();
// On GPH devices ALL the _videoMode.hardware... are setup in GPHGraphicsManager::loadGFXMode()
#elif !defined(GPH_DEVICE)
	_videoMode.hardwareWidth = _videoMode.overlayWidth;
	_videoMode.hardwareHeight = _videoMode.overlayHeight;
#endif

	//
	// Create the surface that contains the 8 bit game data
	//
#ifdef USE_RGB_COLOR
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight,
						_screenFormat.bytesPerPixel << 3,
						((1 << _screenFormat.rBits()) - 1) << _screenFormat.rShift ,
						((1 << _screenFormat.gBits()) - 1) << _screenFormat.gShift ,
						((1 << _screenFormat.bBits()) - 1) << _screenFormat.bShift ,
						((1 << _screenFormat.aBits()) - 1) << _screenFormat.aShift );
	if (_screen == NULL)
		error("allocating _screen failed");

#else
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("allocating _screen failed");
#endif

	// SDL 1.2 palettes default to all black,
	// SDL 1.3 palettes default to all white,
	// Thus set our own default palette to all black.
	// SDL_SetColors does nothing for non indexed surfaces.
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	if (_videoMode.fullscreen) {
		fixupResolutionForAspectRatio(_videoMode.desiredAspectRatio, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	}

	_hwscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 16,
		_videoMode.fullscreen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
	);
#ifdef USE_RGB_COLOR
	detectSupportedFormats();
#endif

	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		if (!_oldVideoMode.setup) {
			warning("SDL_SetVideoMode says we can't switch to that mode (%s)", SDL_GetError());
			g_system->quit();
		} else {
			return false;
		}
	}

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Need some extra bytes around when using 2xSaI
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth + 3, _videoMode.screenHeight + 3,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("allocating _tmpscreen failed");

	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth, _videoMode.overlayHeight,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_overlayscreen == NULL)
		error("allocating _overlayscreen failed");

	_overlayFormat.bytesPerPixel = _overlayscreen->format->BytesPerPixel;

	_overlayFormat.rLoss = _overlayscreen->format->Rloss;
	_overlayFormat.gLoss = _overlayscreen->format->Gloss;
	_overlayFormat.bLoss = _overlayscreen->format->Bloss;
	_overlayFormat.aLoss = _overlayscreen->format->Aloss;

	_overlayFormat.rShift = _overlayscreen->format->Rshift;
	_overlayFormat.gShift = _overlayscreen->format->Gshift;
	_overlayFormat.bShift = _overlayscreen->format->Bshift;
	_overlayFormat.aShift = _overlayscreen->format->Ashift;

	_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth + 3, _videoMode.overlayHeight + 3,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen2 == NULL)
		error("allocating _tmpscreen2 failed");

#ifdef USE_OSD
	_osdSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_hwscreen->w,
						_hwscreen->h,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);
	if (_osdSurface == NULL)
		error("allocating _osdSurface failed");
	SDL_SetColorKey(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kOSDColorKey);
#endif

	_eventSource->resetKeyboadEmulation(
		_videoMode.screenWidth * _videoMode.scaleFactor - 1,
		effectiveScreenHeight() - 1);

	// Distinguish 555 and 565 mode
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);

	return true;
}

void SurfaceSdlGraphicsManager::unloadGFXMode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL;
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL;
	}

	if (_tmpscreen) {
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}

	if (_tmpscreen2) {
		SDL_FreeSurface(_tmpscreen2);
		_tmpscreen2 = NULL;
	}

	if (_overlayscreen) {
		SDL_FreeSurface(_overlayscreen);
		_overlayscreen = NULL;
	}

#ifdef USE_OSD
	if (_osdSurface) {
		SDL_FreeSurface(_osdSurface);
		_osdSurface = NULL;
	}
#endif
	DestroyScalers();
}

bool SurfaceSdlGraphicsManager::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Keep around the old _screen & _overlayscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	_screen = NULL;
	SDL_Surface *old_overlayscreen = _overlayscreen;
	_overlayscreen = NULL;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen); _hwscreen = NULL;

	SDL_FreeSurface(_tmpscreen); _tmpscreen = NULL;
	SDL_FreeSurface(_tmpscreen2); _tmpscreen2 = NULL;

#ifdef USE_OSD
	// Release the OSD surface
	SDL_FreeSurface(_osdSurface); _osdSurface = NULL;
#endif

	// Setup the new GFX mode
	if (!loadGFXMode()) {
		unloadGFXMode();

		_screen = old_screen;
		_overlayscreen = old_overlayscreen;

		return false;
	}

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_overlayscreen, NULL, _overlayscreen, NULL);

	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	SDL_FreeSurface(old_overlayscreen);

	// Update cursor to new scale
	blitCursor();

	// Blit everything to the screen
	internUpdateScreen();

	return true;
}

void SurfaceSdlGraphicsManager::updateScreen() {
	assert(_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	internUpdateScreen();
}

void SurfaceSdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	ScalerProc *scalerProc;
	int scale1;

	// definitions not available for non-DEBUG here. (needed this to compile in SYMBIAN32 & linux?)
#if defined(DEBUG) && !defined(WIN32) && !defined(_WIN32_WCE)
	assert(_hwscreen != NULL);
	assert(_hwscreen->map->sw_data != NULL);
#endif

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos ||
		(_mouseNeedsRedraw && _mouseBackup.y <= _currentShakePos)) {
		SDL_Rect blackrect = {0, 0, _videoMode.screenWidth * _videoMode.scaleFactor, _newShakePos * _videoMode.scaleFactor};

		if (_videoMode.aspectRatioCorrection && !_overlayVisible)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_screen && _paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart,
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

#ifdef USE_OSD
	// OSD visible (i.e. non-transparent)?
	if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
		// Updated alpha value
		const int diff = SDL_GetTicks() - _osdFadeStartTime;
		if (diff > 0) {
			if (diff >= kOSDFadeOutDuration) {
				// Back to full transparency
				_osdAlpha = SDL_ALPHA_TRANSPARENT;
			} else {
				// Do a linear fade out...
				const int startAlpha = SDL_ALPHA_TRANSPARENT + kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
				_osdAlpha = startAlpha + diff * (SDL_ALPHA_TRANSPARENT - startAlpha) / kOSDFadeOutDuration;
			}
			SDL_SetAlpha(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, _osdAlpha);
			_forceFull = true;
		}
	}
#endif

	if (!_overlayVisible) {
		origSurf = _screen;
		srcSurf = _tmpscreen;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
		scalerProc = _scalerProc;
		scale1 = _videoMode.scaleFactor;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
		scalerProc = Normal1x;

		scale1 = 1;
	}

	// Add the area covered by the mouse cursor to the list of dirty rects if
	// we have to redraw the mouse.
	if (_mouseNeedsRedraw)
		undrawMouse();

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0 || _mouseNeedsRedraw) {
		SDL_Rect *r;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;
			dst.x++;	// Shift rect by one since 2xSai needs to access the data around
			dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.

			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_hwscreen);

		srcPitch = srcSurf->pitch;
		dstPitch = _hwscreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			register int dst_y = r->y + _currentShakePos;
			register int dst_h = 0;
			register int orig_dst_y = 0;
			register int rx1 = r->x * scale1;

			if (dst_y < height) {
				dst_h = r->h;
				if (dst_h > height - dst_y)
					dst_h = height - dst_y;

				orig_dst_y = dst_y;
				dst_y = dst_y * scale1;

				if (_videoMode.aspectRatioCorrection && !_overlayVisible)
					dst_y = real2Aspect(dst_y);

				assert(scalerProc != NULL);
				scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
					(byte *)_hwscreen->pixels + rx1 * 2 + dst_y * dstPitch, dstPitch, r->w, dst_h);
			}

			r->x = rx1;
			r->y = dst_y;
			r->w = r->w * scale1;
			r->h = dst_h * scale1;

#ifdef USE_SCALERS
			if (_videoMode.aspectRatioCorrection && orig_dst_y < height && !_overlayVisible)
				r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y * scale1);
#endif
		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_hwscreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].h = effectiveScreenHeight();
		}

		drawMouse();

#ifdef USE_OSD
		if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
			SDL_BlitSurface(_osdSurface, 0, _hwscreen, 0);
		}
#endif

#ifdef USE_SDL_DEBUG_FOCUSRECT
		// We draw the focus rectangle on top of everything, to assure it's easily visible.
		// Of course when the overlay is visible we do not show it, since it is only for game
		// specific focus.
		if (_enableFocusRect && !_overlayVisible) {
			int y = _focusRect.top + _currentShakePos;
			int h = 0;
			int x = _focusRect.left * scale1;
			int w = _focusRect.width() * scale1;

			if (y < height) {
				h = _focusRect.height();
				if (h > height - y)
					h = height - y;

				y *= scale1;

				if (_videoMode.aspectRatioCorrection && !_overlayVisible)
					y = real2Aspect(y);

				if (h > 0 && w > 0) {
					SDL_LockSurface(_hwscreen);

					// Use white as color for now.
					Uint32 rectColor = SDL_MapRGB(_hwscreen->format, 0xFF, 0xFF, 0xFF);

					// First draw the top and bottom lines
					// then draw the left and right lines
					if (_hwscreen->format->BytesPerPixel == 2) {
						uint16 *top = (uint16 *)((byte *)_hwscreen->pixels + y * _hwscreen->pitch + x * 2);
						uint16 *bottom = (uint16 *)((byte *)_hwscreen->pixels + (y + h) * _hwscreen->pitch + x * 2);
						byte *left = ((byte *)_hwscreen->pixels + y * _hwscreen->pitch + x * 2);
						byte *right = ((byte *)_hwscreen->pixels + y * _hwscreen->pitch + (x + w - 1) * 2);

						while (w--) {
							*top++ = rectColor;
							*bottom++ = rectColor;
						}

						while (h--) {
							*(uint16 *)left = rectColor;
							*(uint16 *)right = rectColor;

							left += _hwscreen->pitch;
							right += _hwscreen->pitch;
						}
					} else if (_hwscreen->format->BytesPerPixel == 4) {
						uint32 *top = (uint32 *)((byte *)_hwscreen->pixels + y * _hwscreen->pitch + x * 4);
						uint32 *bottom = (uint32 *)((byte *)_hwscreen->pixels + (y + h) * _hwscreen->pitch + x * 4);
						byte *left = ((byte *)_hwscreen->pixels + y * _hwscreen->pitch + x * 4);
						byte *right = ((byte *)_hwscreen->pixels + y * _hwscreen->pitch + (x + w - 1) * 4);

						while (w--) {
							*top++ = rectColor;
							*bottom++ = rectColor;
						}

						while (h--) {
							*(uint32 *)left = rectColor;
							*(uint32 *)right = rectColor;

							left += _hwscreen->pitch;
							right += _hwscreen->pitch;
						}
					}

					SDL_UnlockSurface(_hwscreen);
				}
			}
		}
#endif

		// Finally, blit all our changes to the screen
		SDL_UpdateRects(_hwscreen, _numDirtyRects, _dirtyRectList);
	}

	_numDirtyRects = 0;
	_forceFull = false;
	_mouseNeedsRedraw = false;
}

bool SurfaceSdlGraphicsManager::saveScreenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends
	return SDL_SaveBMP(_hwscreen, filename) == 0;
}

void SurfaceSdlGraphicsManager::setFullscreenMode(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.fullscreen == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.fullscreen = enable;
		_transactionDetails.needHotswap = true;
	}
}

void SurfaceSdlGraphicsManager::setAspectRatioCorrection(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.aspectRatioCorrection == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.aspectRatioCorrection = enable;
		_transactionDetails.needHotswap = true;
	}
}

void SurfaceSdlGraphicsManager::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	assert(_transactionMode == kTransactionNone);
	assert(buf);

	if (_screen == NULL) {
		warning("SurfaceSdlGraphicsManager::copyRectToScreen: _screen == NULL");
		return;
	}

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	assert(x >= 0 && x < _videoMode.screenWidth);
	assert(y >= 0 && y < _videoMode.screenHeight);
	assert(h > 0 && y + h <= _videoMode.screenHeight);
	assert(w > 0 && x + w <= _videoMode.screenWidth);

	addDirtyRect(x, y, w, h);

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

#ifdef USE_RGB_COLOR
	byte *dst = (byte *)_screen->pixels + y * _screen->pitch + x * _screenFormat.bytesPerPixel;
	if (_videoMode.screenWidth == w && pitch == _screen->pitch) {
		memcpy(dst, buf, h*pitch);
	} else {
		const byte *src = (const byte *)buf;
		do {
			memcpy(dst, src, w * _screenFormat.bytesPerPixel);
			src += pitch;
			dst += _screen->pitch;
		} while (--h);
	}
#else
	byte *dst = (byte *)_screen->pixels + y * _screen->pitch + x;
	if (_screen->pitch == pitch && pitch == w) {
		memcpy(dst, buf, h*w);
	} else {
		const byte *src = (const byte *)buf;
		do {
			memcpy(dst, src, w);
			src += pitch;
			dst += _screen->pitch;
		} while (--h);
	}
#endif

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);
}

Graphics::Surface *SurfaceSdlGraphicsManager::lockScreen() {
	assert(_transactionMode == kTransactionNone);

	// Lock the graphics mutex
	g_system->lockMutex(_graphicsMutex);

	// paranoia check
	assert(!_screenIsLocked);
	_screenIsLocked = true;

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	_framebuffer.pixels = _screen->pixels;
	_framebuffer.w = _screen->w;
	_framebuffer.h = _screen->h;
	_framebuffer.pitch = _screen->pitch;
#ifdef USE_RGB_COLOR
	_framebuffer.format = _screenFormat;
#else
	_framebuffer.format = Graphics::PixelFormat::createFormatCLUT8();
#endif

	return &_framebuffer;
}

void SurfaceSdlGraphicsManager::unlockScreen() {
	assert(_transactionMode == kTransactionNone);

	// paranoia check
	assert(_screenIsLocked);
	_screenIsLocked = false;

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);

	// Trigger a full screen update
	_forceFull = true;

	// Finally unlock the graphics mutex
	g_system->unlockMutex(_graphicsMutex);
}

void SurfaceSdlGraphicsManager::fillScreen(uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen && screen->pixels)
		memset(screen->pixels, col, screen->h * screen->pitch);
	unlockScreen();
}

void SurfaceSdlGraphicsManager::addDirtyRect(int x, int y, int w, int h, bool realCoordinates) {
	if (_forceFull)
		return;

	if (_numDirtyRects == NUM_DIRTY_RECT) {
		_forceFull = true;
		return;
	}

	int height, width;

	if (!_overlayVisible && !realCoordinates) {
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
	} else {
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
	}

	// Extend the dirty region by 1 pixel for scalers
	// that "smear" the screen, e.g. 2xSAI
	if (!realCoordinates) {
		x--;
		y--;
		w+=2;
		h+=2;
	}

	// clip
	if (x < 0) {
		w += x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		y=0;
	}

	if (w > width - x) {
		w = width - x;
	}

	if (h > height - y) {
		h = height - y;
	}

#ifdef USE_SCALERS
	if (_videoMode.aspectRatioCorrection && !_overlayVisible && !realCoordinates) {
		makeRectStretchable(x, y, w, h);
	}
#endif

	if (w == width && h == height) {
		_forceFull = true;
		return;
	}

	if (w > 0 && h > 0) {
		SDL_Rect *r = &_dirtyRectList[_numDirtyRects++];

		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}

int16 SurfaceSdlGraphicsManager::getHeight() {
	return _videoMode.screenHeight;
}

int16 SurfaceSdlGraphicsManager::getWidth() {
	return _videoMode.screenWidth;
}

void SurfaceSdlGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	assert(colors);

#ifdef USE_RGB_COLOR
	assert(_screenFormat.bytesPerPixel == 1);
#endif

	// Setting the palette before _screen is created is allowed - for now -
	// since we don't actually set the palette until the screen is updated.
	// But it could indicate a programming error, so let's warn about it.

	if (!_screen)
		warning("SurfaceSdlGraphicsManager::setPalette: _screen == NULL");

	const byte *b = colors;
	uint i;
	SDL_Color *base = _currentPalette + start;
	for (i = 0; i < num; i++, b += 3) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;

	// Some games blink cursors with palette
	if (_cursorPaletteDisabled)
		blitCursor();
}

void SurfaceSdlGraphicsManager::grabPalette(byte *colors, uint start, uint num) {
	assert(colors);

#ifdef USE_RGB_COLOR
	assert(_screenFormat.bytesPerPixel == 1);
#endif

	const SDL_Color *base = _currentPalette + start;

	for (uint i = 0; i < num; ++i) {
		colors[i * 3] = base[i].r;
		colors[i * 3 + 1] = base[i].g;
		colors[i * 3 + 2] = base[i].b;
	}
}

void SurfaceSdlGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	assert(colors);
	const byte *b = colors;
	uint i;
	SDL_Color *base = _cursorPalette + start;
	for (i = 0; i < num; i++, b += 3) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
	}

	_cursorPaletteDisabled = false;
	blitCursor();
}

void SurfaceSdlGraphicsManager::setShakePos(int shake_pos) {
	assert(_transactionMode == kTransactionNone);

	_newShakePos = shake_pos;
}

void SurfaceSdlGraphicsManager::setFocusRectangle(const Common::Rect &rect) {
#ifdef USE_SDL_DEBUG_FOCUSRECT
	// Only enable focus rectangle debug code, when the user wants it
	if (!_enableFocusRectDebugCode)
		return;

	_enableFocusRect = true;
	_focusRect = rect;

	if (rect.left < 0 || rect.top < 0 || rect.right > _videoMode.screenWidth || rect.bottom > _videoMode.screenHeight)
		warning("SurfaceSdlGraphicsManager::setFocusRectangle: Got a rect which does not fit inside the screen bounds: %d,%d,%d,%d", rect.left, rect.top, rect.right, rect.bottom);

	// It's gross but we actually sometimes get rects, which are not inside the screen bounds,
	// thus we need to clip the rect here...
	_focusRect.clip(_videoMode.screenWidth, _videoMode.screenHeight);

	// We just fake this as a dirty rect for now, to easily force an screen update whenever
	// the rect changes.
	addDirtyRect(_focusRect.left, _focusRect.top, _focusRect.width(), _focusRect.height());
#endif
}

void SurfaceSdlGraphicsManager::clearFocusRectangle() {
#ifdef USE_SDL_DEBUG_FOCUSRECT
	// Only enable focus rectangle debug code, when the user wants it
	if (!_enableFocusRectDebugCode)
		return;

	_enableFocusRect = false;

	// We just fake this as a dirty rect for now, to easily force an screen update whenever
	// the rect changes.
	addDirtyRect(_focusRect.left, _focusRect.top, _focusRect.width(), _focusRect.height());
#endif
}

#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void SurfaceSdlGraphicsManager::showOverlay() {
	assert(_transactionMode == kTransactionNone);

	int x, y;

	if (_overlayVisible)
		return;

	_overlayVisible = true;

	// Since resolution could change, put mouse to adjusted position
	// Fixes bug #1349059
	x = _mouseCurState.x * _videoMode.scaleFactor;
	if (_videoMode.aspectRatioCorrection)
		y = real2Aspect(_mouseCurState.y) * _videoMode.scaleFactor;
	else
		y = _mouseCurState.y * _videoMode.scaleFactor;

	warpMouse(x, y);

	clearOverlay();
}

void SurfaceSdlGraphicsManager::hideOverlay() {
	assert(_transactionMode == kTransactionNone);

	if (!_overlayVisible)
		return;

	int x, y;

	_overlayVisible = false;

	// Since resolution could change, put mouse to adjusted position
	// Fixes bug #1349059
	x = _mouseCurState.x / _videoMode.scaleFactor;
	y = _mouseCurState.y / _videoMode.scaleFactor;
	if (_videoMode.aspectRatioCorrection)
		y = aspect2Real(y);

	warpMouse(x, y);

	clearOverlay();

	_forceFull = true;
}

void SurfaceSdlGraphicsManager::clearOverlay() {
	//assert(_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (!_overlayVisible)
		return;

	// Clear the overlay by making the game screen "look through" everywhere.
	SDL_Rect src, dst;
	src.x = src.y = 0;
	dst.x = dst.y = 1;
	src.w = dst.w = _videoMode.screenWidth;
	src.h = dst.h = _videoMode.screenHeight;
	if (SDL_BlitSurface(_screen, &src, _tmpscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	SDL_LockSurface(_tmpscreen);
	SDL_LockSurface(_overlayscreen);
	_scalerProc((byte *)(_tmpscreen->pixels) + _tmpscreen->pitch + 2, _tmpscreen->pitch,
	(byte *)_overlayscreen->pixels, _overlayscreen->pitch, _videoMode.screenWidth, _videoMode.screenHeight);

#ifdef USE_SCALERS
	if (_videoMode.aspectRatioCorrection)
		stretch200To240((uint8 *)_overlayscreen->pixels, _overlayscreen->pitch,
						_videoMode.overlayWidth, _videoMode.screenHeight * _videoMode.scaleFactor, 0, 0, 0);
#endif
	SDL_UnlockSurface(_tmpscreen);
	SDL_UnlockSurface(_overlayscreen);

	_forceFull = true;
}

void SurfaceSdlGraphicsManager::grabOverlay(void *buf, int pitch) {
	assert(_transactionMode == kTransactionNone);

	if (_overlayscreen == NULL)
		return;

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *src = (byte *)_overlayscreen->pixels;
	byte *dst = (byte *)buf;
	int h = _videoMode.overlayHeight;
	do {
		memcpy(dst, src, _videoMode.overlayWidth * 2);
		src += _overlayscreen->pitch;
		dst += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

void SurfaceSdlGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	assert(_transactionMode == kTransactionNone);

	if (_overlayscreen == NULL)
		return;

	const byte *src = (const byte *)buf;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		src -= x * 2;
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > _videoMode.overlayWidth - x) {
		w = _videoMode.overlayWidth - x;
	}

	if (h > _videoMode.overlayHeight - y) {
		h = _videoMode.overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	// Mark the modified region as dirty
	addDirtyRect(x, y, w, h);

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_overlayscreen->pixels + y * _overlayscreen->pitch + x * 2;
	do {
		memcpy(dst, src, w * 2);
		dst += _overlayscreen->pitch;
		src += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}


#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool SurfaceSdlGraphicsManager::showMouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;

	bool last = _mouseVisible;
	_mouseVisible = visible;
	_mouseNeedsRedraw = true;

	return last;
}

void SurfaceSdlGraphicsManager::setMousePos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		_mouseNeedsRedraw = true;
		_mouseCurState.x = x;
		_mouseCurState.y = y;
	}
}

void SurfaceSdlGraphicsManager::warpMouse(int x, int y) {
	int y1 = y;

	// Don't change actual mouse position, when mouse is outside of our window (in case of windowed mode)
	if (!(SDL_GetAppState( ) & SDL_APPMOUSEFOCUS)) {
		setMousePos(x, y); // but change game cursor position
		return;
	}

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		y1 = real2Aspect(y);

	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		if (!_overlayVisible)
			SDL_WarpMouse(x * _videoMode.scaleFactor, y1 * _videoMode.scaleFactor);
		else
			SDL_WarpMouse(x, y1);

		// SDL_WarpMouse() generates a mouse movement event, so
		// setMousePos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		setMousePos(x, y);
	}
}

void SurfaceSdlGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
#ifdef USE_RGB_COLOR
	if (!format)
		_cursorFormat = Graphics::PixelFormat::createFormatCLUT8();
	else if (format->bytesPerPixel <= _screenFormat.bytesPerPixel)
		_cursorFormat = *format;

	if (_cursorFormat.bytesPerPixel < 4)
		assert(keycolor < (uint)(1 << (_cursorFormat.bytesPerPixel << 3)));
#else
	assert(keycolor <= 0xFF);
#endif

	if (w == 0 || h == 0)
		return;

	_mouseCurState.hotX = hotspot_x;
	_mouseCurState.hotY = hotspot_y;

	_mouseKeyColor = keycolor;

	_cursorDontScale = dontScale;

	if (_mouseCurState.w != (int)w || _mouseCurState.h != (int)h) {
		_mouseCurState.w = w;
		_mouseCurState.h = h;

		if (_mouseOrigSurface)
			SDL_FreeSurface(_mouseOrigSurface);

		// Allocate bigger surface because AdvMame2x adds black pixel at [0,0]
		_mouseOrigSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_mouseCurState.w + 2,
						_mouseCurState.h + 2,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

		if (_mouseOrigSurface == NULL)
			error("allocating _mouseOrigSurface failed");
		SDL_SetColorKey(_mouseOrigSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kMouseColorKey);
	}

	free(_mouseData);
#ifdef USE_RGB_COLOR
	_mouseData = (byte *)malloc(w * h * _cursorFormat.bytesPerPixel);
	memcpy(_mouseData, buf, w * h * _cursorFormat.bytesPerPixel);
#else
	_mouseData = (byte *)malloc(w * h);
	memcpy(_mouseData, buf, w * h);
#endif

	blitCursor();
}

void SurfaceSdlGraphicsManager::blitCursor() {
	byte *dstPtr;
	const byte *srcPtr = _mouseData;
#ifdef USE_RGB_COLOR
	uint32 color;
#else
	byte color;
#endif
	int w, h, i, j;

	if (!_mouseOrigSurface || !_mouseData)
		return;

	_mouseNeedsRedraw = true;

	w = _mouseCurState.w;
	h = _mouseCurState.h;

	SDL_LockSurface(_mouseOrigSurface);

	// Make whole surface transparent
	for (i = 0; i < h + 2; i++) {
		dstPtr = (byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * i;
		for (j = 0; j < w + 2; j++) {
			*(uint16 *)dstPtr = kMouseColorKey;
			dstPtr += 2;
		}
	}

	// Draw from [1,1] since AdvMame2x adds artefact at 0,0
	dstPtr = (byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch + 2;

	SDL_Color *palette;

	if (_cursorPaletteDisabled)
		palette = _currentPalette;
	else
		palette = _cursorPalette;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
#ifdef USE_RGB_COLOR
			if (_cursorFormat.bytesPerPixel > 1) {
				if (_cursorFormat.bytesPerPixel == 2)
					color = *(const uint16 *)srcPtr;
				else
					color = *(const uint32 *)srcPtr;
				if (color != _mouseKeyColor) {	// transparent, don't draw
					uint8 r, g, b;
					_cursorFormat.colorToRGB(color, r, g, b);
					*(uint16 *)dstPtr = SDL_MapRGB(_mouseOrigSurface->format,
						r, g, b);
				}
				dstPtr += 2;
				srcPtr += _cursorFormat.bytesPerPixel;
			} else {
#endif
				color = *srcPtr;
				if (color != _mouseKeyColor) {	// transparent, don't draw
					*(uint16 *)dstPtr = SDL_MapRGB(_mouseOrigSurface->format,
						palette[color].r, palette[color].g, palette[color].b);
				}
				dstPtr += 2;
				srcPtr++;
#ifdef USE_RGB_COLOR
			}
#endif
		}
		dstPtr += _mouseOrigSurface->pitch - w * 2;
	}

	int rW, rH;
	int cursorScale;

	if (_cursorDontScale) {
		// Don't scale the cursor at all if the user requests this behavior.
		cursorScale = 1;
	} else {
		// Scale the cursor with the game screen scale factor.
		cursorScale = _videoMode.scaleFactor;
	}

	// Adapt the real hotspot according to the scale factor.
	rW = w * cursorScale;
	rH = h * cursorScale;
	_mouseCurState.rHotX = _mouseCurState.hotX * cursorScale;
	_mouseCurState.rHotY = _mouseCurState.hotY * cursorScale;

	// The virtual dimensions will be the same as the original.

	_mouseCurState.vW = w;
	_mouseCurState.vH = h;
	_mouseCurState.vHotX = _mouseCurState.hotX;
	_mouseCurState.vHotY = _mouseCurState.hotY;

#ifdef USE_SCALERS
	int rH1 = rH; // store original to pass to aspect-correction function later
#endif

	if (!_cursorDontScale && _videoMode.aspectRatioCorrection) {
		rH = real2Aspect(rH - 1) + 1;
		_mouseCurState.rHotY = real2Aspect(_mouseCurState.rHotY);
	}

	if (_mouseCurState.rW != rW || _mouseCurState.rH != rH) {
		_mouseCurState.rW = rW;
		_mouseCurState.rH = rH;

		if (_mouseSurface)
			SDL_FreeSurface(_mouseSurface);

		_mouseSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_mouseCurState.rW,
						_mouseCurState.rH,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

		if (_mouseSurface == NULL)
			error("allocating _mouseSurface failed");

		SDL_SetColorKey(_mouseSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kMouseColorKey);
	}

	SDL_LockSurface(_mouseSurface);

	ScalerProc *scalerProc;

	// Only apply scaling, when the user allows it.
	if (!_cursorDontScale) {
		// If possible, use the same scaler for the cursor as for the rest of
		// the game. This only works well with the non-blurring scalers so we
		// actually only use the 1x, 2x and AdvMame scalers.
		if (_videoMode.mode == GFX_DOUBLESIZE || _videoMode.mode == GFX_TRIPLESIZE)
			scalerProc = _scalerProc;
		else
			scalerProc = scalersMagn[_videoMode.scaleFactor - 1];
	} else {
		scalerProc = Normal1x;
	}

	scalerProc((byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch + 2,
		_mouseOrigSurface->pitch, (byte *)_mouseSurface->pixels, _mouseSurface->pitch,
		_mouseCurState.w, _mouseCurState.h);

#ifdef USE_SCALERS
	if (!_cursorDontScale && _videoMode.aspectRatioCorrection)
		cursorStretch200To240((uint8 *)_mouseSurface->pixels, _mouseSurface->pitch, rW, rH1, 0, 0, 0);
#endif

	SDL_UnlockSurface(_mouseSurface);
	SDL_UnlockSurface(_mouseOrigSurface);
}

#ifdef USE_SCALERS
// Basically it is kVeryFastAndUglyAspectMode of stretch200To240 from
// common/scale/aspect.cpp
static int cursorStretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY) {
	int maxDstY = real2Aspect(origSrcY + height - 1);
	int y;
	const uint8 *startSrcPtr = buf + srcX * 2 + (srcY - origSrcY) * pitch;
	uint8 *dstPtr = buf + srcX * 2 + maxDstY * pitch;

	for (y = maxDstY; y >= srcY; y--) {
		const uint8 *srcPtr = startSrcPtr + aspect2Real(y) * pitch;

		if (srcPtr == dstPtr)
			break;
		memcpy(dstPtr, srcPtr, width * 2);
		dstPtr -= pitch;
	}

	return 1 + maxDstY - srcY;
}
#endif

void SurfaceSdlGraphicsManager::undrawMouse() {
	const int x = _mouseBackup.x;
	const int y = _mouseBackup.y;

	// When we switch bigger overlay off mouse jumps. Argh!
	// This is intended to prevent undrawing offscreen mouse
	if (!_overlayVisible && (x >= _videoMode.screenWidth || y >= _videoMode.screenHeight))
		return;

	if (_mouseBackup.w != 0 && _mouseBackup.h != 0)
		addDirtyRect(x, y - _currentShakePos, _mouseBackup.w, _mouseBackup.h);
}

void SurfaceSdlGraphicsManager::drawMouse() {
	if (!_mouseVisible || !_mouseSurface) {
		_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;
		return;
	}

	SDL_Rect dst;
	int scale;
	int hotX, hotY;

	dst.x = _mouseCurState.x;
	dst.y = _mouseCurState.y;

	if (!_overlayVisible) {
		scale = _videoMode.scaleFactor;
		dst.w = _mouseCurState.vW;
		dst.h = _mouseCurState.vH;
		hotX = _mouseCurState.vHotX;
		hotY = _mouseCurState.vHotY;
	} else {
		scale = 1;
		dst.w = _mouseCurState.rW;
		dst.h = _mouseCurState.rH;
		hotX = _mouseCurState.rHotX;
		hotY = _mouseCurState.rHotY;
	}

	// The mouse is undrawn using virtual coordinates, i.e. they may be
	// scaled and aspect-ratio corrected.

	_mouseBackup.x = dst.x - hotX;
	_mouseBackup.y = dst.y - hotY;
	_mouseBackup.w = dst.w;
	_mouseBackup.h = dst.h;

	// We draw the pre-scaled cursor image, so now we need to adjust for
	// scaling, shake position and aspect ratio correction manually.

	if (!_overlayVisible) {
		dst.y += _currentShakePos;
	}

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		dst.y = real2Aspect(dst.y);

	dst.x = scale * dst.x - _mouseCurState.rHotX;
	dst.y = scale * dst.y - _mouseCurState.rHotY;
	dst.w = _mouseCurState.rW;
	dst.h = _mouseCurState.rH;

	// Note that SDL_BlitSurface() and addDirtyRect() will both perform any
	// clipping necessary

	if (SDL_BlitSurface(_mouseSurface, NULL, _hwscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	// The screen will be updated using real surface coordinates, i.e.
	// they will not be scaled or aspect-ratio corrected.

	addDirtyRect(dst.x, dst.y, dst.w, dst.h, true);
}

#pragma mark -
#pragma mark --- On Screen Display ---
#pragma mark -

#ifdef USE_OSD
void SurfaceSdlGraphicsManager::displayMessageOnOSD(const char *msg) {
	assert(_transactionMode == kTransactionNone);
	assert(msg);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	uint i;

	// Lock the OSD surface for drawing
	if (SDL_LockSurface(_osdSurface))
		error("displayMessageOnOSD: SDL_LockSurface failed: %s", SDL_GetError());

	Graphics::Surface dst;
	dst.pixels = _osdSurface->pixels;
	dst.w = _osdSurface->w;
	dst.h = _osdSurface->h;
	dst.pitch = _osdSurface->pitch;
	dst.format = Graphics::PixelFormat(_osdSurface->format->BytesPerPixel,
	                                   8 - _osdSurface->format->Rloss, 8 - _osdSurface->format->Gloss,
	                                   8 - _osdSurface->format->Bloss, 8 - _osdSurface->format->Aloss,
	                                   _osdSurface->format->Rshift, _osdSurface->format->Gshift,
	                                   _osdSurface->format->Bshift, _osdSurface->format->Ashift);

	// The font we are going to use:
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);

	// Clear everything with the "transparent" color, i.e. the colorkey
	SDL_FillRect(_osdSurface, 0, kOSDColorKey);

	// Split the message into separate lines.
	Common::Array<Common::String> lines;
	const char *ptr;
	for (ptr = msg; *ptr; ++ptr) {
		if (*ptr == '\n') {
			lines.push_back(Common::String(msg, ptr - msg));
			msg = ptr + 1;
		}
	}
	lines.push_back(Common::String(msg, ptr - msg));

	// Determine a rect which would contain the message string (clipped to the
	// screen dimensions).
	const int vOffset = 6;
	const int lineSpacing = 1;
	const int lineHeight = font->getFontHeight() + 2 * lineSpacing;
	int width = 0;
	int height = lineHeight * lines.size() + 2 * vOffset;
	for (i = 0; i < lines.size(); i++) {
		width = MAX(width, font->getStringWidth(lines[i]) + 14);
	}

	// Clip the rect
	if (width > dst.w)
		width = dst.w;
	if (height > dst.h)
		height = dst.h;

	// Draw a dark gray rect
	// TODO: Rounded corners ? Border?
	SDL_Rect osdRect;
	osdRect.x = (dst.w - width) / 2;
	osdRect.y = (dst.h - height) / 2;
	osdRect.w = width;
	osdRect.h = height;
	SDL_FillRect(_osdSurface, &osdRect, SDL_MapRGB(_osdSurface->format, 64, 64, 64));

	// Render the message, centered, and in white
	for (i = 0; i < lines.size(); i++) {
		font->drawString(&dst, lines[i],
							osdRect.x, osdRect.y + i * lineHeight + vOffset + lineSpacing, osdRect.w,
							SDL_MapRGB(_osdSurface->format, 255, 255, 255),
							Graphics::kTextAlignCenter);
	}

	// Finished drawing, so unlock the OSD surface again
	SDL_UnlockSurface(_osdSurface);

	// Init the OSD display parameters, and the fade out
	_osdAlpha = SDL_ALPHA_TRANSPARENT + kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
	_osdFadeStartTime = SDL_GetTicks() + kOSDFadeOutDelay;
	SDL_SetAlpha(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, _osdAlpha);

	// Ensure a full redraw takes place next time the screen is updated
	_forceFull = true;
}
#endif

bool SurfaceSdlGraphicsManager::handleScalerHotkeys(Common::KeyCode key) {

	// Ctrl-Alt-a toggles aspect ratio correction
	if (key == 'a') {
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureAspectRatioCorrection, !_videoMode.aspectRatioCorrection);
		endGFXTransaction();
#ifdef USE_OSD
		char buffer[128];
		if (_videoMode.aspectRatioCorrection)
			sprintf(buffer, "%s\n%d x %d -> %d x %d",
				_("Enabled aspect ratio correction"),
				_videoMode.screenWidth, _videoMode.screenHeight,
				_hwscreen->w, _hwscreen->h
				);
		else
			sprintf(buffer, "%s\n%d x %d -> %d x %d",
				_("Disabled aspect ratio correction"),
				_videoMode.screenWidth, _videoMode.screenHeight,
				_hwscreen->w, _hwscreen->h
				);
		displayMessageOnOSD(buffer);
#endif
		internUpdateScreen();
		return true;
	}

	int newMode = -1;
	int factor = _videoMode.scaleFactor - 1;
	SDLKey sdlKey = (SDLKey)key;

	// Increase/decrease the scale factor
	if (sdlKey == SDLK_EQUALS || sdlKey == SDLK_PLUS || sdlKey == SDLK_MINUS ||
		sdlKey == SDLK_KP_PLUS || sdlKey == SDLK_KP_MINUS) {
		factor += (sdlKey == SDLK_MINUS || sdlKey == SDLK_KP_MINUS) ? -1 : +1;
		if (0 <= factor && factor <= 3) {
			newMode = s_gfxModeSwitchTable[_scalerType][factor];
		}
	}

	const bool isNormalNumber = (SDLK_1 <= sdlKey && sdlKey <= SDLK_9);
	const bool isKeypadNumber = (SDLK_KP1 <= sdlKey && sdlKey <= SDLK_KP9);
	if (isNormalNumber || isKeypadNumber) {
		_scalerType = sdlKey - (isNormalNumber ? SDLK_1 : SDLK_KP1);
		if (_scalerType >= ARRAYSIZE(s_gfxModeSwitchTable))
			return false;

		while (s_gfxModeSwitchTable[_scalerType][factor] < 0) {
			assert(factor > 0);
			factor--;
		}
		newMode = s_gfxModeSwitchTable[_scalerType][factor];
	}

	if (newMode >= 0) {
		beginGFXTransaction();
			setGraphicsMode(newMode);
		endGFXTransaction();
#ifdef USE_OSD
		if (_osdSurface) {
			const char *newScalerName = 0;
			const OSystem::GraphicsMode *g = getSupportedGraphicsModes();
			while (g->name) {
				if (g->id == _videoMode.mode) {
					newScalerName = g->description;
					break;
				}
				g++;
			}
			if (newScalerName) {
				char buffer[128];
				sprintf(buffer, "%s %s\n%d x %d -> %d x %d",
					_("Active graphics filter:"),
					newScalerName,
					_videoMode.screenWidth, _videoMode.screenHeight,
					_hwscreen->w, _hwscreen->h
					);
				displayMessageOnOSD(buffer);
			}
		}
#endif
		internUpdateScreen();

		return true;
	} else {
		return false;
	}
}

bool SurfaceSdlGraphicsManager::isScalerHotkey(const Common::Event &event) {
	if ((event.kbd.flags & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {
		const bool isNormalNumber = (Common::KEYCODE_1 <= event.kbd.keycode && event.kbd.keycode <= Common::KEYCODE_9);
		const bool isKeypadNumber = (Common::KEYCODE_KP1 <= event.kbd.keycode && event.kbd.keycode <= Common::KEYCODE_KP9);
		const bool isScaleKey = (event.kbd.keycode == Common::KEYCODE_EQUALS || event.kbd.keycode == Common::KEYCODE_PLUS || event.kbd.keycode == Common::KEYCODE_MINUS ||
			event.kbd.keycode == Common::KEYCODE_KP_PLUS || event.kbd.keycode == Common::KEYCODE_KP_MINUS);

		if (isNormalNumber || isKeypadNumber) {
			int keyValue = event.kbd.keycode - (isNormalNumber ? Common::KEYCODE_1 : Common::KEYCODE_KP1);
			if (keyValue >= ARRAYSIZE(s_gfxModeSwitchTable))
				return false;
		}
		return (isScaleKey || event.kbd.keycode == 'a');
	}
	return false;
}

void SurfaceSdlGraphicsManager::toggleFullScreen() {
	beginGFXTransaction();
		setFullscreenMode(!_videoMode.fullscreen);
	endGFXTransaction();
#ifdef USE_OSD
	if (_videoMode.fullscreen)
		displayMessageOnOSD(_("Fullscreen mode"));
	else
		displayMessageOnOSD(_("Windowed mode"));
#endif
}

bool SurfaceSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	switch ((int)event.type) {
	case Common::EVENT_KEYDOWN:
		// Alt-Return and Alt-Enter toggle full screen mode
		if (event.kbd.hasFlags(Common::KBD_ALT) &&
			(event.kbd.keycode == Common::KEYCODE_RETURN ||
			event.kbd.keycode == (Common::KeyCode)SDLK_KP_ENTER)) {
			toggleFullScreen();
			return true;
		}

		// Alt-S: Create a screenshot
		if (event.kbd.hasFlags(Common::KBD_ALT) && event.kbd.keycode == 's') {
			char filename[20];

			for (int n = 0;; n++) {
				SDL_RWops *file;

				sprintf(filename, "scummvm%05d.bmp", n);
				file = SDL_RWFromFile(filename, "r");
				if (!file)
					break;
				SDL_RWclose(file);
			}
			if (saveScreenshot(filename))
				debug("Saved screenshot '%s'", filename);
			else
				warning("Could not save screenshot");
			return true;
		}

		// Ctrl-Alt-<key> will change the GFX mode
		if (event.kbd.hasFlags(Common::KBD_CTRL|Common::KBD_ALT)) {
			if (handleScalerHotkeys(event.kbd.keycode))
				return true;
		}

	case Common::EVENT_KEYUP:
		return isScalerHotkey(event);

	default:
		break;
	}

	return false;
}

void SurfaceSdlGraphicsManager::notifyVideoExpose() {
	_forceFull = true;
}

void SurfaceSdlGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	if (!_overlayVisible) {
		point.x /= _videoMode.scaleFactor;
		point.y /= _videoMode.scaleFactor;
		if (_videoMode.aspectRatioCorrection)
			point.y = aspect2Real(point.y);
	}
}

void SurfaceSdlGraphicsManager::notifyMousePos(Common::Point mouse) {
	transformMouseCoordinates(mouse);
	setMousePos(mouse.x, mouse.y);
}

#endif
