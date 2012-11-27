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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/system.h"

#include "sword25/sword25.h"	// for kDebugScript
#include "sword25/gfx/bitmapresource.h"
#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/fontresource.h"
#include "sword25/gfx/panel.h"
#include "sword25/gfx/renderobjectmanager.h"
#include "sword25/gfx/screenshot.h"
#include "sword25/gfx/image/renderedimage.h"
#include "sword25/gfx/image/swimage.h"
#include "sword25/gfx/image/vectorimage.h"
#include "sword25/package/packagemanager.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"


#include "sword25/gfx/graphicengine.h"

#include "sword25/fmv/movieplayer.h"

#include "sword25/util/lua/lua.h"
#include "sword25/util/lua/lauxlib.h"
enum {
	BIT_DEPTH = 32,
	BACKBUFFER_COUNT = 1
};


namespace Sword25 {

static const uint FRAMETIME_SAMPLE_COUNT = 5;       // Anzahl der Framezeiten über die, die Framezeit gemittelt wird

GraphicEngine::GraphicEngine(Kernel *pKernel) :
	_width(0),
	_height(0),
	_bitDepth(0),
	_lastTimeStamp((uint) -1), // max. BS_INT64 um beim ersten Aufruf von _UpdateLastFrameDuration() einen Reset zu erzwingen
	_lastFrameDuration(0),
	_timerActive(true),
	_frameTimeSampleSlot(0),
	_thumbnail(NULL),
	ResourceService(pKernel) {
	_frameTimeSamples.resize(FRAMETIME_SAMPLE_COUNT);

	if (!registerScriptBindings())
		error("Script bindings could not be registered.");
	else
		debugC(kDebugScript, "Script bindings registered.");
}

GraphicEngine::~GraphicEngine() {
	unregisterScriptBindings();
	_backSurface.free();
	delete _thumbnail;
}

bool GraphicEngine::init(int width, int height, int bitDepth, int backbufferCount) {
	// Warnung ausgeben, wenn eine nicht unterstützte Bittiefe gewählt wurde.
	if (bitDepth != BIT_DEPTH) {
		warning("Can't use a bit depth of %d (not supported). Falling back to %d.", bitDepth, BIT_DEPTH);
		_bitDepth = BIT_DEPTH;
	}

	// Warnung ausgeben, wenn nicht genau ein Backbuffer gewählt wurde.
	if (backbufferCount != BACKBUFFER_COUNT) {
		warning("Can't use %d backbuffers (not supported). Falling back to %d.", backbufferCount, BACKBUFFER_COUNT);
		backbufferCount = BACKBUFFER_COUNT;
	}

	// Parameter in lokale Variablen kopieren
	_width = width;
	_height = height;
	_bitDepth = bitDepth;
	_screenRect.left = 0;
	_screenRect.top = 0;
	_screenRect.right = _width;
	_screenRect.bottom = _height;

	const Graphics::PixelFormat format = g_system->getScreenFormat();

	_backSurface.create(width, height, format);

	// Standardmäßig ist Vsync an.
	setVsync(true);

	// Layer-Manager initialisieren.
	_renderObjectManagerPtr.reset(new RenderObjectManager(width, height, backbufferCount + 1));

	// Hauptpanel erstellen
	_mainPanelPtr = _renderObjectManagerPtr->getTreeRoot()->addPanel(width, height, BS_ARGB(0, 0, 0, 0));
	if (!_mainPanelPtr.isValid())
		return false;
	_mainPanelPtr->setVisible(true);

	return true;
}

bool GraphicEngine::startFrame(bool updateAll) {
	// Berechnen, wie viel Zeit seit dem letzten Frame vergangen ist.
	// Dieser Wert kann über GetLastFrameDuration() von Modulen abgefragt werden, die zeitabhängig arbeiten.
	updateLastFrameDuration();

	// Den Layer-Manager auf den nächsten Frame vorbereiten
	_renderObjectManagerPtr->startFrame();

	return true;
}

bool GraphicEngine::endFrame() {
#ifndef THEORA_INDIRECT_RENDERING
	if (Kernel::getInstance()->getFMV()->isMovieLoaded())
		return true;
#endif

	_renderObjectManagerPtr->render();

	g_system->updateScreen();

	return true;
}

RenderObjectPtr<Panel> GraphicEngine::getMainPanel() {
	return _mainPanelPtr;
}

void GraphicEngine::setVsync(bool vsync) {
	// ScummVM has no concept of VSync
}

bool GraphicEngine::getVsync() const {
	warning("STUB: getVsync()");

	return true;
}

bool GraphicEngine::fill(const Common::Rect *fillRectPtr, uint color) {
	Common::Rect rect(_width - 1, _height - 1);

	int ca = (color >> 24) & 0xff;

	if (ca == 0)
		return true;

	int cr = (color >> 16) & 0xff;
	int cg = (color >> 8) & 0xff;
	int cb = (color >> 0) & 0xff;

	if (fillRectPtr) {
		rect = *fillRectPtr;
	}

	if (rect.width() > 0 && rect.height() > 0) {
		if (ca == 0xff) {
			_backSurface.fillRect(rect, color);
		} else {
			byte *outo = (byte *)_backSurface.getBasePtr(rect.left, rect.top);
			byte *out;

			for (int i = rect.top; i < rect.bottom; i++) {
				out = outo;
				for (int j = rect.left; j < rect.right; j++) {
#if defined(SCUMM_LITTLE_ENDIAN)
					*out += (byte)(((cb - *out) * ca) >> 8);
					out++;
					*out += (byte)(((cg - *out) * ca) >> 8);
					out++;
					*out += (byte)(((cr - *out) * ca) >> 8);
					out++;
					*out = 255;
					out++;
#else
					*out = 255;
					out++;
					*out += (byte)(((cr - *out) * ca) >> 8);
					out++;
					*out += (byte)(((cg - *out) * ca) >> 8);
					out++;
					*out += (byte)(((cb - *out) * ca) >> 8);
					out++;
#endif
				}

				outo += _backSurface.pitch;
			}
		}

		g_system->copyRectToScreen(_backSurface.getBasePtr(rect.left, rect.top), _backSurface.pitch, rect.left, rect.top, rect.width(), rect.height());
	}

	return true;
}

// -----------------------------------------------------------------------------
// RESOURCE MANAGING
// -----------------------------------------------------------------------------

Resource *GraphicEngine::loadResource(const Common::String &filename) {
	assert(canLoadResource(filename));

	// Load image for "software buffer". These are images used to dynamically
	// add shadows to actors (e.g. when George walks under the shadow of a
	// a building)
	if (filename.hasSuffix("_s.png")) {
		bool result = false;
		SWImage *pImage = new SWImage(filename, result);
		if (!result) {
			delete pImage;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(filename, pImage);
		if (!pResource->isValid()) {
			delete pResource;
			return 0;
		}

		return pResource;
	}

	// Load sprite image. Savegame thumbnails are also loaded here.
	if (filename.hasSuffix(".png") || filename.hasSuffix(".b25s") ||
		filename.hasPrefix("/saves")) {
		bool result = false;
		RenderedImage *pImage = new RenderedImage(filename, result);
		if (!result) {
			delete pImage;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(filename, pImage);
		if (!pResource->isValid()) {
			delete pResource;
			return 0;
		}

		return pResource;
	}


	// Load vector graphics
	if (filename.hasSuffix(".swf")) {
		debug(2, "VectorImage: %s", filename.c_str());

		// Pointer auf Package-Manager holen
		PackageManager *pPackage = Kernel::getInstance()->getPackage();
		assert(pPackage);

		// Datei laden
		byte *pFileData;
		uint fileSize;
		pFileData = pPackage->getFile(filename, &fileSize);
		if (!pFileData) {
			error("File \"%s\" could not be loaded.", filename.c_str());
			return 0;
		}

		bool result = false;
		VectorImage *pImage = new VectorImage(pFileData, fileSize, result, filename);
		if (!result) {
			delete pImage;
			delete[] pFileData;
			return 0;
		}

		BitmapResource *pResource = new BitmapResource(filename, pImage);
		if (!pResource->isValid()) {
			delete pResource;
			delete[] pFileData;
			return 0;
		}

		delete[] pFileData;
		return pResource;
	}

	// Load animation
	if (filename.hasSuffix("_ani.xml")) {
		AnimationResource *pResource = new AnimationResource(filename);
		if (pResource->isValid())
			return pResource;
		else {
			delete pResource;
			return 0;
		}
	}

	// Load font
	if (filename.hasSuffix("_fnt.xml")) {
		FontResource *pResource = new FontResource(Kernel::getInstance(), filename);
		if (pResource->isValid())
			return pResource;
		else {
			delete pResource;
			return 0;
		}
	}

	error("Service cannot load \"%s\".", filename.c_str());
	return 0;
}

// -----------------------------------------------------------------------------

bool GraphicEngine::canLoadResource(const Common::String &filename) {
	return filename.hasSuffix(".png") ||
		filename.hasSuffix("_ani.xml") ||
		filename.hasSuffix("_fnt.xml") ||
		filename.hasSuffix(".swf") ||
		filename.hasSuffix(".b25s") ||
		filename.hasPrefix("/saves");
}

void  GraphicEngine::updateLastFrameDuration() {
	// Record current time
	const uint currentTime = Kernel::getInstance()->getMilliTicks();

	// Compute the elapsed time since the last frame and prevent too big ( > 250 msecs) time jumps.
	// These can occur when loading save states, during debugging or due to hardware inaccuracies.
	_frameTimeSamples[_frameTimeSampleSlot] = static_cast<uint>(currentTime - _lastTimeStamp);
	if (_frameTimeSamples[_frameTimeSampleSlot] > 250000)
		_frameTimeSamples[_frameTimeSampleSlot] = 250000;
	_frameTimeSampleSlot = (_frameTimeSampleSlot + 1) % FRAMETIME_SAMPLE_COUNT;

	// Compute the average frame duration over multiple frames to eliminate outliers.
	Common::Array<uint>::const_iterator it = _frameTimeSamples.begin();
	uint sum = *it;
	for (it++; it != _frameTimeSamples.end(); it++)
		sum += *it;
	_lastFrameDuration = sum * 1000 / FRAMETIME_SAMPLE_COUNT;

	// Update m_LastTimeStamp with the current frame's timestamp
	_lastTimeStamp = currentTime;
}

bool GraphicEngine::saveThumbnailScreenshot(const Common::String &filename) {
	// Note: In ScumMVM, rather than saivng the thumbnail to a file, we store it in memory
	// until needed when creating savegame files
	delete _thumbnail;

	_thumbnail = Screenshot::createThumbnail(&_backSurface);

	return true;
}

void GraphicEngine::ARGBColorToLuaColor(lua_State *L, uint color) {
	lua_Number components[4] = {
		(color >> 16) & 0xff,   // Rot
		(color >> 8) & 0xff,    // Grün
		color & 0xff,          // Blau
		color >> 24,           // Alpha
	};

	lua_newtable(L);

	for (uint i = 1; i <= 4; i++) {
		lua_pushnumber(L, i);
		lua_pushnumber(L, components[i - 1]);
		lua_settable(L, -3);
	}
}

uint GraphicEngine::luaColorToARGBColor(lua_State *L, int stackIndex) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Sicherstellen, dass wir wirklich eine Tabelle betrachten
	luaL_checktype(L, stackIndex, LUA_TTABLE);
	// Größe der Tabelle auslesen
	uint n = luaL_getn(L, stackIndex);
	// RGB oder RGBA Farben werden unterstützt und sonst keine
	if (n != 3 && n != 4)
		luaL_argcheck(L, 0, stackIndex, "at least 3 of the 4 color components have to be specified");

	// Red color component reading
	lua_rawgeti(L, stackIndex, 1);
	uint red = static_cast<uint>(lua_tonumber(L, -1));
	if (!lua_isnumber(L, -1) || red >= 256)
		luaL_argcheck(L, 0, stackIndex, "red color component must be an integer between 0 and 255");
	lua_pop(L, 1);

	// Green color component reading
	lua_rawgeti(L, stackIndex, 2);
	uint green = static_cast<uint>(lua_tonumber(L, -1));
	if (!lua_isnumber(L, -1) || green >= 256)
		luaL_argcheck(L, 0, stackIndex, "green color component must be an integer between 0 and 255");
	lua_pop(L, 1);

	// Blue color component reading
	lua_rawgeti(L, stackIndex, 3);
	uint blue = static_cast<uint>(lua_tonumber(L, -1));
	if (!lua_isnumber(L, -1) || blue >= 256)
		luaL_argcheck(L, 0, stackIndex, "blue color component must be an integer between 0 and 255");
	lua_pop(L, 1);

	// Alpha color component reading
	uint alpha = 0xff;
	if (n == 4) {
		lua_rawgeti(L, stackIndex, 4);
		alpha = static_cast<uint>(lua_tonumber(L, -1));
		if (!lua_isnumber(L, -1) || alpha >= 256)
			luaL_argcheck(L, 0, stackIndex, "alpha color component must be an integer between 0 and 255");
		lua_pop(L, 1);
	}

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(L));
#endif

	return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

bool GraphicEngine::persist(OutputPersistenceBlock &writer) {
	writer.write(_timerActive);

	bool result = _renderObjectManagerPtr->persist(writer);

	return result;
}

bool GraphicEngine::unpersist(InputPersistenceBlock &reader) {
	reader.read(_timerActive);
	_renderObjectManagerPtr->unpersist(reader);

	return reader.isGood();
}

} // End of namespace Sword25
