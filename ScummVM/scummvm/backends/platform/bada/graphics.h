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

#ifndef BADA_GRAPHICS_H
#define BADA_GRAPHICS_H

#include <FBase.h>
#include <FGraphics.h>
#include <FApp.h>
#include <FGraphicsOpengl.h>
#include <FSystem.h>
#include <FUi.h>

#include "config.h"
#include "backends/graphics/opengl/opengl-graphics.h"
#include "graphics/font.h"
#include "backends/platform/bada/form.h"

using namespace Osp::Graphics;
using namespace Osp::Graphics::Opengl;
using namespace Osp::App;

class BadaGraphicsManager : public OpenGLGraphicsManager {
public:
	BadaGraphicsManager(BadaAppForm *appForm);

	Common::List<Graphics::PixelFormat> getSupportedFormats() const;
	bool hasFeature(OSystem::Feature f);
	void updateScreen();
	void setFeatureState(OSystem::Feature f, bool enable);
	void setReady();
	bool isReady() { return !_initState; }
	const Graphics::Font *getFontOSD();
	bool moveMouse(int16 &x, int16 &y);

private:
	void internUpdateScreen();
	bool loadGFXMode();
	void loadTextures();
	void unloadGFXMode();
	void setInternalMousePosition(int x, int y) {}
	void showSplash();

	bool loadEgl();
	BadaAppForm *_appForm;
	EGLDisplay _eglDisplay;
	EGLSurface _eglSurface;
	EGLConfig	 _eglConfig;
	EGLContext _eglContext;
	bool _initState;
};

#endif
