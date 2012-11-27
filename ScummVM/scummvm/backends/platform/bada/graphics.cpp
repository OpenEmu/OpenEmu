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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/fontman.h"

#include "backends/platform/bada/form.h"
#include "backends/platform/bada/system.h"
#include "backends/platform/bada/graphics.h"

//
// BadaGraphicsManager
//
BadaGraphicsManager::BadaGraphicsManager(BadaAppForm *appForm) :
	_appForm(appForm),
	_eglDisplay(EGL_DEFAULT_DISPLAY),
	_eglSurface(EGL_NO_SURFACE),
	_eglConfig(0),
	_eglContext(EGL_NO_CONTEXT),
	_initState(true) {
	assert(appForm != NULL);
	_videoMode.fullscreen = true;
}

const Graphics::Font *BadaGraphicsManager::getFontOSD() {
	return FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
}

bool BadaGraphicsManager::moveMouse(int16 &x, int16 &y) {
	int16 currentX = _cursorState.x;
	int16 currentY = _cursorState.y;

	// save the current hardware coordinates
	_cursorState.x = x;
	_cursorState.y = y;

	// return x/y as game coordinates
	adjustMousePosition(x, y);

	// convert current x/y to game coordinates
	adjustMousePosition(currentX, currentY);

	// return whether game coordinates have changed
	return (currentX != x || currentY != y);
}

Common::List<Graphics::PixelFormat> BadaGraphicsManager::getSupportedFormats() const {
	logEntered();

	Common::List<Graphics::PixelFormat> res;
	res.push_back(Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0));
	res.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	res.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	res.push_back(Graphics::PixelFormat::createFormatCLUT8());
	return res;
}

bool BadaGraphicsManager::hasFeature(OSystem::Feature f) {
	bool result = (f == OSystem::kFeatureFullscreenMode ||
								 f == OSystem::kFeatureVirtualKeyboard ||
								 OpenGLGraphicsManager::hasFeature(f));
	return result;
}

void BadaGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	OpenGLGraphicsManager::setFeatureState(f, enable);
}

void BadaGraphicsManager::setReady() {
	_initState = false;
}

void BadaGraphicsManager::updateScreen() {
	if (_transactionMode == kTransactionNone) {
		internUpdateScreen();
	}
}

bool BadaGraphicsManager::loadEgl() {
	logEntered();

	EGLint numConfigs = 1;
	EGLint eglConfigList[] = {
		EGL_RED_SIZE,		5,
		EGL_GREEN_SIZE, 6,
		EGL_BLUE_SIZE,	5,
		EGL_ALPHA_SIZE, 0,
		EGL_DEPTH_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
		EGL_NONE
	};

	EGLint eglContextList[] = {
		EGL_CONTEXT_CLIENT_VERSION, 1,
		EGL_NONE
	};

	eglBindAPI(EGL_OPENGL_ES_API);

	if (_eglDisplay) {
		unloadGFXMode();
	}

	_eglDisplay = eglGetDisplay((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);
	if (EGL_NO_DISPLAY == _eglDisplay) {
		systemError("eglGetDisplay() failed");
		return false;
	}

	if (EGL_FALSE == eglInitialize(_eglDisplay, NULL, NULL) ||
			EGL_SUCCESS != eglGetError()) {
		systemError("eglInitialize() failed");
		return false;
	}

	if (EGL_FALSE == eglChooseConfig(_eglDisplay, eglConfigList,
																	 &_eglConfig, 1, &numConfigs) ||
			EGL_SUCCESS != eglGetError()) {
		systemError("eglChooseConfig() failed");
		return false;
	}

	if (!numConfigs) {
		systemError("eglChooseConfig() failed. Matching config does not exist \n");
		return false;
	}

	_eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig,
																			(EGLNativeWindowType)_appForm, NULL);
	if (EGL_NO_SURFACE == _eglSurface || EGL_SUCCESS != eglGetError()) {
		systemError("eglCreateWindowSurface() failed. EGL_NO_SURFACE");
		return false;
	}

	_eglContext = eglCreateContext(_eglDisplay, _eglConfig,
																EGL_NO_CONTEXT, eglContextList);
	if (EGL_NO_CONTEXT == _eglContext ||
			EGL_SUCCESS != eglGetError()) {
		systemError("eglCreateContext() failed");
		return false;
	}

	if (false == eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext) ||
			EGL_SUCCESS != eglGetError()) {
		systemError("eglMakeCurrent() failed");
		return false;
	}

	logLeaving();
	return true;
}

bool BadaGraphicsManager::loadGFXMode() {
	logEntered();

	if (!loadEgl()) {
		unloadGFXMode();
		return false;
	}

	int x, y, width, height;
	_appForm->GetBounds(x, y, width, height);
	_videoMode.overlayWidth = _videoMode.hardwareWidth = width;
	_videoMode.overlayHeight = _videoMode.hardwareHeight = height;
	_videoMode.scaleFactor = 3; // for proportional sized cursor in the launcher

	AppLog("screen size: %dx%d", _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	return OpenGLGraphicsManager::loadGFXMode();
}

void BadaGraphicsManager::loadTextures() {
	logEntered();

	OpenGLGraphicsManager::loadTextures();

	// prevent image skew in some games, see:
	// http://www.opengl.org/resources/features/KilgardTechniques/oglpitfall
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void BadaGraphicsManager::internUpdateScreen() {
	if (!_initState) {
		OpenGLGraphicsManager::internUpdateScreen();
		eglSwapBuffers(_eglDisplay, _eglSurface);
	} else {
		showSplash();
	}
}

void BadaGraphicsManager::unloadGFXMode() {
	logEntered();

	if (EGL_NO_DISPLAY != _eglDisplay) {
		eglMakeCurrent(_eglDisplay, NULL, NULL, NULL);

		if (_eglContext != EGL_NO_CONTEXT) {
			eglDestroyContext(_eglDisplay, _eglContext);
			_eglContext = EGL_NO_CONTEXT;
		}

		if (_eglSurface != EGL_NO_SURFACE) {
			eglDestroySurface(_eglDisplay, _eglSurface);
			_eglSurface = EGL_NO_SURFACE;
		}

		eglTerminate(_eglDisplay);
		_eglDisplay = EGL_NO_DISPLAY;
	}

	_eglConfig = NULL;

	OpenGLGraphicsManager::unloadGFXMode();
	logLeaving();
}

// display a simple splash screen until launcher is ready
void BadaGraphicsManager::showSplash() {
	Canvas canvas;
	canvas.Construct();
	canvas.SetBackgroundColor(Color::COLOR_BLACK);
	canvas.Clear();

	int x = _videoMode.hardwareWidth / 3;
	int y = _videoMode.hardwareHeight / 3;

	Font *pFont = new Font();
	pFont->Construct(FONT_STYLE_ITALIC | FONT_STYLE_BOLD, 55);
	canvas.SetFont(*pFont);
	canvas.SetForegroundColor(Color::COLOR_GREEN);
	canvas.DrawText(Point(x, y), L"ScummVM");
	delete pFont;

	pFont = new Font();
	pFont->Construct(FONT_STYLE_ITALIC | FONT_STYLE_BOLD, 35);
	canvas.SetFont(*pFont);
	canvas.SetForegroundColor(Color::COLOR_WHITE);
	canvas.DrawText(Point(x + 70, y + 50), L"Loading ...");
	delete pFont;

	canvas.Show();

}
